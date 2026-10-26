#include "PanelComponents.h"

RM1xLookAndFeel::RM1xLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colours::lightgrey);
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff06030));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff1a1a1a));
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a2a));
    setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);  // White text on bright green active state
}

void RM1xLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                       juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    g.setColour(findColour(juce::Slider::rotarySliderOutlineColourId));
    g.fillEllipse(bounds);
    
    g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
    juce::Path arc;
    arc.addCentredArc(centreX, centreY, radius * 0.8f, radius * 0.8f, 0.0f, 
                     rotaryStartAngle, angle, true);
    g.strokePath(arc, juce::PathStrokeType(3.0f));
    
    juce::Path pointer;
    auto pointerLength = radius * 0.6f;
    auto pointerThickness = 2.5f;
    pointer.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    
    g.setColour(juce::Colours::white);
    g.fillPath(pointer);
}

void RM1xLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted, 
                                           bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    
    // Active state: bright purple-blue with strong visibility
    if (shouldDrawButtonAsDown || button.getToggleState())
    {
        // Bright purple-blue fill for active state
        g.setColour(juce::Colour(0xff6a5acd));
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Bright purple glow border for active state
        g.setColour(juce::Colour(0xff9a8aff).withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 3.0f, 2.5f);
        
        // Set button text to bright white for active state
        button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }
    else
    {
        // Inactive state: dark background
        auto baseColour = juce::Colour(0xff1a1a1a);  // Very dark for clear contrast
        
        if (shouldDrawButtonAsHighlighted)
            baseColour = baseColour.brighter(0.3f);
        
        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Dim border for inactive state
        g.setColour(juce::Colour(0xff444444));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
        
        // Set button text to light grey for inactive state
        button.setColour(juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    }
}

void RM1xLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, 
                                     bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Font font(juce::FontOptions(20.0f, juce::Font::bold));
    g.setFont(font);
    
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId 
                                                           : juce::TextButton::textColourOffId)
                      .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
    
    auto textArea = button.getLocalBounds().reduced(2);
    g.drawText(button.getButtonText(), textArea, juce::Justification::centred, true);
}

MidiLED::MidiLED(const juce::String& label) : labelText(label)
{
}

void MidiLED::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText(labelText, bounds.removeFromTop(15), juce::Justification::centred);
    
    auto ledBounds = bounds.reduced(5);
    g.setColour(isLit ? juce::Colour(0xff00ff00) : juce::Colour(0xff003300));
    g.fillEllipse(ledBounds.toFloat());
    
    g.setColour(juce::Colours::darkgrey);
    g.drawEllipse(ledBounds.toFloat(), 1.0f);
}

void MidiLED::flash()
{
    isLit = true;
    repaint();
    startTimer(50);
}

void MidiLED::timerCallback()
{
    isLit = false;
    repaint();
    stopTimer();
}

TransportButton::TransportButton(const juce::String& label)
{
    setButtonText(label);
    setClickingTogglesState(false);
}

KeyboardKey::KeyboardKey(int idx, bool isBlack) : keyIndex(idx)
{
    if (!isBlack)
    {
        setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    }
    else
    {
        setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    
    setClickingTogglesState(true);
}

KnobStrip::KnobStrip()
{
    const char* bankALabels[] = { "Beat Stretch", "Clock Shift", "Gate Time", "Velocity",
                                  "Cutoff", "Resonance", "EG Decay", "Pitch Bend" };
    
    for (int i = 0; i < 8; ++i)
    {
        knobs[i] = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, 
                                                   juce::Slider::NoTextBox);
        knobs[i]->setRange(0.0, 127.0, 1.0);
        knobs[i]->setValue(64.0);
        addAndMakeVisible(knobs[i].get());
        
        labels[i] = std::make_unique<juce::Label>("", bankALabels[i]);
        labels[i]->setFont(juce::FontOptions(11.0f));
        labels[i]->setJustificationType(juce::Justification::centred);
        labels[i]->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(labels[i].get());
        
        knobs[i]->onValueChange = [this, i]()
        {
            if (onKnobChange)
                onKnobChange(i, (float)knobs[i]->getValue());
        };
    }
}

