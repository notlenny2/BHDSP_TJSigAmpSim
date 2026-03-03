#pragma once

#include <array>
#include <atomic>
#include <vector>

struct RigModuleDescriptor
{
    const char* paramId;
    const char* label;
    int points;
    bool defaultEnabled;
};

static constexpr std::array<RigModuleDescriptor, 9> rigModuleDescriptors {
    RigModuleDescriptor{ "rigAmp1", "Amp 1", 3, true },
    RigModuleDescriptor{ "rigAmp3", "Amp 3", 3, false },
    RigModuleDescriptor{ "rigCompressor", "Compressor", 2, true },
    RigModuleDescriptor{ "rigCab", "Cab Sim", 3, true },
    RigModuleDescriptor{ "rigDelay", "Delay", 2, false },
    RigModuleDescriptor{ "rigEQ", "EQ", 1, false },
    RigModuleDescriptor{ "rigPhaser", "Phaser", 1, false },
    RigModuleDescriptor{ "rigTrem", "Tremolo", 1, false },
    RigModuleDescriptor{ "rigWOW", "WOW Pedal", 1, false }
};

static constexpr int defaultRigMaxPoints = 10;

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "AmpEngine.h"
#include "CabSimulator.h"
#include "NoiseGate.h"
#include "ReverbIRProcessor.h"
#include "TunerStub.h"
#include "WhammyEffect.h"

class BackhouseAmpSimAudioProcessor final : public juce::AudioProcessor
{
public:
    struct MidiMapSlot
    {
        bool enabled = false;
        int ccNumber = 0;
        juce::String parameterId;
        int mode = 0; // 0=Absolute, 1=Toggle, 2=Momentary
        float minNorm = 0.0f;
        float maxNorm = 1.0f;
        bool invert = false;
    };

    BackhouseAmpSimAudioProcessor();
    ~BackhouseAmpSimAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getDetectedPitchHz() const { return tuner.getDetectedPitchHz(); }

    juce::String getProfileName(int profileIndex) const;
    void setProfileName(int profileIndex, const juce::String& newName);
    bool loadUserIR(const juce::File& file);
    void clearUserIR();
    bool hasUserIR() const;
    juce::String getUserIRName() const;
    bool loadReverbIR(const juce::File& file);
    void clearReverbIR();
    bool hasReverbIR() const;
    juce::String getReverbIRName() const;
    bool exportProfilesToJson(const juce::File& file) const;
    bool importProfilesFromJson(const juce::File& file);
    float getInputMeterLevel() const noexcept { return inputMeterLevel.load(std::memory_order_relaxed); }
    float getOutputMeterLevel() const noexcept { return outputMeterLevel.load(std::memory_order_relaxed); }
    std::array<float, 16> getOutputSpectrumBins() const;
    int getRigPointTotal() const;
    int getRigBudgetMax() const;

    bool loadTestDIFile(const juce::File& file);
    void clearTestDIFile();
    bool hasTestDIFile() const;
    juce::String getTestDIFileName() const;
    double getTestDIDurationSeconds() const;

    void setTestDIEnabled(bool enabled);
    bool isTestDIEnabled() const { return testDIEnabled.load(std::memory_order_relaxed); }
    void setTestDIPlaying(bool playing);
    bool isTestDIPlaying() const { return testDIPlaying.load(std::memory_order_relaxed); }
    void setTestDILoopEnabled(bool enabled) { testDILoopEnabled.store(enabled, std::memory_order_relaxed); }
    bool isTestDILoopEnabled() const { return testDILoopEnabled.load(std::memory_order_relaxed); }
    void setTestDIGainDb(float db) { testDIGainDb.store(juce::jlimit(-60.0f, 36.0f, db), std::memory_order_relaxed); }
    float getTestDIGainDb() const { return testDIGainDb.load(std::memory_order_relaxed); }

