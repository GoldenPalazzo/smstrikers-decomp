#include "Game/AI/Scripts/Plays/DefaultOffensive.h"

#include "Game/AI/Fuzzy.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AI/SpaceSearch.h"

class cTeam;

extern cFielder* g_pScriptCurrentFielder;
extern cTeam* g_pScriptCurrentTeam;
extern cTeam* g_pCurrentlyUpdatingTeam;
extern FuzzyVariant fvNotSet;

/**
 * Offset/Address/Size: 0x6B9C | 0x80093628 | size: 0x43C
 */
FuzzyVariant Fuzzy::AbortOffensivePlay(cDecisionEntity*)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = Offensive(g_pScriptCurrentTeam);
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(false);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(true);
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
}

/**
 * Offset/Address/Size: 0x51D8 | 0x80091C64 | size: 0x19C4
 */
static inline float OffensiveIdentity(float value)
{
    return value;
}

static inline float OffensiveGood(float& output, const FuzzyVariant& value)
{
    return output = value.mData.f;
}

FuzzyVariant Fuzzy::DefaultOffensivePlay(cDecisionEntity* pDecision)
{
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = BallOwner(g_pScriptCurrentFielder);
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;
    float fTopLevelBranchRatio = fBranchRatio;
    float fTopLevelFalseConfidence = fFalseConfidence;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        const FuzzyVariant& doShooting = DoShooting(fConfidence, pDecision);
        float fDoShooting = (doShooting.mData.f >= 0.0f) ? doShooting.mData.f : 0.0f;

        float fDoPassing = OffensiveIdentity(DoPassing(fConfidence, pDecision).mData.f);
        fDoPassing = (fDoPassing >= fDoShooting) ? fDoPassing : fDoShooting;

        fBestConfidence = fDoPassing;

        float fGoodBallCarrier = OffensiveIdentity(GoodBallCarrier(g_pScriptCurrentFielder).mData.f);

        float fNotRepeatingDeke = OffensiveIdentity(1.0f - RepeatingLastDesire(g_pScriptCurrentFielder, edDeke));
        float fNotCloseSideline = OffensiveIdentity(1.0f - CloseToSideline(g_pScriptCurrentFielder));
        float fNotInvincible = OffensiveIdentity(1.0f - Invincible(g_pScriptCurrentFielder));

        {
            float fTrueConfidence = InControlOfBall(g_pScriptCurrentFielder);
            fNotCloseSideline = (fNotCloseSideline <= fNotRepeatingDeke) ? fNotCloseSideline : fNotRepeatingDeke;
            fNotInvincible = (fNotInvincible <= fNotCloseSideline) ? fNotInvincible : fNotCloseSideline;
            fTrueConfidence = (fTrueConfidence <= fNotInvincible) ? fTrueConfidence : fNotInvincible;

            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                fTrueConfidence = FGREATER(Attacked(g_pScriptCurrentFielder), 0.4f);
                fFalseConfidence = 1.0f - fTrueConfidence;

                fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }

                    pDecision->QueueActionSetDesire(2, fConfidence, -1.0f, fvNotSet, fvNotSet);

                    SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_DekeChance, Deker(g_pScriptCurrentFielder));
                }

                if (fFalseConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                    if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    fTrueConfidence = FLESS(Open(g_pScriptCurrentFielder), 0.5f);
                    float fOpenFalseConfidence = 1.0f - fTrueConfidence;

                    fMin = (fTrueConfidence <= fOpenFalseConfidence) ? fTrueConfidence : fOpenFalseConfidence;
                    fMax = (fTrueConfidence >= fOpenFalseConfidence) ? fTrueConfidence : fOpenFalseConfidence;
                    fBranchRatio = fMin / fMax;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM4(&fConfidence);
                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                            fConfidence = fConfidence * fBranchRatio;

                        if (fBestConfidence >= fConfidence)
                        {
                            fBestConfidence = OffensiveIdentity(fBestConfidence);
                        }
                        else
                        {
                            fBestConfidence = fConfidence;
                        }

                        pDecision->QueueActionSetDesire(2, fConfidence, -1.0f, fvNotSet, fvNotSet);

                        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(0.5f * pTweaks->Off_DekeChance, Deker(g_pScriptCurrentFielder));
                    }
                }
            }
        }

        {
            float fTrueConfidence = OffensiveIdentity(BallOwner(g_pScriptCurrentFielder));
            float fNotUserControlled = UserControlledT(g_pScriptCurrentTeam);
            fNotUserControlled = 1.0f - fNotUserControlled;
            if (fNotUserControlled <= fTrueConfidence)
            {
                fNotUserControlled = OffensiveIdentity(fNotUserControlled);
            }
            else
            {
                fNotUserControlled = fTrueConfidence;
            }
            fTrueConfidence = fNotUserControlled;

            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                float fUsePowerup = UsePowerupOffensive(fConfidence, pDecision).mData.f;
                if (fUsePowerup >= fBestConfidence)
                    fBestConfidence = fUsePowerup;
            }

            {
                float fTrueConfidence = FGREATER(fGoodBallCarrier, (fDoShooting >= fDoPassing) ? fDoShooting : fDoPassing);
                fGoodBallCarrier = (fGoodBallCarrier <= fTrueConfidence) ? fGoodBallCarrier : fTrueConfidence;

                float fDifficult = FLESS(Difficult(g_pScriptCurrentTeam), 0.1f);
                float fFallback = (0.0f == fBestConfidence) ? 1.0f : 0.0f;

                fTrueConfidence = (fDifficult >= fGoodBallCarrier) ? fDifficult : fGoodBallCarrier;
                fTrueConfidence = (fFallback >= fTrueConfidence) ? fFallback : fTrueConfidence;

                float fFalseConfidence = 1.0f - fTrueConfidence;
                float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM2(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }
                    pDecision->QueueActionSetDesire(9, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }
            }
        }
    }

    fBranchRatio = fTopLevelBranchRatio;
    fFalseConfidence = fTopLevelFalseConfidence;

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        float fCutAndBreak = CutAndBreak(g_pScriptCurrentFielder).mData.f;

        fTrueConfidence = Striker(g_pScriptCurrentFielder);
        fTrueConfidence = (fTrueConfidence <= fCutAndBreak) ? fTrueConfidence : fCutAndBreak;

        float fCutFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fCutFalseConfidence) ? fTrueConfidence : fCutFalseConfidence;
        fMax = (fTrueConfidence >= fCutFalseConfidence) ? fTrueConfidence : fCutFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            if (fBestConfidence >= fConfidence)
            {
                fBestConfidence = OffensiveIdentity(fBestConfidence);
            }
            else
            {
                fBestConfidence = fConfidence;
            }

            pDecision->QueueActionSetDesire(1, fConfidence, -1.0f, fvNotSet, fvNotSet);

            SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            pDecision->m_pLastQueuedAction->m_fSelectionChance = pTweaks->Off_CutAndBreakChance;
        }

        FuzzyVariant strategicBallOwner = GetStrategicBallCarrier(g_pScriptCurrentTeam);

        {
            float fStriker;
            float fBallOwnerGoalie = OffensiveIdentity(BallOwner(g_pScriptCurrentTeam->GetGoalie()));
            fStriker = OffensiveIdentity(Striker(g_pScriptCurrentFielder));
            float fWinger = Winger(g_pScriptCurrentFielder);

            fStriker = (fStriker >= fBallOwnerGoalie) ? fStriker : fBallOwnerGoalie;
            fWinger = (fWinger >= fStriker) ? fWinger : fStriker;

            float fFalseConfidence = 1.0f - fWinger;
            float fTrueConfidence = fWinger;

            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                float fWindup = OffensiveIdentity(WindingUpForShot((cFielder*)strategicBallOwner.mData.pPlayer));
                float fNotNearToTheirNet = OffensiveIdentity(1.0f - NearToTheirNet(g_pScriptCurrentFielder));
                float fNotStrategicConfidence = OffensiveIdentity(1.0f - strategicBallOwner.Confidence);

                fNotNearToTheirNet = (fNotNearToTheirNet >= fWindup) ? fNotNearToTheirNet : fWindup;
                fNotStrategicConfidence = (fNotStrategicConfidence >= fNotNearToTheirNet) ? fNotStrategicConfidence : fNotNearToTheirNet;

                float fFalseConfidence = 1.0f - fNotStrategicConfidence;
                float fTrueConfidence = fNotStrategicConfidence;

                float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }

                    pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fFalseConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                    if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }
                    pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                float fNotFarToTheirNet = 1.0f - FarToTheirNet(g_pScriptCurrentFielder);
                fTrueConfidence = (strategicBallOwner.Confidence <= fNotFarToTheirNet) ? strategicBallOwner.Confidence : fNotFarToTheirNet;
                fFalseConfidence = 1.0f - fTrueConfidence;

                fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }
                    pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fFalseConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                    if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    if (fBestConfidence >= fConfidence)
                    {
                        fBestConfidence = OffensiveIdentity(fBestConfidence);
                    }
                    else
                    {
                        fBestConfidence = fConfidence;
                    }
                    pDecision->QueueActionSetDesire(3, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }
            }
        }
    }

    return FuzzyVariant(fBestConfidence);
}

