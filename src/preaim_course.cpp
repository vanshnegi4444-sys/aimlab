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
        default: return "Custom Map Course";
    }
}

std::string PreAimCourse::mapBinPath() const {
    switch (m_courseId) {
        case MapCourseId::AscentB_Clear:   return "assets/maps/ascentB.bin";
        case MapCourseId::BindB_LongClear: return "assets/maps/bindB.bin";
        case MapCourseId::HavenA_Clear:    return "assets/maps/havenA.bin";
        case MapCourseId::HavenB_Clear:    return "assets/maps/havenB.bin";
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
        default: initAscentB(); break;
    }
}

void PreAimCourse::initAscentB() {
    m_spawnPos = { -20.0f, 1.0f, -50.0f };
    // Faces the first bot (Lane Close Left Corner at -24.5, -58.5):
    // yaw = atan2(-4.5, 8.5) ~ -28 deg. The old -135 deg faced away
    // from every bot, so nothing was visible at spawn.
    m_spawnYaw = -28.0f;

    PreAimBot b;

    b.name = "Lane Close Left Corner";
    b.pos = { -24.5f, 1.0f, -58.5f };
    b.yawDeg = 45.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Green Box Top (Elevated Head Level)";
    b.pos = { -25.0f, 2.2f, -62.0f }; // Elevated! Tests vertical crosshair placement
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

    b.name = "Boathouse Front Entrance";
    b.pos = { -40.0f, 0.0f, -69.0f };
    b.yawDeg = 30.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Boathouse Stairs / Back Alley";
    b.pos = { -52.0f, 1.5f, -71.0f };
    b.yawDeg = 60.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "CT Arch Long Range Peek";
    b.pos = { -4.0f, 1.0f, -88.0f };
    b.yawDeg = 180.0f;
    b.reactionLimit = 0.50;
    m_bots.push_back(b);
}

void PreAimCourse::initBindB() {
    m_spawnPos = { 108.0f, 3.0f, -32.0f };
    // Faces the first bot (Garden Vista at 102, -40):
    // yaw = atan2(-6, 8) ~ -37 deg. The old 180 deg faced away.
    m_spawnYaw = -37.0f;

    PreAimBot b;

    b.name = "Garden Vista Left Angle";
    b.pos = { 102.0f, 3.0f, -40.0f };
    b.yawDeg = 20.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "Hookah Window Long Hold";
    b.pos = { 89.0f, 6.5f, -36.5f }; // Elevated window
    b.yawDeg = 0.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "Site Sand Container Front";
    b.pos = { 115.0f, 3.0f, -48.0f };
    b.yawDeg = 160.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "B Site Center Box";
    b.pos = { 111.0f, 3.0f, -58.0f };
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
    m_spawnPos = { 90.0f, 0.0f, -30.0f };
    // Faces the first bot (A Long Corner Peek at 95, -55):
    // yaw = atan2(5, 25) ~ +11 deg. The old 180 deg faced away.
    m_spawnYaw = 11.0f;

    PreAimBot b;

    b.name = "A Long Corner Peek";
    b.pos = { 95.0f, 0.0f, -55.0f };
    b.yawDeg = 170.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "A Site Default Stack";
    b.pos = { 105.0f, 1.0f, -75.0f };
    b.yawDeg = 160.0f;
    b.reactionLimit = 0.45;
    m_bots.push_back(b);

    b.name = "A Heaven High Window";
    b.pos = { 118.0f, 6.0f, -85.0f }; // Elevated sniper window!
    b.yawDeg = 190.0f;
    b.reactionLimit = 0.40;
    m_bots.push_back(b);

    b.name = "A Hell Under-Window Hold";
    b.pos = { 118.0f, 1.0f, -85.0f };
    b.yawDeg = 180.0f;
    b.reactionLimit = 0.42;
    m_bots.push_back(b);

    b.name = "A Short / Sewers Flank";
    b.pos = { 80.0f, 0.0f, -70.0f };
    b.yawDeg = 90.0f;
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
    m_spawnPos = { 24.0f, 1.0f, -60.0f };
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
    b.pos = { 14.0f, 3.0f, -90.0f };
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

