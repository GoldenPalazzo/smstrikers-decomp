#ifndef _FIELDER_H_
#define _FIELDER_H_

#include "types.h"
#include "NL/nlMath.h"

#include "Game/Net.h"
#include "Game/Player.h"
#include "Game/AI/Powerups.h"
#include "Game/AI/ScriptAction.h"

enum eTurboRequest
{
    TR_FAR_DISTANCE = 0,
    TR_MOVING_TARGET = 1,
    TR_FORCED_ON = 2,
    TR_FORCED_OFF = 3,
};

enum eFielderActionState
{
    ACTION_NEED_ACTION = -1,
    ACTION_DEKE = 0,
    ACTION_ELECTROCUTION = 1,
    ACTION_HIT = 2,
    ACTION_HIT_REACT = 3,
    ACTION_IDLE_TURN = 4,
    ACTION_LATE_ONETIMER_FROM_VOLLEY = 5,
    ACTION_LOOSE_BALL_PASS = 6,
    ACTION_LOOSE_BALL_SHOT = 7,
    ACTION_ONETIMER = 8,
    ACTION_ONETOUCH_PASS_FROM_VOLLEY = 9,
    ACTION_PASS = 10,
    ACTION_POST_WHISTLE = 11,
    ACTION_RECEIVE_PASS = 12,
    ACTION_RUNNING = 13,
    ACTION_RUNNING_WB = 14,
    ACTION_RUNNING_WB_TURBO = 15,
    ACTION_RUNNING_WB_TURBO_TURN = 16,
    ACTION_SHOT = 17,
    ACTION_SHOOT_TO_SCORE = 18,
    ACTION_SLIDE_ATTACK = 19,
    ACTION_SLIDE_ATTACK_REACT = 20,
    ACTION_BOMB_REACT = 21,
    ACTION_SHELL_REACT = 22,
    ACTION_BANANA_REACT = 23,
    ACTION_STS_HIT_REACT = 24,
    ACTION_SQUISH_REACT = 25,
    ACTION_SLIDE_FAIL_REACT = 26,
    ACTION_WAIT = 27,
    NUM_FIELDER_ACTIONS = 28,
};

enum eAwardPowerupType
{
    AWARD_POWERUP_POWER_SHOT = 0,
    AWARD_POWERUP_INTERCEPT_PASS = 1,
    AWARD_POWERUP_PERFECT_PASS = 2,
    AWARD_POWERUP_CONTEXT_DEKE = 3,
    NUM_AWARD_POWERUP_TYPES = 4,
};

enum ePenaltyType
{
    PEN_TYPE_HIT_WITH_BALL = 0,
    PEN_TYPE_HIT_NO_BALL = 1,
    PEN_TYPE_SLIDE_WITH_BALL = 2,
    PEN_TYPE_SLIDE_NO_BALL = 3,
    NUM_PEN_TYPES = 4,
};

enum ePenaltyCardStatus
{
    PENALTY_CARD_NONE = -1,
    PENALTY_CARD_YELLOW_1 = 0,
    PENALTY_CARD_YELLOW_2 = 1,
    PENALTY_CARD_RED = 2,
    NUM_PENALTY_CARDS = 3,
};

enum eStrafeDirection
{
    STRAFE_IDLE = 0,
    STRAFE_RIGHT = 1,
    STRAFE_LEFT = 2,
    STRAFE_FORWARD = 3,
    STRAFE_BACK = 4,
};

enum eRole
{
    ROLE_STRIKER = 0,
    ROLE_WINGER = 1,
    ROLE_MIDFIELD = 2,
    ROLE_DEFENCE = 3,
    NUM_ROLES = 4,
};

enum eShootToScoreResult
{
    S2S_SAVED = 0,
    S2S_SAVED_YELLOW = 1,
    S2S_SCORE = 2,
    S2S_SUPER_SHOT = 3,
};

enum eSlideAttackState
{
    SLIDE_ATTACK_DOWN = 0,
    SLIDE_ATTACK_DECELERATE = 1,
};

