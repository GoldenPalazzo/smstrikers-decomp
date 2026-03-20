#include "Game/SH/SHCupCheater.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feInput.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x800E9970 | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>(Detail::MemFunImpl<void, void
// (CupCheaterScene::*)()>, CupCheaterScene* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E9954 | size: 0x1C
//  */
// void MemFun<CupCheaterScene, void>(void (CupCheaterScene::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E98F8 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>,
// CupCheaterScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x78 | 0x800E98C8 | size: 0x30
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>>::operator()()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E9850 | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupCheaterScene::*)()>, CupCheaterScene*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0x358 | 0x800E96F4 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800E9670 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800E95EC | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800E95B4 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800E9458 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800E93D4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E939C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1C00 | 0x800E9350 | size: 0x4C
 */
CupCheaterScene::CupCheaterScene()
    : BaseSceneHandler()
{
    mSniper = 0;
    mStriker = 0;
    mTactician = 0;
    mParamedic = 0;
    mVeteran = 0;
}

/**
 * Offset/Address/Size: 0x1B80 | 0x800E92D0 | size: 0x80
 */
CupCheaterScene::~CupCheaterScene()
{
    if (m_SlideMenu != NULL)
    {
        delete m_SlideMenu;
    }
}

/**
 * Offset/Address/Size: 0x14D4 | 0x800E8C24 | size: 0x6AC
 */
void CupCheaterScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x1140 | 0x800E8890 | size: 0x394
 */
void CupCheaterScene::Update(float dt)
{
    BaseSceneHandler::Update(dt);

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        if (m_SlideMenu->ApplyFunction())
        {
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        if (m_SlideMenu->PrevItem())
        {
            FEAudio::PlayAnimAudioEvent("sfx_menu_move_up", false);
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        if (m_SlideMenu->NextItem())
        {
            FEAudio::PlayAnimAudioEvent("sfx_menu_move_down", false);
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xB, true, NULL))
    {
        switch (m_SlideMenu->m_currentSlide)
        {
        case 5:
            mSniper--;
            mSniper &= ~(mSniper >> 31);
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
            UpdateSlides();
            break;
        case 6:
            mStriker--;
            mStriker &= ~(mStriker >> 31);
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
            UpdateSlides();
            break;
        case 7:
            mTactician--;
            mTactician &= ~(mTactician >> 31);
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
            UpdateSlides();
            break;
        case 8:
            mParamedic--;
            mParamedic &= ~(mParamedic >> 31);
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
            UpdateSlides();
            break;
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xC, true, NULL))
    {
        switch (m_SlideMenu->m_currentSlide)
        {
        case 5:
            mSniper++;
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
            UpdateSlides();
            break;
        case 6:
            mStriker++;
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
            UpdateSlides();
            break;
        case 7:
            mTactician++;
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
            UpdateSlides();
            break;
        case 8:
            mParamedic++;
            FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
            UpdateSlides();
            break;
        }
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x40, false, NULL))
    {
        mVeteran--;
        mVeteran &= ~(mVeteran >> 31);
        UpdateSlides();
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x20, false, NULL))
    {
        mVeteran++;
        UpdateSlides();
    }
}

/**
 * Offset/Address/Size: 0x10FC | 0x800E884C | size: 0x44
 */
void CupCheaterScene::OnSelectGameplay()
{
    // TODO: Check if this is correct, could also be another interface
    CupCheaterScene* scene = (CupCheaterScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SUPER_LOADING, SCREEN_NOTHING, true);
    scene->m_SlideMenu = NULL;
}

enum ePlayerStats
{
    STATS_GOALS_FOR = 1,
    STATS_WIN = 6,
    STATS_OT_WIN = 7,
    STATS_GAMES_PLAYED = 16,
    STATS_HITS_MADE = 19,
    STATS_STS_ATTEMPTS = 21,
    STATS_PERFECT_PASSES = 22,
};

struct BasicGameInfo
{
    int mTeamIndex[2];
};

class StatsTracker
{
public:
    void SimulateRemainingGames();
    void SetBasicGameInfoPointer(BasicGameInfo*, bool);
    void TrackStat(ePlayerStats, int, int, int, int, int, int);
    void AddStat(ePlayerStats, int, int, int);
    void AddMilestoneUserStat(ePlayerStats, int);
    void CompileEndOfGameStats();
};

