#define BASICSTRING_DELEGATING_CTOR
#define FEPOPUPMENU_BYVAL_DECLS
#define FUNCTION0_VOLATILE_TAG_DTOR
#define BASICSTRING_INDEX_EMPTY_COPY_BYTE_OFFSET
#include "Game/SH/SHChooseCup.h"
#undef FUNCTION0_VOLATILE_TAG_DTOR
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/feFinder.h"
#include "NL/gl/glStruct.h"
#include "NL/nlLocalization.h"

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

unsigned long GetLOCCharacterName(eTeamID, bool, bool);

template <typename StringType, typename ValueType>
StringType Format(const StringType& format, const ValueType& value);

static inline const unsigned short* LookupLocString(const char* id)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    unsigned long hash = nlStringLowerHash(id);
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* entry = nlBSearch(hash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
    if (entry)
        return loc->m_FirstString + entry->StringOffset;
    return MissingLocString;
}

static inline const unsigned short* LookupLocHash(unsigned long hash)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* entry = nlBSearch(hash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
    if (entry)
        return loc->m_FirstString + entry->StringOffset;
    return MissingLocString;
}

static inline BasicStringData<unsigned short>* BuildCupStringData(const unsigned short* text)
{
    return new (8, true) BasicStringData<unsigned short>(text);
}

// At the bottom of SHChooseCup.cpp -- REMOVE once real callers exist.
static void SHChooseCup_stub()
{
    BasicString<unsigned short, Detail::TempStringAllocator> s;
    s.Append((const unsigned short*)0);
}

// /**
//  * Offset/Address/Size: 0xBC | 0x800DCC08 | size: 0x208
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned
// short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800DCB4C | size: 0xBC
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<unsigned short,
// Detail::TempStringAllocator>&) const
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800DCAC8 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800DCA90 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800DC934 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800DC8B0 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800DC878 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800DC71C | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800DC698 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800DC660 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800DC504 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800DC480 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800DC448 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x20B8 | 0x800DC33C | size: 0x10C
 */
void confirmedNewCup(bool isSuperCup)
{
    GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;

    if (gim->mCurrentMode == GameInfoManager::GM_BOWSER_CUP)
    {
        gim->mCurrentCup = (BaseCup*)&gim->mBowserCupSeries;
        gim->mDoingKnockout = false;
        gim->mBowserCupKnockout.mRoundNumber = -5;
    }
    else if (gim->mCurrentMode == GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        gim->mCurrentCup = (BaseCup*)&gim->mSuperBowserCupSeries;
        gim->mDoingKnockout = false;
        gim->mSuperBowserCupKnockout.mRoundNumber = -5;
    }

    BaseCup* cup = nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup;
    cup->mCupStarted = false;

    GameInfoManager* gim2 = nlSingleton<GameInfoManager>::s_pInstance;
    cup->mCupSettings.SkillLevel = gim2->mUserInfo.mGameplayOptions.SkillLevel;
    cup->mCupSettings.GameTime = gim2->mUserInfo.mGameplayOptions.GameTime;
    cup->mCupSettings.PowerUps = gim2->mUserInfo.mGameplayOptions.PowerUps;
    cup->mCupSettings.Shoot2Score = gim2->mUserInfo.mGameplayOptions.Shoot2Score;
    cup->mCupSettings.BowserAttackEnabled = gim2->mUserInfo.mGameplayOptions.BowserAttackEnabled;
    cup->mCupSettings.RumbleEnabled = gim2->mUserInfo.mGameplayOptions.RumbleEnabled;

    if (!isSuperCup)
    {
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_CUP, SCREEN_NOTHING, true);
    }
    else
    {
        if (cup->mCupSettings.SkillLevel == GameplaySettings::ROOKIE)
        {
            cup->mCupSettings.SkillLevel = GameplaySettings::PROFESSIONAL;
        }
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_SUPER, SCREEN_NOTHING, true);
    }
}