struct ActionShootToScoreVars
{
    ActionShootToScoreVars() { memset(this, 0, sizeof(ActionShootToScoreVars)); }
    /* 0x00 */ float fFrameButtonDownTime2;
    /* 0x04 */ float fFrameButtonDownTime1;
    /* 0x08 */ float fGreenRegionWidth;
    /* 0x0C */ float fShootToScoreActiveTime;
    /* 0x10 */ float fMeterFractionTime;
    /* 0x14 */ nlVector3 v3MeterPosition;
    /* 0x20 */ bool bShootWasPressed;
    /* 0x21 */ bool isCaptainSts;
    /* 0x22 */ bool isCurrentlyInvincible;
    /* 0x23 */ bool isInUnbreakablePart;
    /* 0x24 */ cAnimCamera* captainStsCamera;
    /* 0x28 */ float preCaptainStsPlaybackSpeed;
    /* 0x2C */ float fCaptainYellowWidth;
}; // total size: 0x30

struct LooseBallContactAnimInfo
{
    /* 0x0 */ s32 nAnimID;
    /* 0x4 */ float fAnimContactFrame;
    /* 0x8 */ u16 aIncomingAngleMin;
    /* 0xA */ u16 aIncomingAngleMax;
}; // total size: 0xC

class cBall;
class ChainChomp;
class Bowser;
class AIPlay;
class AvoidController;
class ShotMeter;

class cFielder : public cPlayer
{
public:
    cFielder(int, int, eCharacterClass, const int*, cSHierarchy*, cAnimInventory*, const CharacterPhysicsData*, FielderTweaks*, AnimRetargetList*);
    /* 0x08 */ virtual ~cFielder();

    /* 0x0C */ virtual void PostPhysicsUpdate();
    /* 0x10 */ // virtual void PrePhysicsUpdate(float);
    /* 0x14 */ virtual void PreUpdate(float);
    /* 0x18 */ // virtual void SetAnimID(int);
    /* 0x1C */ virtual void SetPosition(const nlVector3&);
    /* 0x20 */ virtual void Update(float);
    /* 0x24 */ virtual bool CanPickupBall(cBall*);
    /* 0x28 */ // virtual void CollideWithBallCallback(cBall*);
    /* 0x2C */ virtual void CollideWithCharacterCallback(CollisionPlayerPlayerData*);
    /* 0x30 */ virtual void CollideWithWallCallback(const CollisionPlayerWallData*);
    /* 0x34 */ // virtual void CollideWithWallCallback(const CollisionPlayerWallData*);
    /* 0x38 */ virtual void InitActionPostWhistle();

