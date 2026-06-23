#include "Game/SH/SHProgressiveScan.h"

#include "Game/FE/feInput.h"
#include "Game/FE/feTemplates.h"
#include "Game/GameSceneManager.h"
#include "NL/gl/glPlat.h"
#include "dolphin/os/OSRtc.h"

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
 * Offset/Address/Size: 0xD08 | 0x801111D8 | size: 0x374
 */
void ProgressiveScanScene::SceneCreated()
{
    typedef TLComponentInstance* (*FindComponentByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindComponentByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindComponentByValue byValue;
        FindComponentByRef byRef;
    } findComponent;
    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    volatile InlineHasher c1NameB, c1NameA;
    volatile InlineHasher c1LayerB, c1LayerA;
    volatile InlineHasher c1SlideB, c1SlideA;
    volatile InlineHasher z5, z4, z3, z2, z1, z0;
    volatile InlineHasher c2NameB, c2NameA;
    volatile InlineHasher c2LayerB, c2LayerA;
    volatile InlineHasher c2SlideB, c2SlideA;
    volatile InlineHasher c2z4, c2z2, c2z0;
    volatile InlineHasher c3NameB, c3NameA;
    volatile InlineHasher c3LayerB, c3LayerA;
    volatile InlineHasher c3SlideB, c3SlideA;
    volatile InlineHasher c3z4, c3z2, c3z0;
    volatile InlineHasher c4NameB, c4NameA;
    volatile InlineHasher c4LayerB, c4LayerA;
    volatile InlineHasher c4SlideB, c4SlideA;
    volatile InlineHasher c4z4, c4z2, c4z0;

    unsigned long hash;
    FEPresentation* presentation = m_pFEPresentation;

    findComponent.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

    z0.m_Hash = 0;
    z2.m_Hash = 0;
    z4.m_Hash = 0;
    z1.m_Hash = 0;
    z3.m_Hash = 0;
    z5.m_Hash = 0;

    hash = nlStringLowerHash("highlite");
    c1SlideA.m_Hash = hash;
    c1SlideB.m_Hash = hash;
    hash = nlStringLowerHash("Layer");
    c1LayerA.m_Hash = hash;
    c1LayerB.m_Hash = hash;
    hash = nlStringLowerHash("Slide1");
    c1NameA.m_Hash = hash;
    c1NameB.m_Hash = hash;

    mSelectorComponent = findComponent.byRef(
        presentation,
        (InlineHasher&)c1NameB,
        (InlineHasher&)c1LayerB,
        (InlineHasher&)c1SlideB,
        (InlineHasher&)z5,
        (InlineHasher&)z3,
        (InlineHasher&)z1);

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

    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<FEPresentation>;

    c2z0.m_Hash = 0;
    z1.m_Hash = 0;
    c2z2.m_Hash = 0;
    z3.m_Hash = 0;
    c2z4.m_Hash = 0;
    z5.m_Hash = 0;

    hash = nlStringLowerHash("ProgressiveScan_deu");
    c2SlideA.m_Hash = hash;
    c2SlideB.m_Hash = hash;
    hash = nlStringLowerHash("Layer");
    c2LayerA.m_Hash = hash;
    c2LayerB.m_Hash = hash;
    hash = nlStringLowerHash("Slide1");
    c2NameA.m_Hash = hash;
    c2NameB.m_Hash = hash;

    TLImageInstance* img = findImage.byRef(
        presentation,
        (InlineHasher&)c2NameB,
        (InlineHasher&)c2LayerB,
        (InlineHasher&)c2SlideB,
        (InlineHasher&)z5,
        (InlineHasher&)z3,
        (InlineHasher&)z1);
    mUseProgressiveImage[0]->mImageInstance = img;
    img->m_bVisible = false;

    c3z0.m_Hash = 0;
    z1.m_Hash = 0;
    c3z2.m_Hash = 0;
    z3.m_Hash = 0;
    c3z4.m_Hash = 0;
    z5.m_Hash = 0;

    hash = nlStringLowerHash("ProgressiveScan_deu");
    c3SlideA.m_Hash = hash;
    c3SlideB.m_Hash = hash;
    hash = nlStringLowerHash("Layer");
    c3LayerA.m_Hash = hash;
    c3LayerB.m_Hash = hash;
    hash = nlStringLowerHash("Slide3");
    c3NameA.m_Hash = hash;
    c3NameB.m_Hash = hash;

    mUseProgressiveImage[1]->mImageInstance = findImage.byRef(
        presentation,
        (InlineHasher&)c3NameB,
        (InlineHasher&)c3LayerB,
        (InlineHasher&)c3SlideB,
        (InlineHasher&)z5,
        (InlineHasher&)z3,
        (InlineHasher&)z1);

    c4z0.m_Hash = 0;
    z1.m_Hash = 0;
    c4z2.m_Hash = 0;
    z3.m_Hash = 0;
    c4z4.m_Hash = 0;
    z5.m_Hash = 0;

    hash = nlStringLowerHash("ProgressiveScan_deu");
    c4SlideA.m_Hash = hash;
    c4SlideB.m_Hash = hash;
    hash = nlStringLowerHash("Layer");
    c4LayerA.m_Hash = hash;
    c4LayerB.m_Hash = hash;
    hash = nlStringLowerHash("Slide2");
    c4NameA.m_Hash = hash;
    c4NameB.m_Hash = hash;

    mConfirmationImage->mImageInstance = findImage.byRef(
        presentation,
        (InlineHasher&)c4NameB,
        (InlineHasher&)c4LayerB,
        (InlineHasher&)c4SlideB,
        (InlineHasher&)z5,
        (InlineHasher&)z3,
        (InlineHasher&)z1);

    char texturePath[0x40] = "";
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

    nlSNPrintf(texturePath, sizeof(texturePath), "fe/health_and_safety/ProgressiveScan_%s", language);

    mUseProgressiveImage[0]->QueueLoad(texturePath, true);
    mUseProgressiveImage[1]->QueueLoad(texturePath, true);
}

