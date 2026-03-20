#include "NL/platpad.h"

// #include "global.h"

#include "math.h"
#include "NL/nlMemory.h"
#include "NL/gl/glPlat.h"

PADStatus PadStatus::s_A[4];
PADStatus PadStatus::s_B[4];
PADStatus* PadStatus::s_Current = PadStatus::s_A;
PADStatus* PadStatus::s_Next = PadStatus::s_B;

u32 g_nPadMasks[PAD_MAX_CONTROLLERS] = { 0x80000000, 0x40000000, 0x20000000, 0x10000000 };

bool cPlatPad::m_bDisableRumble = false;

namespace
{
PadStatus padCategories[2];
PadStatus* padStatus = &padCategories[0];
} // namespace

/**
 * Offset/Address/Size: 0x0 | 0x801C2FB0 | size: 0x5C
 */
cPlatPad::~cPlatPad()
{
}

/**
 * Offset/Address/Size: 0x5C | 0x801C300C | size: 0x524
 */
void VBlankPadUpdate()
{
    PADRead(PadStatus::s_Next);
    PADClampCircle(PadStatus::s_Next);

    int connectedControllerCount = 0;
    for (int i = 0; i < 4; i++)
    {
        if (PadStatus::s_Next[i].err != -3)
        {
            connectedControllerCount = i + 1;
        }
    }

    if (connectedControllerCount == 4)
    {
        return;
    }

    PADStatus* temp = PadStatus::s_Current;
    PadStatus::s_Current = PadStatus::s_Next;
    PadStatus::s_Next = temp;

    float frameTime = 1.0f / (float)glx_GetTargetFPS();

    for (int controllerIndex = 0; controllerIndex < 4; controllerIndex++)
    {
        cGlobalPad* pad = cPadManager::GetPad(controllerIndex);

        if (pad->m_isConnected == false)
        {
            continue;
        }

        float normalizedX = (float)PadStatus::s_Current[controllerIndex].stickX / 56.0f;
        float normalizedY = (float)PadStatus::s_Current[controllerIndex].stickY / 56.0f;

        float absX = fabs(normalizedX);
        float absY = fabs(normalizedY);
        bool hasInput = (absX > 0.1f) || (absY > 0.1f);

        if (hasInput)
        {
            if (absX < 0.1f)
                normalizedX = 0.0f;
            if (absY < 0.1f)
                normalizedY = 0.0f;

            float angle = nlATan2f(normalizedY, normalizedX);

            float degrees = angle * (180.0f / 3.1415927f);
            int direction = (int)(degrees / 45.0f + 0.5f) % 8;

            u16 directionFlags = 0;
            switch (direction)
            {
            case 0:
                directionFlags = 0x02;
                break;
            case 1:
                directionFlags = 0x0A;
                break;
            case 2:
                directionFlags = 0x08;
                break;
            case 3:
                directionFlags = 0x09;
                break;
            case 4:
                directionFlags = 0x01;
                break;
            case 5:
                directionFlags = 0x05;
                break;
            case 6:
                directionFlags = 0x04;
                break;
            case 7:
                directionFlags = 0x06;
                break;
            }

            PadStatus::s_Current[controllerIndex].button |= directionFlags;
        }

        for (int category = 0; category < 2; category++)
        {
            PadStatus* padStatus = &padCategories[category];
            tGameCubePad* gameCubePad = &padStatus->m_GameCubePads[controllerIndex];

            for (int buttonIndex = 0; buttonIndex < 12; buttonIndex++)
            {
                u16 buttonMask = 1 << buttonIndex;
                if (PadStatus::s_Current[controllerIndex].button & buttonMask)
                {
                    gameCubePad->fButtonStateTime[buttonIndex] += frameTime;
                }
                else
                {
                    gameCubePad->fButtonStateTime[buttonIndex] = 0.0f;
                    gameCubePad->fButtonTimeSinceLastRepeat[buttonIndex] = 0.0f;
                }
            }

            for (int buttonIndex = 12; buttonIndex < 16; buttonIndex++)
            {
                u16 buttonMask = 1 << buttonIndex;
                if (PadStatus::s_Current[controllerIndex].button & buttonMask)
                {
                    gameCubePad->fButtonInitialDelay[buttonIndex - 12] += frameTime;
                }
                else
                {
                    gameCubePad->fButtonInitialDelay[buttonIndex - 12] = 0.0f;
                    gameCubePad->fButtonRepeatRate[buttonIndex - 12] = 0.0f;
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x580 | 0x801C3530 | size: 0x40
 */
void cPlatPad::StopRumble()
{
    padStatus[0].m_GameCubePads[m_padIndex].bRumbleActive = 0;
    PADControlMotor(m_padIndex, 2);
}

/**
 * Offset/Address/Size: 0x5C0 | 0x801C3570 | size: 0x78
 */
void cPlatPad::StartRumble(float param1, float param2, float param3)
{
    if (m_bDisableRumble == 0)
    {
        PADControlMotor(m_padIndex, 1);
        padStatus[0].m_GameCubePads[m_padIndex].fRumbleTimer = param1;
        padStatus[0].m_GameCubePads[m_padIndex].bRumbleActive = 1;
    }
}

/**
 * Offset/Address/Size: 0x638 | 0x801C35E8 | size: 0x18
 */
bool cPlatPad::RumbleActive()
{
    return padStatus[0].m_GameCubePads[m_padIndex].bRumbleActive;
}

/**
 * Offset/Address/Size: 0x650 | 0x801C3600 | size: 0x18
 */
f32 cPlatPad::AnalogRightY()
{
    return padStatus[0].m_GameCubePads[m_padIndex].fAnalogRightY;
}

/**
 * Offset/Address/Size: 0x668 | 0x801C3618 | size: 0x18
 */
f32 cPlatPad::AnalogRightX()
{
    return padStatus[0].m_GameCubePads[m_padIndex].fAnalogRightX;
}

/**
 * Offset/Address/Size: 0x680 | 0x801C3630 | size: 0x18
 */
f32 cPlatPad::AnalogLeftY()
{
    return padStatus[0].m_GameCubePads[m_padIndex].fAnalogLeftY;
}

/**
 * Offset/Address/Size: 0x698 | 0x801C3648 | size: 0x14
 */
f32 cPlatPad::AnalogLeftX()
{
    return padStatus[0].m_GameCubePads[m_padIndex].fAnalogLeftX;
}

/**
 * Offset/Address/Size: 0x6AC | 0x801C365C | size: 0x8
 */
f32 cPlatPad::GetPressureDerivative(int, bool)
{
    return 0.f;
}

/**
 * Offset/Address/Size: 0x6B4 | 0x801C3664 | size: 0x84
 */
f32 cPlatPad::GetPressure(int button, bool remap)
{
    if (remap != 0)
    {
        button = cPadManager::m_pRemapArray[button];
    }

    switch (button)
    {
    case 0x40:
        return padStatus[0].m_GameCubePads[m_padIndex].fTriggerLeft;
    case 0x20:
        return padStatus[0].m_GameCubePads[m_padIndex].fTriggerRight;
    default:
        if ((PadStatus::s_Current[m_padIndex].button & button) != 0)
        {
            return 1.f;
        }
        return 0.f;
    }
}

/**
 * Offset/Address/Size: 0x738 | 0x801C36E8 | size: 0x3C
 */
f32 cPlatPad::GetButtonStateTime(int button, bool remap)
{
    if (remap)
    {
        button = cPadManager::m_pRemapArray[button];
    }

    return padStatus[0].m_GameCubePads[m_padIndex].fButtonStateTime[0x1F - __cntlzw(button)];
}

/**
 * Offset/Address/Size: 0x774 | 0x801C3724 | size: 0x3C
 */
bool cPlatPad::PlatJustReleased(int button, bool remap)
{
    if (remap)
    {
        button = cPadManager::m_pRemapArray[button];
    }

    s32 buttonReleased = padStatus[0].m_justReleased[m_padIndex] & button;
    return (buttonReleased != 0) ? true : false;
}

/**
 * Offset/Address/Size: 0x7B0 | 0x801C3760 | size: 0x3C
 */
bool cPlatPad::PlatJustPressed(int button, bool remap)
{
    if (remap)
    {
        button = cPadManager::m_pRemapArray[button];
    }

    s32 buttonPressed = padStatus[0].m_justPressed[m_padIndex] & button;
    return (buttonPressed != 0) ? true : false;
}

/**
 * Offset/Address/Size: 0x7EC | 0x801C379C | size: 0x38
 */
bool cPlatPad::IsPressed(int button, bool remap)
{
    if (remap != 0)
    {
        button = cPadManager::m_pRemapArray[button];
    }

    s32 buttonPressed = PadStatus::s_Current[m_padIndex].button & button;
    return (buttonPressed != 0) ? true : false;
}

/**
 * Offset/Address/Size: 0x824 | 0x801C37D4 | size: 0x34
 */
bool cPlatPad::IsConnected()
{
    bool isConnected = false;

    s8 err = PadStatus::s_Current[m_padIndex].err;
    if ((err == 0) || (err == -3))
    {
        isConnected = true;
    }

    return isConnected;
}

/**
 * Offset/Address/Size: 0x858 | 0x801C3808 | size: 0x270
 */
void PadStatus::Update(float dt)
{
    u32 resetMask = 0;

    for (s32 i = 0; i < 4; ++i)
    {
        s8 err = PadStatus::s_Current[i].err;

        if (err == 0)
        {
            tGameCubePad& pad = m_GameCubePads[i];
            pad.fAnalogLeftX = (f32)PadStatus::s_Current[i].stickX / 56.0f;
            pad.fAnalogLeftY = (f32)PadStatus::s_Current[i].stickY / 56.0f;
            pad.fAnalogRightX = (f32)PadStatus::s_Current[i].substickX / 44.0f;
            pad.fAnalogRightY = (f32)PadStatus::s_Current[i].substickY / 44.0f;
            pad.fTriggerLeft = (f32)(s32)PadStatus::s_Current[i].triggerLeft / 150.0f;
            pad.fTriggerRight = (f32)(s32)PadStatus::s_Current[i].triggerRight / 150.0f;

            u16 button = PadStatus::s_Current[i].button;
            m_justPressed[i] = button & ~m_previousButtons[i];
            m_justReleased[i] = m_previousButtons[i] & ~PadStatus::s_Current[i].button;
            m_previousButtons[i] = PadStatus::s_Current[i].button;
            m_previousErr[i] = PadStatus::s_Current[i].err;
        }
        else if (err == -1)
        {
            if (m_previousErr[i] == 0)
            {
                memset(&m_GameCubePads[i], 0, sizeof(tGameCubePad));
            }

            u32 mask = g_nPadMasks[i];
            int zero = 0;
            m_justPressed[i] = zero;
            resetMask |= mask;
            m_justReleased[i] = zero;
            m_previousButtons[i] = zero;
        }

        if (m_GameCubePads[i].bRumbleActive)
        {
            m_GameCubePads[i].fRumbleTimer -= dt;
            if (m_GameCubePads[i].fRumbleTimer < 0.0f)
            {
                m_GameCubePads[i].bRumbleActive = 0;
                PADControlMotor(i, 0);
            }
        }
    }

    if (resetMask)
    {
        PADReset(resetMask);
    }
}

/**
 * Offset/Address/Size: 0xAC8 | 0x801C3A78 | size: 0x24
 */
void UpdatePlatPad(float dt)
{
    ((PadStatus*)padStatus)->Update(dt);
}

static inline void InitPadStatus(PadStatus* pad)
{
    for (int i = 0; i < 4; ++i)
    {
        pad->m_justPressed[i] = 0;
        pad->m_justReleased[i] = 0;
        pad->m_previousButtons[i] = 0;
        pad->m_previousErr[i] = 0;
        memset(&pad->m_GameCubePads[i], 0, sizeof(tGameCubePad));
    }
}

/**
rm * Offset/Address/Size: 0xAEC | 0x801C3A9C | size: 0xB0
 */
void InitPlatPad()
{
    PADRead(PadStatus::s_Current);
    memcpy(PadStatus::s_Next, PadStatus::s_Next, 4);

    for (int i = 0; i < 2; ++i)
    {
        InitPadStatus(&padStatus[i]);
    }
}

/**
 * Offset/Address/Size: 0xB9C | 0x801C3B4C | size: 0xC
 */
u32 GetButtonIndex(int button)
{
    return 0x1F - __cntlzw(button);
}

/**
 * Offset/Address/Size: 0xBA8 | 0x801C3B58 | size: 0x10
 */
void UseDefaultPad()
{
    padStatus = &padCategories[0];
}

/**
 * Offset/Address/Size: 0xBB8 | 0x801C3B68 | size: 0x14
 */
void UseFixedUpdatePad()
{
    padStatus = &padCategories[1];
}
