#include "renderer.h"
#include "trainer.h"
#include "map_mesh.h"
#include "preaim_course.h"
#include "gl_loader.h"
#include <cstdio>
#include <cmath>

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
uniform int uSurfaceType; // 0 = back/side wall, 1 = floor, 2 = ceiling, 3 = cover wall
out vec4 FragColor;
void main() {
    if (uSurfaceType == 1) {
        // Range Floor (warm sandstone/concrete with 1m grid & 5m distance lines)
        vec3 base = vec3(0.22, 0.21, 0.20);
        vec2 gp = abs(fract(vWorld.xz + 0.5) - 0.5) / max(fwidth(vWorld.xz), vec2(1e-4));
        float grid1 = 1.0 - min(min(gp.x, gp.y), 1.0);
        vec2 gp5 = abs(fract(vWorld.xz * 0.2 + 0.5) - 0.5) / max(fwidth(vWorld.xz * 0.2), vec2(1e-4));
        float grid5 = 1.0 - min(min(gp5.x, gp5.y), 1.0);
        vec3 col = mix(base, vec3(0.32, 0.30, 0.28), grid1 * 0.40);
        col = mix(col, vec3(0.85, 0.52, 0.20), grid5 * 0.65);
        float distFade = clamp(1.0 - length(vWorld.xz) / 45.0, 0.25, 1.0);
        FragColor = vec4(col * distFade, 1.0);
    } else if (uSurfaceType == 2) {
        // Range Ceiling
        vec3 base = vec3(0.09, 0.10, 0.13);
        vec2 gp = abs(fract(vWorld.xz * 0.5 + 0.5) - 0.5) / max(fwidth(vWorld.xz * 0.5), vec2(1e-4));
        float grid = 1.0 - min(min(gp.x, gp.y), 1.0);
        FragColor = vec4(mix(base, vec3(0.15, 0.17, 0.22), grid * 0.35), 1.0);
    } else if (uSurfaceType == 3) {
        // Angle-Hold Cover Wall (tactical concrete barrier with edge highlight)
        vec3 base = vec3(0.28, 0.30, 0.34);
        vec2 gp = abs(fract(vWorld.xy + 0.5) - 0.5) / max(fwidth(vWorld.xy), vec2(1e-4));
        float grid = 1.0 - min(min(gp.x, gp.y), 1.0);
        vec3 col = mix(base, vec3(0.38, 0.41, 0.46), grid * 0.35);
        // Head-level line at Y = 0.0
        float headLine = 1.0 - min(abs(vWorld.y) / max(fwidth(vWorld.y) * 1.8, 1e-4), 1.0);
        col = mix(col, vec3(0.95, 0.75, 0.25), headLine * 0.65);
        FragColor = vec4(col, 1.0);
    } else {
        // Range Back/Side Wall with 1m grid and crisp Y=0.0 head-level horizon line
        vec3 base = vec3(0.14, 0.16, 0.20);
        vec2 gp = abs(fract(vWorld.xy + 0.5) - 0.5) / max(fwidth(vWorld.xy), vec2(1e-4));
        float grid = 1.0 - min(min(gp.x, gp.y), 1.0);
        vec3 col = mix(base, vec3(0.23, 0.27, 0.34), grid * 0.45);
        // Subtle glowing horizontal head-level line at Y = 0.0m (eye height)
        float headLine = 1.0 - min(abs(vWorld.y) / max(fwidth(vWorld.y) * 2.0, 1e-4), 1.0);
        col = mix(col, vec3(0.30, 0.80, 0.95), headLine * 0.50);
        FragColor = vec4(col, 1.0);
    }
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
    float r2 = dot(vP, vP);
    if (r2 > 1.0) discard;
    float d = sqrt(r2);
    // 3D sphere normal and specular highlight
    float z = sqrt(max(0.0, 1.0 - r2));
    vec3 N = normalize(vec3(vP.x, vP.y, z));
    vec3 L = normalize(vec3(-0.35, 0.55, 0.75));
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(reflect(-L, N), vec3(0.0, 0.0, 1.0)), 0.0), 28.0);
    float ring = smoothstep(0.68, 0.82, d);
    vec3 baseCol = mix(uColInner, uColRing, ring);
    vec3 lit = baseCol * (0.45 + 0.55 * diff) + vec3(0.85, 0.95, 1.0) * (spec * 0.45);
    FragColor = vec4(lit, 1.0);
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
uniform vec3 uCamPos;
uniform vec3 uGlowCol;
uniform float uGlowMix;
uniform int uIsBot;
out vec4 FragColor;
void main() {
    vec3 toCam = uCamPos - vWorldPos;
    float distToCam = length(toCam);
    vec3 V = toCam / max(distToCam, 1e-4);
    vec3 L = normalize(uLightDir);

    if (uIsBot == 1) {
        // 3D Humanoid Jett Bot with baked UV diffuse texture + Valorant purple/red enemy rim highlight
        vec3 N = normalize(vNormal);
        if (dot(N, V) < 0.0) N = -N;

        float diff = max(dot(N, L), 0.0);
        float hemi = N.y * 0.5 + 0.5;
        vec3 light = vec3(0.95, 0.95, 1.0) * (diff * 0.55)
                   + mix(vec3(0.38, 0.35, 0.42), vec3(0.75, 0.78, 0.88), hemi) * 0.65;

        vec3 baseCol = vColor.rgb * light;

        // Sharp Fresnel enemy silhouette outline (Valorant purple/red enemy highlight)
        float fresnel = pow(1.0 - clamp(abs(dot(N, V)), 0.0, 1.0), 2.0);
        baseCol = mix(baseCol, uGlowCol, clamp(fresnel * 0.75 + uGlowMix * 0.28, 0.0, 0.88));

        FragColor = vec4(baseCol, 1.0);
    } else {
        // 3D Map Architecture: flat geometric face normal via screen-space derivatives
        vec3 dpx = dFdx(vWorldPos);
        vec3 dpy = dFdy(vWorldPos);
        vec3 faceN = normalize(cross(dpx, dpy));
        if (dot(faceN, V) < 0.0) faceN = -faceN;

        float absNy = abs(faceN.y);
        vec3 archTint;
        float gridFactor = 0.0;

        if (absNy > 0.62) {
            // Walkable Floor / Box Top: warm sandstone tile
            archTint = vec3(0.84, 0.79, 0.72);
            vec2 gp = abs(fract(vWorldPos.xz * 0.5 + 0.5) - 0.5) / max(fwidth(vWorldPos.xz * 0.5), vec2(1e-4));
            gridFactor = (1.0 - min(min(gp.x, gp.y), 1.0)) * 0.12;
        } else if (absNy < 0.35) {
            // Vertical Wall: clean cool architectural plaster/stone
            archTint = vec3(0.70, 0.69, 0.67);
            vec2 gp = abs(fract(vec2(vWorldPos.x + vWorldPos.z, vWorldPos.y * 0.5) + 0.5) - 0.5)
                    / max(fwidth(vec2(vWorldPos.x + vWorldPos.z, vWorldPos.y * 0.5)), vec2(1e-4));
            gridFactor = (1.0 - min(min(gp.x, gp.y), 1.0)) * 0.08;
        } else {
            // Sloped Ramp / Trim / Roof
            archTint = vec3(0.76, 0.73, 0.68);
        }

        vec3 baseCol = mix(vColor.rgb, archTint, 0.55) * (1.0 - gridFactor);

        // Two-sided directional sun + sky/ground hemisphere lighting so every corner edge has crisp contrast
        float sun1 = max(dot(faceN, L), 0.0);
        float sun2 = max(dot(faceN, normalize(vec3(-0.65, 0.40, -0.60))), 0.0) * 0.45;
        float hemi = faceN.y * 0.5 + 0.5;
        float shade = 0.42 + 0.42 * max(sun1, sun2) + 0.16 * hemi;
        // Differentiate X-facing vs Z-facing walls slightly for unambiguous corner depth perception
        shade *= (1.0 + 0.08 * (abs(faceN.x) - abs(faceN.z)));

        vec3 lit = baseCol * shade;

        // Subtle atmospheric distance fog
        float fog = clamp((distToCam - 22.0) / 75.0, 0.0, 0.42);
        lit = mix(lit, vec3(0.15, 0.18, 0.23), fog);

        FragColor = vec4(lit, 1.0);
    }
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

    m_wViewProj    = glGetUniformLocation(m_wallProg, "uViewProj");
    m_wModel       = glGetUniformLocation(m_wallProg, "uModel");
    m_wSurfaceType = glGetUniformLocation(m_wallProg, "uSurfaceType");

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
    m_mCamPos   = glGetUniformLocation(m_meshProg, "uCamPos");
    m_mGlowCol  = glGetUniformLocation(m_meshProg, "uGlowCol");
    m_mGlowMix  = glGetUniformLocation(m_meshProg, "uGlowMix");
    m_mIsBot    = glGetUniformLocation(m_meshProg, "uIsBot");

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

static Mat4 makeBoxPlane(Vec3 center, Vec3 rightAxis, Vec3 upAxis) {
    Mat4 m{};
    m.m[0]  = rightAxis.x; m.m[1]  = rightAxis.y; m.m[2]  = rightAxis.z; m.m[3]  = 0.0f;
    m.m[4]  = upAxis.x;    m.m[5]  = upAxis.y;    m.m[6]  = upAxis.z;    m.m[7]  = 0.0f;
    m.m[8]  = 0.0f;        m.m[9]  = 0.0f;        m.m[10] = 1.0f;        m.m[11] = 0.0f;
    m.m[12] = center.x;    m.m[13] = center.y;    m.m[14] = center.z;    m.m[15] = 1.0f;
    return m;
}

void Renderer::renderWall(const Mat4& viewProj, const Camera& /*cam*/, float wallDist, bool hasCoverWall, float coverWallX) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glUseProgram(m_wallProg);
    glUniformMatrix4fv(m_wViewProj, 1, GL_FALSE, viewProj.m);
    glBindVertexArray(m_quadVao);

    const float halfW = 18.0f;
    const float floorY = -1.62f; // Player eye is at Y = 0.0 in Range mode, so floor is at -1.62m
    const float ceilY  =  6.38f;
    const float nearZ  =  4.0f;
    const float farZ   = -wallDist - 4.0f;
    const float midZ   = 0.5f * (nearZ + farZ);
    const float halfLenZ = 0.5f * (nearZ - farZ);
    const float midY   = 0.5f * (floorY + ceilY);
    const float halfH  = 0.5f * (ceilY - floorY);

    // 1. Back Wall at Z = -wallDist
    {
        Mat4 model = makeBoxPlane({ 0.0f, midY, -wallDist - 0.35f }, { halfW, 0.0f, 0.0f }, { 0.0f, halfH, 0.0f });
        glUniform1i(m_wSurfaceType, 0);
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, model.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 2. Left & Right Side Walls
    {
        Mat4 leftW = makeBoxPlane({ -halfW, midY, midZ }, { 0.0f, 0.0f, halfLenZ }, { 0.0f, halfH, 0.0f });
        glUniform1i(m_wSurfaceType, 0);
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, leftW.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        Mat4 rightW = makeBoxPlane({ halfW, midY, midZ }, { 0.0f, 0.0f, halfLenZ }, { 0.0f, halfH, 0.0f });
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, rightW.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 3. Floor at Y = -1.62m
    {
        Mat4 floorM = makeBoxPlane({ 0.0f, floorY, midZ }, { halfW, 0.0f, 0.0f }, { 0.0f, 0.0f, halfLenZ });
        glUniform1i(m_wSurfaceType, 1);
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, floorM.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 4. Ceiling at Y = 6.38m
    {
        Mat4 ceilM = makeBoxPlane({ 0.0f, ceilY, midZ }, { halfW, 0.0f, 0.0f }, { 0.0f, 0.0f, halfLenZ });
        glUniform1i(m_wSurfaceType, 2);
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, ceilM.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // 5. Optional Angle-Hold Cover Wall (occludes hidden bot until it peeks out at X > coverWallX)
    if (hasCoverWall) {
        float coverCenterX = coverWallX - 2.5f;
        Mat4 coverM = makeBoxPlane({ coverCenterX, 0.8f, -wallDist + 1.2f }, { 2.5f, 0.0f, 0.0f }, { 0.0f, 2.42f, 0.0f });
        glUniform1i(m_wSurfaceType, 3);
        glUniformMatrix4fv(m_wModel, 1, GL_FALSE, coverM.m);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
}

static bool isHumanoidDrill(ScenarioId id) {
    return id == ScenarioId::HeadshotLine ||
           id == ScenarioId::SmoothStrafe ||
           id == ScenarioId::ValorantStrafe ||
           id == ScenarioId::CounterStrafeDrill ||
           id == ScenarioId::AngleHold ||
           id == ScenarioId::OffAngleStrafeConfirm ||
           id == ScenarioId::PeekersAdvantage;
}

void Renderer::renderTargets(const Mat4& viewProj, const Trainer& trainer, const Camera& cam, MapMesh* botMesh) {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    bool useHumanoid = isHumanoidDrill(trainer.scenarioId()) && botMesh && botMesh->isLoaded();

    // 1. Render 3D Jett humanoid model on duel / strafe / headshot drills when target is at eye level
    if (useHumanoid) {
        for (int i = 0; i < trainer.targetCount(); ++i) {
            const Target& t = trainer.target(i);
            if (!t.alive) continue;
            // Jett's eyes are at +1.62m above feet, so placing feet at center.y - 1.62m aligns head with t.center!
            Vec3 footPos = { t.center.x, t.center.y - 1.62f, t.center.z };
            float dx = cam.pos.x - t.center.x;
            float dz = cam.pos.z - t.center.z;
            float yawToCam = degrees(std::atan2(dx, dz));
            renderBot(viewProj, cam, *botMesh, footPos, yawToCam, true, false);
        }
    }

    // 2. Render 3D spheres for multi-target clicking drills (or when botMesh is not used)
    if (!useHumanoid) {
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
}

void Renderer::renderMap(const Mat4& viewProj, const Camera& cam, MapMesh& map) {
    if (!map.isLoaded()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glUseProgram(m_meshProg);
    glUniformMatrix4fv(m_mViewProj, 1, GL_FALSE, viewProj.m);
    glUniform3f(m_mLightDir, 0.45f, 0.82f, 0.35f);
    glUniform3f(m_mCamPos, cam.pos.x, cam.pos.y, cam.pos.z);
    glUniform3f(m_mGlowCol, 0.0f, 0.0f, 0.0f);
    glUniform1f(m_mGlowMix, 0.0f);
    glUniform1i(m_mIsBot, 0);

    Mat4 identity = mat4Identity();
    map.render(m_meshProg, m_mModel, identity);
}

void Renderer::renderBot(const Mat4& viewProj, const Camera& cam, MapMesh& botMesh, Vec3 footPos, float yawDeg, bool isAlert, bool /*isTargeted*/) {
    if (!botMesh.isLoaded()) return;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glUseProgram(m_meshProg);
    glUniformMatrix4fv(m_mViewProj, 1, GL_FALSE, viewProj.m);
    glUniform3f(m_mLightDir, 0.45f, 0.82f, 0.35f);
    glUniform3f(m_mCamPos, cam.pos.x, cam.pos.y, cam.pos.z);
    glUniform1i(m_mIsBot, 1);

    if (isAlert) {
        // Valorant enemy highlight: vibrant crimson/magenta-purple rim glow when engaged
        glUniform3f(m_mGlowCol, 1.0f, 0.22f, 0.38f);
        glUniform1f(m_mGlowMix, 0.45f);
    } else {
        // Valorant purple enemy outline
        glUniform3f(m_mGlowCol, 0.85f, 0.25f, 0.95f);
        glUniform1f(m_mGlowMix, 0.25f);
    }

    Mat4 model = mat4Mul(mat4Translate(footPos), mat4RotateY(yawDeg));
    botMesh.render(m_meshProg, m_mModel, model);
}

void Renderer::renderPreAimBots(const Mat4& viewProj, const Camera& cam, MapMesh& botMesh, const PreAimCourse& course) {
    for (const auto& b : course.bots()) {
        if (!b.alive) continue;
        renderBot(viewProj, cam, botMesh, b.pos, b.yawDeg, b.hasSeenPlayer, false);
    }
}
