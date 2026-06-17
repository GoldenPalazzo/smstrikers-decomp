#define BIND_NO_DECL
#define FUNCTION1_SPLIT_BODIES
#include "Game/SH/SHPause.h"
#include "Game/OverlayManager.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feManager.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/GameInfo.h"
#include "Game/Game.h"
#include "NL/glx/glxSwap.h"
#include "NL/nlPrint.h"

extern FEInput* g_pFEInput;
extern nlColour MenuHighliteColour;

eFEINPUT_PAD PauseMenuScene::mControllingInput = FE_ALL_PADS;
float mDelayBeforeUnpause__14PauseMenuScene = 0.1f;
s32 PauseMenuScene::mLastSelectedIndex;

namespace DoubleHighlite
{
void OpenItem(TLComponentInstance*);
}

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)())
{
    return Detail::MemFunImpl<R, void (T::*)()>(fn);
}

template <typename T, typename R, typename A>
Detail::MemFunImpl<R, void (T::*)(A)> MemFun(void (T::*fn)(A))
{
    return Detail::MemFunImpl<R, void (T::*)(A)>(fn);
}

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg)
{
    return BindExp1<R, F, A>(fn, arg);
}

template <typename R, typename F, typename A, typename B>
BindExp2<R, F, A, B> Bind(F fn, const A& t0, const B& t1)
{
    return BindExp2<R, F, A, B>(fn, t0, t1);
}

typedef Detail::MemFunImpl<void, void (PauseMenuScene::*)()> MemFunImpl_Pause_v_t;
typedef Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)> MemFunImpl_Pause_p_t;
typedef BindExp1<void, MemFunImpl_Pause_v_t, PauseMenuScene*> BindExp1_Pause_t;
typedef BindExp2<void, MemFunImpl_Pause_p_t, PauseMenuScene*, Placeholder<0> > BindExp2_Pause_t;

template <>
inline void Function0<void>::FunctorImpl<BindExp1_Pause_t>::operator()()
{
    FORCE_DONT_INLINE;
    (mBind.mArg->*mBind.mFuncPtr.mMemFun)();
}

template <>
inline Function0<void>::FunctorBase* Function0<void>::FunctorImpl<BindExp1_Pause_t>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}

template <>
inline void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2_Pause_t>::operator()(TLComponentInstance* arg)
{
    (mBind.mT0->*mBind.mFunction.mMemFun)(arg);
}

template <>
inline Function1<void, TLComponentInstance*>::FunctorBase* Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2_Pause_t>::Clone() const
{
    return new (nlMalloc(sizeof(FunctorImpl), 8, false)) FunctorImpl(mBind);
}

/**
 * Offset/Address/Size: 0x1C | 0x800B0160 | size: 0x1C
 * TODO: 69.29% match - r0/r5 register allocation swap and load/store interleaving
 */
template Detail::MemFunImpl<void, void (PauseMenuScene::*)()>
MemFun<PauseMenuScene, void>(void (PauseMenuScene::*)());

/**
 * Offset/Address/Size: 0x0 | 0x800B0144 | size: 0x1C
 * TODO: 69.29% match - r0/r5 register allocation swap and load/store interleaving
 */
template Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>
MemFun<PauseMenuScene, void, TLComponentInstance*>(void (PauseMenuScene::*)(TLComponentInstance*));

/**
 * Offset/Address/Size: 0x225C | 0x800AF754 | size: 0xDC
 */
PauseMenuScene::PauseMenuScene(PauseMenuScene::ScreenContext context)
    : BaseSceneHandler()
    , mContext(context)
    , mGameIsOver(false)
    , mQuitDelay(0.0f)
    , mQuittingController(FE_ALL_PADS)
    , mMenuItems()
    , mTransitionTo(TT_IN)
    , mIsInTransition(false)
    , mStartAnimAtEnd(false)
    , mButtons()
    , mButtons2()
{
    mDelayBeforeUnpause__14PauseMenuScene = 0.1f;
}

/**
 * Offset/Address/Size: 0x21AC | 0x800AF6A4 | size: 0xB0
 */
PauseMenuScene::~PauseMenuScene()
{
}

/**
 * Offset/Address/Size: 0x2158 | 0x800AF650 | size: 0x54
 */
void PauseMenuScene::OnSelectRESUME(TLComponentInstance*)
{
    TransitionOut(TT_OUT);
    g_pFEInput->Reset();
    FEAudio::PlayAnimAudioEvent("sfx_screen_back", false);
    FEAudio::PlayAnimAudioEvent("sfx_pause_resume", false);
    mLastSelectedIndex = 0;
}

/**
 * Offset/Address/Size: 0x18CC | 0x800AEDC4 | size: 0x88C
 */
void PauseMenuScene::OnSelectQUIT(TLComponentInstance*)
{
    FEPopupMenu* popup;

    if (FrontEnd::m_bGameOver)
    {
        OverlayManager::s_pInstance->Pop();
        OverlayManager::s_pInstance->Pop();
        OverlayManager::s_pInstance->Push(OVERLAY_BRAG, SCREEN_FORWARD, false);
    }
    else
    {
        popup = (FEPopupMenu*)OverlayManager::s_pInstance->Push(OVERLAY_POPUP, SCREEN_NOTHING, false);
        popup->mControlInput = mQuittingController;

        if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        {
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeYesMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> yes(Bind<void>(
                makeYesMemFun(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeNoMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> no(Bind<void>(
                makeNoMemFun(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
            popup->Create(POPUP_INGAME_QUIT_STRIKERS_101, yes, no);
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode == GameInfoManager::GM_FRIENDLY || g_pGame->m_eGameState == GS_END_GAME)
        {
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeYesMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> yes(Bind<void>(
                makeYesMemFun(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeNoMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> no(Bind<void>(
                makeNoMemFun(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
            popup->Create(POPUP_INGAME_QUIT_MATCH, yes, no);
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode()
                 || (nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode()
                     && nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)mQuittingController) != -1))
        {
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeYesMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> yes(Bind<void>(
                makeYesMemFun(&PauseMenuScene::OnSelectPopupYESFORFEIT), this));
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeNoMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> no(Bind<void>(
                makeNoMemFun(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
            popup->Create(POPUP_INGAME_FORFEIT_MATCH, yes, no);
        }
        else
        {
            Detail::MemFunImpl<void, void (PauseMenuScene::*)()> (*makeNoMemFun)(void (PauseMenuScene::*)()) = MemFun<PauseMenuScene, void>;
            Function<FnVoidVoid> no(Bind<void>(
                makeNoMemFun(&PauseMenuScene::OnSelectPopupNOFORFEIT), this));
            popup->Create(POPUP_NO_FORFEIT, no);
        }
    }
}

/**
 * Offset/Address/Size: 0x1890 | 0x800AED88 | size: 0x3C
 */
void PauseMenuScene::OnSelectCHOOSESIDES(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_CHOOSE_SIDES, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x1854 | 0x800AED4C | size: 0x3C
 */
void PauseMenuScene::OnSelectAUDIOOPTIONS(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_PAUSE_AUDIO, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x1818 | 0x800AED10 | size: 0x3C
 */
void PauseMenuScene::OnSelectVISUALOPTIONS(TLComponentInstance*)
{
    OverlayManager::s_pInstance->Push(IGSCENE_PAUSE_VISUAL, SCREEN_FORWARD, true);
}

/**
 * Offset/Address/Size: 0x17CC | 0x800AECC4 | size: 0x4C
 */
void PauseMenuScene::OnSelectSTATISTICS(TLComponentInstance*)
{
    SummaryOverlay* scene = (SummaryOverlay*)OverlayManager::s_pInstance->Push(OVERLAY_SUMMARY_PAUSE, SCREEN_FORWARD, true);
    scene->m_controllingInput = mControllingInput;
    scene->mButtonState = ButtonComponent::BS_B_ONLY;
}

/**
 * Offset/Address/Size: 0x17C8 | 0x800AECC0 | size: 0x4
 */
void PauseMenuScene::OnSelectPopupNOFORFEIT()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x1684 | 0x800AEB7C | size: 0x144
 */
void PauseMenuScene::OnSelectPopupYESFORFEIT()
{
    GameInfoManager* gim;
    s32 quittingSide;

    gim = nlSingleton<GameInfoManager>::s_pInstance;

    if (gim->mIsInStrikers101Mode)
    {
        mQuitDelay = 1.0f;
        return;
    }

    if (g_pGame->m_eGameState != GS_END_GAME)
    {
        gim = nlSingleton<GameInfoManager>::s_pInstance;
        quittingSide = -1;

        if (gim->IsInCupMode())
        {
            eTeamID userTeam = gim->GetUserSelectedCupTeam();
            if (userTeam == gim->GetTeam(0))
            {
                quittingSide = 0;
            }
            else if (userTeam == gim->GetTeam(1))
            {
                quittingSide = 1;
            }
        }
        else if (gim->IsInTournamentMode())
        {
            quittingSide = gim->GetPlayingSide(mQuittingController);
        }

        if (gim->IsInCupOrTournamentMode())
        {
            if (quittingSide == 0)
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackWinner(0);
                gim->SetResultsOfLastUserGame((eUserGameResult)0xD);
            }
            else if (quittingSide == 1)
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackWinner(1);
                gim->SetResultsOfLastUserGame((eUserGameResult)0xE);
            }
        }
    }

    mQuitDelay = 1.0f;
}

/**
 * Offset/Address/Size: 0x1640 | 0x800AEB38 | size: 0x44
 */
void PauseMenuScene::OnSelectLESSONS(TLComponentInstance*)
{
    BaseSceneHandler* scene = OverlayManager::s_pInstance->Push(IGSCENE_LESSON_SELECT, SCREEN_FORWARD, true);
    ((u8*)scene)[0x2b2] = 1;
}

/**
 * Offset/Address/Size: 0x84C | 0x800ADD44 | size: 0xDF4
 */
void PauseMenuScene::SceneCreated()
{
    extern bool g_e3_Build;

    typedef TLInstance* (*FindInstByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLInstance* (*FindInstByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompSlideByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompSlideByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompPresByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompPresByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindInstByValue byValue;
        FindInstByRef byRef;
    } findInst;

    union
    {
        FindCompSlideByValue byValue;
        FindCompSlideByRef byRef;
    } findCompSlide;

    union
    {
        FindCompPresByValue byValue;
        FindCompPresByRef byRef;
    } findCompPres;

    typedef Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)> PauseMemFun;
    typedef BindExp2<void, PauseMemFun, PauseMenuScene*, Placeholder<0> > PauseBind;
    typedef void MenuCBSig(TLComponentInstance*);
    typedef Function<MenuCBSig> MenuCallback;

    FEAudio::EnableSounds(false);

    switch (mContext)
    {
    case SC_REGULAR_PAUSE:
    {
        FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

        static void (PauseMenuScene::* PauseMenuCBs[6])(TLComponentInstance*);
        static signed char init;

        if (!init)
        {
            PauseMenuCBs[0] = &PauseMenuScene::OnSelectRESUME;
            PauseMenuCBs[1] = &PauseMenuScene::OnSelectCHOOSESIDES;
            PauseMenuCBs[2] = &PauseMenuScene::OnSelectAUDIOOPTIONS;
            PauseMenuCBs[3] = &PauseMenuScene::OnSelectVISUALOPTIONS;
            PauseMenuCBs[4] = &PauseMenuScene::OnSelectSTATISTICS;
            PauseMenuCBs[5] = &PauseMenuScene::OnSelectQUIT;
            init = 1;
        }

        static char* MENU_NAMES[6]
            = { "MENU ITEM1", "MENU ITEM2", "MENU ITEM3", "MENU ITEM6", "MENU ITEM4", "MENU ITEM5" };

        static const bool E3_BUILD_IS_DISABLED_OPTIONS[6] = { false, false, true, true, false, false };

        int i;
        for (i = 0; i < 6; i++)
        {
            volatile InlineHasher hB, hA;
            volatile InlineHasher h9, h8;
            volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(MENU_NAMES[i]);
            h9.m_Hash = hash;
            h8.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hB.m_Hash = hash;
            hA.m_Hash = hash;

            findInst.byValue = FEFinder<TLInstance, 4>::Find<TLSlide>;
            TLComponentInstance* compinstance = (TLComponentInstance*)findInst.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
            menuItem->mType = compinstance;
            mMenuItems.mNumItemsAdded++;

            void (PauseMenuScene::*openCB)(TLComponentInstance*) = &PauseMenuScene::OpenItem;
            PauseBind bindOpen = Bind<void>(MemFun<PauseMenuScene, void, TLComponentInstance*>(openCB), this, placeholder0);
            menuItem->mCallbacks[ON_HIGHLIGHT] = MenuCallback(bindOpen);

            {
                MenuCallback closeFunc;
                closeFunc.mTag = FREE_FUNCTION;
                closeFunc.mFreeFunction = DoubleHighlite::CloseItem;
                menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunc;
            }

            if (PauseMenuCBs[i])
            {
                PauseBind bindApply = Bind<void>(MemFun<PauseMenuScene, void, TLComponentInstance*>(PauseMenuCBs[i]), this, placeholder0);
                menuItem->mCallbacks[ON_APPLY] = MenuCallback(bindApply);
            }

            TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
            (void)highlite;

            if (i == mLastSelectedIndex)
            {
                menuItem->mCallbacks[ON_HIGHLIGHT](menuItem->mType);
            }
            else
            {
                menuItem->mCallbacks[ON_UNHIGHLIGHT](menuItem->mType);

                TLSlide* slide = compinstance->GetActiveSlide();
                compinstance->Update(1.0f + (slide->m_start + slide->m_duration));
            }

            if (g_e3_Build)
            {
                menuItem->mDisabled = E3_BUILD_IS_DISABLED_OPTIONS[i];
            }
        }

        mMenuItems.mFlags = 1;
        mMenuItems.mCurrentIndex = mLastSelectedIndex;
        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT](mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType);
        break;
    }
    case SC_101_PAUSE:
    {
        FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

        static void (PauseMenuScene::* PauseMenuCBs[3])(TLComponentInstance*);
        static signed char init;

        if (!init)
        {
            PauseMenuCBs[0] = &PauseMenuScene::OnSelectLESSONS;
            PauseMenuCBs[1] = &PauseMenuScene::OnSelectRESUME;
            PauseMenuCBs[2] = &PauseMenuScene::OnSelectQUIT;
            init = 1;
        }

        static char* MENU_NAMES[3] = { "MENU ITEM1", "MENU ITEM2", "MENU ITEM3" };

        int i;
        for (i = 0; i < 3; i++)
        {
            volatile InlineHasher hB, hA;
            volatile InlineHasher h9, h8;
            volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(MENU_NAMES[i]);
            h9.m_Hash = hash;
            h8.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hB.m_Hash = hash;
            hA.m_Hash = hash;

            findInst.byValue = FEFinder<TLInstance, 4>::Find<TLSlide>;
            TLComponentInstance* compinstance = (TLComponentInstance*)findInst.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
            menuItem->mType = compinstance;
            mMenuItems.mNumItemsAdded++;

            {
                MenuCallback openFunc;
                openFunc.mTag = FREE_FUNCTION;
                openFunc.mFreeFunction = DoubleHighlite::OpenItem;
                menuItem->mCallbacks[ON_HIGHLIGHT] = openFunc;
            }

            {
                MenuCallback closeFunc;
                closeFunc.mTag = FREE_FUNCTION;
                closeFunc.mFreeFunction = DoubleHighlite::CloseItem;
                menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunc;
            }

            if (PauseMenuCBs[i])
            {
                PauseBind bindApply = Bind<void>(MemFun<PauseMenuScene, void, TLComponentInstance*>(PauseMenuCBs[i]), this, placeholder0);
                menuItem->mCallbacks[ON_APPLY] = MenuCallback(bindApply);
            }

            TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
            (void)highlite;

            if (i == mLastSelectedIndex)
            {
                menuItem->mCallbacks[ON_HIGHLIGHT](menuItem->mType);
            }
            else
            {
                menuItem->mCallbacks[ON_UNHIGHLIGHT](menuItem->mType);

                TLSlide* slide = compinstance->GetActiveSlide();
                compinstance->Update(1.0f + (slide->m_start + slide->m_duration));
            }
        }

        mMenuItems.mFlags = 1;
        mMenuItems.mCurrentIndex = mLastSelectedIndex;
        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT](mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType);
        break;
    }
    default:
        break;
    }

    volatile InlineHasher bB, bA;
    volatile InlineHasher b9, b8;
    volatile InlineHasher b7, b6, b5, b4, b3, b2, b1, b0;

    b0.m_Hash = 0;
    b1.m_Hash = 0;
    b2.m_Hash = 0;
    b3.m_Hash = 0;
    b4.m_Hash = 0;
    b5.m_Hash = 0;
    b6.m_Hash = 0;
    b7.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("buttons");
    b9.m_Hash = hash;
    b8.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    bB.m_Hash = hash;
    bA.m_Hash = hash;

    findCompSlide.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
    mButtons.mButtonInstance = findCompSlide.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)bB,
        (InlineHasher&)b9,
        (InlineHasher&)b7,
        (InlineHasher&)b5,
        (InlineHasher&)b3,
        (InlineHasher&)b1);

    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    volatile InlineHasher cB, cA;
    volatile InlineHasher c9, c8;
    volatile InlineHasher c7, c6, c5, c4, c3, c2, c1, c0;

    c0.m_Hash = 0;
    c1.m_Hash = 0;
    c2.m_Hash = 0;
    c3.m_Hash = 0;
    c4.m_Hash = 0;
    c5.m_Hash = 0;

    hash = nlStringLowerHash("buttons");
    c5.m_Hash = hash;
    c4.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    c7.m_Hash = hash;
    c6.m_Hash = hash;

    hash = nlStringLowerHash("menu in2");
    c9.m_Hash = hash;
    c8.m_Hash = hash;

    findCompPres.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;
    mButtons2.mButtonInstance = findCompPres.byRef(
        m_pFEPresentation,
        (InlineHasher&)c9,
        (InlineHasher&)c7,
        (InlineHasher&)c5,
        (InlineHasher&)c3,
        (InlineHasher&)c1,
        (InlineHasher&)b1);

    mButtons2.SetState(ButtonComponent::BS_A_AND_B);

    EnableAutoPressed();
    FEAudio::EnableSounds(true);
}

/**
 * Offset/Address/Size: 0x268 | 0x800AD760 | size: 0x5E4
 * TODO: 96.19% match - this/temp register swap and final resume-call sequence still differ
 */
void PauseMenuScene::Update(float fDeltaT)
{
    if (mQuitDelay > 0.0f)
    {
        mQuitDelay = mQuitDelay - fDeltaT;
        if (!nlSingleton<OverlayManager>::s_pInstance->IsOnStack(OVERLAY_POPUP))
        {
            glxSwapSetBlack(true);
        }
        if (mQuitDelay <= 0.0f)
        {
            mQuitDelay = 0.0f;
            FrontEnd::ReturnToFE();
        }
        return;
    }

    if (mStartAnimAtEnd)
    {
        if (m_pFEPresentation->m_currentSlide != NULL)
        {
            m_pFEPresentation->m_fadeDuration = 999.9f;
            mStartAnimAtEnd = false;
        }
    }

    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mButtons2.CentreButtons();

    if (mIsInTransition)
    {
        TLSlide* slide = m_pFEPresentation->m_currentSlide;
        f32 currentTime = slide->m_time;
        f32 endTime = slide->m_start + slide->m_duration;
        if (!(currentTime >= endTime))
            return;

        switch (mTransitionTo)
        {
        case TT_OUT:
            FrontEnd::ExitMenuState();
            break;
        default:
            break;
        }
        mIsInTransition = false;
        mTransitionTo = (TransitionType)0;
        return;
    }

    register u8* connState;
    register u8 goToChooseSides = 0;
    register int i = 0;
    connState = &FrontEnd::m_ctrlConnectedState[0];

    for (; i < 4; i++)
    {
        bool isConn = g_pFEInput->IsConnected((eFEINPUT_PAD)i);

        if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            if (nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i) != -1)
            {
                if (!goToChooseSides)
                {
                    OverlayManager* om = nlSingleton<OverlayManager>::s_pInstance;
                    while (((om = nlSingleton<OverlayManager>::s_pInstance)->mCurrentStackDepth != 0 ? om->mBaseSceneHandlerStack[om->mCurrentStackDepth - 1] : NULL) != (BaseSceneHandler*)this)
                    {
                        om->Pop();
                        nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();
                    }
                    om->Push(IGSCENE_CHOOSE_SIDES, SCREEN_FORWARD, true);
                }
                goToChooseSides = 1;
            }
        }

        *connState = isConn;
        connState++;
    }

    if (goToChooseSides)
        return;

    mDelayBeforeUnpause__14PauseMenuScene = mDelayBeforeUnpause__14PauseMenuScene - fDeltaT;
    if (mDelayBeforeUnpause__14PauseMenuScene > 0.0f)
        return;

    mDelayBeforeUnpause__14PauseMenuScene = 0.0f;

    if (m_pFEPresentation->m_currentSlide == NULL)
        return;

    if (g_pFEInput->IsAutoPressed(mControllingInput, 0xd, true, NULL))
    {
        int currentIndex = mMenuItems.mCurrentIndex;
        int wrapBit = mMenuItems.mFlags & 1;
        int skipDisabled = mMenuItems.mFlags & 2;
        int newIndex = currentIndex - 1;

        while (true)
        {
            if (wrapBit)
            {
                if (newIndex < 0)
                    newIndex = mMenuItems.mNumItemsAdded - 1;
            }
            else
            {
                if (newIndex < 0)
                    return;
            }

            if (!skipDisabled)
                break;

            if (!mMenuItems.mMenuItems[newIndex].mDisabled)
                break;

            newIndex = newIndex - 1;
        }

        mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT](mMenuItems.mMenuItems[currentIndex].mType);

        mMenuItems.mCurrentIndex = newIndex;
        int idx = mMenuItems.mCurrentIndex;
        mMenuItems.mMenuItems[idx].mCallbacks[ON_HIGHLIGHT](mMenuItems.mMenuItems[idx].mType);
        return;
    }

    if (g_pFEInput->IsAutoPressed(mControllingInput, 0xe, true, NULL))
    {
        int currentIndex = mMenuItems.mCurrentIndex;
        int wrapBit = mMenuItems.mFlags & 1;
        int skipDisabled = mMenuItems.mFlags & 2;
        int newIndex = currentIndex + 1;

        while (true)
        {
            if (wrapBit)
            {
                int numItems = mMenuItems.mNumItemsAdded;
                newIndex = newIndex % numItems;
            }
            else
            {
                if (newIndex >= mMenuItems.mNumItemsAdded)
                    return;
            }

            if (!skipDisabled)
                break;

            if (!mMenuItems.mMenuItems[newIndex].mDisabled)
                break;

            newIndex = newIndex + 1;
        }

        mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT](mMenuItems.mMenuItems[currentIndex].mType);

        mMenuItems.mCurrentIndex = newIndex;
        int idx = mMenuItems.mCurrentIndex;
        mMenuItems.mMenuItems[idx].mCallbacks[ON_HIGHLIGHT](mMenuItems.mMenuItems[idx].mType);
        return;
    }

    if (g_pFEInput->JustPressed(mControllingInput, 0x100, false, &mQuittingController))
    {
        int selectedIndex = mMenuItems.mCurrentIndex;
        MenuResult result;

        int tag = mMenuItems.mMenuItems[selectedIndex].mCallbacks[ON_APPLY].mTag;
        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            if (mMenuItems.mMenuItems[selectedIndex].mDisabled)
            {
                result = RES_ITEM_DISABLED;
            }
            else
            {
                if (tag == FREE_FUNCTION)
                    mMenuItems.mMenuItems[selectedIndex].mCallbacks[ON_APPLY].mFreeFunction(mMenuItems.mMenuItems[selectedIndex].mType);
                else
                    (*mMenuItems.mMenuItems[selectedIndex].mCallbacks[ON_APPLY].mFunctor)(mMenuItems.mMenuItems[selectedIndex].mType);
                result = RES_OK;
            }
        }
        else
        {
            result = RES_NO_CALLBACK_FUNC;
        }

        switch (result)
        {
        case RES_OK:
            mLastSelectedIndex = mMenuItems.mCurrentIndex;
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            break;
        case RES_ITEM_DISABLED:
            FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            break;
        default:
            break;
        }
        return;
    }

    if (!g_pFEInput->JustPressed(mControllingInput, 0x200, false, NULL))
    {
        if (!g_pFEInput->JustPressed(mControllingInput, 0x1000, false, NULL))
            return;
        if (FrontEnd::m_bGameOver)
            return;
    }

    if (!FrontEnd::m_bGameOver)
    {
        (this->*(&PauseMenuScene::OnSelectRESUME))(NULL);
    }
    else
    {
        FrontEnd::ExitMenuState();
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
    }
}

/**
 * Offset/Address/Size: 0xD0 | 0x800AD5C8 | size: 0x198
 */
void PauseMenuScene::TransitionOut(PauseMenuScene::TransitionType newtype)
{
    mIsInTransition = true;
    mTransitionTo = newtype;

    if (mTransitionTo == TT_OUT)
    {
        FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
        presentation->SetActiveSlide("menu in2");
        presentation->Update(0.0f);

        int i;
        for (i = 0; i < mMenuItems.mNumItemsAdded; i++)
        {
            typedef TLInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
            typedef TLInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
            union
            {
                FindByValue byValue;
                FindByRef byRef;
            } find;
            find.byValue = FEFinder<TLInstance, 4>::Find<TLSlide>;

            char menuname[64];
            nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

            volatile InlineHasher hB, hA;
            volatile InlineHasher h9, h8;
            volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(menuname);
            h8.m_Hash = hash;
            h9.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hB.m_Hash = hash;
            hA.m_Hash = hash;

            TLInstance* instance = find.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

            TLComponentInstance* compinstance = (TLComponentInstance*)instance;

            if (i == mMenuItems.mCurrentIndex)
            {
                compinstance->SetActiveSlide("out");
                compinstance->Update(0.0f);

                TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
                highlite->SetActiveSlide("out");
                highlite->Update(0.0f);
                highlite->SetAssetColour(MenuHighliteColour);
            }
            else
            {
                compinstance->SetActiveSlide("out");
                compinstance->Update(0.0f);

                TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(compinstance->GetActiveSlide(), "highlite");
                highlite->m_bVisible = false;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800AD4F8 | size: 0xD0
 */
void PauseMenuScene::OpenItem(TLComponentInstance* instance)
{
    DoubleHighlite::OpenItem(instance);

    if (mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mDisabled)
    {
        TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("disabled_text")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        text->m_LocStrId = 0x38202C30;
        text->m_OverloadFlags |= 0x8;
    }
}
