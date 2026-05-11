#ifndef _EVENTDATATYPES_H_
#define _EVENTDATATYPES_H_

#include "types.h"
#include "NL/nlMath.h"
#include "Game/Sys/eventman.h"

// Forward declarations
class cPlayer;
class cFielder;
class cBall;
class Bowser;
class ChainChomp;

// CharacterDirectionData has a vtbl but does NOT inherit from EventData per DWARF
class CharacterDirectionData
{
public:
    virtual u32 GetID();

    /* 0x04 */ nlVector3* home;
    /* 0x08 */ nlVector3* away;
};

struct PowerupData : public EventData
{
    PowerupData() { }
    virtual u32 GetID();

    /* 0x04 */ cFielder* pFielder;
    /* 0x08 */ float fAwardWorth;
};

struct PenaltyData : public EventData
{
    PenaltyData() { }
    virtual u32 GetID();

    /* 0x04 */ cFielder* pFouler;
    /* 0x08 */ cFielder* pFoulee;
    /* 0x0C */ float fPenaltyWorth;
};

struct ShotAtGoalData : public EventData
{
    ShotAtGoalData() { }
    virtual u32 GetID();

    /* 0x04 */ cPlayer* pShooter;
}; // total size: 0x8

struct PassBallData : public EventData
{
    virtual u32 GetID();

    /* 0x04 */ cPlayer* pPasser;
    /* 0x08 */ cPlayer* pTarget;
    /* 0x0C */ s32 mPasserControllerID;
}; // total size: 0x10

enum eReceiveBallResult
{
    RECEIVEBALL_LOOSE_PICKUP = 0,
    RECEIVEBALL_PASS_COMPLETE = 1,
    RECEIVEBALL_PASS_INTERCEPT = 2,
};

class ReceiveBallData : public EventData
{
public:
    ReceiveBallData() { }
    virtual u32 GetID();

    /* 0x04 */ cPlayer* pReceiver;
    /* 0x08 */ eReceiveBallResult eResult;
}; // total size: 0xC

class CollisionBobombData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ nlVector3 v3ExplosionLocation;
    /* 0x10 */ float fExplosionRadius;
    /* 0x14 */ cFielder* pThrower;
    /* 0x18 */ int nThrowerPadID;
    /* 0x1C */ bool bIsFreezeBomb;
}; // total size: 0x20

class CollisionPlayerBananaData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pPlayer;
    /* 0x08 */ cFielder* pThrower;
    /* 0x0C */ s32 nThrowerPadID;
    /* 0x10 */ nlVector3 v3CollisionLocation;
}; // total size: 0x1C

class CollisionBallShellData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ nlVector3 v3CollisionVelocity;
}; // total size: 0x10

class CollisionPlayerFreezeData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pPlayer;
    /* 0x08 */ cFielder* pThrower;
    /* 0x0C */ s32 nThrowerPadID;
    /* 0x10 */ int eSize;
}; // total size: 0x14

class CollisionPowerupStatsData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pThrower;
    /* 0x08 */ s32 nThrowerPadID;
}; // total size: 0x0C

class CollisionPlayerShellData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pPlayer;
    /* 0x08 */ cFielder* pThrower;
    /* 0x0C */ u8 nThrowerPadID;
    /* 0x0D */ bool bIsExploder;
    /* 0x10 */ int eSize;
    /* 0x14 */ nlVector3 v3CollisionLocation;
    /* 0x20 */ nlVector3 v3CollisionVelocity;
}; // total size: 0x2C

class CollisionBowserPlayerData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pFielder;
    /* 0x08 */ Bowser* pBowser;
}; // total size: 0xC

class CollisionChainPlayerData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pFielder;
    /* 0x08 */ ChainChomp* pChain;
}; // total size: 0xC

class CollisionPlayerShootToScoreBallData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pFielder;
    /* 0x08 */ cBall* pBall;
}; // total size: 0xC

class CollisionPlayerBallData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cPlayer* pPlayer;
    /* 0x08 */ cBall* pBall;
    /* 0x0C */ nlVector3 velocity;
    /* 0x18 */ int boneID;
}; // total size: 0x1C

class CollisionPlayerWallData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cPlayer* pPlayer;
    /* 0x08 */ nlVector3 contactPoint;
    /* 0x14 */ nlVector3 wallNormal;
}; // total size: 0x20

class CollisionBallWallData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cBall* pBall;
    /* 0x08 */ u8 bIsPerfect;
    /* 0x09 */ u8 bIsShot;
    /* 0x0C */ nlVector3 position;
    /* 0x18 */ nlVector3 normal;
    /* 0x24 */ float fCollisionVecLen;
}; // total size: 0x28

class CollisionBallGroundData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cBall* pBall;
    /* 0x08 */ u8 bIsShot;
    /* 0x0C */ nlVector3 position;
    /* 0x18 */ nlVector3 normal;
    /* 0x24 */ float fVecZComponent;
}; // total size: 0x28

class CollisionPlayerPlayerData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cPlayer* player1;
    /* 0x08 */ cPlayer* player2;
    /* 0x0C */ nlVector3 velocity1;
    /* 0x18 */ nlVector3 velocity2;
}; // total size: 0x24

#endif // _EVENTDATATYPES_H_
