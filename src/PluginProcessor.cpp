#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <array>
#include <cmath>
#include <utility>

namespace ids
{
static constexpr auto ampType = "ampType";
static constexpr auto guitarProfile = "guitarProfile";

static constexpr auto inputGain = "inputGain";
static constexpr auto stompInputGain = "stompInputGain";
static constexpr auto amp1PreampGain = "amp1PreampGain";
static constexpr auto amp2PreampGain = "amp2PreampGain";
static constexpr auto amp3PreampGain = "amp3PreampGain";
static constexpr auto amp4PreampGain = "amp4PreampGain";
static constexpr auto inputBoost = "inputBoost";
static constexpr auto sub = "sub";
static constexpr auto low = "low";
static constexpr auto mid = "mid";
static constexpr auto high = "high";
static constexpr auto presence = "presence";
static constexpr auto output = "output";

static constexpr auto gateThresh = "gateThresh";
static constexpr auto cabEnabled = "cabEnabled";
static constexpr auto cabTone = "cabTone";
static constexpr auto cabSpeaker = "cabSpeaker";
static constexpr auto cabMic = "cabMic";
static constexpr auto cabMicAOffAxis = "cabMicAOffAxis";
static constexpr auto cabSpeakerB = "cabSpeakerB";
static constexpr auto cabMicB = "cabMicB";
static constexpr auto cabMicBOffAxis = "cabMicBOffAxis";
static constexpr auto cabMicBlend = "cabMicBlend";
static constexpr auto reverbEnabled = "reverbEnabled";
static constexpr auto reverbMix = "reverbMix";
static constexpr auto tsEnabled = "tsEnabled";
static constexpr auto tsDrive = "tsDrive";
static constexpr auto tsTone = "tsTone";
static constexpr auto tsLevel = "tsLevel";
static constexpr auto delayEnabled = "delayEnabled";
static constexpr auto delayTimeMs = "delayTimeMs";
static constexpr auto delayFeedback = "delayFeedback";
static constexpr auto delayMix = "delayMix";
static constexpr auto delayTone = "delayTone";
static constexpr auto delayPitch = "delayPitch";
static constexpr auto wowEnabled = "wowEnabled";
static constexpr auto wowMomentary = "wowMomentary";
static constexpr auto wowMidiEnable = "wowMidiEnable";
static constexpr auto wowMode = "wowMode";
static constexpr auto wowPosition = "wowPosition";
static constexpr auto wowMix = "wowMix";
static constexpr auto phaserEnabled = "phaserEnabled";
static constexpr auto phaserRate = "phaserRate";
static constexpr auto phaserDepth = "phaserDepth";
static constexpr auto phaserCenterHz = "phaserCenterHz";
static constexpr auto phaserFeedback = "phaserFeedback";
static constexpr auto phaserMix = "phaserMix";
static constexpr auto stompCompEnabled = "stompCompEnabled";
static constexpr auto stompCompSustain = "stompCompSustain";
static constexpr auto stompCompLevel = "stompCompLevel";
static constexpr auto stompCompMix = "stompCompMix";
static constexpr auto output1176Enabled = "output1176Enabled";
static constexpr auto outputCompMode = "outputCompMode";
static constexpr auto output1176Input = "output1176Input";
static constexpr auto output1176Release = "output1176Release";
static constexpr auto output1176Mix = "output1176Mix";
static constexpr auto outputEqEnabled = "outputEqEnabled";
static constexpr auto outputEq63 = "outputEq63";
static constexpr auto outputEq160 = "outputEq160";
static constexpr auto outputEq400 = "outputEq400";
static constexpr auto outputEq1000 = "outputEq1000";
static constexpr auto outputEq2500 = "outputEq2500";
static constexpr auto outputEq6300 = "outputEq6300";
static constexpr auto outputEq12000 = "outputEq12000";
static constexpr auto rigAmp1 = "rigAmp1";
static constexpr auto rigAmp3 = "rigAmp3";
static constexpr auto rigCompressor = "rigCompressor";
static constexpr auto rigCab = "rigCab";
static constexpr auto rigDelay = "rigDelay";
static constexpr auto rigEQ = "rigEQ";
static constexpr auto rigPhaser = "rigPhaser";
static constexpr auto rigTrem = "rigTrem";
static constexpr auto rigWOW = "rigWOW";
static constexpr auto rigMaxPoints = "rigMaxPoints";
static constexpr auto outputEqQ63 = "outputEqQ63";
static constexpr auto outputEqQ160 = "outputEqQ160";
static constexpr auto outputEqQ400 = "outputEqQ400";
static constexpr auto outputEqQ1000 = "outputEqQ1000";
static constexpr auto outputEqQ2500 = "outputEqQ2500";
static constexpr auto outputEqQ6300 = "outputEqQ6300";
static constexpr auto outputEqQ12000 = "outputEqQ12000";
static constexpr auto outputEqHpEnabled = "outputEqHpEnabled";
static constexpr auto outputEqHpHz = "outputEqHpHz";
static constexpr auto outputEqLpEnabled = "outputEqLpEnabled";
static constexpr auto outputEqLpHz = "outputEqLpHz";
static constexpr auto amp1TremEnabled = "amp1TremEnabled";
static constexpr auto amp1TremRate = "amp1TremRate";
static constexpr auto amp1TremDepth = "amp1TremDepth";
static constexpr auto amp2Hiwatt = "amp2Hiwatt";
static constexpr auto amp4Tight = "amp4Tight";
static constexpr auto amp4LowOctave = "amp4LowOctave";
static constexpr auto amp4LowOctaveMix = "amp4LowOctaveMix";
static constexpr auto amp4DirectMix = "amp4DirectMix";
static constexpr auto amp4CleanAmpMix = "amp4CleanAmpMix";
static constexpr auto tunerEnabled = "tunerEnabled";
static constexpr auto tunerTranspose = "tunerTranspose";
static constexpr auto whammyEnabled = "whammyEnabled";
static constexpr auto whammyMode    = "whammyMode";
static constexpr auto whammyPosition = "whammyPosition";
static constexpr auto whammyMix     = "whammyMix";
static constexpr auto wahEnabled    = "wahEnabled";
static constexpr auto wahPosition   = "wahPosition";
static constexpr auto wahMix        = "wahMix";

static constexpr auto profile1PickupOutput = "profile1PickupOutput";
static constexpr auto profile1Brightness = "profile1Brightness";
static constexpr auto profile1LowEnd = "profile1LowEnd";
static constexpr auto profile1GateTrim = "profile1GateTrim";

static constexpr auto profile2PickupOutput = "profile2PickupOutput";
static constexpr auto profile2Brightness = "profile2Brightness";
static constexpr auto profile2LowEnd = "profile2LowEnd";
static constexpr auto profile2GateTrim = "profile2GateTrim";

static constexpr auto profile3PickupOutput = "profile3PickupOutput";
static constexpr auto profile3Brightness = "profile3Brightness";
static constexpr auto profile3LowEnd = "profile3LowEnd";
static constexpr auto profile3GateTrim = "profile3GateTrim";

static constexpr auto profileName1 = "profileName1";
static constexpr auto profileName2 = "profileName2";
static constexpr auto profileName3 = "profileName3";
static constexpr auto userIRPath = "userIRPath";
static constexpr auto reverbIRPath = "reverbIRPath";
}

namespace
{
constexpr std::array<const char*, 3> profileNamePropertyIds {
    ids::profileName1,
    ids::profileName2,
    ids::profileName3
};

constexpr std::array<const char*, 3> profileDefaultNames {
    "Guitar 1",
    "Guitar 2",
    "Guitar 3"
};

constexpr std::array<const char*, 3> pickupOutputIds {
    ids::profile1PickupOutput,
    ids::profile2PickupOutput,
    ids::profile3PickupOutput
};

constexpr std::array<const char*, 3> brightnessIds {
    ids::profile1Brightness,
    ids::profile2Brightness,
    ids::profile3Brightness
};

constexpr std::array<const char*, 3> lowEndIds {
    ids::profile1LowEnd,
    ids::profile2LowEnd,
    ids::profile3LowEnd
};

constexpr std::array<const char*, 3> gateTrimIds {
    ids::profile1GateTrim,
    ids::profile2GateTrim,
    ids::profile3GateTrim
};

constexpr std::array<const char*, 4> preampGainIds {
    ids::amp1PreampGain,
    ids::amp2PreampGain,
    ids::amp3PreampGain,
    ids::amp4PreampGain
};

constexpr std::array<const char*, 7> outputEqGainIds {
    ids::outputEq63,
    ids::outputEq160,
    ids::outputEq400,
    ids::outputEq1000,
    ids::outputEq2500,
    ids::outputEq6300,
    ids::outputEq12000
};

constexpr std::array<const char*, 7> outputEqQIds {
    ids::outputEqQ63,
    ids::outputEqQ160,
    ids::outputEqQ400,
    ids::outputEqQ1000,
    ids::outputEqQ2500,
    ids::outputEqQ6300,
    ids::outputEqQ12000
};

constexpr std::array<float, 7> outputEqFrequencies {
    63.0f, 160.0f, 400.0f, 1000.0f, 2500.0f, 6300.0f, 12000.0f
};

constexpr std::array<float, 16> outputSpectrumFrequencies {
    63.0f, 90.0f, 125.0f, 180.0f, 250.0f, 355.0f, 500.0f, 710.0f,
    1000.0f, 1400.0f, 2000.0f, 2800.0f, 4000.0f, 5600.0f, 8000.0f, 12000.0f
};

constexpr auto profileSchema = "backhousedsp.guitar_profiles";
constexpr int profileSchemaVersion = 1;

float computePeakLevel(const juce::AudioBuffer<float>& buffer, int numChannels)
{
    const int channels = juce::jlimit(0, buffer.getNumChannels(), numChannels);
    float peak = 0.0f;

    for (int ch = 0; ch < channels; ++ch)
        peak = juce::jmax(peak, buffer.getMagnitude(ch, 0, buffer.getNumSamples()));

    return peak;
}

void updateMeterLevel(std::atomic<float>& meter, float newPeak)
{
    constexpr float attack = 0.45f;
    constexpr float release = 0.08f;
    const float current = meter.load(std::memory_order_relaxed);
    const float coeff = newPeak > current ? attack : release;
    meter.store(current + coeff * (newPeak - current), std::memory_order_relaxed);
}

float readInterpolatedSample(const juce::AudioBuffer<float>& buffer, int channel, double samplePos)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return 0.0f;

