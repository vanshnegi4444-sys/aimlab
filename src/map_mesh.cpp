#define NOMINMAX
#include "map_mesh.h"
#include "gl_loader.h"
#include <cstdio>
#include <algorithm>
#include <cmath>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

static inline float min3f(float a, float b, float c) {
    float m = a < b ? a : b;
    return m < c ? m : c;
}

static inline float max3f(float a, float b, float c) {
    float m = a > b ? a : b;
    return m > c ? m : c;
}

MapMesh::MapMesh() {}

MapMesh::~MapMesh() {
    destroy();
}

void MapMesh::destroy() {
    if (m_ebo) { glDeleteBuffers(1, &m_ebo); m_ebo = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    m_floorTris.clear();
    m_loaded = false;
    m_numVerts = m_numTris = 0;
}

bool MapMesh::load(const std::string& binPath) {
    destroy();

    FILE* f = std::fopen(binPath.c_str(), "rb");
    if (!f) {
        std::printf("[map_mesh] Failed to open file: %s\n", binPath.c_str());
        return false;
    }

    VMapHeader hdr{};
    if (std::fread(&hdr, sizeof(hdr), 1, f) != 1) {
        std::printf("[map_mesh] Failed to read header: %s\n", binPath.c_str());
        std::fclose(f);
        return false;
    }

    if (std::memcmp(hdr.magic, "VMAP", 4) != 0 && std::memcmp(hdr.magic, "VBOT", 4) != 0) {
        std::printf("[map_mesh] Invalid magic in: %s\n", binPath.c_str());
        std::fclose(f);
        return false;
    }

    m_numVerts = hdr.numVerts;
    m_numTris  = hdr.numTris;
    m_bMin = { hdr.minX, hdr.minY, hdr.minZ };
    m_bMax = { hdr.maxX, hdr.maxY, hdr.maxZ };

    std::vector<VMapVertex> verts(m_numVerts);
    if (std::fread(verts.data(), sizeof(VMapVertex), m_numVerts, f) != m_numVerts) {
        std::printf("[map_mesh] Failed to read %u vertices: %s\n", m_numVerts, binPath.c_str());
        std::fclose(f);
        return false;
    }

    std::vector<uint32_t> indices(m_numTris * 3);
    if (std::fread(indices.data(), sizeof(uint32_t), m_numTris * 3, f) != m_numTris * 3) {
        std::printf("[map_mesh] Failed to read %u indices: %s\n", m_numTris * 3, binPath.c_str());
        std::fclose(f);
        return false;
    }
    std::fclose(f);

    // Extract walkable floor triangles
    m_floorTris.reserve(m_numTris / 4);
    for (uint32_t i = 0; i < m_numTris; ++i) {
        uint32_t i0 = indices[i * 3 + 0];
        uint32_t i1 = indices[i * 3 + 1];
        uint32_t i2 = indices[i * 3 + 2];
        if (i0 >= m_numVerts || i1 >= m_numVerts || i2 >= m_numVerts) continue;

        const auto& v0 = verts[i0];
        const auto& v1 = verts[i1];
        const auto& v2 = verts[i2];

        // If any vertex is flagged as floor or normal points mostly up
        if ((v0.flag & 1) || (v1.flag & 1) || (v2.flag & 1) || v0.ny > 0.65f) {
            FloorTri ft;
            ft.p0 = { v0.x, v0.y, v0.z };
            ft.p1 = { v1.x, v1.y, v1.z };
            ft.p2 = { v2.x, v2.y, v2.z };
            ft.minX = min3f(ft.p0.x, ft.p1.x, ft.p2.x) - 0.05f;
            ft.maxX = max3f(ft.p0.x, ft.p1.x, ft.p2.x) + 0.05f;
            ft.minZ = min3f(ft.p0.z, ft.p1.z, ft.p2.z) - 0.05f;
            ft.maxZ = max3f(ft.p0.z, ft.p1.z, ft.p2.z) + 0.05f;
            m_floorTris.push_back(ft);
        }
    }

    // OpenGL Buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VMapVertex), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    // layout(location = 0): vec3 pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VMapVertex), (const void*)0);

    // layout(location = 1): vec3 normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VMapVertex), (const void*)offsetof(VMapVertex, nx));

    // layout(location = 2): vec4 color
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VMapVertex), (const void*)offsetof(VMapVertex, r));

    glBindVertexArray(0);

    m_loaded = true;
    std::printf("[map_mesh] Loaded %s: %u verts, %u tris, %zu floor tris\n",
                binPath.c_str(), m_numVerts, m_numTris, m_floorTris.size());
    return true;
}

void MapMesh::render(unsigned /*shaderProg*/, unsigned uModelLoc, const Mat4& model) {
    if (!m_loaded || !m_vao) return;
    glUniformMatrix4fv(uModelLoc, 1, GL_FALSE, model.m);
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_numTris * 3), GL_UNSIGNED_INT, (const void*)0);
    glBindVertexArray(0);
}

static inline float sign2D(float x1, float z1, float x2, float z2, float x3, float z3) {
    return (x1 - x3) * (z2 - z3) - (x2 - x3) * (z1 - z3);
}

static inline bool pointInTri2D(float px, float pz, float x1, float z1, float x2, float z2, float x3, float z3) {
    float d1 = sign2D(px, pz, x1, z1, x2, z2);
    float d2 = sign2D(px, pz, x2, z2, x3, z3);
    float d3 = sign2D(px, pz, x3, z3, x1, z1);

    bool has_neg = (d1 < 0.0f) || (d2 < 0.0f) || (d3 < 0.0f);
    bool has_pos = (d1 > 0.0f) || (d2 > 0.0f) || (d3 > 0.0f);

    return !(has_neg && has_pos);
}

bool MapMesh::getFloorHeight(float x, float z, float currentY, float& outFloorY) const {
    if (m_floorTris.empty()) return false;

    float bestY = -999.0f;
    bool  found = false;
    float maxStepUp = 0.75f; // player can step up curbs/stairs up to 75cm

    for (const auto& ft : m_floorTris) {
        if (x < ft.minX || x > ft.maxX || z < ft.minZ || z > ft.maxZ)
            continue;

        if (pointInTri2D(x, z, ft.p0.x, ft.p0.z, ft.p1.x, ft.p1.z, ft.p2.x, ft.p2.z)) {
            // Plane equation from 3 points
            Vec3 e1 = ft.p1 - ft.p0;
            Vec3 e2 = ft.p2 - ft.p0;
            Vec3 n = cross(e1, e2);
            if (std::abs(n.y) > 1e-4f) {
                float triY = ft.p0.y - (n.x * (x - ft.p0.x) + n.z * (z - ft.p0.z)) / n.y;
                // We want the highest floor below current player foot level + step-up allowance
                if (triY <= currentY + maxStepUp && triY > bestY) {
                    bestY = triY;
                    found = true;
                }
            }
        }
    }

    if (found) {
        outFloorY = bestY;
        return true;
    }
    return false;
}
