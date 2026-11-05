#include "renderer.h"
#include "trainer.h"
#include "map_mesh.h"
#include "preaim_course.h"
#include "gl_loader.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// Shaders
// ---------------------------------------------------------------------------

static const char* kWallVS = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uViewProj;
uniform mat4 uModel;
out vec3 vWorld;
void main() {
    vec4 w = uModel * vec4(aPos, 1.0);
    vWorld = w.xyz;
    gl_Position = uViewProj * w;
}
)GLSL";

static const char* kWallFS = R"GLSL(
#version 330 core
in vec3 vWorld;
out vec4 FragColor;
void main() {
    vec3 base = vec3(0.07, 0.08, 0.11);
    vec2 gp = abs(fract(vWorld.xy + 0.5) - 0.5) / fwidth(vWorld.xy);
    float grid = 1.0 - min(min(gp.x, gp.y), 1.0);
    vec3 col = mix(base, vec3(0.16, 0.19, 0.25), grid * 0.55);
    FragColor = vec4(col, 1.0);
}
)GLSL";

static const char* kTargetVS = R"GLSL(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uViewProj;
uniform vec3 uCenter;
uniform float uRadius;
uniform vec3 uRight;
uniform vec3 uUp;
out vec2 vP;
void main() {
    vec3 world = uCenter + (aPos.x * uRadius) * uRight
                        + (aPos.y * uRadius) * uUp;
    vP = aPos;
    gl_Position = uViewProj * vec4(world, 1.0);
}
)GLSL";

static const char* kTargetFS = R"GLSL(
#version 330 core
in vec2 vP;
uniform vec3 uColInner;
uniform vec3 uColRing;
out vec4 FragColor;
void main() {
    float d = length(vP);
    if (d > 1.0) discard;
    float ring = smoothstep(0.60, 0.74, d);
    FragColor = vec4(mix(uColInner, uColRing, ring), 1.0);
}
)GLSL";

static const char* kMeshVS = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;
uniform mat4 uViewProj;
uniform mat4 uModel;
out vec3 vNormal;
out vec4 vColor;
out vec3 vWorldPos;
void main() {
    vec4 w = uModel * vec4(aPos, 1.0);
    vWorldPos = w.xyz;
    vNormal = mat3(uModel) * aNormal;
    vColor = aColor;
    gl_Position = uViewProj * w;
}
)GLSL";

static const char* kMeshFS = R"GLSL(
#version 330 core
in vec3 vNormal;
in vec4 vColor;
in vec3 vWorldPos;
uniform vec3 uLightDir;
uniform vec3 uGlowCol;
uniform float uGlowMix;
out vec4 FragColor;
void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    float diff = max(dot(N, L), 0.0);
    float wrap = diff * 0.45 + 0.55;
    vec3 skyLight = vec3(0.88, 0.92, 1.0) * wrap;
    vec3 groundLight = vec3(0.38, 0.35, 0.32) * (1.0 - wrap);
    vec3 ambient = skyLight * 0.70 + groundLight * 0.30;
    vec3 lit = vColor.rgb * ambient;
    if (uGlowMix > 0.0) {
        lit = mix(lit, uGlowCol, uGlowMix);
    }
    FragColor = vec4(lit, vColor.a);
}
)GLSL";

// ---------------------------------------------------------------------------

unsigned Renderer::compileShader(unsigned type, const char* src) {
    unsigned sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    int ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        std::printf("[renderer] shader compile failed:\n%s\n", log);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

unsigned Renderer::linkProgram(unsigned vs, unsigned fs) {
    unsigned prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    int ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        std::printf("[renderer] program link failed:\n%s\n", log);
        glDeleteProgram(prog);
        return 0;
    }
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    return prog;
}

bool Renderer::init() {
    unsigned wallVS = compileShader(GL_VERTEX_SHADER, kWallVS);
    unsigned wallFS = compileShader(GL_FRAGMENT_SHADER, kWallFS);
    unsigned tgtVS  = compileShader(GL_VERTEX_SHADER, kTargetVS);
    unsigned tgtFS  = compileShader(GL_FRAGMENT_SHADER, kTargetFS);
    unsigned meshVS = compileShader(GL_VERTEX_SHADER, kMeshVS);
    unsigned meshFS = compileShader(GL_FRAGMENT_SHADER, kMeshFS);

    if (!wallVS || !wallFS || !tgtVS || !tgtFS || !meshVS || !meshFS) return false;

    m_wallProg   = linkProgram(wallVS, wallFS);
    m_targetProg = linkProgram(tgtVS, tgtFS);
    m_meshProg   = linkProgram(meshVS, meshFS);

    glDeleteShader(wallVS); glDeleteShader(wallFS);
    glDeleteShader(tgtVS);  glDeleteShader(tgtFS);
    glDeleteShader(meshVS); glDeleteShader(meshFS);

    if (!m_wallProg || !m_targetProg || !m_meshProg) return false;

    m_wViewProj = glGetUniformLocation(m_wallProg, "uViewProj");
    m_wModel    = glGetUniformLocation(m_wallProg, "uModel");
    m_tViewProj = glGetUniformLocation(m_targetProg, "uViewProj");
    m_tCenter   = glGetUniformLocation(m_targetProg, "uCenter");
    m_tRadius   = glGetUniformLocation(m_targetProg, "uRadius");
    m_tRight    = glGetUniformLocation(m_targetProg, "uRight");
    m_tUp       = glGetUniformLocation(m_targetProg, "uUp");
    m_tColInner = glGetUniformLocation(m_targetProg, "uColInner");
    m_tColRing  = glGetUniformLocation(m_targetProg, "uColRing");

    m_mViewProj = glGetUniformLocation(m_meshProg, "uViewProj");
    m_mModel    = glGetUniformLocation(m_meshProg, "uModel");
    m_mLightDir = glGetUniformLocation(m_meshProg, "uLightDir");
    m_mGlowCol  = glGetUniformLocation(m_meshProg, "uGlowCol");
    m_mGlowMix  = glGetUniformLocation(m_meshProg, "uGlowMix");

    const float quad[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };
    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);
    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glBindVertexArray(0);

    return true;
}

