#include "Game/AI/ScriptAction.h"

#include "Game/AI/Fuzzy.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/AI/FielderDesires.h"

extern FuzzyVariant fvNotSet;

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
}

struct ScriptActionDestructorChain
{
    ScriptActionDestructorChain* next;
    void* destructor;
    void* object;
};

void ScriptActionSlotPoolDtor(void* obj, int)
{
    ((SlotPool<ScriptAction>*)obj)->~SlotPool<ScriptAction>();
}

// /**
//  * Offset/Address/Size: 0x68 | 0x800620BC | size: 0x64
//  */
// void SlotPool<ScriptAction>::~SlotPool()
// {
// }

SlotPool<ScriptAction> ScriptAction::m_ScriptActionSlotPool;

/**
 * Offset/Address/Size: 0x0 | 0x80062054 | size: 0x68
 * TODO: 99.23% match - relocation symbols differ for slot-pool destructor and
 * @200 registration chain.
 */
extern "C" void __sinit_ScriptAction_cpp()
{
    static ScriptActionDestructorChain chain;
    SlotPoolBase* pool = (SlotPoolBase*)&ScriptAction::m_ScriptActionSlotPool;

    __ct__12SlotPoolBaseFv(pool);
    pool->m_Initial = 0xA;
    SlotPoolBase::BaseAddNewBlock(pool, 0x80);
    pool->m_Delta = 0xA;
    __register_global_object(pool, (void*)ScriptActionSlotPoolDtor, &chain);
}

/**
 * Offset/Address/Size: 0xB4 | 0x80061E68 | size: 0x1EC
 */
ScriptAction::ScriptAction(eScriptActionType action_type, float fConfidence)
{
    m_sDesireParams.fDuration = 0.0f;
    m_sDesireParams.eDesireType = FIELDERDESIRE_NEED_DESIRE;
    m_sDesireParams.opt1 = fvNotSet;
    m_sDesireParams.opt2 = fvNotSet;

    m_sPlayParams.ePlayType = 0;
    m_sPlayParams.fDuration = 0.0f;

    m_eType = action_type;
    m_fConfidence = fConfidence;
    m_fSelectionChance = 1.0f;
}

/**
 * Offset/Address/Size: 0x64 | 0x80061E18 | size: 0x50
 */
float ScriptAction::CalcSelectionChance()
{
    float fChance;
    if (m_eType == SAT_SET_DESIRE)
    {
        fChance = GetCommonDesireData(m_sDesireParams.eDesireType).CalcFuzzyChance(m_fSelectionChance);
        return fChance;
    }
    fChance = RandomChance(m_fSelectionChance);
    return fChance;
}

/**
 * Offset/Address/Size: 0x0 | 0x80061DB4 | size: 0x64
 */
bool ScriptAction::RollChanceDice()
{
    if (m_eType == SAT_SET_DESIRE)
    {
        return GetCommonDesireData(m_sDesireParams.eDesireType).CalcBoolChance(m_fSelectionChance);
    }
    else
    {
        return RandomChance(m_fSelectionChance) >= 0.5f;
    }
}