    void AbortPlay();
    void AbortPendingThoughts();
    void CalculateNewDesire();
    bool CanDoCaptainShootToScore();
    bool CanLooseBallShoot();
    bool CanLooseBallPass();
    bool CanReceivePass();
    void SetMark(cFielder*);
    bool CollideWithShellCallback(ePowerupSize, bool, const nlVector3&, const nlVector3&);
    bool CollideWithFreezeCallback();
    bool CollideWithBananaCallback(const nlVector3&);
    void CollideWithChainCallback(ChainChomp*);
    void CollideWithBowserCallback(Bowser*);
    void ClearPassTargetIfAmThePassTarget();
    bool UsePerfectPass();
    bool IsPlayingPowerupAnim();
    bool IsCharacterInAir(bool) const;
    bool IsTurboing();
    bool IsRunning() const;
    bool IsInvincible() const;
    bool IsBallAwayFromCarrier() const;
    bool IsReceivingVolleyPass() const;
    bool IsPreparingForOneTimer() const;
    void CleanUpAction();
    // void EndAction();
    void CleanUpPowerupEffect();
    void CalcRegularShot(nlVector3&, nlVector3&);
    void CalcShootToScoreShot(nlVector3&, nlVector3&);
    void SetSlideAttackSuccessFlag();
    void SetKickOffWaitTime();
    void SetBombImpactTime(const nlVector3&, float);
    void SetDesireDuration(float, bool);
    void ShootBallDueToContact(const nlVector3&);
    void ShootBallDueToContact(unsigned short);
    void DoClearBall();
    void DoHandleActiveShotMeter();
    bool DoLooseBallContactFromIdle(nlVector3&, float&, nlVector3&, float&, unsigned short, const LooseBallContactAnimInfo*);
    bool DoLooseBallContactFromRun(nlVector3&, float&, nlVector3&, float&, const LooseBallContactAnimInfo*, const nlVector3&);
    bool DoLooseBallContactFromRunVolley(nlVector3&, float&, nlVector3&, float&, const LooseBallContactAnimInfo*, const nlVector3&);
    void DoPenaltyCardBooking(cFielder*, ePenaltyType);
    void DoPositioningInterceptBall();
    void DoAwardPowerupStuff(eAwardPowerupType, float);
    void DoCalcShootToScoreResult(float, float, float, float, float);
    cFielder* DoFindBestHitTarget();
    void DoFindBestShotTarget(nlVector3&, float&, bool);
    void DoRegularShooting();
    void DoResetShotMeter(float);
    bool IsActionDone() const;
    void SetAction(eFielderActionState);
    void InitActionSlideAttackReact(cPlayer*, bool);
    bool GetFormationPosition(nlVector3&, float);
    LooseBallContactAnimInfo* GetOneTimerBallContactAnimInfo(unsigned short, const nlVector3&, const nlVector3&, bool, bool);
    const LooseBallContactAnimInfo* GetReceivePassBallContactAnimInfo(cBall*, const nlVector3&, unsigned short, bool, bool);
    void GetReceivePassBallContactOffset(nlVector3&, unsigned short, const LooseBallContactAnimInfo*);
    bool IsFallenDown(float) const;
    bool IsHitting() const;
    bool IsSlideTackling() const;
    bool IsStriker() const;
    bool IsWinger() const;
    bool IsMidField() const;
    bool IsDefense() const;
    bool IsFrozen() const;
    void SetFrozen(float);
    float DoFindBestSlideAttackTarget(nlVector3&, nlVector3&);
    bool CanBeBlownUp();
    void CanBreakOutOfSlideTackle();
    eStrafeDirection CalculateStrafeDirection(unsigned short, unsigned short);
    void CalcPointOnPerimeter(nlVector3&, const nlVector3&, float);
    void ClearTimers();
    void ClearVolleyPass();
    void CleanActionShootToScore();
    void SetAttemptOneTouchPass();
    void SetAttemptOneTouchShot();
    s16 GetOneTouchShotDesire();
    void SetStartAnimState(int);
    void SetWindupWBAnimState();
    void SetStartWBAnimState();
    void SetRunTurboAnimState(int, bool);
    void SetHardStopAnimState();
    void SetHardStopRecoverAnimState();
    void SetHardStopTurnAnimState();
    void SetRunBackwardsAnimState();
    void SetRunToBackRunningAnimState();
    void SetBackRunningToRunAnimState();
    void SetBackRunningStopAnimState();
    void SetBackRunningStopStartAnimState();
    void SetBackRunningStopRecoverAnimState();
    void SetStopAnimState();
    void SetStopWBAnimState();
    void SetStrafeLeftAnimState();
    void SetStrafeRightAnimState();
    void SetIdleStrafeAnimState();
    void SetIdleAnimState();
    void SetIdleWBAnimState();
    static void RunningSABcallback(unsigned int, cPN_SingleAxisBlender*);
    inline void SetRunLeanSAB(const int* pSABAnims, int nNumSABAnims, int nPrimaryAnim);
    void SetRunningAnimState(float);
    void SetRunningTurboAnimState();
    void SetRunningWBAnimState(float);
    bool ShouldIClearBall();
    bool ShouldILeadPass();
    bool CanISlideAttack(const nlVector3&, const nlVector3&, float*);
    void SetDesiredSpeedAndDirectionToPosition(float, const nlVector3&, eTurboRequest, float, float);
    void SetDesiredSpeed(float, float);
    float GetSpeedPowerupAdjusted(float);
    float GetSlideAttackSpeed();
    bool SetDesire(eFielderDesireState, float);
    u8 ShouldIStrafe();
    bool ShouldITurboWithoutBall();
    void ShouldIWave();
    void TestCollisionForInvicibility(cFielder*);
    void TestButtonsToQueueActions(float);
    bool TestQueuedActions();
    void TestButtonsRunning();
    void TestButtonsRunningWB(float);
    void ThrowPowerup();
    void SetPowerup(ePowerUpType, int, cFielder*);
    void UseTeamPowerup(cFielder*);
    void UpdateActionState(float);
    void UpdateHeadTracking(float);
    void UpdateController(float);
    void UpdatePlay(float);
    float GetDistanceToDesiredPos();
    bool S2SShootWasPressed();
    void StartRunning();
    bool DoAILooseBallActionSelection();
    bool DoAIReceivePassActionSelection();
    bool DoAIWindupActionSelection();
    void DoSpeedBoost();