/**
 * Offset/Address/Size: 0x4A94 | 0x80091520 | size: 0x744
 */
FuzzyVariant Fuzzy::DoPassing(float fConfidence, cDecisionEntity* pDecision)
{
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pCurrentlyUpdatingTeam;

    float fPassTargetFalseConfidence;
    float fBestConfidence = 0.0f;

    float fFalseConfidence = 1.0f - Invincible(g_pScriptCurrentFielder);
    float fTrueConfidence = 1.0f - fFalseConfidence;
    float fMin = (fFalseConfidence <= fTrueConfidence) ? fFalseConfidence : fTrueConfidence;
    float fMax = (fFalseConfidence >= fTrueConfidence) ? fFalseConfidence : fTrueConfidence;
    float fBranchRatio = fMin / fMax;

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        float fAvoidingPowerups = AvoidingPowerups(g_pScriptCurrentFielder);
        float fOne = 1.0f;
        FuzzyVariant theBestPassTarget = GetBestPassTarget(g_pScriptCurrentFielder);
        float fAdjustedConfidence = theBestPassTarget.Confidence * (fOne - fAvoidingPowerups) + fOne * fAvoidingPowerups;

        fTrueConfidence = FGREATER(theBestPassTarget.Confidence, 0.15f);
        fTrueConfidence = (fTrueConfidence <= fAdjustedConfidence) ? fTrueConfidence : fAdjustedConfidence;

        {
            fPassTargetFalseConfidence = 1.0f - fTrueConfidence;
            float fFalseConfidence = fPassTargetFalseConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                {
                    float fOpenConfidence = OpenTo(g_pScriptCurrentFielder, theBestPassTarget.mData.pPlayer);
                    float fOpenFalseConfidence = 1.0f - fOpenConfidence;
                    float fOpenMin = (fOpenConfidence <= fOpenFalseConfidence) ? fOpenConfidence : fOpenFalseConfidence;
                    float fOpenMax = (fOpenConfidence >= fOpenFalseConfidence) ? fOpenConfidence : fOpenFalseConfidence;
                    float fOpenBranchRatio = fOpenMin / fOpenMax;

                    if (fOpenConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM(&fConfidence);
                        fConfidence = (fConfidence <= fOpenConfidence) ? fConfidence : fOpenConfidence;

                        if (fConfidence < fOpenConfidence && fOpenConfidence < 0.5f)
                            fConfidence = fConfidence * fOpenBranchRatio;

                        float fCurrentConfidence = fConfidence;
                        if (0.0f >= fCurrentConfidence)
                            fBestConfidence = 0.0f;
                        else
                            fBestConfidence = fCurrentConfidence;

                        pDecision->QueueActionSetDesire(19, fConfidence, 0.0f, theBestPassTarget, FuzzyVariant(false));

                        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_GroundPassChance, Passer(g_pScriptCurrentFielder));
                    }

                    if (fOpenFalseConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM(&fConfidence);
                        fConfidence = (fConfidence <= fOpenFalseConfidence) ? fConfidence : fOpenFalseConfidence;

                        if (fConfidence < fOpenFalseConfidence && fOpenFalseConfidence < 0.5f)
                            fConfidence = fConfidence * fOpenBranchRatio;

                        if (fBestConfidence >= fConfidence)
                            fBestConfidence = fBestConfidence;
                        else
                            fBestConfidence = fConfidence;

                        pDecision->QueueActionSetDesire(19, fConfidence, 0.0f, theBestPassTarget, FuzzyVariant(true));

                        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_VolleyPassChance, Passer(g_pScriptCurrentFielder));
                    }
                }
            }
        }
    }

    return FuzzyVariant(fBestConfidence);
}

