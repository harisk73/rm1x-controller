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
    
    // Check for custom button states (for Mute row)
    bool isMuted = button.getProperties()["muted"];
    bool isActive = button.getProperties()["active"];
    bool isSelected = button.getProperties()["selected"];
    bool isSolo = button.getProperties()["solo"];
    
    // Determine button color based on state priority: Solo > Mute > Selected > Active > Inactive
    juce::Colour fillColour;
    juce::Colour borderColour;
    bool showBorder = false;
    
    if (isSolo)
    {
        // Solo state: magenta fill
        fillColour = juce::Colour(0xffff00ff); // Magenta
        borderColour = juce::Colour(0xffff6fff);
        showBorder = true;
    }
    else if (isMuted)
    {
        // Muted state: amber/orange fill
        fillColour = juce::Colour(0xfff06030); // Amber/orange
        borderColour = juce::Colour(0xffff8050);
        showBorder = true;
    }
    else if (isSelected)
    {
        // Selected track: purple outline only on dark background
        fillColour = isActive ? juce::Colour(0xff2a4a4a) : juce::Colour(0xff1a1a1a); // Teal-ish or dark
        borderColour = juce::Colour(0xff6a5acd); // Purple outline
        showBorder = true;
    }
    else if (isActive)
    {
        // Active/unmuted with voice: teal/cyan
        fillColour = juce::Colour(0xff2a5a5a); // Teal/cyan
        borderColour = juce::Colour(0xff4a9a9a);
        showBorder = true;
    }
    else if (shouldDrawButtonAsDown || button.getToggleState())
    {
        // Legacy active state for non-mute buttons: bright purple-blue
        fillColour = juce::Colour(0xff6a5acd);
        borderColour = juce::Colour(0xff9a8aff);
        showBorder = true;
    }
    else
    {
        // Inactive state: dark background with visible border
        fillColour = juce::Colour(0xff1a1a1a);
        borderColour = juce::Colour(0xff444444); // Visible gray border
        
        if (shouldDrawButtonAsHighlighted)
            fillColour = fillColour.brighter(0.3f);
        
        showBorder = true;
    }
    
    // Draw fill
    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw border
    if (showBorder)
    {
        g.setColour(borderColour.withAlpha(0.9f));
        g.drawRoundedRectangle(bounds, 3.0f, 2.0f);
    }
    
    // Set text color
    if (isMuted || isSolo || isActive || button.getToggleState())
    {
        button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }
    else
    {
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
    
    // Draw LED circle in top 20x20 area
    auto ledBounds = bounds.removeFromTop(20);
    g.setColour(isLit ? juce::Colour(0xff00ff00) : juce::Colour(0xff003300));
    g.fillEllipse(ledBounds.toFloat());
    
    g.setColour(juce::Colours::darkgrey);
    g.drawEllipse(ledBounds.toFloat(), 1.0f);
    
    // Draw label below LED
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(8.0f));
    g.drawText(labelText, bounds, juce::Justification::centredTop);
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
            buttons[i]->getProperties().set("muted", muted);
            buttons[i]->repaint();
            if (onMuteToggle)
                onMuteToggle(i + 1, muted);
        };
        
        voiceLabels[i] = std::make_unique<juce::Label>();
        voiceLabels[i]->setFont(juce::FontOptions(10.0f));
        voiceLabels[i]->setJustificationType(juce::Justification::centred);
        voiceLabels[i]->setColour(juce::Label::textColourId, juce::Colour(0xff888888));
        voiceLabels[i]->setText("-", juce::dontSendNotification);
        addAndMakeVisible(voiceLabels[i].get());
    }
}

void MuteButtonRow::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(22); // Remove label area
    auto voiceLabelHeight = 16;
    
    int buttonWidth = bounds.getWidth() / 16;
    int startY = 22; // Start after label area
    
    for (int i = 0; i < 16; ++i)
    {
        int x = i * buttonWidth;
        int width = buttonWidth - 3;
        
        // Voice label at bottom
        voiceLabels[i]->setBounds(x, startY + bounds.getHeight() - voiceLabelHeight, width, voiceLabelHeight);
        
        // Button between label area and voice label
        buttons[i]->setBounds(x, startY, width, bounds.getHeight() - voiceLabelHeight);
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
        int index = track - 1;
        muteStates[index] = muted;
        buttons[index]->setToggleState(muted, juce::dontSendNotification);
        buttons[index]->getProperties().set("muted", muted);
        buttons[index]->repaint();
    }
}

bool MuteButtonRow::getMuteState(int track) const
{
    if (track >= 1 && track <= 16)
        return muteStates[track - 1];
    return false;
}

void MuteButtonRow::setVoiceName(int track, const juce::String& name)
{
    if (track >= 0 && track < 16)
    {
        voiceNames[track] = name;
        
        // Truncate name if too long (8 chars max), use ASCII dash for unknown
        juce::String displayName = name.isEmpty() ? "-" : name.substring(0, 8);
        voiceLabels[track]->setText(displayName, juce::dontSendNotification);
        voiceLabels[track]->setTooltip(name.isEmpty() ? "Trk " + juce::String(track + 1) : name);
    }
}

void MuteButtonRow::setActiveState(int track, bool active)
{
    if (track >= 0 && track < 16)
    {
        activeStates[track] = active;
        buttons[track]->getProperties().set("active", active);
        buttons[track]->repaint();
    }
}

void MuteButtonRow::setSelectedTrack(int track)
{
    // Clear previous selection
    if (selectedTrack >= 0 && selectedTrack < 16)
    {
        buttons[selectedTrack]->getProperties().set("selected", false);
        buttons[selectedTrack]->repaint();
    }
    
    selectedTrack = track;
    
    // Set new selection
    if (selectedTrack >= 0 && selectedTrack < 16)
    {
        buttons[selectedTrack]->getProperties().set("selected", true);
        buttons[selectedTrack]->repaint();
    }
}

void MuteButtonRow::setSoloState(int track, bool solo)
{
    if (track >= 0 && track < 16)
    {
        soloStates[track] = solo;
        buttons[track]->getProperties().set("solo", solo);
        buttons[track]->repaint();
    }
}