typedef BindExp1<void, void (*)(bool), bool> BindExp1_vfb;
typedef Function0<void>::FunctorImpl<BindExp1_vfb> FunctorImpl_vfb;

template <>
struct BindExp1<void, void (*)(bool), bool>
{
    void (*mFuncPtr)(bool);
    bool mArg;

    BindExp1() { }
};

/**
 * Offset/Address/Size: 0x1F64 | 0x800DC1E8 | size: 0x154
 */
void startNewCup(bool isSuperCup)
{
    FEPopupMenu* pPopup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

    pPopup->Create(
        POPUP_REALLY_OVERWRITE,
        Function<FnVoidVoid>(Bind<void, void (*)(bool), bool>(confirmedNewCup, isSuperCup)),
        Function<FnVoidVoid>(FEPopupMenu::Nothing));

    pPopup->mUnknownAA4 = false;
}

/**
 * Offset/Address/Size: 0x1EFC | 0x800DC180 | size: 0x68
 */
class BaseGameSceneManagerCont
{
public:
    virtual ~BaseGameSceneManagerCont();
    virtual void* Push(int, int, bool);
};

extern BaseGameSceneManagerCont* g_pGameSceneManager;

void continueCup(bool isSuperCup)
{
    if (isSuperCup)
    {
        g_pGameSceneManager->Push(20, 0, true); // SCENE_SUPER_CUP_STANDINGS
    }
    else
    {
        g_pGameSceneManager->Push(17, 0, true); // SCENE_CUP_STANDINGS
    }
}

/**
 * Offset/Address/Size: 0x1DF8 | 0x800DC07C | size: 0x104
 */
ChooseCupSceneV2::ChooseCupSceneV2(bool isSuperCup)
    : mIsSuperCup(isSuperCup)
    , mUpdateSlide(false)
    , mIsFirstSlide(true)
    , mTicker(NULL)
{
    mCupInProgressVisible = false;

    const char* TROPHY_FILE_NAME = "art/fe/TrophiesUI.res";

    mCupInProgressDirty = false;

    mCupImage = new ((AsyncImage*)nlMalloc(0x1C, 0x20, true)) AsyncImage(TROPHY_FILE_NAME, NULL);

    if (mIsSuperCup)
    {
        mCupToDisplay = TROPHY_SUPER_MUSHROOM_CUP;
        mMinCup = TROPHY_SUPER_MUSHROOM_CUP;
        mMaxCup = TROPHY_SUPER_BOWSER_CUP;
    }
    else
    {
        mCupToDisplay = TROPHY_MUSHROOM_CUP;
        mMinCup = TROPHY_MUSHROOM_CUP;
        mMaxCup = TROPHY_BOWSER_CUP;
    }
}

/**
 * Offset/Address/Size: 0x1CA0 | 0x800DBF24 | size: 0x158
 */
ChooseCupSceneV2::~ChooseCupSceneV2()
{
    delete mCupImage;

    FEScrollText* ticker = mTicker;

    if (ticker != NULL)
    {
        if (ticker != NULL)
        {
            if ((char*)ticker + 0x21C)
            {
                volatile FEScrollText* vticker = ticker;
                if ((char*)vticker + 0x21C)
                {
                    if (ticker->m_messageFinishedCB.mTag == FUNCTOR)
                    {
                        delete ticker->m_messageFinishedCB.mFunctor;
                    }
                    ticker->m_messageFinishedCB.mTag = EMPTY;
                }
            }

            if ((char*)ticker + 4)
            {
                BasicStringData<unsigned short>* data = ticker->m_message.m_data;
                if (data != NULL)
                {
                    if (--data->mRefCount == 0)
                    {
                        if (data != NULL)
                        {
                            if (data != NULL)
                            {
                                delete[] data->mData;
                            }
                            if (data != NULL)
                            {
                                nlFree(data);
                            }
                        }
                    }
                }
            }

            ::operator delete(ticker);
        }
    }
}

/**
 * Offset/Address/Size: 0x1A2C | 0x800DBCB0 | size: 0x274
 */
