#pragma once

#include <array>
#include <atomic>

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

    juce::AudioProcessorValueTreeState apvts;

    backhouse::NoiseGate noiseGate;
    backhouse::AmpEngine ampEngine;
    backhouse::CabSimulator cabSimulator;
    backhouse::TunerStub tuner;

    double smoothingSampleRate = 44100.0;
    GuitarProfileOffsets smoothedOffsets;
    std::atomic<float> inputMeterLevel { 0.0f };
    std::atomic<float> outputMeterLevel { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BackhouseAmpSimAudioProcessor)
};