/**
 * Offset/Address/Size: 0x940 | 0x80110E10 | size: 0x3C8
 */
#pragma inline_depth(8)
void ProgressiveScanScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    if (mUseProgressiveImage[0]->Update(true))
    {
        TLImageInstance* img = FEFinder<TLImageInstance, 2>::Find<FEPresentation>(
            m_pFEPresentation,
            InlineHasher(nlStringLowerHash("Slide1")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("ProgressiveScan_deu")));
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
        if ((mElapsedTime += fDeltaT) >= 0.5f)
        {
            mSelectorComponent->m_bVisible = true;
            if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 14, true, NULL) || g_pFEInput->IsAutoPressed(FE_ALL_PADS, 13, true, NULL))
            {
                mElapsedTime = 0.5f;
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
                if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL) || mElapsedTime >= 10.5f)
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
        if ((mElapsedTime += fDeltaT) >= 3.0f)
        {
            SwitchMessageImage();
            mCanProceed = true;
            mElapsedTime = 0.0f;
        }
    }
    if (mCanProceed)
    {
        mElapsedTime += fDeltaT;
        if (mElapsedTime >= 3.5f || g_pFEInput->JustReleased(FE_ALL_PADS, 0x100, false, NULL) || g_pFEInput->JustReleased(FE_ALL_PADS, 0x200, false, NULL))
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_HEALTH_WARNING, SCREEN_NOTHING, true);
        }
    }
}
#pragma inline_depth()

/**
 * TODO: 99.31% match - r31/r29 register swap (this vs language literal)
 *       across 8 switch cases
 */
static inline void InitProgressiveScanLanguageStringData(BasicStringData<char>* data, const char* text)
{
    data->mData = 0;
    data->mSize = 0;
    data->mCapacity = 0;

    const char* scan = text;
    while (*scan++ != 0)
    {
        data->mSize++;
    }

    data->mSize++;
    data->mData = (char*)nlMalloc((data->mSize + 1) * sizeof(char), 8, true);
    data->mCapacity = data->mSize;

    for (int i = 0; i < data->mSize; i++)
    {
        data->mData[i] = *text++;
    }

    data->mRefCount = 1;
}

#define ASSIGN_LANG(langLiteral)                                                                                \
    {                                                                                                           \
        BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(sizeof(BasicStringData<char>), 8, true); \
        if (data != 0)                                                                                          \
        {                                                                                                       \
            InitProgressiveScanLanguageStringData(data, langLiteral);                                           \
        }                                                                                                       \
        languageString = BasicString<char, Detail::TempStringAllocator>(data);                                  \
    }

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
        ASSIGN_LANG("eng");
        break;
    case 1:
        ASSIGN_LANG("fre");
        break;
    case 2:
        ASSIGN_LANG("deu");
        break;
    case 3:
        ASSIGN_LANG("spa");
        break;
    case 4:
        ASSIGN_LANG("ita");
        break;
    case 5:
        ASSIGN_LANG("jpn");
        break;
    case 6:
        ASSIGN_LANG("uke");
        break;
    default:
        ASSIGN_LANG("eng");
        break;
    }

    char textureName[64];
    nlSNPrintf(textureName, sizeof(textureName), "fe/health_and_safety/Progressive%s_%s", confirmationText, languageString.c_str());
    textureName[sizeof(textureName) - 1] = '\0';

    mConfirmationImage->QueueLoad(textureName, true);
}
#undef ASSIGN_LANG
