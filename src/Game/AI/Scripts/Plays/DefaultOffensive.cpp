#include "Game/AI/Scripts/Plays/DefaultOffensive.h"

#include "Game/AI/Fuzzy.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/AI/SpaceSearch.h"

class cTeam;

class SaveConfidence
{
public:
    SaveConfidence(float* pFloat)
        : m_savedValue(*pFloat)
        , m_pFloat(pFloat)
    {
    }

    ~SaveConfidence() { *m_pFloat = m_savedValue; }

    float m_savedValue;
    float* m_pFloat;
};

static bool sFalse = false;
static float sZeroCutAndBreak = 0.0f;
static bool sTrue = true;

/**
 * Offset/Address/Size: 0x0 | 0x80093A64 | size: 0x8
 */
template <typename T>
nlVector3& PositionOf(T pObject)
{
    return pObject->m_v3Position;
}
template nlVector3& PositionOf<cFielder*>(cFielder*);

/**
 * Offset/Address/Size: 0x6B9C | 0x80093628 | size: 0x43C
 * TODO: 97.49% match - r29/r30 swap in bool-to-FuzzyVariant branch temporaries.
 */
void Fuzzy::AbortOffensivePlay(cDecisionEntity*)
{
    extern cTeam* g_pScriptCurrentTeam;

    FuzzyVariant bestValue;
    bool bResult;
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
            bResult = sFalse;
            bestValue = FuzzyVariant(bResult);
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
            bResult = sTrue;
            bestValue = FuzzyVariant(bResult);
        }
    }

    bestValue.Confidence = fBestConfidence;

    FuzzyVariant* pOut = (FuzzyVariant*)this;
    new (pOut) FuzzyVariant;
    *pOut = bestValue;
}

/**
 * Offset/Address/Size: 0x51D8 | 0x80091C64 | size: 0x19C4
 */
