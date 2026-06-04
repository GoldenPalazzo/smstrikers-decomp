#ifndef _DEFAULTDEFENSIVE_H_
#define _DEFAULTDEFENSIVE_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/Team.h"

class cFielder;

namespace Fuzzy
{
FuzzyVariant AbortDefencePlay(cDecisionEntity*);
FuzzyVariant DefaultDefencePlay(cDecisionEntity*);
FuzzyVariant DefendPassInPlay(float, cDecisionEntity*);
FuzzyVariant TryAttacking(float, cDecisionEntity*);
FuzzyVariant AttackBallOwner(float, cDecisionEntity*);
FuzzyVariant UsePowerupDefensive(float, cDecisionEntity*);
FuzzyVariant GetPowerupTargetDefensive(cTeam*);
FuzzyVariant InGoodWindupPosition(cFielder*);
} // namespace Fuzzy

#endif // _DEFAULTDEFENSIVE_H_
