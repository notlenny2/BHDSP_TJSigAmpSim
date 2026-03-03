#include "CabSimulator.h"

namespace
{
struct Voicing
{
    float hp = 80.0f;
    float lpMin = 4300.0f;
    float lpMax = 7600.0f;
    float lowFreq = 150.0f;
    float lowQ = 0.9f;
    float lowDb = 1.0f;
    float highFreq = 3200.0f;
    float highQ = 1.1f;
    float highDb = 1.0f;
    float dipFreq = 5300.0f;
    float dipQ = 1.4f;
    float dipDb = -2.5f;
};

Voicing getSpeakerVoicing(int speakerModel)
{
    switch (speakerModel)
    {
        case 0: return { 82.0f, 4100.0f, 7000.0f, 170.0f, 0.92f, 1.8f, 2850.0f, 1.0f, 1.1f, 5200.0f, 1.35f, -2.8f };
        case 1: return { 85.0f, 4500.0f, 8000.0f, 145.0f, 0.95f, 1.1f, 3350.0f, 1.2f, 2.2f, 5600.0f, 1.6f, -3.4f };
        case 2: return { 80.0f, 4300.0f, 7600.0f, 160.0f, 0.9f, 1.5f, 3000.0f, 1.05f, 1.4f, 5350.0f, 1.45f, -2.9f };
        case 3: return { 88.0f, 4700.0f, 8200.0f, 135.0f, 0.95f, 0.9f, 3550.0f, 1.2f, 2.0f, 5900.0f, 1.65f, -3.6f };
        case 4: return { 75.0f, 5200.0f, 9600.0f, 150.0f, 0.85f, 0.6f, 2650.0f, 0.9f, 1.0f, 5000.0f, 1.2f, -1.8f };
        default: break;
    }
    return {};
}

Voicing getMicVoicing(int micModel)
{
    switch (micModel)
    {
        case 0: return { 100.0f, 4300.0f, 7600.0f, 180.0f, 0.95f, -0.2f, 3400.0f, 1.25f, 2.5f, 5600.0f, 1.7f, -2.4f };
        case 1: return { 78.0f, 5200.0f, 9000.0f, 160.0f, 0.85f, 0.7f, 2800.0f, 0.95f, 1.0f, 6300.0f, 1.2f, -1.2f };
        case 2: return { 72.0f, 5000.0f, 8600.0f, 145.0f, 0.9f, 1.2f, 2550.0f, 0.95f, 0.8f, 6000.0f, 1.3f, -1.4f };
        case 3: return { 70.0f, 3600.0f, 6200.0f, 165.0f, 0.85f, 1.1f, 2200.0f, 0.85f, 0.4f, 5000.0f, 1.25f, -1.6f };
        case 4: return { 86.0f, 4600.0f, 7600.0f, 170.0f, 0.95f, 0.9f, 3000.0f, 1.1f, 1.6f, 5700.0f, 1.55f, -2.1f };
        default: break;
    }
    return {};
}

float blend(float a, float b, float wa, float wb)
{
    return (wa * a) + (wb * b);
}
} // namespace

namespace backhouse
{
void CabSimulator::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);
    for (int channel = 0; channel < 2; ++channel)
    {
        hpA[channel].prepare(spec);
        lpA[channel].prepare(spec);
        contourLowA[channel].prepare(spec);
        contourHighA[channel].prepare(spec);
        offAxisDipA[channel].prepare(spec);
        hpA[channel].reset();
        lpA[channel].reset();
        contourLowA[channel].reset();
        contourHighA[channel].reset();
        offAxisDipA[channel].reset();

        hpB[channel].prepare(spec);
        lpB[channel].prepare(spec);
        contourLowB[channel].prepare(spec);
        contourHighB[channel].prepare(spec);
        offAxisDipB[channel].prepare(spec);
        hpB[channel].reset();
        lpB[channel].reset();
        contourLowB[channel].reset();
        contourHighB[channel].reset();
        offAxisDipB[channel].reset();
    }

    convolution.prepare(spec);
    cabPathA.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    cabPathB.setSize(static_cast<int>(spec.numChannels), static_cast<int>(spec.maximumBlockSize));
    cabPathA.clear();
    cabPathB.clear();
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

