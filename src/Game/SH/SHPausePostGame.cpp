#define NL_NO_LEXICALCAST_NLSTRING_INT
#define BASICSTRING_OUTLINE_CTOR
#define BASICSTRING_INDEX_EMPTY_COPY_BYTE_OFFSET
#define BIND_NO_DECL
#define FUNCTION1_SPLIT_BODIES
#define MEMFUN_NO_DECL
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

#include "NL/nlMemFunBody.h"
#include "NL/nlBindBody.h"

typedef Detail::MemFunImpl<void, void (PausePostGameScene::*)()> MemFunImpl_PausePostGame_t;
typedef BindExp1<void, MemFunImpl_PausePostGame_t, PausePostGameScene*> BindExp1_PausePostGame_t;

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
 * Offset/Address/Size: 0x26B8 | 0x801097BC | size: 0xCF0
 * TODO: 99.75% match - post-erase BasicString data and copy-on-write temporaries use shifted registers.
 */
template <>
template <>
FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&
    FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >::operator% <const unsigned short*>(
        const unsigned short* const& t)
{
    BasicString<unsigned short, Detail::TempStringAllocator> insert = LexicalCast<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != (unsigned short)'{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 1] - '0' != mCurrentPos)
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 2] != (unsigned short)'}')
            continue;

        unsigned short* eraseBeginData;
        unsigned short* eraseEnd;
        mString[0];
        eraseBeginData = mString.m_data ? mString.m_data->mData : (unsigned short*)0;
        mString[0];
        eraseEnd = (mString.m_data ? mString.m_data->mData : (unsigned short*)0) + i + 3;
        mString.erase(eraseBeginData + i, eraseEnd);
        mString[i];
        unsigned short* mStringData = mString.m_data ? mString.m_data->mData : 0;
        insert[0];
        unsigned short* insertBegin = insert.m_data ? insert.m_data->mData : 0;
        insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? insert.m_data->mData + insert.m_data->mSize - 1 : (unsigned short*)0);
    }

    mCurrentPos++;
    return *this;
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

/**
 * Offset/Address/Size: 0x608 | 0x8010770C | size: 0x187C
 */
void PausePostGameScene::SceneCreated()
{
    typedef MemFunImpl_PausePostGame_t PauseMemFun;
    typedef BindExp1_PausePostGame_t PauseBind;
    typedef Function1<void, TLComponentInstance*>::FunctorImpl<PauseBind> PauseFunctorImpl;

    EnableAutoPressed();

    static void (PausePostGameScene::* FunctionTable[3])() = { };
    static signed char init;

    if (!init)
    {
        FunctionTable[0] = &PausePostGameScene::OnSelectRematch;
        FunctionTable[1] = &PausePostGameScene::OnSelectChangeTeams;
        FunctionTable[2] = &PausePostGameScene::OnSelectQuit;
        init = 1;
    }

    TLComponentInstance* pButtonComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(nlStringLowerHash("Layer")));
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
            PauseBind bind = Bind<void, MemFunImpl_PausePostGame_t, PausePostGameScene*>(
                MemFun<PausePostGameScene, void>(FunctionTable[i]), this);
            Function<TLComponentInstance*> applyFunction(bind);
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
            InlineHasher(nlStringLowerHash("TEAM")),
            InlineHasher(0));

        text->m_LocStrId = GetLOCTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i));
        text->m_OverloadFlags |= 8;

        text = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            instance->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("LINE_0")),
            InlineHasher(0));

        int numWins = nlSingleton<StatsTracker>::s_pInstance->mNumGamesWon[i];
        BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(numWins);
        unsigned short wscore[8];
        nlStrToWcs(score.c_str(), wscore, 8);
        memcpy(mScoreBuffer[i], wscore, sizeof(wscore));
        mScoreBuffer[i][score.m_data ? score.m_data->mSize - 1 : 0] = 0;
        text->SetString(mScoreBuffer[i]);
    }

    StatsTracker* tracker = nlSingleton<StatsTracker>::s_pInstance;
    int wins0 = tracker->mNumGamesWon[0];
    int wins1 = tracker->mNumGamesWon[1];
    int pointdiff = wins0 - wins1;
    unsigned int absdiff = (pointdiff < 0) ? -pointdiff : pointdiff;

    TLTextInstance* message = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("MESSAGE 1")));

    u8 hasHome = 0;
    BasicGameInfo* game = nlSingleton<GameInfoManager>::s_pInstance->mGameInfo[nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode];
    if ((int)game->mPadSides[0] == 0)
    {
        hasHome = 1;
    }
    else if ((int)game->mPadSides[1] == 0)
    {
        hasHome = 1;
    }
    else if ((int)game->mPadSides[2] == 0)
    {
        hasHome = 1;
    }
    else if ((int)game->mPadSides[3] == 0)
    {
        hasHome = 1;
    }

    u8 hasAway = 0;
    if (hasHome)
    {
        if ((int)game->mPadSides[0] == 1)
        {
            hasAway = 1;
        }
        else if ((int)game->mPadSides[1] == 1)
        {
            hasAway = 1;
        }
        else if ((int)game->mPadSides[2] == 1)
        {
            hasAway = 1;
        }
        else if ((int)game->mPadSides[3] == 1)
        {
            hasAway = 1;
        }
    }

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
            formatLoc = LookupLocHash(0x291A9065);

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
    }
    else
    {
        u8 newHasHome = 0;
        if ((int)game->mPadSides[0] == 0)
        {
            newHasHome = 1;
        }
        else if ((int)game->mPadSides[1] == 0)
        {
            newHasHome = 1;
        }
        else if ((int)game->mPadSides[2] == 0)
        {
            newHasHome = 1;
        }
        else if ((int)game->mPadSides[3] == 0)
        {
            newHasHome = 1;
        }
        int humanside = (newHasHome == 0);

        if (absdiff == 0)
        {
            const unsigned short* formatLoc;
            formatLoc = LookupLocHash(0xFF559E6A);

            BasicString<char, Detail::TempStringAllocator> score = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(absdiff + 1);
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
