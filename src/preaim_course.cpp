#include "preaim_course.h"
#include <cmath>
#include <algorithm>

PreAimCourse::PreAimCourse() {
    start(MapCourseId::AscentB_Clear, 0.0);
}

std::string PreAimCourse::courseName() const {
    switch (m_courseId) {
        case MapCourseId::AscentB_Clear:   return "Ascent B: Main to Site Clearing";
        case MapCourseId::BindB_LongClear: return "Bind B: Long & Hookah Clearing";
        case MapCourseId::HavenA_Clear:    return "Haven A: Long & Site Angle Slicing";
        case MapCourseId::HavenB_Clear:    return "Haven B: Corridor to Site Entry & Clear";
        case MapCourseId::HavenC_Clear:    return "Haven C: Long to Site Clear";
        case MapCourseId::HavenC_Retake:   return "Haven C: Site Retake";
        case MapCourseId::HavenA_ElevLadder: return "Haven A: Elevation Ladder (Vertical Flick Drill)";
        case MapCourseId::AscentB_Defuse:  return "Ascent B: Post-Plant Defuse (Retake from CT, 45 s)";
        case MapCourseId::BindB_Defuse:    return "Bind B: Post-Plant Defuse (Retake from Hall, 45 s)";
        case MapCourseId::HavenA_Defuse:   return "Haven A: Post-Plant Defuse (Retake from Heaven, 45 s)";
        case MapCourseId::AscentB_Clutch1v5: return "Ascent B: 1v5 Clutch (3 hits = round lost)";
        case MapCourseId::BindB_HeadHeight: return "Bind B: Head-Height Lane (Headshots Only)";
        case MapCourseId::AscentB_AnchorHold: return "Ascent B: Boathouse Anchor Hold";
        default: return "Custom Map Course";
    }
}

std::string PreAimCourse::mapBinPath() const {
    switch (m_courseId) {
        case MapCourseId::AscentB_Clear:   return "assets/maps/ascentB.bin";
        case MapCourseId::BindB_LongClear: return "assets/maps/bindB.bin";
        case MapCourseId::HavenA_Clear:    return "assets/maps/havenA.bin";
        case MapCourseId::HavenB_Clear:    return "assets/maps/havenB.bin";
        case MapCourseId::HavenC_Clear:    return "assets/maps/havenA.bin";
        case MapCourseId::HavenC_Retake:   return "assets/maps/havenA.bin";
        case MapCourseId::HavenA_ElevLadder: return "assets/maps/havenA.bin";
        case MapCourseId::AscentB_Defuse:  return "assets/maps/ascentB.bin";
        case MapCourseId::BindB_Defuse:    return "assets/maps/bindB.bin";
        case MapCourseId::HavenA_Defuse:   return "assets/maps/havenA.bin";
        case MapCourseId::AscentB_Clutch1v5: return "assets/maps/ascentB.bin";
        case MapCourseId::BindB_HeadHeight: return "assets/maps/bindB.bin";
        case MapCourseId::AscentB_AnchorHold: return "assets/maps/ascentB.bin";
        default: return "assets/maps/ascentB.bin";
    }
}

void PreAimCourse::start(MapCourseId courseId, double now) {
    m_courseId = courseId;
    m_bots.clear();
    m_history.clear();
    m_activeBotIdx   = -1;
    m_currentVertErr = 0.0f;
    m_damageTaken    = 0;
    m_lastNow        = now;
    // Fail-state / drill-rule state resets every run. start() used to
    // ignore `now`; the spike timer needs it (2026-11-06 tactical round).
    m_startTime = now;
    m_timeLimitSec = 0.0;
    m_maxDamage = 0;
    m_headshotsOnly = false;
    m_bodyHitsWasted = 0;
    m_failReason = FailReason::None;

    switch (courseId) {
        case MapCourseId::AscentB_Clear:   initAscentB(); break;
        case MapCourseId::BindB_LongClear: initBindB(); break;
        case MapCourseId::HavenA_Clear:    initHavenA(); break;
        case MapCourseId::HavenB_Clear:    initHavenB(); break;
        case MapCourseId::HavenC_Clear:    initHavenC(); break;
        case MapCourseId::HavenC_Retake:   initHavenCRetake(); break;
        case MapCourseId::HavenA_ElevLadder: initHavenAElevLadder(); break;
        case MapCourseId::AscentB_Defuse:  initAscentBDefuse(); break;
        case MapCourseId::BindB_Defuse:    initBindBDefuse(); break;
        case MapCourseId::HavenA_Defuse:   initHavenADefuse(); break;
        case MapCourseId::AscentB_Clutch1v5: initAscentBClutch(); break;
        case MapCourseId::BindB_HeadHeight: initBindBHeadHeight(); break;
        case MapCourseId::AscentB_AnchorHold: initAscentBAnchorHold(); break;
        default: initAscentB(); break;
    }
}

void PreAimCourse::snapToMapFloors(const MapMesh& map) {
    if (!map.isLoaded()) return;
    float fy = m_spawnPos.y;
    if (map.getFloorHeight(m_spawnPos.x, m_spawnPos.z, m_spawnPos.y, fy)) {
        m_spawnPos.y = fy;
    }
    for (auto& b : m_bots) {
        float bfy = b.pos.y;
        if (map.getFloorHeight(b.pos.x, b.pos.z, b.pos.y, bfy)) {
            b.pos.y = bfy;
        }
    }
}

