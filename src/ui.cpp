#include "ui.h"
#include "trainer.h"
#include "preaim_course.h"
#include "player.h"
#include <imgui.h>
#include <cmath>
#include <cstdio>

void ui::drawMenu(Settings& s, UiRequest& req, const RoutineManager& routine, int bestScore) {
    (void)routine;
    ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620, 720), ImGuiCond_FirstUseEver);
    ImGui::Begin("ValoAim - Valorant Aim Trainer & Routine Coach", nullptr, ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.7f, 1.0f), "Player: V9pujari#0000 | Peak: Plat 2 | High Score: %d", bestScore);
    ImGui::Separator();

    if (ImGui::BeginTabBar("MainTabs")) {
        // -------------------------------------------------------------------
        // TAB 1: 3D MAP PRE-AIM WORKSHOPS (SHEEPU AIM STYLE)
        // -------------------------------------------------------------------
        if (ImGui::BeginTabItem("3D Map Workshops")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "REAL VALORANT MAP PRE-AIM & CROSSHAIR LEVEL WORKSHOPS");
            ImGui::TextWrapped(
                "Directly extracted from Valorant maps with full 3D geometry and Jett humanoid bots.\n"
                "Practice corner-clearing, angle-slicing, elevated box peeks, and exact head level placement."
            );
            ImGui::Spacing();

            const auto& scenarios = getAllScenarios();
            for (size_t i = 0; i < scenarios.size(); ++i) {
                const auto& sc = scenarios[i];
                if (!isMapPreAim(sc.id)) continue;
                bool isSelected = (s.activeScenario == sc.id);

                char label[128];
                std::snprintf(label, sizeof(label), "[%s] %s", sc.code, sc.name);

                if (ImGui::Selectable(label, isSelected)) {
                    s.activeScenario = sc.id;
                }

                if (isSelected) {
                    ImGui::Indent();
                    ImGui::TextWrapped("%s", sc.description);
                    ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "Crosshair Benefit: %s", sc.trackerFix);
                    ImGui::TextDisabled("Includes WASD corner navigation, counter-strafe deadzones & bot return-fire timer.");
                    ImGui::Unindent();
                    ImGui::Spacing();
                }
            }

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
            if (ImGui::Button("START 3D MAP PRE-AIM WORKSHOP", ImVec2(-1.0f, 52.0f))) {
                req.start = true;
            }
            ImGui::PopStyleColor(2);

            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------------
        // TAB 2: V9PUJARI ROUTINE
        // -------------------------------------------------------------------
        if (ImGui::BeginTabItem("V9pujari Daily Routine")) {
            ImGui::Spacing();
            ImGui::TextWrapped(
                "Custom 6-block training program engineered directly from your tracker.gg stats.\n"
                "Designed to eliminate your negative DD Delta (-10) and 71.5% body hit rate."
            );
            ImGui::Spacing();

            if (ImGui::BeginTable("TrackerProfileTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Your Ranked Metric");
                ImGui::TableSetupColumn("Current Value");
                ImGui::TableSetupColumn("6-Week Target");
                ImGui::TableHeadersRow();

                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("DD Delta / Round (Duel Timing)"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "-10 (Bottom 28%%)"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "+10 Positive");

                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("Body Hit Distribution"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "71.5%% Body Hits"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "< 66.0%% Body");

                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("Headshot Accuracy"); ImGui::TableNextColumn();
                ImGui::Text("22.3%% (8%%-38%% swings)"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "26.0%% (Consistent)");

                ImGui::TableNextRow(); ImGui::TableNextColumn();
                ImGui::Text("Damage Per Round (ADR)"); ImGui::TableNextColumn();
                ImGui::Text("134.9 ADR (0.99 K/D)"); ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "150+ ADR (1.08 K/D)");

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Text("The 6 Routine Blocks (18 sets total, ~25 mins):");
            ImGui::BulletText("Block 1 (2 sets): Gridshot Warmup - Stabilize hand rhythm & remove whiff swings");
            ImGui::BulletText("Block 2 (4 sets): Headshot Line - Strict horizontal crosshair discipline at head height");
            ImGui::BulletText("Block 3 (3 sets): Counter-Strafe Drill - Stop (<1.85 m/s) before shooting to fix DD Delta");
            ImGui::BulletText("Block 4 (3 sets): First-Shot Placement - One-bullet confirmation for Vandal duels");
            ImGui::BulletText("Block 5 (4 sets): Angle Hold & Peeker's Advantage - Defensive & attacking duel timing");
            ImGui::BulletText("Block 6 (2 sets): Duel Tracking - Smooth lock on AD-strafing opponents");

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.65f, 0.45f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.8f, 0.55f, 1.0f));
            if (ImGui::Button("START V9PUJARI DAILY ROUTINE", ImVec2(-1.0f, 52.0f))) {
                req.startRoutine = true;
            }
            ImGui::PopStyleColor(2);

            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------------
        // TAB 3: RANGE DRILLS BROWSER
        // -------------------------------------------------------------------
        if (ImGui::BeginTabItem("Range Drills")) {
            ImGui::Spacing();
            ImGui::Text("Choose an individual clicking/tracking drill to practice:");

            const auto& scenarios = getAllScenarios();

            for (size_t i = 0; i < scenarios.size(); ++i) {
                const auto& sc = scenarios[i];
                if (isMapPreAim(sc.id)) continue;
                bool isSelected = (s.activeScenario == sc.id);

                char label[128];
                std::snprintf(label, sizeof(label), "[%s] %s (%s)", sc.code, sc.name, sc.categoryName);

                if (ImGui::Selectable(label, isSelected)) {
                    s.activeScenario = sc.id;
                }

                if (isSelected) {
                    ImGui::Indent();
                    ImGui::TextWrapped("%s", sc.description);
                    ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "Tracker Benefit: %s", sc.trackerFix);
                    ImGui::TextDisabled("Duration: %.0fs | Default Target Radius: %.2fm", sc.defaultDuration, sc.defaultRadius);
                    ImGui::Unindent();
                    ImGui::Spacing();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.45f, 0.85f, 1.0f));
            if (ImGui::Button("START SELECTED DRILL", ImVec2(-1.0f, 48.0f))) {
                req.start = true;
            }
            ImGui::PopStyleColor();

            ImGui::EndTabItem();
        }

        // -------------------------------------------------------------------
        // TAB 4: SETTINGS & MOUSE FEEL
        // -------------------------------------------------------------------
        if (ImGui::BeginTabItem("Settings & Mouse Feel")) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.4f, 0.9f, 1.0f, 1.0f), "Valorant Sensitivity Engine");
            ImGui::SliderFloat("In-Game Sens", &s.sensitivity, 0.01f, 2.0f, "%.3f");
            ImGui::SliderFloat("Mouse DPI", &s.dpi, 200.0f, 3200.0f, "%.0f");

            float edpi = s.eDpi();
            float cm = s.cmPer360();
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f),
                "eDPI: %.1f   |   cm/360: %.1f cm  (%.1f in/360)",
                edpi, cm, cm / 2.54f);
            ImGui::TextDisabled(
                "Valorant Pro Benchmark: 200-400 eDPI (30-65 cm/360).\n"
                "ValoAim matches Valorant's exact 0.07 deg/count rotation."
            );

            ImGui::Separator();
            ImGui::Text("Crosshair Editor");
            ImGui::SliderFloat("Size (px)", &s.crosshairSize, 4.0f, 30.0f, "%.0f");
            ImGui::SliderFloat("Gap (px)", &s.crosshairGap, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat("Thickness (px)", &s.crosshairThickness, 1.0f, 6.0f, "%.0f");
            ImGui::Checkbox("Center Dot", &s.centerDot);
            ImGui::ColorEdit4("Crosshair Color", s.crosshairColor);

            ImGui::Separator();
            ImGui::Text("Audio & Feedback");
            ImGui::Checkbox("Procedural Hit Sound (Low Latency)", &s.enableHitSound);
            if (s.enableHitSound) {
                ImGui::SliderFloat("Hit Sound Volume", &s.hitSoundVolume, 0.1f, 1.0f, "%.2f");
            }

            ImGui::Separator();
            ImGui::Text("Display & Performance");
            ImGui::Checkbox("VSync (adds display lag - keep OFF)", &s.vsync);
            ImGui::Checkbox("Show In-Game HUD Overlay", &s.showOverlay);

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    ImGui::TextDisabled("Controls: Left Click = Shoot | [R] = Quick Restart Round | [ESC] = Pause / Menu");
    ImGui::End();
}

