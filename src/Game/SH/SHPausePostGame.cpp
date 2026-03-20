#include "Game/SH/SHPausePostGame.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/DB/StatsTracker.h"
#include "Game/FE/feManager.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/OverlayManager.h"

static int gPadThatQuit;

// /**
//  * Offset/Address/Size: 0x118 | 0x801097BC | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator% <const unsigned short*>(const unsigned short*
// const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801096A4 | size: 0x118
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[8]>(
//     const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short (&)[8])
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010966C | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (PausePostGameScene::*)()>, PausePostGameScene*>(
//     Detail::MemFunImpl<void, void (PausePostGameScene::*)()>, PausePostGameScene* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80109650 | size: 0x1C
//  */
// void MemFun<PausePostGameScene, void>(void (PausePostGameScene::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010960C | size: 0x44
//  */
// void nlStrNCpy<unsigned short>(unsigned short*, const unsigned short*, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801095B0 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (PausePostGameScene::*)()>, PausePostGameScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x78 | 0x80109580 | size: 0x30
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (PausePostGameScene::*)()>, PausePostGameScene*>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80109508 | size: 0x78
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (PausePostGameScene::*)()>, PausePostGameScene*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x801093AC | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x80109328 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x801092F0 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x80109194 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x80109110 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801090D8 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1F28 | 0x8010902C | size: 0xAC
 */
PausePostGameScene::PausePostGameScene()
    : BaseSceneHandler()
    , mMenuItems()
    , mButtons()
{
    gPadThatQuit = 8;
}

/**
 * Offset/Address/Size: 0x1E84 | 0x80108F88 | size: 0xA4
 */
PausePostGameScene::~PausePostGameScene()
{
}

/**
 * Offset/Address/Size: 0x608 | 0x8010770C | size: 0x187C
 */
void PausePostGameScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x28C | 0x80107390 | size: 0x37C
 */
void PausePostGameScene::Update(float dt)
{
    BaseSceneHandler::Update(dt);
    mButtons.CentreButtons();

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, (eFEINPUT_PAD*)&gPadThatQuit))
    {
        int currentIndex = mMenuItems.mCurrentIndex;
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mTag;

        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            if (mMenuItems.mMenuItems[currentIndex].mDisabled == 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;

                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFunctor)(type);
                }
            }
        }

        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        BaseSceneHandler* summary = nlSingleton<OverlayManager>::s_pInstance->Push(OVERLAY_SUMMARY, SCREEN_NOTHING, true);
        *(ButtonComponent::ButtonState*)((u8*)summary + 0xC3C) = (ButtonComponent::ButtonState)1;
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int skipDisabledFlag;
        int wrapFlag;
        int currentIndex;
        wrapFlag = flags & 1;
        skipDisabledFlag = flags & 2;
        currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex - 1;

        while (true)
        {
            if (wrapFlag)
            {
                if (newIndex < 0)
                {
                    newIndex = mMenuItems.mNumItemsAdded - 1;
                }
            }
            else if (newIndex < 0)
            {
                return;
            }

            if (skipDisabledFlag && mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex--;
            }
            else
            {
                break;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }

        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int skipDisabledFlag;
        int wrapFlag;
        int currentIndex;
        wrapFlag = flags & 1;
        skipDisabledFlag = flags & 2;
        currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex + 1;

        while (true)
        {
            if (wrapFlag)
            {
                newIndex = newIndex % mMenuItems.mNumItemsAdded;
            }
            else if (newIndex >= mMenuItems.mNumItemsAdded)
            {
                return;
            }

            if (skipDisabledFlag && mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex++;
            }
            else
            {
                break;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x150 | 0x80107254 | size: 0x13C
 */
void PausePostGameScene::OnSelectRematch()
{
    Config& cfg = Config::Global();
    TagValuePair& tvp = cfg.FindTvp("save_stats");
    bool saveStats;
    if (tvp.tag == NULL)
    {
        cfg.Set("save_stats", false);
        saveStats = false;
    }
    else
    {
        if (tvp.type == _BOOL)
        {
            saveStats = LexicalCast<bool, bool>(tvp.value.b);
        }
        else if (tvp.type == _INT)
        {
            saveStats = LexicalCast<bool, int>(tvp.value.i);
        }
        else if (tvp.type == _FLOAT)
        {
            saveStats = LexicalCast<bool, float>(tvp.value.f);
        }
        else if (tvp.type == _STRING)
        {
            saveStats = LexicalCast<bool, const char*>(tvp.value.s);
        }
        else
        {
            saveStats = false;
        }
    }

    if (saveStats)
    {
        StatsTracker* tracker = nlSingleton<StatsTracker>::s_pInstance;
        float gameTime = g_pGame->GetGameTime();
        tracker->WriteStats(gameTime, -1.0f, NULL);
    }

    nlSingleton<StatsTracker>::s_pInstance->ResetCurrentStats();
    nlSingleton<OverlayManager>::s_pInstance->Pop();
    g_pFEInput->EnableAnalogToDPadMapping(FE_ALL_PADS, false);
    FrontEnd::ExitWinnerScreen();
    g_pTrackManager->StopAllTracks(0);
    g_pGame->BeginGame(true, false);
    FrontEnd::m_bGameOver = false;
}

/**
 * Offset/Address/Size: 0xF8 | 0x801071FC | size: 0x58
 */
void PausePostGameScene::OnSelectQuit()
{
    if (GameInfoManager::Instance()->GetNumPlayers() > 1)
    {
        OverlayManager::Instance()->Push(OVERLAY_BRAG, SCREEN_NOTHING, true);
    }
    else
    {
        FrontEnd::ReturnToFE();
    }
}

/**
 * Offset/Address/Size: 0x88 | 0x8010718C | size: 0x70
 */
void PausePostGameScene::OnSelectChangeTeams()
{
    GameInfoManager::Instance()->mGoToChooseCaptains = true;
    GameInfoManager::Instance()->mMainUserPadNumber = (eFEINPUT_PAD)gPadThatQuit;
    if (GameInfoManager::Instance()->GetNumPlayers() > 1)
    {
        OverlayManager::Instance()->Push(OVERLAY_BRAG, SCREEN_NOTHING, true);
    }
    else
    {
        FrontEnd::ReturnToFE();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80107104 | size: 0x88
 */
// void PausePostGameScene::SetText(TLTextInstance&, const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }
