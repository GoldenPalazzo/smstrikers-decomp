#include "Game/AI/Scripts/RootScript.h"
#include "Game/AI/AIPlay.h"

class SaveConfidence
{
public:
    SaveConfidence(float* pFloat);
    ~SaveConfidence();

    float m_savedValue;
    float* m_pFloat;
};

SaveConfidence::SaveConfidence(float* pFloat)
    : m_savedValue(*pFloat)
    , m_pFloat(pFloat)
{
}

class Fuzzy
{
public:
    static void DefaultOffensivePlay(cDecisionEntity*);
    static void AbortOffensivePlay(cDecisionEntity*);
    static void DefaultDefencePlay(cDecisionEntity*);
    static void AbortDefencePlay(cDecisionEntity*);
    static void DefaultLoosePlay(cDecisionEntity*);
    static void AbortLoosePlay(cDecisionEntity*);
};

typedef FuzzyVariant (*DecisionFn)(cDecisionEntity*);

cDecisionEntity g_pDecisionEntities[44] = {
    cDecisionEntity(DECISION_ENTITY_STRATEGY, 0, (DecisionFn)StrategyChoosePlay, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_NULL, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_STRIKER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_STRIKER_MODERATE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_STRIKER_PASSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_WINGER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_WINGER_MODERATE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_WINGER_PASSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_MIDFIELD_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_MIDFIELD_MODERATE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_MIDFIELD_PASSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_DEFENDER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_DEFENDER_MODERATE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_OFFENSE_DEFENDER_PASSIVE, (DecisionFn)Fuzzy::DefaultOffensivePlay, (DecisionFn)Fuzzy::AbortOffensivePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_STRIKER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_STRIKER_MODERATE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_STRIKER_PASSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_WINGER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_WINGER_MODERATE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_WINGER_PASSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_MIDFIELD_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_MIDFIELD_MODERATE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_MIDFIELD_PASSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_DEFENDER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_DEFENDER_MODERATE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_DEFENSE_DEFENDER_PASSIVE, (DecisionFn)Fuzzy::DefaultDefencePlay, (DecisionFn)Fuzzy::AbortDefencePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_STRIKER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_STRIKER_MODERATE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_STRIKER_PASSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_WINGER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_WINGER_MODERATE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_WINGER_PASSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_MIDFIELD_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_MIDFIELD_MODERATE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_MIDFIELD_PASSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_DEFENDER_AGGRESSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_DEFENDER_MODERATE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_LOOSE_DEFENDER_PASSIVE, (DecisionFn)Fuzzy::DefaultLoosePlay, (DecisionFn)Fuzzy::AbortLoosePlay),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_PREKICKOFF, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_BALLOUTOFPLAY, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_ENDHALF, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_ENDGAME, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_POSTGOAL, NULL, NULL),
    cDecisionEntity(DECISION_ENTITY_PLAY, AIPLAY_GSTATE_POSTREDCARD, NULL, NULL),
};

/**
 * Offset/Address/Size: 0x1E8C | 0x8007C478 | size: 0x8
 */
int GetNumDecisionEntities()
{
    return 0x2C;
}

/**
 * Offset/Address/Size: 0x0 | 0x8007A5EC | size: 0x1E8C
 */
void StrategyChoosePlay(cDecisionEntity* pDecision)
{
}
