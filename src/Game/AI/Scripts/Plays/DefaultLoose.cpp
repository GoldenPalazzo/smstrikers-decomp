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
 */
FuzzyVariant Fuzzy::DefaultLoosePlay(cDecisionEntity*)
{
}
