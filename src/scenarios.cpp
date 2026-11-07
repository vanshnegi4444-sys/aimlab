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
        "Push A Long onto Haven A Site. Clear close corner, site default stack, Heaven elevated sniper window, Hell under-window, and Short flank.",
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
    },
    ScenarioInfo{
        ScenarioId::WideFlick180,
        "C-05",
        "Wide Flick 180",
        ScenarioCategory::Clicking,
        "Single target at a wide angle (45-160 deg to either side, 10 m). Trains big arm flicks and fast 180-degree turns.",
        "Builds the large flicks you need when an enemy appears far off your crosshair.",
        30.0f, 0.20f, 10.0f, 1, false, false
    },
    ScenarioInfo{
        ScenarioId::ReactiveTrack,
        "T-05",
        "Reactive Track",
        ScenarioCategory::Tracking,
        "Target changes direction randomly every 0.15-0.5 s. Score is time on target. Trains reacting to sudden direction changes instead of predicting a smooth path.",
        "Kills tracking lag: watch the target's center, not your crosshair.",
        45.0f, 0.13f, 10.0f, 1, true, false
    },
    ScenarioInfo{
        ScenarioId::PeekersAdvantage,
        "R-03",
        "Peeker's Advantage",
        ScenarioCategory::Reaction,
        "Swing out with A (left) or D (right). The bot appears at the angle and fires back 200 ms after you stop moving. Stop (counter-strafe), shoot, then swing back with the opposite key to hide. If you are too slow, the bot shoots first and the rep resets.",
        "Trains the exact duel timing you lose in ranked: swing, stop, first bullet before the enemy reacts.",
        45.0f, 0.14f, 12.0f, 1, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenB_PreAim,
        "MAP-04",
        "Haven B: Corridor to Site Entry & Clear",
        ScenarioCategory::MapPreAim,
        "Push the B corridor up the ramp onto Haven B Site. Clear the ramp-top duel, site corners, the default box and the back site. Real 3D geometry & Jett humanoid bots.",
        "Pre-aim practice for B site: same crosshair-height discipline as Haven A, on your second-worst map.",
        60.0f, 0.14f, 15.0f, 5, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenC_PreAim,
        "MAP-05",
        "Haven C: Long to Site Clear",
        ScenarioCategory::MapPreAim,
        "Enter Haven C site from its north edge and clear it in order: entry right, both site corners, the default stack, then deep south-west. All bot floors verified against havenA.bin (site floor y ~ 4.3).",
        "Site-entry pre-aim on a third Haven site: same corner discipline as MAP-03/04, new angles.",
        60.0f, 0.14f, 15.0f, 5, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenC_Retake,
        "MAP-06",
        "Haven C: Site Retake",
        ScenarioCategory::MapPreAim,
        "Spawn deep on Haven C site and retake it: clear the main entry, the west corner, the right entry, then the long approach. The reverse of MAP-05 — defending angles instead of attacking ones.",
        "Retake pre-aim: holding entry angles from inside the site instead of clearing them from outside.",
        60.0f, 0.14f, 15.0f, 4, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenA_ElevLadder,
        "MAP-07",
        "Haven A: Elevation Ladder (Vertical Flick Drill)",
        ScenarioCategory::MapPreAim,
        "A vertical-flick drill on Haven A, not a tactical clear: four bots, one per verified level — Sewers 1.9 m, A Long 4.2 m, site 7.7 m, Heaven 10.4 m — ordered low to high so each rep is a bigger upward flick.",
        "Trains pure vertical crosshair placement across an 8.4 m height range on real map geometry.",
        60.0f, 0.14f, 15.0f, 4, false, true
    },
    ScenarioInfo{
        ScenarioId::AscentB_Defuse,
        "MAP-09",
        "Ascent B: Post-Plant Defuse (Retake from CT, 45 s)",
        ScenarioCategory::MapPreAim,
        "The spike is planted at Default. Retake from the CT corridor against four post-plant holders (Default, Market Edge, Lane, Boathouse) before the 45 s spike timer runs out. Holders face CT and react in 0.38-0.45 s.",
        "Retake pre-aim under a real spike timer: clear in order, do not waste time on the flank.",
        45.0f, 0.14f, 15.0f, 4, false, true
    },
    ScenarioInfo{
        ScenarioId::BindB_Defuse,
        "MAP-10",
        "Bind B: Post-Plant Defuse (Retake from Hall, 45 s)",
        ScenarioCategory::MapPreAim,
        "The spike is planted on B site. Retake from B Hall against five post-plant holders (Sand, Center Box, Elbow, Garden, Hookah) before the 45 s spike timer runs out. All bots face Hall.",
        "Retake pre-aim under a real spike timer on Bind's tight site.",
        45.0f, 0.14f, 15.0f, 5, false, true
    },
    ScenarioInfo{
        ScenarioId::HavenA_Defuse,
        "MAP-11",
        "Haven A: Post-Plant Defuse (Retake from Heaven, 45 s)",
        ScenarioCategory::MapPreAim,
        "The spike is planted at Default. Retake from Heaven — the classic A retake perch — against four post-plant holders (Default, Hell, Long, Short) before the 45 s spike timer runs out. You shoot down from 10.35 m at heads from 1.91 m to 7.73 m.",
        "Retake pre-aim from height: a vertical-flick drill inside a timed retake.",
        45.0f, 0.14f, 15.0f, 4, false, true
    },
    ScenarioInfo{
        ScenarioId::AscentB_Clutch1v5,
        "MAP-12",
        "Ascent B: 1v5 Clutch (3 hits = round lost)",
        ScenarioCategory::MapPreAim,
        "Last alive on Ascent B: enter from B Main against five bots on the site's real anchor spots. Reactions are staggered by angle distance (0.55-1.25 s) and a bot only fires while it is in your sight cone, so isolate 1v1s and never face two angles at once. 3 hits = round lost. No timer.",
        "Clutch discipline: angle isolation and patience, not flick speed.",
        60.0f, 0.14f, 15.0f, 5, false, true
    },
    ScenarioInfo{
        ScenarioId::BindB_HeadHeight,
        "MAP-13",
        "Bind B: Head-Height Lane (Headshots Only)",
        ScenarioCategory::MapPreAim,
        "Crosshair-placement drill down B Long: the same six anchor spots as MAP-02, but body shots do not kill — only headshots end the round. Head height drops ~2.3 m as the lane falls from Garden (5.33 m) to the site (3.0 m). Wasted body shots are counted on the results screen.",
        "Head-level discipline across changing ground height: the score is headshot rate + vertical error.",
        60.0f, 0.14f, 15.0f, 6, false, true
    },
    ScenarioInfo{
        ScenarioId::AscentB_AnchorHold,
        "MAP-14",
        "Ascent B: Boathouse Anchor Hold",
        ScenarioCategory::MapPreAim,
        "Corner-holding from the defender's side: you ARE the Boathouse anchor and five attackers push in from B Main, peeking in order (reactions staggered 0.50-1.05 s). Hold one angle and tap each peek as it comes. No timer, no damage limit.",
        "Defensive angle holding: patience and peek timing instead of entry clearing.",
        60.0f, 0.14f, 15.0f, 5, false, true
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
