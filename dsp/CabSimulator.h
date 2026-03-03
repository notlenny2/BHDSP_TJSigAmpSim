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
    void setSpeakerModel(int modelIndex);
    void setMicModel(int modelIndex);
    void setMicAOffAxis(bool offAxis);
    void setSpeakerModelB(int modelIndex);
    void setMicModelB(int modelIndex);
    void setMicBOffAxis(bool offAxis);
    void setMicBlend(float blendAmount);
    void setAmp4TightMode(bool tightEnabled);
    void setTone(float tone);
    bool loadUserIR(const juce::File& file);
    void clearUserIR();
    bool hasUserIR() const;
    juce::String getUserIRName() const;
    void process(juce::AudioBuffer<float>& buffer);

private:
    void updateFilters();

    juce::dsp::IIR::Filter<float> hpA[2];
    juce::dsp::IIR::Filter<float> lpA[2];
    juce::dsp::IIR::Filter<float> contourLowA[2];
    juce::dsp::IIR::Filter<float> contourHighA[2];
    juce::dsp::IIR::Filter<float> offAxisDipA[2];

    juce::dsp::IIR::Filter<float> hpB[2];
    juce::dsp::IIR::Filter<float> lpB[2];
    juce::dsp::IIR::Filter<float> contourLowB[2];
    juce::dsp::IIR::Filter<float> contourHighB[2];
    juce::dsp::IIR::Filter<float> offAxisDipB[2];
    juce::dsp::Convolution convolution;
    juce::AudioBuffer<float> cabPathA;
    juce::AudioBuffer<float> cabPathB;

    float sampleRate = 44100.0f;
    float tone = 0.5f;
    int ampType = 0;
    int speakerModel = 0;
    int micModel = 0;
    bool micAOffAxis = true;
    int speakerModelB = 0;
    int micModelB = 0;
    bool micBOffAxis = true;
    float micBlend = 0.0f;
    bool amp4TightMode = true;
    bool enabled = true;
    bool userIRLoaded = false;
    juce::String userIRName;
};
} // namespace backhouse
