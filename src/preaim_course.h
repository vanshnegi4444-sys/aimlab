#pragma once
#include "camera.h"
#include "map_mesh.h"
#include <string>
#include <vector>

enum class MapCourseId {
    AscentB_Clear,    // B Main to B Site Attack & Angle Slicing
    BindB_LongClear,  // B Long / Garden to B Site & Hookah
    HavenA_Clear,     // A Long to Heaven & Site Clear
    HavenB_Clear,     // B Corridor to B Site Entry & Clear
    // Added 2026-11-06 (map-validation round). All positions verified
    // against the .bin floor data (see README "Map validation").
    HavenC_Clear,     // C site entry: north edge into the 4.3 m site floor
    HavenC_Retake,    // C site retake: spawn deep, clear the entries
    HavenA_ElevLadder,// Vertical-flick drill: 1.9 -> 4.2 -> 7.7 -> 10.4 m
    // Added 2026-11-06 (sensible-tactical round). Bots now hold real
    // angles (see README "Tactical grounding"); new scenario types:
    // post-plant defuse (45 s spike timer), 1v5 clutch (3-hit limit),
    // headshots-only crosshair drill, defensive anchor hold.
    AscentB_Defuse,   // Retake B from CT vs post-plant holders, 45 s
    BindB_Defuse,     // Retake B from Hall vs post-plant holders, 45 s
    HavenA_Defuse,    // Retake A from Heaven vs post-plant holders, 45 s
    AscentB_Clutch1v5,// 1v5 clutch: 5 bots, staggered reactions, 3 hits = dead
    BindB_HeadHeight, // Crosshair drill: B Long, headshots only
    AscentB_AnchorHold,// Hold B site from Boathouse vs B Main push
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

    // Fail states (2026-11-06 tactical round). The engine had no way to
    // lose a map course before this: a defuse course fails when the spike
    // timer runs out, a clutch course fails when the player takes
    // maxDamage hits. update() sets m_failReason; main.cpp ends the
    // course when isFailed() is true.
    enum class FailReason { None, TimeUp, DamageLimit };
    bool isFailed() const { return m_failReason != FailReason::None; }
    bool failedByTimer() const { return m_failReason == FailReason::TimeUp; }
    bool failedByDamage() const { return m_failReason == FailReason::DamageLimit; }
    bool hasTimeLimit() const { return m_timeLimitSec > 0.0; }
    double timeLimitSec() const { return m_timeLimitSec; }
    // Seconds left on the spike timer (0 once expired, -1 if no timer).
    double timeRemaining(double now) const {
        if (m_timeLimitSec <= 0.0) return -1.0;
        double r = m_timeLimitSec - (now - m_startTime);
        return r > 0.0 ? r : 0.0;
    }
    int  maxDamage() const { return m_maxDamage; }
    bool isHeadshotsOnly() const { return m_headshotsOnly; }
    int  bodyHitsWasted() const { return m_bodyHitsWasted; }

    const std::vector<PreAimBot>& bots() const { return m_bots; }
    const std::vector<PreAimResult>& history() const { return m_history; }

    // Real-time HUD telemetry for active angle
    bool  hasActiveTarget() const { return m_activeBotIdx >= 0 && m_activeBotIdx < static_cast<int>(m_bots.size()); }
    float activeBotReactionProgress(double now = -1.0) const;
    float activeBotVertAngleError() const { return m_currentVertErr; }
    const std::string& activeBotName() const;

private:
    void initAscentB();
    void initBindB();
    void initHavenA();
    void initHavenB();
    void initHavenC();
    void initHavenCRetake();
    void initHavenAElevLadder();
    void initAscentBDefuse();
    void initBindBDefuse();
    void initHavenADefuse();
    void initAscentBClutch();
    void initBindBHeadHeight();
    void initAscentBAnchorHold();

    MapCourseId m_courseId = MapCourseId::AscentB_Clear;
    Vec3        m_spawnPos{};
    float       m_spawnYaw = 0.0f;

    std::vector<PreAimBot>    m_bots;
    std::vector<PreAimResult> m_history;

    int    m_activeBotIdx   = -1;
    float  m_currentVertErr = 0.0f;
    int    m_damageTaken    = 0;
    double m_lastNow        = 0.0;

    // Fail-state / drill-rule state (2026-11-06 tactical round).
    double     m_startTime = 0.0;     // glfw time at start()
    double     m_timeLimitSec = 0.0;  // 0 = no timer (spike = 45 s)
    int        m_maxDamage = 0;       // 0 = no limit (clutch = 3 hits)
    bool       m_headshotsOnly = false;
    int        m_bodyHitsWasted = 0;
    FailReason m_failReason = FailReason::None;
};
