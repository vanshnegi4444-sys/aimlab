#include "trainer.h"
#include <cmath>
#include <algorithm>

Trainer::Trainer() {
}

Vec3 Trainer::slotPos(int slot) const {
    float spacing = (m_scenarioId == ScenarioId::GridshotLong) ? 2.5f : kSlotSpacing;
    int col = slot % kGridN; // 0, 1, 2
    int row = slot / kGridN; // 0, 1, 2
    return { (col - 1) * spacing,
             (1 - row) * spacing,
             -m_wallDist };
}

int Trainer::randomFreeSlot() {
    int freeSlots[kGridN * kGridN];
    int n = 0;
    for (int s = 0; s < kGridN * kGridN; ++s) {
        bool taken = false;
        for (int i = 0; i < m_activeTargetCount; ++i) {
            if (m_targets[i].alive && m_targets[i].slot == s) {
                taken = true;
                break;
            }
        }
        if (!taken) freeSlots[n++] = s;
    }
    if (n == 0) return 0;
    std::uniform_int_distribution<int> dist(0, n - 1);
    return freeSlots[dist(m_rng)];
}

void Trainer::start(const Settings& s, double now, ScenarioId scenarioId) {
    m_scenarioId = scenarioId;
    const ScenarioInfo& scInfo = getScenarioInfo(scenarioId);

    m_wallDist   = (scenarioId == ScenarioId::GridshotLong) ? scInfo.defaultDist : s.wallDist;
    m_radius     = scInfo.defaultRadius;
    m_duration   = scInfo.defaultDuration;

    m_start      = now;
    m_timeLeft   = m_duration;

    m_score      = 0;
    m_shots      = 0;
    m_hits       = 0;
    m_headshots  = 0;
    m_streak     = 0;
    m_bestStreak = 0;
    m_ttkSum     = 0.0;
    m_ttkCount   = 0;

    m_playerPosX   = 0.0f;
    m_playerVelX   = 0.0f;
    m_stoppedShots = 0;
    m_movingShots  = 0;

    m_overshootCount  = 0;
    m_undershootCount = 0;
    m_reactionSum     = 0.0;
    m_reactionCount   = 0;

    m_trackingOnTargetTime = 0.0;
    m_totalTrackingTime    = 0.0;
    m_isTrackingNow        = false;

    for (int i = 0; i < kMaxTargets; ++i) {
        m_targets[i] = Target{};
    }

    switch (m_scenarioId) {
        case ScenarioId::Gridshot:
        case ScenarioId::GridshotLong:
            initGridshot(now);
            break;
        case ScenarioId::Sixshot:
            initSixshot(now);
            break;
        case ScenarioId::Spidershot:
            initSpidershot(now);
            break;
        case ScenarioId::HeadshotLine:
            initHeadshotLine(now);
            break;
        case ScenarioId::Microshot:
            initMicroshot(now);
            break;
        case ScenarioId::FirstShotPlacement:
            initFirstShotPlacement(now);
            break;
        case ScenarioId::SmoothStrafe:
            initSmoothStrafe(now);
            break;
        case ScenarioId::ValorantStrafe:
            initValorantStrafe(now);
            break;
        case ScenarioId::CounterStrafeDrill:
            initCounterStrafeDrill(now);
            break;
        case ScenarioId::AngleHold:
            initAngleHold(now);
            break;
        case ScenarioId::OffAngleStrafeConfirm:
            initOffAngleStrafeConfirm(now);
            break;
        case ScenarioId::WideFlick180:
            initWideFlick180(now);
            break;
        case ScenarioId::ReactiveTrack:
            initReactiveTrack(now);
            break;
        case ScenarioId::PeekersAdvantage:
            initPeekersAdvantage(now);
            break;
        default:
            initGridshot(now);
            break;
    }
}

void Trainer::initGridshot(double now) {
    m_activeTargetCount = 3;
    for (int i = 0; i < m_activeTargetCount; ++i) {
        int slot = randomFreeSlot();
        Target& t = m_targets[i];
        t.slot      = slot;
        t.center    = slotPos(slot);
        t.radius    = m_radius;
        t.spawnTime = now;
        t.alive     = true;
    }
}

