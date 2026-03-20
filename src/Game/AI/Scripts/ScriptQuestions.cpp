#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/FormationDefines.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/AvoidController.h"
#include "Game/AI/ShotMeter.h"
#include "Game/AI/Fuzzy.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/GameTweaks.h"
#include "Game/Goalie.h"
#include "Game/CharacterTweaks.h"
#include "types.h"

extern cTeam* g_pCurrentlyUpdatingTeam;
extern cBall* g_pScriptBall;
extern cBall* g_pBall;
extern cFielder* g_pScriptCurrentFielder;
extern nlVector2 g_vStallingConfidenceTime;
extern nlVector2 g_vPassCloseToDoneConfidence;
extern cTeam* g_pScriptOtherTeam;
extern nlVector2 g_vOpenToAdjust;

inline float max_float(float a, float b)
{
    return (a >= b) ? a : b;
}

inline float min_float(float a, float b)
{
    return (a <= b) ? a : b;
}

static inline float IsPassInPlay(cBall* pBall)
{
    if (pBall->m_fTotalPassTime > 0.0f)
    {
        float fElapsedTime = pBall->m_tPassTargetTimer.GetSeconds() / pBall->m_fTotalPassTime;
        return (1.0f - fElapsedTime);
    }
    return 0.0f;
}

static inline float check_goalie(const Goalie* pGoalie, const eGoalieActionState actionState)
{
    return (actionState == GOALIEACTION_STS_RECOVER) || ((pGoalie->m_pBall == NULL) && (actionState != GOALIEACTION_PASS) && (actionState != GOALIEACTION_PASS_INTERCEPT) && (actionState != GOALIEACTION_MOVE) && (actionState != GOALIEACTION_MOVE_WB) && (actionState != GOALIEACTION_PASS_INTERCEPT) && (actionState != GOALIEACTION_PURSUE_BALL_CARRIER) && (actionState != GOALIEACTION_PURSUE_BALL_POUNCE) && (actionState != GOALIEACTION_LOOSEBALL_SETUP) && (actionState != GOALIEACTION_LOOSEBALL_CATCH) && (actionState != GOALIEACTION_LOOSEBALL_PICKUP) && (actionState != GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING) && (actionState != GOALIEACTION_LOOSEBALL_PURSUE_ROLLING)) ? 1.0f : 0.0f;
}

static inline float check_fielder(cFielder* pFielder)
{
    return pFielder->IsFrozen() || pFielder->IsFallenDown(25.0f) ? 1.0f : 0.0f;
}

/**
 * Offset/Address/Size: 0x5E44 | 0x800848CC | size: 0x30
 */
