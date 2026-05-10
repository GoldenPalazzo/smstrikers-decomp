#include "Game/AI/Scripts/FormationScript.h"
#include "Game/AI/Scripts/ScriptQuestions.h"

static int sDefFormation;
static int sOffFormation;

class SaveConfidence
{
public:
    SaveConfidence(float* pFloat)
        : m_fSavedVal(*pFloat)
        , m_pVal(pFloat)
    {
    }

    ~SaveConfidence()
    {
        *m_pVal = m_fSavedVal;
    }

    float m_fSavedVal;
    float* m_pVal;
};

/**
 * Offset/Address/Size: 0x1604 | 0x8007E640 | size: 0x320
 * TODO: 89.89% match - template ctor scheduling: mType/mData stores after ExtraData.Reset()
 *       instead of before, likely -inline deferred scheduling difference
 */
FuzzyVariant Fuzzy::GetBestDefensiveFormation(cTeam* TheTeam)
{
    FuzzyVariant bestValue;

    FuzzyVariant teamVar1(TheTeam);
    Variant* pv1 = &teamVar1;
    pv1->GetHash();

    FuzzyVariant teamVar2(TheTeam);

    int formId = sDefFormation;
    FuzzyVariant formResult(formId);

    bestValue = formResult;
    bestValue.Confidence = 1.0f;

    return bestValue;
}

/**
 * Offset/Address/Size: 0x12E4 | 0x8007E320 | size: 0x320
 * TODO: 89.89% match - template ctor scheduling: mType/mData stores after ExtraData.Reset()
 *       instead of before, likely -inline deferred scheduling difference
 */
FuzzyVariant Fuzzy::GetBestOffensiveFormation(cTeam* TheTeam)
{
    FuzzyVariant bestValue;

    FuzzyVariant teamVar1(TheTeam);
    Variant* pv1 = &teamVar1;
    pv1->GetHash();

    FuzzyVariant teamVar2(TheTeam);

    int formId = sOffFormation;
    FuzzyVariant formResult(formId);

    bestValue = formResult;
    bestValue.Confidence = 1.0f;

    return bestValue;
}

/**
 * Offset/Address/Size: 0x0 | 0x8007D03C | size: 0x12E4
 * TODO: 95.18% match - remaining stack/register allocation differences in
 *       nested confidence scopes and temporary FuzzyVariant copies.
 */
FuzzyVariant Fuzzy::GetBestBallFormationSet(cTeam* TheTeam)
{
    FuzzyVariant bestValue;
    float fConfidence = 1.0f;
    float fBestConfidence = 0.0f;

    FuzzyVariant teamVar1(TheTeam);
    Variant* pv1 = &teamVar1;
    pv1->GetHash();

    FuzzyVariant teamVar2(TheTeam);

    float fTrueConfidence = Losing(TheTeam);
    float fFalseConfidence = 1.0f - fTrueConfidence;
    float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    float fBranchRatio = fMin / fMax;
    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = fConfidence * fBranchRatio;
        }

        {
            float fTrueConfidence = TimeCloseToOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(3);
                }
            }
        }

        {
            float fTrueConfidence = TimeNearlyOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(3);
                }
            }
        }

        {
            float fTrueConfidence = TimeFarFromOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }
    }

    fTrueConfidence = Tied(TheTeam);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;
    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = fConfidence * fBranchRatio;
        }

        {
            float fTrueConfidence = TimeCloseToOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(3);
                }
            }
        }

        {
            float fTrueConfidence = TimeNearlyOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }

        {
            float fTrueConfidence = TimeFarFromOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }
    }

    fTrueConfidence = Winning(TheTeam);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;
    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = fConfidence * fBranchRatio;
        }

        {
            float fTrueConfidence = TimeCloseToOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }

        {
            float fTrueConfidence = TimeNearlyOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }

        {
            float fTrueConfidence = TimeFarFromOver(g_pGame);
            float fFalseConfidence = 1.0f - fTrueConfidence;
            float fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
            float fBranchRatio = fMin / fMax;
            if (fTrueConfidence > 0.0f)
            {
                SaveConfidence PushDOM(&fConfidence);
                fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
                if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
                {
                    fConfidence = fConfidence * fBranchRatio;
                }

                if (fConfidence > fBestConfidence)
                {
                    fBestConfidence = fConfidence;
                    bestValue = FuzzyVariant(2);
                }
            }
        }
    }

    fTrueConfidence = (0.0f == fBestConfidence);
    fFalseConfidence = 1.0f - fTrueConfidence;
    fMin = (fTrueConfidence <= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fMax = (fTrueConfidence >= fFalseConfidence) ? fTrueConfidence : fFalseConfidence;
    fBranchRatio = fMin / fMax;
    if (fTrueConfidence > 0.0f)
    {
        SaveConfidence PushDOM(&fConfidence);
        fConfidence = (fConfidence <= fTrueConfidence) ? fConfidence : fTrueConfidence;
        if (fConfidence < fTrueConfidence && fTrueConfidence < 0.5f)
        {
            fConfidence = fConfidence * fBranchRatio;
        }

        if (fConfidence > fBestConfidence)
        {
            fBestConfidence = fConfidence;
            bestValue = FuzzyVariant(2);
        }
    }

    bestValue.Confidence = fBestConfidence;
    return bestValue;
}
