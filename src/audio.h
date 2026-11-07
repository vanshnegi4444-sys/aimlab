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

// Plays a crisp countdown beep for the final 10s of a post-plant spike timer
void playSpikeBeep(float volume = 1.0f);

// Plays a round-lost / spike-detonation audio cue on course failure
void playFail(float volume = 1.0f);

} // namespace audio

