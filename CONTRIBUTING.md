# Contributing to ValoAim

Thank you for your interest in contributing to **ValoAim**! We want to build the most lightweight, exact, and responsive tactical aim trainer for competitive players.

Whether you're fixing bugs, adding new map workshops, polishing the physics/movement, or optimizing the renderer, your help is warmly welcomed.

---

## 🛠️ How to Get Started

### Prerequisites
- **OS**: Windows 10/11 (64-bit)
- **Compiler**: Visual Studio 2022 (MSVC with C++17 support)
- **Build System**: CMake 3.20+
- **Hardware**: Any modern OpenGL 3.3 capable GPU

### Quick Build Guide
```powershell
# Clone the repository
git clone https://github.com/vanshnegi4444-sys/aimlab.git
cd aimlab

# Configure with CMake
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build in Release mode
cmake --build build --config Release -j 8

# Run
.\build\Release\valo-aim.exe
```

---

## 🎯 High-Priority Areas for Community Contributions

Here are key roadmap features where the project actively seeks help:

1. **Cross-Platform Support**:
   - Currently, raw input and procedural audio rely on Win32 (`RegisterRawInputDevices` and `waveOut`).
   - Introducing SDL2/GLFW native raw mouse input and Miniaudio/OpenAL for Linux & macOS portability.

2. **Map Pre-Aim Workshop Expansion**:
   - Adding Sunset, Lotus, Breeze, Split, and Icebox geometry and tactical bot placements.
   - Tooling for exporting Blender scenes directly to `.bin` format.

3. **Shooting Mechanics & Gun Play Polish**:
   - Weapon recoil patterns (Vandal / Phantom burst bloom).
   - Dynamic crosshair spread bloom when running or jumping.
   - Tagging / slow-down effect when receiving bot fire.

4. **Scenario Editor / Custom Placements**:
   - In-game GUI to place bots, record peek paths, set reaction timings, and export custom workshops as JSON.

5. **Audio & UI Polish**:
   - Spatial 3D sound (HRTF or distance panning for footsteps and gunshots).
   - High-refresh rate monitor optimization, low-latency display modes.

---

## 📋 Pull Request Guidelines

1. **Fork and Branch**: Create a feature branch from `main` (e.g. `feature/lotus-a-preaim` or `fix/audio-latency`).
2. **Coding Standards**:
   - Standard C++17.
   - Keep dependencies lean and minimal (header-only or FetchContent).
   - Preserve comments, tactical logic, and documentation.
3. **Verify Build**: Ensure clean compilation without errors or warnings (`cmake --build build --config Release`).
4. **Submit PR**: Open a Pull Request detailing the changes, reasoning, and test steps.

---

## 💬 Community & Questions
Feel free to open an issue or start a Discussion on GitHub!
