#pragma once
#include "settings.h"
#include "scenarios.h"
#include "routine.h"

class Trainer;
class PreAimCourse;
class Player;

struct UiRequest {
    bool start           = false; // MENU -> PLAY
    bool startRoutine    = false; // MENU -> start V9pujari routine
    bool nextRoutineStep = false; // RESULTS -> next routine step
    bool playAgain       = false; // RESULTS -> PLAY
    bool toMenu          = false; // RESULTS -> MENU
};

namespace ui {

void drawMenu(Settings& s, UiRequest& req, const RoutineManager& routine, int bestScore);

void drawPlayOverlay(const Trainer& t, const Settings& s, const RoutineManager& routine);
void drawPreAimOverlay(const PreAimCourse& course, const Settings& s, const Player& player, double now);

void drawCrosshair(const Settings& s, const Trainer& t, float screenW, float screenH);
void drawPreAimCrosshair(const Settings& s, const PreAimCourse& course, const Player& player, float screenW, float screenH);

void drawResults(const Trainer& t, UiRequest& req, const RoutineManager& routine, int bestScore, bool newBest);
void drawPreAimResults(const PreAimCourse& course, UiRequest& req);

} // namespace ui