static inline float OffensiveDanger(const FuzzyVariant& value)
{
    return value.mData.f;
}

/**
 * Offset/Address/Size: 0x4490 | 0x80090F1C | size: 0x604
 */
FuzzyVariant Fuzzy::GoodBallCarrier(cFielder* TheFielder)
{
    extern cFielder* g_pScriptCurrentFielder;

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fWindupScore;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    fWindupScore = InGoodWindupPosition(g_pScriptCurrentFielder).mData.f;

    float fOnMushrooms = OnMushrooms(g_pScriptCurrentFielder);
    float fInvincible = Invincible(g_pScriptCurrentFielder);

    float fTrueConfidence = (fInvincible >= fOnMushrooms) ? fInvincible : fOnMushrooms;
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            float fLessWindup = FLESS(fWindupScore, 0.8f);
            bestValue = FuzzyVariant(fLessWindup);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            fOnMushrooms = FLESS(fWindupScore, 0.8f);
            fInvincible = 1.0f - CloseToMyNet(g_pScriptCurrentFielder);
            bestValue = FuzzyVariant((fFalseConfidence = 1.0f - OffensiveDanger(InDangerDelayed(g_pScriptCurrentFielder)),
                fInvincible = (fInvincible <= fOnMushrooms) ? fInvincible : fOnMushrooms,
                fFalseConfidence = (fFalseConfidence <= fInvincible) ? fFalseConfidence : fInvincible));
        }
    }

    bestValue.Confidence = fBestConfidence;

    return bestValue;
}

/**
 * Offset/Address/Size: 0x3128 | 0x8008FBB4 | size: 0x1368
 * TODO: 99.96% match - late scoring temporaries still differ in f-register
 * allocation.
 */
struct StdMapNodeBase
{
    void* left;
    void* right;
    void* parent;
};

struct StdMapTree
{
    unsigned long x0;
    StdMapNodeBase x4;
};

struct StdMapNode
{
    StdMapNodeBase base;
    unsigned long key;
    FuzzyVariant value;
};

#include "Game/AI/Scripts/ScriptCaching.h"

static inline unsigned long OffensiveHash(unsigned long value)
{
    return value;
}


static inline float OffensiveMax(float a, float b)
{
    return (b <= a) ? a : b;
}

static inline float OffensiveHalfScore(const FuzzyVariant& value, float fLosing)
{
    float fGoodToShoot = OffensiveIdentity(OffensiveDanger(value));
    float fHalf = 0.5f;
    float fWeightedGoodToShoot = fGoodToShoot * fHalf;
    return fLosing * fHalf + fWeightedGoodToShoot;
}

extern float InFrontOfTheirNet(cFielder*);
extern cFielder* g_pScriptCurrentFielder;
extern float FarToTheirNet(cPlayer*);
extern float FGREATER(float, float);

static inline float OffensiveInitialMax(float a)
{
    float b = FGREATER(FarToTheirNet(g_pScriptCurrentFielder), 0.85f);
    return (b >= a) ? b : a;
}

static inline void OffensiveBranchValues(
    float fTrueConfidence, float& fBranchRatio, float& fFalseConfidence)
{
    fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMinVal / fMaxVal;
}


static inline float OffensiveThreeScore(
    const FuzzyVariant& value,
    float fNotFarToTheirNet,
    float fOpen,
    float fLosing)
{
    float fGoodToShoot = OffensiveDanger(value);
    fGoodToShoot = (fNotFarToTheirNet >= fGoodToShoot) ? fNotFarToTheirNet : fGoodToShoot;
    fGoodToShoot = (fOpen >= fGoodToShoot) ? fOpen : fGoodToShoot;

    float fInFront = InFrontOfTheirNet(g_pScriptCurrentFielder);
    float fWeighted = fInFront * 0.2f;
    fWeighted = fLosing * 0.25f + fWeighted;
    return fGoodToShoot * 0.55f + fWeighted;
}

