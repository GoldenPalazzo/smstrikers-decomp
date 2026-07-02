#ifndef _FORMATION_H_
#define _FORMATION_H_

#include "NL/nlMath.h"
#include "NL/nlTimer.h"

class cTeam;
class cFielder;
class FormationManager;
class cPlayer;

enum eFormationType
{
    FTYPE_DEFENSIVE = 0,
    FTYPE_OFFENSIVE = 1,
    FTYPE_BALLPOSITION = 2,
    NUM_FORMATION_TYPES = 3,
};

enum eFormationSet
{
    FSET_NONE = -1,
    FSET_OFFENSIVE_DEFENSIVE = 0,
    FSET_BALL_PASSIVE = 1,
    FSET_BALL_MODERATE = 2,
    FSET_BALL_AGGRESIVE = 3,
    NUM_FORMATION_SETS = 4,
};
enum eFormation
{
    FORMATION_NONE = -1,
    FORMATION_OFF_DEF_KICKOFF_NEUTRAL = 0,
    FORMATION_OFF_DEF_KICKOFF_ADVANTAGE = 1,
    FORMATION_OFF_DEF_31 = 2,
    FORMATION_OFF_DEF_121_DIAMOND = 3,
    FORMATION_OFF_DEF_22_SQUARE = 4,
    FORMATION_OFF_DEF_22_TRAPEZE = 5,
    FORMATION_BAL_PAS_OFFENSIVE_RIGHT = 6,
    FORMATION_BAL_PAS_OFFENSIVE_LEFT = 7,
    FORMATION_BAL_PAS_OFFENSIVE_CENTER = 8,
    FORMATION_BAL_PAS_DEFENSIVE_RIGHT = 9,
    FORMATION_BAL_PAS_DEFENSIVE_CENTER = 10,
    FORMATION_BAL_PAS_DEFENSIVE_LEFT = 11,
    FORMATION_BAL_PAS_CENTER_RIGHT = 12,
    FORMATION_BAL_PAS_CENTER_LEFT = 13,
    FORMATION_BAL_PAS_MIDOFFENSIVE_RIGHT = 14,
    FORMATION_BAL_PAS_MIDOFFENSIVE_LEFT = 15,
    FORMATION_BAL_PAS_MIDOFFENSIVE_CENTRE = 16,
    FORMATION_BAL_MOD_OFFENSIVE_RIGHT = 17,
    FORMATION_BAL_MOD_OFFENSIVE_LEFT = 18,
    FORMATION_BAL_MOD_OFFENSIVE_CENTER = 19,
    FORMATION_BAL_MOD_DEFENSIVE_RIGHT = 20,
    FORMATION_BAL_MOD_DEFENSIVE_CENTER = 21,
    FORMATION_BAL_MOD_DEFENSIVE_LEFT = 22,
    FORMATION_BAL_MOD_CENTER_LEFT = 23,
    FORMATION_BAL_MOD_CENTER_RIGHT = 24,
    FORMATION_BAL_MOD_MIDOFFENSIVE_RIGHT = 25,
    FORMATION_BAL_MOD_MIDOFFENSIVE_LEFT = 26,
    FORMATION_BAL_MOD_MIDOFFENSIVE_CENTRE = 27,
    FORMATION_BAL_MOD_CENTER = 28,
    FORMATION_BAL_AGG_OFFENSIVE_RIGHT = 29,
    FORMATION_BAL_AGG_OFFENSIVE_LEFT = 30,
    FORMATION_BAL_AGG_OFFENSIVE_CENTER = 31,
    FORMATION_BAL_AGG_DEFENSIVE_RIGHT = 32,
    FORMATION_BAL_AGG_DEFENSIVE_CENTER = 33,
    FORMATION_BAL_AGG_DEFENSIVE_LEFT = 34,
    FORMATION_BAL_AGG_CENTER_LEFT = 35,
    FORMATION_BAL_AGG_CENTER_RIGHT = 36,
    FORMATION_BAL_AGG_MIDOFFENSIVE_RIGHT = 37,
    FORMATION_BAL_AGG_MIDOFFENSIVE_LEFT = 38,
    FORMATION_BAL_AGG_MIDOFFENSIVE_CENTRE = 39,
    FORMATION_BAL_AGG_MIDDEFENSIVE_CENTRE = 40,
    NUM_FORMATIONS = 41,
};