void ui::drawPlayOverlay(const Trainer& t, const Settings& s, const RoutineManager& routine) {
    (void)s;
    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoBackground;
    ImGui::Begin("HUD", nullptr, flags);

    if (routine.isActive()) {
        const auto& item = routine.currentItem();
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f),
            "[V9PUJARI ROUTINE] %s | Set %d of %d",
            item.blockName, item.setIndex, item.totalSetsInBlock);
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.6f, 1.0f), "FOCUS: %s", item.focusCue);
        ImGui::Spacing();
    } else {
        const auto& sc = t.info();
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[%s] %s", sc.code, sc.name);
    }

    int tl = static_cast<int>(std::ceil(t.timeLeft()));
    if (t.info().isTracking) {
        bool tracking = t.isCurrentlyTracking();
        ImGui::TextColored(tracking ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.2f, 1.0f),
            "Time %02d:%02d   Score %d   Track On-Target: %.1f%%",
            tl / 60, tl % 60, t.score(), t.accuracy());
    } else {
        ImGui::Text("Time %02d:%02d   Score %d   Acc %.1f%%   Streak %d",
            tl / 60, tl % 60, t.score(), t.accuracy(), t.streak());
    }

    if (t.info().allowPlayerMove) {
        float speed = t.playerSpeed();
        bool stopped = t.isPlayerStopped();
        if (stopped) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
                "SPEED: %.2f m/s  [STOPPED - LASER ACCURATE]", speed);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f),
                "SPEED: %.2f m/s  [MOVING - SPREAD INACCURACY]", speed);
        }
    }

    ImGui::TextDisabled("[R] Restart round  |  [ESC] Menu");
    ImGui::End();
}

