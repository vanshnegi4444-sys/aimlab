#include "input.h"

#ifdef _WIN32

#include <cstdio>

RawMouse::RawMouse() = default;

RawMouse::~RawMouse() {
    // Unregister on shutdown so we don't leak the sink registration.
    if (m_attached) {
        RAWINPUTDEVICE rid{};
        rid.usUsagePage = 0x01; // HID: generic desktop controls
        rid.usUsage     = 0x02; // HID: mouse
        rid.dwFlags     = RIDEV_REMOVE;
        rid.hwndTarget  = nullptr;
        RegisterRawInputDevices(&rid, 1, sizeof(rid));
    }
}

bool RawMouse::attach(HWND hwnd) {
    RAWINPUTDEVICE rid{};
    rid.usUsagePage = 0x01;          // HID generic desktop page
    rid.usUsage     = 0x02;          // HID mouse usage
    rid.dwFlags     = RIDEV_INPUTSINK; // WHY: keep receiving WM_INPUT for this
                                     // hwnd even when it is not the focused
                                     // window (robustness during alt-tab etc.)
    rid.hwndTarget  = hwnd;
    m_attached = (RegisterRawInputDevices(&rid, 1, sizeof(rid)) != FALSE);
    if (!m_attached)
        std::printf("[input] RegisterRawInputDevices failed, GetLastError=%lu\n",
                    GetLastError());
    return m_attached;
}

void RawMouse::onWin32Message(HRAWINPUT hRawInput) {
    // Two-step GetRawInputData: first call with nullptr to learn the size.
    UINT size = 0;
    if (GetRawInputData(hRawInput, RID_INPUT, nullptr, &size,
                        sizeof(RAWINPUTHEADER)) != 0)
        return;
    if (size == 0 || size > 4096)
        return; // sanity: a mouse packet is ~40 bytes; anything huge is bogus

    BYTE buffer[4096]; // on the stack; freed automatically when we return
    if (GetRawInputData(hRawInput, RID_INPUT, buffer, &size,
                        sizeof(RAWINPUTHEADER)) != size)
        return;

    const RAWINPUT* raw = reinterpret_cast<const RAWINPUT*>(buffer);
    if (raw->header.dwType != RIM_TYPEMOUSE)
        return; // keyboard/HID packets can arrive here too; ignore them

    const RAWMOUSE& m = raw->data.mouse;

    // WHY relative-only: gaming mice report RELATIVE counts (lLastX/lLastY =
    // "moved this many counts since the last packet"). Absolute mode is for
    // tablets/touchscreens and would need last-position differencing — out of
    // scope for v1, so those packets are skipped deliberately.
    if (m.usFlags & MOUSE_MOVE_ABSOLUTE)
        return;

    // NOTE: button state (usButtonFlags) is available here too, but v1 reads
    // clicks through GLFW's mouse-button callback in main.cpp — same message
    // pump, negligible latency difference, much simpler code.
    m_dx += m.lLastX;
    m_dy += m.lLastY;
}

void RawMouse::consume(int& outDx, int& outDy) {
    outDx = static_cast<int>(m_dx);
    outDy = static_cast<int>(m_dy);
    m_dx = 0;
    m_dy = 0;
}

#endif // _WIN32
