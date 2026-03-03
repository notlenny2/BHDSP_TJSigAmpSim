#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cmath>

namespace backhouse {

/**
 * Two-grain overlapping pitch shifter — DigiTech Whammy-style.
 *
 * Two Hann-windowed grains with 180° phase offset crossfade continuously,
 * hiding the periodic jump that a single-grain algorithm would produce.
 *
 * pitchRatio: 1.0 = dry  |  2.0 = +1 oct  |  0.5 = -1 oct
 * mix:        0.0 = dry only  |  1.0 = wet only
 *
 * Inherent latency ≈ kGrainSamples / 2 samples (~23ms at 44.1kHz).
 * Smoothing on ratio and mix (~5ms RC) eliminates zipper noise.
 */
class WhammyEffect {
public:
    static constexpr int kGrainSamples = 2048;
    static constexpr int kBufSamples   = 1 << 15; // 32768 >= 2 * kGrainSamples
    static constexpr int kBufMask      = kBufSamples - 1;

    void prepare(double sampleRate, int maxBlockSize);
    void reset();
    void processBlock(juce::AudioBuffer<float>& buffer, float pitchRatio, float mix);

private:
    float processSample(float input, float pitchRatio);

    std::array<float, kBufSamples> circBuf {};
    int   wPos = 0;
    float gPhase[2] = { static_cast<float>(kGrainSamples) * 0.5f, 0.0f };

    float smoothedRatio = 1.0f;
    float smoothedMix   = 0.0f;
    float smoothRC      = 0.0f; // pre-computed one-pole coefficient (~5ms)
    double sampleRate   = 44100.0;
};

} // namespace backhouse
