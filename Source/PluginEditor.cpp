#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ProjectInfo.h"

RM1xControllerAudioProcessorEditor::RM1xControllerAudioProcessorEditor(RM1xControllerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    
    setupMIDISection();
    setupDisplaySection();
    setupTransportSection();
    setupKnobSection();
    setupKeyboardSection();
    
    audioProcessor.onMidiInputActivity = [this]()
    {
        juce::MessageManager::callAsync([this]()
        {
            if (midiInLED)
                midiInLED->flash();
        });
    };
    
    audioProcessor.onSectionChanged = [this](int section)
    {
        if (sectionRow)
        {
            sectionRow->setActiveSection(section);
            statusLabel->setText("Section " + juce::String(char('A' + section)) + " selected on hardware", juce::dontSendNotification);
        }
    };
    
    audioProcessor.onMuteChanged = [this](int channel, bool isMuted)
    {
        if (muteRow)
        {
            muteRow->setMuteState(channel, isMuted);
            statusLabel->setText("Ch " + juce::String(channel) + " " + (isMuted ? "muted" : "unmuted") + " on hardware", juce::dontSendNotification);
        }
    };
    
    audioProcessor.onPartVoiceChanged = [this](int part, const juce::String& voiceName)
    {
        if (muteRow)
        {
            muteRow->setVoiceName(part, voiceName);
            if (!voiceName.isEmpty())
                muteRow->setActiveState(part, true);
        }
    };
    
    audioProcessor.onTrackChanged = [this](int track)
    {
        statusLabel->setText("Track " + juce::String(track + 1) + " selected on hardware", juce::dontSendNotification);
    };
    
    audioProcessor.onDumpReceived = [this](const juce::String& info)
    {
        if (info.startsWith("Pattern sync:"))
        {
            int patternNum = info.fromLastOccurrenceOf(": ", false, false).getIntValue();
            if (patternNum >= 0 && patternNum < 128)
            {
                updatingPatternFromMidi.store(true);
                patternCombo->setSelectedId(patternNum + 1, juce::dontSendNotification);
                updatingPatternFromMidi.store(false);
            }
            statusLabel->setText("Pattern " + juce::String(patternNum) + " sync complete", juce::dontSendNotification);
        }
        else
        {
            statusLabel->setText(info, juce::dontSendNotification);
        }
    };
    
    // Initialize Section A as default
    if (sectionRow)
        sectionRow->setActiveSection(0);
    
    startTimer(100);
    
    setSize(1200, 820);
    setResizable(true, true);
    setResizeLimits(1000, 700, 2400, 1600);
}

RM1xControllerAudioProcessorEditor::~RM1xControllerAudioProcessorEditor()
{
    audioProcessor.onMidiInputActivity = nullptr;
    audioProcessor.onSectionChanged = nullptr;
    audioProcessor.onMuteChanged = nullptr;
    audioProcessor.onTrackChanged = nullptr;
    audioProcessor.onDumpReceived = nullptr;
    audioProcessor.onPartVoiceChanged = nullptr;
    
    setLookAndFeel(nullptr);
}

