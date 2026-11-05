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
        case MapCourseId::HavenB_Clear:    return "Haven B: Mid Courtyard to B Site Clearing";
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

void PreAimCourse::start(MapCourseId courseId, double now) {
    m_courseId = courseId;
    m_bots.clear();
    m_history.clear();
    m_activeBotIdx   = -1;
    m_currentVertErr = 0.0f;
    m_damageTaken    = 0;
    m_lastNow        = now;

    switch (courseId) {
        case MapCourseId::AscentB_Clear:   initAscentB(); break;
        case MapCourseId::BindB_LongClear: initBindB();   break;
        case MapCourseId::HavenA_Clear:    initHavenA();  break;
        case MapCourseId::HavenB_Clear:    initHavenB();  break;
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
        float dx = m_spawnPos.x - b.pos.x;
        float dz = m_spawnPos.z - b.pos.z;
        b.yawDeg = degrees(std::atan2(dx, dz));
    }
}

void PreAimCourse::initAscentB() {
    // Inside Ascent B Main room looking directly at the B Main archway entrance
    m_spawnPos = { -12.5f, 1.00f, -57.5f };
    m_spawnYaw = -15.0f;

    PreAimBot b;

    b.name = "B Main Arch Close Left Corner";
    b.pos  = { -18.5f, 0.97f, -67.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Lane Catwalk Hold";
    b.pos  = { -26.0f, 0.99f, -67.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Lane Back / Elevated Platform";
    b.pos  = { -35.0f, 1.50f, -66.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Stairs / Logs Angle";
    b.pos  = { -15.0f, 1.00f, -76.0f };
    b.reactionLimit = 0.52;
    m_bots.push_back(b);

    b.name = "Market Doorway Peeker";
    b.pos  = { -9.5f, 1.00f, -75.5f };
    b.reactionLimit = 0.50;
    m_bots.push_back(b);

    b.name = "B Site Pit Default";
    b.pos  = { -28.0f, -1.00f, -78.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "CT Spawn Arch Long Hold";
    b.pos  = { -13.5f, 1.00f, -92.0f };
    b.reactionLimit = 0.60;
    m_bots.push_back(b);
}

void PreAimCourse::initBindB() {
    // Inside Bind B Garden octagon looking directly through the Garden archway onto B Site
    m_spawnPos = { 92.5f, 3.00f, -55.5f };
    m_spawnYaw = 105.0f;

    PreAimBot b;

    b.name = "Garden Exit Close Right";
    b.pos  = { 96.5f, 3.00f, -52.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "Garden Archway Wide Angle";
    b.pos  = { 103.5f, 3.02f, -53.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "Hookah Window Elevated Hold";
    b.pos  = { 100.5f, 5.00f, -44.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Site Container Front";
    b.pos  = { 110.0f, 3.38f, -48.5f };
    b.reactionLimit = 0.52;
    m_bots.push_back(b);

    b.name = "B Site Back Pillar";
    b.pos  = { 116.5f, 3.01f, -56.5f };
    b.reactionLimit = 0.52;
    m_bots.push_back(b);

    b.name = "B Elbow Corner Hold";
    b.pos  = { 109.0f, 3.02f, -69.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Hall / Hell Deep Angle";
    b.pos  = { 126.5f, 3.00f, -59.5f };
    b.reactionLimit = 0.60;
    m_bots.push_back(b);
}

void PreAimCourse::initHavenA() {
    // At the bottom of Haven A Long ramp looking directly up A Long into A Site
    m_spawnPos = { 66.5f, 1.12f, -72.5f };
    m_spawnYaw = -10.0f;

    PreAimBot b;

    b.name = "A Long Ramp Close Right";
    b.pos  = { 68.0f, 1.85f, -78.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "A Ramp Top Corner";
    b.pos  = { 67.5f, 2.95f, -83.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "A Site Default Box";
    b.pos  = { 60.5f, 3.00f, -85.5f };
    b.reactionLimit = 0.52;
    m_bots.push_back(b);

    b.name = "A Short / Sewers Exit";
    b.pos  = { 53.5f, 3.00f, -79.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "A Hell Under-Window Hold";
    b.pos  = { 49.5f, 3.00f, -88.5f };
    b.reactionLimit = 0.52;
    m_bots.push_back(b);

    b.name = "A Heaven Sniper Window (High)";
    b.pos  = { 45.5f, 6.35f, -83.5f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "CT Connector Archway";
    b.pos  = { 58.0f, 3.00f, -102.0f };
    b.reactionLimit = 0.60;
    m_bots.push_back(b);
}

void PreAimCourse::initHavenB() {
    // In Haven Mid Courtyard looking straight up the steps into B Site & Gong
    m_spawnPos = { 18.0f, 1.00f, -63.0f };
    m_spawnYaw = 0.0f;

    PreAimBot b;

    b.name = "Mid Doors / Garage Right";
    b.pos  = { 25.5f, 0.93f, -64.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Site Left Entry Corner";
    b.pos  = { 11.5f, 3.50f, -83.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Site Right Pillar";
    b.pos  = { 24.5f, 3.00f, -83.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "C Link Archway Hold";
    b.pos  = { 3.5f, 3.00f, -92.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "A Link Doorway Hold";
    b.pos  = { 32.5f, 3.00f, -92.0f };
    b.reactionLimit = 0.55;
    m_bots.push_back(b);

    b.name = "B Back Site Gong";
    b.pos  = { 18.0f, 3.00f, -101.0f };
    b.reactionLimit = 0.60;
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

        if (dist > 65.0f || dist < 0.2f) continue;

        Vec3 dirToHead = toHead * (1.0f / dist);
        float dotFwd   = dot(camFwd, dirToHead);

        // Must be within camera view cone AND have true 3D line-of-sight (unoccluded by walls)
        bool visible = (dotFwd > 0.45f);
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
            }
        }
    }
}

float PreAimCourse::activeBotReactionProgress() const {
    if (m_activeBotIdx < 0 || m_activeBotIdx >= static_cast<int>(m_bots.size())) return 0.0f;
    const auto& b = m_bots[m_activeBotIdx];
    if (!b.hasSeenPlayer || !b.alive || b.reactionLimit <= 1e-4) return 0.0f;
    double elapsed = m_lastNow - b.firstSeenTime;
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
