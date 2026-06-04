#ifndef _GOALIELOOSEBALL_H_
#define _GOALIELOOSEBALL_H_

#include "NL/nlMath.h"
#include "Game/Player.h"

// bool LooseBallCallback(float, float, unsigned long, float, void*);

enum eLooseBallAnimType
{
    LOOSEBALL_ANIM_PICKUP = 0,
    LOOSEBALL_ANIM_KICK = 1,
    LOOSEBALL_ANIM_TRAP = 2,
    LOOSEBALL_ANIM_ATTACK = 3,
    LOOSEBALL_ANIM_SWAT = 4,
    NUM_LOOSEBALL_ANIM_TYPES = 5,
};

class LooseBallInfo
{
public:
    /* 0x00 */ nlVector3 mv3PickupPos;
    /* 0x0C */ int mnAnimID;
    /* 0x10 */ eLooseBallAnimType mAnimType;
    /* 0x14 */ float mfPickupDistance;
    /* 0x18 */ unsigned short maPickupAngle;
    /* 0x1C */ float mfPickupTime;
    /* 0x20 */ float mfAnimDuration;
}; // total size: 0x24

class LooseBallAnims
{
public:
    static void Init(cPlayer*);
    static void Destroy();
    static const LooseBallInfo* FindLooseBallAnim(const nlVector3&, bool);
    static const LooseBallInfo* GetSwatSTSInfo(int nSide);
    static const LooseBallInfo* GetDesperationInfo(unsigned int);

    static LooseBallInfo* mpLooseBallInfo;
    static unsigned int muNumLooseBallAnims;
    static LooseBallInfo mLooseBallKickInfo[3];
    static LooseBallInfo mTrapBallInfo;
    static LooseBallInfo mAttackSTSInfo;
    static LooseBallInfo mDiveBallInfo;
    static LooseBallInfo* mLooseBallDesperationInfo[4];
    static LooseBallInfo mSwatSTSInfo[2];
};

#endif // _GOALIELOOSEBALL_H_