static inline float OffensiveTwoScore(
    const FuzzyVariant& value,
    float fNotFarToTheirNet,
    float fOpen)
{
    float fGoodToShoot = OffensiveDanger(value);
    fGoodToShoot = (fNotFarToTheirNet >= fGoodToShoot) ? fNotFarToTheirNet : fGoodToShoot;
    float fSelected = OffensiveMax(fGoodToShoot, fOpen);

    float fInFront = InFrontOfTheirNet(g_pScriptCurrentFielder);
    float fGoodWeight = 0.7f;
    float fFrontWeight = 0.3f;
    float fWeighted = fSelected * fGoodWeight;
    return fInFront * fFrontWeight + fWeighted;
}

static inline void OffensiveThreeValues(
    const FuzzyVariant& value,
    float fNotFarToTheirNet,
    float fOpen,
    float fLosing,
    float& fGoodToShoot,
    float& fInFrontScore)
{
    fGoodToShoot = OffensiveDanger(value);
    fGoodToShoot = (fNotFarToTheirNet >= fGoodToShoot) ? fNotFarToTheirNet : fGoodToShoot;
    fGoodToShoot = (fOpen >= fGoodToShoot) ? fOpen : fGoodToShoot;
    fInFrontScore = InFrontOfTheirNet(g_pScriptCurrentFielder);
}

static inline FuzzyVariant OffensiveThreeVariant(
    const FuzzyVariant& value,
    float fNotFarToTheirNet,
    float fOpen,
    float fLosing)
{
    float fGoodToShoot = OffensiveDanger(value);
    fGoodToShoot = (fNotFarToTheirNet >= fGoodToShoot) ? fNotFarToTheirNet : fGoodToShoot;
    fGoodToShoot = (fOpen >= fGoodToShoot) ? fOpen : fGoodToShoot;
    float fInFrontScore = InFrontOfTheirNet(g_pScriptCurrentFielder);
    float fGoodWeight = 0.55f;
    float fLosingWeight = 0.25f;
    float fFrontWeight = 0.2f;
    return FuzzyVariant(
        fGoodToShoot * fGoodWeight
        + (fLosing * fLosingWeight + fInFrontScore * fFrontWeight));
}

extern unsigned char g_bScriptQuestionCachingUseSTD;
extern unsigned char g_bScriptQuestionCachingOn;

static inline unsigned char OffensiveLookup(unsigned long hash, FuzzyVariant& returnVal)
{
    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    FuzzyVariant* pValue;

    cache->mTotalLookups++;

    if (g_bScriptQuestionCachingUseSTD)
    {
        StdMapNode* stdFound = (StdMapNode*)cache->mQuestionCacheMapSTD.find(hash).ptr_;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            returnVal = stdFound->value;
            return 1;
        }
    }
    else
    {
        unsigned long key;
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = cache->mQuestionCacheMap.m_Root;
        key = hash;
        unsigned char found;

        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
            {
                cmpResult = 0;
            }
            else if (key < node->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&pValue != NULL)
                {
                    pValue = &node->value;
                }
                found = 1;
                goto found_done;
            }
            if (cmpResult < 0)
            {
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.left;
            }
            else
            {
                node = (AVLTreeEntry<unsigned long, FuzzyVariant>*)node->node.right;
            }
        }

        found = 0;

    found_done:

        if (found)
        {
            cache->mCacheHits++;
            returnVal = *pValue;
            return 1;
        }
    }

    return 0;
}

static inline void OffensiveAddAVL(
    ScriptQuestionCache* cache,
    unsigned long& key,
    const FuzzyVariant& variant)
{
    AVLTreeNode* existingNode;
    cache->mQuestionCacheMap.AddAVLNode(
        (AVLTreeNode**)&cache->mQuestionCacheMap.m_Root,
        (void*)&key,
        (void*)&variant,
        &existingNode,
        cache->mQuestionCacheMap.m_NumElements);
    if (existingNode == NULL)
    {
        cache->mQuestionCacheMap.m_NumElements++;
    }
}

static inline const FuzzyVariant& OffensiveAddToCache(
    ScriptQuestionCache* cache,
    unsigned long hash,
    const FuzzyVariant& variant,
    const char* name)
{
    unsigned long key = hash;
    if (g_bScriptQuestionCachingOn)
    {
        unsigned char useSTD = g_bScriptQuestionCachingUseSTD;
        const FuzzyVariant* variantPtr = &variant;
        if (useSTD)
        {
            std::pair<const unsigned long, FuzzyVariant>& pair =
                cache->mQuestionCacheMapSTD.tree_.find_or_insert<unsigned long, FuzzyVariant>(key);
            pair.second = *variantPtr;
        }
        else
        {
            OffensiveAddAVL(cache, key, *variantPtr);
        }
    }
    return variant;
}

