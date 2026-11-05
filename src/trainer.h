#pragma once
#include "settings.h"
#include "camera.h"
#include "scenarios.h"
#include <random>
#include <vector>
#include <string>

struct Target {
    Vec3   center{};           // world position
    Vec3   velocity{};         // world units per second
    float  radius   = 0.15f;   // radius in metres
    double spawnTime = 0.0;    // seconds (glfwGetTime())
    bool   alive    = false;
    int    slot     = -1;      // for gridshot (0..8)
    int    hitRegion = 0;      // 0 = head, 1 = body
    float  colorInner[3] = { 0.25f, 0.90f, 1.00f };
    float  colorRing[3]  = { 0.05f, 0.30f, 0.55f };

    // Movement / strafe params
    float  minX = -3.0f, maxX = 3.0f;
    float  baseY = 0.0f;

    // Angle hold / peeking
    bool   isPeeking = false;
    double peekTime = 0.0;
    double peekDuration = 0.40; // window to kill before bot fires back
};

struct ShotRecord {
    double time;
    bool   hit;
    bool   isHeadshot;
    bool   isStopped;
    bool   isOvershoot;
    float  errX; // angular error
    float  errY;
};

class Trainer {
public:
    static constexpr int kMaxTargets = 16;
    static constexpr int kGridN      = 3;
    static constexpr float kSlotSpacing = 1.6f;

    Trainer();

    // Start or restart a round
    void start(const Settings& s, double now, ScenarioId scenarioId);

    // Frame update (moves targets, handles tracking, updates timers)
    void update(double now, double dt, const Camera& cam, bool movingLeft, bool movingRight);

    // Fire one shot
    // Returns: 1 = hit, 2 = headshot, 0 = miss, -1 = shot spread whiff (due to moving)
    int shoot(double now, const Camera& cam);

    // ---- Scenario & state queries ----
    ScenarioId   scenarioId()    const { return m_scenarioId; }
    const ScenarioInfo& info()   const { return getScenarioInfo(m_scenarioId); }
    double       timeLeft()      const { return m_timeLeft; }
    bool         isComplete()    const { return m_timeLeft <= 0.0; }

    // ---- Core stats ----
    int          score()         const { return m_score; }
    int          shots()         const { return m_shots; }
    int          hits()          const { return m_hits; }
    int          headshots()     const { return m_headshots; }
    float        accuracy()      const;
    double       avgTtkMs()      const;
    int          bestStreak()    const { return m_bestStreak; }
    int          streak()        const { return m_streak; }

    // ---- Movement & Counter-Strafe stats ----
    float        playerSpeed()   const { return std::abs(m_playerVelX); }
    bool         isPlayerStopped() const { return std::abs(m_playerVelX) < 1.85f; } // <27.5% of 6.75 m/s
    float        stopAccuracy()  const; // % of shots fired while fully stopped

    // ---- Tracking stats ----
    bool         isCurrentlyTracking() const { return m_isTrackingNow; }
    float        trackingAccuracy()    const;

    // ---- Detailed Analytics ----
    float        overshootPct()  const;
    float        undershootPct() const;
    double       avgReactionMs() const;
    std::string  generateDiagnosticAdvice() const;

    // ---- Target rendering access ----
    int          targetCount()   const { return m_activeTargetCount; }
    const Target& target(int i)  const { return m_targets[i]; }
    float        targetRadius()  const { return m_radius; }

    // Angle hold obstacle wall X position (for drawing cover wall)
    float        coverWallX()    const { return m_coverWallX; }
    bool         hasCoverWall()  const { return m_scenarioId == ScenarioId::AngleHold; }

private:
    void initGridshot(double now);
    void initSixshot(double now);
    void initSpidershot(double now);
    void initHeadshotLine(double now);
    void initMicroshot(double now);
    void initFirstShotPlacement(double now);
    void initSmoothStrafe(double now);
    void initValorantStrafe(double now);
    void initCounterStrafeDrill(double now);
    void initAngleHold(double now);
    void initOffAngleStrafeConfirm(double now);

    Vec3 slotPos(int slot) const;
    int  randomFreeSlot();
    void spawnRandomTarget(int idx, double now);

    // Settings snapshot
    ScenarioId m_scenarioId = ScenarioId::Gridshot;
    float  m_wallDist   = 10.0f;
    float  m_radius     = 0.15f;
    double m_duration   = 60.0;

    // Round timing
    double m_start      = 0.0;
    double m_timeLeft   = 0.0;

    // Targets
    int    m_activeTargetCount = 3;
    Target m_targets[kMaxTargets]{};

    // Player movement simulation (for M-01)
    float  m_playerPosX   = 0.0f;
    float  m_playerVelX   = 0.0f; // m/s
    int    m_stoppedShots = 0;
    int    m_movingShots  = 0;

    // Spidershot tracking
    bool   m_spidershotCenterTurn = false;

    // Angle hold state
    float  m_coverWallX = -1.2f;
    double m_nextPeekTime = 0.0;
    bool   m_botIsHidden  = true;

    // Tracking state
    double m_trackingOnTargetTime = 0.0;
    double m_totalTrackingTime    = 0.0;
    bool   m_isTrackingNow        = false;

    // Stats
    int    m_score      = 0;
    int    m_shots      = 0;
    int    m_hits       = 0;
    int    m_headshots  = 0;
    int    m_streak     = 0;
    int    m_bestStreak = 0;
    double m_ttkSum     = 0.0;
    int    m_ttkCount   = 0;

    // Analytics counters
    int    m_overshootCount  = 0;
    int    m_undershootCount = 0;
    double m_reactionSum     = 0.0;
    int    m_reactionCount   = 0;
    float  m_lastCrosshairYaw = 0.0f;
    float  m_lastCrosshairPitch = 0.0f;

    std::mt19937 m_rng{ std::random_device{}() };
};