    const int base = juce::jlimit(0, numSamples - 1, static_cast<int>(samplePos));
    const int next = juce::jmin(numSamples - 1, base + 1);
    const float frac = static_cast<float>(samplePos - static_cast<double>(base));

    const float s0 = buffer.getSample(channel, base);
    const float s1 = buffer.getSample(channel, next);
    return s0 + frac * (s1 - s0);
}
} // namespace

BackhouseAmpSimAudioProcessor::BackhouseAmpSimAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BackhouseAmpSimParameters", createParameterLayout())
{
    ensureProfileNameProperties();

    mappableParameterIds = {
        ids::inputGain, ids::amp1PreampGain, ids::amp2PreampGain, ids::amp3PreampGain, ids::amp4PreampGain,
        ids::stompInputGain, ids::sub, ids::low, ids::mid, ids::high, ids::presence, ids::output,
        ids::gateThresh, ids::cabTone, ids::cabMicBlend, ids::reverbMix, ids::wowPosition, ids::wowMix, ids::phaserDepth, ids::phaserCenterHz, ids::phaserFeedback, ids::phaserMix,
        ids::stompCompSustain, ids::stompCompLevel, ids::stompCompMix, ids::output1176Input, ids::output1176Release, ids::output1176Mix,
        ids::outputEq63, ids::outputEq160, ids::outputEq400, ids::outputEq1000, ids::outputEq2500, ids::outputEq6300, ids::outputEq12000,
        ids::outputEqQ63, ids::outputEqQ160, ids::outputEqQ400, ids::outputEqQ1000, ids::outputEqQ2500, ids::outputEqQ6300, ids::outputEqQ12000,
        ids::outputEqHpHz, ids::outputEqLpHz,
        ids::amp1TremRate, ids::amp1TremDepth,
        ids::tsDrive, ids::tsTone, ids::tsLevel, ids::delayTimeMs, ids::delayFeedback, ids::delayMix, ids::delayTone, ids::delayPitch,
        ids::inputBoost, ids::reverbEnabled, ids::stompCompEnabled, ids::output1176Enabled, ids::outputEqEnabled, ids::outputEqHpEnabled, ids::outputEqLpEnabled, ids::amp1TremEnabled, ids::tunerEnabled, ids::amp4Tight
    };
    mappableParameterNames = {
        "Input Gain", "Amp1 Preamp", "Amp2 Preamp", "Amp3 Preamp", "Amp4 Preamp",
        "Stomp Input Gain", "Sub", "Low", "Mid", "High", "Presence", "Output",
        "Gate Threshold", "Cab Tone", "Mic Blend", "Reverb Mix", "WOW Position", "WOW Mix", "Phaser Depth", "Phaser Center", "Phaser Feedback", "Phaser Mix",
        "Stomp Comp Sustain", "Stomp Comp Level", "Stomp Comp Mix", "1176 Input", "1176 Release", "1176 Mix",
        "OutEQ 63", "OutEQ 160", "OutEQ 400", "OutEQ 1k", "OutEQ 2.5k", "OutEQ 6.3k", "OutEQ 12k",
        "OutEQ Q63", "OutEQ Q160", "OutEQ Q400", "OutEQ Q1k", "OutEQ Q2.5k", "OutEQ Q6.3k", "OutEQ Q12k",
        "OutEQ HP Hz", "OutEQ LP Hz",
        "Amp1 Trem Rate", "Amp1 Trem Depth",
        "TS Drive", "TS Tone", "TS Level", "Delay Time", "Delay Feedback", "Delay Mix", "Delay Tone", "Delay Pitch",
        "Input Boost", "Reverb Enabled", "Stomp Comp Enabled", "1176 Enabled", "Output EQ Enabled", "OutEQ HP On", "OutEQ LP On", "Amp1 Trem Enabled", "Tuner Enabled", "Amp4 Tight"
    };

    // FCB1010 starter template: expression + common footswitch controls.
    midiMapSlots[0] = { true, 27, ids::wowPosition, 0, 0.0f, 1.0f, false };
    midiMapSlots[1] = { true, 7, ids::wowMix, 0, 0.0f, 1.0f, false };
    midiMapSlots[2] = { true, 20, ids::inputBoost, 2, 0.0f, 1.0f, false };
    midiMapSlots[3] = { true, 21, ids::reverbEnabled, 1, 0.0f, 1.0f, false };
    midiMapSlots[4] = { true, 22, ids::reverbMix, 0, 0.0f, 1.0f, false };
    midiMapSlots[5] = { true, 23, ids::tunerEnabled, 2, 0.0f, 1.0f, false };
    midiMapSlots[6] = { true, 24, ids::amp4Tight, 1, 0.0f, 1.0f, false };
    midiMapSlots[7] = { true, 25, ids::output, 0, 0.0f, 1.0f, false };
}

void BackhouseAmpSimAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    noiseGate.prepare(spec);
    ampEngine.prepare(spec);
    cabSimulator.prepare(spec);
    reverbIR.prepare(spec);
    phaser.prepare(spec);
    phaser.reset();
    stompCompressor.prepare(spec);
    output1176Compressor.prepare(spec);
    stompCompressor.reset();
    output1176Compressor.reset();
    for (int band = 0; band < static_cast<int>(outputEqBands.size()); ++band)
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            outputEqBands[static_cast<size_t>(band)][static_cast<size_t>(ch)].prepare(spec);
            outputEqBands[static_cast<size_t>(band)][static_cast<size_t>(ch)].reset();
        }
    }
    for (int ch = 0; ch < 2; ++ch)
    {
        outputEqHighPass[static_cast<size_t>(ch)].prepare(spec);
        outputEqHighPass[static_cast<size_t>(ch)].reset();
        outputEqLowPass[static_cast<size_t>(ch)].prepare(spec);
        outputEqLowPass[static_cast<size_t>(ch)].reset();
    }
    for (auto& bin : outputSpectrumBins)
        bin.store(0.0f, std::memory_order_relaxed);
    for (int ch = 0; ch < 2; ++ch)
    {
        tsInputHP[ch].prepare(spec);
        tsToneLP[ch].prepare(spec);
        tsInputHP[ch].reset();
        tsToneLP[ch].reset();
        tsInputHP[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 720.0f, 0.707f);
        tsToneLP[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 3200.0f, 0.707f);
        delayToneState[ch] = 0.0f;
    }
    delayBufferSize = juce::jmax(2048, static_cast<int>(sampleRate * 3.5));
    for (auto& b : delayBuffers)
        b.assign(static_cast<size_t>(delayBufferSize), 0.0f);
    delayWritePos = 0;

    delayPitchBufferSize = juce::jmax(1024, static_cast<int>(sampleRate * 0.08));
    for (auto& b : delayPitchBuffers)
        b.assign(static_cast<size_t>(delayPitchBufferSize), 0.0f);
    delayPitchWritePos = 0;
    delayPitchPhase = 0.0f;

    wowDelaySize = juce::jmax(2048, static_cast<int>(sampleRate * 0.14));
    for (auto& channelBuffer : wowDelayBuffers)
    {
        channelBuffer.assign(static_cast<size_t>(wowDelaySize), 0.0f);
    }
    wowWritePos = 0;
    wowShiftPhase = 0.0f;
    tremPhase = 0.0f;
    wowMidiLastPosition = 0.45f;
    wowMidiLastMix = 0.65f;
    wowMidiStompDown = false;
    tuner.prepare(sampleRate);
    whammyEffect.prepare(sampleRate, samplesPerBlock);
    tryRestoreUserIRFromState();
    tryRestoreReverbIRFromState();

    smoothingSampleRate = sampleRate;
    smoothedOffsets = {};
}

void BackhouseAmpSimAudioProcessor::releaseResources()
{
}

