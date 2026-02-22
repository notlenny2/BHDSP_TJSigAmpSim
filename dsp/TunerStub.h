#pragma once

#include <atomic>

#include <juce_audio_basics/juce_audio_basics.h>

namespace backhouse
{
class TunerStub
{
public:
    void prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        detectedPitchHz.store(0.0f);
    }

    void process(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumChannels() == 0 || buffer.getNumSamples() < 2)
            return;

        const auto* data = buffer.getReadPointer(0);
        int zeroCrossings = 0;

        for (int i = 1; i < buffer.getNumSamples(); ++i)
        {
            const bool prevNeg = data[i - 1] < 0.0f;
            const bool currNeg = data[i] < 0.0f;
            if (prevNeg != currNeg)
                ++zeroCrossings;
        }

        if (zeroCrossings > 1)
        {
            const auto duration = static_cast<double>(buffer.getNumSamples()) / sampleRate;
            const auto hz = static_cast<float>(0.5 * static_cast<double>(zeroCrossings) / duration);
            if (hz >= 20.0f && hz <= 1500.0f)
                detectedPitchHz.store(hz);
        }
    }

    float getDetectedPitchHz() const
    {
        return detectedPitchHz.load();
    }

private:
    double sampleRate = 44100.0;
    std::atomic<float> detectedPitchHz { 0.0f };
};
} // namespace backhouse