void RM1xControllerAudioProcessorEditor::setupMIDISection()
{
    titleLabel = std::make_unique<juce::Label>("title", "RM1x Controller");
    titleLabel->setFont(juce::Font(juce::FontOptions(28.0f).withStyle("Bold")));
    titleLabel->setColour(juce::Label::textColourId, juce::Colour(0xfff06030));
    addAndMakeVisible(titleLabel.get());
    
    versionLabel = std::make_unique<juce::Label>("version", "v" + juce::String(ProjectInfo::versionString));
    versionLabel->setFont(juce::FontOptions(12.0f));
    versionLabel->setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(versionLabel.get());
    
    midiInLED = std::make_unique<MidiLED>("IN");
    addAndMakeVisible(midiInLED.get());
    
    midiOutLED = std::make_unique<MidiLED>("OUT");
    addAndMakeVisible(midiOutLED.get());
    
    midiOutputLabel = std::make_unique<juce::Label>("label", "MIDI Output:");
    midiOutputLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(midiOutputLabel.get());
    
    midiOutputCombo = std::make_unique<juce::ComboBox>();
    midiOutputCombo->addItem("(None)", 1);
    addAndMakeVisible(midiOutputCombo.get());
    
    midiOutputCombo->onChange = [this]()
    {
        auto selectedId = midiOutputCombo->getSelectedId();
        if (selectedId > 1)
        {
            auto deviceName = midiOutputCombo->getText();
            audioProcessor.setMidiOutput(deviceName);
            deviceStatus = "Output: " + deviceName;
        }
        else
        {
            audioProcessor.setMidiOutput("");
            deviceStatus = "No MIDI output selected";
        }
        updateStatusDisplay();
    };
    
    midiInputLabel = std::make_unique<juce::Label>("label", "MIDI Input:");
    midiInputLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(midiInputLabel.get());
    
    midiInputCombo = std::make_unique<juce::ComboBox>();
    midiInputCombo->addItem("(None)", 1);
    addAndMakeVisible(midiInputCombo.get());
    
    // Call updateMidiDeviceList() AFTER both combos exist
    updateMidiDeviceList();
    
    midiInputCombo->onChange = [this]()
    {
        auto selectedId = midiInputCombo->getSelectedId();
        if (selectedId > 1)
        {
            auto deviceName = midiInputCombo->getText();
            audioProcessor.setMidiInput(deviceName);
        }
        else
        {
            audioProcessor.setMidiInput("");
        }
    };
    
    identityButton = std::make_unique<juce::TextButton>("ID");
    identityButton->onClick = [this]() { sendIdentityRequest(); };
    addAndMakeVisible(identityButton.get());
    
    loadPatternButton = std::make_unique<juce::TextButton>("Sync from RM1x");
    loadPatternButton->onClick = [this]() {
        // Send Identity Request to verify connection
        auto identityMsg = RM1x::MIDIProtocol::createIdentityRequest(0x7F);
        audioProcessor.sendMidiMessage(identityMsg);
        midiOutLED->flash();
        
        // Then resend current pattern Song Select to trigger RM1x setup broadcast
        juce::Timer::callAfterDelay(20, [this]() {
            int currentPattern = patternCombo->getSelectedId() - 1;
            if (currentPattern < 0) currentPattern = 0;
            auto msg = RM1x::MIDIProtocol::createSongSelect(currentPattern);
            audioProcessor.sendMidiMessage(msg);
            midiOutLED->flash();
            statusLabel->setText("Syncing from RM1x pattern " + juce::String(currentPattern) + "...", juce::dontSendNotification);
        });
    };
    addAndMakeVisible(loadPatternButton.get());
}

