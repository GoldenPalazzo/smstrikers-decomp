#include "Game/FE/feInput.h"

#include "NL/nlMemory.h"
#include "NL/platpad.h"
#include "types.h"
#include <string.h>
#include <math.h>

FEInput* g_pFEInput = nullptr;

struct FEPadData
{
    /* 0x00 */ float fButtonInitialDelay[12];
    /* 0x30 */ float fButtonRepeatRate[12];
    /* 0x60 */ float fButtonTimeSinceLastRepeat[12];
    /* 0x90 */ bool bIsPressed[12];
}; // total size: 0x9C

FEPadData g_aFEPadData[4];

/**
 * Offset/Address/Size: 0x10B4 | 0x8020FAC0 | size: 0x40
 */
void FEInput::Initialize()
{
    g_pFEInput = new ((u8*)nlMalloc(0x30, 8, false)) FEInput();
}

/**
 * Offset/Address/Size: 0x1050 | 0x8020FA5C | size: 0x64
 */
FEInput::FEInput()
{
    FORCE_DONT_INLINE;
    for (int i = 0; i < 4; i++)
    {
        m_nExclusiveInputSceneHashIDStack[i].m_pBaseSceneHandler = nullptr;
        m_nExclusiveInputSceneHashIDStack[i].m_customID = -1;
    }
    Reset();
}

/**
 * Offset/Address/Size: 0xFF4 | 0x8020FA00 | size: 0x5C
 */
void FEInput::Reset()
{
    FORCE_DONT_INLINE;
    m_bEnableInput[0] = true;
    m_bEnableInput[1] = true;
    m_bEnableInput[2] = true;
    m_bEnableInput[3] = true;
    m_InputLockDepth = 0;
    m_bInputAllowed = true;
    EnableAnalogToDPadMapping(FE_ALL_PADS, false);
    memset(g_aFEPadData, 0, 0x270);
}

/**
 * Offset/Address/Size: 0xF94 | 0x8020F9A0 | size: 0x60
 */
cGlobalPad* FEInput::GetGlobalPad(eFEINPUT_PAD pad) const
{
    if (pad >= 4)
    {
        return nullptr;
    }

    if (!m_bInputAllowed)
    {
        return nullptr;
    }

    if (!m_bEnableInput[pad])
    {
        return nullptr;
    }

    return cPadManager::GetPad(pad);
}

/**
 * Offset/Address/Size: 0xEF8 | 0x8020F904 | size: 0x9C
 */