void KnobStrip::resized()
{
    auto bounds = getLocalBounds();
    int knobWidth = bounds.getWidth() / 8;
    
    for (int i = 0; i < 8; ++i)
    {
        auto knobBounds = bounds.removeFromLeft(knobWidth).reduced(5);
        labels[i]->setBounds(knobBounds.removeFromTop(20));
        knobs[i]->setBounds(knobBounds);
    }
}

SectionButtonRow::SectionButtonRow()
{
    const char* labels[] = { "A", "B", "C", "D", "E", "F", "G", "H", 
                             "I", "J", "K", "L", "M", "N", "O", "P" };
    
    for (int i = 0; i < 16; ++i)
    {
        buttons[i] = std::make_unique<juce::TextButton>(labels[i]);
        buttons[i]->setClickingTogglesState(false);
        addAndMakeVisible(buttons[i].get());
        
        buttons[i]->onClick = [this, i]()
        {
            setActiveSection(i);
            if (onSectionPressed)
                onSectionPressed(i);
        };
    }
}

void SectionButtonRow::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(22);
    int buttonWidth = bounds.getWidth() / 16;
    
    for (int i = 0; i < 16; ++i)
    {
        buttons[i]->setBounds(i * buttonWidth, 22, buttonWidth - 3, bounds.getHeight() - 22);
    }
}

void SectionButtonRow::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("SECTION", getLocalBounds().removeFromTop(20).reduced(2, 0), juce::Justification::centredLeft);
}

void SectionButtonRow::setActiveSection(int section)
{
    activeSection = section;
    for (int i = 0; i < 16; ++i)
    {
        buttons[i]->setToggleState(i == section, juce::dontSendNotification);
    }
}

TrackButtonRow::TrackButtonRow()
{
    for (int i = 0; i < 16; ++i)
    {
        buttons[i] = std::make_unique<juce::TextButton>(juce::String(i + 1));
        buttons[i]->setClickingTogglesState(false);
        addAndMakeVisible(buttons[i].get());
        
        buttons[i]->onClick = [this, i]()
        {
            setActiveTrack(i);
            if (onTrackPressed)
                onTrackPressed(i);
        };
    }
}

void TrackButtonRow::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(22);
    int buttonWidth = bounds.getWidth() / 16;
    
    for (int i = 0; i < 16; ++i)
    {
        buttons[i]->setBounds(i * buttonWidth, 22, buttonWidth - 3, bounds.getHeight() - 22);
    }
}

void TrackButtonRow::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("TRACK", getLocalBounds().removeFromTop(20).reduced(2, 0), juce::Justification::centredLeft);
}

void TrackButtonRow::setActiveTrack(int track)
{
    activeTrack = track;
    for (int i = 0; i < 16; ++i)
    {
        buttons[i]->setToggleState(i == track, juce::dontSendNotification);
    }
}

MuteButtonRow::MuteButtonRow()
{
    for (int i = 0; i < 16; ++i)
    {
        buttons[i] = std::make_unique<juce::TextButton>(juce::String(i + 1));
        buttons[i]->setClickingTogglesState(true);
        addAndMakeVisible(buttons[i].get());
        
        buttons[i]->onClick = [this, i]()
        {
            bool muted = buttons[i]->getToggleState();
            muteStates[i] = muted;
            if (onMuteToggle)
                onMuteToggle(i + 1, muted);
        };
    }
}

void MuteButtonRow::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(22);
    int buttonWidth = bounds.getWidth() / 16;
    
    for (int i = 0; i < 16; ++i)
    {
        buttons[i]->setBounds(i * buttonWidth, 22, buttonWidth - 3, bounds.getHeight() - 22);
    }
}

void MuteButtonRow::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText("MUTE (Expr)", getLocalBounds().removeFromTop(20).reduced(2, 0), juce::Justification::centredLeft);
}

void MuteButtonRow::setMuteState(int track, bool muted)
{
    if (track >= 1 && track <= 16)
    {
        muteStates[track - 1] = muted;
        buttons[track - 1]->setToggleState(muted, juce::dontSendNotification);
    }
}

bool MuteButtonRow::getMuteState(int track) const
{
    if (track >= 1 && track <= 16)
        return muteStates[track - 1];
    return false;
}
