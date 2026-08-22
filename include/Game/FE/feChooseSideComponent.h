#ifndef _FECHOOSESIDECOMPONENT_H_
#define _FECHOOSESIDECOMPONENT_H_

#include "Game/FE/feCaptainComponent.h"
#include "Game/FE/feInput.h"
#include "Game/FE/tlInstance.h"
#include "Game/FE/feTweener.h"

enum Context
{
    CONTEXT_INVALID = -1,
    CONTEXT_FE = 0,
    CONTEXT_PAUSE = 1,
};

class IChooseSide
{
public:
    IChooseSide();
    ~IChooseSide();
    UpdateResult Update(float dt, eFEINPUT_PAD* pad, int param);
    UpdateResult UpdateForFE(float padresult, eFEINPUT_PAD* pad);
    UpdateResult UpdateForPause(float padresult, eFEINPUT_PAD* pad);
    void CheckControllers(int disabledSide);
    void ResetAndPositionControllers(bool reset);
    void SetReady(int controllerIdx, bool ready);
    void PositionController(int padindex, bool usetween, bool setvisibilities);
    bool AllPlayersReady() const;
    bool AllPluggedInAreReady() const;
    bool AtLeastOnePlayerReady() const;
    bool AllControllersAreCentred() const;
    static void TweenSetPosCallback(void* obj, const float* value);
    void SaveChanges();

    /* 0x00 */ int mPlayingSides[4]; // size 0x10
    /* 0x10 */ bool mPlayerReady[4]; // size 0x4
#if defined(VERSION_G4QJ01)
    /* 0x14 */ TLInstance* mInstanceTable[21]; // size 0x54
    /* 0x68 */ float mControllerDestPos[3];    // size 0xC
    /* 0x74 */ FETweenManager mTweenManager;   // size 0x38
    /* 0xAC */ Context mContext;               // size 0x4
#else
    /* 0x14 */ TLInstance* mInstanceTable[17]; // size 0x44
    /* 0x58 */ float mControllerDestPos[3];    // size 0xC
    /* 0x64 */ FETweenManager mTweenManager;   // size 0x38
    /* 0x9C */ Context mContext;               // size 0x4
#endif
}; // total size: 0xA0 (G4QE01), 0xB0 (G4QJ01)

#endif // _FECHOOSESIDECOMPONENT_H_
