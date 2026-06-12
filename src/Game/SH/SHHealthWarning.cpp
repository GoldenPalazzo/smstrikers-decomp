#include "Game/SH/SHHealthWarning.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/FE/tlComponentInstance.h"
#include "NL/nlMemory.h"
#include "NL/nlString.h"
#include "NL/nlTask.h"

extern int g_Language;

/**
 * Offset/Address/Size: 0x5C8 | 0x8010D6F8 | size: 0xDC
 */
HealthWarningSceneV2::HealthWarningSceneV2()
    : BaseSceneHandler()
{
    const char* messageImageFilename = "art/fe/HealthSafetyUI.res";

    mMessageImage = NULL;
    mPressButtonImage = NULL;
    mElapsedTime = 0.0f;

    AsyncImage* messageImage = new (nlMalloc(0x1C, 0x20, 1)) AsyncImage(messageImageFilename, NULL);
    mMessageImage = messageImage;

    AsyncImage* pressButtonImage = new (nlMalloc(0x1C, 0x20, 1)) AsyncImage(messageImageFilename, NULL);
    mPressButtonImage = pressButtonImage;
}

/**
 * Offset/Address/Size: 0x514 | 0x8010D644 | size: 0xB4
 */
HealthWarningSceneV2::~HealthWarningSceneV2()
{
    if (mMessageImage != NULL)
    {
        delete mMessageImage;
    }

    if (mPressButtonImage != NULL)
    {
        delete mPressButtonImage;
    }
}

/**
 * Offset/Address/Size: 0x204 | 0x8010D334 | size: 0x310
 */
void HealthWarningSceneV2::SceneCreated()
{
    TLImageInstance* pImage;
    TLComponentInstance* pComp;

    mMessageImage->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("health_and_safety_EU_english")));

    pImage = NULL;

    pComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("press")));

    pImage = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        pComp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("health_press")),
        InlineHasher(0));

    mPressButtonImage->mImageInstance = pImage;
    pImage->m_bVisible = false;
    mIsPressButtonVisible = false;

    switch (g_Language)
    {
    case 0:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_US_english", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_US_english_press", false);
        break;
    case 1:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_french", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_french_press", false);
        break;
    case 2:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_german", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_german_press", false);
        break;
    case 3:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_spanish", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_spanish_press", false);
        break;
    case 4:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_italian", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_italian_press", false);
        break;
    case 5:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_japan", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_JP_japan_press", false);
        break;
    case 6:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_english", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_EU_english_press", false);
        break;
    default:
        mMessageImage->QueueLoad("fe/health_and_safety/health_and_safety_US_english", false);
        mPressButtonImage->QueueLoad("fe/health_and_safety/health_and_safety_US_english_press", false);
        break;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8010D130 | size: 0x204
 */
void HealthWarningSceneV2::Update(float fDeltaT)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    volatile InlineHasher hN, hM, hL, hK, hJ, hI, hH, hG;
    volatile InlineHasher h0, h1, h2, h3, h4, h5, h6, h7, h8, h9, hA, hB;

    bool proceed;
    TLComponentInstance* pComp;
    TLImageInstance* pImage;

    BaseSceneHandler::Update(fDeltaT);
    mMessageImage->Update(true);
    mPressButtonImage->Update(true);

    f32 newTime = mElapsedTime + fDeltaT;
    mElapsedTime = newTime;
    if (!(newTime >= 2.0f))
        return;

    proceed = false;

    if (!mIsPressButtonVisible)
    {
        AudioLoader::LoadFEButtonSoundGroup();

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h3.m_Hash = 0;
        h2.m_Hash = 0;
        h1.m_Hash = 0;
        h0.m_Hash = 0;
        hG.m_Hash = 0;
        hH.m_Hash = 0;
        hI.m_Hash = 0;
        hJ.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("press");
        hK.m_Hash = hash;
        hL.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hN.m_Hash = hash;
        hM.m_Hash = hash;

        pComp = findComp.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)hN,
            (InlineHasher&)hL,
            (InlineHasher&)hJ,
            (InlineHasher&)hH,
            (InlineHasher&)h0,
            (InlineHasher&)h2);

        pComp->SetActiveSlide("Slide1");
        pComp->Update(0.0f);

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

        hB.m_Hash = 0;
        h2.m_Hash = 0;
        hA.m_Hash = 0;
        h0.m_Hash = 0;
        h9.m_Hash = 0;
        hH.m_Hash = 0;
        h8.m_Hash = 0;
        hJ.m_Hash = 0;
        h7.m_Hash = 0;
        h6.m_Hash = 0;

        hash = nlStringLowerHash("health_press");
        h5.m_Hash = hash;
        h4.m_Hash = hash;

        pImage = findImage.byRef(
            pComp->GetActiveSlide(),
            (InlineHasher&)h4,
            (InlineHasher&)h6,
            (InlineHasher&)hJ,
            (InlineHasher&)hH,
            (InlineHasher&)h0,
            (InlineHasher&)h2);

        pImage->m_bVisible = true;
        mIsPressButtonVisible = true;
    }

    if (g_pFEInput->JustPressed((eFEINPUT_PAD)8, 0x24, true, NULL) || g_pFEInput->JustPressed((eFEINPUT_PAD)8, 0xF70, false, NULL))
    {
        proceed = true;
    }

    if (mElapsedTime >= 60.0f)
    {
        proceed = true;
    }

    if (proceed)
    {
        FEAudio::PlayAnimAudioEvent("sfx_screen_forward", false);
        nlTaskManager::SetNextState(0x80000);
    }
}