class GameInfoManager
{
public:
    enum eGameModes
    {
        GM_INVALID = -1,
        GM_MUSHROOM_CUP = 1,
        GM_SUPER_BOWSER_CUP = 8,
    };

    bool IsInCupMode() const;
    void OnPreCupGameState();
    void OnPostCupGameState();
    int GetUserSelectedCupTeam() const;
    bool DetermineNextMatchups(int);
    s16 GetCurrentRoundNumber() const;
    void IncreaseRoundNumber();
    void IncreaseGameNumber(bool);
    void SetRoundResult(bool, int);

    char _pad0[0x10];
    BasicGameInfo* mGameInfo[11];
    char _pad1[0x4954 - 0x3C];
    eGameModes mCurrentMode;
};

class CupHubScene : public BaseSceneHandler
{
public:
    char _pad0[0x226 - 0x1C];
    bool mDoAutoSave;
};

extern unsigned int nlDefaultSeed;
extern unsigned int nlRandom(unsigned int, unsigned int*);

/**
 * Offset/Address/Size: 0xE90 | 0x800E85E0 | size: 0x26C
 * TODO: 99.71% match - r29/r31 register coalescing for gameInfoManager, r28 load scheduling for gameInfoManagerCup
 */
void CupCheaterScene::OnSelectHomeWin()
{
    CupCheaterScene* const self = this;
    bool homeAway;
    GameInfoManager* gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    GameInfoManager* gameInfoManagerPost;
    GameInfoManager* gameInfoManagerCup;
    BasicGameInfo* gameInfo;
    CupHubScene* hubScene;

    if (gameInfoManager->mCurrentMode >= GameInfoManager::GM_MUSHROOM_CUP
        && gameInfoManager->mCurrentMode <= GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
    {
        nlSingleton<GameInfoManager>::s_pInstance->OnPreCupGameState();
    }

    gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    nlSingleton<StatsTracker>::s_pInstance->SetBasicGameInfoPointer(gameInfoManager->mGameInfo[gameInfoManager->mCurrentMode], true);

    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 0, nlRandom(4, &nlDefaultSeed), -1, 0, 1, 0);
    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_WIN, 0, 0, 1, 0, 0, 0);

    gameInfoManager->SetRoundResult(false, 0);
    gameInfoManagerPost = nlSingleton<GameInfoManager>::s_pInstance;

    nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

    gameInfoManagerCup = nlSingleton<GameInfoManager>::s_pInstance;
    gameInfo = gameInfoManagerCup->mGameInfo[gameInfoManagerCup->mCurrentMode];

    if (gameInfoManagerCup->IsInCupMode())
    {
        int team0 = gameInfo->mTeamIndex[0];
        homeAway = team0 != gameInfoManagerCup->GetUserSelectedCupTeam();

        if (self->mSniper > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_GOALS_FOR, homeAway, 0, self->mSniper);
        }

        if (self->mStriker > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_STS_ATTEMPTS, homeAway, 0, self->mStriker);
        }

        if (self->mTactician > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_PERFECT_PASSES, homeAway, 0, self->mTactician);
        }

        if (self->mParamedic > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_HITS_MADE, homeAway, 0, self->mParamedic);
        }

        if (self->mVeteran > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddMilestoneUserStat(STATS_GAMES_PLAYED, self->mVeteran);
        }
    }

    nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();

    if (gameInfoManagerPost->IsInCupMode())
    {
        gameInfoManagerPost->OnPostCupGameState();
    }
    else
    {
        gameInfoManagerPost->IncreaseGameNumber(true);

        while ((s16)gameInfoManagerPost->GetCurrentRoundNumber() != -5)
        {
            if (gameInfoManagerPost->DetermineNextMatchups(27))
            {
                break;
            }

            gameInfoManagerPost->IncreaseRoundNumber();
        }

        hubScene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push((SceneList)0x18, SCREEN_NOTHING, false);
        hubScene->mDoAutoSave = true;
    }
}

/**
 * Offset/Address/Size: 0xC24 | 0x800E8374 | size: 0x26C
 * TODO: 99.71% match - r29/r31 register coalescing for gameInfoManager, r28 load scheduling for gameInfoManagerCup
 */
