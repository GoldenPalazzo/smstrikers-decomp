#ifndef _COMMONSCRIPT_H_
#define _COMMONSCRIPT_H_

#include "Game/AI/Fuzzy.h"
#include "Game/AI/FuzzyVariant.h"

class cTeam;
class cPlayer;
class cFielder;

namespace Fuzzy
{
FuzzyVariant GetStrategicBallCarrier(cTeam*);
FuzzyVariant GetBestBallInterceptor(cTeam*);
FuzzyVariant GetSwapControllerScore(cPlayer*);
FuzzyVariant ShouldIStrafeBall(cFielder*);
FuzzyVariant ShouldIStrafeMark(cFielder*);
FuzzyVariant ShouldIMarkBallOwner(cFielder*);
FuzzyVariant ShouldIAttemptOneTimer(cFielder*);
FuzzyVariant GetBestLooseBallPassTarget(cFielder*);
FuzzyVariant GetBestPassTarget(cPlayer*);
FuzzyVariant GoodPassTargetFrom(cFielder*, cFielder*);
FuzzyVariant GetBestHitTarget(cFielder*);
FuzzyVariant GetPassDirection(cPlayer*, cPlayer*);
FuzzyVariant GoodToShoot(cFielder*);
FuzzyVariant GoodToChipShot(cFielder*);
FuzzyVariant GetBestPassReceiveAction(cFielder*);
FuzzyVariant GetBestLooseBallAction(cFielder*);
FuzzyVariant GetBestWindupShotAction(cFielder*);
FuzzyVariant GetPowerupToUseForPassReceiveDefence(cFielder*);
FuzzyVariant GetPowerupToUseForWindupDefence(cFielder*);
FuzzyVariant InDanger(cFielder*);
FuzzyVariant InDangerDelayed(cFielder*);
FuzzyVariant GoalieAndGonnaPickupBall(cPlayer*);
} // namespace Fuzzy

#endif // _COMMONSCRIPT_H_
