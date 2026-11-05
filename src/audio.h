#pragma once

// Low-latency procedural audio engine using Windows Multimedia (waveOut).
// Zero external files, zero dependencies beyond winmm.lib, minimal latency (<5ms).

namespace audio {

bool init();
void shutdown();

// Plays a clean hit-click sound
void playHit(float volume = 1.0f);

// Plays a high-pitched metallic dink for headshots / critical hits
void playHeadshot(float volume = 1.0f);

// Plays a subtle stop-indicator sound for counter-strafing
void playStopCue();

} // namespace audio

