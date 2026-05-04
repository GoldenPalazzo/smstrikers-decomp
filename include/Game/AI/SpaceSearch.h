#ifndef _SPACESEARCH_H_
#define _SPACESEARCH_H_

#include "NL/nlMath.h"
#include "Game/Player.h"
#include "Game/Team.h"
#include "Game/Goalie.h"

enum eFieldDirection
{
    DIR_NONE = 0,
    DIR_UPFIELD = 1,
    DIR_DOWNFIELD = 2,
    DIR_TOWARD_TARGET = 3,
    DIR_AWAYFROM_TARGET = 4,
    DIR_CUSTOM = 5,
    NUM_DIRECTIONS = 6,
};

float CalcIdealShootingPositionScore(const nlVector3&, const nlVector3&, const nlVector2&, float);

class SpaceSearch
{
public:
    SpaceSearch(cPlayer* pPlayer)
    {
        m_fNetDirection = pPlayer->m_pTeam->m_pNet->m_sideSign;
        m_bDebugOn = false;
        m_bDrawSearchSpace = false;
    }
    SpaceSearch(float fNetDirection)
    {
        m_fNetDirection = fNetDirection;
        m_bDebugOn = false;
        m_bDrawSearchSpace = false;
    }
    virtual ~SpaceSearch();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short) = 0;
    float FindBestPosition(nlVector3&, const nlVector3&, eFieldDirection, const nlVector3*, float, unsigned short);

    /* 0x4 */ float m_fNetDirection;
    /* 0x8 */ bool m_bDebugOn;
    /* 0x9 */ bool m_bDrawSearchSpace;
    /* 0xC */ float m_unk_0x0C; // TODO: figure out what this is
};

class SSearchCutAndBreak : public SpaceSearch
{
public:
    SSearchCutAndBreak(cPlayer*);
    virtual ~SSearchCutAndBreak();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short);

    /* 0x10 */ cPlayer* m_pPlayer;
}; // total size: 0x10

class SSearchOpenLane : public SpaceSearch
{
public:
    SSearchOpenLane(cPlayer*, cPlayer*);
    virtual ~SSearchOpenLane();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short);

    /* 0x10 */ cPlayer* m_pBallOwner;
    /* 0x14 */ cPlayer* m_pPassTarget;
    /* 0x18 */ nlVector3 m_v3OtherPos;
    /* 0x24 */ bool m_bOtherPosIsTarget;
}; // total size: 0x24

class SSearchIdealShot : public SpaceSearch
{
public:
    SSearchIdealShot(cPlayer* pBallOwner)
        : SpaceSearch(pBallOwner->m_pTeam->m_pNet->m_sideSign)
        , m_SSearchOpenLane(pBallOwner, NULL)
    {
        m_pGoalie = pBallOwner->m_pTeam->GetOtherTeam()->GetGoalie();
    }
    virtual ~SSearchIdealShot();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short);

    /* 0x10 */ Goalie* m_pGoalie;
    /* 0x14 */ SSearchOpenLane m_SSearchOpenLane;
}; // total size: 0x34

class SSearchRunToNet : public SpaceSearch
{
public:
    SSearchRunToNet(cPlayer*);
    virtual ~SSearchRunToNet();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short);

    /* 0x10 */ SSearchIdealShot m_SSearchIdealShot;
}; // total size: 0x40

class SSearchBestPass : public SpaceSearch
{
public:
    SSearchBestPass(cPlayer*, cPlayer*, bool, bool);
    virtual ~SSearchBestPass();
    virtual float EvaluatePosition(const nlVector3&, const nlVector3&, eFieldDirection, unsigned short);

    /* 0x10 */ SSearchIdealShot m_SSearchIdealShot;
    /* 0x4C */ SSearchOpenLane m_SSearchOpenLane;
    /* 0x74 */ bool m_bAllowLeadPass;
    /* 0x75 */ bool m_bIsPerfectPass;
}; // total size: 0x78

#endif // _SPACESEARCH_H_
