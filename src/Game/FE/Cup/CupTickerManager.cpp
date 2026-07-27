#include "Game/FE/Cup/CupTickerManager.h"

#include "Game/DB/StatsTracker.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feScrollText.h"
#include "Game/GameInfo.h"

#include "NL/gl/glStruct.h"
#include "NL/nlAlgorithm.h"
#include "NL/nlFormat.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlLocalization.h"
#include "NL/nlSingleton.h"
#include "NL/nlString.h"

#include "NL/nlBind.h"

typedef Detail::MemFunImpl<void, void (CupTickerManager::*)()> MemFunImpl_CupTickerManager_v;
typedef BindExp1<void, MemFunImpl_CupTickerManager_v, CupTickerManager*> BindExp1_vfmfcp;
typedef Function0<void>::FunctorImpl<BindExp1_vfmfcp> FunctorImpl_vfmfcp;

struct GameInfoAccessor_CupTicker
{
    char _pad6C[0x6C];
    unsigned char mDoingKnockout;
    char _pad4948[0x48DB];
    int mTournamentMode;
    char _pad4954[8];
    int mCurrentMode;
    char _pad4960[8];
    void* mCurrentCup;
};

struct BaseCupAccessor_CupTicker
{
    char _padA[0xA];
    short mGameNumber;
};

extern nlLocalization* g_pLocalization;
extern unsigned short LocalizationTableNotFound[];
extern unsigned short MissingLocString[];

// /**
//  * Offset/Address/Size: 0x0 | 0x800F5EBC | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>(Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1C48 | 0x800F5D94 | size: 0x128
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, const unsigned short*>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0xF58 | 0x800F50A4 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0xE30 | 0x800F4F7C | size: 0x128
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>, BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&, const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x800F428C | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F414C | size: 0x140
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, const unsigned short*, unsigned short[16], unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short* const&, const unsigned short(&)[16], const unsigned short(&)[16])
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F4130 | size: 0x1C
//  */
// void MemFun<CupTickerManager, void>(void (CupTickerManager::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F40D4 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F402C | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTickerManager::*)()>, CupTickerManager*>>::Clone() const
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800F3E24 | size: 0x208
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F3D68 | size: 0xBC
//  */
// void BasicString<unsigned short, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&) const
// {
// }

/**
 * Offset/Address/Size: 0x1D90 | 0x800F3D58 | size: 0x10
 */
CupTickerManager::CupTickerManager()
{
    mTicker = 0;
    mState = CUP_TICKER_STATE_0;
}

/**
 * Offset/Address/Size: 0x1C90 | 0x800F3C58 | size: 0x100
 */
CupTickerManager::~CupTickerManager()
{
    if (mTicker != 0)
    {
        delete mTicker;
    }
}

/**
 * Offset/Address/Size: 0x1968 | 0x800F3930 | size: 0x328
 */
void CupTickerManager::SetTickerTextInstance(TLTextInstance* tickerText)
{
    if (mTicker)
    {
        mTicker->ApplyNewTextInstancePointer(tickerText, 8000.0f, 100.0f);
    }
    else
    {
        gl_ScreenInfo* screenInfo = glGetScreenInfo();
        mTicker = new ((FEScrollText*)nlMalloc(sizeof(FEScrollText), 0x20, true))
            FEScrollText(tickerText, 0, screenInfo->ScreenWidth + 0x32);

        {
            Function<FnVoidVoid> callback(Bind<void, MemFunImpl_CupTickerManager_v, CupTickerManager*>(
                MemFun<CupTickerManager, void>(&CupTickerManager::CreateNewMessage), this));
            mTicker->m_messageFinishedCB = callback;
        }

        this->CreateNewMessage();
    }

    mTicker->SetDisplayMessage(BasicString<unsigned short, Detail::TempStringAllocator>(mMessageBuffer));
}

#define LOC_LOOKUP(_hashExpr, _locVar)                                                                                     \
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

