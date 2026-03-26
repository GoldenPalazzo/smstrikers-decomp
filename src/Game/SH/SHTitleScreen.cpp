#include "Game/SH/SHTitleScreen.h"
#include "Game/GameSceneManager.h"
#include "Game/GameInfo.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feMusic.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/SH/SHLoading.h"
#include "Game/SH/SHMainMenu.h"
#include "Game/main.h"

extern eStadiumID PickStadium__15GameInfoManagerCFb10eStadiumID(const GameInfoManager*, bool, eStadiumID);

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800AD39C | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800AD318 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800AD2E0 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800AD184 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800AD100 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AD0C8 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800AD05C | size: 0x6C
//  */
// void 0x800AD0C8..0x800AD4F8 | size : 0x430
// {
// }

/**
 * Offset/Address/Size: 0x9F8 | 0x800ACFB4 | size: 0xA8
 */
void StartMovieCB()
{
    BaseSceneHandler* handler;
    u32 sceneCount;

    if (GameSceneManager::Instance() != NULL)
    {
        sceneCount = GameSceneManager::Instance()->mCurrentStackDepth;
        if (sceneCount != 0)
        {
            handler = GameSceneManager::Instance()->mBaseSceneHandlerStack[sceneCount];
        }
        else
        {
            handler = NULL;
        }
        if ((handler != NULL) && (GameSceneManager::Instance()->GetSceneType(handler) == 2)
            && (handler->m_pFEScene->m_bValid != false))
        {
            GameSceneManager::Instance()->PopEntireStack();
            FESceneManager::Instance()->ForceImmediateStackProcessing();
            GameSceneManager::Instance()->Push((SceneList)0x35, SCREEN_NOTHING, false);
        }
    }
}

/**
 * Offset/Address/Size: 0x9F4 | 0x800ACFB0 | size: 0x4
 */
void DoNothingCallback()
{
}

/**
 * Offset/Address/Size: 0x7F0 | 0x800ACDAC | size: 0x204
 */
TitleScene::TitleScene()
{
}

/**
 * Offset/Address/Size: 0x794 | 0x800ACD50 | size: 0x5C
 */
TitleScene::~TitleScene()
{
}

// static const char sPressStart[] __attribute__((section(".sdata2"))) = "pressstart";
// static const char sPressStartText[] __attribute__((section(".sdata2"))) = "PressStart";

/**
 * Offset/Address/Size: 0x688 | 0x800ACC44 | size: 0x10C
 */
void TitleScene::SceneCreated()
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

    FEMusic::StopStream();
    AudioLoader::PlayFETitleMusicWithFade();

    volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    u32 hash = nlStringLowerHash("title_screen_fe");
    h8.m_Hash = hash;
    h9.m_Hash = hash;
    hash = nlStringLowerHash("Layer2");
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    TLComponentInstance* comp = findComp.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    volatile InlineHasher g7, g6, g5, g4, g3, g2, g1, g0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    g5.m_Hash = 0;
    hash = nlStringLowerHash("Text");
    g6.m_Hash = hash;
    g7.m_Hash = hash;

    mTextPressStart = findText.byRef(
        comp->GetActiveSlide(),
        (InlineHasher&)g7,
        (InlineHasher&)g5,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);
}

/**
 * Offset/Address/Size: 0x254 | 0x800AC810 | size: 0x434
 * TODO: 99.89% match - +0x14 instruction address offset in diff listing and
 * branch target relocation around demo timeout path; remaining differences are
 * non-functional codegen placement.
 */