void Fuzzy::DefaultOffensivePlay(cDecisionEntity* pDecision)
{
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pScriptCurrentTeam;
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern FuzzyVariant fvNotSet;
    extern FuzzyVariant GetStrategicBallCarrier__5FuzzyFP5cTeam(cTeam*);

    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = BallOwner(g_pScriptCurrentFielder);
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

        FuzzyVariant doShooting;
        ((Fuzzy*)&doShooting)->DoShooting(fConfidence, pDecision);
        float fDoShooting = (doShooting.mData.f >= 0.0f) ? doShooting.mData.f : 0.0f;

        FuzzyVariant doPassing;
        ((Fuzzy*)&doPassing)->DoPassing(fConfidence, pDecision);
        float fDoPassing = doPassing.mData.f;
        fDoPassing = (fDoPassing >= fDoShooting) ? fDoPassing : fDoShooting;

        fBestConfidence = fDoPassing;

        FuzzyVariant goodBallCarrier;
        ((Fuzzy*)&goodBallCarrier)->GoodBallCarrier(g_pScriptCurrentFielder);
        float fGoodBallCarrier = goodBallCarrier.mData.f;

        float fNotRepeatingDeke = 1.0f - RepeatingLastDesire(g_pScriptCurrentFielder, edDeke);
        float fNotCloseSideline = 1.0f - CloseToSideline(g_pScriptCurrentFielder);
        float fNotInvincible = 1.0f - Invincible(g_pScriptCurrentFielder);

        fTrueConfidence = InControlOfBall(g_pScriptCurrentFielder);
        fNotCloseSideline = (fNotCloseSideline <= fNotRepeatingDeke) ? fNotCloseSideline : fNotRepeatingDeke;
        fNotInvincible = (fNotInvincible <= fNotCloseSideline) ? fNotInvincible : fNotCloseSideline;
        fTrueConfidence = (fTrueConfidence <= fNotInvincible) ? fTrueConfidence : fNotInvincible;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fTrueConfidence = 1.0f - FGREATER(Attacked(g_pScriptCurrentFielder), 0.4f);
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

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;

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

                fTrueConfidence = 1.0f - FLESS(Open(g_pScriptCurrentFielder), 0.5f);
                fFalseConfidence = 1.0f - fTrueConfidence;

                fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
                fBranchRatio = fMin / fMax;

                if (fTrueConfidence > 0.0f)
                {
                    SaveConfidence PushDOM4(&fConfidence);
                    fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                    if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                        fConfidence = fConfidence * fBranchRatio;

                    fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;

                    pDecision->QueueActionSetDesire(2, fConfidence, -1.0f, fvNotSet, fvNotSet);

                    SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(0.5f * pTweaks->Off_DekeChance, Deker(g_pScriptCurrentFielder));
                }
            }
        }

        fTrueConfidence = BallOwner(g_pScriptCurrentFielder);
        float fNotUserControlled = 1.0f - UserControlledT(g_pScriptCurrentTeam);
        fTrueConfidence = (fTrueConfidence <= fNotUserControlled) ? fTrueConfidence : fNotUserControlled;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            FuzzyVariant usePowerupOffensive;
            ((Fuzzy*)&usePowerupOffensive)->UsePowerupOffensive(fConfidence, pDecision);
            if (usePowerupOffensive.mData.f >= fBestConfidence)
                fBestConfidence = usePowerupOffensive.mData.f;
        }

        fTrueConfidence = FGREATER(fGoodBallCarrier, (fDoShooting >= fDoPassing) ? fDoShooting : fDoPassing);

        float fDifficult = FLESS(Difficult(g_pScriptCurrentTeam), 0.8f);
        float fFallback = (0.0f == fBestConfidence) ? 1.0f : 0.0f;

        fTrueConfidence = (fDifficult >= fGoodBallCarrier) ? fDifficult : fGoodBallCarrier;
        fTrueConfidence = (fFallback >= fTrueConfidence) ? fFallback : fTrueConfidence;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
            pDecision->QueueActionSetDesire(9, fConfidence, -1.0f, fvNotSet, fvNotSet);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        FuzzyVariant cutAndBreak;
        ((Fuzzy*)&cutAndBreak)->CutAndBreak(g_pScriptCurrentFielder);
        float fCutAndBreak = cutAndBreak.mData.f;

        fTrueConfidence = Striker(g_pScriptCurrentFielder);
        fTrueConfidence = (fTrueConfidence <= fCutAndBreak) ? fTrueConfidence : fCutAndBreak;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;

            pDecision->QueueActionSetDesire(1, fConfidence, -1.0f, fvNotSet, fvNotSet);

            SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            pDecision->m_pLastQueuedAction->m_fSelectionChance = pTweaks->Off_CutAndBreakChance;
        }

        FuzzyVariant strategicBallOwner = GetStrategicBallCarrier__5FuzzyFP5cTeam(g_pScriptCurrentTeam);

        float fBallOwnerGoalie = BallOwner(g_pScriptCurrentTeam->GetGoalie());
        float fStriker = Striker(g_pScriptCurrentFielder);
        float fWinger = Winger(g_pScriptCurrentFielder);

        fStriker = (fStriker >= fBallOwnerGoalie) ? fStriker : fBallOwnerGoalie;
        fWinger = (fWinger >= fStriker) ? fWinger : fStriker;

        fFalseConfidence = 1.0f - fWinger;
        fTrueConfidence = fWinger;

        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            float fWindup = WindingUpForShot((cFielder*)strategicBallOwner.mData.pPlayer);
            float fNotNearToTheirNet = 1.0f - NearToTheirNet(g_pScriptCurrentFielder);
            float fNotStrategicConfidence = 1.0f - strategicBallOwner.Confidence;

            fNotNearToTheirNet = (fNotNearToTheirNet >= fWindup) ? fNotNearToTheirNet : fWindup;
            fNotStrategicConfidence = (fNotStrategicConfidence >= fNotNearToTheirNet) ? fNotStrategicConfidence : fNotNearToTheirNet;

            fFalseConfidence = 1.0f - fNotStrategicConfidence;
            fTrueConfidence = fNotStrategicConfidence;

            fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;

                pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);
                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
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

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);
                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                pDecision->QueueActionSetDesire(3, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }
        }
    }

    new ((FuzzyVariant*)this) FuzzyVariant(fBestConfidence);
}

/**
 * Offset/Address/Size: 0x4A94 | 0x80091520 | size: 0x744
 */
