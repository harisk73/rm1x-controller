#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "VoiceNames.h"

RM1xControllerAudioProcessor::RM1xControllerAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

RM1xControllerAudioProcessor::~RM1xControllerAudioProcessor()
{
    // CRITICAL: Release MIDI resources BEFORE processor is destroyed
    // to prevent device locks on Windows (ASIO/WASAPI, exclusive USB-MIDI ports)
    
    // Stop MIDI clock timer to prevent callbacks during teardown
    stopMidiClock();
    
    // Close MIDI input (exclusive port) - must stop before reset
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
    }
    
    // Close MIDI output (exclusive port) - release shared_ptr
    if (midiOutput)
    {
        midiOutput.reset();
    }
    
    // For Standalone builds, MIDI is now released before
    // the StandaloneFilterWindow's AudioDeviceManager shutdown
}

const juce::String RM1xControllerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RM1xControllerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool RM1xControllerAudioProcessor::producesMidi() const
{
    return true;
}

bool RM1xControllerAudioProcessor::isMidiEffect() const
{
    return false;
}

double RM1xControllerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RM1xControllerAudioProcessor::getNumPrograms()
{
    return 1;
}

int RM1xControllerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RM1xControllerAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String RM1xControllerAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void RM1xControllerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void RM1xControllerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void RM1xControllerAudioProcessor::releaseResources()
{
    // Called when audio device is being closed or reconfigured
    // For Standalone: aggressively release MIDI resources when audio device closes
    // to prevent device locks on Windows
    
    // Stop MIDI clock to prevent any pending callbacks
    stopMidiClock();
    
    // Stop and close MIDI input (exclusive port)
    // This ensures USB-MIDI devices like H4MIDI-WC are released
    if (midiInput)
    {
        midiInput->stop();
    }
    
    // Note: We don't reset() the shared_ptr here to allow reconnection
    // The destructor will fully release the MIDI output
}

bool RM1xControllerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RM1xControllerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    
    juce::ignoreUnused(midiMessages);
}

bool RM1xControllerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RM1xControllerAudioProcessor::createEditor()
{
    return new RM1xControllerAudioProcessorEditor(*this);
}

void RM1xControllerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeString(currentMidiOutputName);
    stream.writeString(currentMidiInputName);
}

void RM1xControllerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    
    // Read MIDI output name (always present)
    currentMidiOutputName = stream.readString();
    
    // Read MIDI input name if available (backwards compatibility with v0.1.0 state)
    if (!stream.isExhausted())
        currentMidiInputName = stream.readString();
    else
        currentMidiInputName = juce::String();
    
    // Restore MIDI output
    setMidiOutput(currentMidiOutputName);
    
    // Restore MIDI input
    setMidiInput(currentMidiInputName);
}

void RM1xControllerAudioProcessor::sendMidiMessage(const juce::MidiMessage& msg)
{
    if (midiOutput != nullptr)
        midiOutput->sendMessageNow(msg);
}

juce::StringArray RM1xControllerAudioProcessor::getAvailableMidiOutputs()
{
    juce::StringArray outputs;
    
    try
    {
        auto devices = juce::MidiOutput::getAvailableDevices();
        
        for (auto& device : devices)
            outputs.add(device.name);
    }
    catch (...)
    {
        // If MIDI output enumeration fails on Windows, return empty list
        DBG("Failed to enumerate MIDI output devices");
    }
    
    return outputs;
}

juce::StringArray RM1xControllerAudioProcessor::getAvailableMidiInputs()
{
    juce::StringArray inputs;
    auto devices = juce::MidiInput::getAvailableDevices();
    for (const auto& device : devices)
        inputs.add(device.name);
    return inputs;
}

void RM1xControllerAudioProcessor::setMidiOutput(const juce::String& deviceName)
{
    midiOutput.reset();
    currentMidiOutputName = deviceName;
    
    if (deviceName.isEmpty())
        return;
    
    try
    {
        auto devices = juce::MidiOutput::getAvailableDevices();
        
        for (auto& device : devices)
        {
            if (device.name == deviceName)
            {
                midiOutput = juce::MidiOutput::openDevice(device.identifier);
                break;
            }
        }
    }
    catch (...)
    {
        // If MIDI output opening fails on Windows, silently fail
        DBG("Failed to open MIDI output device: " + deviceName);
        midiOutput.reset();
    }
}

