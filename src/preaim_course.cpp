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
        default: return "assets/maps/ascentB.bin";
    }
}

void PreAimCourse::start(MapCourseId courseId, double /*now*/) {
    m_courseId = courseId;
    m_bots.clear();
    m_history.clear();
    m_activeBotIdx = -1;
    m_currentVertErr = 0.0f;
    m_damageTaken = 0;

    switch (courseId) {
        case MapCourseId::AscentB_Clear:   initAscentB(); break;
        case MapCourseId::BindB_LongClear: initBindB(); break;
        case MapCourseId::HavenA_Clear:    initHavenA(); break;
        case MapCourseId::HavenB_Clear:    initHavenB(); break;
        case MapCourseId::HavenC_Clear:    initHavenC(); break;
        case MapCourseId::HavenC_Retake:   initHavenCRetake(); break;
        case MapCourseId::HavenA_ElevLadder: initHavenAElevLadder(); break;
        default: initAscentB(); break;
    }
}

void PreAimCourse::initAscentB() {
    // FIXED 2026-11-06 (map validation vs ascentB.bin): the old spawn
    // (-20, 1, -50) sits in a mesh hole — no floor triangle at all there,
    // so getFloorHeight fails and the player is stuck. The nearest solid
    // ground (19/25 neighbours solid, all y = 1.0) is (-23, -50), 3 m west.
    m_spawnPos = { -23.0f, 1.0f, -50.0f };
    // Faces the first bot (Lane Close Left Corner at -24.5, -58.5):
    // yaw = atan2(-1.5, 8.5) ~ -10 deg.
    m_spawnYaw = -10.0f;

    PreAimBot b;

    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = 45.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: box top is y = 2.945 in the .bin, not 2.2 —
    // the old bot was sunk 0.74 m into the box.
    b.name = "Green Box Top (Elevated Head Level)";
    b.pos = { -25.0f, 2.95f, -62.0f }; // Elevated! Tests vertical crosshair placement
    b.yawDeg = 45.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "Market Door Peeker";
    b.pos = { -23.0f, 1.0f, -66.5f };
    b.yawDeg = 90.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    b.name = "B Site Default Box";
    b.pos = { -32.0f, 1.0f, -66.0f };
    b.yawDeg = 45.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the Boathouse waterline is y = -0.83 in the .bin,
    // not 0.0 — the old bot floated 0.83 m above the water.
    b.name = "Boathouse Front Entrance";
    b.pos = { -40.0f, -0.83f, -69.0f };
    b.yawDeg = 30.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Boathouse Stairs / Back Alley";
    b.pos = { -52.0f, 1.5f, -71.0f };
    b.yawDeg = 60.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the old spot (-4, 1, -88) is a mesh hole (only a
    // roof at 16.44 above it). (-6, -88) is the nearest solid site floor
    // (y = 1.003, 15/25 neighbours solid). Yaw faces the spawn (-23, -50):
    // atan2(-17, -38) ~ -156 deg.
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
    b.name = "Garden Vista Left Angle";
    b.pos = { 102.0f, 5.33f, -40.0f };
    b.yawDeg = 20.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: there is no window sill at 6.5 — the only floor
    // under this spot is the Hookah room floor at y = 5.0, so the old bot
    // floated 1.5 m above it.
    b.name = "Hookah Window Long Hold";
    b.pos = { 89.0f, 5.0f, -36.5f }; // Elevated window
    b.yawDeg = 0.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    // FIXED 2026-11-06: game-rule floor is y = 3.081 (8 cm above the old
    // y = 3.0 — the bot's feet were sunk into the site floor).
    b.name = "Site Sand Container Front";
    b.pos = { 115.0f, 3.08f, -48.0f };
    b.yawDeg = 160.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    // FIXED 2026-11-06: the 3.5 m surface is a real box-top plateau
    // (~4 m wide, x 111–115 at z = -58, verified on a 0.5 m probe) — the
    // old bot at y = 3.0 was sunk 0.5 m into the box.
    b.name = "B Site Center Box";
    b.pos = { 111.0f, 3.5f, -58.0f };
    b.yawDeg = 180.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "B Elbow Hold";
    b.pos = { 115.0f, 3.0f, -72.0f };
    b.yawDeg = 200.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    b.name = "B Hall / Hell Deep Hold";
    b.pos = { 130.0f, 3.0f, -53.0f };
    b.yawDeg = 270.0f;
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
    b.name = "A Long Corner Peek";
    b.pos = { 95.0f, 4.21f, -55.0f };
    b.yawDeg = 170.0f;
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

    b.name = "Site Left Corner";
    b.pos = { 14.0f, 2.87f, -90.0f };
    b.yawDeg = 165.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Site Default Box";
    b.pos = { 30.0f, 3.0f, -100.0f };
    b.yawDeg = -150.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "Back Site Corner";
    b.pos = { 20.0f, 3.0f, -108.0f };
    b.yawDeg = 180.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Site Right Flank";
    b.pos = { 12.0f, 3.0f, -88.0f };
    b.yawDeg = 55.0f;
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

int PreAimCourse::botsRemaining() const {
    int rem = 0;
    for (const auto& b : m_bots) {
        if (b.alive) ++rem;
    }
    return rem;
}

void PreAimCourse::update(double now, double /*dt*/, const Vec3& playerEyePos, float playerPitchDeg, float playerYawDeg) {
    m_activeBotIdx = -1;
    m_currentVertErr = 0.0f;

    float closestDist = 9999.0f;

    for (size_t i = 0; i < m_bots.size(); ++i) {
        auto& bot = m_bots[i];
        if (!bot.alive) continue;

        Vec3 headPos = { bot.pos.x, bot.pos.y + 1.70f, bot.pos.z };
        Vec3 toHead = headPos - playerEyePos;
        float dist = length(toHead);

        // Simple frustum / line-of-sight check
        // NOTE: the reaction timer only runs while the bot is actually
        // visible. If the player looks away, the timer resets, so the bot
        // cannot "store up" unseen seconds and instant-fire on re-look.
        float radYaw = radians(playerYawDeg);
        float radPitch = radians(playerPitchDeg);
        Vec3 camFwd = {
            std::sin(radYaw) * std::cos(radPitch),
            std::sin(radPitch),
            -std::cos(radYaw) * std::cos(radPitch)
        };

        Vec3 dirToHead = toHead * (1.0f / dist);
        float dotFwd = dot(camFwd, dirToHead);

        // Within 65 m and the 70-degree sight cone?
        bool inCone = (dist <= 65.0f) && (dotFwd > 0.35f);
        if (!inCone) {
            bot.hasSeenPlayer = false;
            bot.firstSeenTime = 0.0;
            continue;
        }

        // Compute vertical pitch needed to hit head
        float horizDist = std::sqrt(toHead.x * toHead.x + toHead.z * toHead.z);
        float neededPitch = degrees(std::atan2(toHead.y, horizDist));
        float vertErr = playerPitchDeg - neededPitch;

        if (!bot.hasSeenPlayer) {
            bot.hasSeenPlayer = true;
            bot.firstSeenTime = now;
            // Keep the very first sighting's telemetry; later re-peeks
            // restart the timer but do not overwrite it.
            if (!bot.hasFirstSightTelemetry) {
                bot.firstSightVertErrDeg = vertErr;
                bot.hasFirstSightTelemetry = true;
            }
        }

        if (dist < closestDist) {
            closestDist = dist;
            m_activeBotIdx = static_cast<int>(i);
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
    }
}

float PreAimCourse::activeBotReactionProgress(double now) const {
    if (m_activeBotIdx < 0 || m_activeBotIdx >= static_cast<int>(m_bots.size())) return 0.0f;
    const auto& b = m_bots[m_activeBotIdx];
    if (!b.hasSeenPlayer || !b.alive || b.reactionLimit <= 0.0) return 0.0f;
    // Fraction of the bot's reaction window used up since it first saw
    // the player (0 = just spotted you, 1 = about to fire).
    float p = static_cast<float>((now - b.firstSeenTime) / b.reactionLimit);
    if (p < 0.0f) p = 0.0f;
    if (p > 1.0f) p = 1.0f;
    return p;
}

const std::string& PreAimCourse::activeBotName() const {
    static const std::string kEmpty = "";
    if (m_activeBotIdx < 0 || m_activeBotIdx >= static_cast<int>(m_bots.size())) return kEmpty;
    return m_bots[m_activeBotIdx].name;
}

int PreAimCourse::shoot(double now, const Vec3& eyePos, const Vec3& fwdRay, bool isAccurate, PreAimResult& outResult) {
    if (!isAccurate) {
        // Player shot while running! Movement inaccuracy whiff!
        return -1;
    }

    float closestHitT = 9999.0f;
    int   hitBotIdx   = -1;
    int   hitRegion   = 0; // 2 = head, 1 = body

    for (size_t i = 0; i < m_bots.size(); ++i) {
        const auto& bot = m_bots[i];
        if (!bot.alive) continue;

        // Head test (sphere at y = bot.pos.y + 1.70m, radius = headRadius)
        Vec3 headCenter = { bot.pos.x, bot.pos.y + 1.70f, bot.pos.z };
        Vec3 ocH = eyePos - headCenter;
        float bH = dot(ocH, fwdRay);
        float cH = dot(ocH, ocH) - bot.headRadius * bot.headRadius;
        float discH = bH * bH - cH;
        if (discH >= 0.0f) {
            float t = -bH - std::sqrt(discH);
            if (t > 0.0f && t < closestHitT) {
                closestHitT = t;
                hitBotIdx = static_cast<int>(i);
                hitRegion = 2; // HEADSHOT
            }
        }

        // Body test (capsule from y = 0.25 to y = 1.55)
        Vec3 bodyBot = { bot.pos.x, bot.pos.y + 0.25f, bot.pos.z };
        Vec3 bodyTop = { bot.pos.x, bot.pos.y + 1.55f, bot.pos.z };
        // Ray vs vertical cylinder
        float dx = eyePos.x - bot.pos.x;
        float dz = eyePos.z - bot.pos.z;
        float A = fwdRay.x * fwdRay.x + fwdRay.z * fwdRay.z;
        float B = 2.0f * (dx * fwdRay.x + dz * fwdRay.z);
        float C = dx * dx + dz * dz - bot.bodyRadius * bot.bodyRadius;
        float discB = B * B - 4.0f * A * C;
        if (discB >= 0.0f && A > 1e-6f) {
            float t = (-B - std::sqrt(discB)) / (2.0f * A);
            if (t > 0.0f && t < closestHitT) {
                float hitY = eyePos.y + t * fwdRay.y;
                if (hitY >= bodyBot.y && hitY <= bodyTop.y) {
                    closestHitT = t;
                    hitBotIdx = static_cast<int>(i);
                    hitRegion = 1; // BODY
                }
            }
        }
    }

    if (hitBotIdx >= 0) {
        auto& bot = m_bots[hitBotIdx];
        bot.alive = false;

        outResult.botName = bot.name;
        outResult.isHeadshot = (hitRegion == 2);
        outResult.isStopped  = isAccurate;
        outResult.killTimeMs = (bot.firstSeenTime > 0.0) ? (now - bot.firstSeenTime) * 1000.0 : 250.0;
        outResult.vertErrDeg = bot.firstSightVertErrDeg;
        outResult.playerTookDamage = bot.firedAtPlayer;

        m_history.push_back(outResult);
        return hitRegion;
    }

    return 0; // miss
}

