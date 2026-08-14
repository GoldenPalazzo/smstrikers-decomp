#ifndef _CTEAM_H_
#define _CTEAM_H_

#include <types.h>

#include "NL/nlMath.h"
#include "NL/globalpad.h"

#include "Game/GameTweaks.h"
// #include "Game/AI/Fielder.h"

class cFielder;

#include "Game/AI/Powerups.h"
#include "Game/Goalie.h"
#include "Game/Player.h"
#include "Game/Net.h"
#include "Game/Formation.h"

enum eSituation
{
    SITUATION_OFFENSE = 0,
    SITUATION_DEFENSE = 1,
    SITUATION_LOOSE = 2,
    NUM_SITUATIONS = 3,
};

enum eTeamStyle
{
    TEAM_STYLE_AGGRESSIVE = 0,
    TEAM_STYLE_MODERATE = 1,
    TEAM_STYLE_PASSIVE = 2,
    NUM_TEAM_STYLES = 3,
};

enum eTeamSide
{
    NO_SIDE = -1,
    HOME = 0,
    AWAY = 1,
    HOME_AWAY = 2,
};

enum eTeamID
{
    TEAM_INVALID = -1,
    TEAM_DAISY = 0,
    TEAM_DONKEYKONG = 1,
    TEAM_LUIGI = 2,
    TEAM_MARIO = 3,
    TEAM_PEACH = 4,
    TEAM_WALUIGI = 5,
    TEAM_WARIO = 6,
    TEAM_YOSHI = 7,
    TEAM_MYSTERY = 8,
    NUM_TEAMS = 9,
};

int MostDefensivePlayer(const void*, const void*);
int MostOffensiveThreat(const void*, const void*);
int BestAbleToInterceptBall(const void*, const void*);

class cTeam
{
public:
    cTeam(int);
    ~cTeam();
    void SetDifficulty(eDifficultyID);
    void ClearAllPowerUps();
    void ClearCurrentPowerUp();
    void TogglePowerup(bool);
    bool IncrementPowerupMeter(float);
    PowerUpTeamType GetCurrentPowerUp() const;
    bool IsCurrentNoPowerup() const;
    bool IsCurrentMushroom() const;
    bool IsCurrentStar() const;
    PowerUpTeamType GetPowerUpByIndex(int index) const;
    void SetIsPowerUpNew(int, bool);
    void SetCurrentPowerUp(ePowerUpType, int);
    void SetPlayer(cPlayer*, int);
    void SetGoalie(Goalie*);
    Goalie* GetGoalie();
    cPlayer* GetControlledPlayer(cGlobalPad*);
    int GetNumAssignedControllers();
    cFielder* GetFielder(int);
    cFielder* GetBallInterceptFielder(int i) { return m_pBallInterceptOrderedFielders[i]; }
    cPlayer* GetPlayer(int);
    cTeam* GetOtherTeam();
    cNet* GetOtherNet();
    void PreUpdate(float);
    void Update(float);
    void UpdateControllers();
    void ResetCharacters();
    void StopGameplayEffectsAndSounds();
    bool CalculateFormationPosition(nlVector3&, cFielder*, bool, float);
    void AbortPlays();
    bool AssignSituation();
    void UpdateTeamAI(float);
    void UpdatePlays(float);
    void CalculateNewBallInterceptTimes();
    void UpdateBallInterceptTime();
    void AssignMarks(bool);
    cFielder* GetCaptain();
    cFielder* GetStriker() const;
    cFielder* GetMidfield() const;
    cFielder* GetDefence() const;
    cFielder* GetFrontMostFielder();
    cFielder* GetRearMostFielder();

    /* 0x00 */ int m_nSide;
    /* 0x04 */ cFielder* m_pPlayers[5];
    // /* 0x14 */ Goalie* m_pGoalie;
    /* 0x18 */ cFielder* m_pAIOrderedFielders[4];
    /* 0x28 */ cFielder* m_pBallInterceptOrderedFielders[4];
    /* 0x38 */ cNet* m_pNet;
    /* 0x3C */ int m_nScore;
    /* 0x40 */ int m_nCurrentPowerUp;
    /* 0x44 */ PowerUpTeamType m_ePowerupList[2];
    /* 0x5C */ float mfPowerupMeter;
    /* 0x60 */ float mfPowerupTimer;
    /* 0x64 */ eSituation mpCurrentSituation;
    /* 0x68 */ eTeamStyle meCurrentTeamStyle;
    /* 0x6C */ FormationManager* m_pFormationManager;
    /* 0x70 */ Timer mtTeamStyleTimer;
    /* 0x74 */ Timer mtMarkTimer;
    /* 0x78 */ Timer mtRoleTimer;
    /* 0x7C */ Timer mtDefensiveZoneTimer;
    /* 0x80 */ bool mbHasToggledPowerup;
    /* 0x84 */ float mfBallInterceptTimes[4];
    /* 0x94 */ Timer mtBallInterceptTimer;
}; // total size: 0x98

extern cTeam* g_pTeams[];

#endif // _CTEAM_H_
