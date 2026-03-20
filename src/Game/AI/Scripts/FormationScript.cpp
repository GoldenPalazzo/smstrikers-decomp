#include "Game/AI/Scripts/FormationScript.h"

static int sDefFormation;
static int sOffFormation;

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
 */
FuzzyVariant Fuzzy::GetBestBallFormationSet(cTeam*)
{
    FuzzyVariant result;
    return result;
}
