#pragma once

#include <array>

#include <juce_dsp/juce_dsp.h>

namespace backhouse
{
class AmpEngine
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setAmpType(int ampIndex);
    void setInputGainDb(float db);
    void setInputBoost(bool enabled);
    void setSubDb(float db);
    void setToneDb(float lowDb, float midDb, float highDb, float presenceDb);
    void process(juce::AudioBuffer<float>& buffer);

private:
    void updateToneFilters();

    int ampType = 0;
    bool inputBoost = false;

    float sampleRate = 44100.0f;
    float inputGainLinear = 1.0f;
    float subDb = 0.0f;
    float lowDb = 0.0f;
    float midDb = 0.0f;
    float highDb = 0.0f;
    float presenceDb = 0.0f;

    std::array<float, 2> sagEnvelope { 0.0f, 0.0f };

    juce::dsp::IIR::Filter<float> preHighPass[2];
    juce::dsp::IIR::Filter<float> preShape[2];

    juce::dsp::IIR::Filter<float> lowShelf[2];
    juce::dsp::IIR::Filter<float> midPeak[2];
    juce::dsp::IIR::Filter<float> highShelf[2];
    juce::dsp::IIR::Filter<float> presenceShelf[2];
    juce::dsp::IIR::Filter<float> subShelf[2];

    juce::dsp::IIR::Filter<float> depthResonance[2];
    juce::dsp::IIR::Filter<float> fizzCut[2];
};
} // namespace backhouse