void CabSimulator::setSpeakerModel(int modelIndex)
{
    speakerModel = juce::jlimit(0, 4, modelIndex);
    updateFilters();
}

void CabSimulator::setMicModel(int modelIndex)
{
    micModel = juce::jlimit(0, 4, modelIndex);
    updateFilters();
}

void CabSimulator::setMicAOffAxis(bool offAxis)
{
    micAOffAxis = offAxis;
    updateFilters();
}

void CabSimulator::setSpeakerModelB(int modelIndex)
{
    speakerModelB = juce::jlimit(0, 4, modelIndex);
    updateFilters();
}

void CabSimulator::setMicModelB(int modelIndex)
{
    micModelB = juce::jlimit(0, 4, modelIndex);
    updateFilters();
}

void CabSimulator::setMicBOffAxis(bool offAxis)
{
    micBOffAxis = offAxis;
    updateFilters();
}

void CabSimulator::setMicBlend(float blendAmount)
{
    micBlend = juce::jlimit(0.0f, 1.0f, blendAmount);
}

void CabSimulator::setAmp4TightMode(bool tightEnabled)
{
    if (amp4TightMode == tightEnabled)
        return;

    amp4TightMode = tightEnabled;
    if (ampType == 3)
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

    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    if (cabPathA.getNumChannels() < channels || cabPathA.getNumSamples() < samples)
        cabPathA.setSize(channels, samples, false, false, true);
    if (cabPathB.getNumChannels() < channels || cabPathB.getNumSamples() < samples)
        cabPathB.setSize(channels, samples, false, false, true);

    cabPathA.makeCopyOf(buffer, true);
    juce::dsp::AudioBlock<float> blockA(cabPathA);
    for (size_t ch = 0; ch < blockA.getNumChannels(); ++ch)
    {
        auto channelBlock = blockA.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        hpA[ch].process(context);
        lpA[ch].process(context);
        contourLowA[ch].process(context);
        contourHighA[ch].process(context);
        offAxisDipA[ch].process(context);
    }

    const float bMix = juce::jlimit(0.0f, 1.0f, micBlend);
    if (bMix > 0.0001f)
    {
        cabPathB.makeCopyOf(buffer, true);
        juce::dsp::AudioBlock<float> blockB(cabPathB);
        for (size_t ch = 0; ch < blockB.getNumChannels(); ++ch)
        {
            auto channelBlock = blockB.getSingleChannelBlock(ch);
            juce::dsp::ProcessContextReplacing<float> context(channelBlock);
            hpB[ch].process(context);
            lpB[ch].process(context);
            contourLowB[ch].process(context);
            contourHighB[ch].process(context);
            offAxisDipB[ch].process(context);
        }
    }

    const float aMix = 1.0f - bMix;
    for (int ch = 0; ch < channels; ++ch)
    {
        const auto* a = cabPathA.getReadPointer(ch);
        const auto* b = cabPathB.getReadPointer(ch);
        auto* out = buffer.getWritePointer(ch);
        for (int i = 0; i < samples; ++i)
            out[i] = aMix * a[i] + bMix * b[i];
    }

    if (userIRLoaded)
    {
        juce::dsp::AudioBlock<float> outBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> context(outBlock);
        convolution.process(context);
    }
}