FuzzyVariant Fuzzy::InGoodWindupPosition(cFielder* TheFielder)
{
    extern unsigned char g_bScriptQuestionCachingUseSTD;
    extern unsigned char g_bScriptQuestionCachingOn;

    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pScriptCurrentTeam;
    extern cGame* g_pGame;

    extern float InOffensiveZone(cPlayer*);
    extern float FarToTheirNet(cPlayer*);
    extern float Invincible(cFielder*);
    extern float WideOpen(cFielder*);
    extern float InFrontOfTheirNet(cFielder*);
    extern float LikelyToScore(cFielder*);
    extern float Losing(cTeam*);
    extern float TimeNearlyOver(cGame*);
    extern float FLESS(float, float);
    extern float FGREATER(float, float);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    volatile unsigned long funcAddrTemp = (unsigned long)InGoodWindupPosition;
    unsigned long hash = OffensiveHash(funcAddrTemp + ((Variant*)&fvFielder)->GetHash());
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    if (OffensiveLookup(hash, bestValue))
    {
        bestValue.Confidence = bestValue.Confidence;
        OffensiveAddToCache(ScriptQuestionCache::Instance(), hash, bestValue, NULL);
        return bestValue;
    }

    float fTrueConfidence = OffensiveInitialMax(
        FLESS(InOffensiveZone(g_pScriptCurrentFielder), 0.4f));
    float fFalseConfidence;
    float fBranchRatio;
    OffensiveBranchValues(fTrueConfidence, fBranchRatio, fFalseConfidence);
    float fMinVal;
    float fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(0.0f);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            double d = fConfidence;
            fConfidence = (float)d * fBranchRatio;
        }

        float fTrueConfidence = Invincible(g_pScriptCurrentFielder);
        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fBranchRatio;
        float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                double d = fConfidence;
                fConfidence = (float)d * fBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                bestValue = FuzzyVariant(FGREATER(InOffensiveZone(g_pScriptCurrentFielder), 0.0f));
            }
        }

        if (fFalseConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
            if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            {
                double d = fConfidence;
                fConfidence = (float)d * fBranchRatio;
            }

            float fOpen = FGREATER(WideOpen(g_pScriptCurrentFielder), 0.4f);
            float fOpenInOffensiveZone = InOffensiveZone(g_pScriptCurrentFielder);
            if (fOpenInOffensiveZone <= fOpen)
            {
                fOpen = fOpenInOffensiveZone;
            }

            float fNotInDanger = OffensiveIdentity(1.0f - OffensiveDanger(InDanger(g_pScriptCurrentFielder)));
            float fNotFarToTheirNet = FarToTheirNet(g_pScriptCurrentFielder);
            fNotFarToTheirNet = 1.0f - fNotFarToTheirNet;
            fNotFarToTheirNet = (fNotFarToTheirNet <= fNotInDanger) ? fNotFarToTheirNet : fNotInDanger;

            float fLosing = Losing(g_pScriptCurrentTeam);
            float fTimeNearlyOver = TimeNearlyOver(g_pGame);
            if (fTimeNearlyOver <= fLosing)
            {
                fLosing = fTimeNearlyOver;
            }

            float fInFront = FLESS(InFrontOfTheirNet(g_pScriptCurrentFielder), 0.4f);
            float fNotInFront = 1.0f - fInFront;
            float fInFrontMin = (fInFront <= fNotInFront) ? fInFront : fNotInFront;
            float fInFrontMax = (fInFront >= fNotInFront) ? fInFront : fNotInFront;
            float fInFrontBranchRatio = fInFrontMin / fInFrontMax;

            if (fInFront > 0.0f)
            {
                SaveConfidence PushDOM4(&fConfidence);

                fConfidence = (fConfidence <= fInFront) ? fConfidence : fInFront;
                if (fConfidence < fInFront && fInFront < 0.5f)
                {
                    double d = fConfidence;
                    fConfidence = (float)d * fInFrontBranchRatio;
                }

                float fLosingFalse = 1.0f - fLosing;
                float fLosingMin = (fLosing <= fLosingFalse) ? fLosing : fLosingFalse;
                float fLosingMax = (fLosing >= fLosingFalse) ? fLosing : fLosingFalse;
                float fLosingBranchRatio = fLosingMin / fLosingMax;

                if (fLosing > 0.0f)
                {
                    SaveConfidence PushDOM5(&fConfidence);

                    fConfidence = (fConfidence <= fLosing) ? fConfidence : fLosing;
                    if (fConfidence < fLosing && fLosing < 0.5f)
                    {
                        double d = fConfidence;
                        fConfidence = (float)d * fLosingBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = FuzzyVariant(OffensiveHalfScore(
                            GoodToShoot(g_pScriptCurrentFielder), fLosing));
                    }
                }

                if (fLosingFalse > 0.0f)
                {
                    SaveConfidence PushDOM5(&fConfidence);

                    fConfidence = (fConfidence <= fLosingFalse) ? fConfidence : fLosingFalse;
                    if (fConfidence < fLosingFalse && fLosingFalse < 0.5f)
                    {
                        double d = fConfidence;
                        fConfidence = (float)d * fLosingBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = GoodToShoot(g_pScriptCurrentFielder);
                    }
                }
            }

            if (fNotInFront > 0.0f)
            {
                SaveConfidence PushDOM4(&fConfidence);

                fConfidence = (fConfidence <= fNotInFront) ? fConfidence : fNotInFront;
                if (fConfidence < fNotInFront && fNotInFront < 0.5f)
                {
                    double d = fConfidence;
                    fConfidence = (float)d * fInFrontBranchRatio;
                }

                fTrueConfidence = LikelyToScore(g_pScriptCurrentFielder);
                fFalseConfidence = OffensiveIdentity(1.0f - fTrueConfidence);
                fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMinVal / fMaxVal;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM5(&fConfidence);

                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    {
                        double d = fConfidence;
                        fConfidence = (float)d * fBranchRatio;
                    }

                    if (fConfidence > fBestConfidence)
                    {
                        fBestConfidence = fConfidence;
                        bestValue = FuzzyVariant(fConfidence);
                    }
                }

                {
                    float fTrueConfidence = fLosing;
                    float fFalseConfidence = 1.0f - fTrueConfidence;
                    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                    float fBranchRatio = fMinVal / fMaxVal;

                    if (fTrueConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM5(&fConfidence);

                        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        {
                            double d = fConfidence;
                            fConfidence = (float)d * fBranchRatio;
                        }

                        if (fConfidence > fBestConfidence)
                        {
                            fBestConfidence = fConfidence;
                            bestValue = OffensiveThreeVariant(
                                GoodToShoot(g_pScriptCurrentFielder),
                                fNotFarToTheirNet,
                                fOpen,
                                fLosing);
                        }
                    }

                    if (fFalseConfidence > 0.0f)
                    {
                        SaveConfidence PushDOM5(&fConfidence);

                        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
                        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                        {
                            double d = fConfidence;
                            fConfidence = (float)d * fBranchRatio;
                        }

                        if (fConfidence > fBestConfidence)
                        {
                            fBestConfidence = fConfidence;
                            bestValue = FuzzyVariant(OffensiveTwoScore(
                                GoodToShoot(g_pScriptCurrentFielder),
                                fNotFarToTheirNet,
                                fOpen));
                        }
                    }
                }
            }
        }
    }

    bestValue.Confidence = fBestConfidence;

    OffensiveAddToCache(ScriptQuestionCache::Instance(), hash, bestValue, NULL);

    return bestValue;
}