void ChooseCupSceneV2::SceneCreated()
{
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    volatile unsigned long sB, sA;
    volatile unsigned long s9, s8, s6, s4, s2, s0;

    volatile unsigned long p8, p7, p6, p5, p4, p3, p2, p1, p0;
    volatile unsigned long q8, q7, q6, q5, q4, q3, q2, q1, q0;

    TLSlide* slide = m_pFEScene->m_pFEPackage->GetPresentation()->m_currentSlide;

    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

    h0 = 0;
    h1 = 0;
    h2 = 0;
    h3 = 0;
    h4 = 0;
    h5 = 0;
    h6 = 0;
    h7 = 0;

    unsigned long hash = nlStringLowerHash("TROPHY");
    h8 = hash;
    h9 = hash;

    hash = nlStringLowerHash("Layer");
    hA = hash;
    hB = hash;

    TLImageInstance* pTrophyImage = findImage.byRef(slide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);
    mCupImage->mImageInstance = pTrophyImage;

    DisplayCup();

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    s0 = 0;
    h1 = 0;
    s2 = 0;
    h3 = 0;
    s4 = 0;
    h5 = 0;
    s6 = 0;
    h7 = 0;

    hash = nlStringLowerHash("TickerText");
    s8 = hash;
    s9 = hash;

    hash = nlStringLowerHash("Layer");
    sA = hash;
    sB = hash;

    TLTextInstance* scrollText = findText.byRef(slide, (InlineHasher&)sB, (InlineHasher&)s9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

    gl_ScreenInfo* screenInfo = glGetScreenInfo();
    FEScrollText* ticker = new (nlMalloc(0x22C, 0x20, true)) FEScrollText(scrollText, 0, screenInfo->ScreenWidth + 0x32);
    mTicker = ticker;
    mTicker->SetDisplayMessage("CHOOSE_CUP_TICKER");

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

    p0 = 0;
    h1 = 0;
    p1 = 0;
    h3 = 0;
    p2 = 0;
    h5 = 0;

    hash = nlStringLowerHash("buttons");
    p3 = hash;
    p4 = hash;

    hash = nlStringLowerHash("Layer");
    p5 = hash;
    p6 = hash;

    hash = nlStringLowerHash("IN");
    p8 = hash;
    p7 = hash;

    mButtons.mButtonInstance = findComp.byRef(m_pFEPresentation, (InlineHasher&)p8, (InlineHasher&)p6, (InlineHasher&)p4, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);
    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    q0 = 0;
    h1 = 0;
    q1 = 0;
    h3 = 0;
    q2 = 0;
    h5 = 0;

    hash = nlStringLowerHash("buttons");
    q3 = hash;
    q4 = hash;

    hash = nlStringLowerHash("Layer");
    q5 = hash;
    q6 = hash;

    hash = nlStringLowerHash("CHANGE CUPS");
    q8 = hash;
    q7 = hash;

    mButtons2.mButtonInstance = findComp.byRef(m_pFEPresentation, (InlineHasher&)q8, (InlineHasher&)q6, (InlineHasher&)q4, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);
    mButtons2.SetState(ButtonComponent::BS_A_AND_B);
}

static inline bool DisplayCupCanProceed(ChooseCupSceneV2* scene);

/**
 * Offset/Address/Size: 0x132C | 0x800DB5B0 | size: 0x700
 */
void ChooseCupSceneV2::Update(float fDeltaT)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;
    volatile unsigned long sA, sB;
    volatile unsigned long s9, s8, s6, s4, s2, s0;
    volatile unsigned long tB, tA;
    volatile unsigned long t9, t8, t6, t4, t2, t0;
    volatile unsigned long yA, yB;
    volatile unsigned long y9, y8, y6, y4, y2, y0;
    volatile unsigned long vA, vB;
    volatile unsigned long v9, v8, v6, v4, v2, v0;
    volatile unsigned long wB, wA;
    volatile unsigned long w9, w8, w6, w4, w2, w0;
    volatile unsigned long xA, xB;
    volatile unsigned long x9, x8, x6, x4, x2, x0;

    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mButtons2.CentreButtons();
    mCupImage->Update(true);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    TLSlide* activeSlide = presentation->m_currentSlide;

    if (mCupInProgressDirty)
    {
        GameSceneManager* gsm = nlSingleton<GameSceneManager>::s_pInstance;
        BaseSceneHandler* currentScene;

        if (gsm->mCurrentStackDepth != 0)
        {
            currentScene = gsm->mBaseSceneHandlerStack[gsm->mCurrentStackDepth - 1];
        }
        else
        {
            currentScene = NULL;
        }

        if (currentScene == this)
        {
            findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

            h0 = 0;
            h1 = 0;
            h2 = 0;
            h3 = 0;
            h4 = 0;
            h5 = 0;
            h6 = 0;
            h7 = 0;

            unsigned long hash = nlStringLowerHash("cup in progress");
            h8 = hash;
            h9 = hash;

            hash = nlStringLowerHash("Layer");
            hA = hash;
            hB = hash;

            TLComponentInstance* textComp = findComp.byRef(activeSlide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

            textComp->m_bVisible = mCupInProgressVisible;
            mCupInProgressDirty = false;
        }
    }

    if (!mIsFirstSlide || (mIsFirstSlide && activeSlide->m_time >= activeSlide->m_start + activeSlide->m_duration))
    {
        mTicker->Update(fDeltaT);
    }

    if (presentation->m_fadeDuration < activeSlide->m_start + activeSlide->m_duration)
    {
        if (mUpdateSlide)
        {
            DisplayCup();
            mUpdateSlide = false;
        }
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        bool canProceed = DisplayCupCanProceed(this);

        if (canProceed)
        {
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            Proceed();
        }
        else
        {
            FEAudio::PlayAnimAudioEvent("sfx_deny", false);
        }
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, false);
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0x0B, true, NULL))
    {
        if (mCupToDisplay == mMinCup)
        {
            mCupToDisplay = mMaxCup;
        }
        else
        {
            mCupToDisplay = (eTrophyType)(mCupToDisplay - 1);
        }

        presentation = m_pFEPresentation;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        s0 = 0;
        h1 = 0;
        s2 = 0;
        h3 = 0;
        s4 = 0;
        h5 = 0;
        s6 = 0;
        h7 = 0;

        unsigned long hash = nlStringLowerHash("star rotation");
        s8 = hash;
        s9 = hash;

        hash = nlStringLowerHash("Layer");
        sA = hash;
        sB = hash;

        TLComponentInstance* starComp = findComp.byRef(presentation->m_currentSlide, (InlineHasher&)sA, (InlineHasher&)s9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

        float starTime = starComp->GetActiveSlide()->m_time;

        presentation->SetActiveSlide("CHANGE CUPS");
        presentation->Update(0.0f);

        mUpdateSlide = true;
        FEAudio::PlayAnimAudioEvent("sfx_cup_toggle_left", false);

        if (mIsFirstSlide)
        {
            mIsFirstSlide = false;

            TLSlide* activeSlide = presentation->m_currentSlide;

            findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

            t0 = 0;
            h1 = 0;
            t2 = 0;
            h3 = 0;
            t4 = 0;
            h5 = 0;
            t6 = 0;
            h7 = 0;

            unsigned long hash2 = nlStringLowerHash("TickerText");
            t8 = hash2;
            t9 = hash2;

            hash2 = nlStringLowerHash("Layer");
            tA = hash2;
            tB = hash2;

            TLTextInstance* scrollText = findText.byRef(activeSlide, (InlineHasher&)tB, (InlineHasher&)t9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

            scrollText->SetStringId("CHOOSE_CUP_TICKER");
            mTicker->ApplyNewTextInstancePointer(scrollText, 8000.0f, 100.0f);

            mTicker->Update(fDeltaT);

            findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

            y0 = 0;
            h1 = 0;
            y2 = 0;
            h3 = 0;
            y4 = 0;
            h5 = 0;
            y6 = 0;
            h7 = 0;

            hash = nlStringLowerHash("star rotation");
            y8 = hash;
            y9 = hash;

            hash = nlStringLowerHash("Layer");
            yA = hash;
            yB = hash;

            starComp = findComp.byRef(presentation->m_currentSlide, (InlineHasher&)yA, (InlineHasher&)y9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

            starComp->Update(starTime);
        }

        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0x0C, true, NULL))
    {
        if (mCupToDisplay == mMaxCup)
        {
            mCupToDisplay = mMinCup;
        }
        else
        {
            mCupToDisplay = (eTrophyType)(mCupToDisplay + 1);
        }

        presentation = m_pFEPresentation;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        v0 = 0;
        h1 = 0;
        v2 = 0;
        h3 = 0;
        v4 = 0;
        h5 = 0;
        v6 = 0;
        h7 = 0;

        unsigned long hash = nlStringLowerHash("star rotation");
        v8 = hash;
        v9 = hash;

        hash = nlStringLowerHash("Layer");
        vA = hash;
        vB = hash;

        TLComponentInstance* starComp = findComp.byRef(presentation->m_currentSlide, (InlineHasher&)vA, (InlineHasher&)v9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

        float starTime = starComp->GetActiveSlide()->m_time;

        presentation->SetActiveSlide("CHANGE CUPS");
        presentation->Update(0.0f);

        mUpdateSlide = true;
        FEAudio::PlayAnimAudioEvent("sfx_cup_toggle_right", false);

        if (mIsFirstSlide)
        {
            mIsFirstSlide = false;

            TLSlide* activeSlide = presentation->m_currentSlide;

            findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

            w0 = 0;
            h1 = 0;
            w2 = 0;
            h3 = 0;
            w4 = 0;
            h5 = 0;
            w6 = 0;
            h7 = 0;

            unsigned long hash2 = nlStringLowerHash("TickerText");
            w8 = hash2;
            w9 = hash2;

            hash2 = nlStringLowerHash("Layer");
            wA = hash2;
            wB = hash2;

            TLTextInstance* scrollText = findText.byRef(activeSlide, (InlineHasher&)wB, (InlineHasher&)w9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

            scrollText->SetStringId("CHOOSE_CUP_TICKER");
            mTicker->ApplyNewTextInstancePointer(scrollText, 8000.0f, 100.0f);

            mTicker->Update(fDeltaT);

            findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

            x0 = 0;
            h1 = 0;
            x2 = 0;
            h3 = 0;
            x4 = 0;
            h5 = 0;
            x6 = 0;
            h7 = 0;

            hash = nlStringLowerHash("star rotation");
            x8 = hash;
            x9 = hash;

            hash = nlStringLowerHash("Layer");
            xA = hash;
            xB = hash;

            starComp = findComp.byRef(presentation->m_currentSlide, (InlineHasher&)xA, (InlineHasher&)x9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

            starComp->Update(starTime);
        }
    }
}

static const char* TROPHY_TEXTURE_FILENAMES[13] = {
    "fe/trophies/cups_mushroom",
    "fe/trophies/cups_flower",
    "fe/trophies/cups_star",
    "fe/trophies/cups_bowser",
    "fe/trophies/cups_super_mushroom",
    "fe/trophies/cups_super_flower",
    "fe/trophies/cups_super_star",
    "fe/trophies/cups_super_bowser",
    "fe/trophies/cups_veteran",
    "fe/trophies/cups_sniper",
    "fe/trophies/cups_super_striker",
    "fe/trophies/cups_super_team",
    "fe/trophies/cups_lakitu",
};

static unsigned long CUP_EXPLANATIONS[8] = {
    0x92E00D2D,
    0x4442C1E2,
    0x88ABDECD,
    0x3B28BAE5,
    0x13E667DB,
    0xC8F69910,
    0x3684127B,
    0xBFDC9213,
};

static const nlColour CHOOSE_CUP_BLACK = { 0x00, 0x00, 0x00, 0xFF };

static inline bool DisplayCupCanProceed(ChooseCupSceneV2* scene)
{
    bool returnValue;
    if (scene->mIsSuperCup)
    {
        returnValue = true;
    }
    else
    {
        switch (scene->mCupToDisplay)
        {
        case TROPHY_MUSHROOM_CUP:
            returnValue = true;
            break;
        case TROPHY_FLOWER_CUP:
            returnValue = nlSingleton<GameInfoManager>::s_pInstance->IsUserQualified(GameInfoManager::GM_FLOWER_CUP);
            break;
        case TROPHY_STAR_CUP:
            returnValue = nlSingleton<GameInfoManager>::s_pInstance->IsUserQualified(GameInfoManager::GM_STAR_CUP);
            break;
        case TROPHY_BOWSER_CUP:
            returnValue = nlSingleton<GameInfoManager>::s_pInstance->IsUserQualified(GameInfoManager::GM_BOWSER_CUP);
            break;
        }
    }
    return returnValue;
}

/**
 * Offset/Address/Size: 0x7AC | 0x800DAA30 | size: 0xB80
 */
void ChooseCupSceneV2::DisplayCup()
{
    unsigned long GetLOCTrophyName(eTrophyType);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    TLSlide* slide = presentation->m_currentSlide;

    presentation->m_fadeDuration = slide->m_start;
    slide->m_time = slide->m_start;

    TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        slide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("TITLE")));

    pText->m_LocStrId = GetLOCTrophyName(mCupToDisplay);
    pText->m_OverloadFlags |= 0x8;

    mCupImage->QueueLoad(TROPHY_TEXTURE_FILENAMES[(int)mCupToDisplay], false);

    TLImageInstance* pTrophyImage = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("TROPHY")));

    if (DisplayCupCanProceed(this))
    {
        BasicString<unsigned short, Detail::TempStringAllocator> firstHalf(
            BuildCupStringData(LookupLocHash(CUP_EXPLANATIONS[(int)mCupToDisplay])));

        if (((nlLocalization*)g_pLocalization)->m_CurrentLanguage != nlLocalization::LangJapanese)
        {
            firstHalf = firstHalf.Append((const unsigned short*)L" ");
        }

        BasicString<unsigned short, Detail::TempStringAllocator> secondHalf;
        if (mCupToDisplay == TROPHY_BOWSER_CUP || mCupToDisplay == TROPHY_SUPER_BOWSER_CUP)
        {
            secondHalf = BasicString<unsigned short, Detail::TempStringAllocator>(
                BuildCupStringData(LookupLocString("CUPEXP_KNOCKOUT")));
        }
        else
        {
            secondHalf = BasicString<unsigned short, Detail::TempStringAllocator>(
                BuildCupStringData(LookupLocString("CUPEXP_LEAGUE")));
        }

        BasicString<unsigned short, Detail::TempStringAllocator> descriptor = firstHalf.Append(secondHalf);

        memcpy(mDescriptorBuffer, descriptor.c_str(), 0x200);
        mDescriptorBuffer[0xFF] = 0;

        pText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            slide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("DESCRIPTOR")));

        pText->SetString(mDescriptorBuffer);

        nlColour trophyColour = ((FELibObject*)pTrophyImage->m_component)->GetColour();
        pTrophyImage->SetAssetColour(trophyColour);
    }
    else
    {
        pText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            slide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("DESCRIPTOR")));

        pTrophyImage->SetAssetColour(CHOOSE_CUP_BLACK);

        switch (mCupToDisplay)
        {
        case TROPHY_FLOWER_CUP:
            pText->SetStringId("FLOWER_LOCKED");
            break;
        case TROPHY_STAR_CUP:
            pText->SetStringId("STAR_LOCKED");
            break;
        case TROPHY_BOWSER_CUP:
            pText->SetStringId("BOWSER_LOCKED");
            break;
        default:
            break;
        }
    }

    TLComponentInstance* progressComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        slide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("cup in progress")));

    GameInfoManager::eGameModes cupMode;
    switch (mCupToDisplay)
    {
    case TROPHY_MUSHROOM_CUP:
        cupMode = GameInfoManager::GM_MUSHROOM_CUP;
        break;
    case TROPHY_FLOWER_CUP:
        cupMode = GameInfoManager::GM_FLOWER_CUP;
        break;
    case TROPHY_STAR_CUP:
        cupMode = GameInfoManager::GM_STAR_CUP;
        break;
    case TROPHY_BOWSER_CUP:
        cupMode = GameInfoManager::GM_BOWSER_CUP;
        break;
    case TROPHY_SUPER_MUSHROOM_CUP:
        cupMode = GameInfoManager::GM_SUPER_MUSHROOM_CUP;
        break;
    case TROPHY_SUPER_FLOWER_CUP:
        cupMode = GameInfoManager::GM_SUPER_FLOWER_CUP;
        break;
    case TROPHY_SUPER_STAR_CUP:
        cupMode = GameInfoManager::GM_SUPER_STAR_CUP;
        break;
    case TROPHY_SUPER_BOWSER_CUP:
        cupMode = GameInfoManager::GM_SUPER_BOWSER_CUP;
        break;
    }

    BaseCup* cup = nlSingleton<GameInfoManager>::s_pInstance->GetCup(cupMode);
    if (cup->mCupStarted)
    {
        progressComp->m_bVisible = true;

        pText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            progressComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("Text")),
            InlineHasher(0));

        pText->SetStringId("INPROGRESS");
    }
    else
    {
        UserInfo* userInfo = &nlSingleton<GameInfoManager>::s_pInstance->mUserInfo;
        Spoil cupSpoil = userInfo->mSpoils[(int)mCupToDisplay];

        if (cupSpoil.mCurrentChamp == TEAM_INVALID)
        {
            progressComp->m_bVisible = false;
        }
        else
        {
            SetCurrentChamp(cupSpoil.mCurrentChamp, cupSpoil.mIsCPUChamp, progressComp);
            progressComp->m_bVisible = true;
        }
    }
}