void ui::drawPreAimOverlay(const PreAimCourse& course, const Settings& /*s*/, const Player& player, double now) {
    ImGui::SetNextWindowPos(ImVec2(16, 16), ImGuiCond_Always);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoBackground;
    ImGui::Begin("PreAimHUD", nullptr, flags);

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "[3D MAP WORKSHOP] %s", course.courseName().c_str());
    ImGui::Text("Bots Cleared: %d / %d   |   Damage Taken: %d",
        course.botsCleared(), course.totalBots(), course.damageTakenCount());

    // Spike timer (defuse courses, 2026-11-06 tactical round): red under
    // 10 s so the player feels the plant pressure.
    if (course.hasTimeLimit()) {
        double t = course.timeRemaining(now);
        ImVec4 col = t < 10.0 ? ImVec4(1.0f, 0.25f, 0.25f, 1.0f)
                              : ImVec4(1.0f, 0.85f, 0.2f, 1.0f);
        ImGui::TextColored(col, "SPIKE: %.0f s to detonation", t);
    }
    // Damage limit (clutch courses): 3 hits = round lost.
    if (course.maxDamage() > 0) {
        ImGui::Text("Damage: %d / %d hits  (round lost at %d)",
            course.damageTakenCount(), course.maxDamage(), course.maxDamage());
    }
    // Headshots-only tag (crosshair drill): body shots do not kill.
    if (course.isHeadshotsOnly()) {
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f),
            "HEADSHOTS ONLY — body shots do not kill (wasted: %d)",
            course.bodyHitsWasted());
    }
    ImGui::Spacing();

    // Crosshair Level Indicator
    if (course.hasActiveTarget()) {
        float err = course.activeBotVertAngleError();
        ImGui::TextColored(ImVec4(0.3f, 0.85f, 1.0f, 1.0f), "ACTIVE ANGLE: %s", course.activeBotName().c_str());

        if (std::abs(err) <= 1.0f) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "CROSSHAIR LEVEL: PERFECT (%.1f deg)", err);
        } else if (err < -1.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.2f, 1.0f), "CROSSHAIR LEVEL: TOO LOW (%.1f deg - AIM HIGHER!)", err);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "CROSSHAIR LEVEL: TOO HIGH (%.1f deg)", err);
        }
        float reactProg = course.activeBotReactionProgress(now);
        ImGui::ProgressBar(reactProg, ImVec2(240.0f, 12.0f), reactProg < 1.0f ? "Bot Reaction Timer" : "BOT FIRED!");
    } else {
        ImGui::TextDisabled("Slice corners with [A]/[D] to engage the next angle.");
    }

    // Speedometer
    float spd = player.speed();
    if (player.isAccurate()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "SPEED: %.2f m/s  [STOPPED - 100%% ACCURATE]", spd);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "SPEED: %.2f m/s  [MOVING - SPREAD PENALTY!]", spd);
    }

    ImGui::Spacing();
    ImGui::TextDisabled("WASD: Move | Shift: Walk | Ctrl: Crouch | Space: Jump | [R] Restart | [ESC] Menu");
    ImGui::End();
}

