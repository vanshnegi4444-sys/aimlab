#include "audio.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#include <vector>
#include <cmath>
#include <atomic>

#pragma comment(lib, "winmm.lib")

namespace {

constexpr int kSampleRate = 44100;
HWAVEOUT g_hWaveOut = nullptr;

// Pre-generated sound buffers
std::vector<short> g_hitBuffer;
std::vector<short> g_headshotBuffer;
std::vector<short> g_stopBuffer;
std::vector<short> g_spikeBeepBuffer;
std::vector<short> g_failBuffer;

// Voice pool for overlapping sounds
constexpr int kPoolSize = 16;
struct Voice {
    WAVEHDR hdr{};
    std::vector<short> buffer;
    bool inUse = false;
};
Voice g_voices[kPoolSize];
int g_nextVoice = 0;

void CALLBACK waveOutProc(HWAVEOUT, UINT uMsg, DWORD_PTR, DWORD_PTR dwParam1, DWORD_PTR) {
    if (uMsg == WOM_DONE) {
        WAVEHDR* pHdr = reinterpret_cast<WAVEHDR*>(dwParam1);
        if (pHdr) {
            pHdr->dwUser = 0; // Mark free
        }
    }
}

std::vector<short> generateTone(float freq1, float freq2, float durationSec, float decayRate) {
    int totalSamples = static_cast<int>(kSampleRate * durationSec);
    std::vector<short> buf(totalSamples);
    constexpr float kTwoPi = 6.283185307f;

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / kSampleRate;
        float env = std::exp(-decayRate * t); // smooth exponential falloff

        // Two frequencies for rich harmonic punch
        float s1 = std::sin(kTwoPi * freq1 * t);
        float s2 = (freq2 > 0.0f) ? 0.6f * std::sin(kTwoPi * freq2 * t) : 0.0f;
        float sample = (s1 + s2) * env;

        // Soft clip
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;

        buf[i] = static_cast<short>(sample * 28000.0f);
    }
    return buf;
}

void playBuffer(const std::vector<short>& src, float volume) {
    if (!g_hWaveOut || src.empty()) return;

    // Find next available voice
    Voice& v = g_voices[g_nextVoice];
    g_nextVoice = (g_nextVoice + 1) % kPoolSize;

    if (v.hdr.dwUser != 0) {
        waveOutUnprepareHeader(g_hWaveOut, &v.hdr, sizeof(WAVEHDR));
    }

    v.buffer.resize(src.size());
    float vol = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
    for (size_t i = 0; i < src.size(); ++i) {
        v.buffer[i] = static_cast<short>(src[i] * vol);
    }

    memset(&v.hdr, 0, sizeof(WAVEHDR));
    v.hdr.lpData = reinterpret_cast<LPSTR>(v.buffer.data());
    v.hdr.dwBufferLength = static_cast<DWORD>(v.buffer.size() * sizeof(short));
    v.hdr.dwUser = 1; // In use

    if (waveOutPrepareHeader(g_hWaveOut, &v.hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
        waveOutWrite(g_hWaveOut, &v.hdr, sizeof(WAVEHDR));
    }
}

} // namespace

namespace audio {

bool init() {
    WAVEFORMATEX wfx{};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = kSampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    MMRESULT res = waveOutOpen(&g_hWaveOut, WAVE_MAPPER, &wfx,
                               reinterpret_cast<DWORD_PTR>(waveOutProc), 0, CALLBACK_FUNCTION);
    if (res != MMSYSERR_NOERROR) {
        g_hWaveOut = nullptr;
        return false;
    }

    // 1. Hit sound: 1200Hz punchy transient with 500Hz body (25ms)
    g_hitBuffer = generateTone(1200.0f, 500.0f, 0.035f, 120.0f);

    // 2. Headshot sound: 2200Hz + 3300Hz metallic crisp chime (45ms)
    g_headshotBuffer = generateTone(2200.0f, 3300.0f, 0.060f, 75.0f);

    // 3. Counter-strafe stop cue: pleasant subtle 800Hz blip (15ms)
    g_stopBuffer = generateTone(800.0f, 0.0f, 0.020f, 150.0f);

    // 4. Spike countdown beep: crisp 1050Hz + 2100Hz electronic beep (40ms)
    g_spikeBeepBuffer = generateTone(1050.0f, 2100.0f, 0.040f, 95.0f);

    // 5. Fail / Spike detonation cue: deep 180Hz + 135Hz resonant thud/buzzer (260ms)
    g_failBuffer = generateTone(180.0f, 135.0f, 0.260f, 12.0f);

    return true;
}

void shutdown() {
    if (g_hWaveOut) {
        waveOutReset(g_hWaveOut);
        for (Voice& v : g_voices) {
            if (v.hdr.dwUser != 0) {
                waveOutUnprepareHeader(g_hWaveOut, &v.hdr, sizeof(WAVEHDR));
            }
        }
        waveOutClose(g_hWaveOut);
        g_hWaveOut = nullptr;
    }
}

void playHit(float volume) {
    playBuffer(g_hitBuffer, volume);
}

void playHeadshot(float volume) {
    playBuffer(g_headshotBuffer, volume);
}

void playStopCue() {
    playBuffer(g_stopBuffer, 0.4f);
}

void playSpikeBeep(float volume) {
    playBuffer(g_spikeBeepBuffer, volume);
}

void playFail(float volume) {
    playBuffer(g_failBuffer, volume);
}

} // namespace audio