    inline bool IsRunningState()
    {
        bool isRunningWithBall;
        bool result;

        result = false;
        if (m_eActionState != ACTION_RUNNING)
        {
            isRunningWithBall = result;
            if (((u32)(m_eActionState - ACTION_RUNNING_WB) <= 1) || (m_eActionState == ACTION_RUNNING_WB_TURBO_TURN))
            {
                isRunningWithBall = true;
            }
            if (isRunningWithBall)
            {
                goto set_result; // ugly, but works. I suspect it is solvable with another class-inline method.
            }
        }
        else
        {
        set_result:
            result = true;
        }
        return result;
    }

    // to be verified
    void QueueDesire(eFielderDesireState, float, FuzzyVariant, FuzzyVariant);
    void ClearQueuedDesire();
    bool InitDesire(const sDesireParams*, float);
    bool InitDesire(eFielderDesireState eDesire, float fConfidence, float fDuration, FuzzyVariant vOpt1, FuzzyVariant vOpt2);
    void UpdateDesireState(float);
    void EndDesire(bool);
    void CleanUpDesire(eFielderDesireState);
    void DesireInterceptBall(float);
    void DesireMark(float);
    void DesireSupportBall(float, bool);
    bool InitDesireGetOpen();
    bool InitDesireOneTimerFromRun(unsigned short, const nlVector3&, const nlVector3&, bool, bool);
    void DesireOneTimer(float);
    void InitDesireReceivePassFromIdle(const LooseBallContactAnimInfo*, unsigned short, bool);
    void DesireReceivePassFromIdle(float);
    void InitDesireReceivePassFromRun(const LooseBallContactAnimInfo*, const nlVector3&, bool, const nlVector3&);
    void DesireReceivePassFromRun(float);
    u8 InitDesireRunToNet();
    void DesireSlideAttack(float);
    void DesireUserControlled(float);
    void DesireUsePowerup(float);
    void DesireWindupShot(float);

    // actions
    void asmRunning();
    void asmRunningWB(float);
    void EndAction();
    void InitActionDeke(ePadActions);
    void ActionDeke(float);
    void InitActionElectrocution(const nlVector3&, const nlVector3&);
    void ActionElectrocution(float);
    void InitActionHit(cFielder*);
    void ActionHit(float);
    bool InitActionHitReact(cPlayer*, unsigned short, bool);
    void ActionHitReact(float);
    void InitActionIdleTurn(unsigned short);
    void ActionIdleTurn(float);
    void InitActionLateOneTimerFromVolley();
    void ActionLateOneTimerFromVolley(float);
    void DoCommonInitActionLooseBall(const nlVector3&);
    void InitActionLooseBallPass(cFielder*, bool);
    void ActionLooseBallPass(float);
    void InitActionLooseBallShot(bool);
    void ActionLooseBallShot(float);
    void InitActionOneTimer(int, nlVector3&, float, bool);
    void ActionOneTimer(float);
    void InitActionOneTouchPassFromVolley(cPlayer*);
    void ActionOneTouchPassFromVolley(float);
    bool DoCalcCanDoPerfectPass(cFielder*, const nlVector3&);
    void InitActionPass(cPlayer*, bool, bool);
    void ActionPass(float);
    void ActionPostWhistle(float);
    void InitActionBombReact(const nlVector3&, float);
    void InitActionBombHitReact(const nlVector3&);
    void InitActionBananaReact(const nlVector3&);
    void InitActionShellReact(const nlVector3&, const nlVector3&);
    void InitActionRunning();
    void ActionRunning(float);
    void InitActionRunningWB(bool);
    void ActionRunningWB(float);
    void ActionRunningWBTurbo(float);
    void ActionRunningWBTurboTurn(float);
    void InitActionShot(bool);
    void ActionShot(float);
    void InitActionShootToScore();
    void SetupCaptainSTSAnimCam(bool);
    void ActionShootToScore(float);
    void InitActionSlideAttack(cFielder*, float);
    void ActionSlideAttack(float);
    void InitActionSlideAttackFailReact();
    void ActionSlideAttackFailReact(float);
    void InitActionSquishReact(const nlVector3&);
    void DoSlideAttackStats();
    // void InitActionSlideAttackReact(cPlayer*, bool);
    void InitActionSTSHitReact(cPlayer*);
    void ActionSlideAttackReact(float);
    void ActionBombReact(float);
    void ActionSTSHitReact(float);
    void ActionShellReact(float);
    void ActionBananaReact(float);
    void ActionSquishReact(float);
    void InitActionReceivePass(int, nlVector3&, float);
    void ActionReceivePass(float);
    void InitActionWait();
    void ActionWait(float);

