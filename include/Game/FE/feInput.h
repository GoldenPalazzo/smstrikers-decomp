#ifndef _FEINPUT_H_
#define _FEINPUT_H_

#include "types.h"
#include "NL/globalpad.h"

class BaseSceneHandler;

enum eFEINPUT_PAD
{
    FE_PAD1_ID = 0,
    FE_PAD2_ID = 1,
    FE_PAD3_ID = 2,
    FE_PAD4_ID = 3,
    FE_PAD5_ID = 4,
    FE_PAD6_ID = 5,
    FE_PAD7_ID = 6,
    FE_PAD8_ID = 7,
    FE_ALL_PADS = 8,
};

struct InputLockEntry
{
    /* 0x00 */ BaseSceneHandler* m_pBaseSceneHandler;
    /* 0x04 */ int m_customID;
}; // total size: 0x8

struct FEInput
{
    // !! this needs to be at the top of the class, otherwise fields to not map correctly to their needed offsets
    /* 0x00 */ InputLockEntry m_nExclusiveInputSceneHashIDStack[4];
    /* 0x20 */ int m_InputLockDepth;
    /* 0x24 */ bool m_bEnableInput[4];
    /* 0x28 */ bool m_bInputAllowed;

    FEInput();
    virtual ~FEInput() { };

    void EnableAnalogToDPadMapping(eFEINPUT_PAD, bool);
    void SetAutoRepeatParams(eFEINPUT_PAD, int, float, float);
    void Update(float);
    bool HasInputLock(BaseSceneHandler*) const;
    void PopExclusiveInputLock(BaseSceneHandler*);
    void PushExclusiveInputLock(BaseSceneHandler*, int);
    void EnableInputIfSceneHasFocus(BaseSceneHandler*);
    bool JustReleased(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
    bool JustPressed(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
    bool IsAutoPressed(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
    bool IsPressed(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
    bool IsConnected(eFEINPUT_PAD);
    cGlobalPad* GetGlobalPad(eFEINPUT_PAD) const;
    void Reset();

    static int* m_pRemapArray;
    static void Initialize();
}; // namespace FEInput

extern FEInput* g_pFEInput;

#endif // _FEINPUT_H_
