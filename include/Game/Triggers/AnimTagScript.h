#ifndef _ANIMTAGSCRIPT_H_
#define _ANIMTAGSCRIPT_H_

#include "NL/nlSlotPool.h"
#include "Game/Inventory.h"
#include "Game/InterpreterCore.h"
#include "Game/SAnim.h"

class AnimTagScriptInterpreter;

struct AnimTagInfo
{
    /* 0x0 */ unsigned long Trigger;
    /* 0x4 */ unsigned long ScriptFuncOffset;
}; // total size: 0x8

struct AnimTagCBInfo
{
    /* 0x0 */ AnimTagScriptInterpreter* pAnimTagScript;
    /* 0x4 */ AnimTagInfo ScriptInfo;
}; // total size: 0xC

class AnimTagScriptInterpreter : private InterpreterCore
{
public:
    AnimTagScriptInterpreter(unsigned int stackSize)
        : InterpreterCore(stackSize)
        , m_AnimTagSlotPool(16, 16)
    {
        m_BytecodeCount = 0;
    }
    ~AnimTagScriptInterpreter();

    void AnimControllerCB(unsigned int);
    u8 SetupAnimationTriggers(const char*, cInventory<cSAnim>*);

    /* 0x24 */ BasicSlotPool<AnimTagCBInfo> m_AnimTagSlotPool; // size 0x18
    /* 0x3C */ void* m_ppBytecode[32];                         // size 0x80
    /* 0xBC */ int m_BytecodeCount;                            // size 0x4
}; // total size: 0xC0

// class nlBSearch<BinaryTriggerFile
// {
// public:
//     void ANIM_RECORD, unsigned long>(const unsigned long&, BinaryTriggerFile::ANIM_RECORD*, int);
// };

#endif // _ANIMTAGSCRIPT_H_
