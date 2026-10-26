#include "MIDIProtocol.h"

namespace RM1x
{

juce::MidiMessage MIDIProtocol::createIdentityRequest(int channel)
{
    juce::uint8 data[] = { 0xF0, UNIVERSAL_SYSEX, (juce::uint8)(channel & 0x0F), 0x06, 0x01, 0xF7 };
    return juce::MidiMessage(data, 6);
}

juce::MidiMessage MIDIProtocol::createSectionChange(Section section)
{
    juce::uint8 data[] = { 
        0xF0, 
        YAMAHA_ID, 
        0x7E,
        0x00, 
        (juce::uint8)section,
        0x7F,
        0xF7 
    };
    return juce::MidiMessage(data, 7);
}

juce::MidiMessage MIDIProtocol::createSongSelect(int songNumber)
{
    return juce::MidiMessage(0xF3, juce::jlimit(0, 127, songNumber), 0);
}

juce::MidiMessage MIDIProtocol::createTransport(TransportCommand cmd)
{
    return juce::MidiMessage((int)cmd);
}

juce::MidiMessage MIDIProtocol::createExpressionMute(int channel, bool muted)
{
    return juce::MidiMessage::controllerEvent(channel, 11, muted ? 0 : 127);
}

juce::MidiMessage MIDIProtocol::createXGParameterChange(int channel,
                                                         juce::uint8 addressHigh,
                                                         juce::uint8 addressMid,
                                                         juce::uint8 addressLow,
                                                         const juce::Array<juce::uint8>& data)
{
    juce::MemoryBlock block;
    juce::uint8 startSysEx = 0xF0;
    block.append(&startSysEx, 1);
    block.append(&YAMAHA_ID, 1);
    
    juce::uint8 deviceNum = 0x10 | (channel & 0x0F);
    block.append(&deviceNum, 1);
    block.append(&MODEL_ID, 1);
    block.append(&addressHigh, 1);
    block.append(&addressMid, 1);
    block.append(&addressLow, 1);
    
    for (auto byte : data)
        block.append(&byte, 1);
    
    juce::uint8 endSysEx = 0xF7;
    block.append(&endSysEx, 1);
    
    return juce::MidiMessage(block.begin(), (int)block.getSize());
}

juce::MidiMessage MIDIProtocol::createMultiPartRequest(int partNumber)
{
    // Request Multi Part parameters for a specific part (0-15)
    // XG Parameter Request: F0 43 3n 4C ah am al F7
    // Multi Part base address: 08 nn 00 where nn = part number
    juce::uint8 data[] = { 
        0xF0, 
        YAMAHA_ID, 
        0x30,  // Parameter Request (3n where n=0)
        MODEL_ID,
        0x08,  // Multi Part high address
        (juce::uint8)(partNumber & 0x0F),  // Part number
        0x00,  // Low address
        0xF7 
    };
    return juce::MidiMessage(data, 8);
}

juce::MidiMessage MIDIProtocol::createAllPartsRequest()
{
    // Request all Multi Part parameters
    juce::uint8 data[] = { 
        0xF0, 
        YAMAHA_ID, 
        0x30,
        MODEL_ID,
        0x08,  // Multi Part base
        0x00,  // Start from part 0
        0x00,
        0xF7 
    };
    return juce::MidiMessage(data, 8);
}

juce::MidiMessage MIDIProtocol::createCutoff(int channel, int value)
{
    return juce::MidiMessage::controllerEvent(channel, 74, juce::jlimit(0, 127, value));
}

juce::MidiMessage MIDIProtocol::createResonance(int channel, int value)
{
    return juce::MidiMessage::controllerEvent(channel, 71, juce::jlimit(0, 127, value));
}

juce::MidiMessage MIDIProtocol::createReverb(int channel, int value)
{
    return juce::MidiMessage::controllerEvent(channel, 91, juce::jlimit(0, 127, value));
}

juce::MidiMessage MIDIProtocol::createChorus(int channel, int value)
{
    return juce::MidiMessage::controllerEvent(channel, 93, juce::jlimit(0, 127, value));
}

juce::MidiMessage MIDIProtocol::createPan(int channel, int value)
{
    return juce::MidiMessage::controllerEvent(channel, 10, juce::jlimit(0, 127, value));
}

bool MIDIProtocol::isIdentityReply(const juce::MidiMessage& msg)
{
    if (!msg.isSysEx() || msg.getSysExDataSize() < 6)
        return false;
        
    auto data = msg.getSysExData();
    return data[0] == UNIVERSAL_SYSEX && 
           data[2] == 0x06 && 
           data[3] == 0x02;
}

juce::String MIDIProtocol::getIdentityInfo(const juce::MidiMessage& msg)
{
    if (!isIdentityReply(msg))
        return "Not an identity reply";
        
    auto data = msg.getSysExData();
    int size = msg.getSysExDataSize();
    
    if (size >= 10)
    {
        juce::uint8 family1 = data[5];
        juce::uint8 family2 = data[6];
        juce::uint8 model1 = data[7];
        juce::uint8 model2 = data[8];
        
        if (family1 == 0x1D && family2 == 0x03)
            return "Yamaha RM1x";
    }
    
    return "Unknown device";
}

} // namespace RM1x