/**
 * Offset/Address/Size: 0x2B2C | 0x8008F5B8 | size: 0x5FC
 */
FuzzyVariant Fuzzy::CutAndBreak(cFielder* TheFielder)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    float fTrueConfidence = InOffensiveZone(TheFielder);
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        SSearchCutAndBreak ssearch = SSearchCutAndBreak(TheFielder);
        nlVector3 v3Position;
        nlVector3& (*PositionOfFielder)(cFielder*) = PositionOf<cFielder*>;
        float fCutAndBreakScore = ssearch.FindBestPosition(v3Position, PositionOfFielder(TheFielder), DIR_NONE, NULL, 8.0f, 0x8000);

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(fCutAndBreakScore);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(0.0f);
        }
    }

    bestValue.Confidence = fBestConfidence;

    return bestValue;
}

/**
 * Offset/Address/Size: 0x22A0 | 0x8008ED2C | size: 0x88C
 * TODO: 99.85% match - chip-shot confidence register and defensive temporary
 * register/stack allocation differ.
 */
FuzzyVariant Fuzzy::DoShooting(float fConfidence, cDecisionEntity* pDecision)
{
    extern cTeam* g_pScriptCurrentTeam;
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern cGame* g_pGame;
    extern FuzzyVariant fvNotSet;

    float fBestConfidence = 0.0f;

    float fTrueConfidence = InGoodWindupPosition(g_pScriptCurrentFielder).mData.f;
    float fFalseConfidence = 1.0f - fTrueConfidence;

    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        float fCurrentConfidence = fConfidence;
        if (0.0f >= fCurrentConfidence)
            fBestConfidence = 0.0f;
        else
            fBestConfidence = fCurrentConfidence;

        pDecision->QueueActionSetDesire(20, fConfidence, -1.0f, fvNotSet, fvNotSet);

        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_ShootingChance, Shooter(g_pScriptCurrentFielder));
    }

    {
        fTrueConfidence = OffensiveIdentity(GoodToChipShot(g_pScriptCurrentFielder).mData.f);
        float fFarToMyNet = FarToMyNet(g_pScriptCurrentFielder);
        fTrueConfidence = (fFarToMyNet <= fTrueConfidence) ? fFarToMyNet : fTrueConfidence;
        float fFalseConfidence = 1.0f - fTrueConfidence;

        float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            if (fBestConfidence >= fConfidence)
                fBestConfidence = fBestConfidence;
            else
                fBestConfidence = fConfidence;

            pDecision->QueueActionSetDesire(14, fConfidence, 0.0f, FuzzyVariant(false), FuzzyVariant(true));

            SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_ChipShotChance, Shooter(g_pScriptCurrentFielder));
        }
    }

    {
        fTrueConfidence = Winning(g_pScriptCurrentTeam);
        float fTimeNearlyOver = TimeNearlyOver(g_pGame);
        fTrueConfidence = (fTimeNearlyOver <= fTrueConfidence) ? fTimeNearlyOver : fTrueConfidence;

        float fStunnedGoalie;
        float fInDanger = InDanger(g_pScriptCurrentFielder).mData.f;
        fStunnedGoalie = Stunned(g_pScriptCurrentTeam->GetGoalie());
        float fCloseToNet = CloseToMyNet(g_pScriptCurrentFielder);

        fStunnedGoalie = (fStunnedGoalie <= fInDanger) ? fStunnedGoalie : fInDanger;
        fStunnedGoalie = (fCloseToNet <= fStunnedGoalie) ? fCloseToNet : fStunnedGoalie;

        float fFurthestBack = FGREATER(FurthestBackOnMyTeam(g_pScriptCurrentFielder).mData.f, 0.5f);
        fStunnedGoalie = (fStunnedGoalie >= fTrueConfidence) ? fStunnedGoalie : fTrueConfidence;
        fStunnedGoalie = (fFurthestBack >= fStunnedGoalie) ? fFurthestBack : fStunnedGoalie;

        fTrueConfidence = InDefensiveZone(g_pScriptCurrentFielder);
        fTrueConfidence = (fTrueConfidence <= fStunnedGoalie) ? fTrueConfidence : fStunnedGoalie;

        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            if (fBestConfidence >= fConfidence)
                fBestConfidence = fBestConfidence;
            else
                fBestConfidence = fConfidence;

            pDecision->QueueActionSetDesire(14, fConfidence, -1.0f, fvNotSet, fvNotSet);
            pDecision->m_pLastQueuedAction->m_fSelectionChance = 0.3f;
        }
    }

    return FuzzyVariant(fBestConfidence);
}

/**
 * Offset/Address/Size: 0x1EF8 | 0x8008E984 | size: 0x3A8
 */
