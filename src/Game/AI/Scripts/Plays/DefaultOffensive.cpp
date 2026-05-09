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
void Fuzzy::DefaultOffensivePlay(cDecisionEntity*)
{
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
 */
FuzzyVariant Fuzzy::InGoodWindupPosition(cFielder* TheFielder)
{
    FORCE_DONT_INLINE;
    FuzzyVariant bestValue;
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
void Fuzzy::UsePowerupOffensive(float, cDecisionEntity*)
{
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
