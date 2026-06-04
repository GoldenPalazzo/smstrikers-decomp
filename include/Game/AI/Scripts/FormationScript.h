#ifndef _FORMATIONSCRIPT_H_
#define _FORMATIONSCRIPT_H_

#include "Game/AI/FuzzyVariant.h"
#include "Game/Team.h"

namespace Fuzzy
{
FuzzyVariant GetStrategicBallCarrier(cTeam*);
FuzzyVariant GetBestBallInterceptor(cTeam*);
FuzzyVariant GetBestDefensiveFormation(cTeam*);
FuzzyVariant GetBestOffensiveFormation(cTeam*);
FuzzyVariant GetBestBallFormationSet(cTeam*);
} // namespace Fuzzy

#endif // _FORMATIONSCRIPT_H_
