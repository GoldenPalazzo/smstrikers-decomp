#include "Game/AI/Scripts/Plays/DefaultLoose.h"

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
static bool sTrue = true;

/**
 * Offset/Address/Size: 0x15CC | 0x8008C650 | size: 0x43C
 * TODO: 99.70% match - r29/r30 swap in bool-to-FuzzyVariant branch temporaries.
 */
FuzzyVariant Fuzzy::AbortLoosePlay(cDecisionEntity*)
{
    extern cTeam* g_pScriptCurrentTeam;
    extern float Loose(cTeam*);

    FuzzyVariant bestValue;
    bool bResult;
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
    return bestValue;
}

/**
 * Offset/Address/Size: 0x0 | 0x8008B084 | size: 0x15CC
 * TODO: 71.62% match - remaining diffs include ABI/stack-frame and control-flow
 * divergence against target assembly.
 */
FuzzyVariant Fuzzy::DefaultLoosePlay(cDecisionEntity* pDecision)
{
    class cPlayer;
    class cFielder;
    class cBall;
    class Goalie;

    extern cFielder* g_pScriptCurrentFielder;
    extern cFielder* g_pScriptCurrentMark;
    extern cTeam* g_pScriptCurrentTeam;
    extern cTeam* g_pScriptOtherTeam;
    extern cBall* g_pScriptBall;
    extern FuzzyVariant fvNotSet;

    extern float ChasingBall__FP7cPlayer(cPlayer*);
    extern float GonnaGetBall__FP5cTeam(cTeam*);
    extern float RepeatingLastDesire__FP8cFielder20eScriptFielderDesire(cFielder*, int);
    extern float AtIdealDistanceForTackling__FP7cPlayerP7cPlayer(cPlayer*, cPlayer*);
    extern float Winger__FP8cFielder(cFielder*);
    extern float InOffensiveZoneOfPlayer__FP5cBallP7cPlayer(cBall*, cPlayer*);
    extern float InDefensiveZoneOfPlayer__FP5cBallP7cPlayer(cBall*, cPlayer*);
    extern float InDefensiveZone__FP7cPlayer(cPlayer*);
    extern float InOffensiveZone__FP7cPlayer(cPlayer*);
    extern float Midfield__FP8cFielder(cFielder*);
    extern float Defence__FP8cFielder(cFielder*);
    extern float NearToMyNet__FP7cPlayer(cPlayer*);
    extern float NearToBall__FP7cPlayer(cPlayer*);
    extern float Stunned__FP6Goalie(Goalie*);

    extern cFielder* GetFielder__5cTeamFi(cTeam*, int);
    extern cTeam* GetOtherTeam__5cTeamFv(cTeam*);
    extern Goalie* GetGoalie__5cTeamFv(cTeam*);

    extern FuzzyVariant GetBestBallInterceptor__5FuzzyFP5cTeam(cTeam*);
    extern FuzzyVariant GoalieAndGonnaPickupBall__5FuzzyFP7cPlayer(cPlayer*);

    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    float fTrueConfidence = ChasingBall__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
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

        fTrueConfidence = GonnaGetBall__FP5cTeam(g_pScriptCurrentTeam);
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

            for (int i = 0; i < 3; i++)
            {
                cFielder* theOpponent = GetFielder__5cTeamFi(g_pScriptOtherTeam, i);

                float fNotRepeating = 1.0f - RepeatingLastDesire__FP8cFielder20eScriptFielderDesire(g_pScriptCurrentFielder, 5);
                float fNotChasing = 1.0f - ChasingBall__FP7cPlayer((cPlayer*)theOpponent);
                float fIdealTackle = AtIdealDistanceForTackling__FP7cPlayerP7cPlayer((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)theOpponent);

                fNotChasing = (fNotChasing <= fIdealTackle) ? fNotChasing : fIdealTackle;
                fNotChasing = (fNotChasing <= fNotRepeating) ? fNotChasing : fNotRepeating;

                if (fNotChasing > fBestConfidence)
                {
                    fBestConfidence = fNotChasing;
                    g_pScriptCurrentMark = theOpponent;
                }
            }

            if (fBestConfidence > 0.0f && g_pScriptCurrentMark)
                pDecision->QueueActionSetDesire(5, fConfidence, 0.0f, FuzzyVariant((cPlayer*)g_pScriptCurrentMark), fvNotSet);

            FuzzyVariant bestBallInterceptor = GetBestBallInterceptor__5FuzzyFP5cTeam(g_pScriptCurrentTeam);

            cTeam* pOtherTeam = GetOtherTeam__5cTeamFv(g_pScriptCurrentTeam);
            float fOtherGoaliePickup = GoalieAndGonnaPickupBall__5FuzzyFP7cPlayer((cPlayer*)GetGoalie__5cTeamFv(pOtherTeam)).Confidence;
            float fGoaliePickup = GoalieAndGonnaPickupBall__5FuzzyFP7cPlayer((cPlayer*)GetGoalie__5cTeamFv(g_pScriptCurrentTeam)).Confidence;

            float fIntercept = bestBallInterceptor.Confidence;
            float fNotOtherGoaliePickup = 1.0f - fOtherGoaliePickup;
            float fNotGoaliePickup = 1.0f - fGoaliePickup;

            fIntercept = (fIntercept <= fNotOtherGoaliePickup) ? fIntercept : fNotOtherGoaliePickup;
            fIntercept = (fIntercept <= fNotGoaliePickup) ? fIntercept : fNotGoaliePickup;

            float fNotIntercept = 1.0f - fIntercept;
            float fMin2 = (fIntercept <= fNotIntercept) ? fIntercept : fNotIntercept;
            float fMax2 = (fIntercept >= fNotIntercept) ? fIntercept : fNotIntercept;
            float fBranchRatio2 = fMin2 / fMax2;

            if (fIntercept > 0.0f)
            {
                SaveConfidence PushDOM3(&fConfidence);
                fConfidence = (fConfidence <= fIntercept) ? fConfidence : fIntercept;
                if (fConfidence < fIntercept && fIntercept < 0.5f)
                    fConfidence = fConfidence * fBranchRatio2;

                if (fConfidence > fBestConfidence)
                    fBestConfidence = fConfidence;

                pDecision->QueueActionSetDesire(6, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fNotIntercept > 0.0f)
            {
                SaveConfidence PushDOM4(&fConfidence);
                fConfidence = (fConfidence <= fNotIntercept) ? fConfidence : fNotIntercept;
                if (fConfidence < fNotIntercept && fNotIntercept < 0.5f)
                    fConfidence = fConfidence * fBranchRatio2;

                if (fConfidence > fBestConfidence)
                    fBestConfidence = fConfidence;

                pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }
        }
    }

    if (fFalseConfidence > 0.0f)
    {
        SaveConfidence PushDOM5(&fConfidence);
        fConfidence = (fConfidence <= fFalseConfidence) ? fConfidence : fFalseConfidence;
        if (fConfidence < fFalseConfidence && fFalseConfidence < 0.5f)
            fConfidence = fConfidence * fBranchRatio;

        float fWinger = Winger__FP8cFielder(g_pScriptCurrentFielder);
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

            float fCanGetBall = GonnaGetBall__FP5cTeam(g_pScriptCurrentTeam);
            float fNotInOffZone = 1.0f - InOffensiveZoneOfPlayer__FP5cBallP7cPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);
            float fNotInDefZone = 1.0f - InDefensiveZoneOfPlayer__FP5cBallP7cPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);

            fCanGetBall = (fCanGetBall <= fNotInOffZone) ? fCanGetBall : fNotInOffZone;
            fCanGetBall = (fCanGetBall <= fNotInDefZone) ? fCanGetBall : fNotInDefZone;

            float fCannotGetBall = 1.0f - fCanGetBall;
            float fMin4 = (fCanGetBall <= fCannotGetBall) ? fCanGetBall : fCannotGetBall;
            float fMax4 = (fCanGetBall >= fCannotGetBall) ? fCanGetBall : fCannotGetBall;
            float fBranchRatio4 = fMin4 / fMax4;

            if (fCanGetBall > 0.0f)
            {
                SaveConfidence PushDOM7(&fConfidence);
                fConfidence = (fConfidence <= fCanGetBall) ? fConfidence : fCanGetBall;
                if (fConfidence < fCanGetBall && fCanGetBall < 0.5f)
                    fConfidence = fConfidence * fBranchRatio4;

                if (fConfidence > fBestConfidence)
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(7, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fCannotGetBall > 0.0f)
            {
                SaveConfidence PushDOM8(&fConfidence);
                fConfidence = (fConfidence <= fCannotGetBall) ? fConfidence : fCannotGetBall;
                if (fConfidence < fCannotGetBall && fCannotGetBall < 0.5f)
                    fConfidence = fConfidence * fBranchRatio4;

                float fInDefensive = InDefensiveZone__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
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
                    if (fConfidence > fBestConfidence)
                        fBestConfidence = fConfidence;
                    pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fNotDef > 0.0f)
                {
                    SaveConfidence PushDOM10(&fConfidence);
                    fConfidence = (fConfidence <= fNotDef) ? fConfidence : fNotDef;
                    if (fConfidence < fNotDef && fNotDef < 0.5f)
                        fConfidence = fConfidence * fBranchRatio5;

                    float fInOffensive = InOffensiveZone__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
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
                        if (fConfidence > fBestConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(1, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }

                    if (fNotOffensive > 0.0f)
                    {
                        SaveConfidence PushDOM12(&fConfidence);
                        fConfidence = (fConfidence <= fNotOffensive) ? fConfidence : fNotOffensive;
                        if (fConfidence < fNotOffensive && fNotOffensive < 0.5f)
                            fConfidence = fConfidence * fBranchRatio6;
                        if (fConfidence > fBestConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }
                }
            }
        }

        if (fNotWinger > 0.0f)
        {
            SaveConfidence PushDOM13(&fConfidence);
            fConfidence = (fConfidence <= fNotWinger) ? fConfidence : fNotWinger;
            if (fConfidence < fNotWinger && fNotWinger < 0.5f)
                fConfidence = fConfidence * fBranchRatio3;

            float fRoleMid = Midfield__FP8cFielder(g_pScriptCurrentFielder);
            float fRoleDef = Defence__FP8cFielder(g_pScriptCurrentFielder);
            float fRole = (fRoleMid >= fRoleDef) ? fRoleMid : fRoleDef;

            float fCanGetBall2 = GonnaGetBall__FP5cTeam(g_pScriptCurrentTeam);
            float fNotInOffZone2 = 1.0f - InOffensiveZoneOfPlayer__FP5cBallP7cPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);
            float fNotInDefZone2 = 1.0f - InDefensiveZoneOfPlayer__FP5cBallP7cPlayer(g_pScriptBall, (cPlayer*)g_pScriptCurrentFielder);

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
                if (fConfidence > fBestConfidence)
                    fBestConfidence = fConfidence;
                pDecision->QueueActionSetDesire(7, fConfidence, -1.0f, fvNotSet, fvNotSet);
            }

            if (fNotRole > 0.0f)
            {
                SaveConfidence PushDOM15(&fConfidence);
                fConfidence = (fConfidence <= fNotRole) ? fConfidence : fNotRole;
                if (fConfidence < fNotRole && fNotRole < 0.5f)
                    fConfidence = fConfidence * fBranchRatio7;

                float fNearMyNet = NearToMyNet__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
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
                    if (fConfidence > fBestConfidence)
                        fBestConfidence = fConfidence;
                    pDecision->QueueActionSetDesire(10, fConfidence, -1.0f, fvNotSet, fvNotSet);
                }

                if (fNotNearMyNet > 0.0f)
                {
                    SaveConfidence PushDOM17(&fConfidence);
                    fConfidence = (fConfidence <= fNotNearMyNet) ? fConfidence : fNotNearMyNet;
                    if (fConfidence < fNotNearMyNet && fNotNearMyNet < 0.5f)
                        fConfidence = fConfidence * fBranchRatio8;

                    float fInOffensive2 = InOffensiveZone__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
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
                        if (fConfidence > fBestConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(4, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }

                    if (fNotInOffensive2 > 0.0f)
                    {
                        SaveConfidence PushDOM19(&fConfidence);
                        fConfidence = (fConfidence <= fNotInOffensive2) ? fConfidence : fNotInOffensive2;
                        if (fConfidence < fNotInOffensive2 && fNotInOffensive2 < 0.5f)
                            fConfidence = fConfidence * fBranchRatio9;
                        if (fConfidence > fBestConfidence)
                            fBestConfidence = fConfidence;
                        pDecision->QueueActionSetDesire(3, fConfidence, -1.0f, fvNotSet, fvNotSet);
                    }
                }
            }
        }
    }

    float fNearBall = NearToBall__FP7cPlayer((cPlayer*)g_pScriptCurrentFielder);
    float fGoalieStunned = Stunned__FP6Goalie(GetGoalie__5cTeamFv(g_pScriptCurrentTeam));
    float fIdealTackle = AtIdealDistanceForTackling__FP7cPlayerP7cPlayer((cPlayer*)g_pScriptCurrentFielder, (cPlayer*)g_pScriptCurrentMark);

    fNearBall = (fNearBall <= (1.0f - fGoalieStunned)) ? fNearBall : (1.0f - fGoalieStunned);
    fNearBall = (fNearBall <= fIdealTackle) ? fNearBall : fIdealTackle;

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
        if (fConfidence > fBestConfidence)
            fBestConfidence = fConfidence;
        if (g_pScriptCurrentMark)
            pDecision->QueueActionSetDesire(5, fConfidence, 0.0f, FuzzyVariant((cPlayer*)g_pScriptCurrentMark), fvNotSet);
    }

    if (fNotNearBall > 0.0f)
    {
        SaveConfidence PushDOM21(&fConfidence);
        fConfidence = (fConfidence <= fNotNearBall) ? fConfidence : fNotNearBall;
        if (fConfidence < fNotNearBall && fNotNearBall < 0.5f)
            fConfidence = fConfidence * fBranchRatio10;
        if (fConfidence > fBestConfidence)
            fBestConfidence = fConfidence;
        pDecision->QueueActionSetDesire(11, fConfidence, -1.0f, fvNotSet, fvNotSet);
    }

    return FuzzyVariant(fBestConfidence);
}