void Renderer::shutdown() {
    if (m_quadVbo) glDeleteBuffers(1, &m_quadVbo);
    if (m_quadVao) glDeleteVertexArrays(1, &m_quadVao);
    if (m_wallProg) glDeleteProgram(m_wallProg);
    if (m_targetProg) glDeleteProgram(m_targetProg);
    if (m_meshProg) glDeleteProgram(m_meshProg);
    m_quadVbo = m_quadVao = m_wallProg = m_targetProg = m_meshProg = 0;
}

void Renderer::renderWall(const Mat4& viewProj, float wallDist) {
    glDisable(GL_DEPTH_TEST);
    Mat4 model = mat4Identity();
    model.m[12] = 0.0f;
    model.m[13] = 0.0f;
    model.m[14] = -wallDist;

    glUseProgram(m_wallProg);
    glUniformMatrix4fv(m_wViewProj, 1, GL_FALSE, viewProj.m);
    glUniformMatrix4fv(m_wModel, 1, GL_FALSE, model.m);
    glBindVertexArray(m_quadVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::renderTargets(const Mat4& viewProj, const Trainer& trainer, const Camera& cam) {
    glDisable(GL_DEPTH_TEST);
    Vec3 fwd   = cam.forward();
    Vec3 right = normalize(cross(fwd, { 0.0f, 1.0f, 0.0f }));
    Vec3 up    = cross(right, fwd);

    glUseProgram(m_targetProg);
    glUniformMatrix4fv(m_tViewProj, 1, GL_FALSE, viewProj.m);
    glUniform3f(m_tRight, right.x, right.y, right.z);
    glUniform3f(m_tUp, up.x, up.y, up.z);

    glBindVertexArray(m_quadVao);
    for (int i = 0; i < trainer.targetCount(); ++i) {
        const Target& t = trainer.target(i);
        if (!t.alive) continue;
        glUniform3f(m_tCenter, t.center.x, t.center.y, t.center.z);
        glUniform1f(m_tRadius, t.radius);
        glUniform3f(m_tColInner, t.colorInner[0], t.colorInner[1], t.colorInner[2]);
        glUniform3f(m_tColRing,  t.colorRing[0],  t.colorRing[1],  t.colorRing[2]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    glBindVertexArray(0);
}

void Renderer::renderMap(const Mat4& viewProj, MapMesh& map) {
    if (!map.isLoaded()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(m_meshProg);
    glUniformMatrix4fv(m_mViewProj, 1, GL_FALSE, viewProj.m);
    glUniform3f(m_mLightDir, 0.45f, 0.85f, 0.25f);
    glUniform3f(m_mGlowCol, 0.0f, 0.0f, 0.0f);
    glUniform1f(m_mGlowMix, 0.0f);

    Mat4 identity = mat4Identity();
    map.render(m_meshProg, m_mModel, identity);
}

void Renderer::renderBot(const Mat4& viewProj, MapMesh& botMesh, Vec3 footPos, float yawDeg, bool isAlert, bool /*isTargeted*/) {
    if (!botMesh.isLoaded()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glUseProgram(m_meshProg);
    glUniformMatrix4fv(m_mViewProj, 1, GL_FALSE, viewProj.m);
    glUniform3f(m_mLightDir, 0.45f, 0.85f, 0.25f);

    if (isAlert) {
        glUniform3f(m_mGlowCol, 1.0f, 0.25f, 0.1f);
        glUniform1f(m_mGlowMix, 0.45f);
    } else {
        glUniform3f(m_mGlowCol, 0.95f, 0.80f, 0.20f);
        glUniform1f(m_mGlowMix, 0.15f);
    }

    Mat4 model = mat4Mul(mat4Translate(footPos), mat4RotateY(yawDeg));
    botMesh.render(m_meshProg, m_mModel, model);
}

void Renderer::renderPreAimBots(const Mat4& viewProj, MapMesh& botMesh, const PreAimCourse& course) {
    for (const auto& b : course.bots()) {
        if (!b.alive) continue;
        renderBot(viewProj, botMesh, b.pos, b.yawDeg, b.hasSeenPlayer, false);
    }
}