/**
 * Offset/Address/Size: 0x438 | 0x800DA6BC | size: 0x374
 * TODO: 99.5% match - r30/r31 swap for localization result/string data
 */
void ChooseCupSceneV2::SetCurrentChamp(eTeamID currentChamp, bool isCPUChamp, TLComponentInstance* cupInProgressComponent)
{
    TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        cupInProgressComponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Text")),
        InlineHasher(0));

    if (isCPUChamp)
    {
        BasicString<unsigned short, Detail::TempStringAllocator> formatted(
            Format(
                BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocString("CUPCHAMP")),
                LookupLocHash(GetLOCCharacterName(currentChamp, false, false))));

        memcpy(mChampBuffer, formatted.c_str(), 0x200);
        pText->SetString(mChampBuffer);
    }
    else
    {
        pText->SetStringId("CUPCHAMP_USER");
    }
}

static inline bool ProceedCupMode(bool isSuperCup)
{
    GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;

    if (gim->mCurrentMode == GameInfoManager::GM_BOWSER_CUP)
    {
        gim->mCurrentCup = (BaseCup*)&gim->mBowserCupSeries;
        gim->mDoingKnockout = false;
        gim->mBowserCupKnockout.mRoundNumber = -5;
    }
    else if (gim->mCurrentMode == GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        gim->mCurrentCup = (BaseCup*)&gim->mSuperBowserCupSeries;
        gim->mDoingKnockout = false;
        gim->mSuperBowserCupKnockout.mRoundNumber = -5;
    }

    return isSuperCup;
}