void RM1xControllerAudioProcessorEditor::setupDisplaySection()
{
    statusLabel = std::make_unique<juce::Label>("status", "Status: Ready");
    statusLabel->setFont(juce::FontOptions(14.0f));
    statusLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    statusLabel->setColour(juce::Label::backgroundColourId, juce::Colour(0xff1a3a1a));
    statusLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel.get());
    
    bpmLabel = std::make_unique<juce::Label>("bpm", "120");
    bpmLabel->setFont(juce::Font(juce::FontOptions(32.0f).withStyle("Bold")));
    bpmLabel->setColour(juce::Label::textColourId, juce::Colour(0xff00ff00));
    bpmLabel->setColour(juce::Label::backgroundColourId, juce::Colours::black);
    bpmLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bpmLabel.get());
    
    tempoLabel = std::make_unique<juce::Label>("tempoLabel", "Tempo (MIDI Clk):");
    tempoLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    tempoLabel->setTooltip("RM1x must be set to Sync=MIDI in Utility settings to respond to external clock");
    addAndMakeVisible(tempoLabel.get());
    
    tempoSlider = std::make_unique<juce::Slider>();
    tempoSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tempoSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    tempoSlider->setRange(5, 300, 1);
    tempoSlider->setValue(120);
    tempoSlider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff6a5acd));
    tempoSlider->setColour(juce::Slider::thumbColourId, juce::Colour(0xff9a8aff));
    tempoSlider->onValueChange = [this]()
    {
        int tempo = static_cast<int>(tempoSlider->getValue());
        tempoValueLabel->setText(juce::String(tempo) + " BPM", juce::dontSendNotification);
        bpmLabel->setText(juce::String(tempo), juce::dontSendNotification);
        
        // Update MIDI Clock tempo and restart if running
        audioProcessor.setTempo(tempo);
        if (!audioProcessor.isMidiClockRunning())
            audioProcessor.startMidiClock();
    };
    addAndMakeVisible(tempoSlider.get());
    
    tempoValueLabel = std::make_unique<juce::Label>("tempoValue", "120 BPM");
    tempoValueLabel->setFont(juce::FontOptions(12.0f));
    tempoValueLabel->setColour(juce::Label::textColourId, juce::Colour(0xff9a8aff));
    tempoValueLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tempoValueLabel.get());
    
    songSelectLabel = std::make_unique<juce::Label>("songlabel", "Pattern/Style:");
    songSelectLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(songSelectLabel.get());

    songSelectSlider = std::make_unique<juce::Slider>();
    songSelectSlider->setSliderStyle(juce::Slider::LinearHorizontal);
    songSelectSlider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    songSelectSlider->setRange(0, 127, 1);
    songSelectSlider->setValue(0);
    addAndMakeVisible(songSelectSlider.get());
    
    // Populate pattern dropdown with RM1x preset styles (1-60) + user patterns (61-128)
    patternCombo = std::make_unique<juce::ComboBox>("Pattern");
    const char* presetStyles[] = {
        "Psychedelic Trance 1", "Psychedelic Trance 2", "Psychedelic Trance 3", "Trance", "Minimal Trance",
        "Breakbeats Trance 1", "Breakbeats Trance 2", "Gabba", "Big Beat 1", "Big Beat 2",
        "Big Beat 3", "Big Beat 4", "Detroit Techno 1", "Detroit Techno 2", "Vintage Acid",
        "Bleep Techno", "Euro Techno", "Epic Trance 1", "Epic Trance 2", "Hardcore 1",
        "Hardcore 2", "Hardcore 3", "Berlin Techno", "Elektro 1", "Elektro 2",
        "Cyber Drum'n'Bass", "Hard Step 1", "Hard Step 2", "Hard Step 3", "Darkcore",
        "Tech Step", "Artcore 1", "Artcore 2", "Jazz Step 1", "Jazz Step 2",
        "Jump Up", "Ragga Jungle", "Ambient 1", "Ambient 2", "Ambient 3",
        "Ambient 4", "Dub 1", "Dub 2", "Ethnic Dance", "House",
        "Hard House", "Garage House", "NY House", "Berlin House", "Dub House",
        "Progressive House 1", "Progressive House 2", "Epic House", "Pop House", "Disco House",
        "Hip-Hop East", "Hip-Hop West", "Hip-Hop Abstract", "Hip-Hop Jazz", "Electric Jazz"
    };
    
    for (int i = 0; i < 60; ++i)
        patternCombo->addItem(juce::String(i) + ": " + presetStyles[i], i + 1);
    
    for (int i = 60; i < 128; ++i)
        patternCombo->addItem(juce::String(i) + ": User Pattern", i + 1);
    
    patternCombo->setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(patternCombo.get());
    
    patternCombo->onChange = [this]()
    {
        // Prevent re-entrant calls from MIDI In updates
        if (updatingPatternFromMidi.load())
            return;
        
        // Throttle rapid pattern changes (50ms minimum between changes)
        auto now = juce::Time::getCurrentTime().toMilliseconds();
        if (now - lastPatternChangeTime < 50)
            return;
        lastPatternChangeTime = now;
        
        int patternNum = patternCombo->getSelectedId() - 1;
        if (patternNum >= 0)
        {
            auto msg = RM1x::MIDIProtocol::createSongSelect(patternNum);
            audioProcessor.sendMidiMessage(msg);
            midiOutLED->flash();
            statusLabel->setText("Loading pattern " + juce::String(patternNum) + ": " + patternCombo->getText().fromFirstOccurrenceOf(": ", false, false), juce::dontSendNotification);
        }
    };
}