void CabSimulator::updateFilters()
{
    auto applyPathVoicing = [this](int spkModel, int micModelIn, bool offAxis,
                                   juce::dsp::IIR::Filter<float>* hp,
                                   juce::dsp::IIR::Filter<float>* lp,
                                   juce::dsp::IIR::Filter<float>* contourLow,
                                   juce::dsp::IIR::Filter<float>* contourHigh,
                                   juce::dsp::IIR::Filter<float>* offAxisDip)
    {
        auto speaker = getSpeakerVoicing(spkModel);
        auto mic = getMicVoicing(micModelIn);

        float baseHp = blend(speaker.hp, mic.hp, 0.62f, 0.38f);
        float baseLpMin = blend(speaker.lpMin, mic.lpMin, 0.68f, 0.32f);
        float baseLpMax = blend(speaker.lpMax, mic.lpMax, 0.68f, 0.32f);
        float lowContourFreq = blend(speaker.lowFreq, mic.lowFreq, 0.64f, 0.36f);
        float lowContourQ = blend(speaker.lowQ, mic.lowQ, 0.64f, 0.36f);
        float lowContourDb = blend(speaker.lowDb, mic.lowDb, 0.58f, 0.42f);
        float highContourFreq = blend(speaker.highFreq, mic.highFreq, 0.56f, 0.44f);
        float highContourQ = blend(speaker.highQ, mic.highQ, 0.56f, 0.44f);
        float highContourDb = blend(speaker.highDb, mic.highDb, 0.56f, 0.44f);
        float offAxisDipFreq = blend(speaker.dipFreq, mic.dipFreq, 0.56f, 0.44f);
        float offAxisDipQ = blend(speaker.dipQ, mic.dipQ, 0.56f, 0.44f);
        float offAxisDipDb = blend(speaker.dipDb, mic.dipDb, 0.56f, 0.44f);

        if (offAxis)
        {
            highContourDb -= 0.55f;
            highContourFreq -= 220.0f;
            offAxisDipDb -= 1.0f;
            offAxisDipFreq -= 280.0f;
            offAxisDipQ += 0.18f;
        }
        else
        {
            highContourDb += 0.55f;
            highContourFreq += 180.0f;
            offAxisDipDb += 0.55f;
            offAxisDipFreq += 160.0f;
        }

        switch (ampType)
        {
            case 0:
                baseLpMin += 400.0f;
                baseLpMax += 700.0f;
                highContourDb += 0.6f;
                break;
            case 1:
                lowContourDb += 0.7f;
                highContourFreq -= 250.0f;
                highContourDb -= 0.5f;
                break;
            case 2:
                baseHp += 8.0f;
                lowContourDb += 0.5f;
                highContourDb += 0.4f;
                highContourFreq -= 220.0f;
                offAxisDipFreq = juce::jmax(4200.0f, offAxisDipFreq - 260.0f);
                offAxisDipDb -= 0.4f;
                break;
            case 3:
                if (amp4TightMode)
                {
                    baseHp += 14.0f;
                    lowContourDb += 0.9f;
                    highContourDb += 0.55f;
                    highContourFreq -= 260.0f;
                    offAxisDipFreq = juce::jmax(4200.0f, offAxisDipFreq - 300.0f);
                    offAxisDipDb -= 1.15f;
                }
                else
                {
                    baseHp += 6.0f;
                    lowContourDb += 1.2f;
                    highContourDb += 0.2f;
                    highContourFreq -= 220.0f;
                    offAxisDipFreq = juce::jmax(4200.0f, offAxisDipFreq - 260.0f);
                    offAxisDipDb -= 0.95f;
                }
                break;
            default:
                break;
        }

        const float hpFreq = baseHp;
        const float lpFreq = juce::jmap(tone, baseLpMin, baseLpMax);
        const float toneOffsetDb = juce::jmap(tone, -1.1f, 1.25f);
        const float lowContourGain = juce::Decibels::decibelsToGain(lowContourDb - 0.45f * toneOffsetDb);
        const float highContourGain = juce::Decibels::decibelsToGain(highContourDb + toneOffsetDb);
        const float dipGain = juce::Decibels::decibelsToGain(offAxisDipDb - 0.35f * juce::jmax(0.0f, toneOffsetDb));

        for (int channel = 0; channel < 2; ++channel)
        {
            hp[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, hpFreq, 0.707f);
            lp[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lpFreq, 0.707f);
            contourLow[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, lowContourFreq, lowContourQ, lowContourGain);
            contourHigh[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, highContourFreq, highContourQ, highContourGain);
            offAxisDip[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, offAxisDipFreq, offAxisDipQ, dipGain);
        }
    };

    applyPathVoicing(speakerModel, micModel, micAOffAxis, hpA, lpA, contourLowA, contourHighA, offAxisDipA);
    applyPathVoicing(speakerModelB, micModelB, micBOffAxis, hpB, lpB, contourLowB, contourHighB, offAxisDipB);
}
} // namespace backhouse