float CalcSelectChance(float fDifficultyChance, float fPlayerAttribute)
{
    float fScore = 0.0f;

    if (fDifficultyChance > 0.0f)
    {
        float f3 = 1.0f;
        float weight = g_pGame->m_pGameTweaks->fFielderAttributeWeight;
        f3 = f3 - weight;
        fScore = (fDifficultyChance * f3) + fPlayerAttribute * weight;
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x5E18 | 0x800848A0 | size: 0x2C
 */
float BallOwner(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    if (pPlayer->m_pBall != NULL)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5DCC | 0x80084854 | size: 0x4C
 */
float BallOwnerT(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    u8 isOwnerOnTeam = 0;
    cPlayer* pOwner = g_pBall->m_pOwner;
    if (pOwner != NULL && pOwner->m_pTeam == pTeam)
    {
        isOwnerOnTeam = 1;
    }

    return isOwnerOnTeam ? 1.0f : 0.0f;
}

/**
 * Offset/Address/Size: 0x5D9C | 0x80084824 | size: 0x30
 */
float LastBallOwner(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    if (g_pScriptBall->m_pPrevOwner == pPlayer)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5D58 | 0x800847E0 | size: 0x44
 */
float Striker(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsStriker())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5D14 | 0x8008479C | size: 0x44
 */
float Winger(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsWinger())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5CD0 | 0x80084758 | size: 0x44
 */
float Midfield(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsMidField())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5C8C | 0x80084714 | size: 0x44
 */
float Defence(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsDefense())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5C48 | 0x800846D0 | size: 0x44
 */
float Captain(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsCaptain())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5C1C | 0x800846A4 | size: 0x2C
 */
float GoalieType(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    if (pPlayer->m_eClassType == GOALIE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5BE0 | 0x80084668 | size: 0x3C
 */
float Marking(cFielder* pFielder, cPlayer* pPlayer)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    if (pFielder->m_pMark == pPlayer)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5BB0 | 0x80084638 | size: 0x30
 */
float OnTheirTeam(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (g_pScriptOtherTeam == pFielder->m_pTeam)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5B20 | 0x800845A8 | size: 0x90
 */
float OnScreen(cPlayer* pPlayer)
{
    u8 onScreenFlags[2];

    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    onScreenFlags[1] = 0;
    onScreenFlags[0] = onScreenFlags[1];

    if ((float)fabs(pPlayer->m_v3ScreenPosition.f.x) <= 1.0f)
    {
        if ((float)fabs(pPlayer->m_v3ScreenPosition.f.y) <= 1.0f)
        {
            onScreenFlags[0] = 1;
        }
    }

    if (onScreenFlags[0] != 0)
    {
        if ((float)fabs(pPlayer->m_v3ScreenPosition.f.z) <= 1.0f)
        {
            onScreenFlags[1] = 1;
        }
    }

    if (onScreenFlags[1] != 0)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5AD0 | 0x80084558 | size: 0x50
 */
float OnTheGround(cPlayer* pPlayer)
{
    if (pPlayer == nullptr)
    {
        return 0.0f;
    }

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    int jointIndex = pPlayer->m_nBip01JointIndex_0xA4;
    const nlVector3& jointPos = pPlayer->GetJointPosition(jointIndex);

    return NormalizeVal(jointPos.f.z, pFuzzyTweaks->vOnGroundConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x59EC | 0x80084474 | size: 0xE4
 */
float StrategicBallOwner(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder == g_pBall->m_pOwner)
    {
        return 1.0f;
    }

    float fWantsToReceivePass;
    if (pFielder == NULL)
    {
        fWantsToReceivePass = 0.0f;
    }
    else
    {
        eFielderDesireState desireState = pFielder->m_eFielderDesireState;
        fWantsToReceivePass = 0.0f;
        if (((s32)desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE) || ((s32)desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN) || ((s32)desireState == FIELDERDESIRE_ONETIMER))
        {
            fWantsToReceivePass = 1.0f;
        }
    }

    float fWantsToReceivePassCheck = 0.0f;
    if (fWantsToReceivePass != fWantsToReceivePassCheck)
    {
        return 0.8f; // @6031 value
    }

    eFielderDesireState desireState = pFielder->m_eFielderDesireState;
    if (desireState == FIELDERDESIRE_INTERCEPT_BALL)
    {
        return AbleToInterceptBall(pFielder);
    }

    if ((g_pBall->m_tShotTimer.m_uPackedTime != 0) && (pFielder == g_pBall->m_pPrevOwner))
    {
        return 0.4f; // @6199 value
    }

    cTeam* pTeam = pFielder->m_pTeam;
    if (pFielder->m_pTeam->mpCurrentSituation == SITUATION_LOOSE && desireState == FIELDERDESIRE_USER_CONTROLLED)
    {
        return AbleToInterceptBall(pFielder);
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x59A0 | 0x80084428 | size: 0x4C
 */
float UserControlled(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    u32 pad = (u32)pFielder->GetGlobalPad();
    // if (pFielder->GetGlobalPad() != NULL)
    if (((-pad | pad) >> 0x1F) != 0) // TODO: matches but is not clean...
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x5838 | 0x800842C0 | size: 0x168
 * TODO: 98.04% match - remaining diffs are GetClosestPoint temporary stack slot
 * ordering (0x10/0x1C) and float register allocation around second distance calc.
 */
float InPassingLane(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    cPlayer* pPassTarget = g_pScriptBall->m_pPassTarget;
    if (pPassTarget == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsOnSameTeam(pPassTarget))
    {
        return 0.0f;
    }

    cPlayer* pPassingTarget = g_pScriptBall->m_pPassTarget;
    if (g_pScriptBall->m_pPrevOwner == NULL || pPassingTarget == NULL)
    {
        return 0.0f;
    }

    nlPolar polar;
    nlCartesianToPolar(polar, g_pScriptBall->m_v3Velocity.f.x, g_pScriptBall->m_v3Velocity.f.y);
    float fVelocityMagnitude = polar.r;

    nlVector3 closestPoint = GetClosestPointOnLineABFromPointC(g_pScriptBall->m_v3Position, pPassingTarget->m_v3Position, pFielder->m_v3Position);

    float fScaledDistance;
    float fClosestY = closestPoint.f.y;
    float dy1 = g_pScriptBall->m_v3Position.f.y - fClosestY;
    float dx1 = g_pScriptBall->m_v3Position.f.x - closestPoint.f.x;
    fScaledDistance = (nlSqrt(dx1 * dx1 + dy1 * dy1, true) / fVelocityMagnitude) * pFielder->m_pTweaks->fRunningSpeed;

    float dy2 = pFielder->m_v3Position.f.y - fClosestY;
    float dx2 = pFielder->m_v3Position.f.x - closestPoint.f.x;
    return NormalizeVal(nlSqrt(dx2 * dx2 + dy2 * dy2, true), fScaledDistance + ((GameTweaks*)g_pGame->m_pFuzzyTweaks)->fShellExplodeChance, fScaledDistance);
}

/**
 * Offset/Address/Size: 0x581C | 0x800842A4 | size: 0x1C
 */
float Aggressive(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    FielderTweaks* pTweaks = (FielderTweaks*)pFielder->m_pTweaks;
    return pTweaks->fAggression;
}

/**
 * Offset/Address/Size: 0x5800 | 0x80084288 | size: 0x1C
 */
float Shooter(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    FielderTweaks* pTweaks = (FielderTweaks*)pFielder->m_pTweaks;
    return pTweaks->fShooting;
}

/**
 * Offset/Address/Size: 0x57E4 | 0x8008426C | size: 0x1C
 */
float Passer(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    FielderTweaks* pTweaks = (FielderTweaks*)pFielder->m_pTweaks;
    return pTweaks->fPassing;
}

/**
 * Offset/Address/Size: 0x57C8 | 0x80084250 | size: 0x1C
 */
float Deker(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    FielderTweaks* pTweaks = (FielderTweaks*)pFielder->m_pTweaks;
    return pTweaks->fDekeing;
}

/**
 * Offset/Address/Size: 0x579C | 0x80084224 | size: 0x2C
 */
float RepeatingLastDesire(cFielder* pFielder, eScriptFielderDesire desire)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if ((eFielderDesireState)desire == pFielder->m_ePrevFielderDesireState)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x54B4 | 0x80083F3C | size: 0x2E8
 */
float AbleToInterceptBall(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float fScore = 0.0f;

    float var_f1;
    if (pPlayer == NULL)
    {
        var_f1 = 0.0f;
    }
    else
    {
        var_f1 = 0.0f;
        if (pPlayer->m_eClassType == GOALIE)
        {
            Goalie* pGoalie = (Goalie*)pPlayer;
            bool canIntercept = true;
            eGoalieActionState goalieState = pGoalie->mGoalieActionState;
            int isRecover = (((int)GOALIEACTION_STS_RECOVER - goalieState) == 0);
            if ((isRecover & 0xFF) == 0)
            {
                bool isBusy = (pGoalie->m_pBall != NULL) || (goalieState == GOALIEACTION_PASS) || (goalieState == GOALIEACTION_PASS_INTERCEPT) || (goalieState == GOALIEACTION_MOVE) || (goalieState == GOALIEACTION_MOVE_WB) || (goalieState == GOALIEACTION_PASS_INTERCEPT) || (goalieState == GOALIEACTION_PURSUE_BALL_CARRIER) || (goalieState == GOALIEACTION_PURSUE_BALL_POUNCE) || (goalieState == GOALIEACTION_LOOSEBALL_SETUP) || (goalieState == GOALIEACTION_LOOSEBALL_CATCH) || (goalieState == GOALIEACTION_LOOSEBALL_PICKUP) || (goalieState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING) || (goalieState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                if (isBusy)
                {
                    canIntercept = false;
                }
            }
            var_f1 = canIntercept ? 1.0f : 0.0f;
        }
        else if (pPlayer->m_eClassType == FIELDER)
        {
            bool isDisabled = false;
            if (((cFielder*)pPlayer)->IsFrozen() || ((cFielder*)pPlayer)->IsFallenDown(25.0f))
            {
                isDisabled = true;
            }
            var_f1 = isDisabled ? 1.0f : 0.0f;
        }
    }

    float temp_cmp = 0.0f;
    if (var_f1 == temp_cmp)
    {
        if (pPlayer->m_pBall != NULL)
        {
            fScore = 1.0f;
        }
        else
        {
            int classType = pPlayer->m_eClassType;
            if (classType == FIELDER)
            {
                float temp_f31 = NormalizeVal(pPlayer->m_pTeam->mfBallInterceptTimes[pPlayer->m_ID], g_pGame->m_pFuzzyTweaks->vInterceptBallConfidenceTime);
                float temp_f0 = pPlayer->m_v3Position.f.x;
                float temp_f3 = g_pBall->m_v3Position.f.x - temp_f0;
                temp_f0 = pPlayer->m_v3Position.f.y;
                float temp_f1 = g_pBall->m_v3Position.f.y - temp_f0;
                float distance = nlSqrt((temp_f3 * temp_f3) + (temp_f1 * temp_f1), true);
                float normalizedDistance = NormalizeVal(distance, g_pGame->m_pFuzzyTweaks->vInterceptBallConfidenceDistance);
                float weight = g_pGame->m_pFuzzyTweaks->fInterceptBallScoreWeight;
                fScore = (temp_f31 * weight) + (normalizedDistance * (1.0f - weight));
            }
            else if (classType == GOALIE)
            {
                bool isBusy = (pPlayer->m_pBall != NULL) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_PASS) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_PASS_INTERCEPT) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_MOVE) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_MOVE_WB) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_PASS_INTERCEPT) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_PURSUE_BALL_CARRIER) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_PURSUE_BALL_POUNCE) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_LOOSEBALL_SETUP) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_LOOSEBALL_CATCH) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_LOOSEBALL_PICKUP) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING) || (((Goalie*)pPlayer)->mGoalieActionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                if (isBusy)
                {
                    float result;
                    if (pPlayer == NULL)
                    {
                        result = 0.0f;
                    }
                    else
                    {
                        float temp_f0 = pPlayer->m_v3Position.f.x;
                        float temp_f3 = g_pScriptBall->m_v3Position.f.x - temp_f0;
                        temp_f0 = pPlayer->m_v3Position.f.y;
                        float temp_f1 = g_pScriptBall->m_v3Position.f.y - temp_f0;
                        result = NormalizeVal(nlSqrt((temp_f3 * temp_f3) + (temp_f1 * temp_f1), true), g_pGame->m_pFuzzyTweaks->vCloseBallConfidenceDistance);
                    }
                    fScore = result;
                }
            }
        }
    }

    return fScore;
}

static inline bool check_goalie2(const Goalie* pGoalie, const eGoalieActionState actionState)
{
    bool result = true;
    if (actionState != (int)GOALIEACTION_STS_RECOVER)
    {
        bool isBusy = (pGoalie->m_pBall != NULL) || (actionState == GOALIEACTION_PASS) || (actionState == GOALIEACTION_PASS_INTERCEPT) || (actionState == GOALIEACTION_MOVE) || (actionState == GOALIEACTION_MOVE_WB) || (actionState == GOALIEACTION_PASS_INTERCEPT) || (actionState == GOALIEACTION_PURSUE_BALL_CARRIER) || (actionState == GOALIEACTION_PURSUE_BALL_POUNCE) || (actionState == GOALIEACTION_LOOSEBALL_SETUP) || (actionState == GOALIEACTION_LOOSEBALL_CATCH) || (actionState == GOALIEACTION_LOOSEBALL_PICKUP) || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING) || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
        if (isBusy)
        {
            result = false;
        }
    }
    return result;
}

/**
 * Offset/Address/Size: 0x52B0 | 0x80083D38 | size: 0x204
 */
float AbleToInterceptBallForSwapController(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    float fScore = 0.0f;

    float var_f1;
    if (pFielder == NULL)
    {
        var_f1 = 0.0f;
    }
    else
    {
        var_f1 = 0.0f;
        if (pFielder->m_eClassType == 3)
        {
            Goalie* pGoalie = (Goalie*)pFielder;
            bool result = true;
            eGoalieActionState goalieState = pGoalie->mGoalieActionState;
            int isRecover = (((int)GOALIEACTION_STS_RECOVER - goalieState) == 0);
            if ((isRecover & 0xFF) == 0)
            {
                bool isBusy = (pGoalie->m_pBall != NULL) || (goalieState == GOALIEACTION_PASS) || (goalieState == GOALIEACTION_PASS_INTERCEPT) || (goalieState == GOALIEACTION_MOVE) || (goalieState == GOALIEACTION_MOVE_WB) || (goalieState == GOALIEACTION_PASS_INTERCEPT) || (goalieState == GOALIEACTION_PURSUE_BALL_CARRIER) || (goalieState == GOALIEACTION_PURSUE_BALL_POUNCE) || (goalieState == GOALIEACTION_LOOSEBALL_SETUP) || (goalieState == GOALIEACTION_LOOSEBALL_CATCH) || (goalieState == GOALIEACTION_LOOSEBALL_PICKUP) || (goalieState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING) || (goalieState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                if (isBusy)
                {
                    result = false;
                }
            }
            var_f1 = result ? 1.0f : 0.0f;
        }
        else if (pFielder->m_eClassType == 2)
        {
            bool isFrozenOrDown = false;
            if (pFielder->IsFrozen() || pFielder->IsFallenDown(25.0f))
            {
                isFrozenOrDown = true;
            }
            var_f1 = isFrozenOrDown ? 1.0f : 0.0f;
        }
    }

    float temp_cmp = 0.0f;
    if (var_f1 == temp_cmp)
    {
        if (pFielder->m_pBall != NULL)
        {
            fScore = 1.0f;
        }
        else
        {
            float temp_f31 = NormalizeVal(pFielder->m_pTeam->mfBallInterceptTimes[pFielder->m_ID], g_pGame->m_pFuzzyTweaks->vInterceptBallConfidenceTime);
            float temp_f0 = pFielder->m_v3Position.f.x;
            float temp_f3 = g_pBall->m_v3Position.f.x - temp_f0;
            temp_f0 = pFielder->m_v3Position.f.y;
            float temp_f1 = g_pBall->m_v3Position.f.y - temp_f0;
            float distance = nlSqrt((temp_f3 * temp_f3) + (temp_f1 * temp_f1), true);
            float normalizedDistance = NormalizeVal(distance, g_pGame->m_pFuzzyTweaks->vInterceptBallConfidenceDistance);
            float weight = g_pGame->m_pFuzzyTweaks->fInterceptBallSwapControlerScoreWeight;
            fScore = (temp_f31 * weight) + (normalizedDistance * (1.0f - weight));
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x5228 | 0x80083CB0 | size: 0x88
 */
float AbleToUsePowerup(cFielder* pFielder, int powerupType)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    bool canUse = false;
    if (!pFielder->IsPlayingPowerupAnim())
    {
        const PowerUpTeamType& currentPowerup = pFielder->m_pTeam->GetCurrentPowerUp();
        if (powerupType == currentPowerup.eType)
        {
            canUse = true;
        }
    }

    return canUse ? 1.0f : 0.0f;
}

static inline float IsPowerupMatch(cFielder* pFielder, int powerupType)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    return (!pFielder->IsPlayingPowerupAnim() && (powerupType == pFielder->m_pTeam->GetCurrentPowerUp().eType)) ? 1.0f : 0.0f;
}

static inline float FielderOnGround(cFielder* pFielder, int powerupType)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    return NormalizeVal(pFielder->GetJointPosition(pFielder->m_nBip01JointIndex_0xA4).f.z, pFuzzyTweaks->vOnGroundConfidenceDistance);
}

static inline float GetPowerupUsageRandomChance(cFielder* pFielder, int powerupType)
{
    cTeam* pTeam = pFielder->m_pTeam;
    eSituation situation = pTeam->mpCurrentSituation;
    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(pTeam->m_nSide);
    return RandomChance(pSkillTweaks->PowerupUsageChance[situation][powerupType]);
}

/**
 * Offset/Address/Size: 0x50E8 | 0x80083B70 | size: 0x140
 */
float LikelyToUsePowerup(cFielder* pFielder, int powerupType)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    float fRandomChance = GetPowerupUsageRandomChance(pFielder, powerupType);
    float fPowerupMatch = IsPowerupMatch(pFielder, powerupType);
    float fNormalizedZ = FielderOnGround(pFielder, powerupType);

    fPowerupMatch = min_float(fPowerupMatch, fRandomChance);
    if (fNormalizedZ <= fPowerupMatch)
    {
        return fNormalizedZ;
    }

    return fPowerupMatch;
}

/**
 * Offset/Address/Size: 0x5080 | 0x80083B08 | size: 0x68
 */
float CloseToBall(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float dx = g_pScriptBall->m_v3Position.f.x - pPlayer->m_v3Position.f.x;
    float dy = g_pScriptBall->m_v3Position.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vCloseBallConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x5018 | 0x80083AA0 | size: 0x68
 */
float NearToBall(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float dx = g_pScriptBall->m_v3Position.f.x - pPlayer->m_v3Position.f.x;
    float dy = g_pScriptBall->m_v3Position.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vNearBallConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4FB0 | 0x80083A38 | size: 0x68
 */
float FarToBall(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float dx = g_pScriptBall->m_v3Position.f.x - pPlayer->m_v3Position.f.x;
    float dy = g_pScriptBall->m_v3Position.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vFarBallConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4F34 | 0x800839BC | size: 0x7C
 */
float CloseToMyNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIDefNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vCloseNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4EB8 | 0x80083940 | size: 0x7C
 */
float NearToMyNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIDefNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vNearNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4E3C | 0x800838C4 | size: 0x7C
 */
float FarToMyNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIDefNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vFarNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4DC0 | 0x80083848 | size: 0x7C
 */
float CloseToTheirNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIOffNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vCloseNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4D44 | 0x800837CC | size: 0x7C
 */
float NearToTheirNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIOffNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vNearNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4CC8 | 0x80083750 | size: 0x7C
 */
float FarToTheirNet(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIOffNetLocation(NULL);

    float dx = netLocation.f.x - pPlayer->m_v3Position.f.x;
    float dy = netLocation.f.y - pPlayer->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(distance, pFuzzyTweaks->vFarNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x4934 | 0x800833BC | size: 0x394
 */
float Pressured(cFielder*)
{
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x4490 | 0x80082F18 | size: 0x4A4
 */
float Attacked(cFielder*)
{
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x440C | 0x80082E94 | size: 0x84
 */
float AvoidingPowerups(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    nlVector3 repulsionVec = pFielder->m_pAvoidance->GetLastRepulsionVector(AVOID_POWERUPS);

    float magnitude = nlSqrt(repulsionVec.f.x * repulsionVec.f.x + repulsionVec.f.y * repulsionVec.f.y + repulsionVec.f.z * repulsionVec.f.z, true);

    return NormalizeVal(magnitude, g_pGame->m_pFuzzyTweaks->vAvoidPowerupsRepulsionConfidence);
}

/**
 * Offset/Address/Size: 0x43C8 | 0x80082E50 | size: 0x44
 */
float Invincible(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsInvincible())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x4294 | 0x80082D1C | size: 0x134
 */
float Incapacitated(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float fScore = 0.0f;

    if (pPlayer->m_eClassType == GOALIE)
    {
        Goalie* pGoalie = (Goalie*)pPlayer;
        eGoalieActionState actionState = pGoalie->mGoalieActionState;
        fScore = check_goalie(pGoalie, actionState);
    }
    else if (pPlayer->m_eClassType == FIELDER)
    {
        cFielder* pFielder = (cFielder*)pPlayer;
        fScore = check_fielder(pFielder);
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x4250 | 0x80082CD8 | size: 0x44
 */
float Frozen(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsFrozen())
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x4208 | 0x80082C90 | size: 0x48
 */
float FallenDown(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder->IsFallenDown(0.0f))
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x3FA0 | 0x80082A28 | size: 0x268
 * TODO: 95.81% match - remaining FP temp register allocation (f0/f2 swap for 65536.0f constant),
 *   fGoalDeltaX fmuls operand order, and goalie delta scheduling (compiler computes DeltaY before DeltaX)
 */
float LikelyToScoreFromPosition(const nlVector3& v3Position, const nlVector3& v3GoaliePosition, const cNet* pNet, bool)
{
    float fNetHalfWidth;
    float fGoalLine;
    unsigned short aNetAngle;
    unsigned short aGoalieAngle1;
    unsigned short aGoalieAngle2;
    unsigned short aNetOpenAngle;
    unsigned short aNetOpenAngle2;
    float fNetOpenScore;
    float fGoalieDeltaY;
    float fGoalieDeltaX;

    float fSideSign = pNet->m_sideSign;
    float fOpenAngle = g_pGame->m_pGameTweaks->unk2D4;
    fOpenAngle *= 65536.0f;
    fNetHalfWidth = 0.5f * cNet::m_fNetWidth;
    aNetAngle = ((s32)fOpenAngle) / 360;

    fGoalLine = pNet->GetGoalLineX();

    float fGoalDeltaX = (fGoalLine - v3Position.f.x) * fSideSign;
    float fPositionY = v3Position.f.y;

    float fUpperAngle = nlATan2f(fNetHalfWidth - fPositionY, fGoalDeltaX);
    float fLowerAngle = nlATan2f(-fNetHalfWidth - fPositionY, fGoalDeltaX);

    aGoalieAngle1 = (u16)(s32)(10430.378f * fLowerAngle);
    aGoalieAngle2 = (u16)(s32)(10430.378f * fUpperAngle);
    aNetOpenAngle = aGoalieAngle2 - aGoalieAngle1;

    fGoalieDeltaX = v3GoaliePosition.f.x - v3Position.f.x;
    fGoalieDeltaX *= fSideSign;
    fGoalieDeltaY = v3GoaliePosition.f.y - fPositionY;

    float fGoalieCenterAngle = nlATan2f(fGoalieDeltaY, fGoalieDeltaX);
    float fGoalieHalfAngle = nlATan2f(0.5f, nlSqrt(fGoalieDeltaX * fGoalieDeltaX + fGoalieDeltaY * fGoalieDeltaY, true));

    u16 aGoalieHalfOpen = (u16)(s32)(10430.378f * fGoalieHalfAngle);
    u16 aGoalieCenter = (u16)(s32)(10430.378f * fGoalieCenterAngle);

    u16 aMinOpen = (aGoalieCenter - aGoalieHalfOpen) - aGoalieAngle1;
    u16 aMaxOpen = (aGoalieCenter + aGoalieHalfOpen) - aGoalieAngle1;

    if (aMinOpen < aNetOpenAngle && aMaxOpen < aNetOpenAngle)
    {
        u16 aGap = aNetOpenAngle - aMaxOpen;
        aNetOpenAngle2 = aGap;
        if (aMinOpen >= aGap)
        {
            aNetOpenAngle2 = aMinOpen;
        }
    }
    else if (aMinOpen < aNetOpenAngle)
    {
        aNetOpenAngle2 = aMinOpen;
    }
    else if (aMaxOpen < aNetOpenAngle)
    {
        aNetOpenAngle2 = aNetOpenAngle - aMaxOpen;
    }
    else if (aMinOpen > aMaxOpen && aMaxOpen > aNetOpenAngle)
    {
        aNetOpenAngle2 = 0;
    }
    else
    {
        aNetOpenAngle2 = aNetOpenAngle;
    }

    fNetOpenScore = InterpolateRangeClamped(0.0f, 1.0f, 0.0f, (float)aNetAngle, (float)aNetOpenAngle2);
    return fNetOpenScore;
}

/**
 * Offset/Address/Size: 0x3F28 | 0x800829B0 | size: 0x78
 */
float PlayerShotDistance(cFielder* pFielder)
{
    cNet* pOtherNet = pFielder->m_pTeam->GetOtherNet();

    nlVector3 v;
    nlVec3Set(v, pFielder->m_v3Position.f.x - pOtherNet->m_baseLocation.f.x, pFielder->m_v3Position.f.y - pOtherNet->m_baseLocation.f.y, pFielder->m_v3Position.f.z - pOtherNet->m_baseLocation.f.z);

    return NormalizeVal(nlSqrt((v.f.x * v.f.x) + (v.f.y * v.f.y) + (v.f.z * v.f.z), true), g_pGame->m_pFuzzyTweaks->vPlayerShotDistance);
}

/**
 * Offset/Address/Size: 0x3E9C | 0x80082924 | size: 0x8C
 */
float LikelyToScore(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pFielder->m_pTeam->GetOtherTeam();

    bool bIsChipShot = false;
    if ((pFielder->mActionShotVars.bIsChipShot != 0) || (pFielder->mActionLooseBallShotVars.bIsChipShot != 0))
    {
        bIsChipShot = true;
    }

    cNet* pNet = pOtherTeam->m_pNet;
    cPlayer* pGoalie = pOtherTeam->GetGoalie();

    return LikelyToScoreFromPosition(pFielder->m_v3Position, pGoalie->m_v3Position, pNet, bIsChipShot);
}

/**
 * Offset/Address/Size: 0x3D40 | 0x800827C8 | size: 0x15C
 */
float GoalieOutOfPosition(cFielder* pFielder)
{
    nlVector3 goalieNetPos;
    cPlayer* pGoalie;

    if (pFielder == NULL)
    {
        return 0.0f;
    }

    pGoalie = pFielder->m_pTeam->GetOtherTeam()->GetGoalie();
    float halfNetWidth = 0.5f * cNet::m_fNetWidth;
    goalieNetPos = pGoalie->m_v3Position;
    goalieNetPos.f.x = pGoalie->m_pTeam->m_pNet->m_baseLocation.f.x;

    float goalieY = goalieNetPos.f.y;
    if (goalieY < -halfNetWidth)
    {
        goalieNetPos.f.y = -halfNetWidth;
    }
    else if (goalieY > halfNetWidth)
    {
        goalieNetPos.f.y = halfNetWidth;
    }

    const nlVector3& offNetLocation = pFielder->GetAIOffNetLocation(NULL);

    float dx1 = pFielder->m_v3Position.f.x - offNetLocation.f.x;
    float dy1 = pFielder->m_v3Position.f.y - offNetLocation.f.y;
    float fielderDistance = nlSqrt(dx1 * dx1 + dy1 * dy1, true);

    float dx2 = pGoalie->m_v3Position.f.x - goalieNetPos.f.x;
    float dy2 = pGoalie->m_v3Position.f.y - goalieNetPos.f.y;
    float goalieDistance = nlSqrt(dx2 * dx2 + dy2 * dy2, true);

    if (!((double)fielderDistance > 0.0))
    {
        fielderDistance = 0.1f;
    }

    if (!((double)goalieDistance > 0.0))
    {
        goalieDistance = 0.1f;
    }

    return NormalizeVal(goalieDistance / fielderDistance, g_pGame->m_pFuzzyTweaks->vGoalieOutOfPositionDistance);
}

/**
 * Offset/Address/Size: 0x3C18 | 0x800826A0 | size: 0x128
 */
float PositionIsInFrontOfNet(const nlVector3& position, const cNet* pNet)
{
    float sideSign;
    nlVector3 diff;
    nlVec3Set(diff,
        position.f.x - pNet->m_baseLocation.f.x,
        position.f.y - pNet->m_baseLocation.f.y,
        position.f.z - pNet->m_baseLocation.f.z);
    sideSign = pNet->m_sideSign;
    nlVec3Scale(diff, diff, sideSign);

    nlPolar polar;
    nlCartesianToPolar(polar, diff);

    float angleRad = 0.005493164f * (u16)(polar.a - 0x8000);
    if (angleRad > 180.0f)
    {
        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        float complementaryMidAngle = 360.0f - pFuzzyTweaks->fFrontOfNetMidAngle;
        if (angleRad < complementaryMidAngle)
        {
            return InterpolateRangeClamped(0.0f, pFuzzyTweaks->fFrontOfNetMidScore, 360.0f - pFuzzyTweaks->fFrontOfNetMaxAngle, complementaryMidAngle, angleRad);
        }
        return InterpolateRangeClamped(1.0f, pFuzzyTweaks->fFrontOfNetMidScore, 360.0f, complementaryMidAngle, angleRad);
    }

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    float midAngle = pFuzzyTweaks->fFrontOfNetMidAngle;
    if (angleRad > midAngle)
    {
        return InterpolateRangeClamped(0.0f, pFuzzyTweaks->fFrontOfNetMidScore, pFuzzyTweaks->fFrontOfNetMaxAngle, midAngle, angleRad);
    }
    return InterpolateRangeClamped(1.0f, pFuzzyTweaks->fFrontOfNetMidScore, 0.0f, midAngle, angleRad);
}

/**
 * Offset/Address/Size: 0x3ACC | 0x80082554 | size: 0x14C
 */
float InFrontOfTheirNet(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pFielder->m_pTeam->GetOtherTeam();
    cNet* pNet = pOtherTeam->m_pNet;

    nlVector3 diff;
    nlVec3Set(diff,
        pFielder->m_v3Position.f.x - pNet->m_baseLocation.f.x,
        pFielder->m_v3Position.f.y - pNet->m_baseLocation.f.y,
        pFielder->m_v3Position.f.z - pNet->m_baseLocation.f.z);

    float sideSign = pNet->m_sideSign;
    nlVec3Scale(diff, diff, sideSign);

    nlPolar polar;
    nlCartesianToPolar(polar, diff);

    float angleRad = 0.005493164f * (u16)(polar.a - 0x8000);

    if (angleRad > 180.0f)
    {
        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        float complementaryMidAngle = 360.0f - pFuzzyTweaks->fFrontOfNetMidAngle;
        if (angleRad < complementaryMidAngle)
        {
            return InterpolateRangeClamped(0.0f, pFuzzyTweaks->fFrontOfNetMidScore, 360.0f - pFuzzyTweaks->fFrontOfNetMaxAngle, complementaryMidAngle, angleRad);
        }
        return InterpolateRangeClamped(1.0f, pFuzzyTweaks->fFrontOfNetMidScore, 360.0f, complementaryMidAngle, angleRad);
    }

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    float midAngle = pFuzzyTweaks->fFrontOfNetMidAngle;
    if (angleRad > midAngle)
    {
        return InterpolateRangeClamped(0.0f, pFuzzyTweaks->fFrontOfNetMidScore, pFuzzyTweaks->fFrontOfNetMaxAngle, midAngle, angleRad);
    }
    return InterpolateRangeClamped(1.0f, pFuzzyTweaks->fFrontOfNetMidScore, 0.0f, midAngle, angleRad);
}

/**
 * Offset/Address/Size: 0x36FC | 0x80082184 | size: 0x3D0
 */
void OnBreakaway(cFielder*)
{
}

/**
 * Offset/Address/Size: 0x32DC | 0x80081D64 | size: 0x420
 */
float OpenToPosition(const nlVector3&, const nlVector3&, const cTeam*, const cPlayer*, const cPlayer*, bool)
{
    FORCE_DONT_INLINE;
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x3030 | 0x80081AB8 | size: 0x2AC
 * TODO: 99.91% match - remaining 1 scored diff is SDA label ordering (offset 60 i diff)
 */
float OpenPosition(const nlVector3& v3Position, cTeam* pOpponentTeam, cPlayer* pCurrentPlayer, const nlVector2* pOpenRadius)
{
    f32 fTotalScore;
    f32 fWeight;
    cTeam* pMyTeam;
    int i;

    fTotalScore = 0.0f;
    fWeight = 1.0f;

    if (pOpenRadius == NULL)
    {
        pOpenRadius = &g_pGame->m_pFuzzyTweaks->vOpenRadius;
    }

    pMyTeam = pOpponentTeam->GetOtherTeam();

    for (i = 0; i < 5; i++)
    {
        cPlayer* pPlayers[2] = { NULL, NULL };
        pPlayers[0] = pOpponentTeam->GetPlayer(i);

        u8 bCheckMyTeam = 0;
        if (pCurrentPlayer != NULL)
        {
            if (i != pCurrentPlayer->m_ID)
            {
                bCheckMyTeam = 1;
            }
        }

        cPlayer* pMyTeamPlayer;
        if (bCheckMyTeam)
        {
            pMyTeamPlayer = pMyTeam->GetPlayer(i);
        }
        else
        {
            pMyTeamPlayer = NULL;
        }
        pPlayers[1] = pMyTeamPlayer;

        for (int i_player = 0; i_player < 2; i_player++)
        {
            u8 isIncap;
            cPlayer* pPlayer = pPlayers[i_player];
            if (pPlayer == NULL)
            {
                continue;
            }

            f32 fIncapacitated;
            if (pPlayer == NULL)
            {
                fIncapacitated = 0.0f;
            }
            else
            {
                fIncapacitated = 0.0f;
                if (pPlayer->m_eClassType == GOALIE)
                {
                    Goalie* pGoalie = (Goalie*)pPlayer;
                    bool result = true;
                    eGoalieActionState actionState = pGoalie->mGoalieActionState;
                    int isRecover = (((int)GOALIEACTION_STS_RECOVER - (int)actionState) == 0);

                    if ((isRecover & 0xFF) == 0)
                    {
                        bool isBusy = (pGoalie->m_pBall != NULL)
                                   || (actionState == GOALIEACTION_PASS)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_MOVE)
                                   || (actionState == GOALIEACTION_MOVE_WB)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_CARRIER)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_POUNCE)
                                   || (actionState == GOALIEACTION_LOOSEBALL_SETUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_CATCH)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PICKUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                        if (isBusy)
                        {
                            result = false;
                        }
                    }

                    fIncapacitated = result ? 1.0f : 0.0f;
                }
                else if (pPlayer->m_eClassType == FIELDER)
                {
                    isIncap = 0;
                    if (((cFielder*)pPlayer)->IsFrozen() || ((cFielder*)pPlayer)->IsFallenDown(25.0f))
                    {
                        isIncap = 1;
                    }

                    fIncapacitated = isIncap ? 1.0f : 0.0f;
                }
            }

            if (!fIncapacitated)
            {
                f32 dx = v3Position.f.x - pPlayers[i_player]->m_v3Position.f.x;
                f32 dy = v3Position.f.y - pPlayers[i_player]->m_v3Position.f.y;
                f32 dist = nlSqrt(dx * dx + dy * dy, true);
                f32 normalized = NormalizeVal(dist, *pOpenRadius);
                if (normalized > 0.0f)
                {
                    fTotalScore += fWeight * normalized;
                    fWeight *= 0.5f;
                }
            }
        }
    }

    return min_float(max_float(1.0f - fTotalScore, 0.0f), 1.0f);
}

/**
 * Offset/Address/Size: 0x2D84 | 0x8008180C | size: 0x2AC
 * TODO: 96.23% match - remaining diffs are register allocation/layout around
 * pOpenRadius/i_player/pPlayer and final clamp/load order scheduling.
 */
/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x2AC
 * TODO: 99.97% match - remaining 1 scored diff is SDA label ordering (offset 60 i diff), unfixable in scratch
 */
float WideOpenPosition(const nlVector3& v3Position, cTeam* pOpponentTeam, cPlayer* pCurrentPlayer)
{
    f32 fWeight;
    f32 fTotalScore;
    fTotalScore = 0.0f;
    fWeight = 1.0f;
    int i_player;
    int i;
    cTeam* pMyTeam;
    const nlVector2* pOpenRadius = &g_pGame->m_pFuzzyTweaks->vWideOpenRadius;

    if (pOpenRadius == NULL)
    {
        pOpenRadius = &g_pGame->m_pFuzzyTweaks->vOpenRadius;
    }

    pMyTeam = pOpponentTeam->GetOtherTeam();

    for (i = 0; i < 5; i++)
    {
        cPlayer* pPlayers[2] = { NULL, NULL };
        pPlayers[0] = pOpponentTeam->GetPlayer(i);

        u8 bCheckMyTeam = 0;
        if (pCurrentPlayer != NULL)
        {
            if (i != pCurrentPlayer->m_ID)
            {
                bCheckMyTeam = 1;
            }
        }

        cPlayer* pMyTeamPlayer;
        if (bCheckMyTeam)
        {
            pMyTeamPlayer = pMyTeam->GetPlayer(i);
        }
        else
        {
            pMyTeamPlayer = NULL;
        }
        pPlayers[1] = pMyTeamPlayer;

        cPlayer** ppPlayer = pPlayers;
        for (i_player = 0; i_player < 2; i_player++, ppPlayer++)
        {
            u8 isIncap;
            cPlayer* pPlayer = *ppPlayer;
            if (pPlayer == NULL)
            {
                continue;
            }

            f32 fIncapacitated;
            if (pPlayer == NULL)
            {
                fIncapacitated = 0.0f;
            }
            else
            {
                fIncapacitated = 0.0f;
                if (pPlayer->m_eClassType == GOALIE)
                {
                    Goalie* pGoalie = (Goalie*)pPlayer;
                    bool result = true;
                    eGoalieActionState actionState = pGoalie->mGoalieActionState;
                    int isRecover = (((int)GOALIEACTION_STS_RECOVER - (int)actionState) == 0);

                    if ((isRecover & 0xFF) == 0)
                    {
                        bool isBusy = (pGoalie->m_pBall != NULL)
                                   || (actionState == GOALIEACTION_PASS)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_MOVE)
                                   || (actionState == GOALIEACTION_MOVE_WB)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_CARRIER)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_POUNCE)
                                   || (actionState == GOALIEACTION_LOOSEBALL_SETUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_CATCH)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PICKUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                        if (isBusy)
                        {
                            result = false;
                        }
                    }

                    fIncapacitated = result ? 1.0f : 0.0f;
                }
                else if (pPlayer->m_eClassType == FIELDER)
                {
                    isIncap = 0;
                    if (((cFielder*)pPlayer)->IsFrozen() || ((cFielder*)pPlayer)->IsFallenDown(25.0f))
                    {
                        isIncap = 1;
                    }

                    fIncapacitated = isIncap ? 1.0f : 0.0f;
                }
            }

            if (fIncapacitated == 0.0f)
            {
                f32 dx = v3Position.f.x - (*ppPlayer)->m_v3Position.f.x;
                f32 dy = v3Position.f.y - (*ppPlayer)->m_v3Position.f.y;
                f32 dist = nlSqrt(dx * dx + dy * dy, true);
                f32 normalized = NormalizeVal(dist, *pOpenRadius);
                if (normalized > 0.0f)
                {
                    fTotalScore += fWeight * normalized;
                    fWeight *= 0.5f;
                }
            }
        }
    }

    return min_float(max_float(1.0f - fTotalScore, 0.0f), 1.0f);
}

/**
 * Offset/Address/Size: 0x2B10 | 0x80081598 | size: 0x274
 * TODO: 91.72% match - remaining blockers are f30/f31 load order, inner-loop index/pointer
 * register swap (r31/r27), and NULL-check/control-flow ordering around the inner loop header.
 */
static inline float check_goalie_local(const Goalie* pGoalie, const eGoalieActionState actionState)
{
    bool result = true;
    int isRecover = (((int)GOALIEACTION_STS_RECOVER - (int)actionState) == 0);

    if ((isRecover & 0xFF) == 0)
    {
        bool isBusy = (pGoalie->m_pBall != NULL) || (actionState == GOALIEACTION_PASS)
                   || (actionState == GOALIEACTION_PASS_INTERCEPT) || (actionState == GOALIEACTION_MOVE)
                   || (actionState == GOALIEACTION_MOVE_WB) || (actionState == GOALIEACTION_PASS_INTERCEPT)
                   || (actionState == GOALIEACTION_PURSUE_BALL_CARRIER)
                   || (actionState == GOALIEACTION_PURSUE_BALL_POUNCE)
                   || (actionState == GOALIEACTION_LOOSEBALL_SETUP)
                   || (actionState == GOALIEACTION_LOOSEBALL_CATCH)
                   || (actionState == GOALIEACTION_LOOSEBALL_PICKUP)
                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING)
                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
        if (isBusy)
        {
            result = false;
        }
    }

    return result ? 1.0f : 0.0f;
}

static inline float check_fielder_local(cFielder* pFielder)
{
    u8 isIncap = 0;
    if (pFielder->IsFrozen() || pFielder->IsFallenDown(25.0f))
    {
        isIncap = 1;
    }

    return isIncap ? 1.0f : 0.0f;
}

float Open(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    int i;
    cTeam* pOtherTeam = pFielder->m_pTeam->GetOtherTeam();
    f32 fWeight = 1.0f;
    f32 fTotal = 0.0f;
    const nlVector2* pOpenRadius = &g_pGame->m_pFuzzyTweaks->vOpenRadius;

    pOtherTeam->GetOtherTeam();

    for (i = 0; i < 5; i++)
    {
        cPlayer* pPlayers[2] = { pOtherTeam->GetPlayer(i), NULL };

        for (int j = 0; j < 2; j++)
        {
            cPlayer* pPlayer = pPlayers[j];
            if (pPlayer == NULL)
            {
                continue;
            }

            f32 fIncapacitated = 0.0f;
            if (pPlayer == NULL)
            {
                fIncapacitated = 0.0f;
            }
            else if (pPlayer->m_eClassType == GOALIE)
            {
                Goalie* pGoalie = (Goalie*)pPlayer;
                eGoalieActionState actionState = pGoalie->mGoalieActionState;
                fIncapacitated = check_goalie_local(pGoalie, actionState);
            }
            else if (pPlayer->m_eClassType == FIELDER)
            {
                fIncapacitated = check_fielder_local((cFielder*)pPlayer);
            }

            if (fIncapacitated == 0.0f)
            {
                cPlayer* pDistPlayer = pPlayers[j];
                f32 dy = pFielder->m_v3Position.f.y - pDistPlayer->m_v3Position.f.y;
                f32 dx = pFielder->m_v3Position.f.x - pDistPlayer->m_v3Position.f.x;
                f32 dist = nlSqrt(dx * dx + dy * dy, true);
                f32 normalized = NormalizeVal(dist, *pOpenRadius);
                if (normalized > 0.0f)
                {
                    fTotal += fWeight * normalized;
                    fWeight *= 0.5f;
                }
            }
        }
    }

    f32 fResult = 1.0f - fTotal;
    if (fResult >= 0.0f)
    {
    }
    else
    {
        fResult = 0.0f;
    }

    if (fResult <= 1.0f)
    {
    }
    else
    {
        fResult = 1.0f;
    }

    return fResult;
}

/**
 * Offset/Address/Size: 0x2854 | 0x800812DC | size: 0x2BC
 * TODO: 99.97% match - remaining 1 scored diff is SDA label ordering at stack init (@1538+0x4 load).
 */
float WideOpen(cFielder* pFielder)
{
    cTeam* pOtherTeam;
    f32 fWeight;
    f32 fTotal;
    int i_player;
    int i;
    cTeam* pMyTeam;
    const nlVector2* pOpenRadius;

    if (pFielder == NULL)
    {
        return 0.0f;
    }

    pOtherTeam = pFielder->m_pTeam->GetOtherTeam();
    fTotal = 0.0f;
    fWeight = 1.0f;
    pOpenRadius = &g_pGame->m_pFuzzyTweaks->vWideOpenRadius;

    if (pOpenRadius == NULL)
    {
        pOpenRadius = &g_pGame->m_pFuzzyTweaks->vOpenRadius;
    }

    pMyTeam = pOtherTeam->GetOtherTeam();

    for (i = 0; i < 5; i++)
    {
        cPlayer* pPlayers[2] = { NULL, NULL };
        pPlayers[0] = pOtherTeam->GetPlayer(i);

        u8 bCheckMyTeam = 0;
        if (pFielder != NULL)
        {
            if (i != pFielder->m_ID)
            {
                bCheckMyTeam = 1;
            }
        }

        cPlayer* pMyTeamPlayer;
        if (bCheckMyTeam)
        {
            pMyTeamPlayer = pMyTeam->GetPlayer(i);
        }
        else
        {
            pMyTeamPlayer = NULL;
        }
        pPlayers[1] = pMyTeamPlayer;

        cPlayer** ppPlayer = pPlayers;
        for (i_player = 0; i_player < 2; i_player++, ppPlayer++)
        {
            u8 isIncap;
            cPlayer* pPlayer = *ppPlayer;
            if (pPlayer == NULL)
            {
                continue;
            }

            f32 fIncapacitated;
            if (pPlayer == NULL)
            {
                fIncapacitated = 0.0f;
            }
            else
            {
                fIncapacitated = 0.0f;
                if (pPlayer->m_eClassType == GOALIE)
                {
                    Goalie* pGoalie = (Goalie*)pPlayer;
                    bool result = true;
                    eGoalieActionState actionState = pGoalie->mGoalieActionState;
                    int isRecover = (((int)GOALIEACTION_STS_RECOVER - (int)actionState) == 0);

                    if ((isRecover & 0xFF) == 0)
                    {
                        bool isBusy = (pGoalie->m_pBall != NULL)
                                   || (actionState == GOALIEACTION_PASS)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_MOVE)
                                   || (actionState == GOALIEACTION_MOVE_WB)
                                   || (actionState == GOALIEACTION_PASS_INTERCEPT)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_CARRIER)
                                   || (actionState == GOALIEACTION_PURSUE_BALL_POUNCE)
                                   || (actionState == GOALIEACTION_LOOSEBALL_SETUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_CATCH)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PICKUP)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_BOUNCING)
                                   || (actionState == GOALIEACTION_LOOSEBALL_PURSUE_ROLLING);
                        if (isBusy)
                        {
                            result = false;
                        }
                    }

                    fIncapacitated = result ? 1.0f : 0.0f;
                }
                else if (pPlayer->m_eClassType == FIELDER)
                {
                    isIncap = 0;
                    if (((cFielder*)pPlayer)->IsFrozen() || ((cFielder*)pPlayer)->IsFallenDown(25.0f))
                    {
                        isIncap = 1;
                    }

                    fIncapacitated = isIncap ? 1.0f : 0.0f;
                }
            }

            if (!fIncapacitated)
            {
                f32 ySelf = pFielder->m_v3Position.f.x;
                f32 yOther = (*ppPlayer)->m_v3Position.f.x;
                f32 xSelf = pFielder->m_v3Position.f.y;
                f32 dy = ySelf - yOther;
                f32 dx = xSelf - (*ppPlayer)->m_v3Position.f.y;
                f32 dist = nlSqrt(dy * dy + dx * dx, true);
                f32 normalized = NormalizeVal(dist, *pOpenRadius);
                if (normalized > 0.0f)
                {
                    fTotal += fWeight * normalized;
                    fWeight *= 0.5f;
                }
            }
        }
    }

    return min_float(max_float(1.0f - fTotal, 0.0f), 1.0f);
}

/**
 * Offset/Address/Size: 0x27E0 | 0x80081268 | size: 0x74
 */
float OpenToTheirNet(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pFielder->m_pTeam->GetOtherTeam();
    // nlVector3 netLocation = pFielder->GetAIOffNetLocation(NULL);

    return OpenToPosition(pFielder->m_v3Position, pFielder->GetAIOffNetLocation(NULL), pOtherTeam, pFielder, NULL, true);
}

/**
 * Offset/Address/Size: 0x2780 | 0x80081208 | size: 0x60
 */
float OpenToMyNet(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    return OpenToPosition(pFielder->m_v3Position, pFielder->GetAIDefNetLocation(NULL), NULL, pFielder, NULL, true);
}

static float InBetween(const nlVector3& v3InBetweenPos, const nlVector3& v3A, const nlVector3& v3B)
{
    nlVector3 v3Intercept = GetClosestPointOnLineABFromPointC(v3A, v3InBetweenPos, v3B);

    bool atA = (v3A.f.x == v3Intercept.f.x && v3A.f.y == v3Intercept.f.y && v3A.f.z == v3Intercept.f.z);
    if (atA)
    {
        goto ReturnZero;
    }

    {
        bool atInBetween = (v3InBetweenPos.f.x == v3Intercept.f.x && v3InBetweenPos.f.y == v3Intercept.f.y && v3InBetweenPos.f.z == v3Intercept.f.z);
        if (!atInBetween)
        {
            goto Compute;
        }
    }

ReturnZero:
    return 0.0f;

Compute:
    float dx1 = v3A.f.x - v3Intercept.f.x;
    float dy1 = v3A.f.y - v3Intercept.f.y;
    float distA = nlSqrt(dx1 * dx1 + dy1 * dy1, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    float fMaxConeWidth = InterpolateRangeClamped(pFuzzyTweaks->vInBetweenConeWidth, pFuzzyTweaks->vInBetweenInterceptRange, distA);

    float dy2 = v3Intercept.f.y - v3B.f.y;
    float dx2 = v3Intercept.f.x - v3B.f.x;

    return InterpolateRangeClamped(1.0f, 0.0f, 0.0f, fMaxConeWidth, nlSqrt(dx2 * dx2 + dy2 * dy2, true));
}

/**
 * Offset/Address/Size: 0x25DC | 0x80081064 | size: 0x1A4
 */
float InBetweenMyNetAnd(cFielder* pFielder1, cFielder* pFielder2)
{
    if (pFielder1 == NULL)
    {
        return 0.0f;
    }
    if (pFielder2 == NULL)
    {
        return 0.0f;
    }

    cNet* pNet = pFielder1->m_pTeam->m_pNet;
    nlVector3* fielderPos = &pFielder1->m_v3Position;
    nlVector3* targetPos = &pFielder2->m_v3Position;
    nlVector3* netPos = &pNet->m_baseLocation;

    return InBetween(*netPos, *targetPos, *fielderPos);
}

/**
 * Offset/Address/Size: 0x2438 | 0x80080EC0 | size: 0x1A4
 */
float InBetweenMyNetAnd(cFielder* pFielder, cBall* pBall)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }
    if (pBall == NULL)
    {
        return 0.0f;
    }

    cNet* pNet = pFielder->m_pTeam->m_pNet;
    nlVector3* fielderPos = &pFielder->m_v3Position;
    nlVector3* ballPos = &pBall->m_v3Position;
    nlVector3* netPos = &pNet->m_baseLocation;

    return InBetween(*netPos, *ballPos, *fielderPos);
}