void RM1xControllerAudioProcessor::setMidiInput(const juce::String& deviceName)
{
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
    }
    
    currentMidiInputName = deviceName;
    
    if (deviceName.isEmpty())
        return;
    
    auto devices = juce::MidiInput::getAvailableDevices();
    
    for (auto& device : devices)
    {
        if (device.name == deviceName)
        {
            midiInput = juce::MidiInput::openDevice(device.identifier, this);
            if (midiInput)
                midiInput->start();
            break;
        }
    }
}

void RM1xControllerAudioProcessor::setTempo(int bpm)
{
    currentTempoBPM.store(juce::jlimit(5, 300, bpm));
    
    if (midiClockRunning.load())
    {
        // Recalculate interval: 60 / (BPM * 24) seconds = interval per F8
        // HighResolutionTimer interval is in milliseconds
        double intervalMs = (60.0 / (currentTempoBPM.load() * 24.0)) * 1000.0;
        stopTimer();
        startTimer(static_cast<int>(intervalMs + 0.5)); // Round to nearest ms
    }
}

void RM1xControllerAudioProcessor::startMidiClock()
{
    if (midiClockRunning.load())
        return;
    
    midiClockRunning.store(true);
    sentClockStart.store(false);
    
    // Calculate interval: 60 / (BPM * 24) seconds per F8 (24 PPQN)
    double intervalMs = (60.0 / (currentTempoBPM.load() * 24.0)) * 1000.0;
    startTimer(static_cast<int>(intervalMs + 0.5)); // Round to nearest ms
    
    // Send MIDI Start (FA) to begin transport
    if (midiOutput && !sentClockStart.load())
    {
        auto startMsg = juce::MidiMessage(0xFA);
        midiOutput->sendMessageNow(startMsg);
        sentClockStart.store(true);
    }
}

void RM1xControllerAudioProcessor::stopMidiClock()
{
    if (!midiClockRunning.load())
        return;
    
    midiClockRunning.store(false);
    stopTimer();
    
    // Send MIDI Stop (FC)
    if (midiOutput)
    {
        auto stopMsg = juce::MidiMessage(0xFC);
        midiOutput->sendMessageNow(stopMsg);
    }
    
    sentClockStart.store(false);
}

void RM1xControllerAudioProcessor::shutdownMidiResources()
{
    // Explicit shutdown of MIDI resources for Standalone mode
    // Call this before the audio device manager shuts down
    
    // Stop MIDI clock timer
    stopMidiClock();
    
    // Stop and close MIDI input (releases exclusive port on Windows)
    if (midiInput)
    {
        midiInput->stop();
        midiInput.reset();
    }
    
    // Close MIDI output (releases exclusive port on Windows)
    if (midiOutput)
    {
        midiOutput.reset();
    }
}

void RM1xControllerAudioProcessor::hiResTimerCallback()
{
    // Send MIDI Clock (F8) at regular intervals
    if (midiOutput && midiClockRunning.load())
    {
        auto clockMsg = juce::MidiMessage(0xF8);
        midiOutput->sendMessageNow(clockMsg);
    }
}