void ui::drawCrosshair(const Settings& s, const Trainer& t, float screenW, float screenH) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 c(screenW * 0.5f, screenH * 0.5f);

    bool isMovingInMovementDrill = t.info().allowPlayerMove && !t.isPlayerStopped();

    ImU32 col;
    if (isMovingInMovementDrill) {
        col = IM_COL32(255, 60, 60, 240);
    } else {
        col = ImGui::ColorConvertFloat4ToU32(
            ImVec4(s.crosshairColor[0], s.crosshairColor[1],
                   s.crosshairColor[2], s.crosshairColor[3]));
    }

    float gap = isMovingInMovementDrill ? (s.crosshairGap * 2.2f + 4.0f) : s.crosshairGap;
    float len = s.crosshairSize;
    float th  = s.crosshairThickness;

    dl->AddLine(ImVec2(c.x - gap - len, c.y), ImVec2(c.x - gap, c.y), col, th);
    dl->AddLine(ImVec2(c.x + gap, c.y), ImVec2(c.x + gap + len, c.y), col, th);
    dl->AddLine(ImVec2(c.x, c.y - gap - len), ImVec2(c.x, c.y - gap), col, th);
    dl->AddLine(ImVec2(c.x, c.y + gap), ImVec2(c.x, c.y + gap + len), col, th);

    if (s.centerDot) {
        dl->AddCircleFilled(c, th * 0.9f, col);
    }
}

void ui::drawPreAimCrosshair(const Settings& s, const PreAimCourse& course, const Player& player, float screenW, float screenH) {
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 c(screenW * 0.5f, screenH * 0.5f);

    bool isMoving = !player.isAccurate();

    ImU32 col;
    if (isMoving) {
        col = IM_COL32(255, 60, 60, 240); // Red spread alert
    } else if (course.hasActiveTarget() && std::abs(course.activeBotVertAngleError()) <= 1.0f) {
        col = IM_COL32(50, 255, 120, 255); // Green head-level lock!
    } else {
        col = ImGui::ColorConvertFloat4ToU32(
            ImVec4(s.crosshairColor[0], s.crosshairColor[1],
                   s.crosshairColor[2], s.crosshairColor[3]));
    }

    float gap = isMoving ? (s.crosshairGap * 2.0f + 5.0f) : s.crosshairGap;
    float len = s.crosshairSize;
    float th  = s.crosshairThickness;

    dl->AddLine(ImVec2(c.x - gap - len, c.y), ImVec2(c.x - gap, c.y), col, th);
    dl->AddLine(ImVec2(c.x + gap, c.y), ImVec2(c.x + gap + len, c.y), col, th);
    dl->AddLine(ImVec2(c.x, c.y - gap - len), ImVec2(c.x, c.y - gap), col, th);
    dl->AddLine(ImVec2(c.x, c.y + gap), ImVec2(c.x, c.y + gap + len), col, th);

    if (s.centerDot) {
        dl->AddCircleFilled(c, th * 0.9f, col);
    }

    // Head level guidance arrows
    if (course.hasActiveTarget() && !isMoving) {
        float err = course.activeBotVertAngleError();
        if (err < -1.2f) {
            // Crosshair too low -> draw small upward arrow below crosshair
            dl->AddTriangleFilled(
                ImVec2(c.x, c.y + gap + len + 8.0f),
                ImVec2(c.x - 5.0f, c.y + gap + len + 15.0f),
                ImVec2(c.x + 5.0f, c.y + gap + len + 15.0f),
                IM_COL32(255, 120, 50, 220)
            );
        } else if (err > 1.2f) {
            // Crosshair too high -> draw small downward arrow above crosshair
            dl->AddTriangleFilled(
                ImVec2(c.x, c.y - gap - len - 8.0f),
                ImVec2(c.x - 5.0f, c.y - gap - len - 15.0f),
                ImVec2(c.x + 5.0f, c.y - gap - len - 15.0f),
                IM_COL32(255, 200, 50, 220)
            );
        }
    }
}