/**
 * Offset/Address/Size: 0x23D8 | 0x80080E60 | size: 0x60
 */
float OpenTo(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    float fResult = OpenToPosition(pPlayer1->m_v3Position, pPlayer2->m_v3Position, g_pScriptOtherTeam, pPlayer1, pPlayer2, false);
    return NormalizeVal(fResult, g_vOpenToAdjust);
}

/**
 * Offset/Address/Size: 0x2298 | 0x80080D20 | size: 0x140
 */
float CloseTo(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer1->m_eClassType == GOALIE)
    {
        float dx = pPlayer2->m_v3Position.f.x - pPlayer1->m_v3Position.f.x;
        float dy = pPlayer2->m_v3Position.f.y - pPlayer1->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vCloseGoalieConfidenceDistance);
    }

    if (pPlayer2->m_eClassType == GOALIE)
    {
        float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
        float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vCloseGoalieConfidenceDistance);
    }

    nlVector2* pConfidenceDistance;
    if (pPlayer1->IsOnSameTeam(pPlayer2))
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vCloseTeammateConfidenceDistance;
    }
    else
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vCloseOpponentConfidenceDistance;
    }

    float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
    float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), *pConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x2158 | 0x80080BE0 | size: 0x140
 */
float NearTo(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer1->m_eClassType == GOALIE)
    {
        float dx = pPlayer2->m_v3Position.f.x - pPlayer1->m_v3Position.f.x;
        float dy = pPlayer2->m_v3Position.f.y - pPlayer1->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vNearGoalieConfidenceDistance);
    }

    if (pPlayer2->m_eClassType == GOALIE)
    {
        float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
        float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vNearGoalieConfidenceDistance);
    }

    nlVector2* pConfidenceDistance;
    if (pPlayer1->IsOnSameTeam(pPlayer2))
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vNearTeammateConfidenceDistance;
    }
    else
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vNearOpponentConfidenceDistance;
    }

    float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
    float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), *pConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x2018 | 0x80080AA0 | size: 0x140
 */
