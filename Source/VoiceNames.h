#pragma once

#include <juce_core/juce_core.h>
#include <map>

namespace RM1x
{

struct VoiceKey
{
    int bankMSB;
    int bankLSB;
    int program;
    
    bool operator<(const VoiceKey& other) const
    {
        if (bankMSB != other.bankMSB) return bankMSB < other.bankMSB;
        if (bankLSB != other.bankLSB) return bankLSB < other.bankLSB;
        return program < other.program;
    }
};

class VoiceNames
{
public:
    static juce::String getVoiceName(int bankMSB, int bankLSB, int program);
    static juce::String getShortName(int bankMSB, int bankLSB, int program, int maxChars = 8);
    
private:
    static const std::map<VoiceKey, juce::String>& getVoiceMap();
};

} // namespace RM1x
