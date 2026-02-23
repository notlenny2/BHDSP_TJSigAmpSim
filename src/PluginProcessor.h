#pragma once

#include <array>
#include <atomic>
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "AmpEngine.h"
#include "CabSimulator.h"
#include "NoiseGate.h"
#include "TunerStub.h"

class BackhouseAmpSimAudioProcessor final : public juce::AudioProcessor
{
public:
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
    bool exportProfilesToJson(const juce::File& file) const;
    bool importProfilesFromJson(const juce::File& file);
    float getInputMeterLevel() const noexcept { return inputMeterLevel.load(std::memory_order_relaxed); }
    float getOutputMeterLevel() const noexcept { return outputMeterLevel.load(std::memory_order_relaxed); }

    bool loadTestDIFile(const juce::File& file);
    void clearTestDIFile();
    bool hasTestDIFile() const;
    juce::String getTestDIFileName() const;
    double getTestDIDurationSeconds() const;

    void setTestDIEnabled(bool enabled) { testDIEnabled.store(enabled, std::memory_order_relaxed); }
    bool isTestDIEnabled() const { return testDIEnabled.load(std::memory_order_relaxed); }
    void setTestDIPlaying(bool playing) { testDIPlaying.store(playing, std::memory_order_relaxed); }
    bool isTestDIPlaying() const { return testDIPlaying.load(std::memory_order_relaxed); }
    void setTestDILoopEnabled(bool enabled) { testDILoopEnabled.store(enabled, std::memory_order_relaxed); }
    bool isTestDILoopEnabled() const { return testDILoopEnabled.load(std::memory_order_relaxed); }

    void setTestDIPositionSeconds(double seconds);
    double getTestDIPositionSeconds() const;
    void setTestDILoopRangeSeconds(double startSeconds, double endSeconds);
    double getTestDILoopStartSeconds() const;
    double getTestDILoopEndSeconds() const;
    std::vector<float> getTestDIWaveformPeaks(int numPoints) const;

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
    GuitarProfileOffsets getProfileOffsetsFromParams(int profileIndex) const;
    void renderTestDI(juce::AudioBuffer<float>& buffer);

    juce::AudioProcessorValueTreeState apvts;

    backhouse::NoiseGate noiseGate;
    backhouse::AmpEngine ampEngine;
    backhouse::CabSimulator cabSimulator;
    backhouse::TunerStub tuner;

    double smoothingSampleRate = 44100.0;
    GuitarProfileOffsets smoothedOffsets;
    std::atomic<float> inputMeterLevel { 0.0f };
    std::atomic<float> outputMeterLevel { 0.0f };

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackhouseAmpSimAudioProcessor)
};
