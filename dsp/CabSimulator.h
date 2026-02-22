#pragma once

#include <juce_dsp/juce_dsp.h>

namespace backhouse
{
class CabSimulator
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setEnabled(bool enabled);
    void setAmpType(int ampIndex);
    void setTone(float tone);
    bool loadUserIR(const juce::File& file);
    void clearUserIR();
    bool hasUserIR() const;
    juce::String getUserIRName() const;
    void process(juce::AudioBuffer<float>& buffer);

private:
    void updateFilters();

    juce::dsp::IIR::Filter<float> hp[2];
    juce::dsp::IIR::Filter<float> lp[2];
    juce::dsp::IIR::Filter<float> contour[2];
    juce::dsp::Convolution convolution;

    float sampleRate = 44100.0f;
    float tone = 0.5f;
    int ampType = 0;
    bool enabled = true;
    bool userIRLoaded = false;
    juce::String userIRName;
};
} // namespace backhouse
