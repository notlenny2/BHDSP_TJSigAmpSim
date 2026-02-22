#include "CabSimulator.h"

namespace backhouse
{
void CabSimulator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    for (auto channel = 0; channel < 2; ++channel)
    {
        hp[channel].prepare(spec);
        lp[channel].prepare(spec);
        contour[channel].prepare(spec);
        hp[channel].reset();
        lp[channel].reset();
        contour[channel].reset();
    }
    convolution.prepare(spec);

    updateFilters();
}

void CabSimulator::setEnabled(bool isEnabled)
{
    enabled = isEnabled;
}

void CabSimulator::setAmpType(int ampIndex)
{
    ampType = juce::jlimit(0, 3, ampIndex);
    updateFilters();
}

void CabSimulator::setTone(float newTone)
{
    tone = juce::jlimit(0.0f, 1.0f, newTone);
    updateFilters();
}

bool CabSimulator::loadUserIR(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    convolution.loadImpulseResponse(file,
                                    juce::dsp::Convolution::Stereo::yes,
                                    juce::dsp::Convolution::Trim::yes,
                                    8192,
                                    juce::dsp::Convolution::Normalise::yes);

    userIRLoaded = true;
    userIRName = file.getFileName();
    return true;
}

void CabSimulator::clearUserIR()
{
    userIRLoaded = false;
    userIRName = {};
}

bool CabSimulator::hasUserIR() const
{
    return userIRLoaded;
}

juce::String CabSimulator::getUserIRName() const
{
    return userIRName;
}

void CabSimulator::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled)
        return;

    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        hp[ch].process(context);
        lp[ch].process(context);
        contour[ch].process(context);
    }

    if (userIRLoaded)
    {
        juce::dsp::ProcessContextReplacing<float> context(block);
        convolution.process(context);
    }
}

void CabSimulator::updateFilters()
{
    float baseHp = 70.0f;
    float baseLpMin = 2200.0f;
    float baseLpMax = 8200.0f;
    float contourFreq = 1900.0f;
    float contourQ = 0.8f;
    float contourDb = -1.0f;

    switch (ampType)
    {
        case 0: // Clean sparkle
            baseHp = 80.0f;
            baseLpMin = 3000.0f;
            baseLpMax = 10000.0f;
            contourFreq = 3200.0f;
            contourQ = 0.7f;
            contourDb = 1.0f;
            break;
        case 1: // Dirty blues
            baseHp = 85.0f;
            baseLpMin = 2600.0f;
            baseLpMax = 7600.0f;
            contourFreq = 900.0f;
            contourQ = 1.0f;
            contourDb = 1.5f;
            break;
        case 2: // Thrash
            baseHp = 95.0f;
            baseLpMin = 2400.0f;
            baseLpMax = 6800.0f;
            contourFreq = 4200.0f;
            contourQ = 1.1f;
            contourDb = -1.5f;
            break;
        case 3: // Modern metal
            baseHp = 100.0f;
            baseLpMin = 2200.0f;
            baseLpMax = 6200.0f;
            contourFreq = 500.0f;
            contourQ = 0.9f;
            contourDb = 1.2f;
            break;
        default:
            break;
    }

    const float hpFreq = baseHp;
    const float lpFreq = juce::jmap(tone, baseLpMin, baseLpMax);
    const float toneOffsetDb = juce::jmap(tone, -1.5f, 1.5f);
    const float contourGain = juce::Decibels::decibelsToGain(contourDb + toneOffsetDb);

    for (auto channel = 0; channel < 2; ++channel)
    {
        hp[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq, 0.707f);
        lp[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpFreq, 0.707f);
        contour[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, contourFreq, contourQ, contourGain);
    }
}
} // namespace backhouse
