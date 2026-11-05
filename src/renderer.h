#pragma once
#include "camera.h"

class Trainer;
class MapMesh;
class PreAimCourse;

class Renderer {
public:
    bool init();
    void shutdown();

    // 2D Range Wall & Targets
    void renderWall(const Mat4& viewProj, float wallDist);
    void renderTargets(const Mat4& viewProj, const Trainer& trainer, const Camera& cam);

    // 3D Valorant Maps & Humanoid Bots
    void renderMap(const Mat4& viewProj, MapMesh& map);
    void renderBot(const Mat4& viewProj, MapMesh& botMesh, Vec3 footPos, float yawDeg, bool isAlert, bool isTargeted);
    void renderPreAimBots(const Mat4& viewProj, MapMesh& botMesh, const PreAimCourse& course);

private:
    static unsigned compileShader(unsigned type, const char* src);
    static unsigned linkProgram(unsigned vs, unsigned fs);

    unsigned m_wallProg   = 0;
    unsigned m_targetProg = 0;
    unsigned m_meshProg   = 0;

    unsigned m_quadVao    = 0;
    unsigned m_quadVbo    = 0;

    // wall program uniforms
    int m_wViewProj = -1, m_wModel = -1;
    // target program uniforms
    int m_tViewProj = -1, m_tCenter = -1, m_tRadius = -1;
    int m_tRight = -1, m_tUp = -1, m_tColInner = -1, m_tColRing = -1;
    // 3d mesh program uniforms
    int m_mViewProj = -1, m_mModel = -1, m_mLightDir = -1;
    int m_mGlowCol  = -1, m_mGlowMix = -1;
};
