#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace RM1x
{

enum class Section
{
    A = 0x00, B, C, D, E, F, G, H,
    I, J, K, L, M, N, O, P
};

enum class TransportCommand
{
    Start = 0xFA,
    Continue = 0xFB,
    Stop = 0xFC
};

class MIDIProtocol
{
public:
    static juce::MidiMessage createIdentityRequest(int channel = 0);
    static juce::MidiMessage createSectionChange(Section section);
    static juce::MidiMessage createSongSelect(int songNumber);
    static juce::MidiMessage createTransport(TransportCommand cmd);
    static juce::MidiMessage createExpressionMute(int channel, bool muted);
    
    static juce::MidiMessage createXGParameterChange(int channel, 
                                                       juce::uint8 addressHigh,
                                                       juce::uint8 addressMid, 
                                                       juce::uint8 addressLow,
                                                       const juce::Array<juce::uint8>& data);
    
    static juce::MidiMessage createMultiPartRequest(int partNumber);
    static juce::MidiMessage createAllPartsRequest();
    
    static juce::MidiMessage createCutoff(int channel, int value);
    static juce::MidiMessage createResonance(int channel, int value);
    static juce::MidiMessage createReverb(int channel, int value);
    static juce::MidiMessage createChorus(int channel, int value);
    static juce::MidiMessage createPan(int channel, int value);
    static juce::MidiMessage createMasterVolume(int value); // 0-127, sent as 14-bit
    
    static bool isIdentityReply(const juce::MidiMessage& msg);
    static juce::String getIdentityInfo(const juce::MidiMessage& msg);
    
private:
    static constexpr juce::uint8 YAMAHA_ID = 0x43;
    static constexpr juce::uint8 MODEL_ID = 0x4C;
    static constexpr juce::uint8 UNIVERSAL_SYSEX = 0x7E;
};

} // namespace RM1x
