#ifndef _ROOTSCRIPT_H_
#define _ROOTSCRIPT_H_

#include "Game/AI/DecisionEntity.h"
#include "Game/AI/FuzzyVariant.h"

// class SaveConfidence
// {
// public:
//     SaveConfidence(float* pFloat)
//         : m_savedValue(*pFloat)
//         , m_pFloat(pFloat)
//     {
//     }

//     ~SaveConfidence();

//     float m_savedValue;
//     float* m_pFloat;
// };

FuzzyVariant StrategyChoosePlay(cDecisionEntity* pDecision);
int GetNumDecisionEntities();

#endif // _ROOTSCRIPT_H_