void PreAimCourse::initAscentB() {
    // TACTICAL REDESIGN 2026-11-06 (sensible-tactical round): the old
    // bots stood at roughly the right spots but FACED AWAY from the
    // attacker (yaws 30-90 deg face into the site, not the entry) — a
    // defender holding Lane faces B Main, not CT. Every yaw below is
    // atan2(dx, -dz) toward the B Main spawn (-23, -50). The old
    // "Market Door Peeker" at (-23, -66.5) was mid-site, mislabelled:
    // a floor probe of ascentB.bin shows the real Market-side floor is
    // the east edge at x = -8 (floor 1.0; x = 0 is a mesh void, so there
    // is no Market corridor to stand in — the door position is the east
    // site edge). That bot moves to (-8, -70).
    // FIXED 2026-11-06 (map validation vs ascentB.bin): the old spawn
    // (-20, 1, -50) sits in a mesh hole — no floor triangle at all there,
    // so getFloorHeight fails and the player is stuck. The nearest solid
    // ground (19/25 neighbours solid, all y = 1.0) is (-23, -50), 3 m west.
    m_spawnPos = { -23.0f, 1.0f, -50.0f };
    // Faces the first bot (Lane Close Left Corner at -24.5, -58.5):
    // yaw = atan2(-1.5, 8.5) ~ -10 deg.
    m_spawnYaw = -10.0f;

    PreAimBot b;

    // Lane close corner: the first angle an attacker meets, held tight
    // to the wall. Yaw faces the B Main entry: atan2(1.5, -8.5) ~ 170.
    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = 170.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: box top is y = 2.945 in the .bin, not 2.2 —
    // the old bot was sunk 0.74 m into the box.
    // Elevated Green Box: watches Lane and the site from above. Faces
    // the entry: atan2(2, -12) ~ 170 deg.
    b.name = "Green Box Top (Elevated Head Level)";
    b.pos = { -25.0f, 2.95f, -62.0f }; // Elevated! Tests vertical crosshair placement
    b.yawDeg = 170.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // MOVED 2026-11-06: (-23, -66.5) was mid-site, not Market. The
    // Market door is the east site edge; (-8, -70) is the verified floor
    // there (y = 1.0). Holds the Market door and the east site. Faces
    // the entry: atan2(15, -20) ~ -143 deg.
    b.name = "Market Door Edge";
    b.pos = { -8.0f, 1.0f, -70.0f };
    b.yawDeg = -143.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    // Default / Triple boxes: the safest plant spot, played from
    // Boathouse. Faces the entry: atan2(9, -16) ~ 150 deg.
    b.name = "B Site Default Box";
    b.pos = { -32.0f, 1.0f, -66.0f };
    b.yawDeg = 150.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the Boathouse waterline is y = -0.83 in the .bin,
    // not 0.0 — the old bot floated 0.83 m above the water.
    // Boathouse: the back-site anchor — most cover, hardest to clear,
    // the post-plant spot. Faces the entry: atan2(17, -19) ~ 138 deg.
    b.name = "Boathouse Front Entrance";
    b.pos = { -40.0f, -0.83f, -69.0f };
    b.yawDeg = 138.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // Deep back alley behind Boathouse. Faces the entry:
    // atan2(29, -21) ~ 126 deg.
    b.name = "Boathouse Stairs / Back Alley";
    b.pos = { -52.0f, 1.5f, -71.0f };
    b.yawDeg = 126.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the old spot (-4, 1, -88) is a mesh hole (only a
    // roof at 16.44 above it). (-6, -88) is the nearest solid site floor
    // (y = 1.003, 15/25 neighbours solid). Yaw faces the spawn (-23, -50):
    // atan2(-17, -38) ~ -156 deg.
    // CT Arch: the retake lane — the long angle into the site. Already
    // faced the entry; unchanged.
    b.name = "CT Arch Long Range Peek";
    b.pos = { -6.0f, 1.0f, -88.0f };
    b.yawDeg = -156.0f;
    b.reactionLimit = 0.50;
    m_bots.push_back(b);
}

void PreAimCourse::initBindB() {
    // FIXED 2026-11-06 (map validation vs bindB.bin): the B Long floor at
    // the old spawn is y = 3.959, which is 0.96 above the old spawn y = 3.0
    // — beyond the 0.75 step-up, so getFloorHeight finds NO floor and the
    // player falls through the world. Spawn y = 3.96 sits on the ramp
    // (25/25 neighbours solid, smooth 3.46–4.59 ramp).
    m_spawnPos = { 108.0f, 3.96f, -32.0f };
    // Faces the first bot (Garden Vista at 102, -40):
    // yaw = atan2(-6, 8) ~ -37 deg. The old 180 deg faced away.
    m_spawnYaw = -37.0f;

    PreAimBot b;

    // FIXED 2026-11-06: Garden floor is y = 5.33 in the .bin, not 3.0 —
    // the old bot was buried 2.3 m inside the ramp. (The game floor at the
    // exact point is 5.334 — a narrow raised strip/curb along x = 102;
    // the flat Garden floor just west of it is 5.0. The bot stands on the
    // strip, matching the floor the game itself returns there.)
    // TACTICAL REDESIGN 2026-11-06: same six spots (they were already
    // real: Garden, Hookah, Sand, Center/Tube, Elbow, Hall), but three
    // bots faced the wrong way — Garden (20 deg) and Hookah (0 deg)
    // faced into the site, Hell (270 deg) faced due west. Every yaw
    // below is atan2(dx, -dz) toward the B Long spawn (108, -32):
    // a defender holding B Long faces the Long, not the site.
    b.name = "Garden Vista Left Angle";
    b.pos = { 102.0f, 5.33f, -40.0f };
    b.yawDeg = 143.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: there is no window sill at 6.5 — the only floor
    // under this spot is the Hookah room floor at y = 5.0, so the old bot
    // floated 1.5 m above it.
    b.name = "Hookah Window Long Hold";
    b.pos = { 89.0f, 5.0f, -36.5f }; // Elevated window
    b.yawDeg = 103.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // FIXED 2026-11-06: game-rule floor is y = 3.081 (8 cm above the old
    // y = 3.0 — the bot's feet were sunk into the site floor).
    b.name = "Site Sand Container Front";
    b.pos = { 115.0f, 3.08f, -48.0f };
    b.yawDeg = -156.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the 3.5 m surface is a real box-top plateau
    // (~4 m wide, x 111–115 at z = -58, verified on a 0.5 m probe) — the
    // old bot at y = 3.0 was sunk 0.5 m into the box.
    b.name = "B Site Center Box";
    b.pos = { 111.0f, 3.5f, -58.0f };
    b.yawDeg = -173.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "B Elbow Hold";
    b.pos = { 115.0f, 3.0f, -72.0f };
    b.yawDeg = -170.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    b.name = "B Hall / Hell Deep Hold";
    b.pos = { 130.0f, 3.0f, -53.0f };
    b.yawDeg = -134.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);
}

