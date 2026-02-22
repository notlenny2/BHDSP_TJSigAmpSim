#include "AmpEngine.h"

#include <cmath>

namespace
{
struct AmpModelParams
{
    float drive1 = 2.0f;
    float drive2 = 2.8f;
    float asymmetry = 0.0f;
    float sagAmount = 0.0f;
    float sagReleaseMs = 70.0f;
    float cleanBlend = 0.0f;
    float preHpHz = 80.0f;
    float preShapeFreqHz = 900.0f;
    float preShapeGainDb = 0.0f;
    float preShapeQ = 0.9f;
    float fizzCutHz = 7500.0f;
    float depthBaseDb = 0.0f;
};

AmpModelParams getAmpModelParams(int ampType)
{
    switch (ampType)
    {
        case 0: // Amp 1: clean compressed sparkle
            return AmpModelParams {
                .drive1 = 1.35f,
                .drive2 = 1.65f,
                .asymmetry = 0.05f,
                .sagAmount = 0.04f,
                .sagReleaseMs = 40.0f,
                .cleanBlend = 0.42f,
                .preHpHz = 95.0f,
                .preShapeFreqHz = 2900.0f,
                .preShapeGainDb = 1.8f,
                .preShapeQ = 0.7f,
                .fizzCutHz = 9800.0f,
                .depthBaseDb = 0.0f,
            };

        case 1: // Amp 2: dirty bluesy growl
            return AmpModelParams {
                .drive1 = 2.05f,
                .drive2 = 2.65f,
                .asymmetry = 0.23f,
                .sagAmount = 0.09f,
                .sagReleaseMs = 95.0f,
                .cleanBlend = 0.14f,
                .preHpHz = 78.0f,
                .preShapeFreqHz = 780.0f,
                .preShapeGainDb = 2.2f,
                .preShapeQ = 0.85f,
                .fizzCutHz = 6900.0f,
                .depthBaseDb = 0.0f,
            };

        case 2: // Amp 3: classic thrash tight + depth
            return AmpModelParams {
                .drive1 = 2.65f,
                .drive2 = 3.55f,
                .asymmetry = 0.13f,
                .sagAmount = 0.07f,
                .sagReleaseMs = 60.0f,
                .cleanBlend = 0.06f,
                .preHpHz = 120.0f,
                .preShapeFreqHz = 1450.0f,
                .preShapeGainDb = 1.0f,
                .preShapeQ = 1.1f,
                .fizzCutHz = 6200.0f,
                .depthBaseDb = 2.0f,
            };

        case 3: // Amp 4: modern scooped articulate metalcore
            return AmpModelParams {
                .drive1 = 3.05f,
                .drive2 = 4.20f,
                .asymmetry = 0.17f,
                .sagAmount = 0.11f,
                .sagReleaseMs = 72.0f,
                .cleanBlend = 0.02f,
                .preHpHz = 135.0f,
                .preShapeFreqHz = 520.0f,
                .preShapeGainDb = -1.4f,
                .preShapeQ = 0.9f,
                .fizzCutHz = 5600.0f,
                .depthBaseDb = 3.0f,
            };

        default:
            return {};
    }
}

inline float asymSoftClip(float x, float asymmetry)
{
    const float posDrive = 1.0f + 0.5f * asymmetry;
    const float negDrive = 1.0f - 0.35f * asymmetry;

    if (x >= 0.0f)
        return std::tanh(x * posDrive);

    return std::tanh(x * negDrive) * (1.0f - 0.08f * asymmetry);
}
} // namespace

namespace backhouse
{
void AmpEngine::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = static_cast<float>(spec.sampleRate);

    for (int channel = 0; channel < 2; ++channel)
    {
        preHighPass[channel].prepare(spec);
        preShape[channel].prepare(spec);

        lowShelf[channel].prepare(spec);
        midPeak[channel].prepare(spec);
        highShelf[channel].prepare(spec);
        presenceShelf[channel].prepare(spec);
        subShelf[channel].prepare(spec);

        depthResonance[channel].prepare(spec);
        fizzCut[channel].prepare(spec);

        preHighPass[channel].reset();
        preShape[channel].reset();

        lowShelf[channel].reset();
        midPeak[channel].reset();
        highShelf[channel].reset();
        presenceShelf[channel].reset();
        subShelf[channel].reset();

        depthResonance[channel].reset();
        fizzCut[channel].reset();

        sagEnvelope[static_cast<size_t>(channel)] = 0.0f;
    }

    updateToneFilters();
}

void AmpEngine::setAmpType(int ampIndex)
{
    const int clamped = juce::jlimit(0, 3, ampIndex);
    if (clamped != ampType)
    {
        ampType = clamped;
        updateToneFilters();
    }
}

