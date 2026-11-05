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
    Count
};

inline bool isMapPreAim(ScenarioId id) {
    return id == ScenarioId::AscentB_PreAim ||
           id == ScenarioId::BindB_PreAim ||
           id == ScenarioId::HavenA_PreAim;
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