void ui::drawResults(const Trainer& t, UiRequest& req, const RoutineManager& routine, int bestScore, bool newBest) {
    ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 580), ImGuiCond_FirstUseEver);
    ImGui::Begin("Session Results & Diagnostic Report", nullptr, ImGuiWindowFlags_NoCollapse);

    if (newBest) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "NEW PERSONAL RECORD!");
    }

    const auto& sc = t.info();
    ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "Scenario: [%s] %s", sc.code, sc.name);
    ImGui::Text("Final Score: %d   (Best: %d)", t.score(), bestScore);
    ImGui::Separator();

    if (ImGui::BeginTable("ResultsStats", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Total Shots / Tracking Time"); ImGui::TableNextColumn();
        ImGui::Text("%d", t.shots());

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Hits"); ImGui::TableNextColumn();
        ImGui::Text("%d (Headshots: %d)", t.hits(), t.headshots());

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Accuracy"); ImGui::TableNextColumn();
        ImGui::Text("%.1f%%", t.accuracy());

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Avg Time-to-Kill (TTK)"); ImGui::TableNextColumn();
        ImGui::Text("%.0f ms", t.avgTtkMs());

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Best Streak"); ImGui::TableNextColumn();
        ImGui::Text("%d", t.bestStreak());

        if (sc.allowPlayerMove) {
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::Text("Counter-Strafe Stop Rate"); ImGui::TableNextColumn();
            float stopAcc = t.stopAccuracy();
            ImGui::TextColored(stopAcc >= 85.0f ? ImVec4(0.3f, 1.0f, 0.5f, 1.0f) : ImVec4(1.0f, 0.4f, 0.3f, 1.0f),
                "%.1f%% Stopped", stopAcc);
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "Deep Diagnostics & Flick Dynamics:");
    ImGui::BulletText("Overshoots: %.0f%%  |  Undershoots: %.0f%%", t.overshootPct(), t.undershootPct());

    std::string advice = t.generateDiagnosticAdvice();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
    ImGui::BeginChild("DiagnosticBox", ImVec2(0, 80), true);
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.8f, 1.0f), "COACH ANALYSIS:");
    ImGui::TextWrapped("%s", advice.c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();

    if (routine.isActive()) {
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f),
            "V9pujari Routine Progress: Step %d of %d",
            routine.currentStepIndex() + 1, routine.totalSteps());

        if (ImGui::Button("NEXT SET IN ROUTINE [Enter]", ImVec2(-1.0f, 44.0f))) {
            req.nextRoutineStep = true;
        }
    }

    if (ImGui::Button("Play Again [R]", ImVec2(-1.0f, 38.0f))) {
        req.playAgain = true;
    }

    if (ImGui::Button("Back to Menu [ESC]", ImVec2(-1.0f, 0))) {
        req.toMenu = true;
    }

    ImGui::End();
}