void PreAimCourse::initHavenA() {
    // FIXED 2026-11-06 (map validation vs havenA.bin): the old spawn
    // (90, 0, -30) has no floor at all in the .bin — getFloorHeight
    // returns false there, so the player falls through the world. (The
    // only triangle at that point is a steep slope at -2.97, but it is
    // not a floor triangle.) (90, -36) is on the A Long ramp
    // (y = 2.563, 9/9 neighbours solid), 6 m up the ramp.
    m_spawnPos = { 90.0f, 2.56f, -36.0f };
    // Faces the first bot (A Long Corner Peek at 95, -55):
    // yaw = atan2(5, 19) ~ +15 deg. The old 180 deg faced away.
    m_spawnYaw = 15.0f;

    PreAimBot b;

    // FIXED 2026-11-06: A Long floor is y = 4.21 in the .bin, not 0.0 —
    // the old bot was buried 4.2 m inside the ramp.
    // Yaw faces the spawn (90, -36): atan2(-5, -19) ~ -165 deg.
    // (The old 170 deg faced 25 deg off the entry.)
    b.name = "A Long Corner Peek";
    b.pos = { 95.0f, 4.21f, -55.0f };
    b.yawDeg = -165.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the old spot (105, 1, -75) is a true void — no
    // triangle at all in the .bin. The A site floor is NOT at y ~ 4.2:
    // it is a gently sloped surface at y ~ 7.7 (west) down to ~6.7 (east),
    // verified as the lowest surface there (nothing below it). (102, -76)
    // is on it (y = 7.726). Yaw faces the spawn: atan2(-12, -40) ~ -163.
    b.name = "A Site Default Stack";
    b.pos = { 102.0f, 7.73f, -76.0f };
    b.yawDeg = -163.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: Heaven floor is y = 10.35 in the .bin, not 6.0 —
    // the old bot was buried 4.35 m inside the Heaven structure. Yaw faces
    // the spawn: atan2(-28, -49) ~ -150 deg.
    b.name = "A Heaven High Window";
    b.pos = { 118.0f, 10.35f, -85.0f }; // Elevated sniper window!
    b.yawDeg = -150.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the old spot (118, 1, -85) has no ground at all —
    // the only surface there is the Heaven floor at 10.35. The site floor
    // under Heaven ends at x ~ 116, so Hell moves to the closest site
    // floor point (113, -85), y = 6.96. Yaw faces the spawn:
    // atan2(-23, -49) ~ -155 deg.
    b.name = "A Hell Under-Window Hold";
    b.pos = { 113.0f, 6.96f, -85.0f };
    b.yawDeg = -155.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the walkable Sewers floor is y = 1.91 in the .bin
    // (a player descending the ramp lands on it), not 0.0 — the old bot
    // was buried 1.9 m under it.
    // Yaw faces the spawn: atan2(10, -34) ~ +164 deg.
    b.name = "A Short / Sewers Flank";
    b.pos = { 80.0f, 1.91f, -70.0f };
    b.yawDeg = 164.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);
}

void PreAimCourse::initHavenB() {
    // Bot/Spawn positions measured from the map itself
    // (optimized/Maps/havenB.blend, Blender raycast floor scan):
    // game coords = (blend_x, blend_z, -blend_y).
    // Corridor floor is z ~ 1.0, site floor is z ~ 3.0 (ramp between them).
    // Yaw values face the player's approach and are approximate:
    // verify the bot models look right on the first Windows run.
    // FIXED 2026-11-06: corridor floor is y = 1.064 in the .bin (6 cm
    // above the old y = 1.0; the player auto-steps it, but the .bin value
    // is exact). Site Left Corner also fixed: its floor is y = 2.869, so
    // the bot at 3.0 floated 0.13 m — it now stands at 2.87.
    m_spawnPos = { 24.0f, 1.06f, -60.0f };
    // Faces the first bot (Ramp Top Duel at 18, -76):
    // yaw = atan2(-6, 16) ~ -21 deg.
    m_spawnYaw = -21.0f;

    PreAimBot b;

    b.name = "Ramp Top Duel";
    b.pos = { 18.0f, 3.0f, -76.0f };
    b.yawDeg = 160.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // TACTICAL REDESIGN 2026-11-06: the old "Site Right Flank" at
    // (12, -88) stood 2.8 m from Site Left Corner (14, -90) — two bots
    // in the same corner, the random-placement signature. A floor probe
    // of havenB.bin shows the site really extends east to x ~ 44
    // (floor 3.0 at (44, -90)), so that bot moves to the true east
    // corner. Facing the corridor spawn (24, -60) from the east, the
    // player enters on their right — hence "East Corner". Yaw faces the
    // entry: atan2(-20, -30) ~ -146 deg.
    b.name = "Site Left Corner";
    b.pos = { 14.0f, 2.87f, -90.0f };
    b.yawDeg = 162.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Site Default Box";
    b.pos = { 30.0f, 3.0f, -100.0f };
    b.yawDeg = -171.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "Back Site Corner";
    b.pos = { 20.0f, 3.0f, -108.0f };
    b.yawDeg = 175.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Site East Corner";
    b.pos = { 44.0f, 3.0f, -90.0f };
    b.yawDeg = -146.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);
}