void Fuzzy::DoPassing(float fConfidence, cDecisionEntity* pDecision)
{
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern FuzzyVariant GetBestPassTarget__5FuzzyFP7cPlayer(cPlayer*);

    float fBestConfidence = 0.0f;
    bool bResult;

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
        FuzzyVariant theBestPassTarget = GetBestPassTarget__5FuzzyFP7cPlayer(g_pScriptCurrentFielder);
        float fAdjustedConfidence = theBestPassTarget.Confidence * (1.0f - fAvoidingPowerups) + 1.0f * fAvoidingPowerups;

        fTrueConfidence = FGREATER(theBestPassTarget.Confidence, 0.15f);
        fTrueConfidence = (fTrueConfidence <= fAdjustedConfidence) ? fTrueConfidence : fAdjustedConfidence;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fTrueConfidence = OpenTo(g_pScriptCurrentFielder, theBestPassTarget.mData.pPlayer);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                fBestConfidence = (0.0f >= fConfidence) ? 0.0f : fConfidence;

                bResult = sFalse;
                pDecision->QueueActionSetDesire(19, fConfidence, 0.0f, theBestPassTarget, FuzzyVariant(bResult));

                SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_GroundPassChance, Passer(g_pScriptCurrentFielder));
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;

                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                if (fConfidence > fBestConfidence)
                    fBestConfidence = fConfidence;

                bResult = sTrue;
                pDecision->QueueActionSetDesire(19, fConfidence, 0.0f, theBestPassTarget, FuzzyVariant(bResult));

                SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_VolleyPassChance, Passer(g_pScriptCurrentFielder));
            }
        }
    }

    new ((FuzzyVariant*)this) FuzzyVariant(fBestConfidence);
}

/**
 * Offset/Address/Size: 0x4490 | 0x80090F1C | size: 0x604
 * TODO: 98.96% match - f30/f31 register swap for fBestConfidence/fWindupScore
 */
void Fuzzy::GoodBallCarrier(cFielder* TheFielder)
{
    extern cFielder* g_pScriptCurrentFielder;

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    float fWindupScore = InGoodWindupPosition(g_pScriptCurrentFielder).mData.f;

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
            float fLessWindup = FLESS(fWindupScore, 0.8f);
            float fNotCloseToNet = 1.0f - CloseToMyNet(g_pScriptCurrentFielder);
            float fNotInDanger = 1.0f - InDangerDelayed(g_pScriptCurrentFielder).mData.f;
            fNotCloseToNet = (fNotCloseToNet <= fLessWindup) ? fNotCloseToNet : fLessWindup;
            fNotInDanger = (fNotInDanger <= fNotCloseToNet) ? fNotInDanger : fNotCloseToNet;
            bestValue = FuzzyVariant(fNotInDanger);
        }
    }

    bestValue.Confidence = fBestConfidence;

    FuzzyVariant* pOut = (FuzzyVariant*)this;
    new (pOut) FuzzyVariant;
    *pOut = bestValue;
}

/**
 * Offset/Address/Size: 0x3128 | 0x8008FBB4 | size: 0x1368
 * TODO: 37.75% match - stack frame/register allocation still diverges and
 * branch ordering in the nested confidence tree differs from target.
 */