class FormationSet;
class FormationSpec;

class FormationEval
{
public:
    FormationEval();
    FormationEval(FormationManager*, eFormationType, const FormationSpec*);
    virtual ~FormationEval() { }
    virtual void CalculateDesiredLocation(nlVector3&, cFielder*, bool);
    virtual cPlayer* GetKeyPlayer();
    virtual void GetKeyPositions(cFielder*, nlVector3&, nlVector3*, bool);
    virtual float GetWeight();
    virtual float IsFielderInPosition(cFielder*, nlVector3, bool);
    virtual void SortPlayers(const nlVector2*);
    virtual void Update(float);
    static FormationEval* Create(FormationManager*, eFormationType, eFormationSet, eFormation);
    void AssignPositionsToFielders(unsigned int*, float (*)[4]);

    /* 0x04 */ eFormationType m_eFormationType;
    /* 0x08 */ const FormationSpec* m_pFormationSpec;
    /* 0x0C */ cPlayer* m_pKeyPlayer;
    /* 0x10 */ FormationManager* m_pFormationManager;
    /* 0x14 */ Timer m_SortTimer;
    /* 0x18 */ unsigned int m_iFielderFormationPos[4];
}; // total size: 0x28

class FormationBallPosition : public FormationEval
{
public:
    FormationBallPosition(FormationManager*, eFormationType, const FormationSet*);
    FormationBallPosition(FormationManager*, eFormationType, const FormationSpec*);
    ~FormationBallPosition();
    void CalculateDesiredLocation(nlVector3&, cFielder*, bool);
    float GetWeight();
    float GetBlendFactor();
    void CalcBallPosition(nlVector2&);
    void Update(float);
    bool SelectClosestBallFormations(const nlVector2&);

    /* 0x28 */ const FormationSet* m_pFormationSet;
    /* 0x2C */ FormationBallPosition* m_pNextClosestFormation;
}; // total size: 0x30

class FormationOffensive : public FormationEval
{
public:
    FormationOffensive(FormationManager*, eFormationType, const FormationSpec*);
    ~FormationOffensive() { }
    float IsFielderInPosition(cFielder*, nlVector3, bool);
    float GetWeight();
};

class FormationDefensive : public FormationEval
{
public:
    FormationDefensive(FormationManager*, eFormationType, const FormationSpec*);
    ~FormationDefensive() { }
    float IsFielderInPosition(cFielder*, nlVector3, bool);
    float GetWeight();
};

struct CachedPosition
{
    /* 0x0 */ bool bCacheIsValid;
    /* 0x1 */ bool bInPosition;
    /* 0x4 */ nlVector3 vPosition;
}; // total size: 0x10

class FormationManager
{
public:
    FormationManager(cTeam*);
    ~FormationManager();

    static void LoadFormationSets();
    static void UnloadFormationSets();
    static FormationSpec* GetFormationSpec(eFormation);
    void Update(float);
    void ChooseNewFormations();
    void SetNewFormationEval(eFormationType, eFormation);
    void SetNewFormationEval(eFormationType, eFormationSet);
    bool CalculateFielderPosition(nlVector3&, cFielder*, bool, float);

    /* 0x00 */ cTeam* m_pTeam;
    /* 0x04 */ FormationEval* m_pFormations[3];
    /* 0x10 */ nlVector2 m_v2AIFielderCenter;
    /* 0x18 */ Timer m_tSelectFormationsTimer;
    /* 0x1C */ CachedPosition m_CachedPositions[4];

    static FormationSet* m_FormationSetArray;
    static int m_NumFormationSets;
}; // total size: 0x5C

#endif // _FORMATION_H_