void RM1xControllerAudioProcessorEditor::setupTransportSection()
{
    playButton = std::make_unique<juce::TextButton>("PLAY");
    playButton->onClick = [this]() { handleTransport(RM1x::TransportCommand::Start); };
    addAndMakeVisible(playButton.get());
    
    stopButton = std::make_unique<juce::TextButton>("STOP");
    stopButton->onClick = [this]() { handleTransport(RM1x::TransportCommand::Stop); };
    addAndMakeVisible(stopButton.get());
    
    continueButton = std::make_unique<juce::TextButton>("CONTINUE");
    continueButton->onClick = [this]() { handleTransport(RM1x::TransportCommand::Continue); };
    addAndMakeVisible(continueButton.get());
    
    volumeLabel = std::make_unique<juce::Label>("vollabel", "MASTER VOLUME");
    volumeLabel->setFont(juce::FontOptions(11.0f));
    volumeLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    volumeLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel.get());
    
    volumeSlider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                  juce::Slider::NoTextBox);
    volumeSlider->setRange(0, 127, 1);
    volumeSlider->setValue(100);
    volumeSlider->onValueChange = [this]()
    {
        int volume = static_cast<int>(volumeSlider->getValue());
        auto msg = RM1x::MIDIProtocol::createMasterVolume(volume);
        audioProcessor.sendMidiMessage(msg);
        midiOutLED->flash();
    };
    addAndMakeVisible(volumeSlider.get());
}

void RM1xControllerAudioProcessorEditor::setupKnobSection()
{
    knobStrip = std::make_unique<KnobStrip>();
    addAndMakeVisible(knobStrip.get());
    
    knobStrip->onKnobChange = [this](int knobIndex, float value)
    {
        handleKnobChange(knobIndex, value);
    };
    
    knobBankButton = std::make_unique<juce::TextButton>("Knob Bank: A");
    knobBankButton->setClickingTogglesState(true);
    knobBankButton->onClick = [this]()
    {
        bool isA = !knobBankButton->getToggleState();
        knobStrip->setKnobBankA(isA);
        knobBankButton->setButtonText(isA ? "Knob Bank: A" : "Knob Bank: B");
    };
    addAndMakeVisible(knobBankButton.get());
}

