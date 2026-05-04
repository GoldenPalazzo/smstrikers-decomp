#include "Game/SH/SHCredits.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feTemplates.h"
#include "Game/FE/tlImageInstance.h"
#include "NL/nlMemory.h"

static char* CREDITS_LINE_NAMES[] = {
    "Line1",
    "Line2",
    "Line3",
    "Line4",
    "Line5",
    "Line6",
    "Line7",
    "Line8",
    "Line9",
    "Line10",
};

class AudioLoader
{
public:
    static void StartFEStream(const char*, bool, const char*);
};

/**
 * Offset/Address/Size: 0x570 | 0x80110374 | size: 0x15C
 */
template <>
template <>
TLTextInstance* FEFinder<TLTextInstance, 3>::_Find<TLInstance>(
    TLInstance* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->pChildren, Level1);

    if (pChild == 0)
        return 0;

    if (Level2 == 0)
        return (TLTextInstance*)pChild;

    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->pChildren, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x468 | 0x8011026C | size: 0x84
 */
template <>
template <>
TLTextInstance* FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(
    FEPresentation* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLSlide>(pTopLevel->m_slides, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLTextInstance*)pChild;
    return _Find<TLSlide>(CastToSomeType<TLSlide>(pTopLevel->m_slides, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x4EC | 0x801102F0 | size: 0x84
 */
template <>
template <>
TLTextInstance* FEFinder<TLTextInstance, 3>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLTextInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x430 | 0x80110234 | size: 0x38
 */
template <>
template <>
TLTextInstance* FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
    FEPresentation* slide, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find<FEPresentation>(slide, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

// /**
//  * Offset/Address/Size: 0x2D4 | 0x801100D8 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x80110054 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x218 | 0x8011001C | size: 0x38
 */
template <>
template <>
TLImageInstance* FEFinder<TLImageInstance, 2>::Find<TLSlide>(
    TLSlide* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find<TLSlide>(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

// /**
//  * Offset/Address/Size: 0xBC | 0x8010FEC0 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8010FE3C | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8010FE04 | size: 0x38
 */
template <>
template <>
TLComponentInstance* FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
    TLSlide* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find<TLSlide>(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

/**
 * Offset/Address/Size: 0xBCC | 0x8010FD28 | size: 0xDC
 */
CreditScene::CreditScene()
    : mAreCreditsOver(false)
    , mFinalMessageDisplayed(false)
    , mTimeElapsed(false)
{
    *(f32*)&mCreditParser.mFileSize = 0.0f;

    for (int i = 0; i < 10; i++)
    {
        m_pTextLines[i] = NULL;
        mLineOnScreen[i] = false;
    }
}

/**
 * Offset/Address/Size: 0xB40 | 0x8010FC9C | size: 0x8C
 */
CreditScene::~CreditScene()
{
    if (&mCreditParser.mActualSize != NULL)
    {
        char** data = (char**)&mCreditParser.mActualSize;
        if (data[1] != NULL)
        {
            nlFree(data[1]);
            data[1] = NULL;
        }
    }
}

/**
 * Offset/Address/Size: 0xB20 | 0x8010FC7C | size: 0x20
 */
void CreditScene::SceneCreated()
{
    SetupForPhase();
}

/**
 * Offset/Address/Size: 0xA80 | 0x8010FBDC | size: 0xA0
 */
void CreditScene::Update(float dt)
{
    BaseSceneHandler::Update(dt);
    switch ((s32)mCreditParser.mFileData)
    {
    case 2:
        UpdateForCredits(dt);
        break;
    case 3:
        UpdateForCopyrightMessage(dt);
        break;
    case 1:
        UpdateForNintendoLogo(dt);
        break;
    case 0:
        UpdateForNLGMovie(dt);
        break;
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x978 | 0x8010FAD4 | size: 0x108
 */
void CreditScene::SetupForPhase()
{
    FORCE_DONT_INLINE;
    mTimeElapsed = 0;
    *(f32*)&mCreditParser.mFileSize = 0.0f;

    switch ((s32)mCreditParser.mFileData)
    {
    case 0:
        SetupForNLGMovie();
        break;
    case 1:
        m_pFEPresentation->SetActiveSlide("NINTENDO");
        m_pFEPresentation->m_currentSlide->Update(0.0f);
        break;
    case 2:
        SetupForCredits();
        break;
    case 3:
        m_pFEPresentation->SetActiveSlide("COPYRIGHTS");
        m_pFEPresentation->m_currentSlide->Update(0.0f);
        break;
    case 4:
    {
        nlSingleton<GameSceneManager>::s_pInstance->Push(mNextScene, SCREEN_NOTHING, true);
        if (mNextScene == SCENE_OPTIONS)
        {
            FEMusic::StartStreamIfDifferent(7);
        }
        else
        {
            FEMusic::StartStreamIfDifferent(0);
        }
        mNextScene = SCENE_MAIN_MENU;
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0x94C | 0x8010FAA8 | size: 0x2C
 */
void CreditScene::GotoNextPhase()
{
    FORCE_DONT_INLINE;
    mCreditParser.mFileData++;
    SetupForPhase();
}

/**
 * Offset/Address/Size: 0x6E4 | 0x8010F840 | size: 0x268
 */
void CreditScene::SetupForCredits()
{
    FORCE_DONT_INLINE;
    m_pFEPresentation->SetActiveSlide("CREDITS");
    m_pFEPresentation->m_currentSlide->Update(0.0f);

    s32 yOffset;
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    TLTextInstance* pFinalText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("CREDITS")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Final Message")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pFinalText->m_bVisible = false;

    mCreditParser.mActualData = (char*)nlLoadEntireFile("credits.txt", &mCreditParser.mActualSize, 0x20, (eAllocType)1);
    mCreditParser.mParser.StartParsing(mCreditParser.mActualData, mCreditParser.mActualSize, false);

    nlVector2 boxSize = { 1280.0f, 480.0f };

    s32 i = 0;
    yOffset = 0;
    for (; i < 10; i++)
    {
        m_pTextLines[i] = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
            presentation,
            InlineHasher(nlStringLowerHash("CREDITS")),
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(CREDITS_LINE_NAMES[i])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        m_pTextLines[i]->SetAssetScale(0.75f, 0.75f, 1.0f);
        m_pTextLines[i]->m_OverloadFlags |= 0x10;
        m_pTextLines[i]->m_DrawOptions |= 0x10;
        m_pTextLines[i]->m_OverloadedAttributes.BoxSize = boxSize;
        m_pTextLines[i]->m_OverloadFlags |= 0x4;

        feVector3 pos = m_pTextLines[i]->GetAssetPosition();
        m_pTextLines[i]->SetAssetPosition(pos.f.x, (f32)(-230 - yOffset), pos.f.z);

        yOffset += 46;
    }

    AudioLoader::StartFEStream("FE_Credits", true, "FE");
}

/**
 * Offset/Address/Size: 0x60C | 0x8010F768 | size: 0xD8
 */
void CreditScene::SetupForNLGMovie()
{
    BaseSceneHandler* pScene = nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_NLG_MOVIE, SCREEN_NOTHING, false);
    ((u8*)pScene)[0xAA] = 0;

    nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();

    m_pFEPresentation->SetActiveSlide("nlg");

    typedef TLImageInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    union
    {
        FindByValue byValue;
        FindByRef byRef;
    } findImg;
    findImg.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

    volatile InlineHasher lRef, lLocal;
    volatile InlineHasher nRef, nLocal;
    volatile InlineHasher h, g, f, e, d, c, b, a;
    a.m_Hash = 0;
    b.m_Hash = 0;
    c.m_Hash = 0;
    d.m_Hash = 0;
    e.m_Hash = 0;
    f.m_Hash = 0;
    g.m_Hash = 0;
    h.m_Hash = 0;
    u32 hash1 = nlStringLowerHash("NLG Logo");
    nLocal.m_Hash = hash1;
    nRef.m_Hash = hash1;
    u32 hash2 = nlStringLowerHash("Layer");
    lRef.m_Hash = hash2;
    lLocal.m_Hash = hash2;
    TLImageInstance* pImage = findImg.byRef(
        m_pFEPresentation->m_currentSlide, (InlineHasher&)lRef, (InlineHasher&)nRef, (InlineHasher&)h, (InlineHasher&)f, (InlineHasher&)d, (InlineHasher&)b);
    pImage->m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x554 | 0x8010F6B0 | size: 0xB8
 */
void CreditScene::UpdateForCopyrightMessage(float dt)
{
    FORCE_DONT_INLINE;
    TLComponentInstance* pWhiteFade = GetWhiteFadeComponent();
    f32 timeElapsed = *(f32*)&mCreditParser.mFileSize;
    timeElapsed += dt;
    *(f32*)&mCreditParser.mFileSize = timeElapsed;
    if (timeElapsed < 1.0f)
    {
        return;
    }
    TLSlide* pSlide = pWhiteFade->GetActiveSlide();
    f32 slideEnd = pSlide->m_start + pSlide->m_duration;
    TLSlide* pSlide2 = pWhiteFade->GetActiveSlide();
    if (pSlide2->m_time >= slideEnd)
    {
        if (!mTimeElapsed)
        {
            pWhiteFade->SetActiveSlide("credits");
            mTimeElapsed = 1;
        }
        else
        {
            GotoNextPhase();
        }
    }
}

/**
 * Offset/Address/Size: 0x1BC | 0x8010F318 | size: 0x398
 * TODO: 97.87% match - register permutation differences in both string copy loops
 */
void CreditScene::UpdateForCredits(float dt)
{
    s32 yDelta = (s32)(-120.0f * (dt / 1000.0f));
    f32 resetY = -230.0f;
    s32 lineCount = 0;

    for (s32 i = 0; i < 10; i++)
    {
        feVector3 pos = m_pTextLines[i]->GetAssetPosition();

        if (pos.f.y >= resetY && !mLineOnScreen[i])
        {
            bool hasToken;
            const char* pToken = mCreditParser.mParser.NextToken(false);

            if (pToken != NULL)
            {
                if (pToken[0] == '+')
                {
                    u32 count = 64;
                    const unsigned char* pSrc = (const unsigned char*)"";
                    u32 k = 0;
                    while (count-- && (mStrings[i][k] = *pSrc) != 0)
                    {
                        ++pSrc;
                        ++k;
                    }
                    mStrings[i][63] = 0;
                }
                else
                {
                    const unsigned char* pSrc = (const unsigned char*)pToken;
                    u32 ch = 0;
                    u32 count = 64;
                    while (count-- && (mStrings[i][ch] = pSrc[ch]) != 0)
                    {
                        ch++;
                    }
                    mStrings[i][63] = 0;
                }

                mCreditParser.mParser.AdvanceLine();
                hasToken = true;
            }
            else
            {
                hasToken = false;
            }

            if (hasToken)
            {
                m_pTextLines[i]->SetString(mStrings[i]);
                mLineOnScreen[i] = true;
                pos.f.y += (float)yDelta;
                m_pTextLines[i]->SetAssetPosition(pos.f.x, pos.f.y, pos.f.z);
            }
        }
        else
        {
            if (pos.f.y >= -720.0f && mLineOnScreen[i])
            {
                mLineOnScreen[i] = false;
                pos.f.y = resetY;
                m_pTextLines[i]->SetAssetPosition(pos.f.x, pos.f.y, pos.f.z);
            }
            else
            {
                pos.f.y += (float)yDelta;
                m_pTextLines[i]->SetAssetPosition(pos.f.x, pos.f.y, pos.f.z);
            }
        }

        if (mLineOnScreen[i])
        {
            lineCount++;
        }
    }

    if (lineCount == 0)
    {
        mAreCreditsOver = true;
    }

    if (!mTimeElapsed)
    {
        bool shouldFade = false;

        if (!mAreCreditsOver)
        {
            f32 time = *(f32*)&mCreditParser.mFileSize;
            time += dt;
            *(f32*)&mCreditParser.mFileSize = time;

            if (time >= 2.0 && !mFinalMessageDisplayed)
            {
                shouldFade = true;
            }
        }
        else
        {
            if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x24, true, NULL)
                || g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
            {
                shouldFade = true;
            }
        }

        if (shouldFade)
        {
            mTimeElapsed = 1;

            TLComponentInstance* pWhiteFade = GetWhiteFadeComponent();
            pWhiteFade->SetActiveSlide("FADEIN");
            pWhiteFade->Update(0.0f);
        }
    }
    else
    {
        TLComponentInstance* pWhiteFade = GetWhiteFadeComponent();
        TLSlide* pSlide = pWhiteFade->GetActiveSlide();
        f32 slideEnd = pSlide->m_start + pSlide->m_duration;

        if (pWhiteFade->GetActiveSlide()->m_time >= slideEnd)
        {
            if (mCreditParser.mActualData != NULL)
            {
                nlFree(mCreditParser.mActualData);
                mCreditParser.mActualData = NULL;
            }

            GotoNextPhase();
        }
    }
}

/**
 * Offset/Address/Size: 0x104 | 0x8010F260 | size: 0xB8
 */
void CreditScene::UpdateForNintendoLogo(float dt)
{
    FORCE_DONT_INLINE;
    TLComponentInstance* pWhiteFade = GetWhiteFadeComponent();
    f32 timeElapsed = *(f32*)&mCreditParser.mFileSize;
    timeElapsed += dt;
    *(f32*)&mCreditParser.mFileSize = timeElapsed;
    if (timeElapsed < 1.0f)
    {
        return;
    }
    TLSlide* pSlide = pWhiteFade->GetActiveSlide();
    f32 slideEnd = pSlide->m_start + pSlide->m_duration;
    TLSlide* pSlide2 = pWhiteFade->GetActiveSlide();
    if (pSlide2->m_time >= slideEnd)
    {
        if (!mTimeElapsed)
        {
            pWhiteFade->SetActiveSlide("FADEIN");
            mTimeElapsed = 1;
        }
        else
        {
            GotoNextPhase();
        }
    }
}

/**
 * Offset/Address/Size: 0x98 | 0x8010F1F4 | size: 0x6C
 */
void CreditScene::UpdateForNLGMovie(float)
{
    FORCE_DONT_INLINE;
    if (FESceneManager::GetInstance()->AreAllScenesValid())
    {
        GameSceneManager* pGameSceneMgr = GameSceneManager::GetInstance();
        BaseSceneHandler* pCurrentScene;
        if (pGameSceneMgr->mCurrentStackDepth != 0)
        {
            pCurrentScene = pGameSceneMgr->mBaseSceneHandlerStack[pGameSceneMgr->mCurrentStackDepth - 1];
        }
        else
        {
            pCurrentScene = NULL;
        }
        if (pCurrentScene == this)
        {
            GotoNextPhase();
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8010F15C | size: 0x98
 */
TLComponentInstance* CreditScene::GetWhiteFadeComponent()
{
    FORCE_DONT_INLINE;
    typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    union
    {
        FindByValue byValue;
        FindByRef byRef;
    } findComp;
    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    volatile InlineHasher lRef, lLocal;
    volatile InlineHasher wRef, wLocal;
    volatile InlineHasher h, g, f, e, d, c, b, a;
    a.m_Hash = 0;
    b.m_Hash = 0;
    c.m_Hash = 0;
    d.m_Hash = 0;
    e.m_Hash = 0;
    f.m_Hash = 0;
    g.m_Hash = 0;
    h.m_Hash = 0;
    u32 hash1 = nlStringLowerHash("WHITE FADE");
    wLocal.m_Hash = hash1;
    wRef.m_Hash = hash1;
    u32 hash2 = nlStringLowerHash("Layer");
    lRef.m_Hash = hash2;
    lLocal.m_Hash = hash2;
    return findComp.byRef(
        m_pFEPresentation->m_currentSlide, (InlineHasher&)lRef, (InlineHasher&)wRef, (InlineHasher&)h, (InlineHasher&)f, (InlineHasher&)d, (InlineHasher&)b);
}
