#ifndef TEXTTRIGCOMPILER_H
#define TEXTTRIGCOMPILER_H
#include "Basics.h"
#include "StaticTrigComponentParser.h"
#include "Sc.h"
#include "Scenario.h"
#include <unordered_map>
#include <sstream>
#include <string>

struct LocationTableNode {
    RawString locationName;
    u8 locationId;
};
struct UnitTableNode {
    RawString unitName;
    Sc::Unit::Type unitType;
};
struct SwitchTableNode {
    RawString switchName;
    u8 switchId;
};
struct WavTableNode {
    RawString wavName;
    u32 wavId;
};
struct GroupTableNode {
    RawString groupName;
    u32 groupId;
};
struct ScriptTableNode {
    RawString scriptName;
    u32 scriptId;
};

constexpr u32 maxErrorMessageSize = 512;

class TextTrigCompiler : public StaticTrigComponentParser
{
    public:

        TextTrigCompiler(bool useAddressesForMemory, u32 deathTableOffset);
        virtual ~TextTrigCompiler();
        bool CompileTriggers(std::string trigText, ScenarioPtr chk, Sc::Data & scData); // Compiles text, overwrites TRIG and STR upon success
        bool CompileTriggers(buffer & text, ScenarioPtr chk, Sc::Data & scData); // Compiles text, overwrites TRIG and STR upon success
        bool CompileTrigger(std::string trigText, Chk::Trigger* trigger, ScenarioPtr chk, Sc::Data & scData); // Compiles text, fills trigger upon success
        bool CompileTrigger(buffer & text, Chk::Trigger* trigger, ScenarioPtr chk, Sc::Data & scData); // Compiles text, fills trigger upon success

        // Attempts to compile the argNum'th condition argument into the given condition
        bool ParseConditionName(std::string text, Chk::Condition::Type & conditionType);
        bool ParseConditionArg(std::string conditionArgText, u8 argNum, std::vector<u8> & argMap, Chk::Condition & condition, ScenarioPtr chk, Sc::Data & scData);
        bool ParseActionName(std::string text, Chk::Action::Type & actionType);
        bool ParseActionArg(std::string actionArgText, u8 argNum, std::vector<u8> & argMap, Chk::Action & action, ScenarioPtr chk, Sc::Data & scData);
        static u8 defaultConditionFlags(Chk::Condition::Type conditionType);
        static u8 defaultActionFlags(Chk::Action::Type actionType);
        static u8 numConditionArgs(Chk::Condition::VirtualType conditionType);
        static u8 numActionArgs(Chk::Action::VirtualType actionType);


    protected:

        bool LoadCompiler(ScenarioPtr chk, Sc::Data & scData); // Sets up all the data needed for a run of the compiler
        void ClearCompiler(); // Clears data loaded for a run of the compiler
        void CleanText(buffer & text); // Remove spacing and standardize line endings

        bool ParseTriggers(buffer & text, std::deque<std::shared_ptr<Chk::Trigger>> & output, std::stringstream & error); // Parse trigger, generate a trig section in buffer output
        inline bool ParsePartZero(buffer & text, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);
        inline bool ParsePartOne(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting, s64 & playerEnd, s64 & lineEnd);
        inline bool ParsePartTwo(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);
        inline bool ParsePartThree(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);
        inline bool ParsePartFour(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting,
            s64 & conditionEnd, s64 & lineEnd, Chk::Condition::VirtualType & conditionType, u8 & flags, u32 & argsLeft, u32 & numConditions,
            Chk::Condition* & currCondition);
        inline bool ParsePartFive(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting, u32 & argsLeft, s64 & argEnd,
            Chk::Condition* & currCondition, Chk::Condition::VirtualType & conditionType);
        inline bool ParsePartSix(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);
        inline bool ParsePartSeven(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting,
            u8 & flags, s64 & actionEnd, s64 & lineEnd, Chk::Action::VirtualType & actionType, u32 & argsLeft, u32 & numActions,
            Chk::Action* & currAction);
        inline bool ParsePartEight(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting,
            u32 & argsLeft, s64 & argEnd, Chk::Action* & currAction, Chk::Action::VirtualType & actionType);
        inline bool ParsePartNine(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);
        inline bool ParsePartTen(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting,
            s64 & flagsEnd);
        inline bool ParsePartEleven(buffer & text, Chk::Trigger & output, std::stringstream & error, s64 & pos, u32 & line, u32 & expecting);