    inline void SetAnim(const u16 facingDelta, const int* animIDs)
    {
        // u16 facingDelta = delta + 0x2000;
        // int animID = animIDs[(facingDelta >> 14) & 3];
        SetAnimState(animIDs[(facingDelta >> 14) & 3], true, 0.2f, false, false);
    }

    bool m_bHasBeenUpdated;             // offset 0x1D4, size 0x1
    eFielderActionState m_eActionState; // offset 0x1D8, size 0x4
    float m_fShootToScoreActiveTime;    // offset 0x1DC, size 0x4
    Timer m_tFrozenTimer;               // offset 0x1E0, size 0x4
    struct                              /* @class$486Powerups_cpp */
    {
        // total size: 0x4
        bool bStickWasReset;             // offset 0x0, size 0x1
        bool bPossibleSuccessfulDeke;    // offset 0x1, size 0x1
        bool bPossibleTurboMove;         // offset 0x2, size 0x1
        bool bTurboButtonDownLastUpdate; // offset 0x3, size 0x1
    } mActionDekeVars;                   // offset 0x1E4, size 0x4
    struct                               /* @class$487Powerups_cpp */
    {
        // total size: 0x4
        float electrocutionTime; // offset 0x0, size 0x4
    } mActionElectrocutionVars;  // offset 0x1E8, size 0x4
    struct                       /* @class$488Powerups_cpp */
    {
        // total size: 0x1
        bool bDoFrameLock;       // offset 0x0, size 0x1
    } mActionHitReactActionVars; // offset 0x1EC, size 0x1
    struct                       /* @class$489Powerups_cpp */
    {
        // total size: 0x4
        float fRumbleDirection; // offset 0x0, size 0x4
    } mActionRumbleVars;        // offset 0x1F0, size 0x4
    struct                      /* @class$490Powerups_cpp */
    {
        // total size: 0x2
        bool bIsShootToScore; // offset 0x0, size 0x1
        bool bIsChipShot;     // offset 0x1, size 0x1
    } mActionShotVars;        // offset 0x1F4, size 0x2
    struct                    /* @class$491Powerups_cpp */
    {
        // total size: 0x8
        cPlayer* passTarget;    // offset 0x0, size 0x4
        bool bVolleyPass;       // offset 0x4, size 0x1
    } mActionLooseBallPassVars; // offset 0x1F8, size 0x8
    struct                      /* @class$492Powerups_cpp */
    {
        // total size: 0x1
        bool bIsChipShot;       // offset 0x0, size 0x1
    } mActionLooseBallShotVars; // offset 0x200, size 0x1
    struct                      /* @class$493Powerups_cpp */
    {
        // total size: 0x4
        float fOneTimerAnimTime; // offset 0x0, size 0x4
    } mActionOneTimerVars;       // offset 0x204, size 0x4
    struct                       /* @class$494Powerups_cpp */
    {
        // total size: 0x8
        cPlayer* pPassTarget; // offset 0x0, size 0x4
        bool bVolleyPass;     // offset 0x4, size 0x1
        bool bAllowLeadPass;  // offset 0x5, size 0x1
    } mActionPassingVars;     // offset 0x208, size 0x8
    struct                    /* @class$495Powerups_cpp */
    {
        // total size: 0x8
        eStrafeDirection eLastStrafeDirection; // offset 0x0, size 0x4
        bool bFirstCycleOfTurbo;               // offset 0x4, size 0x1
    } mActionRunningVars;                      // offset 0x210, size 0x8
    struct                                     /* @class$496Powerups_cpp */
    {
        // total size: 0x1
        bool bWaitForAnimToFinish; // offset 0x0, size 0x1
    } mActionRunningWBVars;        // offset 0x218, size 0x1
    struct                         /* @class$497Powerups_cpp */
    {
        // total size: 0x1
        bool bForcedMirrorSwap;  // offset 0x0, size 0x1
    } mActionRunningWBTurboVars; // offset 0x219, size 0x1
    struct                       /* @class$498Powerups_cpp */
    {
        // total size: 0x8
        eSlideAttackState eSlideAttackState;        // offset 0x0, size 0x4
        bool bAttackSucceeded;                      // offset 0x4, size 0x1
        bool bIsBButtonReset;                       // offset 0x5, size 0x1
        bool bWasStarMushroomUsedDuring;            // offset 0x6, size 0x1
    } mActionSlideAttackVars;                       // offset 0x21C, size 0x8
    ActionShootToScoreVars mActionShootToScoreVars; // offset 0x224, size 0x30
    eFielderDesireState m_eFielderDesireState;      // offset 0x254, size 0x4
    eFielderDesireState m_ePrevFielderDesireState;  // offset 0x258, size 0x4
    int m_eDesireSubState;                          // offset 0x25C, size 0x4
    Timer m_tDesireDuration;                        // offset 0x260, size 0x4
    float m_fDesireConfidence;                      // offset 0x264, size 0x4
    sDesireParams m_sQueuedDesireParams;            // offset 0x268, size 0x68
    struct                                          /* @class$499Powerups_cpp */
    {
        // total size: 0x28
        Timer tMiscTimer;            // offset 0x0, size 0x4
        Timer tAge;                  // offset 0x4, size 0x4
        float fMisc;                 // offset 0x8, size 0x4
        cPlayer* pSBC;               // offset 0xC, size 0x4
        cPlayer* pBallOwner;         // offset 0x10, size 0x4
        nlVector3 v3DesiredPosition; // offset 0x14, size 0xC
        bool bInPosition;            // offset 0x20, size 0x1
        eTurboRequest turboRequest;  // offset 0x24, size 0x4
    } m_DesireCommonVars;            // offset 0x2D0, size 0x28
    struct                           /* @class$500Powerups_cpp */
    {
        // total size: 0x1
        bool bDidWave;     // offset 0x0, size 0x1
    } m_DesireGetOpenVars; // offset 0x2F8, size 0x1
    struct                 /* @class$501Powerups_cpp */
    {
        // total size: 0x2C
        unsigned short aDesiredFacingDirection; // offset 0x0, size 0x2
        nlVector3 v3DesiredPosition;            // offset 0x4, size 0xC
        float fDesiredTime;                     // offset 0x10, size 0x4
        nlVector3 v3BallPosition;               // offset 0x14, size 0xC
        int nOneTimerAnim;                      // offset 0x20, size 0x4
        float fOneTimerAnimTime;                // offset 0x24, size 0x4
        bool bIsChipShot;                       // offset 0x28, size 0x1
        bool bVolleyPassReceive;                // offset 0x29, size 0x1
    } m_DesireOneTimerVars;                     // offset 0x2FC, size 0x2C
    struct                                      /* @class$502Powerups_cpp */
    {
        // total size: 0x38
        nlVector3 v3DesiredPosition;            // offset 0x0, size 0xC
        unsigned short aDesiredFacingDirection; // offset 0xC, size 0x2
        float fDesiredTime;                     // offset 0x10, size 0x4
        nlVector3 v3BallPosition;               // offset 0x14, size 0xC
        int nReceivePassAnim;                   // offset 0x20, size 0x4
        float fReceivePassAnimTime;             // offset 0x24, size 0x4
        int iAttemptOneTouchShot;               // offset 0x28, size 0x4
        bool bFailedToInitOneTouchShot;         // offset 0x2C, size 0x1
        signed short iAttemptOneTouchPass;      // offset 0x2E, size 0x2
        bool bVolleyPassReceive;                // offset 0x30, size 0x1
        cPlayer* pOneTouchPassTarget;           // offset 0x34, size 0x4
    } m_DesireReceivePassSharedVars;            // offset 0x328, size 0x38
    struct                                      /* @class$503Powerups_cpp */
    {
        // total size: 0x4
        cFielder* m_pSlideAttackTarget; // offset 0x0, size 0x4
    } m_DesireSlideAttackVars;          // offset 0x360, size 0x4
    struct                              /* @class$504Powerups_cpp */
    {
        // total size: 0x1
        bool bIsBallAwayFromCarrier;           // offset 0x0, size 0x1
    } m_DesireWindupForShotVars;               // offset 0x364, size 0x1
    Timer mtKickOffWaitTimer;                  // offset 0x368, size 0x4
    Timer m_tPowerupEffectTime;                // offset 0x36C, size 0x4
    ePowerUpType m_ePowerup;                   // offset 0x370, size 0x4
    int mnNumPowerups;                         // offset 0x374, size 0x4
    cFielder* m_pPowerupTarget;                // offset 0x378, size 0x4
    int m_nPowerupAnimID;                      // offset 0x37C, size 0x4
    Timer mtBombImpactTime;                    // offset 0x380, size 0x4
    nlVector3 mv3BombImpactLocation;           // offset 0x384, size 0xC
    float mfBombImpactRadius;                  // offset 0x390, size 0x4
    ePenaltyCardStatus m_ePenaltyCardStatus;   // offset 0x394, size 0x4
    cFielder* m_pMark;                         // offset 0x398, size 0x4
    cFielder* m_pMarker;                       // offset 0x39C, size 0x4
    eRole m_eRole;                             // offset 0x3A0, size 0x4
    AIPlay* m_pCurrentPlay;                    // offset 0x3A4, size 0x4
    nlVector3 m_v3DesiredPosition;             // offset 0x3A8, size 0xC
    float m_fDistanceToDesiredPosition;        // offset 0x3B4, size 0x4
    nlVector3 m_v3AccumDesiredPos;             // offset 0x3B8, size 0xC
    float m_fAccumDesiredPosWeight;            // offset 0x3C4, size 0x4
    AvoidController* m_pAvoidance;             // offset 0x3C8, size 0x4
    ShotMeter* m_pShotMeter;                   // offset 0x3CC, size 0x4
    eShootToScoreResult meS2SResult;           // offset 0x3D0, size 0x4
    bool mbCanKickoff;                         // offset 0x3D4, size 0x1
    bool mbWasHitByPowerupThisFrame;           // offset 0x3D5, size 0x1
    bool mbCaptShootToScoreEffectOn;           // offset 0x3D6, size 0x1
    unsigned long mThoughtHashCalcDesire;      // offset 0x3D8, size 0x4
    unsigned long mThoughtHashInitRunToNet;    // offset 0x3DC, size 0x4
    unsigned long mThoughtHashInitGetOpen;     // offset 0x3E0, size 0x4
    unsigned long mThoughtHashInitWindupPass;  // offset 0x3E4, size 0x4
    unsigned long mThoughtHashInitCutAndBreak; // offset 0x3E8, size 0x4

    static const int SlideAttackReactAnims[4];
    // static const int SlideAttackReactAnims[4] = {
    //     0x66,
    //     0x69,
    //     0x68,
    //     0x67,
    // };
}; // total size: 0x3EC

#endif // _FIELDER_H_