static inline const unsigned short* CupTickerLookupLocString(unsigned long hash)
{
    nlLocalization* loc = g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }

    nlLocalization::StringLookup* entry = nlBSearch(hash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
    if (entry != 0)
    {
        return loc->m_FirstString + entry->StringOffset;
    }
    return MissingLocString;
}

/**
 * Offset/Address/Size: 0x680 | 0x800F2648 | size: 0x12E8
 */
void CupTickerManager::CreateNewMessage()
{
    bool tournamentLeague = false;
    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    GameInfoAccessor_CupTicker* gameInfoMem = (GameInfoAccessor_CupTicker*)gameInfo;
    WideBasicString tickerMessage;
    bool messageDisplayed = false;
    const unsigned short* locString;

    if (gameInfo->IsInTournamentMode() && gameInfoMem->mTournamentMode == 0)
    {
        tournamentLeague = true;
    }

    if (mTicker == 0)
    {
        return;
    }

    while (!messageDisplayed)
    {
        if (gameInfo->GetCurrentRoundNumber() == -5)
        {
            if (mState != 5)
            {
                mState = (eCupTickerState)5;

                unsigned long modeHash = GetLOCModeName((GameInfoManager::eGameModes)gameInfoMem->mCurrentMode);
                LOC_LOOKUP(modeHash, locString);
                WideBasicString modeWBS(locString);

                unsigned long charHash = GetLOCCharacterName(
                    gameInfo->FindWinningTeam(), false, false);
                LOC_LOOKUP(charHash, locString);
                WideBasicString charWBS(locString);

                LOC_LOOKUP(0x273CF730UL, locString);
                WideBasicString fmtWBS(locString);

                tickerMessage = Format<WideBasicString, WideBasicString, WideBasicString>(
                    fmtWBS, modeWBS, charWBS);
                break;
            }
            else if (mState != 2)
            {
                mState = (eCupTickerState)2;
                if (gameInfoMem->mTournamentMode != 0)
                {
                    continue;
                }
                BuildGoalTotalTickerMessage(tickerMessage, false);
                break;
            }
        }

        if (mState == 0)
        {
            if (gameInfoMem->mDoingKnockout)
            {
                LOC_LOOKUP(0x474DA1D4UL, locString);
                tickerMessage = WideBasicString(locString);
            }
            else
            {
                LOC_LOOKUP(0x67493499UL, locString);
                tickerMessage = WideBasicString(locString);
            }
            messageDisplayed = true;
        }

        if (mState == 1)
        {
            if (!tournamentLeague)
            {
                mState = (eCupTickerState)3;
            }
            else
            {
                LOC_LOOKUP(0xA81870D8UL, locString);
                tickerMessage = WideBasicString(locString);
                messageDisplayed = true;
            }
        }

        if (mState == 2)
        {
            if (!tournamentLeague
                || (gameInfo->GetCurrentRoundNumber() == 0
                    && gameInfo->mCurrentCup->mGameNumber == 0))
            {
                mState = (eCupTickerState)3;
            }
            else
            {
                BuildGoalTotalTickerMessage(tickerMessage, false);
                messageDisplayed = true;
            }
        }

        if (mState == 3)
        {
            if (gameInfo->GetCurrentRoundNumber() == -5)
            {
                mState = (eCupTickerState)4;
            }
            else
            {
                unsigned long team0LOC = GetLOCTeamName(gameInfo->GetTeam(0));
                unsigned long team1LOC = GetLOCTeamName(gameInfo->GetTeam(1));

                unsigned long fmtHash = nlStringLowerHash("CUPHUB_TICKER_NEXT_MATCH");
                LOC_LOOKUP(fmtHash, locString);
                WideBasicString fmtWBS(locString);
                const unsigned short* team0LocString;
                const unsigned short* team1LocString;
                LOC_LOOKUP(team0LOC, team0LocString);
                LOC_LOOKUP(team1LOC, team1LocString);

                tickerMessage = Format<WideBasicString, const unsigned short*, const unsigned short*>(
                    fmtWBS, team0LocString, team1LocString);
                messageDisplayed = true;
            }
        }

        if (mState == 4)
        {
            GameInfoManager::eGameModes mode = gameInfo->mCurrentMode;
            if ((mode == GameInfoManager::GM_MUSHROOM_CUP
                    && !gameInfo->IsUserQualified(GameInfoManager::GM_FLOWER_CUP))
                || (mode == GameInfoManager::GM_FLOWER_CUP
                    && !gameInfo->IsUserQualified(GameInfoManager::GM_STAR_CUP)))
            {
                LOC_LOOKUP(0x751FA62FUL, locString);
                tickerMessage = WideBasicString(locString);
                messageDisplayed = true;
            }
            else if (gameInfo->IsUserQualified(GameInfoManager::GM_FLOWER_CUP)
                     && gameInfo->IsUserQualified(GameInfoManager::GM_STAR_CUP)
                     && !gameInfo->IsUserQualified(GameInfoManager::GM_BOWSER_CUP)
                     && gameInfo->IsInCupMode())
            {
                LOC_LOOKUP(0xEEC22902UL, locString);
                tickerMessage = WideBasicString(locString);
                messageDisplayed = true;
            }
            else if ((mode == GameInfoManager::GM_BOWSER_CUP || mode == GameInfoManager::GM_SUPER_BOWSER_CUP)
                     && !gameInfoMem->mDoingKnockout)
            {
                LOC_LOOKUP(0x4B50DF6AUL, locString);
                tickerMessage = WideBasicString(locString);
                messageDisplayed = true;
            }
            else
            {
                mState = (eCupTickerState)5;
            }
        }

        if (mState == 5)
        {
            int firstRound = gameInfo->GetFirstRoundNumber();
            int currentRound = gameInfo->GetCurrentRoundNumber();
            if (currentRound == firstRound)
            {
                mState = CUP_TICKER_STATE_0;
            }
            else
            {
                int roundNumber = gameInfo->GetPreviousRoundNumber(-7);
                int numGames = (int)gameInfo->GetNumGamesPerRound(roundNumber);
                for (int gameNumber = 0; gameNumber < numGames; gameNumber++)
                {
                    BasicGameInfo* game;
                    if (gameInfo->GetCurrentRoundNumber() == -1)
                    {
                        game = &gameInfo->mUserInfo.mBowserCupFinalRound;
                    }
                    else
                    {
                        game = gameInfo->GetMatchupInfo(roundNumber, (unsigned short)gameNumber);
                    }

                    unsigned long team0Name = GetLOCTeamName(game->mTeamIndex[0]);
                    unsigned long team1Name = GetLOCTeamName(game->mTeamIndex[1]);

                    unsigned long formatHash;
                    if (gameNumber == 0)
                    {
                        formatHash = 0x97372F0FUL;
                    }
                    else
                    {
                        formatHash = 0x3E3B44CAUL;
                    }

                    NLString score0Str = LexicalCast<NLString, int>((int)game->mFinalScore[0]);
                    NLString score1Str = LexicalCast<NLString, int>((int)game->mFinalScore[1]);

                    unsigned short wideScore0[16];
                    nlStrToWcs(score0Str.c_str(), wideScore0, 16);

                    unsigned short wideScore1[16];
                    nlStrToWcs(score1Str.c_str(), wideScore1, 16);

                    if (game->mFinalScore[0] > game->mFinalScore[1])
                    {
                        const unsigned short* fmtLocString;
                        LOC_LOOKUP(formatHash, fmtLocString);
                        WideBasicString fmtWBS(fmtLocString);
                        const unsigned short* t0Str;
                        const unsigned short* t1Str;
                        LOC_LOOKUP(team0Name, t0Str);
                        LOC_LOOKUP(team1Name, t1Str);
                        tickerMessage = tickerMessage.Append(
                            Format<WideBasicString, const unsigned short*, const unsigned short*, unsigned short[16], unsigned short[16]>(
                                fmtWBS, t0Str, t1Str, wideScore0, wideScore1));
                    }
                    else
                    {
                        const unsigned short* fmtLocString;
                        LOC_LOOKUP(formatHash, fmtLocString);
                        WideBasicString fmtWBS(fmtLocString);
                        const unsigned short* t1Str;
                        const unsigned short* t0Str;
                        LOC_LOOKUP(team1Name, t1Str);
                        LOC_LOOKUP(team0Name, t0Str);
                        tickerMessage = tickerMessage.Append(
                            Format<WideBasicString, const unsigned short*, const unsigned short*, unsigned short[16], unsigned short[16]>(
                                fmtWBS, t1Str, t0Str, wideScore1, wideScore0));
                    }
                }
                messageDisplayed = true;
            }
        }

        mState = (eCupTickerState)(((int)mState + 1) % 6);
    }

    memcpy(mMessageBuffer, tickerMessage.c_str(), 0x400);
    WideBasicString displayMessage(mMessageBuffer);
    mTicker->SetDisplayMessage(displayMessage);
}