    void setTestDIPositionSeconds(double seconds);
    double getTestDIPositionSeconds() const;
    void setTestDILoopRangeSeconds(double startSeconds, double endSeconds);
    double getTestDILoopStartSeconds() const;
    double getTestDILoopEndSeconds() const;
    bool trimTestDIToRangeSeconds(double startSeconds, double endSeconds);
    bool cutTestDIRangeSeconds(double startSeconds, double endSeconds);
    std::vector<float> getTestDIWaveformPeaks(int numPoints) const;
    int getMidiMapSlotCount() const;
    MidiMapSlot getMidiMapSlot(int index) const;
    void setMidiMapSlot(int index, const MidiMapSlot& slot);
    juce::StringArray getMappableParameterIds() const;
    juce::StringArray getMappableParameterNames() const;

private:
    struct GuitarProfileOffsets
    {
        float inputGainDb = 0.0f;
        float lowDb = 0.0f;
        float midDb = 0.0f;
        float highDb = 0.0f;
        float presenceDb = 0.0f;
        float gateThresholdDb = 0.0f;
    };

    static constexpr int numUserProfiles = 3;

    void ensureProfileNameProperties();
    void tryRestoreUserIRFromState();
    void tryRestoreReverbIRFromState();
    GuitarProfileOffsets getProfileOffsetsFromParams(int profileIndex) const;
    void renderTestDI(juce::AudioBuffer<float>& buffer);
    void applyMappedMidiController(int ccNumber, int ccValue);

    juce::AudioProcessorValueTreeState apvts;

    backhouse::NoiseGate noiseGate;
    backhouse::AmpEngine ampEngine;
    backhouse::CabSimulator cabSimulator;
    backhouse::ReverbIRProcessor reverbIR;
    juce::dsp::Phaser<float> phaser;
    juce::dsp::Compressor<float> stompCompressor;
    juce::dsp::Compressor<float> output1176Compressor;
    std::array<std::array<juce::dsp::IIR::Filter<float>, 2>, 7> outputEqBands;
    std::array<juce::dsp::IIR::Filter<float>, 2> outputEqHighPass;
    std::array<juce::dsp::IIR::Filter<float>, 2> outputEqLowPass;
    std::array<juce::dsp::IIR::Filter<float>, 2> tsInputHP;
    std::array<juce::dsp::IIR::Filter<float>, 2> tsToneLP;
    std::array<std::vector<float>, 2> delayBuffers;
    int delayBufferSize = 0;
    int delayWritePos = 0;
    std::array<float, 2> delayToneState { 0.0f, 0.0f };
    std::array<std::vector<float>, 2> delayPitchBuffers;
    int delayPitchBufferSize = 0;
    int delayPitchWritePos = 0;
    float delayPitchPhase = 0.0f;
    std::array<std::vector<float>, 2> wowDelayBuffers;
    int wowDelaySize = 0;
    int wowWritePos = 0;
    float wowShiftPhase = 0.0f;
    float tremPhase = 0.0f;
    float wowMidiLastPosition = 0.45f;
    float wowMidiLastMix = 0.65f;
    bool wowMidiStompDown = false;
    backhouse::TunerStub tuner;
    backhouse::WhammyEffect whammyEffect;

    // Crybaby wah — Chamberlin state-variable filter state (stereo)
    float wahBandpass[2] = { 0.0f, 0.0f };
    float wahLowpass[2]  = { 0.0f, 0.0f };

    double smoothingSampleRate = 44100.0;
    GuitarProfileOffsets smoothedOffsets;
    std::atomic<float> inputMeterLevel { 0.0f };
    std::atomic<float> outputMeterLevel { 0.0f };
    std::array<std::atomic<float>, 16> outputSpectrumBins {};

    mutable juce::CriticalSection testDILock;
    juce::AudioBuffer<float> testDIAudio;
    juce::String testDIFileName;
    double testDISourceSampleRate = 44100.0;
    std::atomic<double> testDIPlayheadSamples { 0.0 };
    std::atomic<double> testDILoopStartSamples { 0.0 };
    std::atomic<double> testDILoopEndSamples { 0.0 };
    std::atomic<bool> testDIEnabled { false };
    std::atomic<bool> testDIPlaying { false };
    std::atomic<bool> testDILoopEnabled { true };
    std::atomic<float> testDIGainDb { 0.0f };

    mutable juce::CriticalSection midiMapLock;
    std::array<MidiMapSlot, 8> midiMapSlots;
    juce::StringArray mappableParameterIds;
    juce::StringArray mappableParameterNames;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackhouseAmpSimAudioProcessor)
};
