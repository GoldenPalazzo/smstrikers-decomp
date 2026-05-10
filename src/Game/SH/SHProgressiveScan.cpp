#include "Game/SH/SHProgressiveScan.h"

#include "Game/FE/feInput.h"
#include "Game/FE/feTemplates.h"
#include "Game/GameSceneManager.h"
#include "NL/gl/glPlat.h"
#include "dolphin/os/OSRtc.h"

// /**
//  * Offset/Address/Size: 0x3DC | 0x80111B3C | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x358 | 0x80111AB8 | size: 0x84
 */
template <>
template <>
TLComponentInstance* FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLComponentInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x2D4 | 0x80111A34 | size: 0x84
 */
template <>
template <>
TLComponentInstance* FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(
    FEPresentation* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLSlide>(pTopLevel->m_slides, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLComponentInstance*)pChild;
    return _Find<TLSlide>(CastToSomeType<TLSlide>(pTopLevel->m_slides, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

// /**
//  * Offset/Address/Size: 0x29C | 0x801119FC | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                             InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x801118A0 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0xBC | 0x8011181C | size: 0x84
 */
template <>
template <>
TLImageInstance* FEFinder<TLImageInstance, 2>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLImageInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x38 | 0x80111798 | size: 0x84
 */
#pragma inline_depth(0)
template <>
template <>
TLImageInstance* FEFinder<TLImageInstance, 2>::_Find<FEPresentation>(
    FEPresentation* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLSlide>(pTopLevel->m_slides, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLImageInstance*)pChild;
    return _Find<TLSlide>(CastToSomeType<TLSlide>(pTopLevel->m_slides, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}
#pragma inline_depth()

/**
 * Offset/Address/Size: 0x1148 | 0x80111618 | size: 0x148
 */
ProgressiveScanScene::ProgressiveScanScene(bool doRGB60Instead)
{
    const char* MESSAGES_FILE_NAME = "art/fe/HealthSafetyUI.res";

    mConfirmationImage = NULL;
    mHasChoiceBeenMade = false;
    mUseProgressiveMode = true;
    mCanProceed = false;
    mFadingOut = false;
    mDoRGB60Instead = doRGB60Instead;
    mSelectorComponent = NULL;
    mElapsedTime = 0.0f;

    AsyncImage* useProgressiveImage0 = new (nlMalloc(0x1C, 0x20, 1)) AsyncImage(MESSAGES_FILE_NAME, NULL);
    mUseProgressiveImage[0] = useProgressiveImage0;

    AsyncImage* useProgressiveImage1 = new (nlMalloc(0x1C, 0x20, 1)) AsyncImage(MESSAGES_FILE_NAME, NULL);
    mUseProgressiveImage[1] = useProgressiveImage1;

    AsyncImage* confirmationImage = new (nlMalloc(0x1C, 0x20, 1)) AsyncImage(MESSAGES_FILE_NAME, NULL);
    mConfirmationImage = confirmationImage;

    if (mDoRGB60Instead)
    {
        mUseProgressiveMode = OSGetEuRgb60Mode() != 0;
    }
    else
    {
        mUseProgressiveMode = true;
    }
}

/**
 * Offset/Address/Size: 0x107C | 0x8011154C | size: 0xCC
 */
#pragma inline_depth(8)
ProgressiveScanScene::~ProgressiveScanScene()
{
    delete mUseProgressiveImage[0];
    delete mUseProgressiveImage[1];
    delete mConfirmationImage;
}
#pragma inline_depth()

extern unsigned long nlStringLowerHash(const char*);
extern int nlSNPrintf(char*, unsigned long, const char*, ...);
extern int g_Language;

/**
 * Offset/Address/Size: 0x29C | 0x801119FC | size: 0x38
 */
template <>
template <>
TLComponentInstance* FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(
    FEPresentation* slide, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find<FEPresentation>(slide, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

/**
 * Offset/Address/Size: 0x0 | 0x80111760 | size: 0x38
 */
template <>
template <>
TLImageInstance* FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
    FEPresentation* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

/**
 * Offset/Address/Size: 0xD08 | 0x801111D8 | size: 0x374
 * TODO: 89.03% match - all instructions correct, stack frame 0x20 too large
 * due to MWCC not reusing InlineHasher parameter copy slots across Find calls.
 */
void ProgressiveScanScene::SceneCreated()
{
    FEPresentation* presentation;

    presentation = m_pFEPresentation;

    TLComponentInstance* (*findComponent)(
        FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

    mSelectorComponent = findComponent(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("highlite")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mSelectorComponent->m_bVisible = false;

    if (mUseProgressiveMode)
    {
        mSelectorComponent->SetActiveSlide("Slide1");
    }
    else
    {
        mSelectorComponent->SetActiveSlide("Slide2");
    }

    mSelectorComponent->Update(0.0f);

    TLImageInstance* (*findImage)(
        FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = FEFinder<TLImageInstance, 2>::Find<FEPresentation>;

    TLImageInstance* img = findImage(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    mUseProgressiveImage[0]->mImageInstance = img;
    img->m_bVisible = false;

    mUseProgressiveImage[1]->mImageInstance = findImage(
        presentation,
        InlineHasher(nlStringLowerHash("Slide3")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mConfirmationImage->mImageInstance = findImage(
        presentation,
        InlineHasher(nlStringLowerHash("Slide2")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    char texturePath[0x40] = "ProgressiveScan_deu";
    const char* language;

    switch (g_Language)
    {
    case 0:
        language = "eng";
        break;
    case 1:
        language = "fre";
        break;
    case 2:
        language = "deu";
        break;
    case 3:
        language = "spa";
        break;
    case 4:
        language = "ita";
        break;
    case 5:
        language = "jpn";
        break;
    case 6:
        language = "uke";
        break;
    default:
        language = "eng";
        break;
    }

    nlSNPrintf(texturePath, sizeof(texturePath), "ProgressiveScan_%s", language);

    mUseProgressiveImage[0]->QueueLoad(texturePath, true);
    mUseProgressiveImage[1]->QueueLoad(texturePath, true);
}

/**
 * Offset/Address/Size: 0x940 | 0x80110E10 | size: 0x3C8
 * TODO: 98.31% match - bool negation uses cntlzw/srwi (2 instr) instead of
 * subfic/addi/or/srwi (4 instr) due to -inline deferred vs -inline auto flag difference.
 */
void ProgressiveScanScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    if (mUseProgressiveImage[0]->Update(true))
    {
        TLImageInstance* img = FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
            m_pFEPresentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("ProgressiveScan_deu")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        img->m_bVisible = true;
    }
    mUseProgressiveImage[1]->Update(true);
    mConfirmationImage->Update(true);
    if (mHasChoiceBeenMade && mFadingOut)
    {
        TLSlide* slide = m_pFEPresentation->m_currentSlide;
        if (slide->m_time < slide->m_start + slide->m_duration)
        {
            return;
        }
        mFadingOut = false;
        if (mUseProgressiveMode)
        {
            if (!mDoRGB60Instead)
            {
                OSSetProgressiveMode(1);
                glx_SetProgressiveMode();
            }
            else
            {
                OSSetEuRgb60Mode(1);
                glx_SetRGB60Mode();
            }
        }
        else
        {
            if (!mDoRGB60Instead)
            {
                OSSetProgressiveMode(0);
                glx_SetInterlacedMode();
            }
            else
            {
                OSSetEuRgb60Mode(0);
            }
            SwitchMessageImage();
            mCanProceed = true;
            mElapsedTime = 0.0f;
        }
    }
    if (!mHasChoiceBeenMade)
    {
        if ((mElapsedTime += fDeltaT) >= 1.0f)
        {
            mSelectorComponent->m_bVisible = true;
            if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 14, true, NULL) || g_pFEInput->IsAutoPressed(FE_ALL_PADS, 13, true, NULL))
            {
                mElapsedTime = 1.0f;
                mUseProgressiveMode = (mUseProgressiveMode != 1);
                if (mUseProgressiveMode)
                {
                    mSelectorComponent->SetActiveSlide("Slide1");
                    mSelectorComponent->Update(0.0f);
                }
                else
                {
                    mSelectorComponent->SetActiveSlide("Slide2");
                    mSelectorComponent->Update(0.0f);
                }
            }
            else
            {
                if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL) || mElapsedTime >= 10.0f)
                {
                    mHasChoiceBeenMade = true;
                    mFadingOut = true;
                    mElapsedTime = 0.0f;
                    mSelectorComponent->m_bVisible = false;
                    FEPresentation* pres = m_pFEPresentation;
                    pres->SetActiveSlide("Slide3");
                    pres->Update(0.0f);
                    return;
                }
            }
        }
    }
    if (!mCanProceed && mHasChoiceBeenMade && mUseProgressiveMode)
    {
        if ((mElapsedTime += fDeltaT) >= 2.0f)
        {
            SwitchMessageImage();
            mCanProceed = true;
            mElapsedTime = 0.0f;
        }
    }
    if (mCanProceed)
    {
        mElapsedTime += fDeltaT;
        if (mElapsedTime >= 5.0f || g_pFEInput->JustReleased(FE_ALL_PADS, 0x100, false, NULL) || g_pFEInput->JustReleased(FE_ALL_PADS, 0x200, false, NULL))
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_HEALTH_WARNING, SCREEN_NOTHING, true);
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801104D0 | size: 0x940
 */
void ProgressiveScanScene::SwitchMessageImage()
{
    const char* confirmationText = "No";
    if (mUseProgressiveMode)
    {
        confirmationText = "Yes";
    }

    BasicString<char, Detail::TempStringAllocator> languageString;
    FEPresentation* presentation = m_pFEPresentation;

    presentation->SetActiveSlide("Slide1");
    presentation->Update(0.0f);

    switch (g_Language)
    {
    case 0:
        languageString = "eng";
        break;
    case 1:
        languageString = "fre";
        break;
    case 2:
        languageString = "deu";
        break;
    case 3:
        languageString = "spa";
        break;
    case 4:
        languageString = "ita";
        break;
    case 5:
        languageString = "jpn";
        break;
    case 6:
        languageString = "uke";
        break;
    default:
        languageString = "eng";
        break;
    }

    char textureName[64];
    nlSNPrintf(textureName, sizeof(textureName), "fe/health_and_safety/Progressive%s_%s", confirmationText, languageString.c_str());
    textureName[sizeof(textureName) - 1] = '\0';

    mConfirmationImage->QueueLoad(textureName, true);
}
