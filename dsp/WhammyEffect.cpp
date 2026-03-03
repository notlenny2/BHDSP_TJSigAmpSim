#include "WhammyEffect.h"

#include <cmath>

namespace backhouse {

void WhammyEffect::prepare(double sr, int /*maxBlockSize*/)
{
    sampleRate = sr;
    // One-pole coefficient for ~5ms RC smoothing: alpha = exp(-1 / (tau * sr))
    smoothRC = std::exp(-1.0f / static_cast<float>(0.005 * sr));
    reset();
}

void WhammyEffect::reset()
{
    circBuf.fill(0.0f);
    wPos = 0;
    gPhase[0] = static_cast<float>(kGrainSamples) * 0.5f;
    gPhase[1] = 0.0f;
    smoothedRatio = 1.0f;
    smoothedMix   = 0.0f;
}

float WhammyEffect::processSample(float input, float targetRatio)
{
    // Smooth the pitch ratio to avoid zipper noise
    smoothedRatio = smoothRC * smoothedRatio + (1.0f - smoothRC) * targetRatio;

    // Write input into circular buffer
    circBuf[static_cast<size_t>(wPos)] = input;

    float out = 0.0f;
    const float delta = smoothedRatio - 1.0f; // how fast the read pointer drifts

    for (int g = 0; g < 2; ++g)
    {
        // Hann window: 0→1→0 over kGrainSamples
        const float phase = gPhase[g];
        const float win = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * phase
                                                   / static_cast<float>(kGrainSamples));

        // Read position (fractional, behind the write position)
        const float readFrac = static_cast<float>(wPos) - phase;
        const int   r0 = static_cast<int>(std::floor(readFrac)) & kBufMask;
        const int   r1 = (r0 + 1) & kBufMask;
        const float frac = readFrac - std::floor(readFrac);
        const float grain = circBuf[static_cast<size_t>(r0)]
                          + frac * (circBuf[static_cast<size_t>(r1)] - circBuf[static_cast<size_t>(r0)]);

        out += win * grain;

        // Advance grain phase; reset when it crosses kGrainSamples so grains loop seamlessly
        gPhase[g] += 1.0f + delta; // reads advance at (1 + delta) per sample
        while (gPhase[g] >= static_cast<float>(kGrainSamples))
            gPhase[g] -= static_cast<float>(kGrainSamples);
        while (gPhase[g] < 0.0f)
            gPhase[g] += static_cast<float>(kGrainSamples);
    }

    wPos = (wPos + 1) & kBufMask;
    return out;
}

void WhammyEffect::processBlock(juce::AudioBuffer<float>& buffer,
                                float pitchRatio,
                                float mix)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Early-exit: if both smoothed mix and target are near zero, skip processing
    if (mix < 0.0001f && smoothedMix < 0.0001f)
        return;

    // Process mono (channel 0) then copy to other channels
    auto* ch0 = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i)
    {
        const float wet = processSample(ch0[i], pitchRatio);
        smoothedMix = smoothRC * smoothedMix + (1.0f - smoothRC) * mix;
        ch0[i] = (1.0f - smoothedMix) * ch0[i] + smoothedMix * wet;
    }

    // Copy processed channel 0 to remaining channels
    for (int ch = 1; ch < numChannels; ++ch)
        buffer.copyFrom(ch, 0, buffer.getReadPointer(0), numSamples);
}

} // namespace backhouse