void RM1xControllerAudioProcessorEditor::setupKeyboardSection()
{
    sectionRow = std::make_unique<SectionButtonRow>();
    addAndMakeVisible(sectionRow.get());
    
    sectionRow->onSectionPressed = [this](int section)
    {
        handleSectionChange((RM1x::Section)section);
    };
    
    trackRow = std::make_unique<TrackButtonRow>();
    addAndMakeVisible(trackRow.get());
    
    trackRow->onTrackPressed = [this](int track)
    {
        handleTrackSelected(track + 1);
    };
    
    muteRow = std::make_unique<MuteButtonRow>();
    addAndMakeVisible(muteRow.get());
    
    muteRow->onMuteToggle = [this](int track, bool muted)
    {
        handleMuteToggle(track, muted);
    };
    
    // Mute helper buttons
    muteAllButton = std::make_unique<juce::TextButton>("All");
    muteAllButton->setTooltip("Mute all channels");
    muteAllButton->onClick = [this]()
    {
        for (int i = 1; i <= 16; ++i)
        {
            auto msg = RM1x::MIDIProtocol::createExpressionMute(i, true);
            audioProcessor.sendMidiMessage(msg);
            muteRow->setMuteState(i, true);
        }
        midiOutLED->flash();
        statusLabel->setText("All channels muted", juce::dontSendNotification);
    };
    addAndMakeVisible(muteAllButton.get());
    
    mute18Button = std::make_unique<juce::TextButton>("1-8");
    mute18Button->setTooltip("Mute channels 1-8");
    mute18Button->onClick = [this]()
    {
        for (int i = 1; i <= 8; ++i)
        {
            auto msg = RM1x::MIDIProtocol::createExpressionMute(i, true);
            audioProcessor.sendMidiMessage(msg);
            muteRow->setMuteState(i, true);
        }
        midiOutLED->flash();
        statusLabel->setText("Channels 1-8 muted", juce::dontSendNotification);
    };
    addAndMakeVisible(mute18Button.get());
    
    mute916Button = std::make_unique<juce::TextButton>("9-16");
    mute916Button->setTooltip("Mute channels 9-16");
    mute916Button->onClick = [this]()
    {
        for (int i = 9; i <= 16; ++i)
        {
            auto msg = RM1x::MIDIProtocol::createExpressionMute(i, true);
            audioProcessor.sendMidiMessage(msg);
            muteRow->setMuteState(i, true);
        }
        midiOutLED->flash();
        statusLabel->setText("Channels 9-16 muted", juce::dontSendNotification);
    };
    addAndMakeVisible(mute916Button.get());
    
    clearMuteButton = std::make_unique<juce::TextButton>("Clear");
    clearMuteButton->setTooltip("Unmute all channels");
    clearMuteButton->onClick = [this]()
    {
        for (int i = 1; i <= 16; ++i)
        {
            auto msg = RM1x::MIDIProtocol::createExpressionMute(i, false);
            audioProcessor.sendMidiMessage(msg);
            muteRow->setMuteState(i, false);
        }
        midiOutLED->flash();
        statusLabel->setText("All channels unmuted", juce::dontSendNotification);
    };
    addAndMakeVisible(clearMuteButton.get());
    
    soloButton = std::make_unique<juce::TextButton>("Solo");
    soloButton->setTooltip("Solo selected track (mutes all others)");
    soloButton->onClick = [this]()
    {
        if (currentTrack >= 1 && currentTrack <= 16)
        {
            // Unmute solo track, mute all others
            for (int i = 1; i <= 16; ++i)
            {
                bool shouldMute = (i != currentTrack);
                auto msg = RM1x::MIDIProtocol::createExpressionMute(i, shouldMute);
                audioProcessor.sendMidiMessage(msg);
                muteRow->setMuteState(i, shouldMute);
                muteRow->setSoloState(i - 1, i == currentTrack);
            }
            soloTrack = currentTrack - 1;
            midiOutLED->flash();
            statusLabel->setText("Track " + juce::String(currentTrack) + " soloed", juce::dontSendNotification);
        }
    };
    addAndMakeVisible(soloButton.get());
}

void RM1xControllerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
    
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRect(10, 80, getWidth() - 20, 120);
    
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(10, 220, getWidth() - 20, 100);
}

