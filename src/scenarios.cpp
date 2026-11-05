#include "scenarios.h"

const char* getCategoryName(ScenarioCategory cat) {
    switch (cat) {
        case ScenarioCategory::MapPreAim: return "3D Map Pre-Aim";
        case ScenarioCategory::Clicking:  return "Clicking";
        case ScenarioCategory::Precision: return "Precision";
        case ScenarioCategory::Tracking:  return "Tracking";
        case ScenarioCategory::Movement:  return "Movement";
        case ScenarioCategory::Reaction:  return "Reaction";
    }
    return "Unknown";
}

static const std::vector<ScenarioInfo> kScenarios = {
    ScenarioInfo{
        ScenarioId::AscentB_PreAim,
        "MAP-01",
        "Ascent B: Main to Site Clearing",
        ScenarioCategory::MapPreAim,
        "Walk through Ascent B Main, slice angles around the archway, clear Lane, Green Box (elevated head level), Market door, CT Arch, and Boathouse. Real 3D geometry & Jett humanoid bots.",
        "Fixes 71.5% body hits & negative DDΔ: teaches real corner slicing and elevation adjustment for boxes/stairs.",
        60.0f, 0.14f, 15.0f, 7, false, true
    },
    ScenarioInfo{
        ScenarioId::BindB_PreAim,
        "MAP-02",
        "Bind B: Long & Hookah Clearing",
        ScenarioCategory::MapPreAim,
        "Advance down Bind B Long into Site. Clear Garden corner, Hookah elevated window, Sand Container, Elbow, and B Hall / Hell hold. Full 3D map collision.",
        "Teaches pre-aiming the extreme vertical elevation difference of Hookah window and holding tight angles.",
        60.0f, 0.14f, 15.0f, 6, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenA_PreAim,
        "MAP-03",
        "Haven A: Long & Site Angle Slicing",
        ScenarioCategory::MapPreAim,
        "Push A Long onto Haven A Site. Clear close corner, site stack, Heaven elevated sniper window, Hell under-window, and Short flank.",
        "Trains crosshair elevation flick from Hell (ground) to Heaven (elevated sniper window).",
        60.0f, 0.14f, 15.0f, 5, false, true
    },
    ScenarioInfo{
        ScenarioId::Gridshot,
        "C-01",
        "Gridshot",
        ScenarioCategory::Clicking,
        "Classic 3x3 grid on a 10m wall with 3 active targets. Build speed and basic flick rhythm.",
        "Warmup hands & build consistent flick tempo before ranked.",
        60.0f, 0.15f, 10.0f, 3, false, false
    },
    ScenarioInfo{
        ScenarioId::GridshotLong,
        "C-02",
        "Gridshot Long Range",
        ScenarioCategory::Clicking,
        "3x3 grid at 25m distance with smaller targets. Trains small-angle precision at rifle range.",
        "Forces tight visual confirmation at long distance.",
        60.0f, 0.12f, 25.0f, 3, false, false
    },
    ScenarioInfo{
        ScenarioId::Sixshot,
        "C-03",
        "Sixshot Wide Range",
        ScenarioCategory::Clicking,
        "6 simultaneous targets randomly distributed across a wide angle (yaw +-20 deg, pitch +-10 deg).",
        "Trains multi-target path planning and wide resets.",
        60.0f, 0.12f, 10.0f, 6, false, false
    },
    ScenarioInfo{
        ScenarioId::Spidershot,
        "C-04",
        "Spidershot",
        ScenarioCategory::Clicking,
        "Target alternates between center (0,0) and a random peripheral angle. Tests outward snap and return snap.",
        "Trains crosshair reset back to default angle.",
        60.0f, 0.15f, 10.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::HeadshotLine,
        "C-06",
        "Headshot Line",
        ScenarioCategory::Precision,
        "Targets spawn strictly at fixed head height (pitch = 0 deg) across horizontal angles (yaw +-40 deg).",
        "Directly eliminates your 71.5% body hit rate by locking crosshair to head level.",
        60.0f, 0.12f, 12.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::Microshot,
        "C-07",
        "Microshot",
        ScenarioCategory::Precision,
        "Tiny targets (r=0.06m) spawn close (within 2.5 deg) of the last shot. Punishes overshoot and spam.",
        "Refines micro-adjustments and finger/wrist fine control.",
        60.0f, 0.06f, 10.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::FirstShotPlacement,
        "P-02",
        "First-Shot Placement",
        ScenarioCategory::Precision,
        "Single target appears 1-4 deg off-crosshair. You have ONE bullet only! Miss = penalty reset.",
        "Crucial for Vandal first-bullet trades: eliminates firing before aiming is confirmed.",
        60.0f, 0.12f, 12.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::SmoothStrafe,
        "T-01",
        "Smooth Strafe Tracking",
        ScenarioCategory::Tracking,
        "Target smoothly strafes side-to-side at 4.0 m/s with natural deceleration. Score is time on target.",
        "Smooths out jittery mouse movements when tracking moving enemies.",
        45.0f, 0.15f, 10.0f, 1, true, false
    },
    ScenarioInfo{
        ScenarioId::ValorantStrafe,
        "T-02",
        "Valorant AD Strafe Tracking",
        ScenarioCategory::Tracking,
        "Target strafes at Valorant run speed (6.75 m/s) with rapid 0.2-0.5s counter-strafe direction switches.",
        "Prepares you for AD-spamming duelists at close/mid range.",
        45.0f, 0.13f, 10.0f, 1, true, false
    },
    ScenarioInfo{
        ScenarioId::CounterStrafeDrill,
        "M-01",
        "Counter-Strafe Stop & Shoot",
        ScenarioCategory::Movement,
        "You strafe left/right with A/D. A target pops up — you must tap the opposite key to reach 0 velocity before firing!",
        "Stops shooting while moving: enforces deadzone stopping muscle memory.",
        45.0f, 0.15f, 10.0f, 1, false, true
    },
    ScenarioInfo{
        ScenarioId::AngleHold,
        "R-01",
        "Angle Hold & Reaction",
        ScenarioCategory::Reaction,
        "Hold your crosshair on a corner. A bot wide-swings or jiggle-peeks. Tap within the reaction window to kill.",
        "Improves first-bullet reaction time on defensive angle holds.",
        45.0f, 0.14f, 12.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::OffAngleStrafeConfirm,
        "P-03",
        "Off-Angle Strafe Confirm",
        ScenarioCategory::Precision,
        "Target spawns 2-3 body widths off your crosshair (exact distance from your Range screenshot) and performs rapid AD micro-strafes. Teaches you to wait for the strafe apex or use A/D movement-aim rather than panic-flicking.",
        "Fixes your exact miss: eliminates shooting where the enemy WAS instead of confirming where they STOP.",
        45.0f, 0.12f, 12.0f, 1, false, true
    }
};

const ScenarioInfo& getScenarioInfo(ScenarioId id) {
    size_t idx = static_cast<size_t>(id);
    if (idx < kScenarios.size()) return kScenarios[idx];
    return kScenarios[0];
}

const std::vector<ScenarioInfo>& getAllScenarios() {
    return kScenarios;
}