// MAP-05. Added 2026-11-06 (map-validation round).
// Haven C lives in havenA.bin as the large flat area at y ~ 4.3
// (x 71–113, z -143..-89, ~784 m^2, median 4.30 — found by a flat-area
// scan; the site letter is by elimination: A is the 7.7 m area, B is the
// 3.0 m west area at havenB.bin's site level, so this is C — confirm
// in-game). The approach from the north is broken in the .bin (voids at
// z -101..-115), so the player spawns ON the site's north edge and
// clears into it. Every bot below stands on a verified floor.
void PreAimCourse::initHavenC() {
    m_spawnPos = { 95.0f, 4.45f, -115.0f };
    // Faces the first bot (C Entry Right at 100, -120):
    // yaw = atan2(5, 5) ~ +45 deg.
    m_spawnYaw = 45.0f;

    PreAimBot b;

    b.name = "C Entry Right Angle";
    b.pos = { 100.0f, 4.09f, -120.0f };
    b.yawDeg = -135.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "C Site West Corner";
    b.pos = { 88.0f, 4.11f, -125.0f };
    b.yawDeg = 145.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "C Site East Corner";
    b.pos = { 105.0f, 3.95f, -125.0f };
    b.yawDeg = -135.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "C Site Default Stack";
    b.pos = { 100.0f, 4.45f, -130.0f };
    b.yawDeg = -162.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "C Site Deep South-West";
    b.pos = { 90.0f, 4.36f, -135.0f };
    b.yawDeg = 166.0f;
    b.reactionLimit = 0.50;
    m_bots.push_back(b);
}

// MAP-06. Added 2026-11-06. The reverse of MAP-05: spawn deep on the
// C site floor and clear the entry angles (retake). Same verified floor
// data as MAP-05; the entry bot at (85, -105) stands on the approach
// floor (y = 3.62) north of the site.
void PreAimCourse::initHavenCRetake() {
    m_spawnPos = { 95.0f, 4.50f, -130.0f };
    // Faces the first bot (C Entry North at 95, -115):
    // yaw = atan2(0, -15) = 180 deg.
    m_spawnYaw = 180.0f;

    PreAimBot b;

    b.name = "C Entry North (Main)";
    b.pos = { 95.0f, 4.45f, -115.0f };
    b.yawDeg = 0.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "C Site West Corner";
    b.pos = { 88.0f, 4.11f, -125.0f };
    b.yawDeg = 54.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "C Entry Right";
    b.pos = { 100.0f, 4.09f, -120.0f };
    b.yawDeg = -27.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "C Long Approach";
    b.pos = { 85.0f, 3.62f, -105.0f };
    b.yawDeg = 22.0f;
    b.reactionLimit = 0.50;
    m_bots.push_back(b);
}

// MAP-07. Added 2026-11-06. A vertical-flick drill on Haven A, not a
// tactical clear: four bots, one per verified level of the A site
// (Sewers 1.91 -> A Long 4.21 -> site 7.73 -> Heaven 10.35), ordered low
// to high so each rep is a bigger upward flick. Spawn is the fixed Haven
// A spawn on the ramp; all four bots are within 57 m of it.
void PreAimCourse::initHavenAElevLadder() {
    m_spawnPos = { 90.0f, 2.56f, -36.0f };
    // Faces the first bot (Sewers Low at 80, -70):
    // yaw = atan2(-10, 34) ~ -16 deg.
    m_spawnYaw = -16.0f;

    PreAimBot b;

    b.name = "Sewers Low (1.9 m)";
    b.pos = { 80.0f, 1.91f, -70.0f };
    b.yawDeg = 164.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "A Long Mid (4.2 m)";
    b.pos = { 95.0f, 4.21f, -55.0f };
    b.yawDeg = -165.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "A Site High (7.7 m)";
    b.pos = { 102.0f, 7.73f, -76.0f };
    b.yawDeg = -163.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    b.name = "Heaven Top (10.4 m)";
    b.pos = { 118.0f, 10.35f, -85.0f };
    b.yawDeg = -150.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);
}

