#ifndef _SCRIPTQUESTIONS_H_
#define _SCRIPTQUESTIONS_H_

#include "NL/nlMath.h"
#include "Game/Player.h"
#include "Game/Ball.h"
#include "Game/Team.h"
#include "Game/Game.h"
#include "Game/AI/Fielder.h"

extern cFielder* g_pScriptCurrentFielder;
extern cFielder* g_pScriptCurrentMark;

enum eScriptFielderDesire
{
    edNone = 0,
    edCutAndBreak = 1,
    edBlockPass = 6,
    edBlockShot = 6,
    edDeke = 2,
    edGetInPosition = 3,
    edGetOpen = 4,
    edHeavyAttack = 5,
    edInterceptBall = 6,
    edMark = 7,
    edProtectBall = 8,
    edRunToNet = 9,
    edRunUpfield = 10,
    edRunDownfield = 11,
    edRunToLocation = 12,
    edPass = 13,
    edShoot = 14,
    edSlideAttack = 15,
    edSupportBallDefensive = 16,
    edSupportBallOffensive = 17,
    edUsePowerup = 18,
    edWindupPass = 19,
    edWindupShot = 20,
    edUserControl = 22,
    edOneTimer = 24,
    edPostWhistle = 25,
    edWait = 28,
};

float InOffensiveZoneOfPlayer(cBall* pBall, cPlayer* pPlayer);
float InDefensiveZoneOfPlayer(cBall* pBall, cPlayer* pPlayer);
float InOffensiveZone(cPlayer* pPlayer);
float InDefensiveZone(cPlayer* pPlayer);
float InOffensiveZone(const nlVector3& position, eTeamSide teamSide);
float Difficult(cTeam* pTeam);
float TimeFarFromOver(cGame* pGame);
float TimeNearlyOver(cGame* pGame);
float TimeCloseToOver(cGame* pGame);
float PerfectPassCandidateFrom(cFielder* pReceiver, cFielder* pBallOwner);
float IsPerfectPassInPlay();
float IsPassInPlayDelayed();
float Stalling(cTeam* pTeam);
float Loose(cTeam* pTeam);
float Defensive(cTeam* pTeam);
float Offensive(cTeam* pTeam);
float Winning(cTeam* pTeam);
float Tied(cTeam* pTeam);
float Losing(cTeam* pTeam);
float GonnaGetBall(cTeam* team);
float UserControlledT(cTeam* pTeam);
float Passive(cTeam* pTeam);
float Moderate(cTeam* pTeam);
float AggressiveT(cTeam* pTeam);
float Ownerless(cBall* pBall);
float High(cBall* pBall);
float ReceivingVolleyPassDelayed(cPlayer* pPlayer);
float PassReceiveCloseToDone(cFielder* pFielder);
float ReceivingPassDelayed(cFielder* pFielder);
float ReceivingVolleyPass(cPlayer* pPlayer);
float ReceivingPass(cFielder* pFielder);
float ChasingBall(cPlayer* pPlayer);
float OnMushrooms(cFielder* pFielder);
float WindingUpForShot(cFielder* pFielder);
float InControlOfBall(cFielder* pFielder);
float FarToPlayersNet(cBall* pBall, cPlayer* pPlayer);
float NearToPlayersNet(cBall* pBall, cPlayer* pPlayer);
float CloseToPlayersNet(cBall* pBall, cPlayer* pPlayer);
float FarToTheirNetB(cBall* pBall);
float NearToTheirNetB(cBall* pBall);
float CloseToTheirNetB(cBall* pBall);
float Stunned(Goalie* pGoalie);
float OutOfNet(Goalie* pGoalie);
float SeparatingFrom(cPlayer* pPlayer1, cPlayer* pPlayer2);
float ClosingTo(cPlayer* pPlayer, cBall* pBall);
float ClosingTo(cPlayer* pPlayer1, cPlayer* pPlayer2);
float DownfieldFrom(cPlayer* pPlayer1, cPlayer* pPlayer2);
float UpfieldFrom(cPlayer* pPlayer1, cPlayer* pPlayer2);
float Facing(cPlayer* pPlayer1, cPlayer* pPlayer2);
float PositionIsAtIdealDistanceForShooting(const nlVector3& pos1, const nlVector3& pos2);
float AtIdealDistanceForTackling(cPlayer* pPlayer1, cPlayer* pPlayer2);
float StuckOnSidelines(cFielder* pFielder);
float FacingSideline(cFielder* pFielder);
float CloseToSideline(cFielder* pFielder);
float NearToSideline(const nlVector3& v3Position);
float CloseToSideline(const nlVector3& v3Position, const nlVector2* vDistanceConfidence, bool bInvert);
float FarToTheirGoalie(cPlayer* pPlayer);
float NearToTheirGoalie(cPlayer* pPlayer);
float CloseToTheirGoalie(cPlayer* pPlayer);
float NearToGoaliePosition(const nlVector3& pos1, const nlVector3& pos2);
float FarTo(cPlayer* pPlayer1, cPlayer* pPlayer2);
float NearTo(cPlayer* pPlayer1, cPlayer* pPlayer2);
float CloseTo(cPlayer* pPlayer1, cPlayer* pPlayer2);
float OpenTo(cPlayer* pPlayer1, cPlayer* pPlayer2);
float InBetweenMyNetAnd(cFielder* pFielder, cBall* pBall);
float InBetweenMyNetAnd(cFielder* pFielder1, cFielder* pFielder2);
float OpenToMyNet(cFielder* pFielder);
float OpenToTheirNet(cFielder* pFielder);
float WideOpen(cFielder* pFielder);
float Open(cFielder* pFielder);
float WideOpenPosition(const nlVector3& v3Position, cTeam* pOpponentTeam, cPlayer* pCurrentPlayer);
float OpenPosition(const nlVector3& v3Position, cTeam* pOpponentTeam, cPlayer* pCurrentPlayer, const nlVector2* pOpenRadius);
float OpenToPosition(const nlVector3& v3From, const nlVector3& v3To, const cTeam* pTeam, const cPlayer* pCurrentPlayer, const cPlayer* pIgnorePlayer, bool bNoGoalies);
float OnBreakaway(cFielder* pFielder);
float InFrontOfTheirNet(cFielder* pFielder);
float PositionIsInFrontOfNet(const nlVector3& position, const cNet* pNet);
float GoalieOutOfPosition(cFielder* pFielder);
float LikelyToScore(cFielder* pFielder);
float PlayerShotDistance(cFielder* pFielder);
float LikelyToScoreFromPosition(const nlVector3& v3Position, const nlVector3& v3GoaliePosition, const cNet* pNet, bool bIsChipShot);
float FallenDown(cFielder* pFielder);
float Frozen(cFielder* pFielder);
float Incapacitated(cPlayer* pPlayer);
float Invincible(cFielder* pFielder);
float AvoidingPowerups(cFielder* pFielder);
float Attacked(cFielder* pFielder);
float Pressured(cFielder* pFielder);
float FarToTheirNet(cPlayer* pPlayer);
float NearToTheirNet(cPlayer* pPlayer);
float CloseToTheirNet(cPlayer* pPlayer);
float FarToMyNet(cPlayer* pPlayer);
float NearToMyNet(cPlayer* pPlayer);
float CloseToMyNet(cPlayer* pPlayer);
float FarToBall(cPlayer* pPlayer);
float NearToBall(cPlayer* pPlayer);
float CloseToBall(cPlayer* pPlayer);
float LikelyToUsePowerup(cFielder* pFielder, int powerupType);
float AbleToUsePowerup(cFielder* pFielder, int powerupType);
float AbleToInterceptBallForSwapController(cFielder* pFielder);
float AbleToInterceptBall(cPlayer* pPlayer);
float RepeatingLastDesire(cFielder* pFielder, eScriptFielderDesire desire);
float Deker(cFielder* pFielder);
float Passer(cFielder* pFielder);
float Shooter(cFielder* pFielder);
float Aggressive(cFielder* pFielder);
float InPassingLane(cFielder* pFielder);
float UserControlled(cFielder* pFielder);
float StrategicBallOwner(cFielder* pFielder);
float OnTheGround(cPlayer* pPlayer);
float OnScreen(cPlayer* pPlayer);
float OnTheirTeam(cFielder* pFielder);
float Marking(cFielder* pFielder, cPlayer* pPlayer);
float GoalieType(cPlayer* pPlayer);
float Captain(cFielder* pFielder);
float Defence(cFielder* pFielder);
float Midfield(cFielder* pFielder);
float Winger(cFielder* pFielder);
float Striker(cFielder* pFielder);
float LastBallOwner(cPlayer* pPlayer);
float BallOwnerT(cTeam* pTeam);
float BallOwner(cPlayer* pPlayer);
float CalcSelectChance(float fDifficultyChance, float fPlayerAttribute);

