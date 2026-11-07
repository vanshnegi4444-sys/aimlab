// ValoAim — a native Windows aim trainer with Valorant-matched mouse feel.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>          // must come before GLFW's native header

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h> // glfwGetWin32Window

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdio>
#include <fstream>
#include <map>
#include <string>

#include "gl_loader.h"
#include "input.h"
#include "camera.h"
#include "settings.h"
#include "trainer.h"
#include "renderer.h"
#include "ui.h"
#include "audio.h"
#include "routine.h"
#include "scenarios.h"
#include "map_mesh.h"
#include "player.h"
#include "preaim_course.h"

// ---------------------------------------------------------------------------
// App state
// ---------------------------------------------------------------------------
enum class Mode { Menu, Play, Results };

static Mode           g_mode             = Mode::Menu;
static bool           g_shootRequested   = false;
static bool           g_restartRequested = false;
static bool           g_enterRequested   = false;
static HWND           g_hwnd             = nullptr;
static RawMouse       g_mouse;
static RoutineManager g_routine;
static std::map<int, int> g_bestScores;

static MapMesh        g_mapMesh;
static MapMesh        g_botMesh;
static Player         g_player;
static PreAimCourse   g_preAimCourse;
static std::string    g_currentMapPath   = "";
static bool           g_isMapCourseActive = false;

// ---------------------------------------------------------------------------
// High score persistence per scenario
// ---------------------------------------------------------------------------
static void loadBests() {
    std::ifstream f("best.txt");
    int id = 0, score = 0;
    while (f >> id >> score) {
        g_bestScores[id] = score;
    }
}

static void saveBests() {
    std::ofstream f("best.txt", std::ios::trunc);
    for (const auto& [id, score] : g_bestScores) {
        f << id << " " << score << "\n";
    }
}

static int getBestScore(ScenarioId id) {
    auto it = g_bestScores.find(static_cast<int>(id));
    if (it != g_bestScores.end()) return it->second;
    return 0;
}

// ---------------------------------------------------------------------------
// WndProc subclass for WM_INPUT
// ---------------------------------------------------------------------------
static WNDPROC g_origWndProc = nullptr;

static LRESULT CALLBACK HookedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_INPUT)
        g_mouse.onWin32Message(reinterpret_cast<HRAWINPUT>(lParam));
    return CallWindowProcW(g_origWndProc, hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Cursor management
// ---------------------------------------------------------------------------
static bool g_cursorHidden = false;

static void hideOsCursor() {
    if (!g_cursorHidden) {
        while (::ShowCursor(FALSE) >= 0) {}
        g_cursorHidden = true;
    }
}

static void showOsCursor() {
    if (g_cursorHidden) {
        while (::ShowCursor(TRUE) < 0) {}
        g_cursorHidden = false;
    }
}

static void clipCursorToWindow(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
    ClientToScreen(hwnd, &tl);
    ClientToScreen(hwnd, &br);
    RECT scr{ tl.x, tl.y, br.x, br.y };
    ClipCursor(&scr);
}

// ---------------------------------------------------------------------------
// Key and Mouse callbacks
// ---------------------------------------------------------------------------
static void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    (void)window;
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            if (g_mode == Mode::Play || g_mode == Mode::Results) {
                g_mode = Mode::Menu;
                showOsCursor();
                ClipCursor(nullptr);
            }
        } else if (key == GLFW_KEY_R) {
            if (g_mode == Mode::Play || g_mode == Mode::Results) {
                g_restartRequested = true;
            }
        } else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
            if (g_mode == Mode::Results) {
                g_enterRequested = true;
            }
        }
    }
}

static void onMouseButton(GLFWwindow* window, int button, int action, int /*mods*/) {
    (void)window;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (g_mode == Mode::Play && !ImGui::GetIO().WantCaptureMouse)
            g_shootRequested = true;
    }
}

// ---------------------------------------------------------------------------
// Path Resolver
// ---------------------------------------------------------------------------
static std::string resolveAssetPath(const std::string& relPath) {
    if (std::ifstream(relPath).good()) return relPath;
    std::string up2 = "../../" + relPath;
    if (std::ifstream(up2).good()) return up2;
    std::string up1 = "../" + relPath;
    if (std::ifstream(up1).good()) return up1;
    return relPath;
}

