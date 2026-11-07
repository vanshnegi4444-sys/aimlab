# ValoAim

A native Windows aim trainer for **VALORANT players**, written in C++20.
It does what [Sheepu Aim](https://aim.sheepu.dev/aim) does (scenario-based
aim training: target drills, map pre-aim, bot duels), but as a real desktop app
with **lower input latency, exact Valorant mouse feel, deeper per-shot
analytics, and a training plan that adapts to you**.

> **This README has three jobs:**
> 1. **Part A – Analysis:** what Sheepu Aim has, and where it falls short.
> 2. **Part B – Design spec:** exactly how ValoAim should work: every scenario,
>    the scoring formulas, the analytics, the architecture and the roadmap.
> 3. **Part C – How to actually improve your aim with it:** the training
>    program.
>
> Build and controls for the current v1 come first so you can start playing now.

---

## Table of contents

- [0. Quick start (v1, works today)](#0-quick-start-v1-works-today)
- [Part A — Analysis of aim.sheepu.dev](#part-a--analysis-of-aimsheepudev)
- [Part B — ValoAim design spec](#part-b--valoaim-design-spec)
  - [B1. Core principles](#b1-core-principles)
  - [B2. Mouse feel, FOV and latency](#b2-mouse-feel-fov-and-latency)
  - [B3. Scenario catalogue (exact parameters)](#b3-scenario-catalogue-exact-parameters)
  - [B4. Scoring formulas](#b4-scoring-formulas)
  - [B5. Analytics (the main thing we do better)](#b5-analytics-the-main-thing-we-do-better)
  - [B6. Adaptive difficulty](#b6-adaptive-difficulty)
  - [B7. Scenario file format](#b7-scenario-file-format)
  - [B8. Architecture](#b8-architecture)
  - [B9. Using the Agents / Maps / Weapons assets](#b9-using-the-agents--maps--weapons-assets)
  - [B10. Roadmap](#b10-roadmap)
- [Part C — How to improve your aim (the training program)](#part-c--how-to-improve-your-aim-the-training-program)
- [Part D — Personal plan from your tracker (V9pujari#0000)](#part-d--personal-plan-from-your-tracker-v9pujari0000)

---

## 0. Quick start (v1, works today)

### Build (on your Windows 10/11 PC)

You need: **Visual Studio 2022** (the free Community edition is fine, with the
"C++ desktop development" workload), **CMake 3.24+**, and **Git** (CMake uses
it to download GLFW and Dear ImGui on the first configure).

```bat
cd path\to\aimlab
cmake -B build
cmake --build build --config Release
```

The exe lands at `build\Release\valo-aim.exe`. Double-click it. (A console
window opens next to the game in v1. Startup messages and errors show up
there. Closing the game window quits.)

> The very first `cmake -B build` needs internet to fetch GLFW 3.4 and Dear
> ImGui. After that, everything is cached and the game itself never touches
> the network.

### Controls

| Input | Action |
|---|---|
| Mouse | Raw Mouse Input (exact Valorant 0.07 deg/count rotation) |
| Left Click | Shoot / Hitscan (Vandal mechanics) |
| W / A / S / D | 3D Movement through Valorant maps (5.4 m/s run) |
| Left Shift | Walk (3.0 m/s silent walk) |
| Left Ctrl | Crouch (1.05m eye height, 1.5 m/s crouch speed) |
| Space | Jump (4.8 m/s impulse) |
| R | Instant quick-restart round / course |
| ENTER | Advance to next set in Routine / next map (on Results screen) |
| ESC | Pause round → Menu / Settings |

---

## 3D Map Pre-Aim Workshops (Sheepu Aim Style)

ValoAim loads real 3D Valorant map geometry directly from extracted game assets (`Maps/*.blend` converted to high-performance zero-lag `.bin` meshes) with 3D Jett humanoid bot models.

### 1. Ascent B: Main to Site Clearing (`assets/maps/ascentB.bin`)
- **Player Spawn:** B Main corridor, looking into the B Main archway toward B Site.
- **Corner Clearing Progression:**
  1. *Lane Close Left Corner (tight 90° angle check)* — requires clearing the immediate left corner without overpeeking into site.
  2. *Green Box Top (Elevated Head Level)* — **Crucial for crosshair height!** Bot stands on top of the radianite crate (+1.2m elevation). Forces you to lift your crosshair before slicing the angle.
  3. *Market Door Edge* — defender holding the Market door and the east site edge. (Moved 2026-11-06: the old spot (-23, -66.5) was mid-site, mislabelled — the real Market-side floor is the east edge at x = -8.)
  4. *B Site Default Box* — Standard plant box hold (Triple — the safest plant spot, played from Boathouse).
  5. *Boathouse Front Entrance* — Slicing the back-site corner (the anchor: most cover, hardest to clear, the post-plant spot).
  6. *Boathouse Workshop Stairs* — Stairs angle requiring diagonal crosshair tracking down the ramp.
  7. *CT Arch Long Range Peek* — Long range headshot duel against defender holding from CT spawn arch.
- **Tactical note (2026-11-06):** every bot now faces the B Main entry (yaw = atan2 toward the spawn) — the old yaws faced into the site, away from the attacker. See "Tactical grounding" below.

### 2. Bind B: Long & Hookah Clearing (`assets/maps/bindB.bin`)
- **Player Spawn:** B Long entrance near Garden.
- **Corner Clearing Progression:**
  1. *Garden Vista Left Angle* — Close corner check.
  2. *Hookah Window Long Hold* — **Extreme vertical elevation test!** Hookah window is elevated +3.5m above ground. Teaches you not to run down B Long with crosshair glued to ground level.
  3. *Site Sand Container Front* — Peeking the main Radianite container.
  4. *B Site Center Box* — Default site dueling angle.
  5. *B Elbow Hold* — Tight defensive angle hold.
  6. *B Hall / Hell Deep Hold* — Deep corner check in B Hall ("Hell").
- **Tactical note (2026-11-06):** positions unchanged (they were already real spots), but Garden, Hookah and Hell faced the wrong way — all six bots now face the B Long entry. See "Tactical grounding" below.

### 3. Haven A: Long to Heaven Angle Slicing (`assets/maps/havenA.bin`)
- **Player Spawn:** A Long entrance corridor.
- **Corner Clearing Progression:**
  1. *A Long Corner Peek* — Opening duel down A Long.
  2. *A Site Default Stack* — Mid-site box hold.
  3. *A Heaven High Window* — Elevated sniper window hold. Trains instantaneous vertical micro-flick from ground level to Heaven.
  4. *A Hell Under-Window Hold* — Ground level contrast against Heaven.
  5. *A Short / Sewers Flank* — Flank angle check.
- **Tactical note (2026-11-06):** positions unchanged; only the A Long bot's yaw was wrong (170° faced 25° off the entry — now -165°, exactly at the A Long ramp). Heaven/Hell/Default/Short already faced the entry.

### 4. Haven B: Corridor to Site Entry & Clear (`assets/maps/havenB.bin`)
- **Player Spawn:** B corridor, south end, looking up the ramp toward B Site.
- **Corner Clearing Progression:**
  1. *Ramp Top Duel* — Opening duel at the top of the entry ramp.
  2. *Site Left Corner* — Close corner check on site entry.
  3. *Site Default Box* — Standard plant box hold.
  4. *Back Site Corner* — Deep corner check at the back of site.
  5. *Site East Corner* — The site's far east corner. (Moved 2026-11-06: the old "Site Right Flank" at (12, -88) stood 2.8 m from Site Left Corner — two bots in the same corner. The site really extends east to x ~ 44.)
- **Note:** bot positions were measured from the map file itself (Blender floor scan: corridor floor ~1.0 m, site floor ~3.0 m). Bot facing angles are now computed exactly toward the corridor entry (atan2 toward the spawn), not approximate.

---

## Crosshair Level Guidance System (How It Fixes Your Aim)

### The Tracker Problem:
In your Tracker.gg profile (`V9pujari#0000` - Plat 2):
- **71.5% Body Hits (719 hits)** vs **22.1% Headshot Rate (222 hits)**.
- **-10 Damage Delta per Round (DDΔ)** (Bottom 28%).
- **Why this happens:** When moving around corners, your crosshair naturally drifts downward to body/torso height (sitting at -2.5° to -5.0° pitch error). When an opponent appears, your first bullet hits their chest, giving them time to headshot you.

### How ValoAim Fixes It in Real-Time:
1. **Vertical Angular Telemetry:** The engine continuously calculates the exact vertical angular discrepancy between your camera reticle and the enemy's head center:
   $$\Delta \theta_{\text{pitch}} = \theta_{\text{player}} - \arctan\left(\frac{y_{\text{head}} - y_{\text{eye}}}{d_{\text{horizontal}}}\right)$$
2. **Dynamic Crosshair State:**
   - **PERFECT LOCK (Green reticle):** $|\Delta \theta_{\text{pitch}}| \le 1.0^\circ$. You are holding exact head height. Swings will result in instant headshots without needing vertical micro-correction.
   - **TOO LOW (Red reticle + `▲` Upward Chevron):** Crosshair is sitting below head level. Lift reticle before swinging!
   - **TOO HIGH (Yellow reticle + `▼` Downward Chevron):** Crosshair is floating above head level.
3. **Bot Reaction Timer (Peeker Duel Pressure):**
   - Bots don't wait for you. When you establish line-of-sight to a bot around a corner, an internal reaction timer (420ms) begins.
   - If you do not counter-strafe to a stop and hit the head before 420ms expires, the bot returns fire, damaging you and logging a duel loss.
4. **Counter-Strafe Stopping Enforcement:**
   - Velocity deadzone is strictly enforced at $1.85\text{ m/s}$ (34% of max speed).
   - If you shoot while running ($> 1.85\text{ m/s}$), the bullet whiffs with weapon spread inaccuracy. You must tap the opposite strafe key to stop before firing.

---

**What's in the app:**
- **3D Map Workshops:** Real Ascent B, Bind B, Haven A and Haven B geometry with Jett humanoid bots and full WASD navigation.
- **Crosshair Level Guidance:** Real-time head height indicators and post-run vertical error diagnostics.
- **V9pujari Daily Routine:** 6 structured blocks (18 sets total, ~16 mins of drills) tailored directly from your tracker.gg profile.
- **22 Scenarios in Total:** 7 3D Map Workshops + 15 Precision, Tracking, Movement & Reaction Drills.
- **Deep Analytics:** Overshoot vs. Undershoot %, Counter-strafe stop accuracy %, TTK (ms), and coach advice.
- **Low-Latency Audio:** Procedural hit clicks and crisp metallic headshot dinks.
- **Valorant Sens Engine:** Exact 0.07 deg/count rotation, eDPI, and cm/360 calculation.

---

# Part A — Analysis of aim.sheepu.dev

I went through the live site (HTML shell, JS bundles, locale/copy files and the
built-in workshop guide). Here is what it is.

### A1. What it is

- **Sheepu Aim** ("Luyện aim VALORANT trên trình duyệt", which means *VALORANT
  aim training in the browser*). It's a Vietnamese/English re-skin of a
  Chinese WebGL aim trainer. It runs in the browser as a ~700 KB entry bundle
  plus about 400 lazy-loaded chunks and WebAssembly.
- Mouse + keyboard, PC only. On mobile it shows an "Open on PC" message.

### A2. Features it has

| Area | What Sheepu Aim does |
|---|---|
| **Two scenario types** | *Target scenario* ("Ball workshop": spheres) and *Map scenario* (humanoid bots on real Valorant map geometry). |
| **Map lessons** | Pre-aim, angle holding and anti-flash on **9 Valorant maps** (e.g. *Sunset – A Elbow / A Short / B Main attack pre-aim*, 15–16 bots at random positions; *Ascent – A main entry & anti-flash*; *Bind – Hold Hell 1v4*; *Haven – C long sniper hold*). |
| **Target drills** | 3×3 grid, long-range 3×3, four-target flicks, five-target challenge, tiny three-target micro-adjust, single-target micro-adjust, falling-target clicking, smooth tracking, continuous strafing target, head-level moving aim (Standard/Advanced), counter-strafe micro-adjust ("Very hard"), turn-and-reset. |
| **"DESTINY" series** | Bigger challenges: rapid-fire Sheriff, stationary→moving flicks, "bullet time", triple-peek duels, expert-peek holds, recorded-bot duels, A-site hold. |
| **Bots** | Humanoid targets with skin, stance (stand/crouch), held weapon, head size, **150 HP default**, reaction delay, aim error, preferred hit region (they shoot back). |
| **Behaviours** | Strafe (speed, distance, turnaround pause, braking), jump, waypoint patrol, shooter, respawn, visibility intervals, lifetime decay, recorded motion takes. |
| **Movement model** | Standing / crouched eye height **1.75 / 1.35 m**, step height 0.45 m. **Movement error disappears below 27.5 % of max speed** (Valorant-style counter-strafe accuracy). |
| **Props** | Smokes (Brimstone, Omen, Astra, Clove, Jett…) that block sight, flashes. |
| **Scoring** | Click score, reaction bonus, stop-to-fire rule, tracking score (time on target, needs invincible targets), expiry penalties. "Moment mode" with 1–3 stars (par time, no damage, personally cleared). |
| **Social** | Accounts, per-scene leaderboards, play counts, votes/ratings, community workshop, cloud scenes. |
| **Editor** | Full scene editor: maps, blocks, templates, spawners, dispatchers, recordings, timeline, playtest, publishing. |

### A3. Where it falls short (= our chance to be better)

| Weakness | Why it matters for improving | ValoAim answer |
|---|---|---|
| **Runs in a browser** | The browser compositor adds 1–3 frames of input→photon delay, and Pointer Lock + `movementX` is not true raw HID data on every browser/OS combination. You train on a slightly different feel than the game. | Native Win32 Raw Input, VSync off, uncapped FPS (B2). |
| **Results = one score** | A score tells you *that* you're bad, not *why*. You can't see whether you overshoot, undershoot, react late or flick slowly. | Per-shot telemetry: overshoot/undershoot, reaction time, flick time vs angle, Fitts throughput, heatmaps, replay (B5). |
| **No training plan** | Players grind random scenarios. That's fun, but it's poor practice. | Built-in routines, weakness detection, adaptive difficulty (B6, Part C). |
| **Difficulty is fixed per scene** | A scene too easy teaches nothing, and a scene too hard teaches panic. | Each scenario auto-tunes size/speed to keep you at ~75–85 % success (B6). |
| **Ads + accounts + loading** | Friction before every rep. | Instant start, offline, zero ads. |
| **Weak sensitivity tooling** | No cm/360 display, no sens finder. | Valorant-exact sens, cm/360 readout, sens-finder scenario (S-01). |
| **Content is mostly community-made** | Quality varies a lot. | A curated, versioned **benchmark set** with rank thresholds, so progress is comparable week to week. |

---

# Part B — ValoAim design spec

## B1. Core principles

1. **Same feel as Valorant.** Same sens scale, same FOV, same hitbox
   proportions, same counter-strafe rule. Anything you learn here should
   transfer directly to the game.
2. **Measure the cause, not just the result.** Log every shot and every mouse
   sample, and tell the player what to fix.
3. **Train at the edge of your ability.** Adaptive difficulty plus deliberate
   practice blocks.
4. **Zero friction.** Launch → scenario in under 2 seconds. Retry with one key (`R`).
5. **Data-driven scenarios.** Every scenario is a JSON file (B7), so new drills
   need no recompile.

## B2. Mouse feel, FOV and latency

### Valorant sensitivity matching

This is the heart of the program (`src/camera.h`, `Camera::applyMouse`).
Every frame, for every raw mouse count that arrived:

```
yaw_deg   += dx * sens * 0.07
pitch_deg += -dy * sens * 0.07
```

- **0.07** is Valorant's real constant: at sens 1.0, one raw HID count turns
  you 0.07 degrees (360° ÷ ~5143 counts). Copy your Valorant sens number into
  the menu slider, and one physical mouse swipe covers the same angle here as
  in-game. Same cm/360.
- **`-dy`**: Windows raw input reports upward motion as *negative* dy, while
  "look up" is *positive* pitch. The minus sign converts between the two.
- **Pitch clamp ±89°**: at exactly ±90° the view matrix degenerates (the
  forward vector becomes parallel to the up vector → NaNs → black screen).
- **Yaw wrap ±180°**: keeps float precision healthy in long sessions.

**cm/360 readout (to add in v2).** Show this next to the sens slider:

```
cm_per_360 = (360 / (sens * 0.07 * DPI)) * 2.54
```

Example: sens 0.35 @ 800 DPI → 14 694 counts → 18.4 in → **46.6 cm/360**.
The settings screen also takes **DPI** so it can show eDPI (= sens × DPI)
and cm/360.

**Scoped sens (v4, Operator/Marshal):** apply Valorant's
`scoped sensitivity multiplier` on top: `effective = sens * scopedMult`, with the
zoomed FOV taken from the weapon file.

### FOV

FOV is locked the way Valorant locks it: **vertical FOV is constant** at

```
vFov = 2 * atan( tan(103°/2) * 9/16 )  ≈  70.53°
```

That's Valorant's 103° horizontal FOV at 16:9. Wider or narrower windows derive
the horizontal FOV from the aspect ratio, so ultrawide just sees more on the
sides, exactly like the game. This matters because sensitivity is defined in
*degrees per count*: if the view covered a different number of
degrees per pixel than Valorant, the sens numbers would not transfer.

### Input path

**Win32 Raw Input** (`src/input.h/.cpp`), registered with `RIDEV_INPUTSINK` on
the GLFW window's HWND and read in a subclassed WndProc. The game **never looks
at the cursor position**, so Windows' "Enhance pointer precision" (mouse
acceleration) cannot affect it, just like Valorant.

### Latency

True "0 latency" is impossible. The USB poll (~1 ms at 1000 Hz) and the
display's scanout are physical floors no software removes. What a native app
*does* remove is the browser's extra compositor/input pipeline delay.

Done in v1:

1. **VSync off by default** (`glfwSwapInterval(0)`). VSync can hold a finished
   frame for up to a full refresh before showing it. Toggleable in the menu.
2. **Every pending WM_INPUT is drained each frame.** `glfwPollEvents()` runs
   the subclassed WndProc synchronously for the whole queue, then the counts
   are consumed once. No packet waits longer than one frame.
3. **Trivial GPU work**, so the GPU is never the bottleneck.
4. **Raw input, not cursor messages**, which skips the Windows ballistics pipeline.

To add:

5. **Borderless / exclusive fullscreen** option (bypasses DWM composition on
   flip-model swapchains).
6. **Hit-test on the input timestamp, not the frame.** On click, record
   `QueryPerformanceCounter` in the WndProc and test the hit against the camera
   orientation *after applying all mouse counts received before that click*.
   This removes the up-to-one-frame error between where you were aiming and
   where the game thinks you aimed. Browser trainers can't do this.
7. **Frame-time graph + FPS cap** (default uncapped; optional cap for
   stable frame pacing).
8. **Audio on a separate low-latency path** (miniaudio, WASAPI shared mode, a
   pre-loaded hit sound) so hit feedback arrives in under 10 ms.

## B3. Scenario catalogue (exact parameters)

**Implementation status (2026-11-06):** implemented in code — C-01, C-02, C-03, C-04, C-05, C-06, C-07, P-02, T-01, T-02, T-05, R-02, R-03, M-01, MAP-01, MAP-02, MAP-03 (Haven A), MAP-04 (Haven B), MAP-05 (Haven C), MAP-06 (Haven C Retake), MAP-07 (Haven A Elevation Ladder), MAP-09 (Ascent B Defuse), MAP-10 (Bind B Defuse), MAP-11 (Haven A Defuse), MAP-12 (Ascent B 1v5 Clutch), MAP-13 (Bind B Head-Height Lane), MAP-14 (Ascent B Anchor Hold). MAP-08 is the spec's Pre-aim Ghost and is still pending — the tactical round deliberately skipped it so the new codes would not collide with the spec. The code's P-03 is a custom drill ("Off-Angle Strafe Confirm", built from a Range screenshot) and is **not** the Comfort-zone Shrink in the table below. The code's R-01 is the Angle Hold drill (the spec's R-02); a pure Reaction Test (spec R-01) is not implemented yet. Still pending: C-08, P-01, T-03, T-04, S-01/02/03, R-01, R-04, M-02, M-03, MAP-08 (Pre-aim Ghost), benchmarks/ranks.

Units: distances in **metres**, angles in **degrees from crosshair**, speeds
in **m/s**, radius = sphere radius. A Valorant head is ~0.12 m radius (the
Sheepu default sphere is also 0.12 m). At 10 m that's ~0.69° radius.

Every scenario has a **default duration of 60 s** unless stated, and every
one logs full telemetry (B5).

### Category 1 — Static clicking (flicks)

Trains: speed + accuracy of one-shot target acquisition.

| ID | Name | Targets | Radius | Distance | Spawn area | Rules |
|---|---|---|---|---|---|---|
| C-01 | **Gridshot** *(v1)* | 3 live | 0.15 | 10 | 3×3 grid, 1.6 m spacing | Hit → respawn in free slot. |
| C-02 | **Gridshot Long** | 3 live | 0.12 | 25 | 3×3, 2.5 m | Same, smaller angular size. |
| C-03 | **Sixshot** | 6 live | 0.15 | 10 | random in ±20°×±10° | Respawn random, min 4° from others. |
| C-04 | **Spidershot** | 1 | 0.20 | 10 | alternates center ↔ random ±35°×±15° | Trains flick-out *and* return-to-center. |
| C-05 | **Wide Flick 180** *(implemented)* | 1 | 0.20 | 10 | random 45°–160° yaw, either side | Big arm flicks. 30 s. |
| C-06 | **Headshot Line** | 1 | 0.12 | 8–30 random | yaw ±40°, **pitch fixed at head height (1.6 m)** | Teaches crosshair placement at head level. |
| C-07 | **Microshot** | 1 | 0.06 | 10 | within 3° of previous target | Tiny corrections, punishes overshoot. |
| C-08 | **Timed Flick** | 1 | 0.15 | 10 | ±30° | Target **expires after 600 ms** (adaptive). Miss/expire = −points. |

### Category 2 — Micro-adjustment & precision

| ID | Name | Description |
|---|---|---|
| P-01 | **Micro Cluster** | 3 tiny targets (r 0.05) within 2° of each other at 10 m. Kill all 3, new cluster spawns 5–10° away. |
| P-02 | **First-Shot Placement** | A single bot head appears 1–4° off-crosshair in your "comfort zone". You have **one shot**: hit = point, miss = round reset. Measures first-bullet accuracy. |
| P-03 | **Comfort-zone Shrink** | Static target. Every hit shrinks radius 5 %, every miss grows it 10 %. Score = smallest radius held for 10 hits. |

### Category 3 — Tracking

Trains: smooth mouse control. **Targets are invincible. Score = time on target.**

| ID | Name | Motion |
|---|---|---|
| T-01 | **Smooth Strafe** | Head-size sphere at 10 m strafing ±3 m at 4 m/s with smooth braking (0.15 s decel). |
| T-02 | **Valorant Strafe** | Humanoid head hitbox doing **A-D strafes at Valorant run speed (6.75 m/s)**, random 0.2–0.8 s direction changes, instant-ish (0.05 s) turnarounds. |
| T-03 | **Air Track** | Sphere jumping on a parabola (gravity 9.81) + lateral drift. |
| T-04 | **Close Swing** | Bot at 4 m wide-swinging past you at 6.75 m/s. Large angular velocity → wrist + arm. |
| T-05 | **Reactive Track** *(implemented — uniform random re-aim, not Perlin)* | Random direction changes every 0.15–0.5 s (Perlin noise path). Measures **reaction to direction change** in ms. |

### Category 4 — Target switching

| ID | Name | Description |
|---|---|---|
| S-01 | **Sens Finder** *(utility)* | Runs C-03 and T-02 for 30 s each at 5 sens values (your sens ×0.8, ×0.9, ×1.0, ×1.1, ×1.2), in random order without showing the value. Recommends the sens with the best combined z-score. |
| S-02 | **Multi-Bot Switch** | 3–5 moving bots (150 HP). Vandal headshot one-taps, body needs 4 hits (B4). Kill all → next wave. |
| S-03 | **Five-Target Challenge** | 5 targets appear at once in ±40°. Clear time measured. Order is free, so it trains path planning. |

### Category 5 — Reaction & peeking

| ID | Name | Description |
|---|---|---|
| R-01 | **Reaction Test** | Screen-center target appears after random 1–4 s delay. Click ASAP. False starts = penalty. Pure reaction (no aim). |
| R-02 | **Angle Hold** | You hold a doorway. A bot peeks out after a random 1–6 s at random speed (wide/jiggle/shoulder), visible for 250–600 ms. Kill before it hides or it "shoots" you. |
| R-03 | **Peeker's Advantage** *(implemented — 200 ms window starts when you stop moving)* | *You* peek with A/D. The bot is pre-placed with 200 ms reaction. Stop (counter-strafe), shoot, re-hide. |
| R-04 | **Anti-flash** | A white flash (full-screen fade) hits from a direction. Turn away within 250 ms to avoid a blind (screen whiteout scaled by how long you faced it). Then turn back and kill. |

### Category 6 — Movement (counter-strafe)

Uses the Valorant movement model: run speed 6.75 m/s, walk ~ 0.55× run,
crouch eye height 1.35 m, stand 1.75 m. **Shots are only accurate if speed <
27.5 % of max** (otherwise apply a random spread cone, ~3–6° depending on speed).

| ID | Name | Description |
|---|---|---|
| M-01 | **Counter-Strafe Drill** | Strafe A↔D. Target appears. Tap the opposite key to stop and shoot. HUD shows your speed at shot time and "STOPPED / MOVING". Score counts only accurate shots. |
| M-02 | **Strafe-Shoot-Strafe** | Bot in front. Rhythm: move → stop → one tap → move. Measures stop-to-shot time. |
| M-03 | **Very Hard Micro** | Counter-strafe + micro target (r 0.06). |

### Category 7 — Map drills (needs maps, v4)

Uses the `.blend` maps in `Maps/` exported to glTF (see B9).

| ID | Name | Description |
|---|---|---|
| MAP-01 | **Ascent B – Pre-aim entry** | Walk the B main route. 7 bots at fixed common angles. Teaches **crosshair placement** on every corner. |
| MAP-02 | **Bind B – Hold** | Hold B site. 6 bots at fixed common angles (Garden, Hookah window, Sand, Center, Elbow, Hell). |
| MAP-03 | **Haven A – Clear** | Clear A site corners in order. 5 bots (A Long, Default Stack, Heaven window, Hell, Short flank). |
| MAP-04 | **Haven B – Clear** | Clear B site corners in order. 5 bots (Ramp Top, Left Corner, Default Box, Back Site, Right Flank). Bot positions measured from the map file. |
| MAP-05 | **Haven C – Clear** | Enter C site from its north edge and clear it in order. 5 bots (Entry Right, West Corner, East Corner, Default Stack, Deep South-West). All floors verified against `havenA.bin` (site floor y ~ 4.3). |
| MAP-06 | **Haven C – Retake** | Spawn deep on C site and retake it. 4 bots (Main Entry, West Corner, Entry Right, Long Approach). The reverse of MAP-05. |
| MAP-07 | **Haven A – Elevation Ladder** *(drill)* | Vertical-flick drill, not a tactical clear: 4 bots, one per verified level — Sewers 1.9 m, A Long 4.2 m, site 7.7 m, Heaven 10.4 m — ordered low to high. |
| MAP-08 | **Pre-aim Ghost** *(pending)* | Same as above, but a transparent "ideal crosshair line" is drawn along the route. Toggle it off once learned. |
| MAP-09 | **Ascent B – Post-Plant Defuse** | Retake from CT against 4 post-plant holders (Default, Market Edge, Lane, Boathouse). 45 s spike timer — clear before it detonates. |
| MAP-10 | **Bind B – Post-Plant Defuse** | Retake from B Hall against 5 post-plant holders (Sand, Center Box, Elbow, Garden, Hookah). 45 s spike timer. |
| MAP-11 | **Haven A – Post-Plant Defuse** | Retake from Heaven against 4 post-plant holders (Default, Hell, Long, Short). 45 s spike timer; you shoot down from 10.35 m. |
| MAP-12 | **Ascent B – 1v5 Clutch** | Last alive vs 5 bots on the site anchors. Reactions staggered 0.55–1.25 s by angle distance; 3 hits = round lost. No timer. |
| MAP-13 | **Bind B – Head-Height Lane** *(drill)* | Crosshair drill down B Long: same 6 anchors as MAP-02, but body shots do not kill — headshots only. Head height drops ~2.3 m along the lane. |
| MAP-14 | **Ascent B – Boathouse Anchor Hold** | Corner-holding from the defender's side: you are the Boathouse anchor, 5 attackers peek in from B Main in order (reactions 0.50–1.05 s). |

**Stars (moment mode):** ★ cleared; ★★ cleared under par time; ★★★ under par,
zero damage taken, all kills yours.

### Category 8 — Benchmarks (the ranked set)

A fixed, never-changing set used for progress tracking and ranks:
`C-01, C-02, C-06, C-07, P-02, T-02, T-05, S-02, R-02, M-01`.
Each has thresholds for **Iron → Bronze → Silver → Gold → Platinum → Diamond →
Ascendant → Immortal → Radiant** (calibrated after collecting data. Store them
in `benchmarks/thresholds.json`). Your overall rank = median rank of the 10.

## B4. Scoring formulas

Consistent and transparent. The results screen always shows the breakdown.

**Clicking (C-xx, P-xx):**

```
hitPts      = 100 per hit
speedBonus  = 50 * clamp(1 - (ttk_ms - 250) / 750, 0, 1)   // full bonus <=250 ms, 0 at >=1000 ms
accMult     = accuracy^2                                   // 90% acc -> 0.81, punishes spam
score       = (sum(hitPts + speedBonus)) * accMult
```

Squaring accuracy is deliberate. Sheepu-style "click fast, ignore misses"
builds a habit that loses fights in Valorant, where a miss costs the round.

**Tracking (T-xx):**

```
onTarget%   = time crosshair inside hitbox / total time
smoothness  = 1 - clamp(RMS(jerk of yaw/pitch) / jerkRef, 0, 1)
score       = 10000 * onTarget% * (0.85 + 0.15 * smoothness)
```

**Bots (S-02, R-xx, MAP-xx):** Vandal-like damage table at <50 m:

| Region | Damage | Kills 150 HP in |
|---|---|---|
| Head | 160 | 1 |
| Body | 40 | 4 |
| Legs | 34 | 5 |

```
score = 1000 * kills * hsRate_bonus(1 + 0.5*headshot%) - 300 * deathsTaken + time bonus
```

**Reaction (R-01):** median reaction ms (median, not mean, so one bad rep doesn't skew it).

## B5. Analytics (the main thing we do better)

### What we record

- **Per mouse sample** (every WM_INPUT, QPC timestamp): `t, dx, dy, yaw, pitch`.
- **Per shot:** `t, yaw, pitch, hit/miss, region, targetId, angular error
  (deg, signed x/y), target angular size, distance`.
- **Per target:** `spawnT, firstMoveT (reaction), firstEnterT, killT,
  startAngle (D), angularRadius (W)`.

Written to `sessions/YYYY-MM-DD_HHMMSS_<scenario>.bin` (compact binary) plus a
summary row in `history.csv`.

### What we show on the results screen

| Metric | How it's computed | What it tells you |
|---|---|---|
| **Reaction time** | spawn → first mouse movement > 0.05° toward target | Visual reaction / attention. |
| **Flick time** | first movement → first entry into hitbox | Raw flick speed. |
| **Correction time** | first entry → shot | Micro-adjustment / confirmation. |
| **Overshoot vs undershoot** | Did the crosshair path cross past the target center along the flick axis before the shot? Count % of each. | **The #1 diagnostic.** Overshoot → sens may be high / you're tense. Undershoot → sens low / hesitant. |
| **Shot error scatter** | 2D plot of shot positions relative to target center | Bias (e.g. always low-left → crosshair placement / grip). |
| **Fitts throughput** | `ID = log2(D/W + 1)`, `TP = mean(ID / MT)` in **bits/s** | One number for "aim skill" that doesn't depend on scenario. Great for long-term graphs. |
| **Direction profile** | Avg flick time + accuracy by 8 directions (polar chart) | Many players are weak flicking up-right or left. |
| **Tracking lag** | Cross-correlate crosshair angular velocity vs target's → delay in ms | How far behind the target you are. |
| **Consistency** | Coefficient of variation of TTK | Low = reliable. |
| **Fatigue curve** | Accuracy per 10 s bucket | Drop at the end → shorten sessions. |

### Replay

Every session can be replayed from the telemetry (it's deterministic: seed +
mouse stream). It shows a ghost crosshair trail with misses marked red. Scrub with
the mouse wheel. Compare two runs side by side (your best vs today).

### Progress page

- Line graph per scenario (score, accuracy, throughput) with a 7-run moving average.
- Benchmark rank card.
- **Weakness detector:** after ≥5 sessions, compare your z-scores across skills
  (flicking, micro, tracking, reaction, switching, movement). It flags the lowest
  two and builds tomorrow's routine around them (Part C).

## B6. Adaptive difficulty

Each scenario exposes 1–3 **tunables** (e.g. radius, target speed, expire
time). A simple **staircase controller** keeps your success rate in the
learning zone:

```
every N=10 targets:
    rate = hits / attempts
    if rate > 0.85:  difficulty += step       // smaller / faster
    if rate < 0.70:  difficulty -= step       // bigger / slower
    clamp to scenario limits
```

- **Practice mode:** adaptive ON. Scores are stored but not ranked.
- **Benchmark mode:** adaptive OFF. Fixed parameters, ranked.
- The final difficulty level is saved and becomes the next session's start point.

## B7. Scenario file format

Scenarios live in `scenarios/*.json`, loaded at startup (use
[nlohmann/json](https://github.com/nlohmann/json) via FetchContent). Example
for **C-03 Sixshot**:

```json
{
  "id": "C-03",
  "name": "Sixshot",
  "category": "clicking",
  "duration": 60,
  "weapon": "click",
  "scoring": "clicking",
  "player": { "move": false, "spawn": [0, 1.75, 0] },
  "targets": [
    {
      "template": "sphere",
      "radius": 0.15,
      "health": 0,
      "color": [0.0, 0.85, 1.0],
      "count": 6,
      "spawn": {
        "type": "angular_box",
        "distance": 10,
        "yaw": [-20, 20],
        "pitch": [-10, 10],
        "minSeparationDeg": 4
      },
      "behaviors": [
        { "type": "respawn_on_kill" }
      ]
    }
  ],
  "adaptive": [
    { "param": "targets[0].radius", "min": 0.06, "max": 0.25, "step": -0.01 }
  ],
  "benchmark": false
}
```

Behaviour types to implement: `strafe {speed, distance, pause, decel}`,
`jump {interval, height}`, `waypoints {points[], speed, dwell}`,
`peek {style: wide|jiggle|shoulder, speed, visibleMs}`, `noise_path {freq, amp}`,
`expire {ms, penalty}`, `shooter {reactionMs, aimErrorDeg, damage}`,
`respawn_on_kill`, `visible_interval {on, off}`.

Map scenarios add `"map": "ascentB"` and spawners with world positions instead
of `angular_box`.

## B8. Architecture

Builds directly on the v1 code. The rule from v1 still holds: **Win32 code only
in `input.cpp` and `main.cpp`; game logic and math stay portable.**

```
CMakeLists.txt          + nlohmann_json, cgltf, miniaudio, implot (FetchContent)
src/main.cpp            init, WndProc subclass, main loop, app state machine
src/input.h/.cpp        RawMouse + keyboard (WASD/R/ESC), QPC timestamps per packet
src/camera.h            yaw/pitch, sens math, view/projection
src/settings.h          Settings (+ DPI, scopedMult, fullscreen, fpsCap, sounds)

src/scenario/
  scenario.h/.cpp       Scenario = loaded JSON → runtime instance; start/update/shoot
  loader.cpp            JSON → ScenarioDef (validation with clear error messages)
  target.h              Target { pos, vel, radius/hitboxes, hp, behaviours[] }
  behaviors.h/.cpp      Strafe, Jump, Waypoints, Peek, NoisePath, Expire, Shooter...
  spawn.h/.cpp          angular_box, grid, near_previous, world_points
  scoring.h/.cpp        ClickingScore, TrackingScore, BotScore, ReactionScore
  adaptive.h/.cpp       staircase controller
  movement.h/.cpp       player controller: accel/decel, 6.75 m/s, 27.5 % accuracy rule

src/analytics/
  recorder.h/.cpp       per-sample + per-shot + per-target logging (ring buffer → file)
  metrics.h/.cpp        reaction/flick/correction, overshoot, Fitts TP, direction profile
  history.h/.cpp        history.csv, bests, benchmark ranks
  replay.h/.cpp         deterministic playback from seed + mouse stream

src/render/
  renderer.h/.cpp       spheres, humanoid hitbox capsules, wall/floor, crosshair
  model.h/.cpp          glTF loading (cgltf) for maps/agents/weapons
  shaders/              basic lit + flat target shader

src/ui/
  ui.h/.cpp             menu, HUD, crosshair editor
  results.cpp           results screen + charts (ImPlot)
  progress.cpp          graphs, benchmark card, weakness detector
  routine.cpp           daily routine player (Part C)

scenarios/*.json        the catalogue in B3
benchmarks/thresholds.json
sessions/, history.csv  created at runtime
```

**Main loop (per frame):**

```
pollEvents()                        // drains WM_INPUT → timestamped sample queue
for each sample in queue:           // in timestamp order
    camera.applyMouse(sample)
    if sample.isClick: scenario.shoot(sample.t, camera)   // hit-test at input time
    recorder.logSample(sample, camera)
scenario.update(now)                // behaviours, spawns, timers, adaptive
render(scenario, camera); ui(); swap()
```

**Hit-testing:** spheres = ray–sphere. Humanoids = 3 capsules (head sphere r
0.12, torso capsule, legs capsule), so each region gives a different damage.
Head test first.

## B9. Using the Agents / Maps / Weapons assets

The repo already contains `.blend` files (`Agents/*`, `Maps/ascentB, bindB,
havenA, havenB`, `Weapons/*`). The C++ app shouldn't parse `.blend`. Do this
instead:

1. Open in Blender → **File → Export → glTF 2.0 (.glb)**. Apply modifiers. For
   maps, also export a simplified **collision mesh** (decimate ~10 %) as a separate
   `*_col.glb`.
2. Put the results in `assets/maps/`, `assets/agents/`, `assets/weapons/`.
3. Load with **cgltf** (single header). Render the visual mesh; raycast shots and
   line-of-sight against the collision mesh (BVH).
4. Agents use their `Simple` variant for bots (fewer polys). Hitboxes are
   **capsules, not the mesh**, same as Valorant.
5. `Weapons/*` → first-person viewmodel (Vandal, Phantom, Sheriff, Operator…),
   cosmetic only. Damage comes from the weapon table in `weapons.json`.

> [!WARNING]
> These look like ripped VALORANT assets (© Riot Games). Keep them for
> **personal, local use only**. Don't redistribute them or publish builds that
> include them. The trainer must still work with the plain sphere/capsule
> targets and simple box maps when `assets/` is missing.

## B10. Roadmap

| Version | Content | Status |
|---|---|---|
| **v1** | Gridshot, Valorant sens/FOV, raw input, best score | ✅ done |
| **v2 – Scenario engine** | JSON loader, behaviours, scoring modules, Categories 1–4 (C, P, T, S), `R` to retry, cm/360 + DPI, hit sound, crosshair editor | next |
| **v3 – Analytics** | Recorder, results breakdown, overshoot/undershoot, Fitts TP, direction profile, history graphs, replay, benchmarks + ranks | |
| **v4 – Movement, bots & maps** | WASD + counter-strafe model, humanoid capsule bots, peeks, shooters, glTF maps, Category 5–7, smokes/flashes | |
| **v5 – Coach** | Weakness detector, auto daily routine, adaptive difficulty everywhere | |
| **v6 – Editor (optional)** | In-app scenario editor writing the same JSON (spawners, waypoints, playtest) | |

Implement in that order. Each version is playable on its own.

---

# Part C — How to improve your aim (the training program)

Aim isn't one skill. It's **six skills**, and each one needs its own drill.
ValoAim tests each one separately so you know which to work on.

| Skill | What it is | Main drills |
|---|---|---|
| **Crosshair placement** | Crosshair already at head height where the enemy will appear | C-06, MAP-01/04 |
| **Flicking** | Fast, accurate snap to a new target | C-01–C-05, C-08 |
| **Micro-adjustment** | Small final corrections, first-bullet accuracy | C-07, P-01–P-03 |
| **Tracking** | Staying on a moving target | T-01–T-05 |
| **Reaction & peeking** | See → shoot fast; hold angles | R-01–R-04 |
| **Movement** | Counter-strafe so the first bullet is accurate | M-01–M-03 |

> In Valorant, **crosshair placement + counter-strafing + first-bullet
> accuracy** win most duels. Huge flicks are rare. That's why the daily plan
> below weights them highest.

## C1. Setup first (once)

1. **Copy your Valorant sens exactly** and enter your DPI. Check cm/360. Most
   pros sit around **30–60 cm/360** (eDPI ~200–400). If you're far outside that,
   run **S-01 Sens Finder** on 3 different days and take the average result.
2. **Then don't touch it.** Changing sens resets your muscle memory. Change at
   most once a month, by at most 10 %.
3. Windows: turn off *Enhance pointer precision* (ValoAim doesn't care, but
   Windows/desktop use builds habits). Mouse polling 1000 Hz. Monitor at its max
   refresh rate.
4. Posture: forearm resting, elbow supported, mouse pad big enough for a full
   180° swipe at your sens.
5. Use the **same crosshair** as in Valorant (ValoAim crosshair editor).

## C2. Daily routine (30 minutes)

Do this **before** you play Valorant, as a warm-up plus practice. ValoAim's
routine player (`routine.cpp`) runs it automatically.

| Block | Time | Drills | Focus cue |
|---|---|---|---|
| 1. Warm-up | 5 min | C-01 Gridshot ×2, T-01 Smooth Strafe ×2 | Relaxed grip. Don't care about score. |
| 2. Crosshair placement | 5 min | C-06 Headshot Line ×3 | Never let the crosshair drop below head height. |
| 3. **Weakness block** | 10 min | The 2 skills flagged by the weakness detector, adaptive ON | Slow and accurate first; speed comes later. |
| 4. Movement / first bullet | 5 min | M-01 Counter-Strafe ×2, P-02 First-Shot ×2 | Stop → shoot. Never shoot while moving. |
| 5. Benchmark | 5 min | 2 benchmark scenarios (rotating so all 10 are covered each week) | Full effort, compare to last week. |

**Then go play Deathmatch / Team Deathmatch for 1–2 games** to transfer it
into the real game. Aim trainer skills that never get used in-game don't stick.

## C3. Weekly plan

| Day | Weakness block focus |
|---|---|
| Mon | Flicking |
| Tue | Tracking |
| Wed | Micro + first bullet |
| Thu | Reaction / angle hold |
| Fri | Movement + map pre-aim |
| Sat | Weakest skill (from detector) – double block |
| Sun | **Rest** or only warm-up. Your brain consolidates motor skill during rest. |

## C4. How to practice (the rules that make it work)

1. **Accuracy first, then speed.** Stay at **≥ 90 % accuracy** on clicking
   drills. If you're below that, slow down. The scoring formula (accuracy²)
   punishes spam on purpose.
2. **Train at 70–85 % success.** That's what adaptive difficulty does. If it
   feels easy, it's not teaching you anything.
3. **Read the diagnostics after every run** (B5):
   - **Mostly overshooting** → relax your grip, slow the end of the flick. If it
     stays above 60 % across all flick drills for a week, try sens −5 %.
   - **Mostly undershooting** → commit to the flick, use more arm. If
     persistent, try sens +5 %.
   - **Reaction > 250 ms** → drills with shorter expire times (C-08, R-02), and
     get enough sleep (it really affects this).
   - **Slow correction time but fast flicks** → micro drills (P-01, C-07).
   - **Tracking lag > 80 ms** → T-05 Reactive Track. Watch the target's
     *center*, not your crosshair.
   - **Shot scatter biased low** → crosshair placement drills (C-06).
   - **Weak direction in the polar chart** → do C-03 with spawn limited to that
     direction (practice mode filter).
4. **Short and focused beats long and tired.** If the fatigue curve shows
   accuracy dropping in the last third of runs, stop. 30 focused minutes beat
   2 sloppy hours.
5. **One cue per block.** Think about one thing (e.g. "stop before shooting"),
   not five.
6. **Don't chase high scores every run.** Practice mode is for learning, and
   benchmark mode is for measuring.

## C5. Measuring progress

- Look at the **7-run moving average**, not single runs. Daily noise is
  ±10 %.
- Expect visible gains in **2–3 weeks**, then plateaus. On a plateau, change the
  stimulus: a different drill for the same skill, a smaller target, or a new
  map drill.
- The **benchmark rank** and **Fitts throughput** graph are your long-term
  truth. If throughput goes up month over month, your aim is getting better,
  whatever your Valorant rank says this week.

## C6. Checklist: carrying it into Valorant

- [ ] Same sens, same crosshair, same FOV feel.
- [ ] Warm up with Block 1 + 2 before ranked.
- [ ] In Deathmatch, focus on **one** cue from today's weakness block.
- [ ] Counter-strafe every shot (M-01 habit).
- [ ] Crosshair at head height while walking around corners (C-06 / MAP habit).
- [ ] Review: if you lost duels by being late → reaction/angle-hold days. If
      you lost them by missing → micro/first-bullet days.

---

# Part D — Personal plan from your tracker (V9pujari#0000)

Source: your tracker.gg profile, **V26 Act VI Competitive** (61 matches, 35 h,
Platinum 2). This section turns those numbers into a diagnosis and an exact
ValoAim + in-game plan. Part D2 says how ValoAim should do this automatically
for anyone.

## D1. Diagnosis: what your numbers say

### Your stats vs what they mean

| Stat | Yours | Percentile | Read |
|---|---|---|---|
| Win % | 55.7 % | Top 22 % | You help win. Game sense / utility are fine. |
| KAST | 71.5 % | Top 39 % | You stay useful in rounds. |
| Headshot % | 22.3 % (Vandal 33 %, Phantom 29 %, Classic 38 %) | Top 41 % | **Your aim *can* hit heads.** Not the main problem. |
| Body / Legs | 71.5 % / 6.5 % | — | Body-heavy: crosshair often a bit **below head height**, or you keep spraying after the first bullet. |
| K/D | 0.99 (last 20: **0.89**) | Bottom 47 % | You lose about half your duels, and it's getting worse. |
| ADR | 134.9 (last 20: **127**) | Bottom 35 % | Low damage for a Plat player with a 55 % win rate. |
| **DDΔ / round** | **−10** | **Bottom 28 %** | **Your worst stat.** You take more damage than you deal: **you're getting shot first** or losing the first-bullet trade. |
| ACS | 208 | Bottom 41 % | Follows from low ADR. |
| HS % per match | 8 % → 38 % | — | **Very inconsistent.** Some games your aim is on, others it isn't. Points to warm-up and focus, not raw skill. |
| First Bloods | 110 in 61 games (~1.8/game) | — | You do take opening fights. |

### The key insight

High HS% on rifles + good win rate, but **negative DDΔ and low ADR**, means
**you can aim, but you lose the *timing* of duels.** The enemy usually lands
the first damage. In Plat, that almost always comes down to these three things,
in this order:

1. **Crosshair placement / pre-aim.** If your crosshair isn't already on the
   angle at head height, you need a flick, and the enemy who was pre-aimed wins.
   (The 71.5 % body hits support this.)
2. **Shooting while moving / not counter-strafing.** Your first bullet misses,
   theirs doesn't. That gives you DDΔ minus and lots of body/leg hits from
   inaccurate spray.
3. **Peeking badly.** Wide-swinging into held angles, re-peeking the same spot
   and holding angles too close.

The swing from **8 % to 38 % HS** says you also need a **fixed warm-up** so your
first rounds aren't your worst.

### Agents and maps

- **Duelist WR 62.5 % (Reyna/Phoenix) > Controller 52 % (Clove, your most
  played).** On Clove your K/D is 0.94. You're playing a lot of duels as a
  controller that you then lose. On controller, play for trades, not first contact.
- **Best:** Haven 85.7 %, Corrode 66.7 %, Sunset 63.6 %. **Worst:** Abyss 36.4 %,
  Bind 45.5 %. Map drills should prioritise **Bind** (you already have
  `Maps/bindB.blend`) and Abyss (no asset, so use generic angle drills + in-game
  custom games).

## D2. Your goals (6 weeks)

| KPI | Now | Week 3 target | Week 6 target |
|---|---|---|---|
| **DDΔ / round** | −10 | 0 | **+10** |
| ADR | 135 | 142 | **150** |
| K/D | 0.99 | 1.02 | **1.08** |
| HS % | 22.3 % | 24 % | **26 %** |
| HS % spread (min–max of last 10 games) | 8–38 | 14–34 | **18–32** |
| Body % | 71.5 % | 69 % | **66 %** |

**Most important: DDΔ.** If DDΔ turns positive, K/D, ADR and rank follow.
Check these every Sunday on tracker.gg (Last 20 matches) and write them into
ValoAim's **Progress → Tracker check-in** (D5).

## D3. Your daily routine (35 min, replaces C2 for you)

Do it **before every ranked session**. The goal is to kill the "bad first
games" variance.

| # | Time | ValoAim drill | Why (for *your* stats) | Pass condition |
|---|---|---|---|---|
| 1 | 4 min | C-01 Gridshot ×2 | Warm up hands, relaxed grip | Accuracy ≥ 90 % |
| 2 | **7 min** | **C-06 Headshot Line ×4** | Fixes body-heavy hits. Crosshair lives at head height | ≥ 85 % hits, scatter not biased low |
| 3 | **7 min** | **M-01 Counter-Strafe ×3, M-02 Strafe-Shoot-Strafe ×2** | Stops you missing the first bullet while moving → DDΔ | ≥ 90 % of shots "STOPPED" |
| 4 | **6 min** | **P-02 First-Shot Placement ×3** | One-tap / first bullet is what wins Vandal duels | ≥ 70 % first-shot hits |
| 5 | **6 min** | **R-02 Angle Hold ×2, R-03 Peeker's Advantage ×2** | Learn to win the timing of the duel (you're shot first too often) | Reaction ≤ 220 ms median, deaths ≤ 3 |
| 6 | 5 min | T-02 Valorant Strafe ×2 | Track A-D strafing enemies in close fights | On-target ≥ 55 % |

Then **in-game, 1 Deathmatch (or 2 TDM)** with **one focus cue per day**:

| Day | ValoAim focus block (+10 min) | DM focus cue |
|---|---|---|
| Mon | C-06 + MAP-01 pre-aim | "Crosshair on head height at every corner, never on the floor." |
| Tue | M-01 / M-03 | "Stop → shoot. Never shoot while moving." |
| Wed | P-02 + C-07 Microshot | "First bullet to head, then burst max 3–4." |
| Thu | R-02 / R-03 | "Jiggle for info, then shoulder/wide peek. Never re-peek the same spot after a kill." |
| Fri | **MAP-02 Bind B Hold** + Abyss angle holds (custom game) | "Hold off-angles, not the obvious close spot." |
| Sat | Weakest drill of the week (from ValoAim weakness detector) | Vandal only, tap/burst |
| Sun | Rest + tracker check-in (D5) | — |

## D4. In-game habits that move *your* numbers

These aren't aim drills, but they fix DDΔ faster than any drill:

1. **Spray control:** with Vandal/Phantom at > 15 m, **tap or burst 2–3**, reset,
   repeat. Full sprays create your 71.5 % body hits.
2. **Pre-aim every common angle** while walking (MAP-01/04 trains this). Your
   crosshair should move *before* you see the enemy, not after.
3. **On Clove (controller):** don't take first contact. Smoke → let the duelist
   entry → trade. Your duelist WR (62.5 %) says take fights when *that's your role*.
   Consider queuing duelist more in solo ranked.
4. **After you get a kill, reposition.** Many Plat deaths are re-peeking the same
   angle and getting traded, which shows up as damage taken.
5. **First 2 rounds of a match:** play safer (your HS % swings mean your aim may not be
   warm yet). The routine in D3 is there to remove this.
6. **Abyss/Bind:** learn 2 default setups per site and the common off-angles. Watch
   1 pro VOD per map (focus on where they hold, not on utility).

## D5. Feature spec: "Tracker coach" in ValoAim (so it does this for you)

ValoAim should generate Part D automatically for any player.

### Input

tracker.gg's Valorant data requires an approved API key and the player having
their profile public. So v1 of this feature uses **manual entry / paste**
(always works, offline). An optional API mode comes later, only via the official
[Tracker Network Developer API](https://tracker.gg/developers) with the user's
own key.

`profile/tracker.json` (filled in **Progress → Tracker check-in**, one entry per week):

```json
{
  "player": "V9pujari#0000",
  "date": "2026-11-04",
  "rank": "Platinum 2",
  "matches": 61,
  "winPct": 55.7, "kd": 0.99, "adr": 134.9, "acs": 208.4,
  "ddDelta": -10, "kast": 71.5,
  "hsPct": 22.3, "bodyPct": 71.5, "legPct": 6.5,
  "hsPctLast10": [16, 19, 19, 8, 17, 17, 26, 27, 19, 23],
  "firstBloods": 110,
  "weapons": { "Vandal": { "hs": 33 }, "Phantom": { "hs": 29 } },
  "roles":   { "Controller": { "wr": 52.0, "kd": 0.94 }, "Duelist": { "wr": 62.5 } },
  "maps":    { "Haven": 85.7, "Bind": 45.5, "Abyss": 36.4 }
}
```

### Rules engine (`src/coach/tracker_rules.cpp`)

Each rule maps a stat pattern to a weakness and a set of drills. The plan is
built from the top 3 triggered rules (by severity).

| Rule | Trigger | Weakness | Drills added | DM cue |
|---|---|---|---|---|
| R1 | `ddDelta < 0` | Losing duel timing | M-01, M-02, R-02, R-03, P-02 | "Stop → shoot" |
| R2 | `bodyPct > 68` | Crosshair low / spraying | C-06, MAP-01, P-02 | "Head height" |
| R3 | `stdev(hsPctLast10) > 7` | Inconsistent | Fixed warm-up, mandatory before ranked | "Warm-up first" |
| R4 | `hsPct < 18` | Raw precision | C-07, P-01, P-03 | "First bullet head" |
| R5 | `adr < 140 && winPct > 52` | Passive / low impact | R-03, S-02, T-02 | "Take good fights" |
| R6 | `legPct > 9` | Crosshair too low / moving shots | C-06, M-01 | "Head height" |
| R7 | `kd(role) - kd(other) > 0.1` | Role mismatch | (advice text only) | — |
| R8 | `mapWr < 45` for ≥ 5 games | Map knowledge | MAP-xx for that map if asset exists | Map-specific |

For your profile this triggers **R1 (severity high), R2, R3, R5, R7, R8 (Abyss,
Bind)**. That produces exactly the D3 routine above.

### Feedback loop

- Every weekly check-in, ValoAim plots tracker KPIs (**DDΔ, ADR, HS %, body %**)
  **next to** ValoAim drill scores on the same timeline. Then you can see
  whether drill improvement actually transfers to ranked.
- If a drill improves but the linked tracker stat doesn't after 3 weeks, the coach
  swaps it for another drill on the same skill and shows an in-game habit tip
  instead (D4). Sometimes the problem isn't aim.
- When a KPI hits its target, the rule's severity drops and the next weakness
  takes the main slot.

### Roadmap placement

Add to **v5 – Coach** (B10): tracker check-in form, `tracker.json`, rules
engine, combined progress chart.

---

## Project layout (current)

The app grew well past the original gridshot-only v1. The layout below is
what the code actually looks like now (26 source files).

```
CMakeLists.txt        FetchContent: GLFW 3.4, Dear ImGui v1.93.0 (sources added manually)
src/main.cpp          init, WndProc subclass, main loop, MENU/PLAY/RESULTS machine
src/input.h/.cpp      RawMouse — Win32 Raw Input, isolated here
src/camera.h          yaw/pitch, sens math, hand-rolled view/projection (no glm)
src/trainer.h/.cpp    all range drills: ray-vs-disc hitscan, timer, stats (portable)
src/preaim_course.h/.cpp   3D map workshops: bot placement, reaction timers, HUD telemetry
src/map_mesh.h/.cpp   .bin map/bot loader (VMAP/VBOT), floor-height queries, collision
src/player.h/.cpp     WASD movement, gravity, floor snapping, crouch/walk speeds
src/renderer.h/.cpp   wall + billboard targets + map/bot rendering (portable OpenGL)
src/routine.h/.cpp    V9pujari daily routine: 6 blocks, 18 sets
src/scenarios.h/.cpp  scenario registry: id, name, category, defaults (see below)
src/ui.h/.cpp         ImGui: settings menu, HUD, crosshair, results
src/settings.h        one shared Settings struct
src/audio.h/.cpp      Win32 waveOut hit sounds (procedural, no audio files)
src/gl_loader.h/.cpp  minimal GL 3.3 loader (Windows opengl32 is only GL 1.1)
assets/maps/*.bin     exported map geometry (ascentB, bindB, havenA, havenB)
assets/models/bot_jett.bin   exported Jett model (14,659 verts)
best.txt              created at runtime next to the exe; stores best score
Agents/ Maps/ Weapons/  Blender source assets (see B9) — not committed to git
```

### How scenarios are registered (the part most people get wrong)

Scenarios are **hardcoded, not JSON-driven** (the B7 JSON loader is still
pending). To add one you touch four places:

1. `src/scenarios.h` — append a new `ScenarioId` **before `Count`**. Existing
   ids must never move: `best.txt` keys scores off the id number.
2. `src/scenarios.cpp` — append a matching `ScenarioInfo` to `kScenarios`
   in the **same order** as the enum (`getScenarioInfo` indexes by id).
   Menus iterate `getAllScenarios()`, so a new entry shows up automatically.
3. `src/trainer.h/.cpp` — add an `initXxx()` function, a case in `start()`,
   and, if it respawns or moves, cases in `spawnRandomTarget()` / `update()`.
4. `src/routine.cpp` — only if the drill belongs in the daily routine.

Map workshops are different: the course (bots, spawn, yaw) lives in
`src/preaim_course.cpp` (`initAscentB` / `initBindB` / `initHavenA` /
`initHavenB` / `initHavenC` / `initHavenCRetake` / `initHavenAElevLadder` /
`initAscentBDefuse` / `initBindBDefuse` / `initHavenADefuse` /
`initAscentBClutch` / `initBindBHeadHeight` / `initAscentBAnchorHold`),
the dispatch in `main.cpp` `enterPlay()`, and the geometry in
`assets/maps/*.bin`.

### The .bin format (what the maps actually are)

The B9 glTF/cgltf pipeline was never built. Maps and bots use a custom
binary format written by a Blender export script (see `map_mesh.h`):

- Header (40 bytes): magic `"VMAP"` or `"VBOT"`, version (1), numVerts,
  numTris, min/max X/Y/Z bounds (floats).
- Vertices (32 bytes each): position xyz, normal xyz, color rgba, flag
  (1 = floor, 2 = head), padding.
- Triangles: 3 × uint32 indices each.
- Coordinate mapping: `game = (blend_x, blend_z, -blend_y)` (Blender Z-up
  to game Y-up). Bot positions in `preaim_course.cpp` use game coords.

The loader (`map_mesh.cpp`) checks the magic, the version, and caps
numVerts/numTris at 2M each plus a file-size check before allocating, so a
corrupt header cannot crash the game with a huge allocation.

### Map validation (2026-11-06)

All four `.bin` files were validated against the code: header, exact file
size (`40 + verts*32 + tris*12`), zero NaN/inf, zero non-unit normals,
bounds match the header, and every bot/spawn floor checked with the exact
`getFloorHeight` rule (highest floor-flagged surface at or below foot level
+ 0.75 step-up). The Blender suspicion was right — and the `.bin` agrees
with the `.blend` on nearly every point, so the `.bin` is trusted.

| file | verts / tris | size | floor tris | result |
|---|---|---|---|---|
| ascentB.bin | 206,190 / 192,209 | 8,904,628 B exact | 44,659 | clean (67 degenerate slivers, harmless) |
| bindB.bin | 189,699 / 171,861 | 8,132,740 B exact | 37,286 | clean |
| havenA.bin | 435,637 / 359,902 | 18,259,248 B exact | 82,424 | clean (17 degenerate slivers) |
| havenB.bin | 338,407 / 247,166 | 13,795,056 B exact | 51,893 | clean (9 degenerate slivers) |

Fixes applied (old → verified `.bin` value):
- **Ascent B:** spawn (-20, 1, -50) → (-23, 1, -50), yaw -28° → -10° (old
  spot is a mesh hole — no floor at all, player stuck); Green Box Top
  2.2 → 2.95 (was sunk 0.74 m); Boathouse Front 0.0 → -0.83 (floated
  0.83 m); CT Arch (-4, 1, -88) → (-6, 1, -88), yaw 180° → -156° (old
  spot is a hole; only a roof at 16.44 above it).
- **Bind B:** spawn y 3.0 → 3.96 (old y is 0.96 above the floor — beyond
  the 0.75 step-up, so the player falls through the world); Garden Vista
  3.0 → 5.33 (buried 2.3 m; the game floor at the exact point is 5.334,
  a narrow raised strip along x = 102 — the flat Garden floor just west
  is 5.0); Hookah Window 6.5 → 5.0 (no sill exists at 6.5 — floated
  1.5 m); Sand Container 3.0 → 3.08; Center Box 3.0 → 3.5 (the 3.5
  surface is a real ~4 m box-top plateau, verified on a 0.5 m probe).
- **Haven A:** spawn (90, 0, -30) → (90, 2.56, -36), yaw 11° → 15°
  (old spot has no floor at all — the player falls through the world;
  the only triangle there is a non-floor slope at -2.97); A Long
  0.0 → 4.21 (buried 4.2 m); Default Stack (105, 1, -75) →
  (102, 7.73, -76) (old spot is a true void — no triangle at all; the
  site floor is a sloped surface at ~7.7, not ~4.2); Heaven 6.0 → 10.35
  (buried 4.35 m); Hell (118, 1, -85) → (113, 6.96, -85) (no ground
  under Heaven — the site floor ends at x ~ 116); Short 0.0 → 1.91
  (buried 1.9 m under the Sewers floor).
- **Haven B:** spawn y 1.0 → 1.06; Site Left Corner 3.0 → 2.87 (the bot
  floated 0.13 m above the 2.869 floor). Everything else was already
  correct.

New scenarios (all floors verified the same way):
- **MAP-05 Haven C: Long to Site Clear** — C site is the large flat area
  at y ~ 4.3 in `havenA.bin` (x 71–113, z -143..-89, ~784 m²). 5 bots,
  spawn on the site's north edge.
- **MAP-06 Haven C: Site Retake** — the reverse: spawn deep on the site,
  clear the entries. 4 bots.
- **MAP-07 Haven A: Elevation Ladder** — vertical-flick drill: Sewers
  1.9 m → A Long 4.2 m → site 7.7 m → Heaven 10.4 m, low to high.

Still open (not code bugs — the `.bin` files themselves):
- **Floor meshes have small holes.** A straight-line walk from any spawn
  hits a gap within 1–17 m (1–3 m holes scattered across ramps and
  corridors). The player must weave around them; bots and spawns were all
  placed on solid points, so the courses still work. Filling the holes
  needs a re-export (no Blender exporter script exists yet matching
  `map_mesh.h`).
- **No Ascent A or Bind A geometry.** `ascentB.bin` / `bindB.bin` contain
  only the B half of each map (one flat component each — the B site), so no
  A-site course can be built from them. The `.blend` sources may hold the
  full maps; that needs a new export.
- **Haven C identity is by elimination** (A = the 7.7 m area, B = the 3.0 m
  west area at `havenB.bin`'s site level, so the 4.3 m area is C). Confirm
  the site letter in-game.
- **Haven A walk to the site is broken in the `.bin`:** the A Long ramp
  has a hole at (92, -45) and the Short route crosses a pit — the player
  can shoot every bot from the spawn (all within 57 m), but cannot walk
  the full route. Same root cause as the holes above.
- **No CT floor for a Haven B or Haven C defuse (2026-11-06):** a probe
  of `havenB.bin` shows no floor beyond the Back Site Corner (z < -108
  is a void), and a probe of `havenA.bin` shows no floor south of the C
  site (z < -135 is a void) — so neither site has a retake entry to
  spawn on. Their defuse scenarios were NOT built rather than faked.
  (Haven A's defuse spawns on Heaven instead — the classic retake
  perch — because the CT side north of the site is also a void.)
- **No Market corridor in `ascentB.bin` (2026-11-06):** the east site
  edge at x = -8 has floor (y = 1.0) but x = 0 is a void — the Market
  door position is the east site edge, not a corridor. The "Market
  Door" bot stands there; it cannot stand "in Market".

### Tactical grounding (2026-11-06)

The original bot placements (made by the other AI) were random: bots
stood anywhere, faced any direction, and the player spawned anywhere.
This round re-grounded every course in how real players actually play
these sites — researched from pro positioning guides and VOD-breakdown
articles, not invented. Two rules now hold everywhere:

1. **Every bot stands on a named spot a real player would hold** (the
   anchor, the plant spot, the retake lane, the deep corner) and **faces
   the angle it is holding** — yaw = atan2 toward the entry the attacker
   comes from. The old code had 6 of 7 Ascent bots facing INTO the site
   (away from the player) and 3 of 6 Bind bots facing the wrong way.
2. **Every spawn is a real attack/retake entry** (B Main, B Long, A Long
   ramp, B corridor, CT, Hall, Heaven, Boathouse) with the yaw facing
   the first angle to clear.

Sources (all read for this round; every tactical claim below traces to
one of them):

- Ascent B callouts & holds: https://esportsdriven.com/gaming/guides/111/valorant-ascent-map-callouts-guide-with-images — https://www.gearupbooster.com/blog/valorant-ascent-map-guide-pro-strategies.html — https://safezone-totoking.mystrikingly.com/blog/valorant-ascent-map-guide
- Bind B callouts & holds: https://juked.gg/valorant/game-guide/valorant-bind-map-breakdown-best-plays-and-agent-synergies/ — https://www.strafe.com/articles/read/valorant-bind-guide/ — https://esportsdriven.com/gaming/guides/120/valorant-map-guide-bind
- Haven A/C callouts & holds: https://esportsdriven.com/valorant/guides/122/valorant-haven-map-callouts-guide-with-images — https://www.cd44-vip.net/article/valorant-haven-guide-bangladesh-three-site-.html — https://overgear.com/guides/valorant/maps-list/
- Crosshair placement: https://metabot.gg/en/valorant/guides/valorant-crosshair-placement-fundamentals — https://sassy.wtf/blog/valorant-crosshair-placement — https://1v9.gg/blog/how-to-improve-crosshair-placement-in-valorant — https://boosteria.org/guides/valorant-crosshair-placement-guide-head-level-angles-2026
- Post-plant / retake / clutch: https://bo3.gg/valorant/articles/how-to-play-post-plant-in-valorant-positioning-and-utility-after-the-spike — https://cosmocheats.com/game-guides/valorant/ — https://valocheck.com/faq/

What the sources say, and where each bot sits:

- **Ascent B:** the anchor holds Boathouse / Back Site (most cover,
  hardest to clear, the post-plant spot); Market watches B Main crossers
  from safety; CT / Connector is the fastest retake entry; Triple boxes
  are the safest plant spot (plant beside the boxes, parallel to
  Boathouse); Lane is the shortest B Main → site path; attack smokes
  Market + CT, clears Logs / Boathouse, closes the Market door, then
  post-plants back toward B Main / B Stairs. Retake: attackers hide in
  Back Site, clear B Main / Market first.
- **Bind B:** defenders hold Elbow + Garden, Tube in the middle of the
  site; Hookah / Window is the elevated entry, hard to clear; Split B
  pressures Long + Hookah together, smokes Elbow, clears Tube and the
  close corners; B Backsite + Platform are the post-plant spots; retake
  doors are Hookah / Baths.
- **Haven A:** the default plant is visible from Short / Long; Heaven is
  elevated and sees Long + Short; Hell sits under Heaven, has cover
  from Short, sees Long, and crossfires with Cubby on the post-plant;
  Cubby is the deep corner post-plant spot; CT is the main retake
  entrance. Entry order: clear Long / Short, smoke Heaven + link,
  entry clears close left / right, carrier plants; then hold Heaven /
  link and watch the flank.
- **Haven C:** Long control alone is not enough if defenders hold
  Garage; defenders play Platform + C Window.
- **Crosshair placement:** head level always — height follows the ground
  the enemy stands on (raise on ramps / boxes, lower looking down);
  pre-aim the exact pixel at the corner edge, not the wall; do not hug
  walls (a wider hold gives more reaction time); clear one angle at a
  time ("slice the pie"); stop before shooting.
- **Post-plant / retake / clutch:** the spike detonates **45 s** after
  plant; defuse takes **7 s** with a half checkpoint at **3.5 s**;
  post-plant spread into connected positions (one holds the closest
  retake entrance, one watches the spike from long, one covers the
  flank); retakers regroup, smoke the distant angle, clear close
  corners in order of threat; a clutch is won by isolating 1v1s — never
  face-check two angles at once.

Placement audit (old → new; every Y is still a verified `.bin` floor):

| course | bot | old | new | why |
|---|---|---|---|---|
| MAP-01 Ascent B | Lane Close | yaw 45° | yaw 170° | holds the Lane entry — must face B Main, not the site |
| MAP-01 | Green Box Top | yaw 45° | yaw 170° | elevated, holds Lane + site from above |
| MAP-01 | Market Door Peeker | (-23, 1, -66.5), yaw 90° | **Market Door Edge (-8, 1, -70), yaw -143°** | old spot was mid-site, mislabelled; real Market-side floor is the east edge (x = -8; x = 0 is a void) |
| MAP-01 | Default Box | yaw 45° | yaw 150° | Triple — safest plant spot, holds Lane |
| MAP-01 | Boathouse Front | yaw 30° | yaw 138° | back-site anchor, hardest to clear |
| MAP-01 | Boathouse Stairs | yaw 60° | yaw 126° | deep back alley, post-plant spot |
| MAP-01 | CT Arch | yaw -156° | unchanged | already faced the entry — the long retake angle |
| MAP-02 Bind B | Garden / Hookah / Hell | yaw 20° / 0° / 270° | yaw 143° / 103° / -134° | faced into the site / due west; now face B Long |
| MAP-02 | Sand / Center / Elbow | yaw 160° / 180° / 200° | yaw -156° / -173° / -170° | same spots, exact facing of the Long |
| MAP-03 Haven A | A Long Corner | yaw 170° | yaw -165° | faced 25° off the entry |
| MAP-03 | Default / Heaven / Hell / Short | — | unchanged | already faced the A Long ramp |
| MAP-04 Haven B | Site Right Flank | (12, 3, -88), yaw 55° | **Site East Corner (44, 3, -90), yaw -146°** | old spot was 2.8 m from Site Left Corner — two bots in one corner; the site really extends east to x ~ 44 |
| MAP-04 | Default Box / Back Corner | yaw -150° / 180° | yaw -171° / 175° | exact facing of the corridor entry |
| MAP-05/06/07 | all bots | — | unchanged | yaws already faced the entry/spawn exactly |

New scenario types (MAP-09..MAP-14; MAP-08 stays reserved for the
spec's Pre-aim Ghost):

- **Defuse (MAP-09/10/11):** post-plant retakes with a real **45 s**
  spike timer. New engine mechanism: `PreAimCourse` gained
  `timeLimitSec` + `startTime` + `isFailed()` / `failedByTimer()`;
  `main.cpp` ends the course when the timer expires, and the results
  screen says "SPIKE DETONATED — DEFUSE FAILED" instead of "COURSE
  COMPLETE". The HUD shows the countdown (red under 10 s). Holders are
  pre-aimed, so their reactions are 0.38–0.45 s — slightly faster than
  the attack courses.
- **1v5 clutch (MAP-12):** five bots, reactions staggered by angle
  distance (0.55–1.25 s), and a new **3-hit damage limit**
  (`maxDamage` + `failedByDamage()`; each hit is ~35 HP of a 100 HP
  round). Fair because the engine only runs a bot's reaction timer
  while the bot is in the player's sight cone — a bot you are not
  looking at cannot fire — so the clutch trains the real skill:
  isolate 1v1s, never face two angles at once. There is no timer; a
  clutch is won by patience.
- **Crosshair placement (MAP-13):** the Bind B Long lane as a
  headshots-only drill (`headshotsOnly`: a body hit does not kill, it
  is counted as wasted and shown on the results screen). Head height
  drops ~2.3 m along the lane (Garden 5.33 m → site 3.0 m), so the
  crosshair must drop with it. Scoring already exists: the results
  screen tracks headshot rate and average crosshair vertical error —
  those two numbers ARE the score here. No timer, no damage limit.
- **Corner-holding (MAP-14):** the player IS the Boathouse anchor and
  five attackers peek in from B Main in order (reactions staggered
  0.50–1.05 s), every bot facing the Boathouse. A hold drill, not a
  round: no timer, no damage limit.

Verification (2026-11-06, Linux harness with the real `MapMesh`
loader — `/tmp/harness/harness2.cpp`, not in the repo): **600 checks,
0 failed.** All 13 courses: spawn on a floor, spawn yaw faces bot 1
within 1°, every bot on its floor, every bot within 65 m, full clear
without crashing. Timer: no fail at 44.9 s, `failedByTimer` at 45.1 s,
resets on restart. Clutch: damage 0 before 0.55 s, 3 hits by 1.0 s →
`failedByDamage`. Headshots-only: body shot returns 1, bot stays
alive, `bodyHitsWasted` = 1, headshot kills; control body shot on the
normal MAP-02 still kills. Old courses have no timer / damage limit /
headshots-only — no behaviour change.

### Code rules (and where they are broken)

- **Intended:** Win32 code lives only in `input.cpp` and `main.cpp`. The
  game logic (`trainer`, `preaim_course`) and math (`camera`) are portable.
- **Broken right now:** `audio.cpp` and `gl_loader.h` are not `#ifdef _WIN32`
  guarded (unlike `input.cpp`), and `map_mesh.h` was missing
  `#include <cstdint>` (compiles on MSVC only via transitive includes).
  Both are small fixes, not done yet.

### Known gaps (honest list, 2026-11-06)

- **Map validation is done (2026-11-06) — see the section below.** The old
  "Haven A bot heights are suspect" item is closed: every bot/spawn Y in
  the four courses was checked against the .bin floor data with the exact
  `getFloorHeight` rule (flag-1 triangle, or first-vertex normal y > 0.65),
  and all wrong values were fixed. Remaining .bin issues are holes in the
  floor meshes (a straight-line walk from spawn hits a gap within 1–17 m
  on every map — the player must weave around 1–3 m holes) and the
  missing Ascent A / Bind A geometry (see below).
- **Tactical grounding is done (2026-11-06) — see "Tactical grounding"
  above.** Every bot now stands on a named spot a real player would hold
  and faces the entry it is holding; 6 new scenarios (MAP-09..MAP-14:
  defuse × 3, 1v5 clutch, headshots-only lane, anchor hold). What could
  NOT be built: Haven B / Haven C defuse (no CT floor in the .bins to
  spawn a retake on) and a Market-corridor bot (the corridor is a void
  in `ascentB.bin`).
- **Scoring does not match the B4 formulas yet** (no accuracy² multiplier,
  speed bonus is a 40/20 step instead of the formula, tracking score is
  `onTargetTime × 250` instead of the B4 expression).
- **Overshoot/undershoot metric is wrong:** it measures the world-X offset
  of target[0] only, with no flick direction, so a leftward flick
  overshooting left counts as "undershoot".
- **`avgReactionMs()` always returns 210.0** (the counters are never
  accumulated). Either wire it up or delete the metric.
- **Movement speed is inconsistent:** `player.h` run speed is 5.4 m/s but
  the trainer sim uses 6.75 m/s; the 1.85 m/s deadzone is 27.5 % of one
  and 34 % of the other.
- **Range wall is a 2×2 m quad** with no scale; Gridshot Long targets sit
  at 25 m while the wall renders at `wallDist` (10 m).
- **Scenario parameters differ from B3 in places** (Sixshot ±22° not ±20°,
  Spidershot radius 0.15 not 0.20, Headshot Line 9–16 m not 8–30 m,
  First-Shot Placement 2.9–8° not 1–4°, Angle Hold peek 380 ms fixed not
  250–600 ms). Either update B3 or retune the code — pick one source of
  truth per scenario.

### Fixes applied in this round (2026-11-05)

1. **Spawn yaws fixed** — all three map courses spawned the player facing
   away from every bot (Ascent −135° → −28°, Bind 180° → −37°,
   Haven A 180° → +11°, computed from spawn/bot positions).
2. **Stray crosshair line deleted** (`ui.cpp` drew a fifth line ~280 px left
   of centre in every range drill).
3. **Map loader hardened** — version check, 2M vert/tri caps, file-size
   check before allocating; missing `#include <cstdint>` / `<cstring>` added.
4. **Bot reaction timer fixed** — the HUD progress now uses real elapsed
   time (`(now − firstSeenTime) / reactionLimit`, was a constant 1.0), and
   the timer resets when you look away instead of freezing and instant-
   firing on re-look.
5. **New scenarios** — C-05 Wide Flick 180, T-05 Reactive Track,
   R-03 Peeker's Advantage (also added to routine Block 5: Angle Hold ×2
   + Peeker's Advantage ×2), MAP-04 Haven B course (5 bots, positions
   measured from `havenB.blend` via Blender floor scan).