FuzzyVariant Fuzzy::InGoodWindupPosition(cFielder* TheFielder)
{
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

    struct FuzzyMapPair
    {
        unsigned long key;
        FuzzyVariant value;
    };

    extern unsigned char g_bScriptQuestionCachingUseSTD;
    extern unsigned char g_bScriptQuestionCachingOn;
    extern void __find(StdMapNode * *outNode, void* tree, const unsigned long* key);
    extern FuzzyMapPair* __find_or_insert(void* tree, const unsigned long* key);

    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern float InFrontOfTheirNet(cFielder*);
    extern float IsPerfectPassInPlay();
    extern float ReceivingVolleyPass(cPlayer*);
    extern float CloseToTheirGoalie(cPlayer*);
    extern float NearToTheirNet(cPlayer*);
    extern float Stunned(Goalie*);
    extern float Shooter(cFielder*);
    extern float CalcSelectChance(float, float);
    extern FuzzyVariant InDanger__5FuzzyFP8cFielder(cFielder*);
    extern FuzzyVariant GoodToShoot__5FuzzyFP8cFielder(cFielder*);

    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant fvFielder((cPlayer*)TheFielder);
    unsigned long hash = (unsigned long)InGoodWindupPosition + ((Variant*)&fvFielder)->GetHash();
    FuzzyVariant fvFielder2((cPlayer*)TheFielder);

    ScriptQuestionCache* cache = ScriptQuestionCache::Instance();
    cache->mTotalLookups++;

    unsigned char lookupFound = 0;
    FuzzyVariant* pValue;

    if (g_bScriptQuestionCachingUseSTD)
    {
        StdMapNode* stdNode;
        __find(&stdNode, &cache->mQuestionCacheMapSTD, &hash);
        StdMapNode* stdFound = stdNode;
        if ((StdMapNodeBase*)stdFound != &((StdMapTree*)&cache->mQuestionCacheMapSTD)->x4)
        {
            cache->mCacheHits++;
            bestValue = stdFound->value;
            lookupFound = 1;
        }
    }
    else
    {
        AVLTreeEntry<unsigned long, FuzzyVariant>* node = cache->mQuestionCacheMap.m_Root;
        unsigned long key = hash;

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
                lookupFound = 1;
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

        lookupFound = 0;

    found_done:

        if (lookupFound)
        {
            cache->mCacheHits++;
            bestValue = *pValue;
        }
    }

    if (lookupFound)
    {
        unsigned long hashCopy1 = hash;
        if (g_bScriptQuestionCachingOn)
        {
            if (g_bScriptQuestionCachingUseSTD)
            {
                FuzzyMapPair* pair = __find_or_insert(&cache->mQuestionCacheMapSTD, &hashCopy1);
                pair->value = bestValue;
            }
            else
            {
                AVLTreeNode* existingNode1;
                cache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&cache->mQuestionCacheMap.m_Root, (void*)&hashCopy1, (void*)&bestValue, &existingNode1, cache->mQuestionCacheMap.m_NumElements);
                if (existingNode1 == NULL)
                    cache->mQuestionCacheMap.m_NumElements++;
            }
        }
        return bestValue;
    }

    float fTrueConfidence = FarToTheirNet((cPlayer*)TheFielder);
    float fInFrontOfNet = 1.0f - InFrontOfTheirNet(TheFielder);
    if (fTrueConfidence < fInFrontOfNet)
    {
        fTrueConfidence = fInFrontOfNet;
    }

    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMinVal / fMaxVal;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > 0.0f)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        fTrueConfidence = IsPerfectPassInPlay();
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            if (fConfidence > fBestConfidence)
            {
                fBestConfidence = fConfidence;
                FuzzyVariant returnValue(1.0f);
                SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                bestValue = returnValue;
            }
        }

        fTrueConfidence = 1.0f - FarToTheirNet((cPlayer*)TheFielder);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMinVal / fMaxVal;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);

            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            {
                fConfidence = (float)fConfidence * fBranchRatio;
            }

            fTrueConfidence = ReceivingVolleyPass((cPlayer*)TheFielder);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMinVal = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMaxVal = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMinVal / fMaxVal;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger__5FuzzyFP8cFielder(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }

                float fNearToNet = NearToTheirNet((cPlayer*)TheFielder);
                fDanger = GoodToShoot__5FuzzyFP8cFielder(TheFielder).Confidence;
                if (fDanger < fNearToNet)
                {
                    fDanger = fNearToNet;
                }
                if (fGoalieStunned < fDanger)
                {
                    fDanger = fGoalieStunned;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_VolleyOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }

            if (fFalseConfidence > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);

                fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
                if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                {
                    fConfidence = (float)fConfidence * fBranchRatio;
                }

                Goalie* pGoalie = NULL;
                if (TheFielder != NULL)
                {
                    pGoalie = TheFielder->m_pTeam->GetOtherTeam()->GetGoalie();
                }

                float fGoalieStunned = Stunned(pGoalie);
                float fDanger = InDanger__5FuzzyFP8cFielder(TheFielder).Confidence;
                if (fDanger < fGoalieStunned)
                {
                    fDanger = fGoalieStunned;
                }
                fDanger = GoodToShoot__5FuzzyFP8cFielder(TheFielder).Confidence * 0.5f + fDanger * 0.5f;

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    FuzzyVariant returnValue(fDanger);
                    SkillTweaks* pSkillTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    returnValue.SelectionChance = CalcSelectChance(pSkillTweaks->Off_GroundOneTimerChance, Shooter(TheFielder));
                    bestValue = returnValue;
                }
            }
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);

        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
        {
            fConfidence = (float)fConfidence * fBranchRatio;
        }

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            FuzzyVariant returnValue(0.0f);
            bestValue = returnValue;
        }
    }

    bestValue.Confidence = fBestConfidence;

    unsigned long hashCopy2 = hash;
    if (g_bScriptQuestionCachingOn)
    {
        if (g_bScriptQuestionCachingUseSTD)
        {
            FuzzyMapPair* pair = __find_or_insert(&cache->mQuestionCacheMapSTD, &hashCopy2);
            pair->value = bestValue;
        }
        else
        {
            AVLTreeNode* existingNode2;
            cache->mQuestionCacheMap.AddAVLNode((AVLTreeNode**)&cache->mQuestionCacheMap.m_Root, (void*)&hashCopy2, (void*)&bestValue, &existingNode2, cache->mQuestionCacheMap.m_NumElements);
            if (existingNode2 == NULL)
                cache->mQuestionCacheMap.m_NumElements++;
        }
    }

    return bestValue;
}

