#ifndef _DEFAULTDEFENSIVE_H_
#define _DEFAULTDEFENSIVE_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/Team.h"

class cFielder;

class Fuzzy
{
public:
    static FuzzyVariant AbortDefencePlay(cDecisionEntity*);
    static FuzzyVariant DefaultDefencePlay(cDecisionEntity*);
    static FuzzyVariant DefendPassInPlay(float, cDecisionEntity*);
    static FuzzyVariant TryAttacking(float, cDecisionEntity*);
    static FuzzyVariant AttackBallOwner(float, cDecisionEntity*);
    static FuzzyVariant UsePowerupDefensive(float, cDecisionEntity*);
    static FuzzyVariant GetPowerupTargetDefensive(cTeam*);
    static FuzzyVariant InGoodWindupPosition(cFielder*);
};

#endif // _DEFAULTDEFENSIVE_H_
