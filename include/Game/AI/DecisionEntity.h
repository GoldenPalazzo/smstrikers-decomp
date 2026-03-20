#ifndef _DECISIONENTITY_H_
#define _DECISIONENTITY_H_

#include "Game/AI/Fielder.h"
#include "Game/AI/FuzzyVariant.h"

// Forward declarations
class ScriptAction;
struct sDesireParams;
struct sPlayParams;

enum eDecisionEntity
{
    DECISION_ENTITY_STRATEGY = 0,
    DECISION_ENTITY_PLAY = 1,
    NUM_DECISION_ENTITIES = 2,
};

// void nlListRemoveElement<ScriptAction>(ScriptAction**, ScriptAction*, ScriptAction**);
// void nlListRemoveStart<ScriptAction>(ScriptAction**, ScriptAction**);
// void nlListAddEnd<ScriptAction>(ScriptAction**, ScriptAction**, ScriptAction*);
// void nlListAddStart<ScriptAction>(ScriptAction**, ScriptAction*, ScriptAction**);

class cDecisionEntity
{
public:
    cDecisionEntity() { };
    cDecisionEntity(eDecisionEntity, unsigned long, FuzzyVariant (*)(cDecisionEntity*), FuzzyVariant (*)(cDecisionEntity*));
    float CallDTF(cFielder*);
    bool DoAbort(cFielder*);
    sDesireParams& GetLastDesireParams();
    sPlayParams& GetLastPlayParams();
    ScriptAction* QueueActionSetDesire(int, float, float, FuzzyVariant, FuzzyVariant);
    ScriptAction* QueueActionSetPlay(int, float, float);
    ScriptAction* FindDesireAction(int, FuzzyVariant, FuzzyVariant);
    bool SelectAction(eScriptActionSelection, float);

    /* 0x00 */ eDecisionEntity m_type;                         // offset 0x0, size 0x4
    /* 0x04 */ unsigned long m_id;                             // offset 0x4, size 0x4
    /* 0x08 */ FuzzyVariant (*m_pDTF)(class cDecisionEntity*); // offset 0x8, size 0x4
    /* 0x0C */ FuzzyVariant (*m_pAF)(class cDecisionEntity*);  // offset 0xC, size 0x4
    /* 0x10 */ ScriptAction* m_pLastQueuedAction;              // offset 0x10, size 0x4
    /* 0x14 */ ScriptAction m_LastSelectedAction;              // offset 0x14, size 0x80
    /* 0x94 */ nlList<ScriptAction> m_lQueuedActions;          // offset 0x94, size 0x8
    /* 0x9C */ unsigned long m_iNumDTFCalls;                   // offset 0x9C, size 0x4
}; // total size: 0xA0

cDecisionEntity* GetDecisionEntity(eDecisionEntity type, unsigned long id);

#endif // _DECISIONENTITY_H_