bool BackhouseAmpSimAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn != juce::AudioChannelSet::mono() && mainIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void BackhouseAmpSimAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalIn = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();

    for (auto ch = totalIn; ch < totalOut; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    renderTestDI(buffer);
    updateMeterLevel(inputMeterLevel, computePeakLevel(buffer, totalIn));

    // FCB1010-friendly WOW MIDI mapping:
    // CC27 = expression A (position), CC7 = expression B (mix), CC64 = stomp gate.
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (!msg.isController())
            continue;

        const int cc = msg.getControllerNumber();
        const float value = static_cast<float>(msg.getControllerValue()) / 127.0f;
        if (cc == 27)
            wowMidiLastPosition = juce::jlimit(0.0f, 1.0f, value);
        else if (cc == 7)
            wowMidiLastMix = juce::jlimit(0.0f, 1.0f, value);
        else if (cc == 64)
            wowMidiStompDown = msg.getControllerValue() >= 64;

        applyMappedMidiController(cc, msg.getControllerValue());
    }

    const int amp = static_cast<int>(apvts.getRawParameterValue(ids::ampType)->load());
    const int guitarProfile = static_cast<int>(apvts.getRawParameterValue(ids::guitarProfile)->load());

    const float inputGainDb = apvts.getRawParameterValue(ids::inputGain)->load();
    const float stompInputGainDb = apvts.getRawParameterValue(ids::stompInputGain)->load();
    const float preampGain = apvts.getRawParameterValue(preampGainIds[static_cast<size_t>(juce::jlimit(0, 3, amp))])->load();
    const bool inputBoost = apvts.getRawParameterValue(ids::inputBoost)->load() > 0.5f;
    const float subDb = apvts.getRawParameterValue(ids::sub)->load();
    const float lowDb = apvts.getRawParameterValue(ids::low)->load();
    const float midDb = apvts.getRawParameterValue(ids::mid)->load();
    const float highDb = apvts.getRawParameterValue(ids::high)->load();
    const float presenceDb = apvts.getRawParameterValue(ids::presence)->load();
    const float outputDb = apvts.getRawParameterValue(ids::output)->load();

    const float gateThreshDb = apvts.getRawParameterValue(ids::gateThresh)->load();
    const bool cabEnabled = apvts.getRawParameterValue(ids::cabEnabled)->load() > 0.5f;
    const float cabTone = apvts.getRawParameterValue(ids::cabTone)->load();
    const int cabSpeaker = static_cast<int>(apvts.getRawParameterValue(ids::cabSpeaker)->load());
    const int cabMic = static_cast<int>(apvts.getRawParameterValue(ids::cabMic)->load());
    const bool cabMicAOffAxis = apvts.getRawParameterValue(ids::cabMicAOffAxis)->load() > 0.5f;
    const int cabSpeakerB = static_cast<int>(apvts.getRawParameterValue(ids::cabSpeakerB)->load());
    const int cabMicB = static_cast<int>(apvts.getRawParameterValue(ids::cabMicB)->load());
    const bool cabMicBOffAxis = apvts.getRawParameterValue(ids::cabMicBOffAxis)->load() > 0.5f;
    const float cabMicBlend = apvts.getRawParameterValue(ids::cabMicBlend)->load();
    const bool reverbEnabled = apvts.getRawParameterValue(ids::reverbEnabled)->load() > 0.5f;
    const float reverbMix = apvts.getRawParameterValue(ids::reverbMix)->load();
    const bool tsEnabled = apvts.getRawParameterValue(ids::tsEnabled)->load() > 0.5f;
    const float tsDrive = apvts.getRawParameterValue(ids::tsDrive)->load();
    const float tsTone = apvts.getRawParameterValue(ids::tsTone)->load();
    const float tsLevel = apvts.getRawParameterValue(ids::tsLevel)->load();
    const bool stompCompEnabled = apvts.getRawParameterValue(ids::stompCompEnabled)->load() > 0.5f;
    const float stompCompSustain = apvts.getRawParameterValue(ids::stompCompSustain)->load();
    const float stompCompLevel = apvts.getRawParameterValue(ids::stompCompLevel)->load();
    const float stompCompMix = apvts.getRawParameterValue(ids::stompCompMix)->load();
    const bool delayEnabled = apvts.getRawParameterValue(ids::delayEnabled)->load() > 0.5f;
    const float delayTimeMs = apvts.getRawParameterValue(ids::delayTimeMs)->load();
    const float delayFeedbackAmt = apvts.getRawParameterValue(ids::delayFeedback)->load();
    const float delayMixAmt = apvts.getRawParameterValue(ids::delayMix)->load();
    const float delayToneAmt = apvts.getRawParameterValue(ids::delayTone)->load();
    const float delayPitchSemis = apvts.getRawParameterValue(ids::delayPitch)->load();
    const bool wowEnabled = apvts.getRawParameterValue(ids::wowEnabled)->load() > 0.5f;
    const bool wowMomentary = apvts.getRawParameterValue(ids::wowMomentary)->load() > 0.5f;
    const bool wowMidiEnable = apvts.getRawParameterValue(ids::wowMidiEnable)->load() > 0.5f;
    const int wowMode = static_cast<int>(apvts.getRawParameterValue(ids::wowMode)->load());
    const float wowPosition = apvts.getRawParameterValue(ids::wowPosition)->load();
    const float wowMix = apvts.getRawParameterValue(ids::wowMix)->load();
    const bool phaserEnabled = apvts.getRawParameterValue(ids::phaserEnabled)->load() > 0.5f;
    const float phaserRateHz = apvts.getRawParameterValue(ids::phaserRate)->load();
    const float phaserDepthAmt = apvts.getRawParameterValue(ids::phaserDepth)->load();
    const float phaserCenterHz = apvts.getRawParameterValue(ids::phaserCenterHz)->load();
    const float phaserFeedbackAmt = apvts.getRawParameterValue(ids::phaserFeedback)->load();
    const float phaserMix = apvts.getRawParameterValue(ids::phaserMix)->load();
    const bool amp1TremEnabled = apvts.getRawParameterValue(ids::amp1TremEnabled)->load() > 0.5f;
    const float amp1TremRateHz = apvts.getRawParameterValue(ids::amp1TremRate)->load();
    const float amp1TremDepth = apvts.getRawParameterValue(ids::amp1TremDepth)->load();
    const bool amp2Hiwatt = apvts.getRawParameterValue(ids::amp2Hiwatt)->load() > 0.5f;
    const bool amp4Tight = apvts.getRawParameterValue(ids::amp4Tight)->load() > 0.5f;
    const bool amp4LowOctave = apvts.getRawParameterValue(ids::amp4LowOctave)->load() > 0.5f;
    const float amp4LowOctaveMix = apvts.getRawParameterValue(ids::amp4LowOctaveMix)->load();
    const float amp4DirectMix = apvts.getRawParameterValue(ids::amp4DirectMix)->load();
    const float amp4CleanAmpMix = apvts.getRawParameterValue(ids::amp4CleanAmpMix)->load();
    const bool tunerEnabled = apvts.getRawParameterValue(ids::tunerEnabled)->load() > 0.5f;
    const bool output1176Enabled = apvts.getRawParameterValue(ids::output1176Enabled)->load() > 0.5f;
    const int outputCompMode = static_cast<int>(apvts.getRawParameterValue(ids::outputCompMode)->load());
    const float output1176Input = apvts.getRawParameterValue(ids::output1176Input)->load();
    const float output1176Release = apvts.getRawParameterValue(ids::output1176Release)->load();
    const float output1176Mix = apvts.getRawParameterValue(ids::output1176Mix)->load();
    const bool outputEqEnabled = apvts.getRawParameterValue(ids::outputEqEnabled)->load() > 0.5f;
    const bool outputEqHpEnabled = apvts.getRawParameterValue(ids::outputEqHpEnabled)->load() > 0.5f;
    const float outputEqHpCutoff = apvts.getRawParameterValue(ids::outputEqHpHz)->load();
    const bool outputEqLpEnabled = apvts.getRawParameterValue(ids::outputEqLpEnabled)->load() > 0.5f;
    const float outputEqLpCutoff = apvts.getRawParameterValue(ids::outputEqLpHz)->load();

    const auto targetOffsets = getProfileOffsetsFromParams(guitarProfile);

    const float tauSeconds = 0.035f;
    const float blockSeconds = static_cast<float>(buffer.getNumSamples() / smoothingSampleRate);
    const float alpha = 1.0f - std::exp(-blockSeconds / tauSeconds);
    auto smoothOne = [alpha](float& current, float target) {
        current += alpha * (target - current);
    };

    smoothOne(smoothedOffsets.inputGainDb, targetOffsets.inputGainDb);
    smoothOne(smoothedOffsets.lowDb, targetOffsets.lowDb);
    smoothOne(smoothedOffsets.midDb, targetOffsets.midDb);
    smoothOne(smoothedOffsets.highDb, targetOffsets.highDb);
    smoothOne(smoothedOffsets.presenceDb, targetOffsets.presenceDb);
    smoothOne(smoothedOffsets.gateThresholdDb, targetOffsets.gateThresholdDb);

    const float profInputGainDb = juce::jlimit(-24.0f, 24.0f, inputGainDb + smoothedOffsets.inputGainDb);
    const float profLowDb = juce::jlimit(-18.0f, 18.0f, lowDb + smoothedOffsets.lowDb);
    const float profMidDb = juce::jlimit(-18.0f, 18.0f, midDb + smoothedOffsets.midDb);
    const float profHighDb = juce::jlimit(-18.0f, 18.0f, highDb + smoothedOffsets.highDb);
    const float profPresenceDb = juce::jlimit(-12.0f, 12.0f, presenceDb + smoothedOffsets.presenceDb);
    const float profGateThreshDb = juce::jlimit(-80.0f, -20.0f, gateThreshDb + smoothedOffsets.gateThresholdDb);

    if (std::abs(stompInputGainDb) > 0.001f)
        buffer.applyGain(juce::Decibels::decibelsToGain(stompInputGainDb));

    if (stompCompEnabled && stompCompMix > 0.0001f)
    {
        juce::AudioBuffer<float> dry;
        dry.makeCopyOf(buffer, true);
        const float sustain = juce::jlimit(0.0f, 1.0f, stompCompSustain);
        stompCompressor.setAttack(juce::jmap(sustain, 4.0f, 12.0f));
        stompCompressor.setRelease(juce::jmap(sustain, 100.0f, 260.0f));
        stompCompressor.setThreshold(juce::jmap(sustain, -14.0f, -36.0f));
        stompCompressor.setRatio(juce::jmap(sustain, 2.2f, 8.0f));
        juce::dsp::AudioBlock<float> compBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> compContext(compBlock);
        stompCompressor.process(compContext);

        const float levelGain = juce::jlimit(0.0f, 2.0f, stompCompLevel);
        const float mix = juce::jlimit(0.0f, 1.0f, stompCompMix);
        const float dryGain = 1.0f - mix;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* w = buffer.getWritePointer(ch);
            const auto* d = dry.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                w[i] = dryGain * d[i] + mix * w[i] * levelGain;
        }
    }

    // ---- Whammy ----
    {
        const bool whammyEnabled = apvts.getRawParameterValue(ids::whammyEnabled)->load() > 0.5f;
        if (whammyEnabled)
        {
            const int   whammyMode     = static_cast<int>(apvts.getRawParameterValue(ids::whammyMode)->load());
            const float whammyPosition = apvts.getRawParameterValue(ids::whammyPosition)->load();
            const float whammyMix      = apvts.getRawParameterValue(ids::whammyMix)->load();

            // semitone target per mode (+1Oct, +2Oct, -1Oct, -2Oct, +5th, +4th)
            static constexpr std::array<float, 6> modeSemitones { 12.0f, 24.0f, -12.0f, -24.0f, 7.0f, 5.0f };
            const float targetSemis = whammyPosition
                * modeSemitones[static_cast<size_t>(juce::jlimit(0, 5, whammyMode))];
            const float pitchRatio  = std::pow(2.0f, targetSemis / 12.0f);

            whammyEffect.processBlock(buffer, pitchRatio, whammyMix);
        }
    }

    if (tsEnabled)
    {
        const float drive = juce::jmap(tsDrive, 0.8f, 6.5f);
        const float hpFreq = juce::jmap(tsTone, 520.0f, 900.0f);
        const float lpFreq = juce::jmap(tsTone, 1800.0f, 6200.0f);
        for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
        {
            tsInputHP[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), hpFreq, 0.707f);
            tsToneLP[ch].coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), lpFreq, 0.707f);
            auto* w = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                float x = tsInputHP[ch].processSample(w[i]);
                x = std::tanh(drive * x);
                x = tsToneLP[ch].processSample(x);
                w[i] = x * tsLevel;
            }
        }
    }

    if (phaserEnabled && phaserMix > 0.0001f)
    {
        phaser.setRate(phaserRateHz);
        phaser.setDepth(juce::jlimit(0.0f, 1.0f, phaserDepthAmt));
        phaser.setCentreFrequency(juce::jlimit(120.0f, 4000.0f, phaserCenterHz));
        phaser.setFeedback(juce::jlimit(-0.95f, 0.95f, phaserFeedbackAmt));
        phaser.setMix(phaserMix);
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        phaser.process(context);
    }

    // ── Crybaby wah ──────────────────────────────────────────────────────────
    // Classic GCB-95 character: Chamberlin SVF bandpass, 350Hz→2.2kHz log sweep, Q≈3.5
    {
        const bool  wahEn  = apvts.getRawParameterValue(ids::wahEnabled)->load() > 0.5f;
        const float wahPos = apvts.getRawParameterValue(ids::wahPosition)->load();
        const float wahMx  = apvts.getRawParameterValue(ids::wahMix)->load();

        if (wahEn && wahMx > 0.0001f)
        {
            const float freqHz = 350.0f * std::pow(2200.0f / 350.0f,
                                                    juce::jlimit(0.0f, 1.0f, wahPos));
            const float F      = 2.0f * std::sin(juce::MathConstants<float>::pi * freqHz
                                                  / static_cast<float>(juce::jmax(1.0, getSampleRate())));
            constexpr float wahQinv = 1.0f / 3.5f;   // Q ≈ 3.5  (classic resonance)
            const int numSamples    = buffer.getNumSamples();
            const int numChannels   = juce::jmin(buffer.getNumChannels(), 2);

            for (int i = 0; i < numSamples; ++i)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const float in = buffer.getSample(ch, i);
                    const float hp = in - wahLowpass[ch] - wahQinv * wahBandpass[ch];
                    wahBandpass[ch] += F * hp;
                    wahLowpass[ch]  += F * wahBandpass[ch];
                    // Scale by wahQinv so peak gain ≈ unity at resonant frequency
                    const float wet = wahBandpass[ch] * wahQinv;
                    buffer.setSample(ch, i, in * (1.0f - wahMx) + wet * wahMx);
                }
            }
        }
    }

    if (wowEnabled && wowMix > 0.0001f)
    {
        juce::AudioBuffer<float> dry;
        dry.makeCopyOf(buffer, true);
        const float effectivePosition = wowMidiEnable ? wowMidiLastPosition : wowPosition;
        float effectiveMix = wowMidiEnable ? wowMidiLastMix : wowMix;
        if (wowMomentary && wowMidiEnable && !wowMidiStompDown)
            effectiveMix = 0.0f;

        const float mix = juce::jlimit(0.0f, 1.0f, effectiveMix);
        if (mix > 0.0001f)
        {
            const float dryGain = 1.0f - mix;
            const float pedal = juce::jlimit(0.0f, 1.0f, effectivePosition);

            // Gojira-style WOW behavior: expression-controlled pitch sweep, not auto-LFO.
            float semitones = 0.0f;
            switch (juce::jlimit(0, 2, wowMode))
            {
                case 0: semitones = juce::jmap(pedal, -12.0f, 12.0f); break; // FATSO-like
                case 1: semitones = juce::jmap(pedal, 0.0f, 12.0f); break;   // BLADE 1-like
                case 2: semitones = juce::jmap(pedal, 0.0f, 24.0f); break;   // BLADE 2-like
                default: break;
            }

            const float ratio = std::pow(2.0f, semitones / 12.0f);
            const float grainSamples = juce::jlimit(320.0f, static_cast<float>(wowDelaySize - 8), static_cast<float>(getSampleRate()) * 0.045f);
            const float phaseStep = (1.0f - ratio) / grainSamples;

            auto readDelaySample = [this](int channel, float delaySamples) -> float {
                if (wowDelaySize <= 1)
                    return 0.0f;

                float readPos = static_cast<float>(wowWritePos) - delaySamples;
                while (readPos < 0.0f)
                    readPos += static_cast<float>(wowDelaySize);
                while (readPos >= static_cast<float>(wowDelaySize))
                    readPos -= static_cast<float>(wowDelaySize);

                const int i0 = static_cast<int>(readPos);
                const int i1 = (i0 + 1) % wowDelaySize;
                const float frac = readPos - static_cast<float>(i0);
                const auto& delay = wowDelayBuffers[static_cast<size_t>(juce::jlimit(0, 1, channel))];
                return delay[static_cast<size_t>(i0)] + frac * (delay[static_cast<size_t>(i1)] - delay[static_cast<size_t>(i0)]);
            };

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                wowShiftPhase += phaseStep;
                while (wowShiftPhase < 0.0f)
                    wowShiftPhase += 1.0f;
                while (wowShiftPhase >= 1.0f)
                    wowShiftPhase -= 1.0f;

                const float delayA = wowShiftPhase * grainSamples;
                const float delayB = std::fmod(delayA + 0.5f * grainSamples, grainSamples);
                const float winA = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * wowShiftPhase);
                const float winB = 1.0f - winA;

                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                {
                    const float in = buffer.getSample(ch, sample);
                    auto& delay = wowDelayBuffers[static_cast<size_t>(juce::jmin(ch, 1))];
                    delay[static_cast<size_t>(wowWritePos)] = in;

                    const float a = readDelaySample(ch, delayA);
                    const float b = readDelaySample(ch, delayB);
                    const float wet = (winA * a) + (winB * b);
                    const float out = (dryGain * dry.getSample(ch, sample)) + (mix * wet);
                    buffer.setSample(ch, sample, out);
                }

                wowWritePos = (wowWritePos + 1) % juce::jmax(1, wowDelaySize);
            }
        }
    }

    noiseGate.setThresholdDb(profGateThreshDb);
    noiseGate.process(buffer);

    ampEngine.setAmpType(amp);
    ampEngine.setInputGainDb(profInputGainDb);
    ampEngine.setPreampDrive(preampGain);
    ampEngine.setInputBoost(inputBoost);
    ampEngine.setAmp2HiwattMode(amp2Hiwatt);
    ampEngine.setAmp4TightMode(amp4Tight);
    ampEngine.setAmp4BlendControls(amp4LowOctave, amp4LowOctaveMix, amp4DirectMix, amp4CleanAmpMix);
    ampEngine.setToneDb(profLowDb, profMidDb, profHighDb, profPresenceDb);
    ampEngine.setSubDb(subDb);
    ampEngine.process(buffer);

    if (amp == 0 && amp1TremEnabled && amp1TremDepth > 0.0001f)
    {
        const float phaseStep = juce::MathConstants<float>::twoPi * amp1TremRateHz / static_cast<float>(juce::jmax(1.0, getSampleRate()));
        const float depth = juce::jlimit(0.0f, 1.0f, amp1TremDepth);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float lfo = 0.5f * (1.0f + std::sin(tremPhase));
            const float gain = (1.0f - depth) + (depth * lfo);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample(ch, i, buffer.getSample(ch, i) * gain);

            tremPhase += phaseStep;
            if (tremPhase >= juce::MathConstants<float>::twoPi)
                tremPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    cabSimulator.setEnabled(cabEnabled);
    cabSimulator.setAmpType(amp);
    cabSimulator.setSpeakerModel(cabSpeaker);
    cabSimulator.setMicModel(cabMic);
    cabSimulator.setMicAOffAxis(cabMicAOffAxis);
    cabSimulator.setSpeakerModelB(cabSpeakerB);
    cabSimulator.setMicModelB(cabMicB);
    cabSimulator.setMicBOffAxis(cabMicBOffAxis);
    cabSimulator.setMicBlend(cabMicBlend);
    cabSimulator.setAmp4TightMode(amp4Tight);
    cabSimulator.setTone(cabTone);
    cabSimulator.process(buffer);

    if (delayEnabled && delayMixAmt > 0.0001f && delayBufferSize > 8)
    {
        const float timeSamples = juce::jlimit(1.0f, static_cast<float>(delayBufferSize - 4), delayTimeMs * 0.001f * static_cast<float>(getSampleRate()));
        const float feedback = juce::jlimit(0.0f, 0.95f, delayFeedbackAmt);
        const float mix = juce::jlimit(0.0f, 1.0f, delayMixAmt);
        const float toneHz = juce::jmap(delayToneAmt, 1200.0f, 9800.0f);
        const float dt = 1.0f / static_cast<float>(juce::jmax(1.0, getSampleRate()));
        const float rc = 1.0f / (juce::MathConstants<float>::twoPi * toneHz);
        const float alpha = dt / (rc + dt);
        const float pitchRatio = std::pow(2.0f, delayPitchSemis / 12.0f);
        const float grain = juce::jlimit(120.0f, static_cast<float>(delayPitchBufferSize - 8), static_cast<float>(getSampleRate()) * 0.03f);
        const float pitchStep = (1.0f - pitchRatio) / grain;

        auto readInterp = [](const std::vector<float>& b, int size, float pos) -> float {
            float p = pos;
            while (p < 0.0f) p += static_cast<float>(size);
            while (p >= static_cast<float>(size)) p -= static_cast<float>(size);
            const int i0 = static_cast<int>(p);
            const int i1 = (i0 + 1) % size;
            const float frac = p - static_cast<float>(i0);
            return b[static_cast<size_t>(i0)] + frac * (b[static_cast<size_t>(i1)] - b[static_cast<size_t>(i0)]);
        };

        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto& d = delayBuffers[static_cast<size_t>(juce::jmin(ch, 1))];
                const float readPos = static_cast<float>(delayWritePos) - timeSamples;
                const float delayed = readInterp(d, delayBufferSize, readPos);

                auto& pbuf = delayPitchBuffers[static_cast<size_t>(juce::jmin(ch, 1))];
                pbuf[static_cast<size_t>(delayPitchWritePos)] = delayed;
                delayPitchPhase += pitchStep;
                while (delayPitchPhase < 0.0f) delayPitchPhase += 1.0f;
                while (delayPitchPhase >= 1.0f) delayPitchPhase -= 1.0f;
                const float dA = delayPitchPhase * grain;
                const float dB = std::fmod(dA + 0.5f * grain, grain);
                const float wA = 0.5f - 0.5f * std::cos(juce::MathConstants<float>::twoPi * delayPitchPhase);
                const float pitched = wA * readInterp(pbuf, delayPitchBufferSize, static_cast<float>(delayPitchWritePos) - dA)
                                    + (1.0f - wA) * readInterp(pbuf, delayPitchBufferSize, static_cast<float>(delayPitchWritePos) - dB);
                delayToneState[static_cast<size_t>(juce::jmin(ch, 1))] += alpha * (pitched - delayToneState[static_cast<size_t>(juce::jmin(ch, 1))]);
                const float fbSample = delayToneState[static_cast<size_t>(juce::jmin(ch, 1))];

                const float in = buffer.getSample(ch, i);
                d[static_cast<size_t>(delayWritePos)] = in + feedback * fbSample;
                buffer.setSample(ch, i, (1.0f - mix) * in + mix * delayed);
            }

            delayWritePos = (delayWritePos + 1) % delayBufferSize;
            delayPitchWritePos = (delayPitchWritePos + 1) % delayPitchBufferSize;
        }
    }

    reverbIR.setEnabled(reverbEnabled);
    reverbIR.setMix(reverbMix);
    reverbIR.process(buffer);

    if (output1176Enabled && output1176Mix > 0.0001f)
    {
        juce::AudioBuffer<float> dry;
        dry.makeCopyOf(buffer, true);
        const float inputDrive = juce::jlimit(0.0f, 1.0f, output1176Input);
        const float rel = juce::jlimit(0.0f, 1.0f, output1176Release);
        if (outputCompMode == 1)
        {
            // Opto style: slower attack, smoother release, gentler ratio.
            output1176Compressor.setAttack(juce::jmap(inputDrive, 7.0f, 38.0f));
            output1176Compressor.setRelease(juce::jmap(rel, 120.0f, 900.0f));
            output1176Compressor.setThreshold(juce::jmap(inputDrive, -10.0f, -28.0f));
            output1176Compressor.setRatio(2.8f);
        }
        else
        {
            // 1176 style: very fast attack/release and harder grab.
            output1176Compressor.setAttack(juce::jmap(inputDrive, 0.04f, 0.8f));
            output1176Compressor.setRelease(juce::jmap(rel, 20.0f, 220.0f));
            output1176Compressor.setThreshold(juce::jmap(inputDrive, -8.0f, -35.0f));
            output1176Compressor.setRatio(4.0f);
        }
        juce::dsp::AudioBlock<float> compBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> compContext(compBlock);
        output1176Compressor.process(compContext);

        const float makeup = juce::Decibels::decibelsToGain(juce::jmap(inputDrive, 0.0f, 12.0f));
        const float mix = juce::jlimit(0.0f, 1.0f, output1176Mix);
        const float dryGain = 1.0f - mix;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* w = buffer.getWritePointer(ch);
            const auto* d = dry.getReadPointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
                w[i] = dryGain * d[i] + mix * w[i] * makeup;
        }
    }

    if (outputEqEnabled)
    {
        for (int band = 0; band < static_cast<int>(outputEqBands.size()); ++band)
        {
            const float gainDb = apvts.getRawParameterValue(outputEqGainIds[static_cast<size_t>(band)])->load();
            const float qValue = juce::jlimit(0.2f, 4.0f, apvts.getRawParameterValue(outputEqQIds[static_cast<size_t>(band)])->load());
            const auto coeff = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                                    outputEqFrequencies[static_cast<size_t>(band)],
                                                                                    qValue,
                                                                                    juce::Decibels::decibelsToGain(gainDb));
            for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
            {
                outputEqBands[static_cast<size_t>(band)][static_cast<size_t>(ch)].coefficients = coeff;
                juce::dsp::AudioBlock<float> block(buffer);
                auto channelBlock = block.getSingleChannelBlock(static_cast<size_t>(ch));
                juce::dsp::ProcessContextReplacing<float> context(channelBlock);
                outputEqBands[static_cast<size_t>(band)][static_cast<size_t>(ch)].process(context);
            }
        }

        if (outputEqHpEnabled)
        {
            const auto hpCoeff = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), juce::jlimit(20.0f, 1200.0f, outputEqHpCutoff), 0.707f);
            for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
            {
                outputEqHighPass[static_cast<size_t>(ch)].coefficients = hpCoeff;
                juce::dsp::AudioBlock<float> block(buffer);
                auto channelBlock = block.getSingleChannelBlock(static_cast<size_t>(ch));
                juce::dsp::ProcessContextReplacing<float> context(channelBlock);
                outputEqHighPass[static_cast<size_t>(ch)].process(context);
            }
        }

        if (outputEqLpEnabled)
        {
            const auto lpCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), juce::jlimit(1500.0f, 20000.0f, outputEqLpCutoff), 0.707f);
            for (int ch = 0; ch < juce::jmin(2, buffer.getNumChannels()); ++ch)
            {
                outputEqLowPass[static_cast<size_t>(ch)].coefficients = lpCoeff;
                juce::dsp::AudioBlock<float> block(buffer);
                auto channelBlock = block.getSingleChannelBlock(static_cast<size_t>(ch));
                juce::dsp::ProcessContextReplacing<float> context(channelBlock);
                outputEqLowPass[static_cast<size_t>(ch)].process(context);
            }
        }
    }

    buffer.applyGain(juce::Decibels::decibelsToGain(outputDb));
    if (buffer.getNumSamples() > 0)
    {
        const int numBins = static_cast<int>(outputSpectrumFrequencies.size());
        const int n = juce::jmin(buffer.getNumSamples(), 1024);
        const int nCh = juce::jmax(1, juce::jmin(2, buffer.getNumChannels()));
        for (int b = 0; b < numBins; ++b)
        {
            const float freq = outputSpectrumFrequencies[static_cast<size_t>(b)];
            const float omega = juce::MathConstants<float>::twoPi * freq / static_cast<float>(juce::jmax(1.0, getSampleRate()));
            const float coeff = 2.0f * std::cos(omega);
            float s1 = 0.0f;
            float s2 = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                float x = 0.0f;
                for (int ch = 0; ch < nCh; ++ch)
                    x += buffer.getSample(ch, i);
                x *= (1.0f / static_cast<float>(nCh));
                const float s0 = x + coeff * s1 - s2;
                s2 = s1;
                s1 = s0;
            }

            const float power = juce::jmax(0.0f, s1 * s1 + s2 * s2 - coeff * s1 * s2);
            const float mag = std::sqrt(power) / static_cast<float>(juce::jmax(1, n));
            const float db = juce::Decibels::gainToDecibels(mag + 1.0e-7f, -96.0f);
            const float norm = juce::jlimit(0.0f, 1.0f, (db + 96.0f) / 96.0f);
            const float current = outputSpectrumBins[static_cast<size_t>(b)].load(std::memory_order_relaxed);
            const float coeffSmooth = norm > current ? 0.28f : 0.08f;
            const float updated = current + coeffSmooth * (norm - current);
            outputSpectrumBins[static_cast<size_t>(b)].store(updated, std::memory_order_relaxed);
        }
    }
    updateMeterLevel(outputMeterLevel, computePeakLevel(buffer, totalOut));

    if (tunerEnabled)
        tuner.process(buffer);
}

