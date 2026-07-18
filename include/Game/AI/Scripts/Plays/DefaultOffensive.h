#ifndef _DEFAULTOFFENSIVE_H_
#define _DEFAULTOFFENSIVE_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/Team.h"

class cPlayer;

class Fuzzy
{
public:
    static FuzzyVariant AbortOffensivePlay(cDecisionEntity*);
    static FuzzyVariant DefaultOffensivePlay(cDecisionEntity*);
    static FuzzyVariant DoPassing(float, cDecisionEntity*);
    static FuzzyVariant GetBestPassTarget(cPlayer*);
    static FuzzyVariant GetStrategicBallCarrier(cTeam*);
    static FuzzyVariant GoodBallCarrier(cFielder*);
    static FuzzyVariant GoodToShoot(cFielder*);
    static FuzzyVariant GoodToChipShot(cFielder*);
    static FuzzyVariant InGoodWindupPosition(cFielder*);
    static FuzzyVariant InDanger(cFielder*);
    static FuzzyVariant CutAndBreak(cFielder*);
    static FuzzyVariant DoShooting(float, cDecisionEntity*);
    static FuzzyVariant FurthestBackOnMyTeam(cFielder*);
    static FuzzyVariant UsePowerupOffensive(float, cDecisionEntity*);
    static FuzzyVariant GetPowerupTargetOffensive(cTeam*);
    static FuzzyVariant InDangerDelayed(cFielder*);
};

#endif // _DEFAULTOFFENSIVE_H_