// MAP-09. Added 2026-11-06 (sensible-tactical round). Post-plant
// defuse on Ascent B: the spike is planted at Default (Triple), the
// player retakes from CT. The four bots hold real post-plant spots —
// Default (on the spike), Market Edge (closest retake entrance), Lane
// Close (watches the B Main flank), Boathouse (the long post-plant
// angle, hardest to clear) — and all face CT, the retake entry.
// Spike timer 45 s (real Valorant: the spike detonates 45 s after
// plant); holders are pre-aimed, so their reactions are 0.38–0.45 s,
// slightly faster than the attack course. Clear all four before the
// timer or the round is lost.
// Spawn: the CT corridor at (-6, -95) — a floor probe of ascentB.bin
// shows the CT floor (y = 1.003) runs from (-6, -88) to (-6, -95);
// (-6, -100) is a void, so -95 is as deep into CT as the .bin goes.
void PreAimCourse::initAscentBDefuse() {
    m_timeLimitSec = 45.0;
    m_spawnPos = { -6.0f, 1.0f, -95.0f };
    // Faces the first bot (Default Box at -32, -66):
    // yaw = atan2(-26, -29) ~ -138 deg.
    m_spawnYaw = -138.0f;

    PreAimBot b;

    // On the spike (Default / Triple). Faces CT: atan2(26, 29) ~ 42.
    b.name = "B Site Default Box (Spike)";
    b.pos = { -32.0f, 1.0f, -66.0f };
    b.yawDeg = 42.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    // Holds the closest retake entrance (CT). Faces CT:
    // atan2(2, 25) ~ 5 deg.
    b.name = "Market Door Edge";
    b.pos = { -8.0f, 1.0f, -70.0f };
    b.yawDeg = 5.0f;
    b.reactionLimit = 0.38;
    m_bots.push_back(b);

    // Watches the B Main flank. Faces CT: atan2(18.5, 36.5) ~ 27 deg.
    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = 27.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // The long post-plant angle from the back site. Faces CT:
    // atan2(34, 26) ~ 53 deg.
    b.name = "Boathouse Front Entrance";
    b.pos = { -40.0f, -0.83f, -69.0f };
    b.yawDeg = 53.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);
}

// MAP-10. Added 2026-11-06. Post-plant defuse on Bind B: the player
// retakes from B Hall — the defender-side corridor (a floor probe of
// bindB.bin shows the Hall floor at y = 3.0 runs z -45..-65 at x ~ 130;
// east of x = 130 there is no floor, so the Hall IS the retake entry).
// The five bots hold the post-plant spread: Sand (site entry), Center
// Box / Tube (mid site), Elbow (deep corner), Garden + Hookah (watch
// the Long flank). All face Hall. Spike timer 45 s.
void PreAimCourse::initBindBDefuse() {
    m_timeLimitSec = 45.0;
    m_spawnPos = { 130.0f, 3.0f, -53.0f };
    // Faces the first bot (Sand Container at 115, -48):
    // yaw = atan2(-15, -5) ~ -108 deg.
    m_spawnYaw = -108.0f;

    PreAimBot b;

    // Site entry, closest to Hall. Faces Hall: atan2(15, 5) ~ 72 deg.
    b.name = "Site Sand Container Front";
    b.pos = { 115.0f, 3.08f, -48.0f };
    b.yawDeg = 72.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    // Mid-site Tube box. Faces Hall: atan2(19, -5) ~ 105 deg.
    b.name = "B Site Center Box";
    b.pos = { 111.0f, 3.5f, -58.0f };
    b.yawDeg = 105.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // Deep corner. Faces Hall: atan2(15, -19) ~ 142 deg.
    b.name = "B Elbow Hold";
    b.pos = { 115.0f, 3.0f, -72.0f };
    b.yawDeg = 142.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // Watches the Long flank from the Garden ramp. Faces Hall:
    // atan2(28, 13) ~ 65 deg.
    b.name = "Garden Vista Left Angle";
    b.pos = { 102.0f, 5.33f, -40.0f };
    b.yawDeg = 65.0f;
    b.reactionLimit = 0.38;
    m_bots.push_back(b);

    // Watches the Long flank from the Hookah window. Faces Hall:
    // atan2(41, 16.5) ~ 68 deg.
    b.name = "Hookah Window Long Hold";
    b.pos = { 89.0f, 5.0f, -36.5f };
    b.yawDeg = 68.0f;
    b.reactionLimit = 0.38;
    m_bots.push_back(b);
}

// MAP-11. Added 2026-11-06. Post-plant defuse on Haven A: the player
// retakes from Heaven — the classic A retake position. (There is no CT
// floor in the .bin north of the site: a probe shows z -90..-110 is a
// void except a 3.92 m lower area at (100, -110) with no walk-up, so
// Heaven, at the verified 10.35 m floor, is the only real retake
// perch.) The four bots hold the post-plant spread around the Default
// plant spot and all face Heaven. Spike timer 45 s. Note the height
// spread: the player shoots DOWN from 10.35 m at heads from 1.91 m
// (Short) to 7.73 m (Default) — the retake is a vertical-flick drill
// as much as a clear.
void PreAimCourse::initHavenADefuse() {
    m_timeLimitSec = 45.0;
    m_spawnPos = { 118.0f, 10.35f, -85.0f };
    // Faces the first bot (Default Stack at 102, -76):
    // yaw = atan2(-16, -9) ~ -119 deg.
    m_spawnYaw = -119.0f;

    PreAimBot b;

    // On the spike (Default). Faces Heaven: atan2(16, 9) ~ 61 deg.
    b.name = "A Site Default Stack (Spike)";
    b.pos = { 102.0f, 7.73f, -76.0f };
    b.yawDeg = 61.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    // Under Heaven, covers the site from below. Faces Heaven: 90 deg.
    b.name = "A Hell Under-Window Hold";
    b.pos = { 113.0f, 6.96f, -85.0f };
    b.yawDeg = 90.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // Watches the Long flank. Faces Heaven: atan2(23, 30) ~ 37 deg.
    b.name = "A Long Corner Peek";
    b.pos = { 95.0f, 4.21f, -55.0f };
    b.yawDeg = 37.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // Watches the Short flank. Faces Heaven: atan2(38, 15) ~ 68 deg.
    b.name = "A Short / Sewers Flank";
    b.pos = { 80.0f, 1.91f, -70.0f };
    b.yawDeg = 68.0f;
    b.reactionLimit = 0.38;
    m_bots.push_back(b);
}

