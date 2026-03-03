#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

namespace backhouse
{
class ReverbIRProcessor
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setEnabled(bool isEnabled);
    void setMix(float newMix);
    bool loadIR(const juce::File& file);
    void clearIR();
    bool hasIR() const;
    juce::String getIRName() const;
    void process(juce::AudioBuffer<float>& buffer);

private:
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> wetBuffer;
    float mix = 0.12f;
    float dryMix = 0.88f;
    bool enabled = false;
    bool irLoaded = false;
    juce::String irName;
};
} // namespace backhouse
