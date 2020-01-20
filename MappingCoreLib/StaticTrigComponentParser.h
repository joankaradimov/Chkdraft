#ifndef STATICTRIGCOMPONENTPARSER_H
#define STATICTRIGCOMPONENTPARSER_H
#include "Sections.h"
#include "Basics.h"

class StaticTrigComponentParser
{
    public:

        virtual ~StaticTrigComponentParser();
        bool ParseNumericComparison(char* text, Chk::Condition::Comparison & dest, s64 pos, s64 end); // At least, at most, exactly
        bool ParseSwitchState(char* text, Chk::Condition::Comparison & dest, s64 pos, s64 end); // Set, cleared
        bool ParseSpecialUnitAmount(char* text, u8 & dest, s64 pos, s64 end); // All
        bool ParseAllianceStatus(char* text, u16 & dest, s64 pos, s64 end); // Ally, Enemy, Allied Victory
        bool ParseResourceType(char* text, u8 & dest, s64 pos, s64 end); // Ore, Gas, Ore and Gas
        bool ParseScoreType(char* text, u8 & dest, s64 pos, s64 end); // Total, Units, Buildings, Units and Buildings, Kill, Razings, Kill and Razing, Custom
        bool ParseTextDisplayFlag(char* text, u8 & dest, s64 pos, s64 end); // Always Display, Don't Always Display
        bool ParseNumericModifier(char* text, u8 & dest, s64 pos, s64 end); // Add, subtract, set to
        bool ParseSwitchMod(char* text, u8 & dest, s64 pos, s64 end); // Set, clear, toggle, randomize
        bool ParseStateMod(char* text, u8 & dest, s64 pos, s64 end); // Disable, Enable, Toggle
        bool ParseOrder(char* text, u8 & dest, s64 pos, s64 end); // Attack, move, patrol
        bool ParseMemoryAddress(char* text, u32 & dest, s64 pos, s64 end, u32 deathTableOffset);

        bool ParseResourceType(char* text, u16 & dest, s64 pos, s64 end); // Accelerator for 2-byte resource types
        bool ParseScoreType(char* text, u16 & dest, s64 pos, s64 end); // Accelerator for 2-byte score types

        bool ParseBinaryLong(char* text, u32 & dest, s64 pos, s64 end); // Grabs a 4-byte binary int from the given position in the 
        bool ParseLong(char* text, u32 & dest, s64 pos, s64 end); // Grabs a 4-byte int from the given position in the text
        bool ParseTriplet(char* text, u8* dest, s64 pos, s64 end); // Grabs a 3-byte int from the given position in the text
        bool ParseShort(char* text, u16 & dest, s64 pos, s64 end); // Grabs a 2-byte int from the given position in the text
        bool ParseByte(char* text, u8 & dest, s64 pos, s64 end); // Grabs a 1-byte int from the given position in the text
};

#endif