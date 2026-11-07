#pragma once
#include "camera.h"
#include <cstdint>
#include <string>
#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct VMapHeader {
    char     magic[4];     // 'VMAP' or 'VBOT'
    uint32_t version;      // 1
    uint32_t numVerts;
    uint32_t numTris;
    float    minX, maxX;
    float    minY, maxY;
    float    minZ, maxZ;
};

struct VMapVertex {
    float   x, y, z;
    float   nx, ny, nz;
    uint8_t r, g, b, a;
    uint8_t flag;          // 1 = floor, 2 = head
    uint8_t pad[3];
};
#pragma pack(pop)

struct FloorTri {
    Vec3  p0, p1, p2;
    Vec3  n;
    float minX, maxX, minY, maxY, minZ, maxZ;
};

struct WallTri {
    Vec3  p0, p1, p2;
    float minX, maxX, minY, maxY, minZ, maxZ;
};

class MapMesh {
public:
    MapMesh();
    ~MapMesh();

    bool load(const std::string& binPath);
    void destroy();

    void render(unsigned shaderProg, unsigned uModelLoc, const Mat4& model);

    // Query floor height at (x, z) close to currentY. Returns true if floor found.
    bool getFloorHeight(float x, float z, float currentY, float& outFloorY) const;

    // Push player cylinder (x, z) out of vertical wall geometry between [footY + 0.55m, footY + 1.55m]
    void resolveWallCollision(float& x, float& z, float footY, float radius = 0.35f) const;

    // Returns true if unobstructed line-of-sight exists between `from` and `to`
    bool hasLineOfSight(Vec3 from, Vec3 to) const;

    bool isLoaded() const { return m_loaded; }
    uint32_t numTris() const { return m_numTris; }
    uint32_t numVerts() const { return m_numVerts; }

    Vec3 boundsMin() const { return m_bMin; }
    Vec3 boundsMax() const { return m_bMax; }

private:
    bool queryFloorExact(float x, float z, float currentY, float maxStepUp, float maxDropDown, float& outFloorY) const;
    int  cellIndex(int gx, int gz) const { return gz * m_gridCols + gx; }
    void worldToCell(float x, float z, int& gx, int& gz) const;

    bool     m_loaded = false;
    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_ebo = 0;
    uint32_t m_numVerts = 0;
    uint32_t m_numTris = 0;
    Vec3     m_bMin{};
    Vec3     m_bMax{};

    std::vector<FloorTri> m_floorTris;
    std::vector<WallTri>  m_wallTris;

    // 2D Uniform Spatial Grid on XZ for O(1) floor, wall, and raycast queries
    static constexpr float kCellSize = 4.0f;
    int m_gridCols = 0;
    int m_gridRows = 0;
    std::vector<std::vector<uint32_t>> m_floorGrid;
    std::vector<std::vector<uint32_t>> m_wallGrid;
};