static float FacingMark(cFielder* fielder)
{
    return Facing(fielder, g_pScriptCurrentMark);
}

static float FacingMe(cFielder* fielder)
{
    return Facing(fielder, g_pScriptCurrentFielder);
}

static float FarToMark(cFielder* fielder)
{
    return FarTo(fielder, g_pScriptCurrentMark);
}

static float NearToMark(cFielder* fielder)
{
    return NearTo(fielder, g_pScriptCurrentMark);
}

static float CloseToMark(cFielder* fielder)
{
    return CloseTo(fielder, g_pScriptCurrentMark);
}

static float OpenFromMe(cPlayer* fielder)
{
    return OpenTo(g_pScriptCurrentFielder, fielder);
}

static float OpenToMe(cPlayer* fielder)
{
    return OpenTo(fielder, g_pScriptCurrentFielder);
}

static float FarToMe(cPlayer* fielder)
{
    return FarTo(fielder, g_pScriptCurrentFielder);
}

static float NearToMe(cPlayer* fielder)
{
    return NearTo(fielder, g_pScriptCurrentFielder);
}

static float CloseToMe(cPlayer* fielder)
{
    return CloseTo(fielder, g_pScriptCurrentFielder);
}

static float FarToMyNetB(cBall* ball)
{
    return FarToPlayersNet(ball, g_pScriptCurrentFielder);
}

static float NearToMyNetB(cBall* ball)
{
    return NearToPlayersNet(ball, g_pScriptCurrentFielder);
}

static float CloseToMyNetB(cBall* ball)
{
    return CloseToPlayersNet(ball, g_pScriptCurrentFielder);
}

template <typename T>
nlVector3& PositionOf(T pObject)
{
    return pObject->m_v3Position;
}

#endif // _SCRIPTQUESTIONS_H_
