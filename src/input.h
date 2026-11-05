#pragma once

// Win32 Raw Input mouse — the ONLY input path used for aiming.
// WHY raw input instead of the normal cursor position:
//   * The normal cursor goes through Windows' pointer ballistics ("Enhance
//     pointer precision" = mouse acceleration). That makes the same physical
//     mouse movement rotate the view by different amounts depending on speed.
//   * Valorant reads HID counts straight from the device and multiplies by
//     sens * 0.07 degrees. We do exactly the same, so the feel transfers 1:1.
// main.cpp subclasses the GLFW window's WndProc and forwards every WM_INPUT
// here. Everything runs on the main thread (GLFW pumps the message queue in
// glfwPollEvents), so no atomics/locks are needed around the accumulator.

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

class RawMouse {
public:
    RawMouse();
    ~RawMouse();

    RawMouse(const RawMouse&) = delete;
    RawMouse& operator=(const RawMouse&) = delete;

    // Register `hwnd` to receive raw mouse WM_INPUT messages.
    // RIDEV_INPUTSINK means we keep getting data even if another window has
    // focus — the aim never "misses" counts because of a focus flicker.
    // Returns false if registration failed (call GetLastError() for why).
    bool attach(HWND hwnd);

    // Feed one WM_INPUT message in. Called from the subclassed WndProc.
    // Returns immediately for non-mouse input. Adds relative movement counts
    // into the accumulator.
    void onWin32Message(HRAWINPUT hRawInput);

    // Grab the accumulated counts and reset to zero. Call ONCE per frame,
    // after glfwPollEvents() has drained the message queue (so every pending
    // WM_INPUT has already been added — no count is ever dropped).
    void consume(int& outDx, int& outDy);

private:
    LONG m_dx = 0;      // accumulated relative counts since last consume()
    LONG m_dy = 0;
    bool m_attached = false;
};

#else // not Windows

// Non-Windows stub: lets the project still be parsed/configured elsewhere,
// but aiming genuinely needs Win32 Raw Input, so attach() reports failure.
class RawMouse {
public:
    bool attach(void*) { return false; }
    void onWin32Message(void*) {}
    void consume(int& dx, int& dy) { dx = 0; dy = 0; }
};

#endif
