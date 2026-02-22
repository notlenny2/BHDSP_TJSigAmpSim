#include "NoiseGate.h"
#include <cmath>

namespace backhouse
{
void NoiseGate::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    env = 0.0f;
    gainSmoothed = 1.0f;
}

void NoiseGate::setThresholdDb(float thresholdDb)
{
    thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
}

void NoiseGate::process(juce::AudioBuffer<float>& buffer)
{
    const float attackCoeff = std::exp(-1.0f / (0.0015f * sampleRate));
    const float releaseCoeff = std::exp(-1.0f / (0.050f * sampleRate));
    const float gateAttack = std::exp(-1.0f / (0.002f * sampleRate));
    const float gateRelease = std::exp(-1.0f / (0.015f * sampleRate));

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float xL = left[i];
        const float xR = right != nullptr ? right[i] : xL;
        const float absSample = std::max(std::abs(xL), std::abs(xR));

        const float envCoeff = absSample > env ? attackCoeff : releaseCoeff;
        env = envCoeff * env + (1.0f - envCoeff) * absSample;

        const float targetGain = env >= thresholdLinear ? 1.0f : 0.0f;
        const float coeff = targetGain > gainSmoothed ? gateAttack : gateRelease;
        gainSmoothed = coeff * gainSmoothed + (1.0f - coeff) * targetGain;

        left[i] *= gainSmoothed;
        if (right != nullptr)
            right[i] *= gainSmoothed;
    }
}
} // namespace backhouse
