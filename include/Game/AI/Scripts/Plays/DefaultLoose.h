#ifndef _DEFAULTLOOSE_H_
#define _DEFAULTLOOSE_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/FuzzyVariant.h"

class Fuzzy
{
public:
    FuzzyVariant AbortLoosePlay(cDecisionEntity*);
    FuzzyVariant DefaultLoosePlay(cDecisionEntity*);
};

#endif // _DEFAULTLOOSE_H_
