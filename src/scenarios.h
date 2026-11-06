#pragma once
#include <string>
#include <vector>

enum class ScenarioCategory {
    MapPreAim, // Real 3D Valorant Maps (Ascent, Bind, Haven)
    Clicking,
    Precision,
    Tracking,
    Movement,
    Reaction
};

enum class ScenarioId {
    AscentB_PreAim = 0,
    BindB_PreAim,
    HavenA_PreAim,
    Gridshot,
    GridshotLong,
    Sixshot,
    Spidershot,
    HeadshotLine,
    Microshot,
    FirstShotPlacement,
    SmoothStrafe,
    ValorantStrafe,
    CounterStrafeDrill,
    AngleHold,
    OffAngleStrafeConfirm,
    // Added 2026-11-05 (review round). Appended here, before Count, so
    // every existing id keeps its number (best.txt keys off the id).
    WideFlick180,
    ReactiveTrack,
    PeekersAdvantage,
    HavenB_PreAim,
    // Added 2026-11-06 (map-validation round). Same rule: append only.
    HavenC_PreAim,
    HavenC_Retake,
    HavenA_ElevLadder,
    // Added 2026-11-06 (sensible-tactical round). Same rule: append
    // only, before Count. Codes MAP-09..MAP-14 — MAP-08 stays reserved
    // for the spec's Pre-aim Ghost (B3 table), which is still pending.
    AscentB_Defuse,
    BindB_Defuse,
    HavenA_Defuse,
    AscentB_Clutch1v5,
    BindB_HeadHeight,
    AscentB_AnchorHold,
    Count
};

inline bool isMapPreAim(ScenarioId id) {
    return id == ScenarioId::AscentB_PreAim ||
           id == ScenarioId::BindB_PreAim ||
           id == ScenarioId::HavenA_PreAim ||
           id == ScenarioId::HavenB_PreAim ||
           id == ScenarioId::HavenC_PreAim ||
           id == ScenarioId::HavenC_Retake ||
           id == ScenarioId::HavenA_ElevLadder ||
           id == ScenarioId::AscentB_Defuse ||
           id == ScenarioId::BindB_Defuse ||
           id == ScenarioId::HavenA_Defuse ||
           id == ScenarioId::AscentB_Clutch1v5 ||
           id == ScenarioId::BindB_HeadHeight ||
           id == ScenarioId::AscentB_AnchorHold;
}

const char* getCategoryName(ScenarioCategory cat);

struct ScenarioInfo {
    ScenarioId id = ScenarioId::AscentB_PreAim;
    const char* code = "";
    const char* name = "";
    ScenarioCategory category = ScenarioCategory::MapPreAim;
    const char* categoryName = "";
    const char* description = "";
    const char* trackerFix = "";
    float defaultDuration = 60.0f;
    float defaultRadius = 0.15f;
    float defaultDist = 10.0f;
    int targetCount = 7;
    bool isTracking = false;
    bool allowPlayerMove = true;

    ScenarioInfo() = default;
    ScenarioInfo(ScenarioId id_, const char* code_, const char* name_, ScenarioCategory cat_,
                 const char* desc_, const char* fix_,
                 float dur_, float rad_, float dist_, int count_, bool track_, bool move_)
        : id(id_), code(code_), name(name_), category(cat_), categoryName(getCategoryName(cat_)),
          description(desc_), trackerFix(fix_), defaultDuration(dur_), defaultRadius(rad_),
          defaultDist(dist_), targetCount(count_), isTracking(track_), allowPlayerMove(move_) {}
};

const ScenarioInfo& getScenarioInfo(ScenarioId id);
const std::vector<ScenarioInfo>& getAllScenarios();
