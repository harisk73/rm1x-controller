#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MIDIProtocol.h"

class RM1xLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RM1xLookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
                         
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                             
    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, 
                       bool shouldDrawButtonAsDown) override;
};

class MidiLED : public juce::Component, public juce::Timer
{
public:
    MidiLED(const juce::String& label);
    void paint(juce::Graphics& g) override;
    void flash();
    void timerCallback() override;
    
private:
    juce::String labelText;
    bool isLit = false;
};

class TransportButton : public juce::TextButton
{
public:
    TransportButton(const juce::String& label);
};

class KeyboardKey : public juce::TextButton
{
public:
    KeyboardKey(int keyIndex, bool isBlack = false);
    int getKeyIndex() const { return keyIndex; }
    
private:
    int keyIndex;
};

class KnobStrip : public juce::Component
{
public:
    KnobStrip();
    void resized() override;
    
    juce::Slider* getKnob(int index) { return knobs[index].get(); }
    bool isKnobBankA() const { return knobBankA; }
    void setKnobBankA(bool bankA) { knobBankA = bankA; }
    
    std::function<void(int, float)> onKnobChange;
    
private:
    std::array<std::unique_ptr<juce::Slider>, 8> knobs;
    std::array<std::unique_ptr<juce::Label>, 8> labels;
    bool knobBankA = true;
};

class SectionButtonRow : public juce::Component
{
public:
    SectionButtonRow();
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setActiveSection(int section);
    int getActiveSection() const { return activeSection; }
    
    std::function<void(int)> onSectionPressed;
    
private:
    std::array<std::unique_ptr<juce::TextButton>, 16> buttons;
    int activeSection = -1;
};

class TrackButtonRow : public juce::Component
{
public:
    TrackButtonRow();
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setActiveTrack(int track);
    int getActiveTrack() const { return activeTrack; }
    
    std::function<void(int)> onTrackPressed;
    
private:
    std::array<std::unique_ptr<juce::TextButton>, 16> buttons;
    int activeTrack = -1;
};

class MuteButtonRow : public juce::Component
{
public:
    MuteButtonRow();
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setMuteState(int track, bool muted);
    bool getMuteState(int track) const;
    void setVoiceName(int track, const juce::String& name);
    void setActiveState(int track, bool active);
    void setSelectedTrack(int track);
    void setSoloState(int track, bool solo);
    
    std::function<void(int, bool)> onMuteToggle;
    
private:
    std::array<std::unique_ptr<juce::TextButton>, 16> buttons;
    std::array<std::unique_ptr<juce::Label>, 16> voiceLabels;
    std::array<bool, 16> muteStates = {};
    std::array<bool, 16> activeStates = {};
    std::array<bool, 16> soloStates = {};
    std::array<juce::String, 16> voiceNames;
    int selectedTrack = -1;
};