juce::AudioProcessorEditor* BackhouseAmpSimAudioProcessor::createEditor()
{
    return new BackhouseAmpSimAudioProcessorEditor(*this);
}

bool BackhouseAmpSimAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String BackhouseAmpSimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BackhouseAmpSimAudioProcessor::acceptsMidi() const
{
    return true;
}

bool BackhouseAmpSimAudioProcessor::producesMidi() const
{
    return false;
}

bool BackhouseAmpSimAudioProcessor::isMidiEffect() const
{
    return false;
}

double BackhouseAmpSimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BackhouseAmpSimAudioProcessor::getNumPrograms()
{
    return 1;
}

int BackhouseAmpSimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BackhouseAmpSimAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String BackhouseAmpSimAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void BackhouseAmpSimAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void BackhouseAmpSimAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }
}

void BackhouseAmpSimAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr)
    {
        const auto tree = juce::ValueTree::fromXml(*xml);
        if (tree.isValid())
            apvts.replaceState(tree);
    }

    ensureProfileNameProperties();
    tryRestoreUserIRFromState();
    tryRestoreReverbIRFromState();
}

juce::AudioProcessorValueTreeState::ParameterLayout BackhouseAmpSimAudioProcessor::createParameterLayout()
{
    using Parameter = std::unique_ptr<juce::RangedAudioParameter>;
    std::vector<Parameter> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::ampType, "Amp", juce::StringArray { "Amp 1", "Amp 2", "Amp 3", "Amp 4" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::guitarProfile, "Guitar Profile", juce::StringArray { "Neutral", "Profile 1", "Profile 2", "Profile 3" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::inputGain, "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::stompInputGain, "Stomp Input Gain", juce::NormalisableRange<float>(0.0f, 36.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp1PreampGain, "Amp 1 Preamp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.42f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp2PreampGain, "Amp 2 Preamp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.52f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp3PreampGain, "Amp 3 Preamp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.58f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp4PreampGain, "Amp 4 Preamp", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.63f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::inputBoost, "Input Boost", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::sub, "Sub", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::low, "Low", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::mid, "Mid", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::high, "High", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::presence, "Presence", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::output, "Output", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), -6.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::gateThresh, "Gate Threshold", juce::NormalisableRange<float>(-80.0f, -20.0f, 0.1f), -55.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::cabEnabled, "Cab Enabled", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::cabTone, "Cab Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::cabSpeaker, "Cab Speaker",
                                                                  juce::StringArray { "Greenback 25", "Vintage 30", "Creamback H75", "G12T-75", "Jensen C12N" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::cabMic, "Cab Mic",
                                                                  juce::StringArray { "SM57", "U87", "U47 FET", "Royer 122", "MD421" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::cabMicAOffAxis, "Cab Mic A Off Axis", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::cabSpeakerB, "Cab Speaker B",
                                                                  juce::StringArray { "Greenback 25", "Vintage 30", "Creamback H75", "G12T-75", "Jensen C12N" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::cabMicB, "Cab Mic B",
                                                                  juce::StringArray { "SM57", "U87", "U47 FET", "Royer 122", "MD421" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::cabMicBOffAxis, "Cab Mic B Off Axis", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::cabMicBlend, "Cab Mic Blend", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::reverbEnabled, "Reverb Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::reverbMix, "Reverb Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.18f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::tsEnabled, "Tube Screamer Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::tsDrive, "Tube Screamer Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::tsTone, "Tube Screamer Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.55f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::tsLevel, "Tube Screamer Level", juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::delayEnabled, "Delay Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::delayTimeMs, "Delay Time", juce::NormalisableRange<float>(40.0f, 1200.0f, 1.0f), 380.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::delayFeedback, "Delay Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.001f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::delayMix, "Delay Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.24f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::delayTone, "Delay Tone", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.55f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::delayPitch, "Delay Pitch", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.01f), 0.0f));
    for (const auto& module : rigModuleDescriptors)
        params.push_back(std::make_unique<juce::AudioParameterBool>(module.paramId, module.label, module.defaultEnabled));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::rigMaxPoints, "Rig Max Points", juce::NormalisableRange<float>(4.0f, 20.0f, 1.0f), static_cast<float>(defaultRigMaxPoints)));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::wowEnabled, "WOW Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::wowMomentary, "WOW Momentary", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::wowMidiEnable, "WOW MIDI Enable", true));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::wowMode, "WOW Mode", juce::StringArray { "FATSO", "BLADE 1", "BLADE 2" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::wowPosition, "WOW Position", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::wowMix, "WOW Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::stompCompEnabled, "Stomp Comp Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::stompCompSustain, "Stomp Comp Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.50f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::stompCompLevel, "Stomp Comp Level", juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::stompCompMix, "Stomp Comp Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.65f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::phaserEnabled, "Phaser Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::phaserRate, "Phaser Rate", juce::NormalisableRange<float>(0.05f, 8.0f, 0.001f), 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::phaserDepth, "Phaser Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.85f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::phaserCenterHz, "Phaser Center", juce::NormalisableRange<float>(120.0f, 4000.0f, 1.0f, 0.35f), 950.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::phaserFeedback, "Phaser Feedback", juce::NormalisableRange<float>(-0.95f, 0.95f, 0.001f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::phaserMix, "Phaser Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::output1176Enabled, "Output 1176 Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::outputCompMode, "Output Comp Mode", juce::StringArray { "1176", "Opto" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::output1176Input, "Output 1176 Input", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::output1176Release, "Output 1176 Release", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.55f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::output1176Mix, "Output 1176 Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.70f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::outputEqEnabled, "Output EQ Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq63, "Output EQ 63Hz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq160, "Output EQ 160Hz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq400, "Output EQ 400Hz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq1000, "Output EQ 1kHz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq2500, "Output EQ 2.5kHz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq6300, "Output EQ 6.3kHz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEq12000, "Output EQ 12kHz", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ63, "Output EQ Q 63Hz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ160, "Output EQ Q 160Hz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ400, "Output EQ Q 400Hz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ1000, "Output EQ Q 1kHz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ2500, "Output EQ Q 2.5kHz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ6300, "Output EQ Q 6.3kHz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqQ12000, "Output EQ Q 12kHz", juce::NormalisableRange<float>(0.2f, 4.0f, 0.01f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::outputEqHpEnabled, "Output EQ HP Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqHpHz, "Output EQ HP Hz", juce::NormalisableRange<float>(20.0f, 1200.0f, 1.0f, 0.35f), 80.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::outputEqLpEnabled, "Output EQ LP Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::outputEqLpHz, "Output EQ LP Hz", juce::NormalisableRange<float>(1500.0f, 20000.0f, 1.0f, 0.35f), 14000.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::amp1TremEnabled, "Amp1 Trem Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp1TremRate, "Amp1 Trem Rate", juce::NormalisableRange<float>(0.20f, 12.0f, 0.001f), 3.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp1TremDepth, "Amp1 Trem Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.45f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::amp2Hiwatt, "Amp 2 Hiwatt", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::amp4Tight, "Amp 4 Tight", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::amp4LowOctave, "Amp 4 Low Octave", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp4LowOctaveMix, "Amp 4 Octave Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp4DirectMix, "Amp 4 Direct Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.18f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::amp4CleanAmpMix, "Amp 4 Clean Amp Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.22f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::tunerEnabled, "Tuner", false));
    params.push_back(std::make_unique<juce::AudioParameterInt>(ids::tunerTranspose, "Tuner Transpose", -12, 12, 0));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::whammyEnabled, "Whammy Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::whammyMode, "Whammy Mode",
        juce::StringArray { "+1 Oct", "+2 Oct", "-1 Oct", "-2 Oct", "+5th", "+4th" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::whammyPosition, "Whammy Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::whammyMix, "Whammy Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::wahEnabled, "Wah Enabled", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::wahPosition, "Wah Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::wahMix, "Wah Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile1PickupOutput, "P1 Pickup Output", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -2.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile1Brightness, "P1 Brightness", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile1LowEnd, "P1 Low End", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile1GateTrim, "P1 Gate Trim", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 3.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile2PickupOutput, "P2 Pickup Output", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 2.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile2Brightness, "P2 Brightness", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -1.8f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile2LowEnd, "P2 Low End", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile2GateTrim, "P2 Gate Trim", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -2.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile3PickupOutput, "P3 Pickup Output", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile3Brightness, "P3 Brightness", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.2f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile3LowEnd, "P3 Low End", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), -1.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::profile3GateTrim, "P3 Gate Trim", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 4.0f));

    return { params.begin(), params.end() };
}

