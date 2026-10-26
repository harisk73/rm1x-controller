#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "PanelComponents.h"

class RM1xControllerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    RM1xControllerAudioProcessorEditor(RM1xControllerAudioProcessor&);
    ~RM1xControllerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void setupMIDISection();
    void setupTransportSection();
    void setupKnobSection();
    void setupKeyboardSection();
    void setupDisplaySection();
    
    void sendIdentityRequest();
    void handleTransport(RM1x::TransportCommand cmd);
    void handleSectionChange(RM1x::Section section);
    void handleMuteToggle(int track, bool muted);
    void handleTrackSelected(int track);
    void handleKnobChange(int knobIndex, float value);
    
    void updateMidiDeviceList();
    void updateStatusDisplay();
    
    RM1xControllerAudioProcessor& audioProcessor;
    RM1xLookAndFeel lookAndFeel;
    
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> versionLabel;
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::Label> bpmLabel;
    
    std::unique_ptr<MidiLED> midiInLED;
    std::unique_ptr<MidiLED> midiOutLED;
    
    std::unique_ptr<juce::ComboBox> midiOutputCombo;
    std::unique_ptr<juce::Label> midiOutputLabel;
    std::unique_ptr<juce::ComboBox> midiInputCombo;
    std::unique_ptr<juce::Label> midiInputLabel;
    
    std::unique_ptr<juce::TextButton> identityButton;
    std::unique_ptr<juce::TextButton> loadPatternButton;
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> continueButton;
    
    std::unique_ptr<juce::Slider> songSelectSlider;
    std::unique_ptr<juce::Label> songSelectLabel;
    std::unique_ptr<juce::ComboBox> patternCombo;
    
    std::unique_ptr<KnobStrip> knobStrip;
    std::unique_ptr<juce::TextButton> knobBankButton;
    
    std::unique_ptr<SectionButtonRow> sectionRow;
    std::unique_ptr<TrackButtonRow> trackRow;
    std::unique_ptr<MuteButtonRow> muteRow;
    
    std::unique_ptr<juce::Slider> volumeSlider;
    std::unique_ptr<juce::Label> volumeLabel;
    
    int currentTrack = 1;
    bool muteStates[16] = { false };
    juce::String deviceStatus;
    
    std::atomic<bool> updatingPatternFromMidi { false };
    juce::int64 lastPatternChangeTime { 0 };
    
    std::unique_ptr<juce::Slider> tempoSlider;
    std::unique_ptr<juce::Label> tempoLabel;
    std::unique_ptr<juce::Label> tempoValueLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RM1xControllerAudioProcessorEditor)
};
