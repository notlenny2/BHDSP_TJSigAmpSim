#include "ReverbIRProcessor.h"

#include <cmath>

namespace backhouse
{
void ReverbIRProcessor::prepare(const juce::dsp::ProcessSpec& spec)
{
    convolution.prepare(spec);
    wetBuffer.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    wetBuffer.clear();
}

void ReverbIRProcessor::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
}

void ReverbIRProcessor::setMix(float newMix)
{
    constexpr float uiMaxMix = 1.0f;
    const float clamped = juce::jlimit(0.0f, uiMaxMix, newMix);
    const float t = uiMaxMix > 0.0f ? clamped / uiMaxMix : 0.0f;

    // Nonlinear crossfade: higher settings push wet harder and duck direct more aggressively.
    mix = std::pow(t, 0.72f);
    dryMix = juce::jlimit(0.08f, 1.0f, 1.0f - 0.92f * std::pow(t, 0.92f));
}

bool ReverbIRProcessor::loadIR(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    convolution.loadImpulseResponse(file,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::yes,
                                    65536,
                                    juce::dsp::Convolution::Normalise::yes);

    irLoaded = true;
    irName = file.getFileName();
    return true;
}

void ReverbIRProcessor::clearIR()
{
    irLoaded = false;
    irName = {};
}

bool ReverbIRProcessor::hasIR() const
{
    return irLoaded;
}

juce::String ReverbIRProcessor::getIRName() const
{
    return irName;
}

void ReverbIRProcessor::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled || !irLoaded || mix <= 0.0001f)
        return;

    if (wetBuffer.getNumChannels() < buffer.getNumChannels() || wetBuffer.getNumSamples() < buffer.getNumSamples())
        wetBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples(), false, false, true);

    wetBuffer.makeCopyOf(buffer, true);
    juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
    convolution.process(wetContext);

    float dryGain = juce::jlimit(0.0f, 1.0f, dryMix);
    float wetGain = juce::jlimit(0.0f, 1.9f, mix * (1.05f + 0.85f * mix));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* dry = buffer.getWritePointer(ch);
        const auto* wet = wetBuffer.getReadPointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            dry[i] = (dryGain * dry[i]) + (wetGain * wet[i]);
    }
}
} // namespace backhouse
