#include "PreservedUnitStats.h"
#include "../../../../Chkdraft.h"
#include "../../../../Mapping/Undos/ChkdUndos/UnitChange.h"

PreservedUnitStats::PreservedUnitStats() : field(Chk::Unit::Field::ClassId)
{

}

PreservedUnitStats::~PreservedUnitStats()
{

}

void PreservedUnitStats::Clear()
{
    field = Chk::Unit::Field::ClassId;
    preservedValues.clear();
}

void PreservedUnitStats::AddStats(Selections & sel, Chk::Unit::Field statField)
{
    Clear();
    field = statField;
    auto & unitIndexes = sel.getUnits();
    for ( u16 & unitIndex : unitIndexes )
    {
        const Chk::Unit & unit = CM->getUnit(unitIndex);
        switch ( field )
        {
            case Chk::Unit::Field::ClassId: preservedValues.push_back(unit.classId); break;
            case Chk::Unit::Field::Xc: preservedValues.push_back(unit.xc); break;
            case Chk::Unit::Field::Yc: preservedValues.push_back(unit.yc); break;
            case Chk::Unit::Field::Type: preservedValues.push_back((u16)unit.type); break;
            case Chk::Unit::Field::RelationFlags: preservedValues.push_back(unit.relationFlags); break;
            case Chk::Unit::Field::ValidStateFlags: preservedValues.push_back(unit.validStateFlags); break;
            case Chk::Unit::Field::ValidFieldFlags: preservedValues.push_back(unit.validFieldFlags); break;
            case Chk::Unit::Field::Owner: preservedValues.push_back(unit.owner); break;
            case Chk::Unit::Field::HitpointPercent: preservedValues.push_back(unit.hitpointPercent); break;
            case Chk::Unit::Field::ShieldPercent: preservedValues.push_back(unit.shieldPercent); break;
            case Chk::Unit::Field::EnergyPercent: preservedValues.push_back(unit.energyPercent); break;
            case Chk::Unit::Field::ResourceAmount: preservedValues.push_back(unit.resourceAmount); break;
            case Chk::Unit::Field::HangerAmount: preservedValues.push_back(unit.hangerAmount); break;
            case Chk::Unit::Field::StateFlags: preservedValues.push_back(unit.stateFlags); break;
            case Chk::Unit::Field::Unused: preservedValues.push_back(unit.unused); break;
            case Chk::Unit::Field::RelationClassId: preservedValues.push_back(unit.relationClassId); break;
        }
    }
}

void PreservedUnitStats::convertToUndo()
{
    if ( preservedValues.size() > 0 )
    {
        // For each selected unit, add the corresponding undo from values
        u32 i = 0;

        Selections & selections = CM->GetSelections();
        auto unitChanges = ReversibleActions::Make();
        auto & selectedUnitIndexes = selections.getUnits();
        for ( u16 unitIndex : selectedUnitIndexes )
        {
            unitChanges->Insert(UnitChange::Make(unitIndex, field, preservedValues[i]));
            i++;
        }
        CM->AddUndo(unitChanges);
    }
    Clear();
}