/**
 * Offset/Address/Size: 0x2B2C | 0x8008F5B8 | size: 0x5FC
 * TODO: 95.38% match - 4 @sda21 constant pool label diffs for 1.0f/0.0f loads
 */
void Fuzzy::CutAndBreak(cFielder* TheFielder)
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
        float fCutAndBreakScore = ssearch.FindBestPosition(v3Position, PositionOf(TheFielder), DIR_NONE, NULL, 8.0f, 0x8000);

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
            bestValue = FuzzyVariant(sZeroCutAndBreak);
        }
    }

    bestValue.Confidence = fBestConfidence;

    FuzzyVariant* pOut = (FuzzyVariant*)this;
    new (pOut) FuzzyVariant;
    *pOut = bestValue;
}

/**
 * Offset/Address/Size: 0x22A0 | 0x8008ED2C | size: 0x88C
 * TODO: 95.60% match - register allocation differences in GoodToChipShot and
 * InDanger/FurthestBack confidence branches.
 */
void Fuzzy::DoShooting(float fConfidence, cDecisionEntity* pDecision)
{
    extern cTeam* g_pScriptCurrentTeam;
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern cGame* g_pGame;
    extern FuzzyVariant fvNotSet;
    extern FuzzyVariant GoodToChipShot__5FuzzyFP8cFielder(cFielder*);
    extern FuzzyVariant InDanger__5FuzzyFP8cFielder(cFielder*);

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

        if (0.0f >= fConfidence)
            fBestConfidence = 0.0f;
        else
            fBestConfidence = fConfidence;

        pDecision->QueueActionSetDesire(20, fConfidence, -1.0f, fvNotSet, fvNotSet);

        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_ShootingChance, Shooter(g_pScriptCurrentFielder));
    }

    fTrueConfidence = GoodToChipShot__5FuzzyFP8cFielder(g_pScriptCurrentFielder).mData.f;
    float fFarToMyNet = FarToMyNet(g_pScriptCurrentFielder);
    fTrueConfidence = (fFarToMyNet <= fTrueConfidence) ? fFarToMyNet : fTrueConfidence;
    fFalseConfidence = 1.0f - fTrueConfidence;

    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fBestConfidence < fConfidence)
            fBestConfidence = fConfidence;

        pDecision->QueueActionSetDesire(14, fConfidence, 0.0f, FuzzyVariant(sFalse), FuzzyVariant(sTrue));

        SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
        pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Off_ChipShotChance, Shooter(g_pScriptCurrentFielder));
    }

    fTrueConfidence = Winning(g_pScriptCurrentTeam);
    float fTimeNearlyOver = TimeNearlyOver(g_pGame);
    fTrueConfidence = (fTimeNearlyOver <= fTrueConfidence) ? fTimeNearlyOver : fTrueConfidence;

    float fInDanger = InDanger__5FuzzyFP8cFielder(g_pScriptCurrentFielder).mData.f;
    float fStunnedGoalie = Stunned(g_pScriptCurrentTeam->GetGoalie());
    float fCloseToNet = CloseToMyNet(g_pScriptCurrentFielder);

    fStunnedGoalie = (fStunnedGoalie <= fInDanger) ? fStunnedGoalie : fInDanger;
    fStunnedGoalie = (fCloseToNet <= fStunnedGoalie) ? fCloseToNet : fStunnedGoalie;

    float fFurthestBack = FGREATER(FurthestBackOnMyTeam(g_pScriptCurrentFielder).mData.f, 0.5f);
    fStunnedGoalie = (fStunnedGoalie >= fTrueConfidence) ? fStunnedGoalie : fTrueConfidence;
    fStunnedGoalie = (fFurthestBack >= fStunnedGoalie) ? fStunnedGoalie : fFurthestBack;

    fTrueConfidence = InDefensiveZone(g_pScriptCurrentFielder);
    fTrueConfidence = (fTrueConfidence <= fStunnedGoalie) ? fTrueConfidence : fStunnedGoalie;

    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        if (fBestConfidence < fConfidence)
            fBestConfidence = fConfidence;

        pDecision->QueueActionSetDesire(14, fConfidence, -1.0f, fvNotSet, fvNotSet);
        pDecision->m_pLastQueuedAction->m_fSelectionChance = 0.3f;
    }

    new ((FuzzyVariant*)this) FuzzyVariant(fBestConfidence);
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
void Fuzzy::UsePowerupOffensive(float fConfidence, cDecisionEntity* pDecision)
{
    extern cFielder* g_pScriptCurrentFielder;
    extern cTeam* g_pScriptCurrentTeam;
    extern FuzzyVariant fvNotSet;

    float fBestConfidence = 0.0f;
    FuzzyVariant theTarget = GetPowerupTargetOffensive(g_pScriptCurrentTeam);

    float fPressured = Pressured(g_pScriptCurrentFielder);
    float fCaptain = Captain(g_pScriptCurrentFielder);

    float fTrueConfidence = theTarget.Confidence * 0.6f + fCaptain * 0.2f + fPressured * 0.2f;
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

                fBestConfidence = (0.0f >= fConfidence) ? 0.0f : fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(0), theTarget);
            }
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

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
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

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
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

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
            pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(3), theTarget);
        }
    }

    fTrueConfidence = 1.0f - (ClosingTo(g_pScriptCurrentFielder, theTarget.mData.pPlayer) + NearTo(g_pScriptCurrentFielder, theTarget.mData.pPlayer)) * 0.5f;
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;

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

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(4), theTarget);
            }
        }
    }

    fTrueConfidence = FGREATER(InGoodWindupPosition(g_pScriptCurrentFielder).mData.f, 0.3f);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;

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

                fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(5), theTarget);
            }
        }
    }

    {
        float fCloseSideline = CloseToSideline(g_pScriptCurrentFielder);
        float fFacingSideline = FacingSideline(g_pScriptCurrentFielder);
        fTrueConfidence = 1.0f - ((fFacingSideline <= fCloseSideline) ? fFacingSideline : fCloseSideline);
        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            fTrueConfidence = 1.0f - OnMushrooms(g_pScriptCurrentFielder);
            fFalseConfidence = 1.0f - fTrueConfidence;
            fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            fBranchRatio = fMin / fMax;

            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM2(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                    fConfidence = fConfidence * fBranchRatio;

                float fOpen = Open(g_pScriptCurrentFielder);
                float fOpenToNet = OpenToTheirNet(g_pScriptCurrentFielder);
                FuzzyVariant goodBallCarrier;
                ((Fuzzy*)&goodBallCarrier)->GoodBallCarrier(g_pScriptCurrentFielder);
                float fGoodBallCarrier = goodBallCarrier.mData.f;

                float fOpenScore = fOpenToNet * 0.3f + fGoodBallCarrier * 0.55f + fOpen * 0.15f;
                fTrueConfidence = OnBreakaway(g_pScriptCurrentFielder);
                fTrueConfidence = (fTrueConfidence >= fOpenScore) ? fTrueConfidence : fOpenScore;

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

                            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                            pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(7), fvNotSet);
                        }
                    }
                }
            }
        }
    }

    fTrueConfidence = Captain(g_pScriptCurrentFielder);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;

    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        fTrueConfidence = FGREATER(InGoodWindupPosition(g_pScriptCurrentFielder).mData.f, 0.0f);
        float fNotWideOpen = 1.0f - WideOpen(g_pScriptCurrentFielder);
        fTrueConfidence = (fNotWideOpen >= fTrueConfidence) ? fNotWideOpen : fTrueConfidence;

        fFalseConfidence = 1.0f - fTrueConfidence;
        fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
        fBranchRatio = fMin / fMax;

        if (fTrueConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;

            if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
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

                    fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
                    pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(8), fvNotSet);
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

            fBestConfidence = (fBestConfidence >= fConfidence) ? fBestConfidence : fConfidence;
            pDecision->QueueActionSetDesire(18, fConfidence, 0.0f, FuzzyVariant(6), fvNotSet);
        }
    }

    new ((FuzzyVariant*)this) FuzzyVariant(fBestConfidence);
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
        float fNotInvincible = 1.0f - Invincible(theOpponent);
        float fTrueConfidence = 1.0f - Incapacitated((cPlayer*)theOpponent);
        fTrueConfidence = (fTrueConfidence <= fNotInvincible) ? fTrueConfidence : fNotInvincible;
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
            float fTrueConfidence2 = fClosing * 0.2f + (1.0f - fFar) * 0.2f + fDownfield * 0.3f + fMarking * 0.3f;
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