void RM1xControllerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    auto headerArea = bounds.removeFromTop(70);
    titleLabel->setBounds(headerArea.removeFromLeft(300).removeFromTop(40));
    versionLabel->setBounds(titleLabel->getBounds().removeFromBottom(20).translated(5, 25));
    
    headerArea.removeFromLeft(20);
    midiInLED->setBounds(headerArea.removeFromLeft(50).removeFromTop(45).withTrimmedLeft(15).withTrimmedRight(15));
    headerArea.removeFromLeft(5);
    midiOutLED->setBounds(headerArea.removeFromLeft(50).removeFromTop(45).withTrimmedLeft(15).withTrimmedRight(15));
    headerArea.removeFromLeft(15);
    
    auto bpmArea = headerArea.removeFromLeft(100);
    tempoLabel->setBounds(bpmArea.removeFromTop(15));
    tempoSlider->setBounds(bpmArea.removeFromTop(60).reduced(5, 0));
    tempoValueLabel->setBounds(bpmArea);
    
    // Hidden BPM label (now shown by tempoValueLabel)
    bpmLabel->setBounds(0, 0, 0, 0);
    headerArea.removeFromLeft(20);
    
    auto midiArea = headerArea.removeFromTop(50);
    
    auto midiOutArea = midiArea.removeFromTop(25);
    midiOutputLabel->setBounds(midiOutArea.removeFromLeft(95));
    midiOutArea.removeFromLeft(5);
    midiOutputCombo->setBounds(midiOutArea.removeFromLeft(200));
    midiOutArea.removeFromLeft(10);
    identityButton->setBounds(midiOutArea.removeFromLeft(0)); // Hidden - functionality in Sync button
    loadPatternButton->setBounds(midiOutArea.removeFromLeft(180));
    
    auto midiInArea = midiArea.removeFromTop(25);
    midiInputLabel->setBounds(midiInArea.removeFromLeft(100));
    midiInArea.removeFromLeft(5);
    midiInputCombo->setBounds(midiInArea.removeFromLeft(200));
    
    bounds.removeFromTop(10);
    
    auto displayArea = bounds.removeFromTop(120);
    displayArea.reduce(10, 10);
    statusLabel->setBounds(displayArea.removeFromTop(40));
    displayArea.removeFromTop(10);
    
    auto songArea = displayArea.removeFromTop(30);
    songSelectLabel->setBounds(songArea.removeFromLeft(110));
    patternCombo->setBounds(songArea.removeFromLeft(400));
    
    bounds.removeFromTop(10);
    
    auto knobArea = bounds.removeFromTop(120);
    knobArea.reduce(10, 10);
    auto knobBankArea = knobArea.removeFromBottom(30);
    knobBankButton->setBounds(knobBankArea.removeFromLeft(150));
    knobStrip->setBounds(knobArea);
    
    bounds.removeFromTop(10);
    
    auto transportArea = bounds.removeFromTop(90);
    transportArea.reduce(10, 10);
    
    volumeLabel->setBounds(transportArea.removeFromLeft(120).removeFromTop(20));
    volumeSlider->setBounds(transportArea.removeFromLeft(80).reduced(0, 10));
    transportArea.removeFromLeft(30);
    
    playButton->setBounds(transportArea.removeFromLeft(100));
    transportArea.removeFromLeft(10);
    stopButton->setBounds(transportArea.removeFromLeft(100));
    transportArea.removeFromLeft(10);
    continueButton->setBounds(transportArea.removeFromLeft(120));
    
    bounds.removeFromTop(10);
    
    // Main button rows - Section/Track/Mute (primary interface)
    auto buttonRowsArea = bounds; // Use remaining space
    buttonRowsArea.reduce(10, 10);
    
    sectionRow->setBounds(buttonRowsArea.removeFromTop(100));
    buttonRowsArea.removeFromTop(10);
    
    trackRow->setBounds(buttonRowsArea.removeFromTop(100));
    buttonRowsArea.removeFromTop(10);
    
    // Mute helper buttons row (below label, above mute row)
    auto muteHelpersArea = buttonRowsArea.removeFromTop(35);
    int buttonWidth = 60;
    int spacing = 5;
    muteHelpersArea.removeFromLeft(10); // Left padding
    muteAllButton->setBounds(muteHelpersArea.removeFromLeft(buttonWidth));
    muteHelpersArea.removeFromLeft(spacing);
    mute18Button->setBounds(muteHelpersArea.removeFromLeft(buttonWidth));
    muteHelpersArea.removeFromLeft(spacing);
    mute916Button->setBounds(muteHelpersArea.removeFromLeft(buttonWidth));
    muteHelpersArea.removeFromLeft(spacing);
    clearMuteButton->setBounds(muteHelpersArea.removeFromLeft(buttonWidth));
    muteHelpersArea.removeFromLeft(spacing * 3);
    soloButton->setBounds(muteHelpersArea.removeFromLeft(buttonWidth));
    
    buttonRowsArea.removeFromTop(5);
    muteRow->setBounds(buttonRowsArea); // Use all remaining space
}

void RM1xControllerAudioProcessorEditor::timerCallback()
{
    static int counter = 0;
    if (++counter % 10 == 0)
        updateMidiDeviceList();
}