// ---------------------------------------------------------------------------
// Map Loader
// ---------------------------------------------------------------------------
static void ensureMapLoaded(const std::string& path) {
    std::string resolved = resolveAssetPath(path);
    if (g_currentMapPath != resolved) {
        g_mapMesh.load(resolved);
        g_currentMapPath = resolved;
    }
}

// ---------------------------------------------------------------------------
// Mode transitions
// ---------------------------------------------------------------------------
static int g_lastSpikeBeepSec = -1;

static void enterPlay(Trainer& trainer, Camera& camera, const Settings& s, ScenarioId scId) {
    g_isMapCourseActive = isMapPreAim(scId);
    g_lastSpikeBeepSec  = -1;

    if (g_isMapCourseActive) {
        MapCourseId cid = MapCourseId::AscentB_Clear;
        if (scId == ScenarioId::BindB_PreAim) cid = MapCourseId::BindB_LongClear;
        else if (scId == ScenarioId::HavenA_PreAim) cid = MapCourseId::HavenA_Clear;
        else if (scId == ScenarioId::HavenB_PreAim) cid = MapCourseId::HavenB_Clear;
        else if (scId == ScenarioId::HavenC_PreAim) cid = MapCourseId::HavenC_Clear;
        else if (scId == ScenarioId::HavenC_Retake) cid = MapCourseId::HavenC_Retake;
        else if (scId == ScenarioId::HavenA_ElevLadder) cid = MapCourseId::HavenA_ElevLadder;
        else if (scId == ScenarioId::AscentB_Defuse) cid = MapCourseId::AscentB_Defuse;
        else if (scId == ScenarioId::BindB_Defuse) cid = MapCourseId::BindB_Defuse;
        else if (scId == ScenarioId::HavenA_Defuse) cid = MapCourseId::HavenA_Defuse;
        else if (scId == ScenarioId::AscentB_Clutch1v5) cid = MapCourseId::AscentB_Clutch1v5;
        else if (scId == ScenarioId::BindB_HeadHeight) cid = MapCourseId::BindB_HeadHeight;
        else if (scId == ScenarioId::AscentB_AnchorHold) cid = MapCourseId::AscentB_AnchorHold;

        g_preAimCourse.start(cid, glfwGetTime());
        ensureMapLoaded(g_preAimCourse.mapBinPath());
        g_preAimCourse.snapToMapFloors(g_mapMesh);

        g_player.spawn(g_preAimCourse.playerSpawnPos(), g_preAimCourse.playerSpawnYaw());
        camera.pos = g_player.eyePos();
        camera.yawDeg = g_preAimCourse.playerSpawnYaw();
        camera.pitchDeg = 0.0f;
    } else {
        camera = Camera{};
        trainer.start(s, glfwGetTime(), scId);
    }

    g_mode = Mode::Play;
    hideOsCursor();
    clipCursorToWindow(g_hwnd);
}

static void enterMenu() {
    g_mode = Mode::Menu;
    showOsCursor();
    ClipCursor(nullptr);
}

static void enterResults(Trainer& t, bool& newBest) {
    g_mode = Mode::Results;
    showOsCursor();
    ClipCursor(nullptr);

    int prevBest = getBestScore(t.scenarioId());
    newBest = (t.score() > prevBest);
    if (newBest) {
        g_bestScores[static_cast<int>(t.scenarioId())] = t.score();
        saveBests();
    }
}

static void enterPreAimResults() {
    g_mode = Mode::Results;
    showOsCursor();
    ClipCursor(nullptr);
}