FuzzyVariant Fuzzy::FurthestBackOnMyTeam(cFielder* TheFielder)
{
    FuzzyVariant bestValue;
    float fFarTo;
    float fTotalUpfieldScore;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    fTotalUpfieldScore = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        cFielder* TeamMate = TheFielder->m_pTeam->GetFielder(i);

        if (TeamMate != TheFielder)
        {
            float fUpfield;

            fFarTo = FarTo((cPlayer*)TeamMate, (cPlayer*)TheFielder);
            fUpfield = UpfieldFrom((cPlayer*)TeamMate, (cPlayer*)TheFielder);

            fUpfield = (fUpfield <= fFarTo) ? fUpfield : fFarTo;
            fTotalUpfieldScore += fUpfield;
        }
    }

    fTotalUpfieldScore = fTotalUpfieldScore / 3.0f;

    FuzzyVariant fvResult(fTotalUpfieldScore);
    bestValue = fvResult;
    bestValue.Confidence = 1.0f;

    return bestValue;
}

/**
 * Offset/Address/Size: 0x560 | 0x8008CFEC | size: 0x1998
 */
FuzzyVariant Fuzzy::UsePowerupOffensive(float fConfidence, cDecisionEntity* pDecision)
{
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pScriptCurrentTeam;
    extern FuzzyVariant fvNotSet;

    float fBestConfidence = 0.0f;
    FuzzyVariant theTarget = GetPowerupTargetOffensive(g_pScriptCurrentTeam);

    float fPressured = Pressured(g_pScriptCurrentFielder);
    float fCaptain = Captain(g_pScriptCurrentFielder);

    float fSmallWeight = 0.2f;
    float fTargetWeight = 0.6f;
    float fTrueConfidence = theTarget.Confidence * fTargetWeight + fCaptain * fSmallWeight + fPressured * fSmallWeight;
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        {
            float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 0);
            float fLFalse = 1.0f - fLikely;
            float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
            float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
            float fLRatio = fLMin / fLMax;

            if (fLikely > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                if (fConfidence < fLikely && fLikely < 0.5f)
                    fConfidence = fConfidence * fLRatio;

                float fCurrentConfidence = fConfidence;
                if (0.0f >= fCurrentConfidence)
                    fBestConfidence = 0.0f;
                else
                    fBestConfidence = fCurrentConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(0), theTarget);
            }
        }

        {
            float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 1);
            float fLFalse = 1.0f - fLikely;
            float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
            float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
            float fLRatio = fLMin / fLMax;

            if (fLikely > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                if (fConfidence < fLikely && fLikely < 0.5f)
                    fConfidence = fConfidence * fLRatio;

                if (fBestConfidence >= fConfidence)
                    fBestConfidence = fBestConfidence;
                else
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(1), theTarget);
            }
        }

        {
            float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 2);
            float fLFalse = 1.0f - fLikely;
            float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
            float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
            float fLRatio = fLMin / fLMax;

            if (fLikely > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                if (fConfidence < fLikely && fLikely < 0.5f)
                    fConfidence = fConfidence * fLRatio;

                if (fBestConfidence >= fConfidence)
                    fBestConfidence = fBestConfidence;
                else
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(2), theTarget);
            }
        }

        {
            float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 3);
            float fLFalse = 1.0f - fLikely;
            float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
            float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
            float fLRatio = fLMin / fLMax;

            if (fLikely > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                if (fConfidence < fLikely && fLikely < 0.5f)
                    fConfidence = fConfidence * fLRatio;

                if (fBestConfidence >= fConfidence)
                    fBestConfidence = fBestConfidence;
                else
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(3), theTarget);
            }
        }

        {
            float fClosing = ClosingTo(g_pScriptCurrentFielder, theTarget.mData.pPlayer);
            float fNear = NearTo(g_pScriptCurrentFielder, theTarget.mData.pPlayer);
            float fTrueConfidence = (fNear + fClosing) * 0.5f;
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                {
                    float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 4);
                    float fLFalse = 1.0f - fLikely;
                    float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
                    float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
                    float fLRatio = fLMin / fLMax;

                    if (fLikely > 0.0f)
                    {
                        SaveConfidence PushDOM2(&fConfidence);
                        fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                        if (fConfidence < fLikely && fLikely < 0.5f)
                            fConfidence = fConfidence * fLRatio;

                        if (fBestConfidence >= fConfidence)
                            fBestConfidence = fBestConfidence;
                        else
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(4), theTarget);
                    }
                }
            }
        }

        {
            float fTrueConfidence = FGREATER(InGoodWindupPosition(g_pScriptCurrentFielder).mData.f, 0.3f);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                {
                    float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 5);
                    float fLFalse = 1.0f - fLikely;
                    float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
                    float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
                    float fLRatio = fLMin / fLMax;

                    if (fLikely > 0.0f)
                    {
                        SaveConfidence PushDOM2(&fConfidence);
                        fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                        if (fConfidence < fLikely && fLikely < 0.5f)
                            fConfidence = fConfidence * fLRatio;

                        if (fBestConfidence >= fConfidence)
                            fBestConfidence = fBestConfidence;
                        else
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(5), theTarget);
                    }
                }
            }
        }
    }

    {
        float fCloseSideline = CloseToSideline(g_pScriptCurrentFielder);
        float fFacingSideline = FacingSideline(g_pScriptCurrentFielder);
        fTrueConfidence = 1.0f - ((fFacingSideline <= fCloseSideline) ? fFacingSideline : fCloseSideline);
        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fTrueConfidence = 1.0f - OnMushrooms(g_pScriptCurrentFielder);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                float fOpen = Open(g_pScriptCurrentFielder);
                float fOpenToNet = OpenToTheirNet(g_pScriptCurrentFielder);
                const FuzzyVariant& goodBallCarrier = GoodBallCarrier(g_pScriptCurrentFielder);
                float fGoodBallCarrier = goodBallCarrier.mData.f;

                float fOpenWeight = 0.15f;
                float fCarrierWeight = 0.55f;
                float fNetWeight = 0.3f;
                fOpen = fOpen * fOpenWeight + (fGoodBallCarrier * fCarrierWeight + fOpenToNet * fNetWeight);
                fTrueConfidence = OnBreakaway(g_pScriptCurrentFielder);
                fTrueConfidence = (fTrueConfidence >= fOpen) ? fTrueConfidence : fOpen;

                float fFalseConfidence = 1.0f - fTrueConfidence;
                float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                float fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    {
                        float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 7);
                        float fLFalse = 1.0f - fLikely;
                        float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
                        float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
                        float fLRatio = fLMin / fLMax;

                        if (fLikely > 0.0f)
                        {
                            SaveConfidence PushDOM4(&fConfidence);
                            fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                            if (fConfidence < fLikely && fLikely < 0.5f)
                                fConfidence = fConfidence * fLRatio;

                            if (fBestConfidence >= fConfidence)
                                fBestConfidence = fBestConfidence;
                            else
                                fBestConfidence = fConfidence;
                            pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(7), fvNotSet);
                        }
                    }
                }
            }
        }
    }

    {
        fTrueConfidence = Captain(g_pScriptCurrentFielder);
        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fTrueConfidence = FGREATER(InGoodWindupPosition(g_pScriptCurrentFielder).mData.f, 0.0f);
            float fWideOpen = WideOpen(g_pScriptCurrentFielder);
            float fShotConfidence = (1.0f - fWideOpen >= fTrueConfidence) ? (1.0f - fWideOpen) : fTrueConfidence;

            float fFalseConfidence = 1.0f - fShotConfidence;
            float fMin = (fShotConfidence <= fFalseConfidence) ? fShotConfidence : fFalseConfidence;
            float fMax = (fShotConfidence >= fFalseConfidence) ? fShotConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;

            if (fShotConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fShotConfidence) ? fConfidence : fShotConfidence;

                if (fConfidence < fShotConfidence && fShotConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                {
                    float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 8);
                    float fLFalse = 1.0f - fLikely;
                    float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
                    float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
                    float fLRatio = fLMin / fLMax;

                    if (fLikely > 0.0f)
                    {
                        SaveConfidence PushDOM3(&fConfidence);
                        fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

                        if (fConfidence < fLikely && fLikely < 0.5f)
                            fConfidence = fConfidence * fLRatio;

                        if (fBestConfidence >= fConfidence)
                            fBestConfidence = fBestConfidence;
                        else
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(8), fvNotSet);
                    }
                }
            }
        }
    }

    {
        float fLikely = LikelyToUsePowerup(g_pScriptCurrentFielder, 6);
        float fLFalse = 1.0f - fLikely;
        float fLMin = (fLikely <= fLFalse) ? fLikely : fLFalse;
        float fLMax = (fLikely >= fLFalse) ? fLikely : fLFalse;
        float fLRatio = fLMin / fLMax;

        if (fLikely > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fLikely) ? fConfidence : fLikely;

            if (fConfidence < fLikely && fLikely < 0.5f)
                fConfidence = fConfidence * fLRatio;

            if (fBestConfidence >= fConfidence)
                fBestConfidence = fBestConfidence;
            else
                fBestConfidence = fConfidence;
            pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(6), fvNotSet);
        }
    }

    return FuzzyVariant(fBestConfidence);
}