juce::String BackhouseAmpSimAudioProcessor::getProfileName(int profileIndex) const
{
    if (profileIndex < 1 || profileIndex > numUserProfiles)
        return "Neutral";

    const auto propertyId = profileNamePropertyIds[static_cast<size_t>(profileIndex - 1)];
    return apvts.state.getProperty(propertyId, profileDefaultNames[static_cast<size_t>(profileIndex - 1)]).toString();
}

void BackhouseAmpSimAudioProcessor::setProfileName(int profileIndex, const juce::String& newName)
{
    if (profileIndex < 1 || profileIndex > numUserProfiles)
        return;

    const auto trimmed = newName.trim();
    const juce::String safeName = trimmed.isNotEmpty() ? trimmed : profileDefaultNames[static_cast<size_t>(profileIndex - 1)];
    const auto propertyId = profileNamePropertyIds[static_cast<size_t>(profileIndex - 1)];
    apvts.state.setProperty(propertyId, safeName, nullptr);
}

bool BackhouseAmpSimAudioProcessor::loadUserIR(const juce::File& file)
{
    const bool ok = cabSimulator.loadUserIR(file);
    if (ok)
        apvts.state.setProperty(ids::userIRPath, file.getFullPathName(), nullptr);
    return ok;
}

void BackhouseAmpSimAudioProcessor::clearUserIR()
{
    cabSimulator.clearUserIR();
    apvts.state.removeProperty(ids::userIRPath, nullptr);
}