void RM1xControllerAudioProcessor::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    juce::ignoreUnused(source);
    
    // Filter out realtime clock messages (F8/FA/FB/FC) to prevent MIDI Thru feedback loops
    // RM1x echoes our transmitted clock back when MIDI Thru is active
    if (message.getRawDataSize() > 0)
    {
        auto status = message.getRawData()[0];
        if (status == 0xF8 || status == 0xFA || status == 0xFB || status == 0xFC)
            return; // Ignore echoed clock/transport messages
    }
    
    if (onMidiInputActivity)
        onMidiInputActivity();
    
    // Song Select (F3) - pattern change from hardware or broadcast after load
    if (message.isSongPositionPointer() == false && message.getRawDataSize() == 2 && message.getRawData()[0] == 0xF3)
    {
        int patternNum = message.getRawData()[1] & 0x7F;
        if (onDumpReceived)
        {
            juce::MessageManager::callAsync([this, patternNum]()
            {
                if (onDumpReceived)
                    onDumpReceived("Pattern sync: " + juce::String(patternNum));
            });
        }
    }
    else if (message.isSysEx())
    {
        auto data = message.getSysExData();
        int size = message.getSysExDataSize();
        
        // Check for Identity Reply
        if (RM1x::MIDIProtocol::isIdentityReply(message))
        {
            auto info = RM1x::MIDIProtocol::getIdentityInfo(message);
            if (onDumpReceived)
            {
                juce::MessageManager::callAsync([this, info]()
                {
                    if (onDumpReceived)
                        onDumpReceived("RM1x detected (1D 03)");
                });
            }
        }
        // Check for XG Parameter Change (part of F3 broadcast)
        else if (size >= 8 && data[0] == 0x43 && (data[1] & 0xF0) == 0x10 && data[2] == 0x4C)
        {
            // XG Multi Part param: F0 43 1n 4C 08 pp aa vv F7
            if (size >= 7 && data[3] == 0x08)
            {
                int part = data[4] & 0x0F; // Part 0-15 (channels 1-16)
                int address = data[5];
                int value = data[6];
                
                juce::ScopedLock lock(partStatesLock);
                
                // Parse Multi Part parameters
                if (address == 0x01) // Bank MSB
                {
                    partStates[part].bankMSB = value;
                }
                else if (address == 0x02) // Bank LSB
                {
                    partStates[part].bankLSB = value;
                }
                else if (address == 0x03) // Program Number
                {
                    partStates[part].program = value;
                    partStates[part].hasVoice = true;
                    
                    // Update voice name
                    partStates[part].voiceName = RM1x::VoiceNames::getVoiceName(
                        partStates[part].bankMSB,
                        partStates[part].bankLSB,
                        partStates[part].program
                    );
                    
                    // Notify UI
                    if (onPartVoiceChanged)
                    {
                        juce::MessageManager::callAsync([this, part, voiceName = partStates[part].voiceName]()
                        {
                            if (onPartVoiceChanged)
                                onPartVoiceChanged(part, voiceName);
                        });
                    }
                }
            }
        }
        // Check for Section Control SysEx
        else
        {
            parseSectionSysEx(data, size);
        }
    }
    else if (message.isController())
    {
        int channel = message.getChannel();
        int ccNumber = message.getControllerNumber();
        int value = message.getControllerValue();
        
        if (ccNumber == 11) // Expression (Mute)
        {
            parseExpressionCC(channel, value);
        }
        else if (ccNumber == 0) // Bank MSB
        {
            int part = channel - 1;
            if (part >= 0 && part < 16)
            {
                juce::ScopedLock lock(partStatesLock);
                partStates[part].bankMSB = value;
            }
        }
        else if (ccNumber == 32) // Bank LSB
        {
            int part = channel - 1;
            if (part >= 0 && part < 16)
            {
                juce::ScopedLock lock(partStatesLock);
                partStates[part].bankLSB = value;
            }
        }
    }
    else if (message.isProgramChange())
    {
        int channel = message.getChannel();
        int program = message.getProgramChangeNumber();
        int part = channel - 1;
        
        if (part >= 0 && part < 16)
        {
            juce::ScopedLock lock(partStatesLock);
            partStates[part].program = program;
            partStates[part].hasVoice = true;
            
            // Update voice name
            partStates[part].voiceName = RM1x::VoiceNames::getVoiceName(
                partStates[part].bankMSB,
                partStates[part].bankLSB,
                partStates[part].program
            );
            
            // Notify UI
            if (onPartVoiceChanged)
            {
                juce::MessageManager::callAsync([this, part, voiceName = partStates[part].voiceName]()
                {
                    if (onPartVoiceChanged)
                        onPartVoiceChanged(part, voiceName);
                });
            }
        }
    }
}

void RM1xControllerAudioProcessor::parseSectionSysEx(const juce::uint8* data, int size)
{
    if (size != 5)
        return;
    
    if (data[0] == 0x43 && data[1] == 0x7E && data[2] == 0x00)
    {
        int section = data[3];
        if (section >= 0 && section <= 15 && onSectionChanged)
        {
            juce::MessageManager::callAsync([this, section]()
            {
                if (onSectionChanged)
                    onSectionChanged(section);
            });
        }
    }
}

void RM1xControllerAudioProcessor::parseExpressionCC(int channel, int value)
{
    if (channel < 1 || channel > 16)
        return;
    
    bool isMuted = (value == 0);
    
    if (onMuteChanged)
    {
        juce::MessageManager::callAsync([this, channel, isMuted]()
        {
            if (onMuteChanged)
                onMuteChanged(channel, isMuted);
        });
    }
}

RM1xControllerAudioProcessor::PartState RM1xControllerAudioProcessor::getPartState(int part) const
{
    if (part >= 0 && part < 16)
    {
        juce::ScopedLock lock(partStatesLock);
        return partStates[part];
    }
    return PartState();
}

void RM1xControllerAudioProcessor::clearAllPartStates()
{
    juce::ScopedLock lock(partStatesLock);
    for (auto& state : partStates)
    {
        state = PartState();
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RM1xControllerAudioProcessor();
}