// MAP-12. Added 2026-11-06. 1v5 clutch on Ascent B: the player enters
// from B Main against five bots spread across the site's real anchor
// spots. Fair by design, not by nerfing the bots:
// - Reaction times are STAGGERED by angle distance (Lane 0.55 s, Green
//   Box 0.65 s, Default 0.80 s, Boathouse Stairs 1.05 s, CT Arch
//   1.25 s) — the far angles give you time, the close ones do not.
// - The engine only runs a bot's reaction timer while the bot is in
//   the player's sight cone, so a bot you are not looking at cannot
//   fire. The clutch skill this trains is the real one: isolate 1v1s,
//   never face two angles at once.
// - 3 hits = round lost (m_maxDamage = 3): each bot hit is ~35 HP of
//   a 100 HP round. There is no timer — a clutch is won by patience,
//   not speed.
void PreAimCourse::initAscentBClutch() {
    m_maxDamage = 3;
    m_spawnPos = { -23.0f, 1.0f, -50.0f };
    // Faces the first bot (Lane Close at -24.5, -58.5): ~ -10 deg.
    m_spawnYaw = -10.0f;

    PreAimBot b;

    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = 170.0f;
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "Green Box Top (Elevated Head Level)";
    b.pos = { -25.0f, 2.95f, -62.0f };
    b.yawDeg = 170.0f;
    b.reactionLimit = 0.65;
    m_bots.push_back(b);

    b.name = "B Site Default Box";
    b.pos = { -32.0f, 1.0f, -66.0f };
    b.yawDeg = 150.0f;
    b.reactionLimit = 0.80;
    m_bots.push_back(b);

    // Deep back-site corner instead of Boathouse Front (MAP-01's pick):
    // the clutch needs the two long angles, and the Stairs corner is
    // the deeper, harder clear. Faces the entry: atan2(29, -21) ~ 126.
    b.name = "Boathouse Stairs / Back Alley";
    b.pos = { -52.0f, 1.5f, -71.0f };
    b.yawDeg = 126.0f;
    b.reactionLimit = 1.05;
    m_bots.push_back(b);

    // The long retake angle — the slowest reaction because it is the
    // farthest duel (38 m from the spawn). Faces the entry: -156 deg.
    b.name = "CT Arch Long Range Peek";
    b.pos = { -6.0f, 1.0f, -88.0f };
    b.yawDeg = -156.0f;
    b.reactionLimit = 1.25;
    m_bots.push_back(b);
}

// MAP-13. Added 2026-11-06. Crosshair-placement drill on Bind B Long:
// the same six anchor spots as MAP-02, but BODY SHOTS DO NOT KILL
// (m_headshotsOnly) — the round only ends on headshots. The drill is
// keeping the crosshair at head height while the ground under the
// enemy changes: Garden 5.33 m and Hookah 5.0 m are elevated, Sand
// 3.08 m / Center 3.5 m / Elbow 3.0 m / Hall 3.0 m are low, so the head
// line drops ~2.3 m as you walk down the lane. Wasted body shots are
// counted (see bodyHitsWasted) and shown on the results screen — the
// score that matters here is headshot rate + average vertical error,
// both already tracked. No timer, no damage limit: precision, not
// speed.
void PreAimCourse::initBindBHeadHeight() {
    m_headshotsOnly = true;
    m_spawnPos = { 108.0f, 3.96f, -32.0f };
    // Faces the first bot (Garden Vista at 102, -40): ~ -37 deg.
    m_spawnYaw = -37.0f;

    PreAimBot b;

    b.name = "Garden Vista Left Angle";
    b.pos = { 102.0f, 5.33f, -40.0f };
    b.yawDeg = 143.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Hookah Window Long Hold";
    b.pos = { 89.0f, 5.0f, -36.5f };
    b.yawDeg = 103.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Site Sand Container Front";
    b.pos = { 115.0f, 3.08f, -48.0f };
    b.yawDeg = -156.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "B Site Center Box";
    b.pos = { 111.0f, 3.5f, -58.0f };
    b.yawDeg = -173.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "B Elbow Hold";
    b.pos = { 115.0f, 3.0f, -72.0f };
    b.yawDeg = -170.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "B Hall / Hell Deep Hold";
    b.pos = { 130.0f, 3.0f, -53.0f };
    b.yawDeg = -134.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);
}