void RM1xControllerAudioProcessorEditor::updateMidiDeviceList()
{
    // Update MIDI Output combo
    auto currentOutputText = midiOutputCombo->getText();
    auto outputs = audioProcessor.getAvailableMidiOutputs();
    
    midiOutputCombo->clear();
    midiOutputCombo->addItem("(None)", 1);
    
    int id = 2;
    int selectedId = 1;
    for (auto& output : outputs)
    {
        midiOutputCombo->addItem(output, id);
        if (output == currentOutputText || output == audioProcessor.getCurrentMidiOutput())
            selectedId = id;
        id++;
    }
    
    midiOutputCombo->setSelectedId(selectedId, juce::dontSendNotification);
    
    // Update MIDI Input combo
    auto currentInputText = midiInputCombo->getText();
    auto inputs = audioProcessor.getAvailableMidiInputs();
    
    midiInputCombo->clear();
    midiInputCombo->addItem("(None)", 1);
    
    id = 2;
    selectedId = 1;
    for (auto& input : inputs)
    {
        midiInputCombo->addItem(input, id);
        if (input == currentInputText || input == audioProcessor.getCurrentMidiInput())
            selectedId = id;
        id++;
    }
    
    midiInputCombo->setSelectedId(selectedId, juce::dontSendNotification);
}

void RM1xControllerAudioProcessorEditor::updateStatusDisplay()
{
    juce::String status = "Track " + juce::String(currentTrack) + " | " + deviceStatus;
    statusLabel->setText(status, juce::dontSendNotification);
}

void RM1xControllerAudioProcessorEditor::sendIdentityRequest()
{
    auto msg = RM1x::MIDIProtocol::createIdentityRequest();
    audioProcessor.sendMidiMessage(msg);
    midiOutLED->flash();
    deviceStatus = "Identity request sent";
    updateStatusDisplay();
}

void RM1xControllerAudioProcessorEditor::handleTransport(RM1x::TransportCommand cmd)
{
    auto msg = RM1x::MIDIProtocol::createTransport(cmd);
    audioProcessor.sendMidiMessage(msg);
    midiOutLED->flash();
    
    juce::String cmdName = (cmd == RM1x::TransportCommand::Start) ? "START" :
                           (cmd == RM1x::TransportCommand::Stop) ? "STOP" : "CONTINUE";
    deviceStatus = "Transport: " + cmdName;
    updateStatusDisplay();
}

void RM1xControllerAudioProcessorEditor::handleSectionChange(RM1x::Section section)
{
    auto msg = RM1x::MIDIProtocol::createSectionChange(section);
    audioProcessor.sendMidiMessage(msg);
    midiOutLED->flash();
    
    char sectionChar = 'A' + (int)section;
    deviceStatus = "Section: " + juce::String::charToString(sectionChar);
    updateStatusDisplay();
}

void RM1xControllerAudioProcessorEditor::handleMuteToggle(int track, bool muted)
{
    muteStates[track - 1] = muted;
    
    auto msg = RM1x::MIDIProtocol::createExpressionMute(track, muted);
    audioProcessor.sendMidiMessage(msg);
    midiOutLED->flash();
    
    deviceStatus = "Track " + juce::String(track) + " mute (Expr): " + (muted ? "ON" : "OFF");
    updateStatusDisplay();
}

void RM1xControllerAudioProcessorEditor::handleTrackSelected(int track)
{
    currentTrack = track;
    if (muteRow)
        muteRow->setSelectedTrack(track - 1); // Convert to 0-based
    deviceStatus = "Track " + juce::String(track) + " selected";
    updateStatusDisplay();
}

void RM1xControllerAudioProcessorEditor::handleKnobChange(int knobIndex, float value)
{
    int intValue = (int)value;
    juce::MidiMessage msg;
    
    bool isKnobA = knobStrip->isKnobBankA();
    
    if (isKnobA)
    {
        switch (knobIndex)
        {
            case 4:
                msg = RM1x::MIDIProtocol::createCutoff(currentTrack, intValue);
                break;
            case 5:
                msg = RM1x::MIDIProtocol::createResonance(currentTrack, intValue);
                break;
            default:
                return;
        }
    }
    else
    {
        switch (knobIndex)
        {
            case 5:
                msg = RM1x::MIDIProtocol::createChorus(currentTrack, intValue);
                break;
            case 6:
                msg = RM1x::MIDIProtocol::createReverb(currentTrack, intValue);
                break;
            case 7:
                msg = juce::MidiMessage::controllerEvent(currentTrack, 7, intValue);
                break;
            default:
                return;
        }
    }
    
    audioProcessor.sendMidiMessage(msg);
    midiOutLED->flash();
}