void ui::drawPreAimResults(const PreAimCourse& course, UiRequest& req) {
    ImGui::SetNextWindowPos(ImVec2(32, 32), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(560, 620), ImGuiCond_FirstUseEver);
    ImGui::Begin("Map Workshop Report", nullptr, ImGuiWindowFlags_NoCollapse);

    // Fail states (2026-11-06 tactical round): a defuse lost to the
    // timer or a clutch lost to damage says so plainly instead of
    // "COURSE COMPLETE" — the bots-cleared count below shows how far
    // the player got.
    if (course.failedByTimer()) {
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "SPIKE DETONATED — DEFUSE FAILED: %s", course.courseName().c_str());
    } else if (course.failedByDamage()) {
        ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "ELIMINATED — CLUTCH LOST: %s", course.courseName().c_str());
    } else {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.5f, 1.0f), "COURSE COMPLETE: %s", course.courseName().c_str());
    }
    ImGui::Separator();

    int total = course.totalBots();
    int cleared = course.botsCleared();
    int dmg = course.damageTakenCount();

    const auto& history = course.history();
    int hsCount = 0;
    double totalTtk = 0.0;
    float totalVertErr = 0.0f;
    int stoppedCount = 0;

    for (const auto& r : history) {
        if (r.isHeadshot) hsCount++;
        if (r.isStopped) stoppedCount++;
        totalTtk += r.killTimeMs;
        totalVertErr += std::abs(r.vertErrDeg);
    }

    float hsPct = cleared > 0 ? (100.0f * hsCount / cleared) : 0.0f;
    float avgTtk = cleared > 0 ? static_cast<float>(totalTtk / cleared) : 0.0f;
    float avgVertErr = cleared > 0 ? (totalVertErr / cleared) : 0.0f;
    float stopPct = cleared > 0 ? (100.0f * stoppedCount / cleared) : 0.0f;

    if (ImGui::BeginTable("PreAimSummaryTable", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Bots Cleared"); ImGui::TableNextColumn();
        ImGui::Text("%d / %d", cleared, total);

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Headshot Rate (Ranked benchmark: 22.3%%)"); ImGui::TableNextColumn();
        ImGui::TextColored(hsPct >= 60.0f ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.2f, 1.0f),
            "%.1f%%  (%d / %d Headshots)", hsPct, hsCount, cleared);

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Average Crosshair Level Vertical Error"); ImGui::TableNextColumn();
        ImGui::TextColored(avgVertErr <= 1.0f ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
            "%.2f deg", avgVertErr);

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Average Time-to-Damage (TTD)"); ImGui::TableNextColumn();
        ImGui::Text("%.0f ms", avgTtk);

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Counter-Strafe Stopping Rate"); ImGui::TableNextColumn();
        ImGui::TextColored(stopPct >= 85.0f ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
            "%.1f%%", stopPct);

        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::Text("Times Damaged by Bot Reaction"); ImGui::TableNextColumn();
        ImGui::TextColored(dmg == 0 ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.35f, 0.35f, 1.0f),
            "%d times", dmg);

        // Headshots-only drill: body shots do not kill, so they are
        // counted separately — a wasted shot is the metric here.
        if (course.isHeadshotsOnly()) {
            ImGui::TableNextRow(); ImGui::TableNextColumn();
            ImGui::Text("Body Hits Wasted (headshots only)"); ImGui::TableNextColumn();
            ImGui::TextColored(course.bodyHitsWasted() == 0 ? ImVec4(0.2f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.6f, 0.2f, 1.0f),
                "%d wasted", course.bodyHitsWasted());
        }

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "Angle-by-Angle Clearance Breakdown:");

    if (ImGui::BeginTable("AngleBreakdown", 4, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, ImVec2(0, 160))) {
        ImGui::TableSetupColumn("Angle Name");
        ImGui::TableSetupColumn("TTD");
        ImGui::TableSetupColumn("Headshot?");
        ImGui::TableSetupColumn("Vert Error");
        ImGui::TableHeadersRow();

        for (const auto& r : history) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", r.botName.c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%.0f ms", r.killTimeMs);

            ImGui::TableNextColumn();
            if (r.isHeadshot) ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "HEAD");
            else ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.3f, 1.0f), "BODY");

            ImGui::TableNextColumn();
            ImGui::Text("%+.1f deg", r.vertErrDeg);
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.12f, 0.18f, 1.0f));
    ImGui::BeginChild("PreAimCoachBox", ImVec2(0, 70), true);
    ImGui::TextColored(ImVec4(0.2f, 0.9f, 0.8f, 1.0f), "CROSSHAIR PLACEMENT COACH:");
    if (course.failedByTimer()) {
        ImGui::TextWrapped("The spike detonated. Retakes are won on the clock: clear the closest angle first, skip the flank check, and stick the defuse only after the site is actually clear.");
    } else if (course.failedByDamage()) {
        ImGui::TextWrapped("Three hits and the round is gone. A clutch is not five duels at once — isolate one angle at a time and use the reaction stagger: the far angles give you over a second.");
    } else if (hsPct >= 70.0f) {
        ImGui::TextWrapped("Exceptional head-level discipline! Your crosshair was locked to enemy head heights around corners.");
    } else if (avgVertErr > 1.2f) {
        ImGui::TextWrapped("Warning: Your crosshair was drifting above/below head level on corners. Lift crosshair before peeking boxes!");
    } else {
        ImGui::TextWrapped("Good clearing rhythm. Make sure to complete a full counter-strafe stop before firing to guarantee first-bullet accuracy.");
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Play Course Again [R]", ImVec2(-1.0f, 40.0f))) {
        req.playAgain = true;
    }

    if (ImGui::Button("Back to Menu [ESC]", ImVec2(-1.0f, 0))) {
        req.toMenu = true;
    }

    ImGui::End();
}