float FarTo(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer1->m_eClassType == GOALIE)
    {
        float dx = pPlayer2->m_v3Position.f.x - pPlayer1->m_v3Position.f.x;
        float dy = pPlayer2->m_v3Position.f.y - pPlayer1->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vFarGoalieConfidenceDistance);
    }

    if (pPlayer2->m_eClassType == GOALIE)
    {
        float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
        float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vFarGoalieConfidenceDistance);
    }

    nlVector2* pConfidenceDistance;
    if (pPlayer1->IsOnSameTeam(pPlayer2))
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vFarTeammateConfidenceDistance;
    }
    else
    {
        pConfidenceDistance = &g_pGame->m_pFuzzyTweaks->vFarOpponentConfidenceDistance;
    }

    float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
    float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), *pConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1FBC | 0x80080A44 | size: 0x5C
 */
float NearToGoaliePosition(const nlVector3& pos1, const nlVector3& pos2)
{
    float dx = pos1.f.x - pos2.f.x;
    float dy = pos1.f.y - pos2.f.y;

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vNearGoalieConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1F50 | 0x800809D8 | size: 0x6C
 */
float CloseToTheirGoalie(cPlayer* pPlayer)
{
    cTeam* pOtherTeam = pPlayer->m_pTeam->GetOtherTeam();
    cPlayer* pGoalie = pOtherTeam->GetGoalie();

    float dx = pPlayer->m_v3Position.f.x - pGoalie->m_v3Position.f.x;
    float dy = pPlayer->m_v3Position.f.y - pGoalie->m_v3Position.f.y;

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vCloseGoalieConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1EE4 | 0x8008096C | size: 0x6C
 */
float NearToTheirGoalie(cPlayer* pPlayer)
{
    cTeam* pOtherTeam = pPlayer->m_pTeam->GetOtherTeam();
    cPlayer* pGoalie = pOtherTeam->GetGoalie();

    float dx = pPlayer->m_v3Position.f.x - pGoalie->m_v3Position.f.x;
    float dy = pPlayer->m_v3Position.f.y - pGoalie->m_v3Position.f.y;

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vNearGoalieConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1E78 | 0x80080900 | size: 0x6C
 */
float FarToTheirGoalie(cPlayer* pPlayer)
{
    cTeam* pOtherTeam = pPlayer->m_pTeam->GetOtherTeam();
    cPlayer* pGoalie = pOtherTeam->GetGoalie();

    float dx = pPlayer->m_v3Position.f.x - pGoalie->m_v3Position.f.x;
    float dy = pPlayer->m_v3Position.f.y - pGoalie->m_v3Position.f.y;

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), pFuzzyTweaks->vFarGoalieConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1D08 | 0x80080790 | size: 0x170
 * TODO: 98.32% match - MWCC still rotates callee-saved mapping for
 *       score/zero (f30/f31), base/invert/offset (r30/r31/r26),
 *       and posU0/posU2 (r29/r27).
 */
float CloseToSideline(const nlVector3& v3Position, const nlVector2* vDistanceConfidence, bool bInvert)
{
    if (vDistanceConfidence == NULL)
    {
        vDistanceConfidence = &g_pGame->m_pFuzzyTweaks->vCloseToSidelineDistanceConfidence;
    }

    float fScore;
    if (bInvert)
    {
        fScore = 1.0f;
    }
    else
    {
        fScore = 0.0f;
    }

    u32 posU0 = v3Position.as_u32[0];
    u32 posU1 = v3Position.as_u32[1];
    u32 posU2 = v3Position.as_u32[2];
    const u8* pBase = (const u8*)cField::mSidelines;
    s32 offset = 0;
    s32 i = offset;
    f32 fZero = 0.0f;
    f32 posX = v3Position.f.x;
    f32 posY = v3Position.f.y;

    for (; i < 4; i++, offset += 0xC)
    {
        const sSideLinePlane* sideline = (const sSideLinePlane*)(pBase + offset);
        nlVector3 v3SidelinePos;

        v3SidelinePos.as_u32[2] = posU2;
        v3SidelinePos.as_u32[0] = posU0;
        v3SidelinePos.as_u32[1] = posU1;
        v3SidelinePos.f.z = fZero;

        if (fZero == sideline->vNormal.f.x)
        {
            v3SidelinePos.f.y = sideline->fDistance * sideline->vNormal.f.y;
        }
        else
        {
            v3SidelinePos.f.x = sideline->fDistance * sideline->vNormal.f.x;
        }

        f32 dx = v3SidelinePos.f.x - posX;
        f32 dy = v3SidelinePos.f.y - posY;
        float fDistance = nlSqrt(dx * dx + dy * dy, true);

        if (bInvert)
        {
            float fNormalized = NormalizeVal(fDistance, *vDistanceConfidence);

            if (fScore <= fNormalized)
            {
                fScore = fScore;
            }
            else
            {
                fScore = fNormalized;
            }
        }
        else
        {
            float fNormalized = NormalizeVal(fDistance, *vDistanceConfidence);

            if (fScore >= fNormalized)
            {
                fScore = fScore;
            }
            else
            {
                fScore = fNormalized;
            }
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x1BD4 | 0x8008065C | size: 0x134
 * TODO: 98.8% match - MWCC still rotates callee-saved register assignment for
 *       pos/base/offset (r27-r31) and fZero/posX/posY (f29-f31).
 */
static inline float NearToSidelineImpl(const nlVector3& v3Position, const nlVector2* pConfidence)
{
    float fScore = 0.0f;
    int i = 0;
    u32 posU0 = v3Position.as_u32[0];
    f32 fZero = fScore;
    u32 posU1 = v3Position.as_u32[1];
    u32 posU2 = v3Position.as_u32[2];
    s32 offset = i;
    f32 posX = v3Position.f.x;
    const u8* pBase = (const u8*)cField::mSidelines;
    f32 posY = v3Position.f.y;

    for (; i < 4; i++, offset += 0xC)
    {
        const sSideLinePlane* sideline = (const sSideLinePlane*)(pBase + offset);
        nlVector3 v3Pt;

        v3Pt.as_u32[2] = posU2;
        v3Pt.as_u32[0] = posU0;
        v3Pt.as_u32[1] = posU1;
        v3Pt.f.z = fZero;

        if (fZero == sideline->vNormal.f.x)
        {
            v3Pt.f.y = sideline->fDistance * sideline->vNormal.f.y;
        }
        else
        {
            v3Pt.f.x = sideline->fDistance * sideline->vNormal.f.x;
        }

        f32 dx = v3Pt.f.x - posX;
        f32 dy = v3Pt.f.y - posY;
        float fDistance = nlSqrt(dx * dx + dy * dy, true);
        float fNormalized = NormalizeVal(fDistance, *pConfidence);

        if (fScore >= fNormalized)
        {
            fScore = fScore;
        }
        else
        {
            fScore = fNormalized;
        }
    }

    return fScore;
}

float NearToSideline(const nlVector3& v3Position)
{
    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
    const nlVector2* pConfidence = &pFuzzyTweaks->vNearToSidelineDistanceConfidence;

    if (!pConfidence)
    {
        pConfidence = &pFuzzyTweaks->vCloseToSidelineDistanceConfidence;
    }

    return NearToSidelineImpl(v3Position, pConfidence);
}

/**
 * Offset/Address/Size: 0x1AB8 | 0x80080540 | size: 0x11C
 * TODO: 92.25% match - pre-loop callee-saved register allocation still differs
 *       for sideline base/offset setup and g_pGame->m_pFuzzyTweaks load register.
 */
float CloseToSideline(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    s32 offset = 0;
    s32 i = 0;
    const nlVector2* pConfidence = &g_pGame->m_pFuzzyTweaks->vCloseToSidelineDistanceConfidence;
    const u8* pBase = (const u8*)cField::mSidelines;
    float fScore = 0.0f;
    f32 fZero = fScore;

    for (; i < 4; i++, offset += 0xC)
    {
        const sSideLinePlane* sideline = (const sSideLinePlane*)(pBase + offset);
        nlVector3 v3SidelinePos;

        v3SidelinePos.as_u32[2] = pFielder->m_v3Position.as_u32[2];
        v3SidelinePos.as_u32[0] = pFielder->m_v3Position.as_u32[0];
        v3SidelinePos.as_u32[1] = pFielder->m_v3Position.as_u32[1];
        v3SidelinePos.f.z = fZero;

        if (fZero == sideline->vNormal.f.x)
        {
            v3SidelinePos.f.y = sideline->fDistance * sideline->vNormal.f.y;
        }
        else
        {
            v3SidelinePos.f.x = sideline->fDistance * sideline->vNormal.f.x;
        }

        f32 dx = v3SidelinePos.f.x - pFielder->m_v3Position.f.x;
        f32 dy = v3SidelinePos.f.y - pFielder->m_v3Position.f.y;
        float fDistance = nlSqrt(dx * dx + dy * dy, true);

        float fNormalized = NormalizeVal(fDistance, *pConfidence);

        if (fScore >= fNormalized)
        {
            fScore = fScore;
        }
        else
        {
            fScore = fNormalized;
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0x1A18 | 0x800804A0 | size: 0xA0
 */
float FacingSideline(cFielder* pFielder)
{

    if (pFielder == NULL)
    {
        return 0.0f;
    }

    AvoidController* pAvoidance = pFielder->m_pAvoidance;
    float fResult = 0.0f;

    nlVector2* pSidelineNormal;
    if (pAvoidance->m_VeryCloseToSideline)
    {
        pSidelineNormal = &pAvoidance->m_SidelineNormal;
    }
    else
    {
        pSidelineNormal = NULL;
    }

    if (pSidelineNormal != NULL)
    {
        float fSin;
        float fCos;
        float* pSin = (pSidelineNormal != NULL) ? &fSin : &fSin;
        nlSinCos(pSin, &fCos, pFielder->m_aActualFacingDirection);

        fResult = ((pSidelineNormal->f.x * fCos) + (pSidelineNormal->f.y * fSin));
        fResult = -fResult;

        if (0.0f >= fResult)
        {
            fResult = 0.0f;
        }
    }

    return fResult;
}

/**
 * Offset/Address/Size: 0x187C | 0x80080304 | size: 0x19C
 */
float StuckOnSidelines(cFielder* pFielder)
{
    if (!pFielder)
        return 0.0f;

    bool sidelineUnavoidable = pFielder->m_pAvoidance->m_SidelineUnavoidable;

    float sidelineRepulsion;
    if (!pFielder)
    {
        sidelineRepulsion = 0.0f;
    }
    else
    {
        nlVector3 repulsionVec = pFielder->m_pAvoidance->GetLastRepulsionVector(AVOID_FIELDERS);
        float magnitude = nlSqrt(repulsionVec.f.x * repulsionVec.f.x + repulsionVec.f.y * repulsionVec.f.y + repulsionVec.f.z * repulsionVec.f.z, true);
        sidelineRepulsion = NormalizeVal(magnitude, g_pGame->m_pFuzzyTweaks->vAvoidFieldersRepulsionConfidence);
    }

    float powerupRepulsion;
    if (!pFielder)
    {
        powerupRepulsion = 0.0f;
    }
    else
    {
        nlVector3 repulsionVec = pFielder->m_pAvoidance->GetLastRepulsionVector(AVOID_POWERUPS);
        float magnitude = nlSqrt(repulsionVec.f.x * repulsionVec.f.x + repulsionVec.f.y * repulsionVec.f.y + repulsionVec.f.z * repulsionVec.f.z, true);
        powerupRepulsion = NormalizeVal(magnitude, g_pGame->m_pFuzzyTweaks->vAvoidPowerupsRepulsionConfidence);
    }

    float flagVal = sidelineUnavoidable ? 1.0f : 0.0f;

    powerupRepulsion = (powerupRepulsion >= sidelineRepulsion) ? powerupRepulsion : sidelineRepulsion;
    flagVal = (flagVal >= powerupRepulsion) ? flagVal : powerupRepulsion;

    float stuckBit;
    if (!pFielder)
    {
        stuckBit = 0.0f;
    }
    else if (pFielder->m_pAvoidance->m_CurrentlyAvoiding & AVOID_SIDELINES)
    {
        stuckBit = 1.0f;
    }
    else
    {
        stuckBit = 0.0f;
    }

    stuckBit = (stuckBit <= flagVal) ? stuckBit : flagVal;
    return stuckBit;
}

/**
 * Offset/Address/Size: 0x17E8 | 0x80080270 | size: 0x94
 */
float AtIdealDistanceForTackling(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    float dx = pPlayer1->m_v3Position.f.x - pPlayer2->m_v3Position.f.x;
    float dy = pPlayer1->m_v3Position.f.y - pPlayer2->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    float idealDistance = pFuzzyTweaks->vIdealTacklingDistance.f.x;
    if (distance < idealDistance)
    {
        return NormalizeVal(distance, idealDistance - pFuzzyTweaks->vIdealTacklingDistance.f.y, idealDistance);
    }
    return NormalizeVal(distance, idealDistance + pFuzzyTweaks->vIdealTacklingDistance.f.y, idealDistance);
}

/**
 * Offset/Address/Size: 0x1774 | 0x800801FC | size: 0x74
 */
float PositionIsAtIdealDistanceForShooting(const nlVector3& pos1, const nlVector3& pos2)
{
    float dx = pos1.f.x - pos2.f.x;
    float dy = pos1.f.y - pos2.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    float idealDistance = pFuzzyTweaks->vIdealDistanceForShooting.f.x;
    if (distance < idealDistance)
    {
        return NormalizeVal(distance, idealDistance - pFuzzyTweaks->vIdealDistanceForShooting.f.y, idealDistance);
    }
    return NormalizeVal(distance, idealDistance + pFuzzyTweaks->vIdealDistanceForShooting.f.y, idealDistance);
}

/**
 * Offset/Address/Size: 0x1660 | 0x800800E8 | size: 0x114
 */
float Facing(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    nlVector3 v3Direction;
    nlPolar polar;

    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    nlVec3Set(v3Direction, pPlayer2->m_v3Position.f.x - pPlayer1->m_v3Position.f.x, pPlayer2->m_v3Position.f.y - pPlayer1->m_v3Position.f.y, pPlayer2->m_v3Position.f.z - pPlayer1->m_v3Position.f.z);

    u16 playerFacing = pPlayer1->m_aActualFacingDirection;
    nlCartesianToPolar(polar, v3Direction);

    s16 temp = playerFacing - polar.a;
    s16 angleDiff = temp < 0 ? -temp : temp;

    FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

    s16 fullConf = pFuzzyTweaks->nFacingFullConfidenceAngle;
    s16 angle_s = (u16)angleDiff;

    if (angle_s < fullConf)
    {
        return 1.0f;
    }

    s16 noConf = pFuzzyTweaks->nFacingNoConfidenceAngle;
    if (angle_s > noConf)
    {
        return 0.0f;
    }

    return 1.0f - ((float)(angle_s - fullConf) / (float)(noConf - fullConf));
}

/**
 * Offset/Address/Size: 0x15C0 | 0x80080048 | size: 0xA0
 */
float UpfieldFrom(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    nlVector3 v3FromPos = pPlayer1->m_v3Position;
    nlVector3 v3DownfieldPos = pPlayer2->m_v3Position;
    float delta = v3FromPos.f.x - v3DownfieldPos.f.x;
    float score = delta * AIsgn(pPlayer1->m_pTeam->GetOtherNet()->m_baseLocation.f.x);

    return NormalizeVal(score, 0.0f, g_pGame->m_pFuzzyTweaks->fUpfieldMaxDistance);
}

/**
 * Offset/Address/Size: 0x1520 | 0x8007FFA8 | size: 0xA0
 */
float DownfieldFrom(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    nlVector3 v3FromPos = pPlayer1->m_v3Position;
    nlVector3 v3DownfieldPos = pPlayer2->m_v3Position;
    float delta = v3DownfieldPos.f.x - v3FromPos.f.x;
    float score = delta * AIsgn(pPlayer1->m_pTeam->GetOtherNet()->m_baseLocation.f.x);

    return NormalizeVal(score, 0.0f, g_pGame->m_pFuzzyTweaks->fDownfieldMaxDistance);
}

/**
 * Offset/Address/Size: 0x14B8 | 0x8007FF40 | size: 0x68
 */
float ClosingTo(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    float closingSpeed = GetClosingSpeed2D(
        pPlayer1->m_v3Position,
        pPlayer1->m_v3Velocity,
        pPlayer2->m_v3Position,
        pPlayer2->m_v3Velocity);

    float closingSpeedMax = g_pGame->m_pFuzzyTweaks->fClosingSpeedMax;

    return NormalizeVal(closingSpeed, 0.0f, closingSpeedMax);
}

/**
 * Offset/Address/Size: 0x1450 | 0x8007FED8 | size: 0x68
 */
float ClosingTo(cPlayer* pPlayer, cBall* pBall)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    if (pBall == NULL)
    {
        return 0.0f;
    }

    float closingSpeed = GetClosingSpeed2D(
        pPlayer->m_v3Position,
        pPlayer->m_v3Velocity,
        pBall->m_v3Position,
        pBall->m_v3Velocity);

    float closingSpeedMax = g_pGame->m_pFuzzyTweaks->fClosingSpeedMax;

    return NormalizeVal(closingSpeed, 0.0f, closingSpeedMax);
}

/**
 * Offset/Address/Size: 0x13E4 | 0x8007FE6C | size: 0x6C
 */
float SeparatingFrom(cPlayer* pPlayer1, cPlayer* pPlayer2)
{
    if (pPlayer1 == NULL)
    {
        return 0.0f;
    }

    if (pPlayer2 == NULL)
    {
        return 0.0f;
    }

    float closingSpeed = GetClosingSpeed2D(
        pPlayer1->m_v3Position,
        pPlayer1->m_v3Velocity,
        pPlayer2->m_v3Position,
        pPlayer2->m_v3Velocity);

    float separatingSpeedMax = g_pGame->m_pFuzzyTweaks->fSeparatingSpeedMax;

    return NormalizeVal(closingSpeed, 0.0f, -separatingSpeedMax);
}

/**
 * Offset/Address/Size: 0x1350 | 0x8007FDD8 | size: 0x94
 */
float OutOfNet(Goalie* pGoalie)
{
    if (pGoalie == NULL)
    {
        return 0.0f;
    }

    nlVector3 netLocation = pGoalie->GetAIDefNetLocation(NULL);

    float dx = netLocation.f.x - pGoalie->m_v3Position.f.x;
    float dy = netLocation.f.y - pGoalie->m_v3Position.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), g_pGame->m_pFuzzyTweaks->vOutOfNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1324 | 0x8007FDAC | size: 0x2C
 */
float Stunned(Goalie* pGoalie)
{
    if (pGoalie == NULL)
    {
        return 0.0f;
    }

    if (pGoalie->mGoalieActionState == GOALIEACTION_STS_RECOVER)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x12A8 | 0x8007FD30 | size: 0x7C
 */
float FarToTheirNetB(cBall* pBall)
{
    if (pBall == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = g_pScriptCurrentFielder->GetAIOffNetLocation(&pBall->m_v3Position);

    float dx = pBall->m_v3Position.f.x - netLocation.f.x;
    float dy = pBall->m_v3Position.f.y - netLocation.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), g_pGame->m_pFuzzyTweaks->vFarBallNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x1220 | 0x8007FCA8 | size: 0x88
 */
float NearToPlayersNet(cBall* pBall, cPlayer* pPlayer)
{
    if (pBall == NULL)
    {
        return 0.0f;
    }

    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    const nlVector3& netLocation = pPlayer->GetAIDefNetLocation(&pBall->m_v3Position);

    float dx = pBall->m_v3Position.f.x - netLocation.f.x;
    float dy = pBall->m_v3Position.f.y - netLocation.f.y;

    return NormalizeVal(nlSqrt(dx * dx + dy * dy, true), g_pGame->m_pFuzzyTweaks->vNearBallNetConfidenceDistance);
}

/**
 * Offset/Address/Size: 0x11A4 | 0x8007FC2C | size: 0x7C
 */
float InControlOfBall(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (pFielder != g_pScriptBall->m_pOwner)
    {
        return 0.0f;
    }

    float dx = g_pScriptBall->m_v3Position.f.x - pFielder->m_v3Position.f.x;
    float dy = g_pScriptBall->m_v3Position.f.y - pFielder->m_v3Position.f.y;
    float distance = nlSqrt(dx * dx + dy * dy, true);

    return NormalizeVal(distance, g_pGame->m_pFuzzyTweaks->vControlConfidenceDistance);
}

static inline bool IsShotMeterActive(eShotMeterState state)
{
    return (state == SHOT_METER_ACTIVE || state == SHOT_METER_STS_ACTIVE || state == SHOT_METER_STS_TRANSISTION);
}

/**
 * Offset/Address/Size: 0x1118 | 0x8007FBA0 | size: 0x8C
 */
float WindingUpForShot(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    if (IsShotMeterActive(pFielder->m_pShotMeter->m_eShotMeterState) || pFielder->IsPreparingForOneTimer() || pFielder->m_eActionState == ACTION_SHOOT_TO_SCORE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x10FC | 0x8007FB84 | size: 0x1C
 */
float OnMushrooms(cFielder* pFielder)
{
    if (pFielder->m_ePowerup == POWER_UP_MUSHROOM)
    {
        return 1.0f;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0xFF4 | 0x8007FA7C | size: 0x108
 */
float ChasingBall(cPlayer* pPlayer)
{
    float fScore;

    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    cFielder* pFielder = (cFielder*)pPlayer;
    fScore = 0.0f;

    if (pPlayer->m_eClassType != GOALIE && pPlayer->m_eClassType == FIELDER)
    {
        eFielderDesireState desireState = pFielder->m_eFielderDesireState;

        if (desireState == FIELDERDESIRE_INTERCEPT_BALL)
        {
            return 1.0f;
        }
        else if (desireState == FIELDERDESIRE_USER_CONTROLLED)
        {
            float fScore_;
            if (pPlayer == NULL)
            {
                fScore_ = 0.0f;
            }
            else
            {
                int index = -1;
                cTeam* pTeam = ((cFielder*)pPlayer)->m_pTeam;
                if (pTeam->m_pBallInterceptOrderedFielders[0] == pPlayer)
                {
                    index = 0;
                }
                else if (pTeam->m_pBallInterceptOrderedFielders[1] == pPlayer)
                {
                    index = 1;
                }
                else if (pTeam->m_pBallInterceptOrderedFielders[2] == pPlayer)
                {
                    index = 2;
                }
                else if (pTeam->m_pBallInterceptOrderedFielders[3] == pPlayer)
                {
                    index = 3;
                }

                fScore_ = (3 - index) / 3.0f;
            }
            fScore = fScore_;
        }
        else if (desireState == FIELDERDESIRE_MARK)
        {
            if (pFielder->m_pMark != NULL && pFielder->m_pMark->m_pBall != NULL)
            {
                fScore = 0.5f;
            }
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0xFBC | 0x8007FA44 | size: 0x38
 */
float ReceivingPass(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    eFielderDesireState desireState = pFielder->m_eFielderDesireState;
    float fScore = 0.0f;
    if (((s32)desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE || (desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN) || desireState == FIELDERDESIRE_ONETIMER))
    {
        fScore = 1.0f;
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0xF5C | 0x8007F9E4 | size: 0x60
 */
float ReceivingVolleyPass(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float fScore = 0.0f;
    if (pPlayer->m_eClassType == FIELDER)
    {
        if (((cFielder*)pPlayer)->IsReceivingVolleyPass())
        {
            fScore = 1.0f;
        }
    }

    return fScore;
}

/**
 * Offset/Address/Size: 0xE80 | 0x8007F908 | size: 0xDC
 */
float ReceivingPassDelayed(cFielder* pFielder)
{
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    eFielderDesireState desireState = pFielder->m_eFielderDesireState;
    float fScore = 0.0f;
    if (((s32)desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE || (desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN) || desireState == FIELDERDESIRE_ONETIMER))
    {
        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;
        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);

        float fTemp = pFuzzyTweaks->fPassDeadZone * (1.0f - pSkillTweaks->Off_Reaction);
        float fMaxValue = (0.1f >= fTemp) ? 0.1f : fTemp;

        float fProgress = IsPassInPlay(g_pBall);

        fScore = NormalizeVal(fProgress, 0.0f, fMaxValue);
    }
    return fScore;
}

/**
 * Offset/Address/Size: 0xDF8 | 0x8007F880 | size: 0x88
 */
float PassReceiveCloseToDone(cFielder* pFielder)
{
    float fScore;
    if (pFielder == NULL)
    {
        return 0.0f;
    }

    eFielderDesireState desireState = pFielder->m_eFielderDesireState;
    fScore = 0.0f;

    if (((s32)desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_IDLE || (desireState == FIELDERDESIRE_RECEIVE_PASS_FROM_RUN) || desireState == FIELDERDESIRE_ONETIMER))
    {
        float fProgress = IsPassInPlay(g_pBall);
        fScore = NormalizeVal(fProgress, g_vPassCloseToDoneConfidence);
    }
    return fScore;
}

/**
 * Offset/Address/Size: 0xD18 | 0x8007F7A0 | size: 0xE0
 */
float ReceivingVolleyPassDelayed(cPlayer* pPlayer)
{
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    float fScore = 0.0f;
    if (pPlayer->m_eClassType == FIELDER && ((cFielder*)pPlayer)->IsReceivingVolleyPass())
    {
        FuzzyTweaks* pFuzzyTweaks = g_pGame->m_pFuzzyTweaks;

        float _fScore;

        SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        float fTemp = pFuzzyTweaks->fPassDeadZone * (1.0f - pSkillTweaks->Off_Reaction);

        _fScore = (0.1f >= fTemp) ? 0.1f : fTemp;

        float fProgress = IsPassInPlay(g_pBall);

        fScore = NormalizeVal(fProgress, 0.0f, _fScore);
    }
    return fScore;
}

/**
 * Offset/Address/Size: 0xCD4 | 0x8007F75C | size: 0x44
 */
float High(cBall* pBall)
{
    if (pBall == NULL)
    {
        return 0.0f;
    }

    return NormalizeVal(g_pScriptBall->m_v3Position.f.z, g_pGame->m_pFuzzyTweaks->vHighBallConfidenceDistance);
}

/**
 * Offset/Address/Size: 0xCA8 | 0x8007F730 | size: 0x2C
 */
float Ownerless(cBall* pBall)
{
    if (pBall == NULL)
    {
        return 0.0f;
    }

    if (pBall->m_pOwner == NULL)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0xC7C | 0x8007F704 | size: 0x2C
 */
float AggressiveT(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->meCurrentTeamStyle == TEAM_STYLE_AGGRESSIVE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0xC50 | 0x8007F6D8 | size: 0x2C
 */
float Moderate(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->meCurrentTeamStyle == TEAM_STYLE_MODERATE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0xC24 | 0x8007F6AC | size: 0x2C
 */
float Passive(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->meCurrentTeamStyle == TEAM_STYLE_PASSIVE)
    {
        return 1.0f;
    }

    return 0.0f;
}

// static

/**
 * Offset/Address/Size: 0xBD8 | 0x8007F660 | size: 0x4C
 */
float UserControlledT(cTeam* pTeam)
{
    if (!pTeam)
    {
        return 0.0f;
    }

    s32 numControllers = pTeam->GetNumAssignedControllers();

    if ((((u32)(-numControllers & ~numControllers)) >> 0x1F) != 0) // TODO: matches but is not clean...
    // if ((u32)numControllers != 0)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x744 | 0x8007F1CC | size: 0x494
 */
void GonnaGetBall(cTeam*)
{
}

/**
 * Offset/Address/Size: 0x6CC | 0x8007F154 | size: 0x78
 */
float Losing(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pTeam->GetOtherTeam();
    int scoreDiff = pTeam->m_nScore - pOtherTeam->m_nScore;

    return NormalizeVal((float)scoreDiff, 0.0f, -g_pGame->m_pFuzzyTweaks->fLosingScoreDelta);
}

/**
 * Offset/Address/Size: 0x64C | 0x8007F0D4 | size: 0x80
 */
float Tied(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pTeam->GetOtherTeam();
    int scoreDiff = pTeam->m_nScore - pOtherTeam->m_nScore;
    int absScoreDiff = (scoreDiff < 0) ? -scoreDiff : scoreDiff;

    return NormalizeVal((float)absScoreDiff, g_pGame->m_pFuzzyTweaks->fTiedScoreDelta, 0.0f);
}

/**
 * Offset/Address/Size: 0x5D8 | 0x8007F060 | size: 0x74
 */
float Winning(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    cTeam* pOtherTeam = pTeam->GetOtherTeam();
    int scoreDiff = pTeam->m_nScore - pOtherTeam->m_nScore;

    return NormalizeVal((float)scoreDiff, 0.0f, g_pGame->m_pFuzzyTweaks->fWinningScoreDelta);
}

/**
 * Offset/Address/Size: 0x5AC | 0x8007F034 | size: 0x2C
 */
float Offensive(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->mpCurrentSituation == SITUATION_OFFENSE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x580 | 0x8007F008 | size: 0x2C
 */
float Defensive(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->mpCurrentSituation == SITUATION_DEFENSE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x554 | 0x8007EFDC | size: 0x2C
 */
float Loose(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    if (pTeam->mpCurrentSituation == SITUATION_LOOSE)
    {
        return 1.0f;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x518 | 0x8007EFA0 | size: 0x3C
 */
float Stalling(cTeam* pTeam)
{
    if (pTeam == NULL)
    {
        return 0.0f;
    }

    float seconds = pTeam->mtDefensiveZoneTimer.GetSeconds();
    return NormalizeVal(seconds, g_vStallingConfidenceTime);
}

/**
 * Offset/Address/Size: 0x46C | 0x8007EEF4 | size: 0xAC
 */
float IsPassInPlayDelayed()
{
    float fScore;
    cBall* pBall;
    if (g_pBall->m_pPassTarget == NULL)
    {
        return 0.0f;
    }

    cTeam* pTeam = g_pCurrentlyUpdatingTeam;

    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);

    float fReaction = pSkillTweaks->Off_Reaction;
    float fPassDeadZone = g_pGame->m_pFuzzyTweaks->fPassDeadZone;
    float fMaxValue = fPassDeadZone * (1.0f - fReaction);

    fScore = IsPassInPlay(g_pBall);

    return NormalizeVal(fScore, 0.0f, fMaxValue);
}

/**
 * Offset/Address/Size: 0x3E0 | 0x8007EE68 | size: 0x8C
 */
float IsPerfectPassInPlay()
{
    float fInitialScore;
    if (g_pBall->m_pPassTarget != NULL)
    {
        fInitialScore = 1.0f;
    }
    else
    {
        fInitialScore = 0.0f;
    }

    float fScore = fInitialScore;
    if (fInitialScore > 0.0f)
    {
        if (g_pScriptBall->m_pPrevOwner != NULL && g_pScriptBall->m_pPrevOwner->m_eClassType == FIELDER)
        {
            if (g_pScriptBall->mbHyperSTS == false)
            {
                fScore = 0.0f;
            }
            fScore *= 2.0f;
        }
    }

    float fClampedScore = max_float(fScore, 0.0);
    fClampedScore = min_float(fClampedScore, 1.0);

    return fClampedScore;
}

/**
 * Offset/Address/Size: 0x35C | 0x8007EDE4 | size: 0x84
 */
float PerfectPassCandidateFrom(cFielder* pReceiver, cFielder* pBallOwner)
{
    bool canDoPerfectPass;

    if (pReceiver == NULL)
    {
        return 0.0f;
    }

    if (pBallOwner == NULL)
    {
        return 0.0f;
    }

    canDoPerfectPass = false;
    if ((pBallOwner->m_pBall != 0) && (pBallOwner->DoCalcCanDoPerfectPass(pReceiver, pReceiver->m_v3Position) != 0))
    {
        canDoPerfectPass = true;
    }

    if (canDoPerfectPass)
    {
        return 1.0f;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x310 | 0x8007ED98 | size: 0x4C
 */
float TimeCloseToOver(cGame* pGame)
{
    if (!pGame)
    {
        return 0.0f;
    }

    FuzzyTweaks* pTweaks = g_pGame->m_pFuzzyTweaks;
    float gameTime = pGame->GetNormalizedGameTime();
    return NormalizeVal(gameTime, pTweaks->fGameTimeCloseToOver, 1.0f);
}

/**
 * Offset/Address/Size: 0x2C4 | 0x8007ED4C | size: 0x4C
 */
float TimeNearlyOver(cGame* pGame)
{
    if (!pGame)
    {
        return 0.0f;
    }

    FuzzyTweaks* pTweaks = g_pGame->m_pFuzzyTweaks;
    float gameTime = pGame->GetNormalizedGameTime();
    return NormalizeVal(gameTime, pTweaks->fGameTimeNearlyOver, 1.0f);
}

/**
 * Offset/Address/Size: 0x278 | 0x8007ED00 | size: 0x4C
 */
float TimeFarFromOver(cGame* pGame)
{
    if (!pGame)
    {
        return 0.0f;
    }

    FuzzyTweaks* pTweaks = g_pGame->m_pFuzzyTweaks;
    float gameTime = pGame->GetNormalizedGameTime();
    return NormalizeVal(gameTime, 1.0f, pTweaks->fGameTimeFarFromOver);
}

inline float max0(const float value)
{
    if (value >= 0.f)
    {
        return value;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x1E0 | 0x8007EC68 | size: 0x98
 */
float Difficult(cTeam* pTeam)
{
    eDifficultyID diff;
    f32 fScore;

    if (pTeam == NULL)
    {
        return 0.0f;
    }

    diff = nlSingleton<GameInfoManager>::Instance()->mCurrentDifficulty[(s16)pTeam->m_nSide];
    if (diff == DIFF_HUMAN)
    {
        return 0.5f;
    }

    float fMul = 0.25f;
    fScore = max0((float)diff * fMul);
    fScore = (fScore <= 1.0f) ? fScore : 1.0f;
    return fScore;
}

/**
 * Offset/Address/Size: 0x1A0 | 0x8007EC28 | size: 0x40
 */
float InOffensiveZone(const nlVector3& position, eTeamSide teamSide)
{
    nlVector3 aiLoc;
    FieldLocToAILoc(aiLoc, position, teamSide);

    return NormalizeVal(aiLoc.f.x, g_pGame->m_pFuzzyTweaks->vOffensiveConfidenceDistances);
}

/**
 * Offset/Address/Size: 0x13C | 0x8007EBC4 | size: 0x64
 */
float InDefensiveZone(cPlayer* pPlayer)
{
    nlVector3 aiLoc;
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    eTeamSide teamSide = (eTeamSide)(pPlayer->m_pTeam->m_nSide);
    nlVector3& playerPos = PositionOf(pPlayer);
    FieldLocToAILoc(aiLoc, playerPos, teamSide);

    return NormalizeVal(aiLoc.f.x, g_pGame->m_pFuzzyTweaks->vDefensiveConfidenceDistances);
}

/**
 * Offset/Address/Size: 0xD8 | 0x8007EB60 | size: 0x64
 */
float InOffensiveZone(cPlayer* pPlayer)
{
    nlVector3 aiLoc;
    if (pPlayer == NULL)
    {
        return 0.0f;
    }

    eTeamSide teamSide = (eTeamSide)(pPlayer->m_pTeam->m_nSide);
    nlVector3& playerPos = PositionOf(pPlayer);
    FieldLocToAILoc(aiLoc, playerPos, teamSide);

    return NormalizeVal(aiLoc.f.x, g_pGame->m_pFuzzyTweaks->vOffensiveConfidenceDistances);
}

/**
 * Offset/Address/Size: 0x6C | 0x8007EAF4 | size: 0x6C
 */
template <typename T>
nlVector3& PositionOf(T pObject);

float InDefensiveZoneOfPlayer(cBall* pBall, cPlayer* pPlayer)
{
    nlVector3 aiLoc;
    if ((pBall == NULL) && (pPlayer != NULL))
    {
        return 1.0f;
    }

    eTeamSide teamSide = (eTeamSide)(pPlayer->m_pTeam->m_nSide);
    nlVector3& ballPos = PositionOf<cBall*>(pBall);
    FieldLocToAILoc(aiLoc, ballPos, teamSide);

    return NormalizeVal(aiLoc.f.x, g_pGame->m_pFuzzyTweaks->vDefensiveConfidenceDistances);
}

/**
 * Offset/Address/Size: 0x0 | 0x8007EA88 | size: 0x6C
 */
float InOffensiveZoneOfPlayer(cBall* pBall, cPlayer* pPlayer)
{
    nlVector3 aiLoc;
    if ((pBall == NULL) && (pPlayer != NULL))
    {
        return 1.0f;
    }

    eTeamSide teamSide = (eTeamSide)(pPlayer->m_pTeam->m_nSide);
    nlVector3& ballPos = PositionOf(pBall);
    FieldLocToAILoc(aiLoc, ballPos, teamSide);

    return NormalizeVal(aiLoc.f.x, g_pGame->m_pFuzzyTweaks->vOffensiveConfidenceDistances);
}
