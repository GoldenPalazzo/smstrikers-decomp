#include "Game/SH/SHMainMenu.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feMusic.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/tlImageInstance.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x800AC57C | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*, Placeholder<0>>(
//     Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu* const&, const Placeholder<0>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AC560 | size: 0x1C
//  */
// void MemFun<SHMainMenu, void, TLComponentInstance*>(void (SHMainMenu::*)(TLComponentInstance*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AC504 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*, Placeholder<0>>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x3F0 | 0x800AC404 | size: 0x100
//  */
// void 0x800AC504..0x800AC560 | size : 0x5C
// {
// }

// /**
//  * Offset/Address/Size: 0x384 | 0x800AC398 | size: 0x6C
//  */
// void FEPopupMenu::Create(ePopupMenu)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AC014 | size: 0x4
//  */
// void FEPopupMenu::Nothing()
// {
// }

// /**
//  * Offset/Address/Size: 0x70 | 0x800ABFBC | size: 0x58
//  */
// void MenuItem<TLComponentInstance>::MenuItem()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800ABF4C | size: 0x70
//  */
// void MenuList<TLComponentInstance>::~MenuList()
// {
// }

// /**
//  * Offset/Address/Size: 0xD4 | 0x800ABF1C | size: 0x30
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
// (SHMainMenu::*)(TLComponentInstance*)>,
//                                                                  SHMainMenu*, Placeholder<0>>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x800ABE9C | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (SHMainMenu::*)(TLComponentInstance*)>, SHMainMenu*, Placeholder<0>>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800ABE48 | size: 0x48
//  */
// void Function1<void, TLComponentInstance*>::FunctorBase::~FunctorBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x704 | 0x800ABCEC | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                 unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800ABC68 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                              unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800ABC30 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800ABAD4 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800ABA50 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800ABA18 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800AB8BC | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800AB838 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800AB800 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800AB6A4 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800AB620 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AB5E8 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                  InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1B14 | 0x800AB570 | size: 0x78
 */
static void onSelectFriendly(TLComponentInstance*)
{
    GameInfoManager::Instance()->SetMode(GameInfoManager::GM_FRIENDLY);
    GameInfoManager::Instance()->SetTeam(0, (eTeamID)3);
    GameInfoManager::Instance()->SetTeam(1, (eTeamID)3);
    GameSceneManager::Instance()->PopEntireStack();
    GameInfoManager::Instance()->ResetPlayingSides();
    GameSceneManager::Instance()->Push(SCENE_CHOOSE_CAPTAINS, SCREEN_FORWARD, false);
}

/**
 * Offset/Address/Size: 0x1AD0 | 0x800AB52C | size: 0x44
 */
static void onSelectCup(TLComponentInstance*)
{
    GameSceneManager::s_pInstance->PopEntireStack();
    GameSceneManager::s_pInstance->Push(SCENE_CUP_CHOOSE_CUP, SCREEN_FORWARD, false);
}

/**
 * Offset/Address/Size: 0x1A50 | 0x800AB4AC | size: 0x80
 */
static void onSelectSuperCup(TLComponentInstance*)
{
    if (!GameInfoManager::Instance()->IsSuperCupModeUnlocked())
    {
        FEPopupMenu* menu = (FEPopupMenu*)GameSceneManager::Instance()->Push(SCENE_POPUP_MENU, SCREEN_FORWARD, false);
        menu->Create(POPUP_SUPER_CUPS_LOCKED);
    }
    else
    {
        GameSceneManager::Instance()->PopEntireStack();
        GameSceneManager::Instance()->Push(SCENE_SUPER_CUP_CHOOSE_CUP, SCREEN_FORWARD, false);
    }
}

/**
 * Offset/Address/Size: 0x19C4 | 0x800AB420 | size: 0x8C
 */