bool BackhouseAmpSimAudioProcessor::hasUserIR() const
{
    return cabSimulator.hasUserIR();
}

juce::String BackhouseAmpSimAudioProcessor::getUserIRName() const
{
    return cabSimulator.getUserIRName();
}

bool BackhouseAmpSimAudioProcessor::loadReverbIR(const juce::File& file)
{
    const bool ok = reverbIR.loadIR(file);
    if (ok)
        apvts.state.setProperty(ids::reverbIRPath, file.getFullPathName(), nullptr);
    return ok;
}

void BackhouseAmpSimAudioProcessor::clearReverbIR()
{
    reverbIR.clearIR();
    apvts.state.removeProperty(ids::reverbIRPath, nullptr);
}

bool BackhouseAmpSimAudioProcessor::hasReverbIR() const
{
    return reverbIR.hasIR();
}

juce::String BackhouseAmpSimAudioProcessor::getReverbIRName() const
{
    return reverbIR.getIRName();
}

std::array<float, 16> BackhouseAmpSimAudioProcessor::getOutputSpectrumBins() const
{
    std::array<float, 16> bins {};
    for (size_t i = 0; i < bins.size(); ++i)
        bins[i] = outputSpectrumBins[i].load(std::memory_order_relaxed);
    return bins;
}

int BackhouseAmpSimAudioProcessor::getRigPointTotal() const
{
    int total = 0;
    for (const auto& module : rigModuleDescriptors)
        if (apvts.getRawParameterValue(module.paramId)->load() > 0.5f)
            total += module.points;
    return total;
}

int BackhouseAmpSimAudioProcessor::getRigBudgetMax() const
{
    const float value = apvts.getRawParameterValue(ids::rigMaxPoints)->load();
    return static_cast<int>(std::round(value));
}

bool BackhouseAmpSimAudioProcessor::loadTestDIFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;

    // Cap at 10 minutes at the file's own sample rate to prevent bad_alloc on huge files.
    const double maxSecs = 60.0 * 10.0;
    const auto srcRate = reader->sampleRate > 1.0 ? reader->sampleRate : 44100.0;
    const auto maxSamples = static_cast<int64_t>(maxSecs * srcRate);
    const auto numSamplesToLoad = static_cast<int>(juce::jmin(reader->lengthInSamples, maxSamples));
    const auto numChannels = juce::jlimit(1, 2, static_cast<int>(reader->numChannels));

    juce::AudioBuffer<float> loaded(numChannels, numSamplesToLoad);
    if (!reader->read(&loaded, 0, numSamplesToLoad, 0, true, true))
        return false;

    const juce::ScopedLock lock(testDILock);
    testDIAudio = std::move(loaded);
    testDIFileName = file.getFileName();
    testDISourceSampleRate = reader->sampleRate > 1.0 ? reader->sampleRate : 44100.0;

    testDIPlayheadSamples.store(0.0, std::memory_order_relaxed);
    testDILoopStartSamples.store(0.0, std::memory_order_relaxed);
    testDILoopEndSamples.store(static_cast<double>(testDIAudio.getNumSamples() - 1), std::memory_order_relaxed);
    testDIPlaying.store(false, std::memory_order_relaxed);
    return true;
}

