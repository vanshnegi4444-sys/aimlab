#pragma once
#include "camera.h"
#include "map_mesh.h"
#include <string>
#include <vector>

enum class MapCourseId {
    AscentB_Clear,    // B Main to B Site Attack & Angle Slicing
    BindB_LongClear,  // B Long / Garden to B Site & Hookah
    HavenA_Clear,     // A Long to Heaven & Site Clear
    HavenB_Clear,     // Mid Courtyard to B Site & Links
    Count
};

struct PreAimBot {
    std::string name;
    Vec3   pos{};            // foot position
    float  yawDeg     = 0.0f; // facing direction
    float  headRadius = 0.15f;
    float  bodyRadius = 0.24f;
    float  height     = 1.78f;

    bool   alive          = true;
    bool   hasSeenPlayer  = false;
    double firstSeenTime  = 0.0;
    double reactionLimit  = 0.55; // seconds of continuous exposure before bot fires back
    bool   firedAtPlayer  = false;

    // Crosshair placement analytics on first sight
    float  firstSightVertErrDeg   = 0.0f; // negative = crosshair too low
    bool   hasFirstSightTelemetry = false;
};

struct PreAimResult {
    std::string botName;
    double killTimeMs = 0.0;
    bool   isHeadshot = false;
    bool   isStopped  = false;
    float  vertErrDeg = 0.0f;
    bool   playerTookDamage = false;
};

class PreAimCourse {
public:
    PreAimCourse();

    void start(MapCourseId courseId, double now);
    void snapToMapFloors(const MapMesh& map);
    void update(double now, double dt, const Vec3& playerEyePos, float playerPitchDeg, float playerYawDeg, const MapMesh* map = nullptr);

    // Returns: 2 = headshot, 1 = bodyshot, 0 = miss, -1 = moving whiff
    int shoot(double now, const Vec3& eyePos, const Vec3& fwdRay, bool isAccurate, PreAimResult& outResult, const MapMesh* map = nullptr);

    MapCourseId courseId() const { return m_courseId; }
    std::string courseName() const;
    std::string mapBinPath() const;
    Vec3        playerSpawnPos() const { return m_spawnPos; }
    float       playerSpawnYaw() const { return m_spawnYaw; }

    int  totalBots() const { return static_cast<int>(m_bots.size()); }
    int  botsRemaining() const;
    int  botsCleared() const { return totalBots() - botsRemaining(); }
    int  damageTakenCount() const { return m_damageTaken; }
    bool isComplete() const { return botsRemaining() == 0; }

    const std::vector<PreAimBot>& bots() const { return m_bots; }
    const std::vector<PreAimResult>& history() const { return m_history; }

    // Real-time HUD telemetry for active angle
    bool  hasActiveTarget() const { return m_activeBotIdx >= 0 && m_activeBotIdx < static_cast<int>(m_bots.size()); }
    float activeBotReactionProgress() const;
    float activeBotVertAngleError() const { return m_currentVertErr; }
    const std::string& activeBotName() const;

private:
    void initAscentB();
    void initBindB();
    void initHavenA();
    void initHavenB();

    MapCourseId m_courseId = MapCourseId::AscentB_Clear;
    Vec3        m_spawnPos{};
    float       m_spawnYaw = 0.0f;

    std::vector<PreAimBot>    m_bots;
    std::vector<PreAimResult> m_history;

    int    m_activeBotIdx   = -1;
    float  m_currentVertErr = 0.0f;
    int    m_damageTaken    = 0;
    double m_lastNow        = 0.0;
};