/**
 * Offset/Address/Size: 0x654 | 0x800F261C | size: 0x2C
 */
void CupTickerManager::Update(float dt)
{
    if (mTicker != NULL)
    {
        mTicker->Update(dt);
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x800F23A4 | size: 0x654
 * TODO: 99.65% match - r23/r25/r26 allocation differs in format-string lookup and temporary wide string construction.
 */
void CupTickerManager::BuildGoalTotalTickerMessage(
    BasicString<unsigned short, Detail::TempStringAllocator>& result, bool bIsHuman)
{
    int i;
    int numValid = 0;
    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    PlayerStats* pStats;

    int numTeams;
    if (bIsHuman)
    {
        numTeams = gameInfo->GetNumHumanTeams();
    }
    else
    {
        numTeams = (unsigned short)gameInfo->GetNumPlayingTeams();
    }

    PlayerStats playerStats[8];

    for (i = 0; i < (int)gameInfo->GetNumPlayingTeams(); i++)
    {
        TeamStats teamStats = gameInfo->GetTeamStatsByIndex((unsigned short)i);

        if ((unsigned char)bIsHuman)
        {
            unsigned short humanTeams = gameInfo->mCurrentCup->mHumanTeams;
            if (humanTeams & (1 << (int)teamStats.mTeamIndex))
            {
                playerStats[numValid++] = teamStats.mPlayerTotalStats;
            }
        }
        else
        {
            playerStats[numValid++] = teamStats.mPlayerTotalStats;
        }
    }

    int sortedIndices[8];
    nlSingleton<StatsTracker>::s_pInstance->GetSortedStats(
        playerStats, numTeams, sortedIndices, numTeams, (ePlayerStats)1, (eSortOrder)1);

    int* pSorted = sortedIndices;
    pStats = playerStats;

    for (int j = 0; j < numTeams; pSorted++, j++)
    {
        unsigned long teamNameHash = GetLOCTeamName(pStats[pSorted[0]].mRecordType.mTeamID);

        unsigned long formatHash;
        if (j == 0)
        {
            formatHash = 0xD517194DUL;
        }
        else
        {
            formatHash = 0x1DB17A6DUL;
        }

        NLString goalsStr = LexicalCast<NLString, int>((int)pStats[pSorted[0]].mNumGoalsFor);

        unsigned short wideGoals[16];
        nlStrToWcs(goalsStr.c_str(), wideGoals, 16);

        result = result.Append(Format<WideBasicString, const unsigned short*, unsigned short[16]>(
            WideBasicString(CupTickerLookupLocString(formatHash)), CupTickerLookupLocString(teamNameHash), wideGoals));
    }
}