void BackhouseAmpSimAudioProcessor::clearTestDIFile()
{
    const juce::ScopedLock lock(testDILock);
    testDIAudio.setSize(0, 0);
    testDIFileName.clear();
    testDIPlayheadSamples.store(0.0, std::memory_order_relaxed);
    testDILoopStartSamples.store(0.0, std::memory_order_relaxed);
    testDILoopEndSamples.store(0.0, std::memory_order_relaxed);
    testDIEnabled.store(false, std::memory_order_relaxed);
    testDIPlaying.store(false, std::memory_order_relaxed);
}

void BackhouseAmpSimAudioProcessor::setTestDIEnabled(bool enabled)
{
    testDIEnabled.store(enabled, std::memory_order_relaxed);

    if (!enabled)
        testDIPlaying.store(false, std::memory_order_relaxed);
}

void BackhouseAmpSimAudioProcessor::setTestDIPlaying(bool playing)
{
    if (!playing)
    {
        testDIPlaying.store(false, std::memory_order_relaxed);
        return;
    }

    if (!testDIEnabled.load(std::memory_order_relaxed))
        return;

    const juce::ScopedLock lock(testDILock);
    if (testDIAudio.getNumSamples() <= 1 || testDISourceSampleRate <= 0.0)
        return;

    testDIPlaying.store(true, std::memory_order_relaxed);
}

bool BackhouseAmpSimAudioProcessor::hasTestDIFile() const
{
    const juce::ScopedLock lock(testDILock);
    return testDIAudio.getNumSamples() > 0;
}

juce::String BackhouseAmpSimAudioProcessor::getTestDIFileName() const
{
    const juce::ScopedLock lock(testDILock);
    return testDIFileName;
}

double BackhouseAmpSimAudioProcessor::getTestDIDurationSeconds() const
{
    const juce::ScopedLock lock(testDILock);
    if (testDIAudio.getNumSamples() <= 0 || testDISourceSampleRate <= 0.0)
        return 0.0;

    return static_cast<double>(testDIAudio.getNumSamples()) / testDISourceSampleRate;
}

void BackhouseAmpSimAudioProcessor::setTestDIPositionSeconds(double seconds)
{
    const juce::ScopedLock lock(testDILock);
    if (testDIAudio.getNumSamples() <= 0)
        return;

    const double maxSamples = static_cast<double>(juce::jmax(0, testDIAudio.getNumSamples() - 1));
    const double targetSamples = juce::jlimit(0.0, maxSamples, seconds * testDISourceSampleRate);
    testDIPlayheadSamples.store(targetSamples, std::memory_order_relaxed);
}

double BackhouseAmpSimAudioProcessor::getTestDIPositionSeconds() const
{
    const juce::ScopedLock lock(testDILock);
    if (testDISourceSampleRate <= 0.0)
        return 0.0;

    return testDIPlayheadSamples.load(std::memory_order_relaxed) / testDISourceSampleRate;
}

void BackhouseAmpSimAudioProcessor::setTestDILoopRangeSeconds(double startSeconds, double endSeconds)
{
    const juce::ScopedLock lock(testDILock);
    if (testDIAudio.getNumSamples() <= 1)
        return;

    const double maxSamples = static_cast<double>(juce::jmax(1, testDIAudio.getNumSamples() - 1));
    double start = juce::jlimit(0.0, maxSamples, startSeconds * testDISourceSampleRate);
    double end = juce::jlimit(0.0, maxSamples, endSeconds * testDISourceSampleRate);

    if (end <= start + 1.0)
        end = juce::jmin(maxSamples, start + 1.0);

    testDILoopStartSamples.store(start, std::memory_order_relaxed);
    testDILoopEndSamples.store(end, std::memory_order_relaxed);
}

double BackhouseAmpSimAudioProcessor::getTestDILoopStartSeconds() const
{
    const juce::ScopedLock lock(testDILock);
    if (testDISourceSampleRate <= 0.0)
        return 0.0;

    return testDILoopStartSamples.load(std::memory_order_relaxed) / testDISourceSampleRate;
}

double BackhouseAmpSimAudioProcessor::getTestDILoopEndSeconds() const
{
    const juce::ScopedLock lock(testDILock);
    if (testDISourceSampleRate <= 0.0)
        return 0.0;

    return testDILoopEndSamples.load(std::memory_order_relaxed) / testDISourceSampleRate;
}

bool BackhouseAmpSimAudioProcessor::trimTestDIToRangeSeconds(double startSeconds, double endSeconds)
{
    const juce::ScopedLock lock(testDILock);
    const int sourceSamples = testDIAudio.getNumSamples();
    const int sourceChannels = testDIAudio.getNumChannels();
    if (sourceSamples <= 1 || sourceChannels <= 0 || testDISourceSampleRate <= 0.0)
        return false;

    const int maxIndex = sourceSamples - 1;
    int startSample = juce::jlimit(0, maxIndex, static_cast<int>(std::floor(startSeconds * testDISourceSampleRate)));
    int endSample = juce::jlimit(0, maxIndex, static_cast<int>(std::ceil(endSeconds * testDISourceSampleRate)));
    if (endSample <= startSample)
        std::swap(startSample, endSample);

    const int length = juce::jmax(2, endSample - startSample + 1);
    juce::AudioBuffer<float> trimmed(sourceChannels, length);
    for (int ch = 0; ch < sourceChannels; ++ch)
        trimmed.copyFrom(ch, 0, testDIAudio, ch, startSample, length);

    testDIAudio = std::move(trimmed);
    testDIPlayheadSamples.store(0.0, std::memory_order_relaxed);
    testDILoopStartSamples.store(0.0, std::memory_order_relaxed);
    testDILoopEndSamples.store(static_cast<double>(length - 1), std::memory_order_relaxed);
    return true;
}

bool BackhouseAmpSimAudioProcessor::cutTestDIRangeSeconds(double startSeconds, double endSeconds)
{
    const juce::ScopedLock lock(testDILock);
    const int sourceSamples = testDIAudio.getNumSamples();
    const int sourceChannels = testDIAudio.getNumChannels();
    if (sourceSamples <= 2 || sourceChannels <= 0 || testDISourceSampleRate <= 0.0)
        return false;

    const int maxIndex = sourceSamples - 1;
    int startSample = juce::jlimit(0, maxIndex, static_cast<int>(std::floor(startSeconds * testDISourceSampleRate)));
    int endSample = juce::jlimit(0, maxIndex, static_cast<int>(std::ceil(endSeconds * testDISourceSampleRate)));
    if (endSample <= startSample)
        std::swap(startSample, endSample);

    const int cutLength = juce::jmax(1, endSample - startSample + 1);
    const int keepLength = sourceSamples - cutLength;
    if (keepLength < 2)
        return false;

    juce::AudioBuffer<float> cut(sourceChannels, keepLength);
    for (int ch = 0; ch < sourceChannels; ++ch)
    {
        if (startSample > 0)
            cut.copyFrom(ch, 0, testDIAudio, ch, 0, startSample);
        const int rightCount = sourceSamples - (endSample + 1);
        if (rightCount > 0)
            cut.copyFrom(ch, startSample, testDIAudio, ch, endSample + 1, rightCount);
    }

    testDIAudio = std::move(cut);

    const double maxAfter = static_cast<double>(keepLength - 1);
    const double oldPlayhead = testDIPlayheadSamples.load(std::memory_order_relaxed);
    double newPlayhead = oldPlayhead;
    if (oldPlayhead > endSample)
        newPlayhead -= static_cast<double>(cutLength);
    else if (oldPlayhead >= startSample)
        newPlayhead = static_cast<double>(startSample);
    testDIPlayheadSamples.store(juce::jlimit(0.0, maxAfter, newPlayhead), std::memory_order_relaxed);

    testDILoopStartSamples.store(0.0, std::memory_order_relaxed);
    testDILoopEndSamples.store(maxAfter, std::memory_order_relaxed);
    return true;
}

std::vector<float> BackhouseAmpSimAudioProcessor::getTestDIWaveformPeaks(int numPoints) const
{
    std::vector<float> peaks;
    if (numPoints <= 0)
        return peaks;

    peaks.resize(static_cast<size_t>(numPoints), 0.0f);

    const juce::ScopedLock lock(testDILock);
    const int sourceSamples = testDIAudio.getNumSamples();
    const int sourceChannels = testDIAudio.getNumChannels();
    if (sourceSamples <= 0 || sourceChannels <= 0)
        return peaks;

    const double samplesPerPoint = static_cast<double>(sourceSamples) / static_cast<double>(numPoints);

    for (int point = 0; point < numPoints; ++point)
    {
        const int start = juce::jlimit(0, sourceSamples - 1, static_cast<int>(std::floor(point * samplesPerPoint)));
        const int end = juce::jlimit(start + 1, sourceSamples, static_cast<int>(std::ceil((point + 1) * samplesPerPoint)));

        float maxAbs = 0.0f;
        for (int ch = 0; ch < sourceChannels; ++ch)
        {
            for (int sample = start; sample < end; ++sample)
                maxAbs = juce::jmax(maxAbs, std::abs(testDIAudio.getSample(ch, sample)));
        }

        peaks[static_cast<size_t>(point)] = juce::jlimit(0.0f, 1.0f, maxAbs);
    }

    return peaks;
}