// MAP-14. Added 2026-11-06. Corner-holding, from the defender's side:
// the player IS the Boathouse anchor and five attackers push in from
// B Main. The bots "peek" in order — their reactions are staggered
// 0.50–1.05 s (Lane first, CT Arch last), simulating the push — and
// every bot faces the Boathouse, the angle it is clearing. The drill
// is holding one angle and tapping each peek as it comes, instead of
// swinging. No timer, no damage limit: a hold drill, not a round.
// Spawn is the Boathouse floor itself (-0.83 m, verified in the
// validation round).
void PreAimCourse::initAscentBAnchorHold() {
    m_spawnPos = { -40.0f, -0.83f, -69.0f };
    // Faces the first bot (Lane Close at -24.5, -58.5):
    // yaw = atan2(15.5, -10.5) ~ 124 deg.
    m_spawnYaw = 124.0f;

    PreAimBot b;

    // First peek, from the Lane. Faces Boathouse: atan2(-15.5, 10.5) ~ -56.
    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = -56.0f;
    b.reactionLimit = 0.50;
    m_bots.push_back(b);

    // Elevated peek over the Green Box. Faces Boathouse:
    // atan2(-15, 7) ~ -65 deg.
    b.name = "Green Box Top (Elevated Head Level)";
    b.pos = { -25.0f, 2.95f, -62.0f };
    b.yawDeg = -65.0f;
    b.reactionLimit = 0.60;
    m_bots.push_back(b);

    // Default peek. Faces Boathouse: atan2(-8, 3) ~ -69 deg.
    b.name = "B Site Default Box";
    b.pos = { -32.0f, 1.0f, -66.0f };
    b.yawDeg = -69.0f;
    b.reactionLimit = 0.75;
    m_bots.push_back(b);

    // Market-side peek. Faces Boathouse: atan2(-32, -1) ~ -92 deg.
    b.name = "Market Door Edge";
    b.pos = { -8.0f, 1.0f, -70.0f };
    b.yawDeg = -92.0f;
    b.reactionLimit = 0.90;
    m_bots.push_back(b);

    // The late CT peek — the longest angle in. Faces Boathouse:
    // atan2(-34, -19) ~ -119 deg.
    b.name = "CT Arch Long Range Peek";
    b.pos = { -6.0f, 1.0f, -88.0f };
    b.yawDeg = -119.0f;
    b.reactionLimit = 1.05;
    m_bots.push_back(b);
}

int PreAimCourse::botsRemaining() const {
    int rem = 0;
    for (const auto& b : m_bots) {
        if (b.alive) ++rem;
    }
    return rem;
}

void PreAimCourse::update(double now, double /*dt*/, const Vec3& playerEyePos, float playerPitchDeg, float playerYawDeg, const MapMesh* map) {
    m_lastNow        = now;
    m_activeBotIdx   = -1;
    m_currentVertErr = 0.0f;

    float closestDist = 9999.0f;

    float radYaw   = radians(playerYawDeg);
    float radPitch = radians(playerPitchDeg);
    Vec3 camFwd = {
        std::sin(radYaw) * std::cos(radPitch),
        std::sin(radPitch),
        -std::cos(radYaw) * std::cos(radPitch)
    };

    for (size_t i = 0; i < m_bots.size(); ++i) {
        auto& bot = m_bots[i];
        if (!bot.alive) continue;

        // Orient bot to face toward player
        bot.yawDeg = degrees(std::atan2(playerEyePos.x - bot.pos.x, playerEyePos.z - bot.pos.z));

        // Exact eye/head height (1.62m above feet, matching Player::kEyeHeight and scaled Jett mesh)
        Vec3 headPos = { bot.pos.x, bot.pos.y + 1.62f, bot.pos.z };
        Vec3 toHead  = headPos - playerEyePos;
        float dist   = length(toHead);

        if (dist > 65.0f || dist < 0.2f) {
            if (!bot.firedAtPlayer) {
                bot.hasSeenPlayer = false;
                bot.firstSeenTime = 0.0;
            }
            continue;
        }

        Vec3 dirToHead = toHead * (1.0f / dist);
        float dotFwd   = dot(camFwd, dirToHead);

        // Must be within camera view cone AND have true 3D line-of-sight (unoccluded by walls)
        bool visible = (dotFwd > 0.35f);
        if (visible && map && map->isLoaded()) {
            visible = map->hasLineOfSight(playerEyePos, headPos);
        }

        if (visible) {
            float horizDist   = std::sqrt(toHead.x * toHead.x + toHead.z * toHead.z);
            float neededPitch = degrees(std::atan2(toHead.y, horizDist));
            float vertErr     = playerPitchDeg - neededPitch;

            if (!bot.hasSeenPlayer) {
                bot.hasSeenPlayer  = true;
                bot.firstSeenTime  = now;
                // Keep the very first sighting's telemetry; later re-peeks
                // restart the timer but do not overwrite it.
                if (!bot.hasFirstSightTelemetry) {
                    bot.firstSightVertErrDeg   = vertErr;
                    bot.hasFirstSightTelemetry = true;
                }
            }

            if (dist < closestDist) {
                closestDist      = dist;
                m_activeBotIdx   = static_cast<int>(i);
                m_currentVertErr = vertErr;
            }

            // Reaction timer (only counts time the bot can see the player)
            if (!bot.firedAtPlayer) {
                double elapsed = now - bot.firstSeenTime;
                if (elapsed >= bot.reactionLimit) {
                    bot.firedAtPlayer = true;
                    m_damageTaken++;
                }
            }
        } else {
            // If player breaks line-of-sight (jiggle-peeks back behind cover), reset reaction timer!
            if (bot.hasSeenPlayer && !bot.firedAtPlayer) {
                bot.hasSeenPlayer = false;
                bot.firstSeenTime = 0.0;
            }
        }
    }

    // Fail states (2026-11-06 tactical round). Checked here, in update(),
    // so main.cpp only has to ask isFailed() once per frame.
    // - Spike timer: once the limit passes, the round is lost even if
    //   bots are still alive. (A course already complete cannot fail:
    //   isComplete() is checked first in main.cpp.)
    if (m_failReason == FailReason::None && m_timeLimitSec > 0.0 &&
        now - m_startTime >= m_timeLimitSec) {
        m_failReason = FailReason::TimeUp;
    }
    // - Damage limit: 3 hits = round lost (clutch). Each bot fires at
    //   most once (firedAtPlayer), so m_damageTaken only grows here.
    if (m_failReason == FailReason::None && m_maxDamage > 0 &&
        m_damageTaken >= m_maxDamage) {
        m_failReason = FailReason::DamageLimit;
    }
}