void TitleScene::Update(float dt)
{
    BaseSceneHandler::Update(dt);

    m_fTimeElapsed += dt;
    if (m_fTimeElapsed < 1.0f)
    {
        mTextPressStart->m_bVisible = false;
        return;
    }

    mTextPressStart->m_bVisible = true;
    if (mStartedDemo)
    {
        mTextPressStart->m_bVisible = false;
        return;
    }

    float demoTimeout = GetConfigFloat(Config::Global(), "fe_demo_mode_time_out", 60.0f);

    if (!mStartedDemo && m_fTimeElapsed >= demoTimeout)
    {
        if (nlSingleton<GameInfoManager>::s_pInstance->mDemoEnabled)
        {
            bool doSoak = GetConfigBool(Config::Global(), "dosoak", false);

            if (doSoak)
            {
                eSidekickID awaySidekick;
                eSidekickID homeSidekick;
                eTeamID awayTeam;
                eTeamID homeTeam;
                GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;

                gim->SetMode(GameInfoManager::GM_DEMO);

                homeTeam = (eTeamID)nlRandom(8, &nlDefaultSeed);
                awayTeam = homeTeam;
                while (homeTeam == awayTeam)
                {
                    awayTeam = (eTeamID)nlRandom(8, &nlDefaultSeed);
                }

                if (g_e3_Build)
                {
                    do
                    {
                        homeTeam = (eTeamID)nlRandom(8, &nlDefaultSeed);
                    } while (homeTeam != TEAM_MARIO && homeTeam != TEAM_DONKEYKONG && homeTeam != TEAM_PEACH && homeTeam != TEAM_WARIO);

                    while (true)
                    {
                        eTeamID randomTeam = (eTeamID)nlRandom(8, &nlDefaultSeed);

                        if (randomTeam != TEAM_MARIO && randomTeam != TEAM_DONKEYKONG && randomTeam != TEAM_PEACH && randomTeam != TEAM_WARIO)
                        {
                            continue;
                        }

                        if ((homeTeam != TEAM_INVALID) && (homeTeam == randomTeam))
                        {
                            continue;
                        }

                        awayTeam = randomTeam;
                        break;
                    }
                }

                homeSidekick = (eSidekickID)nlRandom(4, &nlDefaultSeed);
                awaySidekick = homeSidekick;
                while (homeSidekick == awaySidekick)
                {
                    awaySidekick = (eSidekickID)nlRandom(4, &nlDefaultSeed);
                }

                if (g_e3_Build)
                {
                    homeSidekick = SK_TOAD;
                    awaySidekick = SK_KOOPA;
                }

                gim->SetStadium(PickStadium__15GameInfoManagerCFb10eStadiumID(gim, false, STAD_INVALID));
                gim->SetTeam(0, homeTeam);
                gim->SetTeam(1, awayTeam);
                gim->SetSidekick(0, homeSidekick);
                gim->SetSidekick(1, awaySidekick);
                gim->ResetPlayingSides();

                SuperLoadingScene* scene = (SuperLoadingScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SUPER_LOADING, SCREEN_NOTHING, true);
                scene->mType = SuperLoadingScene::TT_IN;
            }
            else
            {
                ((void (*)(TitleScene*))StartIntroMovie)(this);
            }
        }

        m_fTimeElapsed = 0.0f;
        mStartedDemo = true;
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x24, true, NULL)
        || g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_FORWARD, true);

        SHMainMenu::mSnapMenuIntoPosition = false;
        SHMainMenu::mLastMenuItem = 0;

        nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mGameplayOptions.OnSettingsUpdated();
        nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mPowerupOptions.OnSettingsUpdated();
        nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mCheatOptions.OnSettingsUpdated();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800AC5BC | size: 0x254
 * TODO: 99.29% match - sp+0x08/0x10 stack offset swap for Function0<void>
 * copy and Function<FnVoidVoid> temporaries. MWCC internal temp allocation
 * order difference.
 */
void TitleScene::StartIntroMovie()
{
    AudioStreamTrack::TrackManagerBase* trackMgr = g_pTrackManager;
    AudioStreamTrack::StreamTrack* track = trackMgr->GetTrack(nlStringLowerHash("music"));

    if (track != NULL)
    {
        track->Stop(1000);

        if (Audio::MasterVolume::GetVolume(Audio::MasterVolume::VG_Music) == 0.0f)
        {
            BaseSceneHandler* handler;
            u32 sceneCount;

            if (GameSceneManager::Instance() == NULL)
            {
                return;
            }

            sceneCount = GameSceneManager::Instance()->mCurrentStackDepth;
            if (sceneCount != 0)
            {
                handler = GameSceneManager::Instance()->mBaseSceneHandlerStack[sceneCount - 1];
            }
            else
            {
                handler = NULL;
            }

            if ((handler != NULL) && (GameSceneManager::Instance()->GetSceneType(handler) == 2)
                && (handler->m_pFEScene->m_bValid != false))
            {
                GameSceneManager::Instance()->PopEntireStack();
                FESceneManager::Instance()->ForceImmediateStackProcessing();
                GameSceneManager::Instance()->Push((SceneList)0x35, SCREEN_NOTHING, false);
            }
        }
        else
        {
            Function0<void> f0;

            f0.mTag = FREE_FUNCTION;
            f0.mFreeFunction = StartMovieCB;
            track->m_IdleCallback = Function<FnVoidVoid>(f0);
        }
    }
}
