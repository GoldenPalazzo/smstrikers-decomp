#define NL_NO_LEXICALCAST_NLSTRING_INT
#define BASICSTRING_OUTLINE_CTOR
#define BASICSTRING_INDEX_EMPTY_COPY_BYTE_OFFSET
#define BASICSTRING_INDEX_ALLOC_HELPER
#define BIND_NO_DECL
#define BIND_FORCE_DONT_INLINE
#define FUNCTION1_SPLIT_BODIES
#define FUNCTION1_BIND_BY_VALUE
#define MEMFUN_NO_DECL
#define MEMFUN_FORCE_DONT_INLINE
#define NLSTRNCPY_FORCE_DONT_INLINE
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
#include "NL/nlLocalization.h"
#include "NL/nlString.h"

#include "NL/nlMemFunBody.h"
#include "NL/nlBindBody.h"

typedef Detail::MemFunImpl<void, void (PausePostGameScene::*)()> MemFunImpl_PausePostGame_t;
typedef BindExp1<void, MemFunImpl_PausePostGame_t, PausePostGameScene*> BindExp1_PausePostGame_t;
typedef Function1<void, TLComponentInstance*>::FunctorImpl<BindExp1_PausePostGame_t> PausePostGameFunctor_t;

#define NL_FORMAT_EXPLICIT_WIDE_POINTER_BODY
#include "NL/nlFormat.h"
#undef NL_FORMAT_EXPLICIT_WIDE_POINTER_BODY

typedef BasicString<unsigned short, Detail::TempStringAllocator> (*PausePostGameFormat_t)(
    const BasicString<unsigned short, Detail::TempStringAllocator>&,
    const unsigned short (&)[8]);
static PausePostGameFormat_t ForcePausePostGameFormat
    = &Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[8]>;

template WEAKFUNC BindExp1_PausePostGame_t Bind<void, MemFunImpl_PausePostGame_t, PausePostGameScene*>(
    MemFunImpl_PausePostGame_t,
    PausePostGameScene* const&);
template WEAKFUNC MemFunImpl_PausePostGame_t MemFun<PausePostGameScene, void>(void (PausePostGameScene::*)());
template WEAKFUNC unsigned short* nlStrNCpy<unsigned short>(unsigned short*, const unsigned short*, unsigned long);

#include "NL/nlFunctionReap.h"

template void nlFunctionReap<PausePostGameFunctor_t>(PausePostGameFunctor_t*);

namespace DoubleHighlite
{
static const char* SLIDE_IN = "in";
static const char* SLIDE_OUT = "out";
} // namespace DoubleHighlite

static int gPadThatQuit = 8;

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

static inline const unsigned short* LookupLocHash(unsigned long hash)
{
    nlLocalization* loc = g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* entry = nlBSearch(hash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
    if (entry)
    {
        return loc->m_FirstString + entry->StringOffset;
    }
    return MissingLocString;
}

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
    // EMPTY
}

static inline MenuItem<TLComponentInstance>* PausePostGameItemAt(MenuList<TLComponentInstance>& menu, int idx)
{
    return &menu.mMenuItems[idx];
}

static inline u8 PausePostGameHasSide(BasicGameInfo* game, int side)
{
    for (int i = 0; i < 4; i++)
    {
        if ((int)game->mPadSides[i] == side)
        {
            return 1;
        }
    }
    return 0;
}

static inline int PausePostGameGetWin(StatsTracker* tracker, int index)
{
    return tracker->mNumGamesWon[index];
}

static inline unsigned int PausePostGameAbsDiff(int value)
{
    return (value < 0) ? -value : value;
}

/**
 * Offset/Address/Size: 0x608 | 0x8010770C | size: 0x187C
 */
