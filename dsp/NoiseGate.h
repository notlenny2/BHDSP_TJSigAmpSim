#pragma once

#include <juce_dsp/juce_dsp.h>

namespace backhouse
{
class NoiseGate
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setThresholdDb(float thresholdDb);
    void process(juce::AudioBuffer<float>& buffer);

private:
    float sampleRate = 44100.0f;
    float thresholdLinear = 0.001f;
    float env = 0.0f;
    float gainSmoothed = 1.0f;
};
} // namespace backhouse