void CupCheaterScene::OnSelectAwayWin()
{
    CupCheaterScene* const self = this;
    bool homeAway;
    GameInfoManager* gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    GameInfoManager* gameInfoManagerPost;
    GameInfoManager* gameInfoManagerCup;
    BasicGameInfo* gameInfo;
    CupHubScene* hubScene;

    if (gameInfoManager->mCurrentMode >= GameInfoManager::GM_MUSHROOM_CUP
        && gameInfoManager->mCurrentMode <= GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
    {
        nlSingleton<GameInfoManager>::s_pInstance->OnPreCupGameState();
    }

    gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    nlSingleton<StatsTracker>::s_pInstance->SetBasicGameInfoPointer(gameInfoManager->mGameInfo[gameInfoManager->mCurrentMode], true);

    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 1, nlRandom(4, &nlDefaultSeed), -1, 0, 1, 0);
    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_WIN, 1, 0, 0, 1, 0, 0);

    gameInfoManager->SetRoundResult(false, 1);
    gameInfoManagerPost = nlSingleton<GameInfoManager>::s_pInstance;

    nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

    gameInfoManagerCup = nlSingleton<GameInfoManager>::s_pInstance;
    gameInfo = gameInfoManagerCup->mGameInfo[gameInfoManagerCup->mCurrentMode];

    if (gameInfoManagerCup->IsInCupMode())
    {
        int team0 = gameInfo->mTeamIndex[0];
        homeAway = team0 != gameInfoManagerCup->GetUserSelectedCupTeam();

        if (self->mSniper > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_GOALS_FOR, homeAway, 0, self->mSniper);
        }

        if (self->mStriker > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_STS_ATTEMPTS, homeAway, 0, self->mStriker);
        }

        if (self->mTactician > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_PERFECT_PASSES, homeAway, 0, self->mTactician);
        }

        if (self->mParamedic > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_HITS_MADE, homeAway, 0, self->mParamedic);
        }

        if (self->mVeteran > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddMilestoneUserStat(STATS_GAMES_PLAYED, self->mVeteran);
        }
    }

    nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();

    if (gameInfoManagerPost->IsInCupMode())
    {
        gameInfoManagerPost->OnPostCupGameState();
    }
    else
    {
        gameInfoManagerPost->IncreaseGameNumber(true);

        while ((s16)gameInfoManagerPost->GetCurrentRoundNumber() != -5)
        {
            if (gameInfoManagerPost->DetermineNextMatchups(27))
            {
                break;
            }

            gameInfoManagerPost->IncreaseRoundNumber();
        }

        hubScene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push((SceneList)0x18, SCREEN_NOTHING, false);
        hubScene->mDoAutoSave = true;
    }
}

/**
 * Offset/Address/Size: 0x9B8 | 0x800E8108 | size: 0x26C
 * TODO: 99.71% match - r29/r31 register coalescing for gameInfoManager, r28 load scheduling for gameInfoManagerCup
 */
void CupCheaterScene::OnSelectHomeOTWin()
{
    CupCheaterScene* const self = this;
    bool homeAway;
    GameInfoManager* gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    GameInfoManager* gameInfoManagerPost;
    GameInfoManager* gameInfoManagerCup;
    BasicGameInfo* gameInfo;
    CupHubScene* hubScene;

    if (gameInfoManager->mCurrentMode >= GameInfoManager::GM_MUSHROOM_CUP
        && gameInfoManager->mCurrentMode <= GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
    {
        nlSingleton<GameInfoManager>::s_pInstance->OnPreCupGameState();
    }

    gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    nlSingleton<StatsTracker>::s_pInstance->SetBasicGameInfoPointer(gameInfoManager->mGameInfo[gameInfoManager->mCurrentMode], true);

    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 0, nlRandom(4, &nlDefaultSeed), -1, 0, 1, 0);
    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_OT_WIN, 0, 0, 1, 0, 0, 0);

    gameInfoManager->SetRoundResult(true, 0);
    gameInfoManagerPost = nlSingleton<GameInfoManager>::s_pInstance;

    nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

    gameInfoManagerCup = nlSingleton<GameInfoManager>::s_pInstance;
    gameInfo = gameInfoManagerCup->mGameInfo[gameInfoManagerCup->mCurrentMode];

    if (gameInfoManagerCup->IsInCupMode())
    {
        int team0 = gameInfo->mTeamIndex[0];
        homeAway = team0 != gameInfoManagerCup->GetUserSelectedCupTeam();

        if (self->mSniper > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_GOALS_FOR, homeAway, 0, self->mSniper);
        }

        if (self->mStriker > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_STS_ATTEMPTS, homeAway, 0, self->mStriker);
        }

        if (self->mTactician > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_PERFECT_PASSES, homeAway, 0, self->mTactician);
        }

        if (self->mParamedic > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_HITS_MADE, homeAway, 0, self->mParamedic);
        }

        if (self->mVeteran > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddMilestoneUserStat(STATS_GAMES_PLAYED, self->mVeteran);
        }
    }

    nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();

    if (gameInfoManagerPost->IsInCupMode())
    {
        gameInfoManagerPost->OnPostCupGameState();
    }
    else
    {
        gameInfoManagerPost->IncreaseGameNumber(true);

        while ((s16)gameInfoManagerPost->GetCurrentRoundNumber() != -5)
        {
            if (gameInfoManagerPost->DetermineNextMatchups(27))
            {
                break;
            }

            gameInfoManagerPost->IncreaseRoundNumber();
        }

        hubScene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push((SceneList)0x18, SCREEN_NOTHING, false);
        hubScene->mDoAutoSave = true;
    }
}