void AmpEngine::setInputGainDb(float db)
{
    inputGainLinear = juce::Decibels::decibelsToGain(db);
}

void AmpEngine::setInputBoost(bool isEnabled)
{
    inputBoost = isEnabled;
}

void AmpEngine::setSubDb(float db)
{
    subDb = db;
    updateToneFilters();
}

void AmpEngine::setToneDb(float lDb, float mDb, float hDb, float pDb)
{
    lowDb = lDb;
    midDb = mDb;
    highDb = hDb;
    presenceDb = pDb;
    updateToneFilters();
}

void AmpEngine::process(juce::AudioBuffer<float>& buffer)
{
    const auto model = getAmpModelParams(ampType);

    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        preHighPass[ch].process(context);
        preShape[ch].process(context);
    }

    const float boostGain = (inputBoost && (ampType == 1 || ampType == 2)) ? juce::Decibels::decibelsToGain(7.0f) : 1.0f;
    const float sagCoeff = std::exp(-1.0f / ((model.sagReleaseMs * 0.001f) * sampleRate));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        float& sag = sagEnvelope[static_cast<size_t>(ch)];

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            float x = dry * inputGainLinear * boostGain;

            const float rectified = std::abs(x);
            sag = std::max(rectified, sag * sagCoeff + rectified * (1.0f - sagCoeff));
            const float sagGain = juce::jlimit(0.62f, 1.0f, 1.0f - model.sagAmount * sag);

            x *= sagGain;
            x = asymSoftClip(x * model.drive1, model.asymmetry);
            x = asymSoftClip(x * model.drive2, model.asymmetry);

            data[i] = x * (1.0f - model.cleanBlend) + dry * model.cleanBlend;
        }
    }

    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);

        if (ampType == 2 || ampType == 3)
            subShelf[ch].process(context);

        lowShelf[ch].process(context);
        midPeak[ch].process(context);
        highShelf[ch].process(context);
        presenceShelf[ch].process(context);

        if (ampType == 2 || ampType == 3)
            depthResonance[ch].process(context);

        fizzCut[ch].process(context);
    }
}

void AmpEngine::updateToneFilters()
{
    const auto model = getAmpModelParams(ampType);

    float lowFreq = 160.0f;
    float midFreq = 850.0f;
    float midQ = 1.2f;
    float highFreq = 3000.0f;
    float presenceFreq = 4500.0f;

    if (ampType == 0)
    {
        lowFreq = 140.0f;
        midFreq = 1100.0f;
        midQ = 0.95f;
        highFreq = 3600.0f;
        presenceFreq = 5800.0f;
    }
    else if (ampType == 1)
    {
        lowFreq = 190.0f;
        midFreq = 760.0f;
        midQ = 1.0f;
        highFreq = 2600.0f;
        presenceFreq = 4100.0f;
    }
    else if (ampType == 2)
    {
        lowFreq = 130.0f;
        midFreq = 1250.0f;
        midQ = 1.45f;
        highFreq = 3200.0f;
        presenceFreq = 5000.0f;
    }
    else if (ampType == 3)
    {
        lowFreq = 115.0f;
        midFreq = 700.0f;
        midQ = 0.95f;
        highFreq = 2900.0f;
        presenceFreq = 4700.0f;
    }

    const float depthGain = juce::Decibels::decibelsToGain(model.depthBaseDb + subDb);

    for (int channel = 0; channel < 2; ++channel)
    {
        preHighPass[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, model.preHpHz, 0.707f);
        preShape[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                              model.preShapeFreqHz,
                                                                                              model.preShapeQ,
                                                                                              juce::Decibels::decibelsToGain(model.preShapeGainDb));

        subShelf[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate,
                                                                                             92.0f,
                                                                                             0.707f,
                                                                                             juce::Decibels::decibelsToGain(subDb));
        lowShelf[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate,
                                                                                             lowFreq,
                                                                                             0.707f,
                                                                                             juce::Decibels::decibelsToGain(lowDb));
        midPeak[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                              midFreq,
                                                                                              midQ,
                                                                                              juce::Decibels::decibelsToGain(midDb));
        highShelf[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate,
                                                                                               highFreq,
                                                                                               0.707f,
                                                                                               juce::Decibels::decibelsToGain(highDb));
        presenceShelf[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate,
                                                                                                   presenceFreq,
                                                                                                   0.707f,
                                                                                                   juce::Decibels::decibelsToGain(presenceDb));

        depthResonance[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate,
                                                                                                     ampType == 2 ? 115.0f : 95.0f,
                                                                                                     0.65f,
                                                                                                     depthGain);
        fizzCut[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate,
                                                                                           model.fizzCutHz,
                                                                                           0.707f);
    }
}
} // namespace backhouse