bool FEInput::IsConnected(eFEINPUT_PAD pad)
{
    if (pad != FE_ALL_PADS)
    {
        if (m_bEnableInput[pad])
        {
            cGlobalPad* pPad = cPadManager::GetPad(pad);
            return pPad->IsConnected();
        }
        else
        {
            return false;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        cGlobalPad* pPad = cPadManager::GetPad(i);
        if (pPad->IsConnected())
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0xBE8 | 0x8020F5F4 | size: 0x310
 */
bool FEInput::IsPressed(eFEINPUT_PAD pad, int button, bool remap, eFEINPUT_PAD* pOutPad)
{
    if (m_bInputAllowed)
    {
        if (pad != FE_ALL_PADS)
        {
            if (!m_bEnableInput[pad])
            {
                return false;
            }
            cGlobalPad* pPad = cPadManager::GetPad(pad);
            bool ispressed = pPad->IsPressed(button, remap);
            return ispressed;
        }

        for (int i = 0; i < 4; i++)
        {
            if (IsPressed((eFEINPUT_PAD)i, button, remap, (eFEINPUT_PAD*)0))
            {
                if (pOutPad)
                {
                    *pOutPad = (eFEINPUT_PAD)i;
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x87C | 0x8020F288 | size: 0x36C
 * TODO: 95.2% match - register allocation off by 1 (r23-r26 vs r22-r25),
 *       compiler doesn't hoist g_aFEPadData base into register for outer loop
 */
bool FEInput::IsAutoPressed(eFEINPUT_PAD pad, int button, bool remap, eFEINPUT_PAD* pOutPad)
{
    if (m_bInputAllowed)
    {
        if (pad != FE_ALL_PADS)
        {
            if (!m_bEnableInput[pad])
            {
                return false;
            }

            cGlobalPad* pPad = cPadManager::GetPad(pad);
            bool ispressed = pPad->IsPressed(button, remap);

            int mappedButton;
            if (remap)
            {
                mappedButton = cPadManager::GetRemapArray()[button];
            }
            else
            {
                mappedButton = button;
            }
            u32 buttonIndex = GetButtonIndex(mappedButton);

            if (ispressed && !g_aFEPadData[pad].bIsPressed[buttonIndex])
            {
                ispressed = false;
            }

            return ispressed;
        }

        for (int i = 0; i < 4; i++)
        {
            if (IsAutoPressed((eFEINPUT_PAD)i, button, remap, (eFEINPUT_PAD*)0))
            {
                if (pOutPad)
                {
                    *pOutPad = (eFEINPUT_PAD)i;
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x7AC | 0x8020F1B8 | size: 0xD0
 */
bool FEInput::JustPressed(eFEINPUT_PAD pad, int button, bool remap, eFEINPUT_PAD* pOutPad)
{
    if (m_bInputAllowed)
    {
        if (pad != FE_ALL_PADS)
        {
            if (m_bEnableInput[pad])
            {
                cGlobalPad* pPad = cPadManager::GetPad(pad);
                return pPad->JustPressed(button, remap);
            }
            else
            {
                return false;
            }
        }

        for (int i = 0; i < 4; i++)
        {
            cGlobalPad* pPad = cPadManager::GetPad(i);
            if (pPad->JustPressed(button, remap))
            {
                if (pOutPad != nullptr)
                {
                    *pOutPad = (eFEINPUT_PAD)i;
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x6DC | 0x8020F0E8 | size: 0xD0
 */
bool FEInput::JustReleased(eFEINPUT_PAD pad, int button, bool remap, eFEINPUT_PAD* pOutPad)
{
    if (m_bInputAllowed)
    {
        if (pad != FE_ALL_PADS)
        {
            if (m_bEnableInput[pad])
            {
                cGlobalPad* pPad = cPadManager::GetPad(pad);
                return pPad->JustReleased(button, remap);
            }
            else
            {
                return false;
            }
        }

        for (int i = 0; i < 4; i++)
        {
            cGlobalPad* pPad = cPadManager::GetPad(i);
            if (pPad->JustReleased(button, remap))
            {
                if (pOutPad != nullptr)
                {
                    *pOutPad = (eFEINPUT_PAD)i;
                }
                return true;
            }
        }
    }
    return false;
}

/**
 * Offset/Address/Size: 0x6A4 | 0x8020F0B0 | size: 0x38
 */
void FEInput::EnableInputIfSceneHasFocus(BaseSceneHandler* pSceneHandler)
{
    int depth = m_InputLockDepth;
    if (depth == 0 || m_nExclusiveInputSceneHashIDStack[depth - 1].m_pBaseSceneHandler == pSceneHandler)
    {
        m_bInputAllowed = true;
        return;
    }

    m_bInputAllowed = false;
}

/**
 * Offset/Address/Size: 0x678 | 0x8020F084 | size: 0x2C
 */
void FEInput::PushExclusiveInputLock(BaseSceneHandler* pRequestingSceneHandler, int customID)
{
    m_nExclusiveInputSceneHashIDStack[m_InputLockDepth].m_pBaseSceneHandler = pRequestingSceneHandler;
    m_nExclusiveInputSceneHashIDStack[m_InputLockDepth].m_customID = customID;
    m_InputLockDepth++;
}

/**
 * Offset/Address/Size: 0x64C | 0x8020F058 | size: 0x2C
 */
void FEInput::PopExclusiveInputLock(BaseSceneHandler* pRequestingSceneHandler)
{
    m_InputLockDepth -= 1;
    InputLockEntry& entry = m_nExclusiveInputSceneHashIDStack[m_InputLockDepth];
    entry.m_pBaseSceneHandler = nullptr;
    entry.m_customID = -1;
}

/**
 * Offset/Address/Size: 0x61C | 0x8020F028 | size: 0x30
 */
bool FEInput::HasInputLock(BaseSceneHandler* pRequestingSceneHandler) const
{
    int depth = m_InputLockDepth;
    if (depth == 0)
    {
        return false;
    }

    return m_nExclusiveInputSceneHashIDStack[depth - 1].m_pBaseSceneHandler == pRequestingSceneHandler;
}

/**
 * Offset/Address/Size: 0x494 | 0x8020EEA0 | size: 0x188
 * TODO: 99.4% match - register allocation differences (r29/r31 swapped for pad base vs byte cursor)
 */
void FEInput::Update(float dt)
{
    FEPadData* padData;
    float* pFloat;
    char* pByte;

    for (int padIndex = 0; padIndex < 4; padIndex++)
    {
        padData = &g_aFEPadData[padIndex];
        pByte = (char*)padData;
        pFloat = (float*)padData;

        for (int buttonMask = 1, buttonIndex = 0; buttonIndex < 12; buttonIndex++, buttonMask <<= 1, pByte++, pFloat++)
        {
            pByte[0x90] = false;

            if (cPadManager::GetPad(padIndex)->IsPressed(buttonMask, false))
            {
                if (pFloat[0x60 / 4] != 0.0f)
                {
                    float buttonStateTime = cPadManager::GetPad(padIndex)->GetButtonStateTime(buttonMask, false);
                    float diff = buttonStateTime - pFloat[0];
                    bool bShouldRepeat = true;
                    float timeSinceRepeat = pFloat[0x60 / 4];
                    float repeatRate = pFloat[0x30 / 4];

                    if (!(diff > 0.0001f) && !((float)fabs(diff) <= 0.0001f))
                    {
                        bShouldRepeat = false;
                    }

                    if (bShouldRepeat)
                    {
                        float diff2 = buttonStateTime - timeSinceRepeat - repeatRate;
                        bool bShouldRepeat2 = true;

                        if (!(diff2 > 0.0001f) && !((float)fabs(diff2) <= 0.0001f))
                        {
                            bShouldRepeat2 = false;
                        }

                        if (bShouldRepeat2)
                        {
                            pFloat[0x60 / 4] = buttonStateTime;
                            pByte[0x90] = true;
                        }
                    }
                }
                else
                {
                    pFloat[0x60 / 4] = cPadManager::GetPad(padIndex)->GetButtonStateTime(buttonMask, false);
                    pByte[0x90] = true;
                }
            }
            else
            {
                pFloat[0x60 / 4] = 0.0f;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x274 | 0x8020EC80 | size: 0x220
 * TODO: 94.0% match - recursive call inlining causes register allocation differences
 */
void FEInput::SetAutoRepeatParams(eFEINPUT_PAD pad, int button, float initialdelay, float repeatrate)
{
    u32 buttonIndex = GetButtonIndex(cPadManager::GetRemapArray()[button]);

    if (pad == FE_ALL_PADS)
    {
        for (int i = 0; i < 4; i++)
        {
            SetAutoRepeatParams((eFEINPUT_PAD)i, button, initialdelay, repeatrate);
        }
    }
    else
    {
        g_aFEPadData[pad].fButtonInitialDelay[buttonIndex] = initialdelay;
        g_aFEPadData[pad].fButtonRepeatRate[buttonIndex] = repeatrate;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8020EA0C | size: 0x274
 */
void FEInput::EnableAnalogToDPadMapping(eFEINPUT_PAD pad, bool enable)
{
    // FORCE_DONT_INLINE;
    if (pad == FE_ALL_PADS)
    {
        for (int i = 0; i < 4; i++)
        {
            EnableAnalogToDPadMapping((eFEINPUT_PAD)i, enable);
        }
    }
    else
    {
        if (enable)
        {
            cPadManager::GetPad(pad)->EnableLeftAnalogToDPadMap();
        }
        else
        {
            cPadManager::GetPad(pad)->DisableLeftAnalogToDPadMap();
        }
    }
}