void CupCheaterScene::OnSelectAwayOTWin()
{
    CupCheaterScene* const self = this;
    bool homeAway;
    GameInfoManager* gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    GameInfoManager* gameInfoManagerPost;
    GameInfoManager* gameInfoManagerCup;
    BasicGameInfo* gameInfo;
    CupHubScene* hubScene;

    if (gameInfoManager->mCurrentMode >= GameInfoManager::GM_MUSHROOM_CUP
        && gameInfoManager->mCurrentMode <= GameInfoManager::GM_SUPER_BOWSER_CUP)
    {
        nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
    }

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
    {
        nlSingleton<GameInfoManager>::s_pInstance->OnPreCupGameState();
    }

    gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
    nlSingleton<StatsTracker>::s_pInstance->SetBasicGameInfoPointer(gameInfoManager->mGameInfo[gameInfoManager->mCurrentMode], true);

    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 1, nlRandom(4, &nlDefaultSeed), -1, 0, 1, 0);
    nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_OT_WIN, 1, 0, 0, 1, 0, 0);

    gameInfoManager->SetRoundResult(true, 1);
    gameInfoManagerPost = nlSingleton<GameInfoManager>::s_pInstance;

    nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

    gameInfoManagerCup = nlSingleton<GameInfoManager>::s_pInstance;
    gameInfo = gameInfoManagerCup->mGameInfo[gameInfoManagerCup->mCurrentMode];

    if (gameInfoManagerCup->IsInCupMode())
    {
        int team0 = gameInfo->mTeamIndex[0];
        homeAway = team0 != gameInfoManagerCup->GetUserSelectedCupTeam();

        if (self->mSniper > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_GOALS_FOR, homeAway, 0, self->mSniper);
        }

        if (self->mStriker > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_STS_ATTEMPTS, homeAway, 0, self->mStriker);
        }

        if (self->mTactician > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_PERFECT_PASSES, homeAway, 0, self->mTactician);
        }

        if (self->mParamedic > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddStat(STATS_HITS_MADE, homeAway, 0, self->mParamedic);
        }

        if (self->mVeteran > 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->AddMilestoneUserStat(STATS_GAMES_PLAYED, self->mVeteran);
        }
    }

    nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();

    if (gameInfoManagerPost->IsInCupMode())
    {
        gameInfoManagerPost->OnPostCupGameState();
    }
    else
    {
        gameInfoManagerPost->IncreaseGameNumber(true);

        while ((s16)gameInfoManagerPost->GetCurrentRoundNumber() != -5)
        {
            if (gameInfoManagerPost->DetermineNextMatchups(27))
            {
                break;
            }

            gameInfoManagerPost->IncreaseRoundNumber();
        }

        hubScene = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push((SceneList)0x18, SCREEN_NOTHING, false);
        hubScene->mDoAutoSave = true;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800E7750 | size: 0x74C
 */
void CupCheaterScene::UpdateSlides()
{
    FORCE_DONT_INLINE;
}