float PreAimCourse::activeBotReactionProgress(double now) const {
    if (m_activeBotIdx < 0 || m_activeBotIdx >= static_cast<int>(m_bots.size())) return 0.0f;
    const auto& b = m_bots[m_activeBotIdx];
    if (!b.hasSeenPlayer || !b.alive || b.reactionLimit <= 1e-4) return 0.0f;
    double t = (now >= 0.0) ? now : m_lastNow;
    double elapsed = t - b.firstSeenTime;
    return std::clamp(static_cast<float>(elapsed / b.reactionLimit), 0.0f, 1.0f);
}

const std::string& PreAimCourse::activeBotName() const {
    static const std::string kEmpty = "";
    if (m_activeBotIdx < 0 || m_activeBotIdx >= static_cast<int>(m_bots.size())) return kEmpty;
    return m_bots[m_activeBotIdx].name;
}

int PreAimCourse::shoot(double now, const Vec3& eyePos, const Vec3& fwdRay, bool isAccurate, PreAimResult& outResult, const MapMesh* map) {
    if (!isAccurate) {
        return -1; // Moving inaccuracy whiff
    }

    float closestHitT = 9999.0f;
    int   hitBotIdx   = -1;
    int   hitRegion   = 0; // 2 = head, 1 = body

    for (size_t i = 0; i < m_bots.size(); ++i) {
        const auto& bot = m_bots[i];
        if (!bot.alive) continue;

        // 1. Head sphere test (centered at y = bot.pos.y + 1.62m, matching Jett's eyes & Player::kEyeHeight)
        Vec3 headCenter = { bot.pos.x, bot.pos.y + 1.62f, bot.pos.z };
        Vec3 ocH  = eyePos - headCenter;
        float bH  = dot(ocH, fwdRay);
        float cH  = dot(ocH, ocH) - bot.headRadius * bot.headRadius;
        float discH = bH * bH - cH;
        if (discH >= 0.0f) {
            float t = -bH - std::sqrt(discH);
            if (t > 0.0f && t < closestHitT) {
                Vec3 hitPt = eyePos + fwdRay * t;
                if (!map || !map->isLoaded() || map->hasLineOfSight(eyePos, hitPt)) {
                    closestHitT = t;
                    hitBotIdx   = static_cast<int>(i);
                    hitRegion   = 2; // HEADSHOT
                }
            }
        }

        // 2. Body cylinder test (y from +0.15m to +1.47m)
        Vec3 bodyBot = { bot.pos.x, bot.pos.y + 0.15f, bot.pos.z };
        Vec3 bodyTop = { bot.pos.y < -999.0f ? 0.0f : bot.pos.x, bot.pos.y + 1.47f, bot.pos.z };
        float dx = eyePos.x - bot.pos.x;
        float dz = eyePos.z - bot.pos.z;
        float A  = fwdRay.x * fwdRay.x + fwdRay.z * fwdRay.z;
        float B  = 2.0f * (dx * fwdRay.x + dz * fwdRay.z);
        float C  = dx * dx + dz * dz - bot.bodyRadius * bot.bodyRadius;
        float discB = B * B - 4.0f * A * C;
        if (discB >= 0.0f && A > 1e-6f) {
            float t = (-B - std::sqrt(discB)) / (2.0f * A);
            if (t > 0.0f && t < closestHitT) {
                float hitY = eyePos.y + t * fwdRay.y;
                if (hitY >= bodyBot.y && hitY <= bodyTop.y) {
                    Vec3 hitPt = eyePos + fwdRay * t;
                    if (!map || !map->isLoaded() || map->hasLineOfSight(eyePos, hitPt)) {
                        closestHitT = t;
                        hitBotIdx   = static_cast<int>(i);
                        hitRegion   = 1; // BODYSHOT
                    }
                }
            }
        }
    }

    if (hitBotIdx >= 0) {
        auto& bot = m_bots[hitBotIdx];
        // Headshots-only drill (2026-11-06 tactical round, MAP-13): a
        // body hit does not kill — it is a wasted shot. The bot stays
        // alive, no history entry is recorded, and the shot still returns
        // 1 so the hit sound plays. (Note the hit test picks the CLOSEST
        // hit first, so a body in front still blocks a headshot behind
        // it — the same as a real wall of cover.)
        if (m_headshotsOnly && hitRegion == 1) {
            m_bodyHitsWasted++;
            return 1;
        }
        bot.alive = false;

        outResult.botName          = bot.name;
        outResult.isHeadshot       = (hitRegion == 2);
        outResult.isStopped        = isAccurate;
        outResult.killTimeMs       = (bot.firstSeenTime > 0.0) ? std::max(50.0, (now - bot.firstSeenTime) * 1000.0) : 220.0;
        outResult.vertErrDeg       = bot.firstSightVertErrDeg;
        outResult.playerTookDamage = bot.firedAtPlayer;

        m_history.push_back(outResult);
        return hitRegion;
    }

    return 0; // miss
}