static void onSelect101(TLComponentInstance*)
{
    GameInfoManager::Instance()->SetMode(GameInfoManager::GM_FRIENDLY);
    GameInfoManager::Instance()->SetTeam(0, (eTeamID)3);
    GameInfoManager::Instance()->SetTeam(1, (eTeamID)3);
    GameSceneManager::Instance()->PopEntireStack();
    GameInfoManager::Instance()->ResetPlayingSides();
    GameSceneManager::Instance()->Push(SCENE_CHOOSE_CAPTAINS, SCREEN_FORWARD, false);
    GameInfoManager::Instance()->mIsInStrikers101Mode = true;
    FEMusic::StartStreamIfDifferent(5);
}

/**
 * Offset/Address/Size: 0x1964 | 0x800AB3C0 | size: 0x60
 */
static void newTourn()
{
    GameSceneManager::s_pInstance->PopEntireStack();
    GameSceneManager::s_pInstance->Push(SCENE_TOURN_SETPARAMS, SCREEN_FORWARD, false);
    if (GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cupConstructed)
    {
        GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cup->mCupStarted = false;
    }
}

/**
 * Offset/Address/Size: 0x1914 | 0x800AB370 | size: 0x50
 */
static void continueTourn()
{
    GameInfoManager::s_pInstance->SetMode(GameInfoManager::GM_TOURNAMENT);
    GameSceneManager::s_pInstance->PopEntireStack();
    GameSceneManager::s_pInstance->Push(SCENE_TOURNAMENT_STANDINGS, SCREEN_FORWARD, false);
}

/**
 * Offset/Address/Size: 0x182C | 0x800AB288 | size: 0xE8
 * TODO: 99.9% match - i diff on bl Create (branch offset, relocation)
 */
static void confirmNewTourn()
{
    FEPopupMenu* menu = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

    {
        Function<FnVoidVoid> yes;
        yes.mTag = FREE_FUNCTION;
        yes.mFreeFunction = newTourn;

        Function<FnVoidVoid> no;
        no.mTag = FREE_FUNCTION;
        no.mFreeFunction = FEPopupMenu::Nothing;

        menu->Create(POPUP_REALLY_OVERWRITE, yes, no);
    }
    *(u8*)((u8*)menu + 0xAA4) = 0;
}

/**
 * Offset/Address/Size: 0x1680 | 0x800AB0DC | size: 0x1AC
 * TODO: 92.1% match - Function0 default constructor dead stores not eliminated
 *       with -inline auto (decomp.me). File uses -inline deferred which eliminates them.
 */
static void onSelectTournament(TLComponentInstance*)
{
    if (GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cupConstructed
        && GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cup->mCupStarted)
    {
        FEPopupMenu* menu = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

        {
            Function<FnVoidVoid> yes;
            yes.mTag = FREE_FUNCTION;
            yes.mFreeFunction = continueTourn;

            Function<FnVoidVoid> no;
            no.mTag = FREE_FUNCTION;
            no.mFreeFunction = confirmNewTourn;

            menu->Create(POPUP_START_NEW_TOURNAMENT, yes, no);
        }

        {
            Function<FnVoidVoid> back;
            back.mTag = FREE_FUNCTION;
            back.mFreeFunction = FEPopupMenu::Nothing;

            menu->SetBackButtonCallback(back);
        }

        GameInfoManager::s_pInstance->SetMode(GameInfoManager::GM_TOURNAMENT);
    }
    else
    {
        GameSceneManager::s_pInstance->PopEntireStack();
        GameSceneManager::s_pInstance->Push(SCENE_TOURN_SETPARAMS, SCREEN_FORWARD, false);

        if (GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cupConstructed)
        {
            GameInfoManager::s_pInstance->mCustomTournamentInfo.m_cup->mCupStarted = false;
        }
    }
}

/**
 * Offset/Address/Size: 0x1634 | 0x800AB090 | size: 0x4C
 */
static void onSelectTrophies(TLComponentInstance*)
{
    GameSceneManager::s_pInstance->PopEntireStack();
    GameSceneManager::s_pInstance->Push(SCENE_TROPHY_ROOM, SCREEN_FORWARD, false);
    FEMusic::StartStreamIfDifferent(6);
}

