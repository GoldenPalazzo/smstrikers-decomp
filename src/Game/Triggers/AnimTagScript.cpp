#include "Game/Triggers/AnimTagScript.h"
#include "NL/nlMemory.h"

struct FILE_HEADER
{
    s8 Thumbprint[4];
    u16 Version;
    u16 AnimCount;
    u16 BytecodeOffset;
    u16 reserved;
};

class BinaryTriggerFile
{
public:
    struct ANIM_RECORD
    {
        u32 hash;
        u16 TriggerCount;
        u16 TriggerOffset;

        operator unsigned long() const { return hash; }
    };

    struct TRIGGER_RECORD
    {
        f32 Frame;
        u32 Trigger;
        u32 ScriptFuncOffset;
    };

    BinaryTriggerFile(const char*);
    u32 m_FileSize;
    FILE_HEADER* m_pFileData;
    ANIM_RECORD* m_pFirstAnim;
    TRIGGER_RECORD* m_pFirstTrigger;
    ANIM_RECORD* m_pCurrentAnim;
    u32 m_CurrentTrigger;
};

extern "C" void AnimControllerCB__24AnimTagScriptInterpreterFUi(unsigned int);

/**
 * Offset/Address/Size: 0x0 | 0x80214530 | size: 0x64
 */
void AnimTagScriptInterpreter::AnimControllerCB(unsigned int)
{
    InterpreterCore* pVtbl = *(InterpreterCore**)this;
    unsigned long address = (unsigned long)m_Header;
    if ((address + 0x10000) != 0xFFFF)
    {
        pVtbl->CallFunctionAt(address);
    }
    pVtbl->DoFunctionCall(m_Return);
}

/**
 * Offset/Address/Size: 0x0 | 0x80214354 | size: 0x1DC
 */
u8 AnimTagScriptInterpreter::SetupAnimationTriggers(const char* TriggerFileName, cInventory<cSAnim>* pAnimInventory)
{
    BinaryTriggerFile file(TriggerFileName);

    ListEntry<cSAnim*>* node = (ListEntry<cSAnim*>*)pAnimInventory->m_lItemList.m_Head;
    while (node != NULL)
    {
        u32 key = ((cSAnim*)node->data)->m_uHashID;
        file.m_pCurrentAnim = nlBSearch<BinaryTriggerFile::ANIM_RECORD, unsigned long>(key, file.m_pFirstAnim, file.m_pFileData->AnimCount);

        if (file.m_pCurrentAnim != NULL)
        {
            AnimTagCBInfo* pSlot;
            const BinaryTriggerFile::TRIGGER_RECORD* pTriggerRecord;
            file.m_CurrentTrigger = 0;
            while (pTriggerRecord = file.m_pFirstTrigger + file.m_pCurrentAnim->TriggerOffset + file.m_CurrentTrigger,
                file.m_CurrentTrigger < file.m_pCurrentAnim->TriggerCount)
            {
                pSlot = NULL;
                if (m_AnimTagSlotPool.m_FreeList == NULL)
                {
                    SlotPoolBase::BaseAddNewBlock(&m_AnimTagSlotPool, sizeof(AnimTagCBInfo));
                }
                if (m_AnimTagSlotPool.m_FreeList != NULL)
                {
                    pSlot = (AnimTagCBInfo*)m_AnimTagSlotPool.m_FreeList;
                    m_AnimTagSlotPool.m_FreeList = m_AnimTagSlotPool.m_FreeList->m_next;
                }

                pSlot->pAnimTagScript = this;
                pSlot->ScriptInfo.Trigger = pTriggerRecord->Trigger;
                pSlot->ScriptInfo.ScriptFuncOffset = pTriggerRecord->ScriptFuncOffset;

                ((cSAnim*)node->data)->CreateCallback(pTriggerRecord->Frame / (float)((cSAnim*)node->data)->m_nNumKeys, (unsigned int)pSlot, AnimControllerCB__24AnimTagScriptInterpreterFUi);

                file.m_CurrentTrigger++;
            }
        }

        node = node->next;
    }

    m_ppBytecode[m_BytecodeCount] = nlMalloc(file.m_FileSize - ((u32)((u8*)file.m_pFileData + file.m_pFileData->BytecodeOffset) - (u32)file.m_pFileData), 8, false);
    memcpy(m_ppBytecode[m_BytecodeCount], (u8*)file.m_pFileData + file.m_pFileData->BytecodeOffset, file.m_FileSize - ((u32)((u8*)file.m_pFileData + file.m_pFileData->BytecodeOffset) - (u32)file.m_pFileData));
    LoadByteCode(m_ppBytecode[m_BytecodeCount]);
    m_BytecodeCount++;

    nlFree(file.m_pFileData);
    return 0;
}