        bool ParseExecutingPlayer(buffer & text, Chk::Trigger & currTrig, s64 pos, s64 end); // Parse a player that the trigger is executed by
        bool ParseConditionName(buffer & arg, Chk::Condition::VirtualType & conditionType);
        bool ParseCondition(buffer & text, s64 pos, s64 end, bool disabled, Chk::Condition::VirtualType & conditionType, u8 & flags, u32 & argsLeft); // Find the equivilant conditionType
        bool ParseActionName(buffer & arg, Chk::Action::VirtualType & actionType);
        bool ParseAction(buffer & text, s64 pos, s64 end, bool disabled, Chk::Action::VirtualType & actionType, u8 & flags, u32 & argsLeft); // Find the equivilant actionType
        bool ParseConditionArg(buffer & text, Chk::Condition & currCondition, s64 pos, s64 end, Chk::Condition::VirtualType conditionType, u32 argsLeft, std::stringstream & error); // Parse an argument belonging to a condition
        bool ParseActionArg(buffer & text, Chk::Action & currAction, s64 pos, s64 end, Chk::Action::VirtualType actionType, u32 argsLeft, std::stringstream & error); // Parse an argument belonging to an action
        bool ParseExecutionFlags(buffer & text, s64 pos, s64 end, u32 & flags);

        bool zzParseString(buffer & text, u32 & dest, s64 pos, s64 end); // Find a given string (not an extended string) in the map, prepare to add it if necessary
        bool ParseLocationName(buffer & text, u32 & dest, s64 pos, s64 end); // Find a location in the map by its string
        bool ParseUnitName(buffer & text, Sc::Unit::Type & dest, s64 pos, s64 end); // Get a unitID using a unit name
        bool ParseWavName(buffer & text, u32 & dest, s64 pos, s64 end); // Find a wav in the map by its string, redundant? remove me?
        bool ParsePlayer(buffer & text, u32 & dest, s64 pos, s64 end); // Get a groupID using a group/player name
        bool ParseSwitch(buffer & text, u8 & dest, s64 pos, s64 end); // Find a switch in the map by name (or standard name)
        bool ParseScript(buffer & text, u32 & dest, s64 pos, s64 end); // Find a script by name

        bool ParseSwitch(buffer & text, u32 & dest, s64 pos, s64 end); // Accelerator for 4-byte switches

        Chk::Condition::Type ExtendedToRegularCID(Chk::Condition::VirtualType extendedConditionType); // Returns the conditionType the extended condition is based on
        Chk::Action::Type ExtendedToRegularAID(Chk::Action::VirtualType extendedActionType); // Returns the actionType the extended action is based on

        static s32 ExtendedNumConditionArgs(Chk::Condition::VirtualType extendedConditionType); // Returns the number of arguments for the extended condition
        static s32 ExtendedNumActionArgs(Chk::Action::VirtualType extendedActionType); // Returns the number of arguments for the extended action


    private:

        bool useAddressesForMemory; // If true, uses 1.16.1 addresses for memory conditions and actions
        u32 deathTableOffset;
        std::hash<std::string> strHash; // A hasher to help generate tables
        std::unordered_multimap<size_t, LocationTableNode> locationTable; // Binary tree of the maps locations
        std::unordered_multimap<size_t, UnitTableNode> unitTable; // Binary tree of the maps untis
        std::unordered_multimap<size_t, SwitchTableNode> switchTable; // Binary tree of the maps switches
        std::unordered_multimap<size_t, GroupTableNode> groupTable; // Binary tree of the maps groups
        std::unordered_multimap<size_t, ScriptTableNode> scriptTable; // Binary tree of map scripts
        std::unordered_multimap<size_t, zzStringTableNode> zzStringTable; // Binary tree of the maps strings
        std::unordered_multimap<size_t, zzStringTableNode> extendedStringTable; // Binary tree of the maps strings
        std::vector<zzStringTableNode> zzAddedStrings; // Forward list of strings added during compilation
        std::vector<zzStringTableNode> addedExtendedStrings; // Forward list of extended strings added during compilation
        std::bitset<Chk::MaxStrings> stringUsed; // Table of strings currently used in the map
        std::bitset<Chk::MaxStrings> extendedStringUsed; // Table of extended strings currently used in the map
        bool useNextString(u32 & index);
        bool useNextExtendedString(u32 & index);

        bool PrepLocationTable(ScenarioPtr map); // Fills locationTable
        bool PrepUnitTable(ScenarioPtr map); // Fills unitTable
        bool PrepSwitchTable(ScenarioPtr map); // Fills switchTable
        bool PrepGroupTable(ScenarioPtr map); // Fills groupTable
        bool PrepScriptTable(Sc::Data & scData); // Fills scriptTable
        bool PrepStringTable(ScenarioPtr map); // Fills stringUsed and stringTable
        bool PrepExtendedStringTable(ScenarioPtr map); // Fills extendedStringUsed and extendedStringTable

        bool BuildNewStringTable(ScenarioPtr chk, std::stringstream & error); // Builds a new STR section using stringTable and addedStrings
};

#endif