void BackhouseAmpSimAudioProcessor::renderTestDI(juce::AudioBuffer<float>& buffer)
{
    if (!testDIEnabled.load(std::memory_order_relaxed) || !testDIPlaying.load(std::memory_order_relaxed))
        return;

    const juce::ScopedLock lock(testDILock);
    const int sourceSamples = testDIAudio.getNumSamples();
    const int sourceChannels = testDIAudio.getNumChannels();
    if (sourceSamples <= 1 || sourceChannels <= 0 || testDISourceSampleRate <= 0.0 || getSampleRate() <= 0.0)
        return;

    const int targetChannels = juce::jmax(1, getTotalNumInputChannels());
    const int blockSamples = buffer.getNumSamples();

    double playhead = testDIPlayheadSamples.load(std::memory_order_relaxed);
    double loopStart = testDILoopStartSamples.load(std::memory_order_relaxed);
    double loopEnd = testDILoopEndSamples.load(std::memory_order_relaxed);

    const double sourceMax = static_cast<double>(sourceSamples - 1);
    loopStart = juce::jlimit(0.0, sourceMax, loopStart);
    loopEnd = juce::jlimit(0.0, sourceMax, loopEnd);
    if (loopEnd <= loopStart + 1.0)
        loopEnd = juce::jmin(sourceMax, loopStart + 1.0);

    const bool looping = testDILoopEnabled.load(std::memory_order_relaxed);
    const float gain = juce::Decibels::decibelsToGain(testDIGainDb.load(std::memory_order_relaxed));
    const double step = testDISourceSampleRate / getSampleRate();

    for (int i = 0; i < blockSamples; ++i)
    {
        if (playhead >= sourceMax)
        {
            if (looping)
                playhead = loopStart;
            else
            {
                testDIPlaying.store(false, std::memory_order_relaxed);
                for (int ch = 0; ch < targetChannels && ch < buffer.getNumChannels(); ++ch)
                    buffer.setSample(ch, i, 0.0f);
                continue;
            }
        }

        for (int ch = 0; ch < targetChannels && ch < buffer.getNumChannels(); ++ch)
        {
            const int sourceCh = juce::jmin(ch, sourceChannels - 1);
            buffer.setSample(ch, i, gain * readInterpolatedSample(testDIAudio, sourceCh, playhead));
        }

        playhead += step;

        if (looping)
        {
            while (playhead >= loopEnd)
                playhead = loopStart + (playhead - loopEnd);
        }
    }

    testDIPlayheadSamples.store(playhead, std::memory_order_relaxed);
}

bool BackhouseAmpSimAudioProcessor::exportProfilesToJson(const juce::File& file) const
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("schema", profileSchema);
    root->setProperty("version", profileSchemaVersion);

    juce::Array<juce::var> profiles;
    for (int profileIndex = 1; profileIndex <= numUserProfiles; ++profileIndex)
    {
        const size_t idx = static_cast<size_t>(profileIndex - 1);

        auto profile = std::make_unique<juce::DynamicObject>();
        profile->setProperty("index", profileIndex);
        profile->setProperty("name", getProfileName(profileIndex));
        profile->setProperty("pickupOutput", apvts.getRawParameterValue(pickupOutputIds[idx])->load());
        profile->setProperty("brightness", apvts.getRawParameterValue(brightnessIds[idx])->load());
        profile->setProperty("lowEnd", apvts.getRawParameterValue(lowEndIds[idx])->load());
        profile->setProperty("gateTrim", apvts.getRawParameterValue(gateTrimIds[idx])->load());

        profiles.add(juce::var(profile.release()));
    }

    root->setProperty("profiles", juce::var(profiles));
    const juce::String json = juce::JSON::toString(juce::var(root.release()), true);
    return file.replaceWithText(json);
}

bool BackhouseAmpSimAudioProcessor::importProfilesFromJson(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    const juce::String text = file.loadFileAsString();
    const juce::var parsed = juce::JSON::parse(text);
    auto* root = parsed.getDynamicObject();
    if (root == nullptr)
        return false;

    if (root->getProperty("schema").toString() != profileSchema)
        return false;

    const auto profilesVar = root->getProperty("profiles");
    if (!profilesVar.isArray())
        return false;

    auto* profileArray = profilesVar.getArray();
    if (profileArray == nullptr)
        return false;

    auto setFloatParam = [this](const juce::String& paramId, float value) {
        if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(paramId)))
            param->setValueNotifyingHost(param->convertTo0to1(value));
    };

    for (const auto& entry : *profileArray)
    {
        auto* profile = entry.getDynamicObject();
        if (profile == nullptr)
            continue;

        const int index = static_cast<int>(profile->getProperty("index"));
        if (index < 1 || index > numUserProfiles)
            continue;

        const size_t idx = static_cast<size_t>(index - 1);
        setProfileName(index, profile->getProperty("name").toString());

        if (profile->hasProperty("pickupOutput"))
            setFloatParam(pickupOutputIds[idx], static_cast<float>(profile->getProperty("pickupOutput")));
        if (profile->hasProperty("brightness"))
            setFloatParam(brightnessIds[idx], static_cast<float>(profile->getProperty("brightness")));
        if (profile->hasProperty("lowEnd"))
            setFloatParam(lowEndIds[idx], static_cast<float>(profile->getProperty("lowEnd")));
        if (profile->hasProperty("gateTrim"))
            setFloatParam(gateTrimIds[idx], static_cast<float>(profile->getProperty("gateTrim")));
    }

    return true;
}

int BackhouseAmpSimAudioProcessor::getMidiMapSlotCount() const
{
    return static_cast<int>(midiMapSlots.size());
}

BackhouseAmpSimAudioProcessor::MidiMapSlot BackhouseAmpSimAudioProcessor::getMidiMapSlot(int index) const
{
    const juce::ScopedLock lock(midiMapLock);
    const int safe = juce::jlimit(0, static_cast<int>(midiMapSlots.size()) - 1, index);
    return midiMapSlots[static_cast<size_t>(safe)];
}

void BackhouseAmpSimAudioProcessor::setMidiMapSlot(int index, const MidiMapSlot& slot)
{
    const juce::ScopedLock lock(midiMapLock);
    const int safe = juce::jlimit(0, static_cast<int>(midiMapSlots.size()) - 1, index);
    auto copy = slot;
    copy.ccNumber = juce::jlimit(0, 127, copy.ccNumber);
    copy.mode = juce::jlimit(0, 2, copy.mode);
    copy.minNorm = juce::jlimit(0.0f, 1.0f, copy.minNorm);
    copy.maxNorm = juce::jlimit(0.0f, 1.0f, copy.maxNorm);
    if (copy.maxNorm < copy.minNorm)
        std::swap(copy.maxNorm, copy.minNorm);
    midiMapSlots[static_cast<size_t>(safe)] = copy;
}

juce::StringArray BackhouseAmpSimAudioProcessor::getMappableParameterIds() const
{
    return mappableParameterIds;
}

juce::StringArray BackhouseAmpSimAudioProcessor::getMappableParameterNames() const
{
    return mappableParameterNames;
}

void BackhouseAmpSimAudioProcessor::applyMappedMidiController(int ccNumber, int ccValue)
{
    const float src01 = juce::jlimit(0.0f, 1.0f, static_cast<float>(ccValue) / 127.0f);
    std::array<MidiMapSlot, 8> slotsCopy;
    {
        const juce::ScopedLock lock(midiMapLock);
        slotsCopy = midiMapSlots;
    }

    for (const auto& slot : slotsCopy)
    {
        if (!slot.enabled || slot.ccNumber != ccNumber || slot.parameterId.isEmpty())
            continue;

        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(slot.parameterId));
        if (ranged == nullptr)
            continue;

        const float src = slot.invert ? (1.0f - src01) : src01;
        float target01 = ranged->getValue();

        if (slot.mode == 0) // Absolute
        {
            target01 = juce::jmap(src, slot.minNorm, slot.maxNorm);
        }
        else if (slot.mode == 1) // Toggle
        {
            if (ccValue >= 64)
            {
                const float mid = 0.5f * (slot.minNorm + slot.maxNorm);
                target01 = ranged->getValue() < mid ? slot.maxNorm : slot.minNorm;
            }
            else
            {
                continue;
            }
        }
        else // Momentary
        {
            target01 = (ccValue >= 64) ? slot.maxNorm : slot.minNorm;
        }

        ranged->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, target01));
    }
}

void BackhouseAmpSimAudioProcessor::ensureProfileNameProperties()
{
    for (int i = 0; i < numUserProfiles; ++i)
    {
        const auto propertyId = profileNamePropertyIds[static_cast<size_t>(i)];
        if (!apvts.state.hasProperty(propertyId))
            apvts.state.setProperty(propertyId, profileDefaultNames[static_cast<size_t>(i)], nullptr);
    }
}

void BackhouseAmpSimAudioProcessor::tryRestoreUserIRFromState()
{
    const auto path = apvts.state.getProperty(ids::userIRPath).toString();
    if (path.isNotEmpty())
    {
        const juce::File file(path);
        if (file.existsAsFile())
            cabSimulator.loadUserIR(file);
    }
}

void BackhouseAmpSimAudioProcessor::tryRestoreReverbIRFromState()
{
    const auto path = apvts.state.getProperty(ids::reverbIRPath).toString();
    if (path.isNotEmpty())
    {
        const juce::File file(path);
        if (file.existsAsFile())
            reverbIR.loadIR(file);
    }
}

BackhouseAmpSimAudioProcessor::GuitarProfileOffsets BackhouseAmpSimAudioProcessor::getProfileOffsetsFromParams(int profileIndex) const
{
    if (profileIndex < 1 || profileIndex > numUserProfiles)
        return {};

    const size_t idx = static_cast<size_t>(profileIndex - 1);

    const float pickupOutput = apvts.getRawParameterValue(pickupOutputIds[idx])->load();
    const float brightness = apvts.getRawParameterValue(brightnessIds[idx])->load();
    const float lowEnd = apvts.getRawParameterValue(lowEndIds[idx])->load();
    const float gateTrim = apvts.getRawParameterValue(gateTrimIds[idx])->load();

    GuitarProfileOffsets offsets;
    offsets.inputGainDb = pickupOutput;
    offsets.lowDb = lowEnd;
    offsets.midDb = 0.25f * brightness;
    offsets.highDb = brightness;
    offsets.presenceDb = 0.6f * brightness;
    offsets.gateThresholdDb = gateTrim;
    return offsets;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BackhouseAmpSimAudioProcessor();
}
