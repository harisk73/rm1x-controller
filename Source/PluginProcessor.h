#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MIDIProtocol.h"

class RM1xControllerAudioProcessor : public juce::AudioProcessor,
                                     private juce::MidiInputCallback,
                                     private juce::HighResolutionTimer
{
public:
    RM1xControllerAudioProcessor();
    ~RM1xControllerAudioProcessor() override;

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
    
    void sendMidiMessage(const juce::MidiMessage& msg);
    
    juce::StringArray getAvailableMidiOutputs();
    juce::StringArray getAvailableMidiInputs();
    void setMidiOutput(const juce::String& deviceName);
    void setMidiInput(const juce::String& deviceName);
    juce::String getCurrentMidiOutput() const { return currentMidiOutputName; }
    juce::String getCurrentMidiInput() const { return currentMidiInputName; }
    
    void setTempo(int bpm);
    void startMidiClock();
    void stopMidiClock();
    bool isMidiClockRunning() const { return midiClockRunning; }
    
    // MIDI input callbacks
    struct PartState
    {
        int bankMSB = 0;
        int bankLSB = 0;
        int program = 0;
        bool hasVoice = false;
        juce::String voiceName;
    };
    
    std::function<void()> onMidiInputActivity;
    std::function<void(int)> onSectionChanged;
    std::function<void(int, bool)> onMuteChanged;
    std::function<void(int)> onTrackChanged;
    std::function<void(const juce::String&)> onDumpReceived;
    std::function<void(int, const juce::String&)> onPartVoiceChanged;
    
    PartState getPartState(int part) const;
    void clearAllPartStates();

private:
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void parseSectionSysEx(const juce::uint8* data, int size);
    void parseExpressionCC(int channel, int value);
    void hiResTimerCallback() override;
    
    std::shared_ptr<juce::MidiOutput> midiOutput;
    std::unique_ptr<juce::MidiInput> midiInput;
    juce::String currentMidiOutputName;
    juce::String currentMidiInputName;
    
    std::atomic<int> currentTempoBPM { 120 };
    std::atomic<bool> midiClockRunning { false };
    std::atomic<bool> sentClockStart { false };
    
    std::array<PartState, 16> partStates;
    juce::CriticalSection partStatesLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RM1xControllerAudioProcessor)
};
