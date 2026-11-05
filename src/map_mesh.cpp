#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include "map_mesh.h"
#include "gl_loader.h"
#include <cstdio>
#include <cstring>
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
    m_wallTris.clear();
    m_floorGrid.clear();
    m_wallGrid.clear();
    m_gridCols = m_gridRows = 0;
    m_loaded = false;
    m_numVerts = m_numTris = 0;
}

void MapMesh::worldToCell(float x, float z, int& gx, int& gz) const {
    gx = static_cast<int>((x - m_bMin.x) / kCellSize);
    gz = static_cast<int>((z - m_bMin.z) / kCellSize);
    if (gx < 0) gx = 0;
    if (gx >= m_gridCols) gx = m_gridCols - 1;
    if (gz < 0) gz = 0;
    if (gz >= m_gridRows) gz = m_gridRows - 1;
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

    // Initialize 2D spatial grid on XZ
    m_gridCols = std::max(1, static_cast<int>(std::ceil((m_bMax.x - m_bMin.x) / kCellSize)) + 1);
    m_gridRows = std::max(1, static_cast<int>(std::ceil((m_bMax.z - m_bMin.z) / kCellSize)) + 1);
    m_floorGrid.resize(static_cast<size_t>(m_gridCols * m_gridRows));
    m_wallGrid.resize(static_cast<size_t>(m_gridCols * m_gridRows));

    // Extract walkable floor triangles and blocking wall triangles using true geometric face normals
    if (std::memcmp(hdr.magic, "VMAP", 4) == 0) {
        m_floorTris.reserve(m_numTris / 3);
        m_wallTris.reserve(m_numTris / 3);

        for (uint32_t i = 0; i < m_numTris; ++i) {
            uint32_t i0 = indices[i * 3 + 0];
            uint32_t i1 = indices[i * 3 + 1];
            uint32_t i2 = indices[i * 3 + 2];
            if (i0 >= m_numVerts || i1 >= m_numVerts || i2 >= m_numVerts) continue;

            const auto& v0 = verts[i0];
            const auto& v1 = verts[i1];
            const auto& v2 = verts[i2];

            Vec3 p0 = { v0.x, v0.y, v0.z };
            Vec3 p1 = { v1.x, v1.y, v1.z };
            Vec3 p2 = { v2.x, v2.y, v2.z };

            Vec3 e1 = p1 - p0;
            Vec3 e2 = p2 - p0;
            Vec3 fn = cross(e1, e2);
            float nl = length(fn);
            if (nl < 1e-6f) continue;

            float absNy = std::abs(fn.y) / nl;

            // Check if horizontal / walkable floor (absNy > 0.60 handles both CCW and CW triangle windings!)
            if (absNy > 0.60f) {
                FloorTri ft;
                ft.p0 = p0;
                ft.p1 = p1;
                ft.p2 = p2;
                ft.n  = fn * (1.0f / nl);
                ft.minX = min3f(p0.x, p1.x, p2.x) - 0.06f;
                ft.maxX = max3f(p0.x, p1.x, p2.x) + 0.06f;
                ft.minY = min3f(p0.y, p1.y, p2.y);
                ft.maxY = max3f(p0.y, p1.y, p2.y);
                ft.minZ = min3f(p0.z, p1.z, p2.z) - 0.06f;
                ft.maxZ = max3f(p0.z, p1.z, p2.z) + 0.06f;

                uint32_t ftIdx = static_cast<uint32_t>(m_floorTris.size());
                m_floorTris.push_back(ft);

                int gx0, gz0, gx1, gz1;
                worldToCell(ft.minX, ft.minZ, gx0, gz0);
                worldToCell(ft.maxX, ft.maxZ, gx1, gz1);
                for (int gz = gz0; gz <= gz1; ++gz) {
                    for (int gx = gx0; gx <= gx1; ++gx) {
                        m_floorGrid[cellIndex(gx, gz)].push_back(ftIdx);
                    }
                }
            } else if (absNy < 0.38f) {
                // Vertical wall triangle
                WallTri wt;
                wt.p0 = p0;
                wt.p1 = p1;
                wt.p2 = p2;
                wt.minX = min3f(p0.x, p1.x, p2.x);
                wt.maxX = max3f(p0.x, p1.x, p2.x);
                wt.minY = min3f(p0.y, p1.y, p2.y);
                wt.maxY = max3f(p0.y, p1.y, p2.y);
                wt.minZ = min3f(p0.z, p1.z, p2.z);
                wt.maxZ = max3f(p0.z, p1.z, p2.z);

                uint32_t wtIdx = static_cast<uint32_t>(m_wallTris.size());
                m_wallTris.push_back(wt);

                int gx0, gz0, gx1, gz1;
                worldToCell(wt.minX - 0.4f, wt.minZ - 0.4f, gx0, gz0);
                worldToCell(wt.maxX + 0.4f, wt.maxZ + 0.4f, gx1, gz1);
                for (int gz = gz0; gz <= gz1; ++gz) {
                    for (int gx = gx0; gx <= gx1; ++gx) {
                        m_wallGrid[cellIndex(gx, gz)].push_back(wtIdx);
                    }
                }
            }
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
    std::printf("[map_mesh] Loaded %s: %u verts, %u tris, %zu floor tris, %zu wall tris\n",
                binPath.c_str(), m_numVerts, m_numTris, m_floorTris.size(), m_wallTris.size());
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

    bool has_neg = (d1 < -1e-4f) || (d2 < -1e-4f) || (d3 < -1e-4f);
    bool has_pos = (d1 >  1e-4f) || (d2 >  1e-4f) || (d3 >  1e-4f);

    return !(has_neg && has_pos);
}

bool MapMesh::queryFloorExact(float x, float z, float currentY, float maxStepUp, float maxDropDown, float& outFloorY) const {
    int gx, gz;
    worldToCell(x, z, gx, gz);
    const auto& bucket = m_floorGrid[cellIndex(gx, gz)];
    if (bucket.empty()) return false;

    float bestY = -999.0f;
    bool  found = false;

    for (uint32_t idx : bucket) {
        const auto& ft = m_floorTris[idx];
        if (x < ft.minX || x > ft.maxX || z < ft.minZ || z > ft.maxZ)
            continue;
        if (ft.minY > currentY + maxStepUp || ft.maxY < currentY - maxDropDown)
            continue;

        if (pointInTri2D(x, z, ft.p0.x, ft.p0.z, ft.p1.x, ft.p1.z, ft.p2.x, ft.p2.z)) {
            if (std::abs(ft.n.y) > 1e-4f) {
                float triY = ft.p0.y - (ft.n.x * (x - ft.p0.x) + ft.n.z * (z - ft.p0.z)) / ft.n.y;
                if (triY <= currentY + maxStepUp && triY >= currentY - maxDropDown && triY > bestY) {
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

bool MapMesh::getFloorHeight(float x, float z, float currentY, float& outFloorY) const {
    if (m_floorTris.empty()) return false;

    const float maxStepUp   = 0.85f; // step up stairs/curbs up to 85cm
    const float maxDropDown = 3.50f; // drop into sunken sites (e.g. Ascent B pit)

    if (queryFloorExact(x, z, currentY, maxStepUp, maxDropDown, outFloorY))
        return true;

    // Seam tolerance: probe 4 small offsets around player foot radius (18cm)
    const float kOffsets[4][2] = { {0.18f, 0.0f}, {-0.18f, 0.0f}, {0.0f, 0.18f}, {0.0f, -0.18f} };
    for (int i = 0; i < 4; ++i) {
        if (queryFloorExact(x + kOffsets[i][0], z + kOffsets[i][1], currentY, maxStepUp, maxDropDown, outFloorY))
            return true;
    }
    return false;
}

static inline void closestPointOnSegment2D(float px, float pz, float ax, float az, float bx, float bz, float& cx, float& cz) {
    float abx = bx - ax;
    float abz = bz - az;
    float len2 = abx * abx + abz * abz;
    if (len2 < 1e-6f) {
        cx = ax; cz = az;
        return;
    }
    float t = ((px - ax) * abx + (pz - az) * abz) / len2;
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    cx = ax + t * abx;
    cz = az + t * abz;
}

void MapMesh::resolveWallCollision(float& x, float& z, float footY, float radius) const {
    if (m_wallTris.empty()) return;

    int gx, gz;
    worldToCell(x, z, gx, gz);
    const auto& bucket = m_wallGrid[cellIndex(gx, gz)];
    if (bucket.empty()) return;

    // Check walls intersecting player torso band [footY + 0.55m, footY + 1.50m]
    const float torsoLo = footY + 0.55f;
    const float torsoHi = footY + 1.50f;
    const float r2 = radius * radius;

    for (uint32_t idx : bucket) {
        const auto& wt = m_wallTris[idx];
        if (wt.maxY < torsoLo || wt.minY > torsoHi) continue;
        if (x + radius < wt.minX || x - radius > wt.maxX || z + radius < wt.minZ || z - radius > wt.maxZ) continue;

        // Test the 3 triangle edges projected onto XZ
        const Vec3 pts[3] = { wt.p0, wt.p1, wt.p2 };
        for (int e = 0; e < 3; ++e) {
            const Vec3& a = pts[e];
            const Vec3& b = pts[(e + 1) % 3];
            float cx, cz;
            closestPointOnSegment2D(x, z, a.x, a.z, b.x, b.z, cx, cz);
            float dx = x - cx;
            float dz = z - cz;
            float d2 = dx * dx + dz * dz;
            if (d2 < r2 && d2 > 1e-8f) {
                float d = std::sqrt(d2);
                float push = (radius - d) / d;
                x += dx * push;
                z += dz * push;
            }
        }
    }
}

static inline bool rayIntersectsTri(Vec3 orig, Vec3 dir, float maxDist, Vec3 v0, Vec3 v1, Vec3 v2) {
    Vec3 e1 = v1 - v0;
    Vec3 e2 = v2 - v0;
    Vec3 h  = cross(dir, e2);
    float a = dot(e1, h);
    if (a > -1e-6f && a < 1e-6f) return false;

    float f = 1.0f / a;
    Vec3 s  = orig - v0;
    float u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) return false;

    Vec3 q  = cross(s, e1);
    float v = f * dot(dir, q);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * dot(e2, q);
    return (t > 0.15f && t < maxDist - 0.15f);
}

bool MapMesh::hasLineOfSight(Vec3 from, Vec3 to) const {
    if (!m_loaded || (m_wallTris.empty() && m_floorTris.empty())) return true;

    Vec3 diff = to - from;
    float dist = length(diff);
    if (dist < 0.3f) return true;
    Vec3 dir = diff * (1.0f / dist);

    float rayMinX = std::min(from.x, to.x) - 0.05f;
    float rayMaxX = std::max(from.x, to.x) + 0.05f;
    float rayMinY = std::min(from.y, to.y) - 0.05f;
    float rayMaxY = std::max(from.y, to.y) + 0.05f;
    float rayMinZ = std::min(from.z, to.z) - 0.05f;
    float rayMaxZ = std::max(from.z, to.z) + 0.05f;

    // Sample cells along the ray every 2.0m
    int steps = std::max(1, static_cast<int>(dist / 2.0f) + 1);
    int lastCell = -1;

    for (int s = 0; s <= steps; ++s) {
        float f = static_cast<float>(s) / static_cast<float>(steps);
        float px = from.x + diff.x * f;
        float pz = from.z + diff.z * f;
        int gx, gz;
        worldToCell(px, pz, gx, gz);
        int cIdx = cellIndex(gx, gz);
        if (cIdx == lastCell) continue;
        lastCell = cIdx;

        // Check wall triangles in this cell
        for (uint32_t wIdx : m_wallGrid[cIdx]) {
            const auto& wt = m_wallTris[wIdx];
            if (wt.maxY < rayMinY || wt.minY > rayMaxY) continue;
            if (wt.maxX < rayMinX || wt.minX > rayMaxX || wt.maxZ < rayMinZ || wt.minZ > rayMaxZ) continue;
            if (rayIntersectsTri(from, dir, dist, wt.p0, wt.p1, wt.p2))
                return false;
        }

        // Check floor/box-top triangles in this cell (e.g. elevated platforms blocking upward sightlines)
        for (uint32_t fIdx : m_floorGrid[cIdx]) {
            const auto& ft = m_floorTris[fIdx];
            if (ft.maxY < rayMinY || ft.minY > rayMaxY) continue;
            if (ft.maxX < rayMinX || ft.minX > rayMaxX || ft.maxZ < rayMinZ || ft.minZ > rayMaxZ) continue;
            if (rayIntersectsTri(from, dir, dist, ft.p0, ft.p1, ft.p2))
                return false;
        }
    }

    return true;
}
