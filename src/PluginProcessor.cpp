#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <array>
#include <cmath>

namespace ids
{
static constexpr auto ampType = "ampType";
static constexpr auto guitarProfile = "guitarProfile";

static constexpr auto inputGain = "inputGain";
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
static constexpr auto tunerEnabled = "tunerEnabled";

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
} // namespace

BackhouseAmpSimAudioProcessor::BackhouseAmpSimAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "BackhouseAmpSimParameters", createParameterLayout())
{
    ensureProfileNameProperties();
}

void BackhouseAmpSimAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(getTotalNumOutputChannels()) };
    noiseGate.prepare(spec);
    ampEngine.prepare(spec);
    cabSimulator.prepare(spec);
    tuner.prepare(sampleRate);
    tryRestoreUserIRFromState();

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

    return mainOut == mainIn;
}

void BackhouseAmpSimAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const auto totalIn = getTotalNumInputChannels();
    const auto totalOut = getTotalNumOutputChannels();
    updateMeterLevel(inputMeterLevel, computePeakLevel(buffer, totalIn));

    for (auto ch = totalIn; ch < totalOut; ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());

    const int amp = static_cast<int>(apvts.getRawParameterValue(ids::ampType)->load());
    const int guitarProfile = static_cast<int>(apvts.getRawParameterValue(ids::guitarProfile)->load());

    const float inputGainDb = apvts.getRawParameterValue(ids::inputGain)->load();
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
    const bool tunerEnabled = apvts.getRawParameterValue(ids::tunerEnabled)->load() > 0.5f;

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

    noiseGate.setThresholdDb(profGateThreshDb);
    noiseGate.process(buffer);

    ampEngine.setAmpType(amp);
    ampEngine.setInputGainDb(profInputGainDb);
    ampEngine.setInputBoost(inputBoost);
    ampEngine.setToneDb(profLowDb, profMidDb, profHighDb, profPresenceDb);
    ampEngine.setSubDb(subDb);
    ampEngine.process(buffer);

    cabSimulator.setEnabled(cabEnabled);
    cabSimulator.setAmpType(amp);
    cabSimulator.setTone(cabTone);
    cabSimulator.process(buffer);

    buffer.applyGain(juce::Decibels::decibelsToGain(outputDb));
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
    return false;
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
}

juce::AudioProcessorValueTreeState::ParameterLayout BackhouseAmpSimAudioProcessor::createParameterLayout()
{
    using Parameter = std::unique_ptr<juce::RangedAudioParameter>;
    std::vector<Parameter> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::ampType, "Amp", juce::StringArray { "Amp 1", "Amp 2", "Amp 3", "Amp 4" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(ids::guitarProfile, "Guitar Profile", juce::StringArray { "Neutral", "Profile 1", "Profile 2", "Profile 3" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(ids::inputGain, "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
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
    params.push_back(std::make_unique<juce::AudioParameterBool>(ids::tunerEnabled, "Tuner", false));

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
