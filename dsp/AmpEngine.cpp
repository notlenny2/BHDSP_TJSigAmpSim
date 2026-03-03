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
    float postDriveTrimDb = 0.0f;
    float powerCompAmount = 0.0f;
    float powerCompReleaseMs = 95.0f;
};

AmpModelParams getAmpModelParams(int ampType, bool amp2HiwattMode, bool amp4TightMode)
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
                .postDriveTrimDb = 0.0f,
                .powerCompAmount = 0.05f,
                .powerCompReleaseMs = 55.0f,
            };

        case 1: // Amp 2: dirty bluesy growl
            if (amp2HiwattMode)
            {
                return AmpModelParams {
                    .drive1 = 1.85f,
                    .drive2 = 2.20f,
                    .asymmetry = 0.10f,
                    .sagAmount = 0.04f,
                    .sagReleaseMs = 45.0f,
                    .cleanBlend = 0.16f,
                    .preHpHz = 90.0f,
                    .preShapeFreqHz = 1150.0f,
                    .preShapeGainDb = 1.0f,
                    .preShapeQ = 0.95f,
                    .fizzCutHz = 7600.0f,
                    .depthBaseDb = 0.0f,
                    .postDriveTrimDb = -3.0f,
                    .powerCompAmount = 0.06f,
                    .powerCompReleaseMs = 65.0f,
                };
            }

            return AmpModelParams {
                .drive1 = 2.35f,
                .drive2 = 2.95f,
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
                .postDriveTrimDb = -4.5f,
                .powerCompAmount = 0.12f,
                .powerCompReleaseMs = 95.0f,
            };

        case 2: // Amp 3: classic thrash tight + depth
            return AmpModelParams {
                .drive1 = 3.55f,
                .drive2 = 5.15f,
                .asymmetry = 0.09f,
                .sagAmount = 0.045f,
                .sagReleaseMs = 42.0f,
                .cleanBlend = 0.0f,
                .preHpHz = 145.0f,
                .preShapeFreqHz = 1720.0f,
                .preShapeGainDb = 2.2f,
                .preShapeQ = 1.28f,
                .fizzCutHz = 6600.0f,
                .depthBaseDb = 2.8f,
                .postDriveTrimDb = -7.8f,
                .powerCompAmount = 0.08f,
                .powerCompReleaseMs = 56.0f,
            };

        case 3: // Amp 4: modern metalcore modes
            if (!amp4TightMode)
            {
                return AmpModelParams {
                    .drive1 = 2.95f,
                    .drive2 = 4.10f,
                    .asymmetry = 0.18f,
                    .sagAmount = 0.10f,
                    .sagReleaseMs = 80.0f,
                    .cleanBlend = 0.03f,
                    .preHpHz = 118.0f,
                    .preShapeFreqHz = 850.0f,
                    .preShapeGainDb = -0.8f,
                    .preShapeQ = 0.95f,
                    .fizzCutHz = 5600.0f,
                    .depthBaseDb = 3.4f,
                    .postDriveTrimDb = -7.0f,
                    .powerCompAmount = 0.15f,
                    .powerCompReleaseMs = 102.0f,
                };
            }

            // Tight mode
            return AmpModelParams {
                .drive1 = 3.45f,
                .drive2 = 4.85f,
                .asymmetry = 0.11f,
                .sagAmount = 0.06f,
                .sagReleaseMs = 55.0f,
                .cleanBlend = 0.0f,
                .preHpHz = 145.0f,
                .preShapeFreqHz = 1550.0f,
                .preShapeGainDb = 1.8f,
                .preShapeQ = 1.25f,
                .fizzCutHz = 6300.0f,
                .depthBaseDb = 2.4f,
                .postDriveTrimDb = -6.0f,
                .powerCompAmount = 0.11f,
                .powerCompReleaseMs = 78.0f,
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

inline float cubicSoftLimit(float x, float amount)
{
    const float a = juce::jlimit(0.0f, 0.6f, amount);
    return x - a * x * x * x;
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
        powerCompEnvelope[static_cast<size_t>(channel)] = 0.0f;
        amp4OctavePolarity[static_cast<size_t>(channel)] = 1.0f;
        amp4OctaveEnv[static_cast<size_t>(channel)] = 0.0f;
        amp4OctaveLP[static_cast<size_t>(channel)] = 0.0f;
        amp4OctavePrevInput[static_cast<size_t>(channel)] = 0.0f;
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

void AmpEngine::setPreampDrive(float normalizedAmount)
{
    preampDrive = juce::jlimit(0.0f, 1.0f, normalizedAmount);
}

void AmpEngine::setAmp2HiwattMode(bool enabled)
{
    if (amp2HiwattMode == enabled)
        return;

    amp2HiwattMode = enabled;
    if (ampType == 1)
        updateToneFilters();
}

void AmpEngine::setAmp4TightMode(bool enabled)
{
    if (amp4TightMode == enabled)
        return;

    amp4TightMode = enabled;
    if (ampType == 3)
        updateToneFilters();
}

void AmpEngine::setAmp4BlendControls(bool lowOctaveEnabled, float lowOctMix, float directMix, float cleanAmpMix)
{
    amp4LowOctaveEnabled = lowOctaveEnabled;
    amp4LowOctaveMix = juce::jlimit(0.0f, 1.0f, lowOctMix);
    amp4DirectMix = juce::jlimit(0.0f, 1.0f, directMix);
    amp4CleanAmpMix = juce::jlimit(0.0f, 1.0f, cleanAmpMix);
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
    const auto model = getAmpModelParams(ampType, amp2HiwattMode, amp4TightMode);
    const float postDriveTrim = juce::Decibels::decibelsToGain(model.postDriveTrimDb);
    const float driveShaped = std::pow(preampDrive, 0.86f);
    const float preampGain = juce::jmap(driveShaped, 0.50f, 2.45f);
    const float driveLevelComp = juce::jmap(preampDrive, 1.04f, 0.64f);

    juce::dsp::AudioBlock<float> block(buffer);
    for (size_t ch = 0; ch < block.getNumChannels(); ++ch)
    {
        auto channelBlock = block.getSingleChannelBlock(ch);
        juce::dsp::ProcessContextReplacing<float> context(channelBlock);
        preHighPass[ch].process(context);
        preShape[ch].process(context);
    }

    const float boostGain = (inputBoost && (ampType == 1 || ampType == 2 || ampType == 3)) ? juce::Decibels::decibelsToGain(7.0f) : 1.0f;
    const float sagCoeff = std::exp(-1.0f / ((model.sagReleaseMs * 0.001f) * sampleRate));
    const float powerCompCoeff = std::exp(-1.0f / ((model.powerCompReleaseMs * 0.001f) * sampleRate));

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        float& sag = sagEnvelope[static_cast<size_t>(ch)];
        float& powerComp = powerCompEnvelope[static_cast<size_t>(ch)];
        float& octavePolarity = amp4OctavePolarity[static_cast<size_t>(ch)];
        float& octaveEnv = amp4OctaveEnv[static_cast<size_t>(ch)];
        float& octaveLP = amp4OctaveLP[static_cast<size_t>(ch)];
        float& octavePrevInput = amp4OctavePrevInput[static_cast<size_t>(ch)];

        const float octaveEnvCoeff = std::exp(-1.0f / (0.010f * sampleRate));
        const float octaveLPCoeff = std::exp(-1.0f / (0.0025f * sampleRate));

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float dry = data[i];
            float x = dry * inputGainLinear * boostGain * preampGain;

            const float rectified = std::abs(x);
            sag = std::max(rectified, sag * sagCoeff + rectified * (1.0f - sagCoeff));
            const float sagGain = juce::jlimit(0.62f, 1.0f, 1.0f - model.sagAmount * sag);

            x *= sagGain;

            const float stage1Drive = model.drive1 * juce::jmap(preampDrive, 0.90f, 1.16f);
            const float stage2Drive = model.drive2 * juce::jmap(preampDrive, 0.94f, 1.24f);
            float stage1 = asymSoftClip(x * stage1Drive, model.asymmetry);
            float stage2Input = stage1 + 0.28f * std::tanh(stage1 * 1.8f);
            float stage2 = asymSoftClip(stage2Input * stage2Drive, model.asymmetry * 0.85f);

            const float clipHardness = juce::jmap(preampDrive, 0.10f, 0.34f);
            const float stage3 = std::tanh(cubicSoftLimit(stage2, clipHardness) * 1.32f);
            x = 0.77f * stage2 + 0.23f * stage3;

            float out = x * (1.0f - model.cleanBlend) + dry * model.cleanBlend;

            const float compInput = std::abs(out);
            powerComp = powerCompCoeff * powerComp + (1.0f - powerCompCoeff) * compInput;
            const float powerCompDepth = model.powerCompAmount * juce::jmap(preampDrive, 0.65f, 1.25f);
            const float powerCompGain = juce::jlimit(0.66f, 1.0f, 1.0f - powerCompDepth * powerComp);
            out *= powerCompGain;

            if (ampType == 3 && !amp4TightMode)
            {
                const float cleanAmp = 0.62f * asymSoftClip(dry * inputGainLinear * 1.55f, 0.04f) + 0.38f * dry;

                float octave = 0.0f;
                if (amp4LowOctaveEnabled)
                {
                    const bool zeroUp = (octavePrevInput <= 0.0f && dry > 0.0f);
                    if (zeroUp)
                        octavePolarity = -octavePolarity;

                    octavePrevInput = dry;

                    const float targetEnv = std::abs(dry);
                    octaveEnv = octaveEnvCoeff * octaveEnv + (1.0f - octaveEnvCoeff) * targetEnv;
                    const float rawOct = octavePolarity * octaveEnv;
                    octaveLP = octaveLPCoeff * octaveLP + (1.0f - octaveLPCoeff) * rawOct;
                    octave = octaveLP * 0.95f;
                }

                const float dAdd = amp4DirectMix;
                const float cAdd = amp4CleanAmpMix;
                const float oAdd = amp4LowOctaveEnabled ? amp4LowOctaveMix : 0.0f;

                // Additive blend: processed signal stays at full level,
                // direct DI and clean amp are added on top at their own gain
                out = out + dry * dAdd + cleanAmp * cAdd + octave * oAdd;
            }

            data[i] = out * postDriveTrim * driveLevelComp;
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
    const auto model = getAmpModelParams(ampType, amp2HiwattMode, amp4TightMode);

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
        if (amp2HiwattMode)
        {
            lowFreq = 170.0f;
            midFreq = 980.0f;
            midQ = 0.95f;
            highFreq = 3200.0f;
            presenceFreq = 5200.0f;
        }
        else
        {
            lowFreq = 190.0f;
            midFreq = 760.0f;
            midQ = 1.0f;
            highFreq = 2600.0f;
            presenceFreq = 4100.0f;
        }
    }
    else if (ampType == 2)
    {
        lowFreq = 116.0f;
        midFreq = 1520.0f;
        midQ = 1.7f;
        highFreq = 3550.0f;
        presenceFreq = 6050.0f;
    }
    else if (ampType == 3)
    {
        if (amp4TightMode)
        {
            lowFreq = 120.0f;
            midFreq = 1320.0f;
            midQ = 1.35f;
            highFreq = 3300.0f;
            presenceFreq = 5600.0f;
        }
        else
        {
            lowFreq = 115.0f;
            midFreq = 900.0f;
            midQ = 1.0f;
            highFreq = 3000.0f;
            presenceFreq = 4700.0f;
        }
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
                                                                                                     ampType == 2 ? 115.0f : (ampType == 3 ? (amp4TightMode ? 108.0f : 95.0f) : 95.0f),
                                                                                                     0.65f,
                                                                                                     depthGain);
        fizzCut[channel].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate,
                                                                                           model.fizzCutHz,
                                                                                           0.707f);
    }
}
} // namespace backhouse