/**
 * Offset/Address/Size: 0x0 | 0x8008CA8C | size: 0x560
 */
FuzzyVariant Fuzzy::GetPowerupTargetOffensive(cTeam* TheTeam)
{
    extern cTeam* g_pScriptOtherTeam;
    extern cFielder* g_pScriptCurrentFielder;

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;
    FuzzyVariant fvTeam(TheTeam);
    ((Variant*)&fvTeam)->GetHash();
    FuzzyVariant fvTeam2(TheTeam);
    for (int i = 0; i < 4; i++)
    {
        cFielder* theOpponent = g_pScriptOtherTeam->GetFielder(i);
        float fNotInvincible = Invincible(theOpponent);
        fNotInvincible = 1.0f - fNotInvincible;
        float fTrueConfidence = Incapacitated((cPlayer*)theOpponent);
        fTrueConfidence = 1.0f - fTrueConfidence;
        if (fTrueConfidence <= fNotInvincible)
        {
            fTrueConfidence = fTrueConfidence;
        }
        else
        {
            fTrueConfidence = fNotInvincible;
        }
        float fFalseConfidence = 1.0f - fTrueConfidence;
        float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        float fBranchRatio = fMin / fMax;
        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;
            float fMarking = Marking(theOpponent, (cPlayer*)g_pScriptCurrentFielder);
            float fDownfield = DownfieldFrom((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)theOpponent);
            float fClosing = ClosingTo((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)theOpponent);
            float fFar = FarTo((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)theOpponent);
            float fLowWeight = 0.2f;
            float fHighWeight = 0.3f;
            float fTrueConfidence2 = fMarking * fHighWeight + (fDownfield * fHighWeight + ((1.0f - fFar) * fLowWeight + fClosing * fLowWeight));
            float fFalseConfidence2 = 1.0f - fTrueConfidence2;
            float fMin2 = (fTrueConfidence2 <= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
            float fMax2 = (fTrueConfidence2 >= fFalseConfidence2) ? fTrueConfidence2 : fFalseConfidence2;
            float fBranchRatio2 = fMin2 / fMax2;
            if (fTrueConfidence2 > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence2) ? fConfidence : fTrueConfidence2;
                if (fConfidence < fTrueConfidence2 && fTrueConfidence2 < 0.5f)
                    fConfidence = fConfidence * fBranchRatio2;
                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant((cPlayer*)theOpponent);
                }
            }
        }
    }
    bestValue.Confidence = fBestConfidence;
    return bestValue;
}
