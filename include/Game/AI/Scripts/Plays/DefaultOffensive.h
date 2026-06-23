#ifndef _DEFAULTOFFENSIVE_H_
#define _DEFAULTOFFENSIVE_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/Team.h"

class cPlayer;

template <typename T>
nlVector3& PositionOf(T pObject)
{
    return pObject->m_v3Position;
}

class Fuzzy
{
public:
    static FuzzyVariant AbortOffensivePlay(cDecisionEntity*);
    void DefaultOffensivePlay(cDecisionEntity*);
    static FuzzyVariant DoPassing(float, cDecisionEntity*);
    static FuzzyVariant GetBestPassTarget(cPlayer*);
    static FuzzyVariant GoodBallCarrier(cFielder*);
    static FuzzyVariant GoodToChipShot(cFielder*);
    static FuzzyVariant InGoodWindupPosition(cFielder*);
    static FuzzyVariant InDanger(cFielder*);
    static FuzzyVariant CutAndBreak(cFielder*);
    void DoShooting(float, cDecisionEntity*);
    static FuzzyVariant FurthestBackOnMyTeam(cFielder*);
    static FuzzyVariant UsePowerupOffensive(float, cDecisionEntity*);
    static FuzzyVariant GetPowerupTargetOffensive(cTeam*);
    static FuzzyVariant InDangerDelayed(cFielder*);
};

#endif // _DEFAULTOFFENSIVE_H_