// ---------------------------------------------------------------------------
int main() {
    // ---- GLFW -------------------------------------------------------------
    if (!glfwInit()) {
        std::printf("[main] glfwInit failed\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ValoAim - Valorant Map Aim Trainer & Pre-Aim Workshops", nullptr, nullptr);
    if (!window) {
        std::printf("[main] glfwCreateWindow failed\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // vsync OFF by default for minimal latency

    // ---- OpenGL entry points ----------------------------------------------
    if (!LoadOpenGLFunctions()) {
        std::printf("[main] OpenGL 3.3 entry points missing\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // ---- Audio ------------------------------------------------------------
    if (!audio::init()) {
        std::printf("[main] Audio initialization failed (continuing without audio)\n");
    }

    // ---- Win32: raw input + WndProc subclass -------------------------------
    g_hwnd = glfwGetWin32Window(window);
    if (!g_mouse.attach(g_hwnd))
        std::printf("[main] WARNING: raw input unavailable\n");
    g_origWndProc = reinterpret_cast<WNDPROC>(
        SetWindowLongPtrW(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookedWndProc)));

    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);

    // ---- Dear ImGui ---------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ---- Game objects -------------------------------------------------------
    Settings s;
    Trainer  trainer;
    Camera   camera;
    Renderer renderer;
    if (!renderer.init()) {
        std::printf("[main] renderer init failed\n");
        return 1;
    }

    // Load Bot Model
    std::string botPath = resolveAssetPath("assets/models/bot_jett.bin");
    if (!g_botMesh.load(botPath)) {
        std::printf("[main] Note: %s not found\n", botPath.c_str());
    }

    loadBests();
    bool newBest = false;
    double lastFrameTime = glfwGetTime();

    // ---- Main Loop ----------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        double now = glfwGetTime();
        double dt  = now - lastFrameTime;
        if (dt > 0.1) dt = 0.1;
        lastFrameTime = now;

        // 1. Consume Raw Mouse Input
        int dx = 0, dy = 0;
        g_mouse.consume(dx, dy);
        if (g_mode == Mode::Play) {
            camera.applyMouse(dx, dy, s.sensitivity);
            clipCursorToWindow(g_hwnd);
        }

        // 2. Handle Key shortcuts (R to restart)
        if (g_restartRequested) {
            g_restartRequested = false;
            if (g_mode == Mode::Play || g_mode == Mode::Results) {
                ScenarioId activeSc = g_routine.isActive() ? g_routine.currentItem().scenario : s.activeScenario;
                enterPlay(trainer, camera, s, activeSc);
            }
        }

        // 3. Movement input & Game Logic
        PlayerInput pInput;
        pInput.forward  = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        pInput.backward = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        pInput.left     = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        pInput.right    = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
        pInput.jump     = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        pInput.walk     = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        pInput.crouch   = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);

        if (g_shootRequested) {
            g_shootRequested = false;
            if (g_mode == Mode::Play) {
                if (g_isMapCourseActive) {
                    PreAimResult pRes;
                    int res = g_preAimCourse.shoot(now, g_player.eyePos(), camera.forward(), g_player.isAccurate(), pRes, &g_mapMesh);
                    if (s.enableHitSound) {
                        if (res == 2) audio::playHeadshot(s.hitSoundVolume);
                        else if (res == 1) audio::playHit(s.hitSoundVolume);
                    }
                } else {
                    int res = trainer.shoot(now, camera);
                    if (s.enableHitSound) {
                        if (res == 2) audio::playHeadshot(s.hitSoundVolume);
                        else if (res == 1) audio::playHit(s.hitSoundVolume);
                    }
                }
            }
        }

        if (g_mode == Mode::Play) {
            if (g_isMapCourseActive) {
                g_player.update(dt, pInput, camera, &g_mapMesh);
                camera.pos = g_player.eyePos();
                g_preAimCourse.update(now, dt, g_player.eyePos(), camera.pitchDeg, camera.yawDeg, &g_mapMesh);

                // Audible countdown beep for the final 10s of a spike timer
                if (g_preAimCourse.hasTimeLimit() && !g_preAimCourse.isComplete() && !g_preAimCourse.isFailed()) {
                    int remSec = static_cast<int>(std::ceil(g_preAimCourse.timeRemaining(now)));
                    if (remSec >= 1 && remSec <= 10 && remSec != g_lastSpikeBeepSec) {
                        g_lastSpikeBeepSec = remSec;
                        if (s.enableHitSound) {
                            audio::playSpikeBeep(s.hitSoundVolume * 0.65f);
                        }
                    }
                }

                // A map course ends on a clear OR a fail (spike detonated
                // / clutch lost — 2026-11-06 tactical round). The results
                // screen shows which one it was.
                if (g_preAimCourse.isComplete() || g_preAimCourse.isFailed()) {
                    if (g_preAimCourse.isFailed() && s.enableHitSound) {
                        audio::playFail(s.hitSoundVolume);
                    }
                    enterPreAimResults();
                }
            } else {
                bool movingLeft  = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
                bool movingRight = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
                trainer.update(now, dt, camera, movingLeft, movingRight);
                camera.pos.x = trainer.playerPosX();
                if (trainer.isComplete()) {
                    enterResults(trainer, newBest);
                }
            }
        }

        // 4. Render 3D Scene
        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        if (fbw == 0 || fbh == 0) {
            glfwSwapBuffers(window);
            continue;
        }

        glViewport(0, 0, fbw, fbh);
        float aspect = static_cast<float>(fbw) / static_cast<float>(fbh);
        Mat4 viewProj = mat4Mul(camera.projMatrix(aspect), camera.viewMatrix());

        if (g_isMapCourseActive) {
            glClearColor(0.15f, 0.18f, 0.23f, 1.0f); // Valorant sky tint
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderer.renderMap(viewProj, camera, g_mapMesh);
            renderer.renderPreAimBots(viewProj, camera, g_botMesh, g_preAimCourse);
        } else {
            glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float activeWallDist = (g_mode == Mode::Play) ? trainer.wallDist() : s.wallDist;
            bool  hasCover       = (g_mode == Mode::Play) ? trainer.hasCoverWall() : false;
            float coverX         = (g_mode == Mode::Play) ? trainer.coverWallX() : -1.0f;

            renderer.renderWall(viewProj, camera, activeWallDist, hasCover, coverX);
            if (g_mode == Mode::Play) {
                renderer.renderTargets(viewProj, trainer, camera, &g_botMesh);
            }
        }

        // 5. ImGui UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        UiRequest req;
        if (g_enterRequested) {
            g_enterRequested = false;
            if (g_mode == Mode::Results) {
                if (g_routine.isActive()) req.nextRoutineStep = true;
                else req.playAgain = true;
            }
        }

        if (g_mode == Mode::Menu) {
            bool prevVsync = s.vsync;
            int curBest = getBestScore(s.activeScenario);
            ui::drawMenu(s, req, g_routine, curBest);
            if (s.vsync != prevVsync)
                glfwSwapInterval(s.vsync ? 1 : 0);
        } else if (g_mode == Mode::Play) {
            if (g_isMapCourseActive) {
                if (s.showOverlay)
                    ui::drawPreAimOverlay(g_preAimCourse, s, g_player, now);
                ImVec2 ds = ImGui::GetIO().DisplaySize;
                ui::drawPreAimCrosshair(s, g_preAimCourse, g_player, ds.x, ds.y);
            } else {
                if (s.showOverlay)
                    ui::drawPlayOverlay(trainer, s, g_routine);
                ImVec2 ds = ImGui::GetIO().DisplaySize;
                ui::drawCrosshair(s, trainer, ds.x, ds.y);
            }
        } else {
            if (g_isMapCourseActive) {
                ui::drawPreAimResults(g_preAimCourse, req);
            } else {
                int curBest = getBestScore(trainer.scenarioId());
                ui::drawResults(trainer, req, g_routine, curBest, newBest);
            }
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // 6. Handle UI state transitions
        if (req.startRoutine) {
            g_routine.start();
            enterPlay(trainer, camera, s, g_routine.currentItem().scenario);
        } else if (req.nextRoutineStep) {
            if (g_routine.advance()) {
                enterPlay(trainer, camera, s, g_routine.currentItem().scenario);
            } else {
                enterMenu();
            }
        } else if (req.start) {
            g_routine.stop();
            enterPlay(trainer, camera, s, s.activeScenario);
        } else if (req.playAgain) {
            ScenarioId activeSc = g_routine.isActive() ? g_routine.currentItem().scenario : s.activeScenario;
            enterPlay(trainer, camera, s, activeSc);
        } else if (req.toMenu) {
            g_routine.stop();
            enterMenu();
        }

        glfwSwapBuffers(window);
    }

    // ---- Shutdown -----------------------------------------------------------
    if (g_origWndProc)
        SetWindowLongPtrW(g_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(g_origWndProc));
    audio::shutdown();
    renderer.shutdown();
    g_mapMesh.destroy();
    g_botMesh.destroy();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
