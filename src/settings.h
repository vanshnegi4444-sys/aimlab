#pragma once
#include "scenarios.h"

// All tweakables live in ONE plain struct, shared by the UI, the trainer,
// the renderer and main().
struct Settings {
    // ---- Mouse feel -------------------------------------------------------
    // Valorant's real scale: 1.0 sens == 0.07 degrees of yaw/pitch per raw
    // mouse count. Using the same number here is what makes "0.35" feel
    // identical in ValoAim and in-game. See camera.h for the full derivation.
    float sensitivity = 0.35f;   // UI slider: 0.01 .. 2.00, step 0.001
    float dpi         = 800.0f;  // Mouse hardware DPI (for cm/360 and eDPI readout)

    float eDpi() const {
        return sensitivity * dpi;
    }

    float cmPer360() const {
        float degPerCount = sensitivity * 0.07f;
        if (degPerCount <= 0.0f || dpi <= 0.0f) return 0.0f;
        float countsPer360 = 360.0f / degPerCount;
        float inches = countsPer360 / dpi;
        return inches * 2.54f;
    }

    // ---- Active Scenario ---------------------------------------------------
    ScenarioId activeScenario = ScenarioId::Gridshot;

    // ---- Scenario defaults (overridden per scenario or tweaked in menu) ----
    float wallDist     = 10.0f;  // metres from camera to target wall (5..30)
    float roundTime    = 60.0f;  // seconds (30 / 60 / 120)
    float targetRadius = 0.15f;  // metres (0.05 .. 0.50)

    // ---- Crosshair ---------------------------------------------------------
    float crosshairSize = 12.0f;  // px: length of each of the 4 lines
    float crosshairGap  = 5.0f;   // px: center gap
    float crosshairThickness = 2.0f;
    bool  centerDot     = true;
    float crosshairColor[4] = { 0.0f, 1.0f, 0.55f, 1.0f }; // Valorant cyan-green

    // ---- Audio -------------------------------------------------------------
    bool  enableHitSound = true;
    float hitSoundVolume = 0.85f;

    // ---- Video & HUD -------------------------------------------------------
    bool vsync       = false; // Default OFF for minimal input-to-photon latency
    bool showOverlay = true;  // HUD overlay while playing
};
