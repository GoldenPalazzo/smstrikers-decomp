#include "Game/AI/Scripts/Plays/DefaultLoose.h"
#include "Game/AI/ScriptAction.h"
#include "Game/AI/Scripts/ScriptQuestions.h"
#include "Game/GameTweaks.h"

#include "Game/AI/Scripts/SaveConfidence.h"

extern cFielder* g_pScriptCurrentFielder;
extern cFielder* g_pScriptCurrentMark;
extern cTeam* g_pScriptCurrentTeam;
extern cTeam* g_pScriptOtherTeam;
extern cTeam* g_pCurrentlyUpdatingTeam;
extern cBall* g_pScriptBall;
extern FuzzyVariant fvNotSet;

extern float Loose(cTeam*);

/**
 * Offset/Address/Size: 0x15CC | 0x8008C650 | size: 0x43C
 */
FuzzyVariant Fuzzy::AbortLoosePlay(cDecisionEntity*)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = Loose(g_pScriptCurrentTeam);
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
 * Offset/Address/Size: 0x0 | 0x8008B084 | size: 0x15CC
 * TODO: 91.36% match - remaining diffs include register allocation in
 * confidence min/max branches and goalie/interceptor control-flow shaping.
 */
FuzzyVariant Fuzzy::DefaultLoosePlay(cDecisionEntity* pDecision)
{
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = ChasingBall((cPlayer*)g_pScriptCurrentFielder);
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

        fTrueConfidence = GonnaGetBall(g_pScriptCurrentTeam);
        fFalseConfidence = 1.0f - fTrueConfidence;
        float fNotFalse = 1.0f - fFalseConfidence;
        fMin = (fFalseConfidence <= fNotFalse) ? fFalseConfidence : fNotFalse;
        fMax = (fFalseConfidence >= fNotFalse) ? fFalseConfidence : fNotFalse;
        fBranchRatio = fMin / fMax;

        if (fFalseConfidence > 0.0f)
        {
            SaveConfidence PushDOM2(&fConfidence);
            fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
            if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            for (int i = 0; i < 4; i++)
            {
                cFielder* theOpponent = g_pScriptOtherTeam->GetFielder(i);

                float fNotRepeating = 1.0f - RepeatingLastDesire(g_pScriptCurrentFielder, edHeavyAttack);
                float fChasing = ChasingBall((cPlayer*)theOpponent);
                float fIdealTackle = AtIdealDistanceForTackling((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)theOpponent);
                float fNotChasing;

                fChasing = (fChasing <= fNotRepeating) ? fChasing : fNotRepeating;
                fChasing = (fChasing <= fIdealTackle) ? fChasing : fIdealTackle;
                fNotChasing = 1.0f - fChasing;

                float fNotNotChasing = 1.0f - fNotChasing;
                float fMin2 = (fNotChasing <= fNotNotChasing) ? fNotChasing : fNotNotChasing;
                float fMax2 = (fNotChasing >= fNotNotChasing) ? fNotChasing : fNotNotChasing;
                float fBranchRatio2 = fMin2 / fMax2;

                if (fNotChasing > 0.0f)
                {
                    SaveConfidence PushDOM3(&fConfidence);
                    fConfidence = (fConfidence <= fNotChasing) ? fConfidence : fNotChasing;
                    if (fConfidence < fNotChasing && fNotChasing < 0.5f)
                        fConfidence = fConfidence * fBranchRatio2;

                    if (fBestConfidence < fConfidence)
                        fBestConfidence = fConfidence;

                    pDecision->QueueActionSetDesire(5, fConfidence, 0.0f, FuzzyVariant((cPlayer*)theOpponent), fvNotSet);
                    SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
                    pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Loose_HeavyAttackChance, Aggressive(g_pScriptCurrentFielder));
                }
            }
        }
    }

    FuzzyVariant bestBallInterceptor = GetBestBallInterceptor(g_pScriptCurrentTeam);
    fTrueConfidence = (bestBallInterceptor.mData.u == (unsigned long)g_pScriptCurrentFielder) ? 1.0f : 0.0f;
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

        cTeam* pCurrentTeam = g_pScriptCurrentFielder ? g_pScriptCurrentFielder->m_pTeam : (cTeam*)0;
        cTeam* pOtherTeam = pCurrentTeam ? pCurrentTeam->GetOtherTeam() : (cTeam*)0;
        float fGoaliePickup = GoalieAndGonnaPickupBall((cPlayer*)(pCurrentTeam ? pCurrentTeam->GetGoalie() : (Goalie*)0)).Confidence;
        float fOtherGoaliePickup = GoalieAndGonnaPickupBall((cPlayer*)(pOtherTeam ? pOtherTeam->GetGoalie() : (Goalie*)0)).Confidence;
        float fNotOtherGoaliePickup = 1.0f - fOtherGoaliePickup;
        fFalseConfidence = 1.0f - fGoaliePickup;

        fFalseConfidence = (fFalseConfidence <= fNotOtherGoaliePickup) ? fFalseConfidence : fNotOtherGoaliePickup;
        fTrueConfidence = fFalseConfidence;
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

            if (fBestConfidence < fConfidence)
                fBestConfidence = fConfidence;

            pDecision->QueueActionSetDesire(6, fConfidence, -1.0f, fvNotSet, fvNotSet);
        }

        if (fFalseConfidence > 0.0f)
        {
            SaveConfidence PushDOM5(&fConfidence);
            fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
            if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
                fConfidence = fConfidence * fBranchRatio;

            if (fBestConfidence < fConfidence)
                fBestConfidence = fConfidence;

            pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM5(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        float fWinger = Winger(g_pScriptCurrentFielder);
        float fNotWinger = 1.0f - fWinger;
        float fMin3 = (fWinger <= fNotWinger) ? fWinger : fNotWinger;
        float fMax3 = (fWinger >= fNotWinger) ? fWinger : fNotWinger;
        float fBranchRatio3 = fMin3 / fMax3;

        if (fWinger > 0.0f)
        {
            SaveConfidence PushDOM6(&fConfidence);
            fConfidence = (fConfidence <= fWinger) ? fConfidence : fWinger;
            if (fConfidence < fWinger && fWinger < 0.5f)
                fConfidence = fConfidence * fBranchRatio3;

            float fNotCanGetBall = 1.0f - GonnaGetBall(g_pScriptCurrentTeam);
            float fNotInOffZone = 1.0f - InOffensiveZoneOfPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);
            fNotCanGetBall = (fNotCanGetBall <= fNotInOffZone) ? fNotCanGetBall : fNotInOffZone;
            float fInDefZone = InDefensiveZoneOfPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);
            fNotCanGetBall = (fNotCanGetBall >= fInDefZone) ? fNotCanGetBall : fInDefZone;
            float fCanGetBall = 1.0f - fNotCanGetBall;
            float fCannotGetBall = fNotCanGetBall;
            float fMin4 = (fCanGetBall <= fCannotGetBall) ? fCanGetBall : fCannotGetBall;
            float fMax4 = (fCanGetBall >= fCannotGetBall) ? fCanGetBall : fCannotGetBall;
            float fBranchRatio4 = fMin4 / fMax4;

            if (fCanGetBall > 0.0f)
            {
                SaveConfidence PushDOM7(&fConfidence);
                fConfidence = (fConfidence <= fCanGetBall) ? fConfidence : fCanGetBall;
                if (fConfidence < fCanGetBall && fCanGetBall < 0.5f)
                    fConfidence = fConfidence * fBranchRatio4;

                if (fBestConfidence < fConfidence)
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(7, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fCannotGetBall > 0.0f)
            {
                SaveConfidence PushDOM8(&fConfidence);
                fConfidence = (fConfidence <= fCannotGetBall) ? fConfidence : fCannotGetBall;
                if (fConfidence < fCannotGetBall && fCannotGetBall < 0.5f)
                    fConfidence = fConfidence * fBranchRatio4;

                float fInDefensive = InDefensiveZone((cPlayer*)g_pScriptCurrentFielder);
                float fNotDef = 1.0f - fInDefensive;
                float fMin5 = (fInDefensive <= fNotDef) ? fInDefensive : fNotDef;
                float fMax5 = (fInDefensive >= fNotDef) ? fInDefensive : fNotDef;
                float fBranchRatio5 = fMin5 / fMax5;

                if (fInDefensive > 0.0f)
                {
                    SaveConfidence PushDOM9(&fConfidence);
                    fConfidence = (fConfidence <= fInDefensive) ? fConfidence : fInDefensive;
                    if (fConfidence < fInDefensive && fInDefensive < 0.5f)
                        fConfidence = fConfidence * fBranchRatio5;
                    if (fBestConfidence < fConfidence)
                        fBestConfidence = fConfidence;
                    pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fNotDef > 0.0f)
                {
                    SaveConfidence PushDOM10(&fConfidence);
                    fConfidence = (fConfidence <= fNotDef) ? fConfidence : fNotDef;
                    if (fConfidence < fNotDef && fNotDef < 0.5f)
                        fConfidence = fConfidence * fBranchRatio5;

                    float fInOffensive = InOffensiveZone((cPlayer*)g_pScriptCurrentFielder);
                    float fNotOffensive = 1.0f - fInOffensive;
                    float fMin6 = (fInOffensive <= fNotOffensive) ? fInOffensive : fNotOffensive;
                    float fMax6 = (fInOffensive >= fNotOffensive) ? fInOffensive : fNotOffensive;
                    float fBranchRatio6 = fMin6 / fMax6;

                    if (fInOffensive > 0.0f)
                    {
                        SaveConfidence PushDOM11(&fConfidence);
                        fConfidence = (fConfidence <= fInOffensive) ? fConfidence : fInOffensive;
                        if (fConfidence < fInOffensive && fInOffensive < 0.5f)
                            fConfidence = fConfidence * fBranchRatio6;
                        if (fBestConfidence < fConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(1, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }

                    if (fNotOffensive > 0.0f)
                    {
                        SaveConfidence PushDOM12(&fConfidence);
                        fConfidence = (fConfidence <= fNotOffensive) ? fConfidence : fNotOffensive;
                        if (fConfidence < fNotOffensive && fNotOffensive < 0.5f)
                            fConfidence = fConfidence * fBranchRatio6;
                        if (fBestConfidence < fConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }
                }
            }
        }

        float fRoleMid = Midfield(g_pScriptCurrentFielder);
        float fRoleDef = Defence(g_pScriptCurrentFielder);
        float fRole = (fRoleMid >= fRoleDef) ? fRoleMid : fRoleDef;

        if (fNotWinger > 0.0f)
        {
            SaveConfidence PushDOM13(&fConfidence);
            fConfidence = (fConfidence <= fNotWinger) ? fConfidence : fNotWinger;
            if (fConfidence < fNotWinger && fNotWinger < 0.5f)
                fConfidence = fConfidence * fBranchRatio3;

            float fCanGetBall2 = GonnaGetBall(g_pScriptCurrentTeam);
            float fNotInOffZone2 = 1.0f - InOffensiveZoneOfPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);
            float fNotInDefZone2 = 1.0f - InDefensiveZoneOfPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);

            fRole = (fRole <= fCanGetBall2) ? fRole : fCanGetBall2;
            fRole = (fRole <= fNotInOffZone2) ? fRole : fNotInOffZone2;
            fRole = (fRole <= fNotInDefZone2) ? fRole : fNotInDefZone2;

            float fNotRole = 1.0f - fRole;
            float fMin7 = (fRole <= fNotRole) ? fRole : fNotRole;
            float fMax7 = (fRole >= fNotRole) ? fRole : fNotRole;
            float fBranchRatio7 = fMin7 / fMax7;

            if (fRole > 0.0f)
            {
                SaveConfidence PushDOM14(&fConfidence);
                fConfidence = (fConfidence <= fRole) ? fConfidence : fRole;
                if (fConfidence < fRole && fRole < 0.5f)
                    fConfidence = fConfidence * fBranchRatio7;
                if (fBestConfidence < fConfidence)
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(7, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fNotRole > 0.0f)
            {
                SaveConfidence PushDOM15(&fConfidence);
                fConfidence = (fConfidence <= fNotRole) ? fConfidence : fNotRole;
                if (fConfidence < fNotRole && fNotRole < 0.5f)
                    fConfidence = fConfidence * fBranchRatio7;

                float fNearMyNet = NearToMyNet((cPlayer*)g_pScriptCurrentFielder);
                float fNotNearMyNet = 1.0f - fNearMyNet;
                float fMin8 = (fNearMyNet <= fNotNearMyNet) ? fNearMyNet : fNotNearMyNet;
                float fMax8 = (fNearMyNet >= fNotNearMyNet) ? fNearMyNet : fNotNearMyNet;
                float fBranchRatio8 = fMin8 / fMax8;

                if (fNearMyNet > 0.0f)
                {
                    SaveConfidence PushDOM16(&fConfidence);
                    fConfidence = (fConfidence <= fNearMyNet) ? fConfidence : fNearMyNet;
                    if (fConfidence < fNearMyNet && fNearMyNet < 0.5f)
                        fConfidence = fConfidence * fBranchRatio8;
                    if (fBestConfidence < fConfidence)
                        fBestConfidence = fConfidence;
                    pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fNotNearMyNet > 0.0f)
                {
                    SaveConfidence PushDOM17(&fConfidence);
                    fConfidence = (fConfidence <= fNotNearMyNet) ? fConfidence : fNotNearMyNet;
                    if (fConfidence < fNotNearMyNet && fNotNearMyNet < 0.5f)
                        fConfidence = fConfidence * fBranchRatio8;

                    float fInOffensive2 = InOffensiveZone((cPlayer*)g_pScriptCurrentFielder);
                    float fNotInOffensive2 = 1.0f - fInOffensive2;
                    float fMin9 = (fInOffensive2 <= fNotInOffensive2) ? fInOffensive2 : fNotInOffensive2;
                    float fMax9 = (fInOffensive2 >= fNotInOffensive2) ? fInOffensive2 : fNotInOffensive2;
                    float fBranchRatio9 = fMin9 / fMax9;

                    if (fInOffensive2 > 0.0f)
                    {
                        SaveConfidence PushDOM18(&fConfidence);
                        fConfidence = (fConfidence <= fInOffensive2) ? fConfidence : fInOffensive2;
                        if (fConfidence < fInOffensive2 && fInOffensive2 < 0.5f)
                            fConfidence = fConfidence * fBranchRatio9;
                        if (fBestConfidence < fConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }

                    if (fNotInOffensive2 > 0.0f)
                    {
                        SaveConfidence PushDOM19(&fConfidence);
                        fConfidence = (fConfidence <= fNotInOffensive2) ? fConfidence : fNotInOffensive2;
                        if (fConfidence < fNotInOffensive2 && fNotInOffensive2 < 0.5f)
                            fConfidence = fConfidence * fBranchRatio9;
                        if (fBestConfidence < fConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(3, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }
                }
            }
        }
    }

    float fNearBall = NearToBall((cPlayer*)g_pScriptCurrentMark);
    float fGoalieStunned = Stunned(g_pScriptCurrentTeam->GetGoalie());
    fNearBall = (fGoalieStunned <= fNearBall) ? fGoalieStunned : fNearBall;

    float fNotNearBall = 1.0f - fNearBall;
    float fMin10 = (fNearBall <= fNotNearBall) ? fNearBall : fNotNearBall;
    float fMax10 = (fNearBall >= fNotNearBall) ? fNearBall : fNotNearBall;
    float fBranchRatio10 = fMin10 / fMax10;

    if (fNearBall > 0.0f)
    {
        SaveConfidence PushDOM20(&fConfidence);
        fConfidence = (fConfidence <= fNearBall) ? fConfidence : fNearBall;
        if (fConfidence < fNearBall && fNearBall < 0.5f)
            fConfidence = fConfidence * fBranchRatio10;

        float fIdealTackle = AtIdealDistanceForTackling((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)g_pScriptCurrentMark);
        float fNotIdealTackle = 1.0f - fIdealTackle;
        float fMin11 = (fIdealTackle <= fNotIdealTackle) ? fIdealTackle : fNotIdealTackle;
        float fMax11 = (fIdealTackle >= fNotIdealTackle) ? fIdealTackle : fNotIdealTackle;
        float fBranchRatio11 = fMin11 / fMax11;

        if (fIdealTackle > 0.0f)
        {
            SaveConfidence PushDOM21(&fConfidence);
            fConfidence = (fConfidence <= fIdealTackle) ? fConfidence : fIdealTackle;
            if (fConfidence < fIdealTackle && fIdealTackle < 0.5f)
                fConfidence = fConfidence * fBranchRatio11;

            if (fBestConfidence < fConfidence)
                fBestConfidence = fConfidence;

            pDecision->QueueActionSetDesire(5, fConfidence, 0.0f, FuzzyVariant((cPlayer*)g_pScriptCurrentMark), fvNotSet);
            SkillTweaks* pTweaks = SkillTweaks::GetSkillTweaks(g_pCurrentlyUpdatingTeam->m_nSide);
            pDecision->m_pLastQueuedAction->m_fSelectionChance = CalcSelectChance(pTweaks->Loose_HeavyAttackChance, Aggressive(g_pScriptCurrentFielder));
        }
    }

    fNearBall = 1.0f - fBestConfidence;
    fNotNearBall = 1.0f - fNearBall;
    fMin10 = (fNearBall <= fNotNearBall) ? fNearBall : fNotNearBall;
    fMax10 = (fNearBall >= fNotNearBall) ? fNearBall : fNotNearBall;
    fBranchRatio10 = fMin10 / fMax10;

    if (fNearBall > 0.0f)
    {
        SaveConfidence PushDOM22(&fConfidence);
        fConfidence = (fConfidence <= fNearBall) ? fConfidence : fNearBall;
        if (fConfidence < fNearBall && fNearBall < 0.5f)
            fConfidence = fConfidence * fBranchRatio10;
        if (fBestConfidence <= fConfidence)
            fBestConfidence = fConfidence;
        pDecision->QueueActionSetDesire(11, fConfidence, -1.0f, fvNotSet, fvNotSet);
    }

    return FuzzyVariant(fBestConfidence);
}
