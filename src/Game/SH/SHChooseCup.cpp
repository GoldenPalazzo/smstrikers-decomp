#include "Game/SH/SHChooseCup.h"
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

static inline void CopyWideString(BasicStringData<unsigned short>* data, const unsigned short* text)
{
    data->mData = 0;
    data->mSize = 0;
    data->mCapacity = 0;

    const unsigned short* ptr = text;
    while (*ptr++ != 0)
    {
        data->mSize++;
    }

    data->mSize++;
    data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
    data->mCapacity = data->mSize;

    int j;
    int i = 0;
    j = i;
    while (i < data->mSize)
    {
        *(unsigned short*)((char*)data->mData + j) = *text;
        i++;
        text++;
        j += 2;
    }

    data->mRefCount = 1;
}

// /**
//  * Offset/Address/Size: 0x2C4 | 0x800DCE10 | size: 0xBC
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::Append(const unsigned short*) const
// {
// }

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

/**
 * Offset/Address/Size: 0x1F64 | 0x800DC1E8 | size: 0x154
 * TODO: 91.74% match - Bind temp/no callback stack slot order differs (0x0C/0x1C),
 * placement-new path still has duplicate NULL-check branch, and FEPopupMenu::Create
 * call remains ref-vs-value ABI mismatch in available declarations.
 */
template <>
struct BindExp1<void, void (*)(bool), bool>
{
    void (*mFuncPtr)(bool);
    union
    {
        bool mArg;
        int mArgWord;
    };

    BindExp1() { }
    ~BindExp1() { }
};

void startNewCup(bool isSuperCup)
{
    FEPopupMenu* pPopup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

    {
        BindExp1_vfb bindResult = Bind<void, void (*)(bool), bool>(confirmedNewCup, isSuperCup);
        Function<FnVoidVoid> no;

        Function<FnVoidVoid> yes;
        yes.mTag = FUNCTOR;
        FunctorImpl_vfb* functor = (FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false);
        if (functor != NULL)
        {
            functor = new (functor) FunctorImpl_vfb();
            functor->mBind.mFuncPtr = bindResult.mFuncPtr;
            functor->mBind.mArg = bindResult.mArg;
        }
        yes.mFunctor = functor;

        no.mTag = FREE_FUNCTION;
        no.mFreeFunction = FEPopupMenu::Nothing;

        pPopup->Create(POPUP_REALLY_OVERWRITE, yes, no);
    }
    *(u8*)((u8*)pPopup + 0xAA4) = 0;
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
    ((u8*)this)[0x480] = 0;

    const char* TROPHY_FILE_NAME = "art/fe/TrophiesUI.res";

    ((u8*)this)[0x481] = 0;

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
extern "C" void DisplayCup__16ChooseCupSceneV2Fv(ChooseCupSceneV2*);

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

    DisplayCup__16ChooseCupSceneV2Fv(this);

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

/**
 * Offset/Address/Size: 0x132C | 0x800DB5B0 | size: 0x700
 */
void ChooseCupSceneV2::Update(float)
{
}

/**
 * Offset/Address/Size: 0x7AC | 0x800DAA30 | size: 0xB80
 */
void ChooseCupSceneV2::DisplayCup()
{
}

/**
 * Offset/Address/Size: 0x438 | 0x800DA6BC | size: 0x374
 * TODO: 96.64% match - InlineHasher temps at sp+0x14 vs target sp+0x1c (8-byte stack shift),
 * r30/r31 swap for locString/data (r30=locString/r31=data vs target r31=locString/r30=data)
 */
void ChooseCupSceneV2::SetCurrentChamp(eTeamID teamID, bool hasChamp, TLComponentInstance* comp)
{
    TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        comp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Text")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (hasChamp)
    {
        const unsigned short* locString = LookupLocString("CupChampion");

        BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
        if (data != 0)
        {
            CopyWideString(data, locString);
        }

        BasicString<unsigned short, Detail::TempStringAllocator> msg(data);

        unsigned long charHash = GetLOCCharacterName(teamID, false, false);
        const unsigned short* charName = LookupLocHash(charHash);

        BasicString<unsigned short, Detail::TempStringAllocator> formattedResult = Format(msg, charName);

        memcpy(mChampBuffer, formattedResult.c_str(), 0x200);
        text->SetString(mChampBuffer);
    }
    else
    {
        text->SetStringId("");
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800DA284 | size: 0x438
 * TODO: 95.13% match - popup callback Function<FnVoidVoid>/Bind stack-slot layout
 * still differs from target (0x40/0x50/0x60 vs 0x58/0x40 pattern), and FEPopupMenu
 * call sites still resolve to ref-signature symbols instead of by-value symbols.
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
        bool isSuperCup = mIsSuperCup;

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

        {
            BindExp1_vfb bindContinue = Bind<void, void (*)(bool), bool>(continueCup, mIsSuperCup);

            Function<FnVoidVoid> yes;
            yes.mTag = FUNCTOR;
            FunctorImpl_vfb* yesFunctor = new ((FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false)) FunctorImpl_vfb(bindContinue);
            yes.mFunctor = yesFunctor;

            BindExp1_vfb bindNew = Bind<void, void (*)(bool), bool>(startNewCup, mIsSuperCup);

            Function<FnVoidVoid> no;
            no.mTag = FUNCTOR;
            FunctorImpl_vfb* noFunctor = new ((FunctorImpl_vfb*)nlMalloc(sizeof(FunctorImpl_vfb), 8, false)) FunctorImpl_vfb(bindNew);
            no.mFunctor = noFunctor;

            menu->Create(POPUP_START_NEW_CUP, yes, no);
        }

        {
            Function<FnVoidVoid> back;
            back.mTag = FREE_FUNCTION;
            back.mFreeFunction = FEPopupMenu::Nothing;
            menu->SetBackButtonCallback(back);
        }

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

        ((u8*)this)[0x480] = text->m_bVisible;
        text->m_bVisible = false;
        ((u8*)this)[0x481] = 1;
    }
}
