#include "Game/AI/Scripts/Plays/DefaultOffensive.h"

#include "Game/AI/Scripts/ScriptQuestions.h"

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

// /**
//  * Offset/Address/Size: 0x0 | 0x80093A64 | size: 0x8
//  */
// void PositionOf<cFielder*>(cFielder*)
// {
// }

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
void Fuzzy::DoPassing(float, cDecisionEntity*)
{
}

/**
 * Offset/Address/Size: 0x4490 | 0x80090F1C | size: 0x604
 */
void Fuzzy::GoodBallCarrier(cFielder*)
{
}

/**
 * Offset/Address/Size: 0x3128 | 0x8008FBB4 | size: 0x1368
 */
void Fuzzy::InGoodWindupPosition(cFielder*)
{
}

/**
 * Offset/Address/Size: 0x2B2C | 0x8008F5B8 | size: 0x5FC
 */
void Fuzzy::CutAndBreak(cFielder*)
{
}

/**
 * Offset/Address/Size: 0x22A0 | 0x8008ED2C | size: 0x88C
 */
void Fuzzy::DoShooting(float, cDecisionEntity*)
{
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
void Fuzzy::GetPowerupTargetOffensive(cTeam*)
{
}