void Trainer::initSixshot(double now) {
    m_activeTargetCount = 6;
    std::uniform_real_distribution<float> distYaw(-22.0f, 22.0f);
    std::uniform_real_distribution<float> distPitch(-10.0f, 10.0f);

    for (int i = 0; i < m_activeTargetCount; ++i) {
        float yRad = radians(distYaw(m_rng));
        float pRad = radians(distPitch(m_rng));
        float cp   = std::cos(pRad);

        Target& t = m_targets[i];
        t.center = {
            m_wallDist * std::tan(yRad),
            m_wallDist * std::sin(pRad) / cp,
            -m_wallDist
        };
        t.radius = m_radius;
        t.spawnTime = now;
        t.alive = true;
    }
}

void Trainer::initSpidershot(double now) {
    m_activeTargetCount = 1;
    m_spidershotCenterTurn = false;
    Target& t = m_targets[0];
    t.center = { 0.0f, 0.0f, -m_wallDist }; // start at center
    t.radius = m_radius;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initHeadshotLine(double now) {
    m_activeTargetCount = 1;
    std::uniform_real_distribution<float> distYaw(-32.0f, 32.0f);
    std::uniform_real_distribution<float> distDist(9.0f, 16.0f);

    float d = distDist(m_rng);
    float yRad = radians(distYaw(m_rng));

    Target& t = m_targets[0];
    t.center = { d * std::sin(yRad), 0.0f, -d * std::cos(yRad) }; // strictly Y=0 (head height)
    t.radius = 0.12f;
    t.spawnTime = now;
    t.alive = true;
    t.hitRegion = 0; // head
    // Gold/yellow highlight for headshot line
    t.colorInner[0] = 1.0f; t.colorInner[1] = 0.85f; t.colorInner[2] = 0.2f;
    t.colorRing[0]  = 0.7f; t.colorRing[1]  = 0.45f; t.colorRing[2] = 0.05f;
}

void Trainer::initMicroshot(double now) {
    m_activeTargetCount = 1;
    std::uniform_real_distribution<float> distOffset(-0.55f, 0.55f);
    Target& t = m_targets[0];
    t.center = { distOffset(m_rng), distOffset(m_rng), -m_wallDist };
    t.radius = 0.06f; // tiny
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initFirstShotPlacement(double now) {
    m_activeTargetCount = 1;
    std::uniform_real_distribution<float> distAngle(0.0f, 6.283f);
    std::uniform_real_distribution<float> distRadius(0.5f, 1.4f);

    float a = distAngle(m_rng);
    float r = distRadius(m_rng);

    Target& t = m_targets[0];
    t.center = { r * std::cos(a), r * std::sin(a), -m_wallDist };
    t.radius = 0.12f;
    t.spawnTime = now;
    t.alive = true;
    t.hitRegion = 0;
}

void Trainer::initSmoothStrafe(double now) {
    m_activeTargetCount = 1;
    Target& t = m_targets[0];
    t.center = { 0.0f, 0.0f, -m_wallDist };
    t.velocity = { 3.8f, 0.0f, 0.0f };
    t.minX = -3.2f;
    t.maxX =  3.2f;
    t.radius = m_radius;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initValorantStrafe(double now) {
    m_activeTargetCount = 1;
    Target& t = m_targets[0];
    t.center = { 0.0f, 0.0f, -m_wallDist };
    t.velocity = { 6.75f, 0.0f, 0.0f }; // Valorant run speed
    t.minX = -2.5f;
    t.maxX =  2.5f;
    t.radius = 0.13f;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initCounterStrafeDrill(double now) {
    m_activeTargetCount = 1;
    m_playerPosX = 0.0f;
    m_playerVelX = 0.0f;

    std::uniform_real_distribution<float> distYaw(-18.0f, 18.0f);
    float yRad = radians(distYaw(m_rng));

    Target& t = m_targets[0];
    t.center = { m_wallDist * std::sin(yRad), 0.0f, -m_wallDist * std::cos(yRad) };
    t.radius = 0.14f;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initAngleHold(double now) {
    m_activeTargetCount = 1;
    m_coverWallX = -1.0f;
    std::uniform_real_distribution<double> delayDist(1.0, 3.2);
    m_nextPeekTime = now + delayDist(m_rng);
    m_botIsHidden = true;

    Target& t = m_targets[0];
    t.center = { m_coverWallX - 1.5f, 0.0f, -m_wallDist }; // hidden behind wall
    t.radius = 0.13f;
    t.spawnTime = now;
    t.alive = false;
    t.isPeeking = false;
}

void Trainer::initOffAngleStrafeConfirm(double now) {
    m_activeTargetCount = 1;
    m_playerPosX = 0.0f;
    m_playerVelX = 0.0f;

    std::uniform_real_distribution<float> signDist(-1.0f, 1.0f);
    float dir = signDist(m_rng) >= 0.0f ? 1.0f : -1.0f;
    // Exactly matches the user's screenshot: 0.85m to 1.6m horizontal gap (2-3 body widths)
    std::uniform_real_distribution<float> offsetDist(0.85f, 1.60f);
    float ox = dir * offsetDist(m_rng);

    Target& t = m_targets[0];
    t.center = { ox, 0.0f, -m_wallDist };
    // Strafe speed: 4.8 m/s (Valorant strafe speed)
    t.velocity = { -dir * 4.8f, 0.0f, 0.0f };
    t.minX = ox - 1.2f;
    t.maxX = ox + 1.2f;
    t.radius = 0.12f;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initWideFlick180(double now) {
    m_activeTargetCount = 1;
    // One target at a wide angle: 45-160 deg to either side, 10 m out.
    // Big arm flicks and fast turns (spec C-05).
    std::uniform_real_distribution<float> sideDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> angDist(45.0f, 160.0f);
    float side = sideDist(m_rng) >= 0.0f ? 1.0f : -1.0f;
    float yRad = radians(side * angDist(m_rng));

    Target& t = m_targets[0];
    t.center = { m_wallDist * std::sin(yRad), 0.0f, -m_wallDist * std::cos(yRad) };
    t.radius = 0.20f;
    t.spawnTime = now;
    t.alive = true;
}

void Trainer::initReactiveTrack(double now) {
    m_activeTargetCount = 1;
    Target& t = m_targets[0];
    t.center = { 0.0f, 0.0f, -m_wallDist };
    t.velocity = { 4.0f, 0.0f, 0.0f };
    t.minX = -3.0f;
    t.maxX =  3.0f;
    t.minY = -1.5f;
    t.maxY =  1.5f;
    t.radius = m_radius;
    t.spawnTime = now;
    t.alive = true;
    m_reactiveNextTurn = now + 0.3;
}

void Trainer::initPeekersAdvantage(double now) {
    m_activeTargetCount = 1;
    m_playerPosX = 0.0f;
    m_playerVelX = 0.0f;
    m_peekBotVisible = false;
    m_peekReactStart = 0.0;
    m_peekWasMoving = false;
    m_peekSide = 1;

    // Bot starts hidden behind cover. It appears when you swing out with
    // A (left) or D (right), and fires back 200 ms after you stop moving.
    Target& t = m_targets[0];
    t.center = { 0.0f, 0.0f, -m_wallDist };
    t.radius = 0.14f;
    t.spawnTime = now;
    t.alive = false;
    t.isPeeking = false;
}

void Trainer::spawnRandomTarget(int idx, double now) {
    Target& t = m_targets[idx];
    switch (m_scenarioId) {
        case ScenarioId::Gridshot:
        case ScenarioId::GridshotLong: {
            int slot = randomFreeSlot();
            t.slot = slot;
            t.center = slotPos(slot);
            t.radius = m_radius;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::Sixshot: {
            std::uniform_real_distribution<float> distYaw(-22.0f, 22.0f);
            std::uniform_real_distribution<float> distPitch(-10.0f, 10.0f);
            float yRad = radians(distYaw(m_rng));
            float pRad = radians(distPitch(m_rng));
            float cp   = std::cos(pRad);
            t.center = { m_wallDist * std::tan(yRad), m_wallDist * std::sin(pRad) / cp, -m_wallDist };
            t.radius = m_radius;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::Spidershot: {
            m_spidershotCenterTurn = !m_spidershotCenterTurn;
            if (m_spidershotCenterTurn) {
                t.center = { 0.0f, 0.0f, -m_wallDist };
            } else {
                std::uniform_real_distribution<float> distYaw(-26.0f, 26.0f);
                std::uniform_real_distribution<float> distPitch(-10.0f, 10.0f);
                float yRad = radians(distYaw(m_rng));
                float pRad = radians(distPitch(m_rng));
                t.center = { m_wallDist * std::tan(yRad), m_wallDist * std::tan(pRad), -m_wallDist };
            }
            t.radius = m_radius;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::HeadshotLine: {
            std::uniform_real_distribution<float> distYaw(-32.0f, 32.0f);
            std::uniform_real_distribution<float> distDist(9.0f, 16.0f);
            float d = distDist(m_rng);
            float yRad = radians(distYaw(m_rng));
            t.center = { d * std::sin(yRad), 0.0f, -d * std::cos(yRad) };
            t.radius = 0.12f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::Microshot: {
            std::uniform_real_distribution<float> distOffset(-0.65f, 0.65f);
            float ox = t.center.x + distOffset(m_rng);
            float oy = t.center.y + distOffset(m_rng);
            ox = std::clamp(ox, -2.5f, 2.5f);
            oy = std::clamp(oy, -2.0f, 2.0f);
            t.center = { ox, oy, -m_wallDist };
            t.radius = 0.06f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::FirstShotPlacement: {
            std::uniform_real_distribution<float> distAngle(0.0f, 6.283f);
            std::uniform_real_distribution<float> distRadius(0.5f, 1.4f);
            float a = distAngle(m_rng);
            float r = distRadius(m_rng);
            t.center = { r * std::cos(a), r * std::sin(a), -m_wallDist };
            t.radius = 0.12f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::CounterStrafeDrill: {
            std::uniform_real_distribution<float> distYaw(-18.0f, 18.0f);
            float yRad = radians(distYaw(m_rng));
            t.center = { m_wallDist * std::sin(yRad), 0.0f, -m_wallDist * std::cos(yRad) };
            t.radius = 0.14f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::AngleHold: {
            std::uniform_real_distribution<double> delayDist(1.2, 3.5);
            m_nextPeekTime = now + delayDist(m_rng);
            m_botIsHidden = true;
            t.center = { m_coverWallX - 1.5f, 0.0f, -m_wallDist };
            t.alive = false;
            t.isPeeking = false;
            break;
        }
        case ScenarioId::OffAngleStrafeConfirm: {
            std::uniform_real_distribution<float> signDist(-1.0f, 1.0f);
            float dir = signDist(m_rng) >= 0.0f ? 1.0f : -1.0f;
            std::uniform_real_distribution<float> offsetDist(0.85f, 1.60f);
            float ox = dir * offsetDist(m_rng);

            t.center = { ox, 0.0f, -m_wallDist };
            t.velocity = { -dir * 4.8f, 0.0f, 0.0f };
            t.minX = ox - 1.2f;
            t.maxX = ox + 1.2f;
            t.radius = 0.12f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::WideFlick180: {
            std::uniform_real_distribution<float> sideDist(-1.0f, 1.0f);
            std::uniform_real_distribution<float> angDist(45.0f, 160.0f);
            float side = sideDist(m_rng) >= 0.0f ? 1.0f : -1.0f;
            float yRad = radians(side * angDist(m_rng));
            t.center = { m_wallDist * std::sin(yRad), 0.0f, -m_wallDist * std::cos(yRad) };
            t.radius = 0.20f;
            t.spawnTime = now;
            t.alive = true;
            break;
        }
        case ScenarioId::PeekersAdvantage: {
            // Rep done: hide the bot until the next peek.
            t.alive = false;
            t.isPeeking = false;
            m_peekBotVisible = false;
            m_peekReactStart = 0.0;
            m_peekWasMoving = false;
            break;
        }
        default:
            t.center = { 0.0f, 0.0f, -m_wallDist };
            t.alive = true;
            break;
    }
}

void Trainer::update(double now, double dt, const Camera& cam, bool movingLeft, bool movingRight) {
    m_timeLeft = m_duration - (now - m_start);
    if (m_timeLeft < 0.0) m_timeLeft = 0.0;
    if (m_timeLeft <= 0.0) return;

    // 1. Player movement physics (Valorant: 6.75 m/s max speed, 45 m/s^2 accel, 60 m/s^2 braking)
    if (getScenarioInfo(m_scenarioId).allowPlayerMove) {
        float targetDir = 0.0f;
        if (movingLeft)  targetDir -= 1.0f;
        if (movingRight) targetDir += 1.0f;

        float maxSpeed = 6.75f;
        float accel    = 42.0f;
        float brake    = 58.0f;

        if (targetDir != 0.0f) {
            float desiredVel = targetDir * maxSpeed;
            if ((desiredVel > 0 && m_playerVelX < desiredVel) ||
                (desiredVel < 0 && m_playerVelX > desiredVel)) {
                float step = accel * static_cast<float>(dt);
                if (desiredVel > m_playerVelX)
                    m_playerVelX = std::min(m_playerVelX + step, desiredVel);
                else
                    m_playerVelX = std::max(m_playerVelX - step, desiredVel);
            }
        } else {
            // Decelerating / counter-strafing to stop
            float step = brake * static_cast<float>(dt);
            if (m_playerVelX > 0.0f)
                m_playerVelX = std::max(0.0f, m_playerVelX - step);
            else if (m_playerVelX < 0.0f)
                m_playerVelX = std::min(0.0f, m_playerVelX + step);
        }
        m_playerPosX = std::clamp(m_playerPosX + m_playerVelX * static_cast<float>(dt), -6.0f, 6.0f);
    }

    // 2. Scenario specific target movement & behaviors
    if (m_scenarioId == ScenarioId::SmoothStrafe) {
        Target& t = m_targets[0];
        t.center.x += t.velocity.x * static_cast<float>(dt);
        if (t.center.x > t.maxX) {
            t.center.x = t.maxX;
            t.velocity.x = -std::abs(t.velocity.x);
        } else if (t.center.x < t.minX) {
            t.center.x = t.minX;
            t.velocity.x = std::abs(t.velocity.x);
        }
    } else if (m_scenarioId == ScenarioId::ValorantStrafe) {
        Target& t = m_targets[0];
        t.center.x += t.velocity.x * static_cast<float>(dt);
        // Random rapid direction changes (0.2s - 0.5s interval)
        if (now - t.spawnTime > 0.35) {
            std::uniform_real_distribution<float> flipDist(0.0f, 1.0f);
            if (flipDist(m_rng) < 0.45f) {
                t.velocity.x = -t.velocity.x;
                t.spawnTime = now;
            }
        }
        if (t.center.x > t.maxX) {
            t.center.x = t.maxX;
            t.velocity.x = -6.75f;
            t.spawnTime = now;
        } else if (t.center.x < t.minX) {
            t.center.x = t.minX;
            t.velocity.x = 6.75f;
            t.spawnTime = now;
        }
    } else if (m_scenarioId == ScenarioId::AngleHold) {
        Target& t = m_targets[0];
        if (m_botIsHidden && now >= m_nextPeekTime) {
            // Bot swings out from behind the 3D cover wall at Valorant run speed (6.75 m/s)!
            m_botIsHidden = false;
            t.center = { m_coverWallX - 0.25f, 0.0f, -m_wallDist };
            t.velocity = { 6.75f, 0.0f, 0.0f };
            t.maxX = m_coverWallX + 0.85f;
            t.alive = true;
            t.isPeeking = true;
            t.peekTime = now;
            t.spawnTime = now;
            t.peekDuration = 0.58; // 160ms swing + 420ms reaction window
        } else if (!m_botIsHidden && t.isPeeking) {
            if (t.center.x < t.maxX) {
                t.center.x = std::min(t.maxX, t.center.x + t.velocity.x * static_cast<float>(dt));
            }
            if (now - t.peekTime > t.peekDuration) {
                // Player failed to react in time! Bot shot player / escaped
                t.alive = false;
                t.isPeeking = false;
                m_streak = 0;
                spawnRandomTarget(0, now);
            }
        }
    } else if (m_scenarioId == ScenarioId::OffAngleStrafeConfirm) {
        Target& t = m_targets[0];
        t.center.x += t.velocity.x * static_cast<float>(dt);
        // Switch direction at strafe apex every 0.35s (mimicking Valorant AD counter-strafe)
        if (now - t.spawnTime > 0.35) {
            t.velocity.x = -t.velocity.x;
            t.spawnTime = now;
        }
        if (t.center.x > t.maxX) {
            t.center.x = t.maxX;
            t.velocity.x = -std::abs(t.velocity.x);
        } else if (t.center.x < t.minX) {
            t.center.x = t.minX;
            t.velocity.x = std::abs(t.velocity.x);
        }
    } else if (m_scenarioId == ScenarioId::ReactiveTrack) {
        Target& t = m_targets[0];
        // Random direction change every 0.15-0.5 s. Spec calls for a
        // Perlin-noise path; uniform random re-aim is the v1 version.
        if (now >= m_reactiveNextTurn) {
            std::uniform_real_distribution<float> angDist(0.0f, 6.283f);
            std::uniform_real_distribution<float> intDist(0.15f, 0.5f);
            float a = angDist(m_rng);
            float sp = 4.0f;
            t.velocity = { sp * std::cos(a), sp * std::sin(a), 0.0f };
            m_reactiveNextTurn = now + static_cast<double>(intDist(m_rng));
        }
        t.center.x += t.velocity.x * static_cast<float>(dt);
        t.center.y += t.velocity.y * static_cast<float>(dt);
        if (t.center.x > t.maxX) {
            t.center.x = t.maxX;
            t.velocity.x = -std::abs(t.velocity.x);
        } else if (t.center.x < t.minX) {
            t.center.x = t.minX;
            t.velocity.x = std::abs(t.velocity.x);
        }
        if (t.center.y > t.maxY) {
            t.center.y = t.maxY;
            t.velocity.y = -std::abs(t.velocity.y);
        } else if (t.center.y < t.minY) {
            t.center.y = t.minY;
            t.velocity.y = std::abs(t.velocity.y);
        }
    } else if (m_scenarioId == ScenarioId::PeekersAdvantage) {
        Target& t = m_targets[0];
        if (!m_peekBotVisible) {
            // Commit to a peek: the bot appears at the angle on the side
            // you swung toward. (The swing itself is your A/D movement;
            // the movement sim above drives m_playerVelX.)
            if (movingLeft || movingRight) {
                m_peekSide = movingRight ? 1 : -1;
                float yawDeg = 25.0f * static_cast<float>(m_peekSide);
                float yRad = radians(yawDeg);
                float d = 12.0f;
                t.center = { d * std::sin(yRad), 0.0f, -d * std::cos(yRad) };
                t.alive = true;
                t.isPeeking = true;
                t.peekTime = now;
                t.peekDuration = 0.20; // bot fires 200 ms after you stop
                m_peekBotVisible = true;
                m_peekReactStart = 0.0;
                m_peekWasMoving = false;
            }
        } else {
            // Bot is out. Its 200 ms reaction window starts once you have
            // stopped moving (counter-strafed to a stop) AFTER the swing.
            // (m_peekWasMoving guards against the window starting during
            // the first frames of the swing, when speed is still < 1.85.)
            if (!isPlayerStopped()) {
                m_peekWasMoving = true;
            } else if (m_peekWasMoving) {
                if (m_peekReactStart <= 0.0) m_peekReactStart = now;
                if (now - m_peekReactStart > t.peekDuration) {
                    // Too slow: the bot shoots first, rep resets.
                    t.alive = false;
                    t.isPeeking = false;
                    m_peekBotVisible = false;
                    m_peekReactStart = 0.0;
                    m_peekWasMoving = false;
                    m_streak = 0;
                }
            }
            // Swing back with the opposite key to hide (re-hide).
            bool swingingBack =
                (m_peekSide > 0 && movingLeft) || (m_peekSide < 0 && movingRight);
            if (swingingBack) {
                t.alive = false;
                t.isPeeking = false;
                m_peekBotVisible = false;
                m_peekReactStart = 0.0;
                m_peekWasMoving = false;
            }
        }
    }

    // 3. Tracking evaluation
    if (getScenarioInfo(m_scenarioId).isTracking) {
        m_totalTrackingTime += dt;
        Vec3 eye = cam.eye();
        Vec3 dir = cam.forward();
        const Target& t = m_targets[0];

        Vec3 toC = t.center - eye;
        float tt = dot(toC, dir);
        bool onTarget = false;
        if (tt > 0.0f) {
            Vec3 p = eye + dir * tt;
            if (length(p - t.center) <= t.radius) {
                onTarget = true;
            }
        }

        m_isTrackingNow = onTarget;
        if (onTarget) {
            m_trackingOnTargetTime += dt;
            m_score = static_cast<int>(m_trackingOnTargetTime * 250.0);
        }
    }
}

int Trainer::shoot(double now, const Camera& cam) {
    if (getScenarioInfo(m_scenarioId).isTracking) return 0; // tracking doesn't use click

    ++m_shots;

    // Movement accuracy rule for M-01 Counter-Strafe Drill
    bool stopped = isPlayerStopped();
    if (getScenarioInfo(m_scenarioId).allowPlayerMove) {
        if (stopped) ++m_stoppedShots;
        else ++m_movingShots;
    }

    Vec3 eye = cam.eye();
    Vec3 dir = cam.forward();

    // If moving in movement drills, add heavy shot spread!
    // (Peeker's Advantage counts too: you must stop after the swing.)
    if ((m_scenarioId == ScenarioId::CounterStrafeDrill || m_scenarioId == ScenarioId::OffAngleStrafeConfirm || m_scenarioId == ScenarioId::PeekersAdvantage) && !stopped) {
        std::uniform_real_distribution<float> spreadDist(-0.065f, 0.065f);
        dir.x += spreadDist(m_rng);
        dir.y += spreadDist(m_rng);
        dir = normalize(dir);
        m_streak = 0;
        return -1; // Moving shot whiff
    }

    int   hitIdx = -1;
    float hitT   = 0.0f;
    float hitDistFromCenter = 0.0f;
    float targetRad = m_radius;
    int   hitRegion = 0; // 2 = head, 1 = body

    bool humanoid = (m_scenarioId == ScenarioId::HeadshotLine ||
                     m_scenarioId == ScenarioId::CounterStrafeDrill ||
                     m_scenarioId == ScenarioId::AngleHold ||
                     m_scenarioId == ScenarioId::OffAngleStrafeConfirm);

    for (int i = 0; i < m_activeTargetCount; ++i) {
        const Target& t = m_targets[i];
        if (!t.alive) continue;

        float effRad = humanoid ? std::max(t.radius, 0.15f) : t.radius;

        Vec3  toC = t.center - eye;
        float tt  = dot(toC, dir);
        if (tt <= 0.0f) continue;

        Vec3  p = eye + dir * tt;
        float d = length(p - t.center);
        if (d <= effRad && (hitIdx < 0 || tt < hitT)) {
            hitIdx = i;
            hitT   = tt;
            hitDistFromCenter = d;
            targetRad = effRad;
            hitRegion = 2; // Head / center hit
        } else if (humanoid) {
            // Check 3D Jett body cylinder (y in [center.y - 1.47, center.y - 0.15], radius 0.24m)
            float dx = p.x - t.center.x;
            float dy = p.y - t.center.y;
            if (std::abs(dx) <= 0.24f && dy >= -1.47f && dy <= -0.15f && (hitIdx < 0 || tt < hitT)) {
                hitIdx = i;
                hitT   = tt;
                hitDistFromCenter = std::abs(dx);
                targetRad = 0.24f;
                hitRegion = 1; // Body shot
            }
        }
    }

    // Measure overshoot vs undershoot on primary target
    if (m_activeTargetCount > 0 && m_targets[0].alive) {
        const Target& t0 = m_targets[0];
        Vec3 toC = t0.center - eye;
        float tt = dot(toC, dir);
        if (tt > 0.0f) {
            Vec3 p = eye + dir * tt;
            // Crosshair horizontal offset relative to target center
            float dx = p.x - t0.center.x;
            if (std::abs(dx) > t0.radius * 0.2f) {
                if (dx > 0.0f) ++m_overshootCount;
                else ++m_undershootCount;
            }
        }
    }

    if (hitIdx >= 0) {
        Target& t = m_targets[hitIdx];
        ++m_hits;
        ++m_streak;
        if (m_streak > m_bestStreak) m_bestStreak = m_streak;

        double ttk = now - t.spawnTime;
        m_ttkSum += ttk;
        ++m_ttkCount;

        bool isHead = humanoid ? (hitRegion == 2) : ((t.hitRegion == 0) || (hitDistFromCenter <= targetRad * 0.45f));
        if (isHead) ++m_headshots;

        // Scoring: base hit + headshot bonus + speed bonus
        int pts = 100;
        if (isHead) pts += 50;
        if (ttk < 0.35) pts += 40;
        else if (ttk < 0.60) pts += 20;

        m_score += pts;

        // Respawn / next target
        spawnRandomTarget(hitIdx, now);
        return isHead ? 2 : 1;
    } else {
        m_streak = 0;
        // In First-Shot Placement, a miss immediately resets target!
        if (m_scenarioId == ScenarioId::FirstShotPlacement) {
            if (m_score > 30) m_score -= 30;
            spawnRandomTarget(0, now);
        }
        return 0; // Miss
    }
}

float Trainer::accuracy() const {
    if (getScenarioInfo(m_scenarioId).isTracking) {
        return trackingAccuracy();
    }
    return m_shots > 0 ? (100.0f * m_hits / m_shots) : 0.0f;
}

double Trainer::avgTtkMs() const {
    return m_ttkCount > 0 ? (m_ttkSum / m_ttkCount) * 1000.0 : 0.0;
}

float Trainer::stopAccuracy() const {
    int total = m_stoppedShots + m_movingShots;
    return total > 0 ? (100.0f * m_stoppedShots / total) : 100.0f;
}

float Trainer::trackingAccuracy() const {
    return m_totalTrackingTime > 0.0 ?
        static_cast<float>((m_trackingOnTargetTime / m_totalTrackingTime) * 100.0) : 0.0f;
}

float Trainer::overshootPct() const {
    int total = m_overshootCount + m_undershootCount;
    return total > 0 ? (100.0f * m_overshootCount / total) : 50.0f;
}

float Trainer::undershootPct() const {
    int total = m_overshootCount + m_undershootCount;
    return total > 0 ? (100.0f * m_undershootCount / total) : 50.0f;
}

double Trainer::avgReactionMs() const {
    return m_reactionCount > 0 ? (m_reactionSum / m_reactionCount) * 1000.0 : 210.0;
}

std::string Trainer::generateDiagnosticAdvice() const {
    if (m_scenarioId == ScenarioId::CounterStrafeDrill) {
        float stopAcc = stopAccuracy();
        if (stopAcc < 75.0f) {
            return "Crucial Leak Detected: You fired while still moving on " +
                   std::to_string(static_cast<int>(100.0f - stopAcc)) +
                   "% of shots! In Valorant this creates wild bullet spread. Tap opposite key to stop before clicking.";
        } else if (stopAcc < 90.0f) {
            return "Good Counter-Strafing (" + std::to_string(static_cast<int>(stopAcc)) +
                   "% stopped). Clean up the remaining rush to turn -10 DD Delta positive.";
        } else {
            return "Excellent Counter-Strafing (" + std::to_string(static_cast<int>(stopAcc)) +
                   "% stopped)! Your first bullets will be laser accurate in ranked.";
        }
    }

    if (m_scenarioId == ScenarioId::HeadshotLine) {
        float acc = accuracy();
        if (acc >= 85.0f) {
            return "Superb Headshot Discipline (" + std::to_string(static_cast<int>(acc)) +
                   "% hits)! Keeping your crosshair at this exact horizon height cures your 71.5% body hit rate.";
        } else {
            return "Crosshair Dip Warning: Keep your eye locked to head level. Avoid dipping downwards while flicking horizontally.";
        }
    }

    if (m_scenarioId == ScenarioId::FirstShotPlacement) {
        float acc = accuracy();
        if (acc >= 75.0f) {
            return "Clean One-Tap Discipline (" + std::to_string(static_cast<int>(acc)) +
                   "% hits)! This patience wins opening Vandal duels and raises ADR.";
        } else {
            return "Rushing the Shot: Micro-adjust all the way to center before clicking. One confirmed shot beats two rushed misses.";
        }
    }

    // Default flick diagnostics (Overshoot vs Undershoot)
    float over = overshootPct();
    if (over > 65.0f) {
        return "Over-Flicking (" + std::to_string(static_cast<int>(over)) +
               "% overshoots). You are snapping past targets. Relax hand grip or slightly lower sens by 0.02.";
    } else if (over < 35.0f) {
        return "Under-Flicking (" + std::to_string(static_cast<int>(100.0f - over)) +
               "% undershoots). Flicks are stopping short. Commit more wrist/arm follow-through.";
    } else {
        return "Balanced Snap Dynamics (" + std::to_string(static_cast<int>(over)) +
               "% overshoots). Excellent muscular deceleration control!";
    }
}