void PausePostGameScene::SceneCreated()
{
    typedef MemFunImpl_PausePostGame_t PauseMemFun;
    typedef BindExp1_PausePostGame_t PauseBind;
    typedef Function1<void, TLComponentInstance*>::FunctorImpl<PauseBind> PauseFunctorImpl;

    EnableAutoPressed();

    static void (PausePostGameScene::* FunctionTable[3])() = {
        &PausePostGameScene::OnSelectRematch,
        &PausePostGameScene::OnSelectChangeTeams,
        &PausePostGameScene::OnSelectQuit,
    };

    TLComponentInstance* pButtonComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")));
    mButtons.mButtonInstance = pButtonComp;
    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    MenuItem<TLComponentInstance>* menuItem;

    for (int i = 0; i < 3; i++)
    {
        char menuname[64];
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)));

        instance->SetActiveSlide((i == 0) ? DoubleHighlite::SLIDE_IN : DoubleHighlite::SLIDE_OUT);

        int idx = mMenuItems.mNumItemsAdded;
        menuItem = PausePostGameItemAt(mMenuItems, idx);
        mMenuItems.mMenuItems[idx].mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<TLComponentInstance*> openFunction;
            openFunction.mTag = FREE_FUNCTION;
            openFunction.mFreeFunction = DoubleHighlite::OpenItem;
            *(Function<TLComponentInstance*>*)&menuItem->mCallbacks[ON_HIGHLIGHT] = openFunction;
        }

        {
            Function<TLComponentInstance*> closeFunction;
            closeFunction.mTag = FREE_FUNCTION;
            closeFunction.mFreeFunction = DoubleHighlite::CloseItem;
            *(Function<TLComponentInstance*>*)&menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunction;
        }

        {
            Function<TLComponentInstance*> applyFunction(Bind<void, MemFunImpl_PausePostGame_t, PausePostGameScene*>(
                MemFun<PausePostGameScene, void>(FunctionTable[i]), this));
            *(Function<TLComponentInstance*>*)&menuItem->mCallbacks[ON_APPLY] = applyFunction;
        }

        FindComponent(instance->GetActiveSlide(), "highlite");

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
            InlineHasher(nlStringLowerHash(compname)));

        TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("TEAM")));

        text->m_LocStrId = GetLOCTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i));
        text->m_OverloadFlags |= 8;

        text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("LINE_0")));

        BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(
            (int)nlSingleton<StatsTracker>::s_pInstance->mNumGamesWon[i]);
        unsigned short wscore[8];
        nlStrToWcs(score.c_str(), wscore, 8);
        memcpy(mScoreBuffer[i], wscore, sizeof(wscore));
        mScoreBuffer[i][score.m_data ? score.m_data->mSize - 1 : 0] = 0;
        text->SetString(mScoreBuffer[i]);
    }

    int wins0 = PausePostGameGetWin(nlSingleton<StatsTracker>::s_pInstance, 0);
    int wins1 = PausePostGameGetWin(nlSingleton<StatsTracker>::s_pInstance, 1);
    int pointdiff = wins0 - wins1;
    unsigned int absdiff = PausePostGameAbsDiff(pointdiff);

    TLTextInstance* message = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("MESSAGE 1")));

    BasicGameInfo* game = nlSingleton<GameInfoManager>::s_pInstance->mGameInfo[nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode];
    u8 hasHome = PausePostGameHasSide(game, 0);
    if (hasHome)
    {
        u8 hasAway = PausePostGameHasSide(game, 1);
        if (hasAway)
        {
        if (absdiff == 0)
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0x317831E4);

            BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(
                wins0 + wins1 + 1);
            unsigned short wscore[8];
            nlStrToWcs(score.c_str(), wscore, 8);

            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), wscore);
            SetText(*message, formatted);
        }
        else if (absdiff <= 2)
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0x29199065);

            eTeamID winningteam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)((pointdiff > 0) ? 0 : 1));

            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), LookupLocHash(GetLOCCharacterName(winningteam, true, false)));
            SetText(*message, formatted);
        }
        else if (absdiff <= 6)
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0x1214A3EB);

            eTeamID loosingteam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)((pointdiff > 0) ? 1 : 0));

            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), LookupLocHash(GetLOCCharacterName(loosingteam, true, false)));
            SetText(*message, formatted);
        }
        else
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0xAACD893B);

            eTeamID loosingteam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)((pointdiff > 0) ? 1 : 0));

            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), LookupLocHash(GetLOCCharacterName(loosingteam, true, false)));
            SetText(*message, formatted);
        }
        return;
        }
    }
    {
        u8 newHasHome = PausePostGameHasSide(game, 0);
        int humanside = (int)(newHasHome ? 0 : 1);

        if (pointdiff == 0)
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0xFF559E6A);

            BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(
                wins0 + wins1 + 1);
            unsigned short wscore[8];
            nlStrToWcs(score.c_str(), wscore, 8);

            BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), wscore);
            SetText(*message, formatted);
        }
        else if (((humanside == 0) && (pointdiff > 0)) || ((humanside == 1) && (pointdiff < 0)))
        {
            if (absdiff <= 2)
            {
                message->m_LocStrId = 0x8BFA3E58;
                message->m_OverloadFlags |= 8;
            }
            else if (absdiff <= 6)
            {
                const unsigned short* formatLoc;
                formatLoc = LookupLocHash(0xDBBFA4DE);

                eTeamID otherteam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)(humanside ? 0 : 1));

                BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(formatLoc), LookupLocHash(GetLOCCharacterName(otherteam, true, false)));
                SetText(*message, formatted);
            }
            else
            {
                if (nlSingleton<GameInfoManager>::s_pInstance->GetSkillLevel() == GameplaySettings::LEGEND)
                {
                    message->m_LocStrId = 0xA4CA441C;
                    message->m_OverloadFlags |= 8;
                }
                else
                {
                    message->m_LocStrId = 0x460AB22E;
                    message->m_OverloadFlags |= 8;
                }
            }
        }
        else
        {
            if (absdiff <= 2)
            {
                message->m_LocStrId = 0x0B3C1BCB;
                message->m_OverloadFlags |= 8;
            }
            else if (absdiff <= 6)
            {
                message->m_LocStrId = 0x53AE5311;
                message->m_OverloadFlags |= 8;
            }
            else
            {
                message->m_LocStrId = 0x075B0A61;
                message->m_OverloadFlags |= 8;
            }
        }
    }
}

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
#pragma dont_inline on
void PausePostGameScene::SetText(TLTextInstance& textinstance, const BasicString<unsigned short, Detail::TempStringAllocator>& string)
{
    nlStrNCpy(mRematchTextBuffer, string.c_str(), 128);
    mRematchTextBuffer[127] = 0;
    textinstance.SetString(mRematchTextBuffer);
}
#pragma dont_inline reset
