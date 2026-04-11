#ifndef _SCRIPTQUESTIONS_H_
#define _SCRIPTQUESTIONS_H_

#include "NL/nlMath.h"
#include "Game/Player.h"
#include "Game/Ball.h"
#include "Game/Team.h"
#include "Game/Game.h"
#include "Game/AI/Fielder.h"
#include "NL/nlSingleton.h"

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

class ScriptQuestionCache : public nlSingleton<ScriptQuestionCache>
{
public:
    ~ScriptQuestionCache();
    void Clear();

    // nlAVLTreeSlotPool mQuestionCacheMap; // offset 0x0, size 0x28
    // map mQuestionCacheMapSTD;            // offset 0x28, size 0x10
    int mTotalLookups; // offset 0x38, size 0x4
    int mCacheHits;    // offset 0x3C, size 0x4
}; // total size: 0x40

float InOffensiveZoneOfPlayer(cBall*, cPlayer*);
float InDefensiveZoneOfPlayer(cBall*, cPlayer*);
float InOffensiveZone(cPlayer*);
float InDefensiveZone(cPlayer*);
float InOffensiveZone(const nlVector3&, eTeamSide);
float Difficult(cTeam*);
float TimeFarFromOver(cGame*);
float TimeNearlyOver(cGame*);
float TimeCloseToOver(cGame*);
float PerfectPassCandidateFrom(cFielder*, cFielder*);
float IsPerfectPassInPlay();
float IsPassInPlayDelayed();
float Stalling(cTeam*);
float Loose(cTeam*);
float Defensive(cTeam*);
float Offensive(cTeam*);
float Winning(cTeam*);
float Tied(cTeam*);
float Losing(cTeam*);
float GonnaGetBall(cTeam*);
float UserControlledT(cTeam*);
float Passive(cTeam*);
float Moderate(cTeam*);
float AggressiveT(cTeam*);
float Ownerless(cBall*);
float High(cBall*);
float ReceivingVolleyPassDelayed(cPlayer*);
float PassReceiveCloseToDone(cFielder*);
float ReceivingPassDelayed(cFielder*);
float ReceivingVolleyPass(cPlayer*);
float ReceivingPass(cFielder*);
float ChasingBall(cPlayer*);
float OnMushrooms(cFielder*);
float WindingUpForShot(cFielder*);
float InControlOfBall(cFielder*);
float NearToPlayersNet(cBall*, cPlayer*);
float FarToTheirNetB(cBall*);
float Stunned(Goalie*);
float OutOfNet(Goalie*);
float SeparatingFrom(cPlayer*, cPlayer*);
float ClosingTo(cPlayer*, cBall*);
float ClosingTo(cPlayer*, cPlayer*);
float DownfieldFrom(cPlayer*, cPlayer*);
float UpfieldFrom(cPlayer*, cPlayer*);
float Facing(cPlayer*, cPlayer*);
float PositionIsAtIdealDistanceForShooting(const nlVector3&, const nlVector3&);
float AtIdealDistanceForTackling(cPlayer*, cPlayer*);
float StuckOnSidelines(cFielder*);
float FacingSideline(cFielder*);
float CloseToSideline(cFielder*);
float NearToSideline(const nlVector3&);
float CloseToSideline(const nlVector3&, const nlVector2*, bool);
float FarToTheirGoalie(cPlayer*);
float NearToTheirGoalie(cPlayer*);
float CloseToTheirGoalie(cPlayer*);
float NearToGoaliePosition(const nlVector3&, const nlVector3&);
float FarTo(cPlayer*, cPlayer*);
float NearTo(cPlayer*, cPlayer*);
float CloseTo(cPlayer*, cPlayer*);
float OpenTo(cPlayer*, cPlayer*);
float InBetweenMyNetAnd(cFielder*, cBall*);
float InBetweenMyNetAnd(cFielder*, cFielder*);
float OpenToMyNet(cFielder*);
float OpenToTheirNet(cFielder*);
float WideOpen(cFielder*);
float Open(cFielder*);
float WideOpenPosition(const nlVector3&, cTeam*, cPlayer*);
float OpenPosition(const nlVector3&, cTeam*, cPlayer*, const nlVector2*);
float OpenToPosition(const nlVector3&, const nlVector3&, const cTeam*, const cPlayer*, const cPlayer*, bool);
float OnBreakaway(cFielder*);
float InFrontOfTheirNet(cFielder*);
float PositionIsInFrontOfNet(const nlVector3&, const cNet*);
float GoalieOutOfPosition(cFielder*);
float LikelyToScore(cFielder*);
float PlayerShotDistance(cFielder*);
float LikelyToScoreFromPosition(const nlVector3&, const nlVector3&, const cNet*, bool);
float FallenDown(cFielder*);
float Frozen(cFielder*);
float Incapacitated(cPlayer*);
float Invincible(cFielder*);
float AvoidingPowerups(cFielder*);
float Attacked(cFielder*);
float Pressured(cFielder*);
float FarToTheirNet(cPlayer*);
float NearToTheirNet(cPlayer*);
float CloseToTheirNet(cPlayer*);
float FarToMyNet(cPlayer*);
float NearToMyNet(cPlayer*);
float CloseToMyNet(cPlayer*);
float FarToBall(cPlayer*);
float NearToBall(cPlayer*);
float CloseToBall(cPlayer*);
float LikelyToUsePowerup(cFielder*, int);
float AbleToUsePowerup(cFielder*, int);
float AbleToInterceptBallForSwapController(cFielder*);
float AbleToInterceptBall(cPlayer*);
float RepeatingLastDesire(cFielder*, eScriptFielderDesire);
float Deker(cFielder*);
float Passer(cFielder*);
float Shooter(cFielder*);
float Aggressive(cFielder*);
float InPassingLane(cFielder*);
float UserControlled(cFielder*);
float StrategicBallOwner(cFielder*);
float OnTheGround(cPlayer*);
float OnScreen(cPlayer*);
float OnTheirTeam(cFielder*);
float Marking(cFielder*, cPlayer*);
float GoalieType(cPlayer*);
float Captain(cFielder*);
float Defence(cFielder*);
float Midfield(cFielder*);
float Winger(cFielder*);
float Striker(cFielder*);
float LastBallOwner(cPlayer*);
float BallOwnerT(cTeam*);
float BallOwner(cPlayer*);
float CalcSelectChance(float, float);

#endif // _SCRIPTQUESTIONS_H_