static inline void SetupStartNewCupBackCallback(FEPopupMenu* menu)
{
    Function<FnVoidVoid> back;
    back.mTag = FREE_FUNCTION;
    back.mFreeFunction = FEPopupMenu::Nothing;
    menu->SetBackButtonCallback(back);
}

/**
 * Offset/Address/Size: 0x0 | 0x800DA284 | size: 0x438
 * TODO: 99.05% match - popup callback Bind temporaries still use different
 * stack slots/copy width, and FEPopupMenu call sites still resolve to
 * ref-signature symbols.
 */
void ChooseCupSceneV2::Proceed()
{
    switch (mCupToDisplay)
    {
    case TROPHY_MUSHROOM_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_MUSHROOM_CUP);
        break;
    case TROPHY_FLOWER_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_FLOWER_CUP);
        break;
    case TROPHY_STAR_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_STAR_CUP);
        break;
    case TROPHY_BOWSER_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_BOWSER_CUP);
        break;
    case TROPHY_SUPER_MUSHROOM_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_SUPER_MUSHROOM_CUP);
        break;
    case TROPHY_SUPER_FLOWER_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_SUPER_FLOWER_CUP);
        break;
    case TROPHY_SUPER_STAR_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_SUPER_STAR_CUP);
        break;
    case TROPHY_SUPER_BOWSER_CUP:
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_SUPER_BOWSER_CUP);
        break;
    }

    nlSingleton<GameInfoManager>::s_pInstance->GetCurrentRoundNumber();

    GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;
    BaseCup* cup = gim->mCurrentCup;

    if (!cup->mCupStarted)
    {
        bool isSuperCup = ProceedCupMode(mIsSuperCup);

        cup = nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup;
        cup->mCupStarted = false;

        GameInfoManager* gim2 = nlSingleton<GameInfoManager>::s_pInstance;
        cup->mCupSettings.SkillLevel = gim2->mUserInfo.mGameplayOptions.SkillLevel;
        cup->mCupSettings.GameTime = gim2->mUserInfo.mGameplayOptions.GameTime;
        cup->mCupSettings.PowerUps = gim2->mUserInfo.mGameplayOptions.PowerUps;
        cup->mCupSettings.Shoot2Score = gim2->mUserInfo.mGameplayOptions.Shoot2Score;
        cup->mCupSettings.BowserAttackEnabled = gim2->mUserInfo.mGameplayOptions.BowserAttackEnabled;
        cup->mCupSettings.RumbleEnabled = gim2->mUserInfo.mGameplayOptions.RumbleEnabled;

        if (!isSuperCup)
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_CUP, SCREEN_NOTHING, true);
        }
        else
        {
            if (cup->mCupSettings.SkillLevel == GameplaySettings::ROOKIE)
            {
                cup->mCupSettings.SkillLevel = GameplaySettings::PROFESSIONAL;
            }
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_SUPER, SCREEN_NOTHING, true);
        }
    }
    else
    {
        typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;

        volatile unsigned long hB, hA;
        volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;
        unsigned long hash;

        FEPopupMenu* menu = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

        menu->Create(
            POPUP_START_NEW_CUP,
            Function<FnVoidVoid>(Bind<void, void (*)(bool), bool>(continueCup, mIsSuperCup)),
            Function<FnVoidVoid>(Bind<void, void (*)(bool), bool>(startNewCup, mIsSuperCup)));

        menu->SetBackButtonCallback(Function<FnVoidVoid>(FEPopupMenu::Nothing));

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h0 = 0;
        h1 = 0;
        h2 = 0;
        h3 = 0;
        h4 = 0;
        h5 = 0;
        h6 = 0;
        h7 = 0;

        hash = nlStringLowerHash("cup in progress");
        h8 = hash;
        h9 = hash;

        hash = nlStringLowerHash("Layer");
        hB = hash;
        hA = hash;

        TLComponentInstance* text = findComp.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mCupInProgressVisible = text->m_bVisible;
        text->m_bVisible = false;
        mCupInProgressDirty = true;
    }
}
