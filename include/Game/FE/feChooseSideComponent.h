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
    UpdateResult Update(float, eFEINPUT_PAD*, int);
    UpdateResult UpdateForFE(float, eFEINPUT_PAD*);
    UpdateResult UpdateForPause(float, eFEINPUT_PAD*);
    void CheckControllers(int);
    void ResetAndPositionControllers(bool);
    void SetReady(int, bool);
    void PositionController(int, bool, bool);
    bool AllPlayersReady() const;
    bool AllPluggedInAreReady() const;
    bool AtLeastOnePlayerReady() const;
    bool AllControllersAreCentred() const;
    static void TweenSetPosCallback(void*, const float*);
    void SaveChanges();

    /* 0x00 */ int mPlayingSides[4];           // size 0x10
    /* 0x10 */ bool mPlayerReady[4];           // size 0x4
    /* 0x14 */ TLInstance* mInstanceTable[17]; // size 0x44
    /* 0x58 */ float mControllerDestPos[3];    // size 0xC
    /* 0x64 */ FETweenManager mTweenManager;   // size 0x38
    /* 0x9C */ Context mContext;               // size 0x4
}; // total size: 0xA0

#endif // _FECHOOSESIDECOMPONENT_H_
