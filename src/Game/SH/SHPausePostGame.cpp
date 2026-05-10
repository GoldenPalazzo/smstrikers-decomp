#include "Game/SH/SHPausePostGame.h"

#include "Game/Audio/AudioLoader.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/DB/StatsTracker.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feManager.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/OverlayManager.h"
#include "NL/nlFormat.h"
#include "NL/nlLocalization.h"
#include "NL/nlString.h"

static int gPadThatQuit;

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)())
{
    return Detail::MemFunImpl<R, void (T::*)()>(fn);
}

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
    typedef Detail::MemFunImpl<void, void (PausePostGameScene::*)()> PauseMemFun;
    typedef BindExp1<void, PauseMemFun, PausePostGameScene*> PauseBind;
    typedef Function1<void, TLComponentInstance*>::FunctorImpl<PauseBind> PauseFunctorImpl;

#define LOOKUP_LOC_STRING(_hashExpr, _locVar)                                                                              \
    {                                                                                                                      \
        unsigned long _hash = (_hashExpr);                                                                                 \
        nlLocalization* _loc = g_pLocalization;                                                                            \
        if (_loc->m_LookupTable == 0)                                                                                      \
        {                                                                                                                  \
            (_locVar) = LocalizationTableNotFound;                                                                         \
        }                                                                                                                  \
        else                                                                                                               \
        {                                                                                                                  \
            nlLocalization::StringLookup* _entry = nlBSearch(_hash, _loc->m_LookupTable, (int)_loc->m_pFile->StringCount); \
            if (_entry != 0)                                                                                               \
            {                                                                                                              \
                (_locVar) = _loc->m_FirstString + _entry->StringOffset;                                                    \
            }                                                                                                              \
            else                                                                                                           \
            {                                                                                                              \
                (_locVar) = MissingLocString;                                                                              \
            }                                                                                                              \
        }                                                                                                                  \
    }

    EnableAutoPressed();

    static void (PausePostGameScene::* FunctionTable[3])();
    static signed char init;

    if (!init)
    {
        FunctionTable[0] = &PausePostGameScene::OnSelectRematch;
        FunctionTable[1] = &PausePostGameScene::OnSelectChangeTeams;
        FunctionTable[2] = &PausePostGameScene::OnSelectQuit;
        init = 1;
    }

    mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    for (int i = 0; i < 3; i++)
    {
        char menuname[64];
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        instance->SetActiveSlide(i == 0 ? "in" : "out");

        MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
        menuItem->mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<TLComponentInstance*> openFunction;
            openFunction.mTag = FREE_FUNCTION;
            openFunction.mFreeFunction = DoubleHighlite::OpenItem;
            menuItem->mCallbacks[ON_HIGHLIGHT] = openFunction;
        }

        {
            Function<TLComponentInstance*> closeFunction;
            closeFunction.mTag = FREE_FUNCTION;
            closeFunction.mFreeFunction = DoubleHighlite::CloseItem;
            menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunction;
        }

        {
            PauseBind bind = Bind<void, PauseMemFun, PausePostGameScene*>(
                MemFun<PausePostGameScene, void>(FunctionTable[i]), this);
            PauseFunctorImpl* impl = new ((PauseFunctorImpl*)nlMalloc(sizeof(PauseFunctorImpl), 8, false)) PauseFunctorImpl(bind);

            Function<TLComponentInstance*> applyFunction;
            applyFunction.mTag = FUNCTOR;
            applyFunction.mFunctor = impl;
            menuItem->mCallbacks[ON_APPLY] = applyFunction;
        }

        TLComponentInstance* highlite = (TLComponentInstance*)FindComponent(instance->GetActiveSlide(), "highlite");
        (void)highlite;

        if (i == 0)
        {
            DoubleHighlite::TempDisableSound();
        }

        menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
    }

    mMenuItems.mFlags = 1;

    for (int i = 0; i < 2; i++)
    {
        char compname[64];
        nlSNPrintf(compname, 64, "numeric_column_sm%d", i + 1);

        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(compname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("TEAM")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        text->m_LocStrId = GetLOCTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i));
        text->m_OverloadFlags |= 8;

        text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("LINE_0")),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(
            nlSingleton<StatsTracker>::s_pInstance->mNumGamesWon[i]);
        unsigned short wscore[8];
        nlStrToWcs(score.c_str(), wscore, 8);
        memcpy(mScoreBuffer[i], wscore, sizeof(wscore));
        mScoreBuffer[i][nlStrLen(score.c_str())] = 0;
        text->SetString(mScoreBuffer[i]);
    }

    StatsTracker* tracker = nlSingleton<StatsTracker>::s_pInstance;
    int score0 = tracker->mNumGamesWon[0];
    int score1 = tracker->mNumGamesWon[1];
    int pointdiff = score0 - score1;
    int absdiff = pointdiff;
    if (absdiff < 0)
    {
        absdiff = -absdiff;
    }

    TLTextInstance* message = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("MESSAGE 1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    BasicGameInfo* game = gameInfo->mGameInfo[gameInfo->mCurrentMode];

    bool hasHome = (game->mPadSides[0] == 0) || (game->mPadSides[1] == 0) || (game->mPadSides[2] == 0) || (game->mPadSides[3] == 0);
    bool hasAway = (game->mPadSides[0] == 1) || (game->mPadSides[1] == 1) || (game->mPadSides[2] == 1) || (game->mPadSides[3] == 1);

    if (hasHome && hasAway)
    {
        if (absdiff == 0)
        {
            const unsigned short* formatLoc;
            LOOKUP_LOC_STRING(0x317831E4, formatLoc);

            BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(score0 + score1 + 1);
            unsigned short wscore[8];
            nlStrToWcs(score.c_str(), wscore, 8);

            BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, wscore);
            SetText(*message, formatted);
        }
        else if (absdiff <= 2)
        {
            const unsigned short* formatLoc;
            LOOKUP_LOC_STRING(0x291A9065, formatLoc);

            eTeamID winningteam = gameInfo->GetTeam((short)((pointdiff > 0) ? 1 : 0));
            const unsigned short* charLoc;
            LOOKUP_LOC_STRING(GetLOCCharacterName(winningteam, true, false), charLoc);

            BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, charLoc);
            SetText(*message, formatted);
        }
        else if (absdiff <= 6)
        {
            const unsigned short* formatLoc;
            LOOKUP_LOC_STRING(0x1214A3EB, formatLoc);

            eTeamID loosingteam = gameInfo->GetTeam((short)((pointdiff > 0) ? 1 : 0));
            const unsigned short* charLoc;
            LOOKUP_LOC_STRING(GetLOCCharacterName(loosingteam, true, false), charLoc);

            BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, charLoc);
            SetText(*message, formatted);
        }
        else
        {
            const unsigned short* formatLoc;
            LOOKUP_LOC_STRING(0xAACD893B, formatLoc);

            eTeamID loosingteam = gameInfo->GetTeam((short)((pointdiff > 0) ? 1 : 0));
            const unsigned short* charLoc;
            LOOKUP_LOC_STRING(GetLOCCharacterName(loosingteam, true, false), charLoc);

            BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, charLoc);
            SetText(*message, formatted);
        }
    }
    else
    {
        bool noHomePlayers = !hasHome;

        if (pointdiff == 0)
        {
            const unsigned short* formatLoc;
            LOOKUP_LOC_STRING(0xFF559E6A, formatLoc);

            BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(absdiff + 1);
            unsigned short wscore[8];
            nlStrToWcs(score.c_str(), wscore, 8);

            BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, wscore);
            SetText(*message, formatted);
        }
        else if ((!noHomePlayers && pointdiff > 0) || (noHomePlayers && pointdiff < 0))
        {
            if (absdiff <= 2)
            {
                message->m_LocStrId = 0x8BFA3E58;
                message->m_OverloadFlags |= 8;
            }
            else if (absdiff <= 6)
            {
                const unsigned short* formatLoc;
                LOOKUP_LOC_STRING(0xDBBFA4DE, formatLoc);

                eTeamID otherteam = gameInfo->GetTeam((short)(noHomePlayers ? 0 : 1));
                const unsigned short* charLoc;
                LOOKUP_LOC_STRING(GetLOCCharacterName(otherteam, true, false), charLoc);

                BasicString<unsigned short, Detail::TempStringAllocator> locFmt(formatLoc);
                BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(locFmt, charLoc);
                SetText(*message, formatted);
            }
            else
            {
                if (gameInfo->GetSkillLevel() == GameplaySettings::LEGEND)
                {
                    message->m_LocStrId = 0xA4CA441C;
                }
                else
                {
                    message->m_LocStrId = 0x460AB22E;
                }
                message->m_OverloadFlags |= 8;
            }
        }
        else
        {
            if (absdiff <= 2)
            {
                message->m_LocStrId = 0x0B3C1BCB;
            }
            else if (absdiff <= 6)
            {
                message->m_LocStrId = 0x53AE5311;
            }
            else
            {
                message->m_LocStrId = 0x075B0A61;
            }
            message->m_OverloadFlags |= 8;
        }
    }

#undef LOOKUP_LOC_STRING
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
void PausePostGameScene::SetText(TLTextInstance& textinstance, const BasicString<unsigned short, Detail::TempStringAllocator>& string)
{
    nlStrNCpy(mRematchTextBuffer, string.c_str(), 128);
    mRematchTextBuffer[127] = 0;
    textinstance.SetString(mRematchTextBuffer);
}
