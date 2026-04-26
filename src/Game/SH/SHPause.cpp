#include "Game/SH/SHPause.h"
#include "Game/OverlayManager.h"

#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/GameInfo.h"
#include "Game/Game.h"
#include "NL/nlPrint.h"

extern FEInput* g_pFEInput;
extern nlColour MenuHighliteColour;
extern float mDelayBeforeUnpause__14PauseMenuScene;

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

// /**
//  * Offset/Address/Size: 0x38 | 0x800B01B4 | size: 0x40
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>(
//     Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene* const&, const Placeholder<0>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B017C | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>(Detail::MemFunImpl<void, void
// (PauseMenuScene::*)()>,
//                                                                                        PauseMenuScene* const&)
// {
// }

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

// /**
//  * Offset/Address/Size: 0x5C | 0x800B00E8 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2<
//     void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800B008C | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x128 | 0x800B005C | size: 0x30
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::operator()()
// {
// }

// /**
//  * Offset/Address/Size: 0xB0 | 0x800AFFE4 | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)()>, PauseMenuScene*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x800AFFB4 | size: 0x30
//  */
// void Function1<void,
//                TLComponentInstance*>::FunctorImpl<BindExp2<void, Detail::MemFunImpl<void, void
//                (PauseMenuScene::*)(TLComponentInstance*)>,
//                                                            PauseMenuScene*, Placeholder<0>>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AFF34 | size: 0x80
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp2<void, Detail::MemFunImpl<void, void (PauseMenuScene::*)(TLComponentInstance*)>, PauseMenuScene*, Placeholder<0>>>::Clone()
//     const
// {
// }

// /**
//  * Offset/Address/Size: 0x6CC | 0x800AFEFC | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x570 | 0x800AFDA0 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800AFD1C | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800AFC98 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                              unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800AFC60 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                             InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800AFB04 | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                 unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800AFA80 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                              unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800AFA48 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800AF8EC | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800AF868 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AF830 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

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
 * Todo: Figure out how to what scene this is connecting...
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
}

/**
 * Offset/Address/Size: 0x268 | 0x800AD760 | size: 0x5E4
 */
void PauseMenuScene::Update(float)
{
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