/**
 * Offset/Address/Size: 0x15E8 | 0x800AB044 | size: 0x4C
 */
static void onSelectOptions(TLComponentInstance*)
{
    GameSceneManager::s_pInstance->PopEntireStack();
    GameSceneManager::s_pInstance->Push(SCENE_OPTIONS, SCREEN_FORWARD, false);
    FEMusic::StartStreamIfDifferent(7);
}

/**
 * Offset/Address/Size: 0x1538 | 0x800AAF94 | size: 0xB0
 */
SHMainMenu::SHMainMenu()
{
}

// /**
//  * Offset/Address/Size: 0x14D8 | 0x800AAF34 | size: 0x60
//  */
// void MenuItem<TLComponentInstance>::~MenuItem()
// {
// }

/**
 * Offset/Address/Size: 0x1380 | 0x800AADDC | size: 0x158
 */
SHMainMenu::~SHMainMenu()
{
}

/**
 * Offset/Address/Size: 0xA3C | 0x800AA498 | size: 0x8C0
 */
void SHMainMenu::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x60C | 0x800AA068 | size: 0x430
 */
void SHMainMenu::OpenItem(TLComponentInstance*)
{
}

/**
 * Offset/Address/Size: 0x404 | 0x800A9E60 | size: 0x208
 */
void SHMainMenu::CloseItem(TLComponentInstance* compinstance)
{
    typedef TLComponentInstance* (*FindComponentByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindComponentByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    unsigned long hash;
    volatile InlineHasher hC, hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    compinstance->SetActiveSlide("out");
    compinstance->Update(0.0f);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("high");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    union
    {
        FindComponentByValue byValue;
        FindComponentByRef byRef;
    } fc1;
    fc1.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* highlight = fc1.byRef(
        compinstance->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    highlight->SetActiveSlide("out");
    highlight->Update(0.0f);

    volatile InlineHasher g7, g6, g5, g4, g3, g2, g1, g0;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("may_highlite");
    g6.m_Hash = hash;
    g7.m_Hash = hash;

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } fi1;
    fi1.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
    fi1.byRef(
           highlight->GetActiveSlide(),
           (InlineHasher&)g7,
           (InlineHasher&)h9,
           (InlineHasher&)h7,
           (InlineHasher&)h5,
           (InlineHasher&)h3,
           (InlineHasher&)h1)
        ->SetAssetColour(mHighlightColour);

    BaseSceneHandler* scene = nlSingleton<GameSceneManager>::s_pInstance->GetScene(SCENE_MARIO_BACKGROUND);
    if (scene->m_bVisible)
    {
        Audio::gWorldSFX.Stop((Audio::eWorldSFX)0xC, cGameSFX::SFX_STOP_FIRST);
        FEAudio::PlayAnimAudioEvent("sfx_main_menu_highlight_close", false);
    }

    volatile InlineHasher j7, j6, j5, j4, j3, j2, j1, j0;

    j0.m_Hash = 0;
    h1.m_Hash = 0;
    j1.m_Hash = 0;
    h3.m_Hash = 0;
    j2.m_Hash = 0;
    h5.m_Hash = 0;
    j3.m_Hash = 0;
    h7.m_Hash = 0;
    j4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("locked");
    j6.m_Hash = hash;
    j7.m_Hash = hash;

    union
    {
        FindComponentByValue byValue;
        FindComponentByRef byRef;
    } fc2;
    fc2.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    TLComponentInstance* lockedComp = fc2.byRef(
        compinstance->GetActiveSlide(),
        (InlineHasher&)j7,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (lockedComp != NULL)
    {
        if (mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mLocked)
        {
            lockedComp->m_bVisible = true;
        }
        else
        {
            lockedComp->m_bVisible = false;
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800A9A5C | size: 0x404
 */
void SHMainMenu::Update(float)
{
}
