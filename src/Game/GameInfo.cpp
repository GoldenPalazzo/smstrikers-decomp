#include "Game/GameInfo.h"
#include "dolphin/types.h"
#include "NL/nlMemory.h"

#include "Game/GameSceneManager.h"
#include "Game/SH/SHMilestoneTrophy.h"
#include "Game/SH/SHCupHub.h"
#include "Game/Audio/WorldAudio.h"

extern bool g_e3_Build;

bool isFreezingUnlocked = false;
bool isShellsUnlocked = false;
bool isSuperTeamUnlocked = false;
bool isLegendUnlocked = false;
bool isEnhanceUnlocked = false;
bool isGiantUnlocked = false;
bool isExplosiveUnlocked = false;
bool isUnlimitedUnlocked = false;
bool isGoalieUnlocked = false;
bool isTiltUnlocked = false;
bool isAllSTSUnlocked = false;
bool isKongaUnlocked = false;
bool isYoshiUnlocked = false;
bool isForbiddenUnlocked = false;
bool isSuperStadUnlocked = false;

bool inline CheckUnlockStatus(const bool& globalFlag, const unsigned char& trophyValue, const unsigned int bit)
{
    if (!globalFlag)
    {
        if (GetConfigBool(Config::Global(), "givealltrophies", false))
        {
            return true;
        }
        else
        {
            return (trophyValue >> bit) & 0x01;
        }
    }
    return globalFlag;
}

/**
 * Offset/Address/Size: 0x9E90 | 0x8017F534 | size: 0xB84
 * TODO: 95.55% repo match (93.8% decomp.me). Remaining diff: -inline deferred
 *       interleaves body stores (mCurrentMode, mDemoEnabled, etc.) between implicit
 *       member ctor calls (CustomTournament, TeamStats, AudioSettings). Also
 *       placement new beq pattern and Friendly/Demo re-store register allocation.
 */
GameInfoManager::GameInfoManager()
    : mDoingKnockout(false)
    , mDidRoundJustEnd(false)
{
    mCurrentMode = GM_INVALID;
    mDemoEnabled = true;
    mIsInStrikers101Mode = false;
    mGoToChooseCaptains = false;
    mMainUserPadNumber = (eFEINPUT_PAD)0;
    mCurrentCup = NULL;
    mPreviousCup = NULL;

    mUseCurGameSettings = false;
    mLastHumanStadium = (eStadiumID)-1;

    for (int i = 0; i < GM_NUM_MODES; i++)
    {
        mGameInfo[i] = NULL;
    }

    for (int i = 0; i < 8; i++)
    {
        mUserInfo.mSpoils[i].mNumRecords = 0;
        mUserInfo.mSpoils[i].mCurrentChamp = (eTeamID)-1;
        mUserInfo.mSpoils[i].mNumLosses = 0;
        mUserInfo.mSpoils[i].mNumWins = 0;
    }

    BasicGameInfo* pFriendly = new (nlMalloc(sizeof(BasicGameInfo), 8, false)) BasicGameInfo();
    mGameInfo[GM_FRIENDLY] = pFriendly;
    pFriendly->mTeamIndex[0] = (eTeamID)3;
    pFriendly->mTeamIndex[1] = (eTeamID)2;
    pFriendly->mSidekickIndex[0] = (eSidekickID)0;
    pFriendly->mSidekickIndex[1] = (eSidekickID)1;
    pFriendly->mStadiumIndex = (eStadiumID)0;
    pFriendly->mPadSides[0] = -1;
    pFriendly->mPadSides[1] = -1;
    pFriendly->mPadSides[2] = -1;
    pFriendly->mPadSides[3] = -1;
    pFriendly->mFinalScore[0] = 0;
    pFriendly->mFinalScore[1] = 0;

    BasicGameInfo* pDemo = new (nlMalloc(sizeof(BasicGameInfo), 8, false)) BasicGameInfo();
    mGameInfo[GM_DEMO] = pDemo;
    pDemo->mTeamIndex[0] = (eTeamID)3;
    pDemo->mTeamIndex[1] = (eTeamID)2;
    pDemo->mSidekickIndex[0] = (eSidekickID)0;
    pDemo->mSidekickIndex[1] = (eSidekickID)1;
    pDemo->mStadiumIndex = (eStadiumID)0;
    pDemo->mPadSides[0] = -1;
    pDemo->mPadSides[1] = -1;
    pDemo->mPadSides[2] = -1;
    pDemo->mPadSides[3] = -1;
    pDemo->mFinalScore[0] = 0;
    pDemo->mFinalScore[1] = 0;

    mMushroomCupSeries.mRoundNumber = -6;
    mFlowerCupSeries.mRoundNumber = -6;
    mStarCupSeries.mRoundNumber = -6;
    mBowserCupSeries.mRoundNumber = -6;
    mSuperMushroomCupSeries.mRoundNumber = -6;
    mSuperFlowerCupSeries.mRoundNumber = -6;
    mSuperStarCupSeries.mRoundNumber = -6;
    mSuperBowserCupSeries.mRoundNumber = -6;

    bool skipFE = GetConfigBool(Config::Global(), "skipfe", false);
    if (skipFE)
    {
        SetMode(GM_FRIENDLY);
        mGameInfo[mCurrentMode]->mTeamIndex[0] = (eTeamID)3;
        mGameInfo[mCurrentMode]->mTeamIndex[1] = (eTeamID)2;
        mGameInfo[mCurrentMode]->mStadiumIndex = (eStadiumID)1;
        bool dontSetSides = GetConfigBool(Config::Global(), "dont_set_sides_when_skipfe", false);
        if (!dontSetSides)
        {
            if (g_pFEInput->IsConnected((eFEINPUT_PAD)0))
            {
                mGameInfo[mCurrentMode]->mPadSides[0] = 0;
            }
            if (g_pFEInput->IsConnected((eFEINPUT_PAD)1))
            {
                mGameInfo[mCurrentMode]->mPadSides[1] = 1;
            }
            if (g_pFEInput->IsConnected((eFEINPUT_PAD)2))
            {
                mGameInfo[mCurrentMode]->mPadSides[2] = 0;
            }
            if (g_pFEInput->IsConnected((eFEINPUT_PAD)3))
            {
                mGameInfo[mCurrentMode]->mPadSides[3] = 1;
            }
        }
    }

    mUserInfo.mSaveID = nlRandom(0xFFFFFFFF, &nlDefaultSeed);
    memset(mUserInfo.mTrophies, 0, sizeof(mUserInfo.mTrophies));
    mUserInfo.mIsFlowerCupUnlocked = false;
    mUserInfo.mIsStarCupUnlocked = false;
    mUserInfo.mNumGamesPlayed = 0;
    mUserInfo.mNumGoalsScored = 0;
    mUserInfo.mNumHits = 0;
    mUserInfo.mNumPerfectPasses = 0;
    mUserInfo.mNumSTSAttempts = 0;

    for (int i = 0; i < 4; i++)
    {
        memset(&mUserStats[i], 0, sizeof(PlayerStats));
        mUserStats[i].mRecordType.mControllerID = i;
        mUserStats[i].mType = TYPE_USER;
    }

    mUserInfo.mAudioOptions.InitializeDefaults();
    mUserInfo.mVisualOptions.InitializeDefaults();
    mUserInfo.mGameplayOptions.InitializeDefaults();
    mUserInfo.mPowerupOptions.InitializeDefaults();
    mUserInfo.mCheatOptions.InitializeDefaults();
}

/**
 * Offset/Address/Size: 0x9E1C | 0x8017F4C0 | size: 0x74
 */
GameInfoManager::~GameInfoManager()
{
    delete mGameInfo[0];
    delete mGameInfo[10];
}

/**
 * Offset/Address/Size: 0x9DEC | 0x8017F490 | size: 0x30
 */
eTeamID GameInfoManager::GetTeam(short homeaway) const
{
    if (mGameInfo[mCurrentMode] == nullptr)
        return TEAM_INVALID;
    return mGameInfo[mCurrentMode]->mTeamIndex[homeaway];
}

/**
 * Offset/Address/Size: 0x9DCC | 0x8017F470 | size: 0x20
 */
void GameInfoManager::SetTeam(short homeaway, eTeamID teamid)
{
    mGameInfo[mCurrentMode]->mTeamIndex[homeaway] = teamid;
}

/**
 * Offset/Address/Size: 0x9D94 | 0x8017F438 | size: 0x38
 */
eSidekickID GameInfoManager::GetSidekick(short homeaway) const
{
    if (mGameInfo[mCurrentMode]->mTeamIndex[homeaway] == TEAM_MYSTERY)
        return SK_MYSTERY;
    return mGameInfo[mCurrentMode]->mSidekickIndex[homeaway];
}

/**
 * Offset/Address/Size: 0x9D70 | 0x8017F414 | size: 0x24
 */
void GameInfoManager::SetSidekick(short homeaway, eSidekickID sidekickid)
{
    mGameInfo[mCurrentMode]->mSidekickIndex[homeaway] = sidekickid;
}

/**
 * Offset/Address/Size: 0x9D24 | 0x8017F3C8 | size: 0x4C
 */
u16 GameInfoManager::GetNumPlayingTeams() const
{
    if (mCurrentMode == 0x4 || mCurrentMode == 0x8)
    {
        return 8;
    }

    return mCurrentCup->GetNumTeams();
}

/**
 * Offset/Address/Size: 0x9CF4 | 0x8017F398 | size: 0x30
 */
u16 GameInfoManager::GetNumRounds() const
{
    return mCurrentCup->GetNumRounds();
}

/**
 * Offset/Address/Size: 0x9AB8 | 0x8017F15C | size: 0x23C
 */
TeamStats GameInfoManager::GetTeamStatsByIndex(unsigned short index)
{
    if (mCurrentMode == GM_BOWSER_CUP)
    {
        return *mBowserCupSeries.GetTeamStats(index);
    }
    else if (mCurrentMode == GM_SUPER_BOWSER_CUP)
    {
        return *mSuperBowserCupSeries.GetTeamStats(index);
    }
    else
    {
        return *mCurrentCup->GetTeamStats(index);
    }
}

/**
 * Offset/Address/Size: 0x9A38 | 0x8017F0DC | size: 0x80
 */
TeamStats* GameInfoManager::pGetTeamStatsByIndex(unsigned short index)
{
    if (mCurrentMode == GM_BOWSER_CUP)
    {
        return mBowserCupSeries.GetTeamStats(index);
    }
    else if (mCurrentMode == GM_SUPER_BOWSER_CUP)
    {
        return mSuperBowserCupSeries.GetTeamStats(index);
    }
    else
    {
        return mCurrentCup->GetTeamStats(index);
    }
}

/**
 * Offset/Address/Size: 0x96B4 | 0x8017ED58 | size: 0x384
 */
void GameInfoManager::SetPreviousTeamStats()
{
    int i;

    if (IsInCupOrTournamentMode())
    {
        for (i = 0; i < GetNumPlayingTeams(); i++)
        {
            mPreviousTeamStats[i] = GetTeamStatsByIndex(i);
        }
    }
}

/**
 * Offset/Address/Size: 0x969C | 0x8017ED40 | size: 0x18
 */
eStadiumID GameInfoManager::GetStadium() const
{
    return mGameInfo[mCurrentMode]->mStadiumIndex;
}

/**
 * Offset/Address/Size: 0x9574 | 0x8017EC18 | size: 0x128
 */
void GameInfoManager::GetMatchupInfo(short round, unsigned short matchup) const
{
    BaseCup* pCup;
    eGameModes mode;
    pCup = mCurrentCup;
    mode = mCurrentMode;

    if (mode == GM_BOWSER_CUP || mode == GM_SUPER_BOWSER_CUP)
    {
        if (round == -3)
        {
            round = 0;
        }
        else if (round == -2 || round == -5 || round == -1)
        {
            round = 1;
        }
        else
        {
            if (mDoingKnockout)
            {
                pCup = mPreviousCup;
            }
        }
    }
    else
    {
        if (round == -4)
        {
            round = mCurrentCup->GetNumRounds() - 3;
        }
        else if (round == -3)
        {
            round = mCurrentCup->GetNumRounds() - 2;
        }
        else if (round == -2)
        {
            round = mCurrentCup->GetNumRounds() - 1;
        }
    }

    pCup->GetGameInfo(round, matchup);
}

/**
 * Offset/Address/Size: 0x953C | 0x8017EBE0 | size: 0x38
 */
void GameInfoManager::SetUserSelectedCupTeam(eTeamID team)
{
    mCurrentCup->mUserSelectedTeam = team;

    if (team != TEAM_INVALID)
    {
        mCurrentCup->mHumanTeams = 0;
        mCurrentCup->mHumanTeams = (s32)(mCurrentCup->mHumanTeams | (1 << team));
    }
}

/**
 * Offset/Address/Size: 0x9530 | 0x8017EBD4 | size: 0xC
 */
void GameInfoManager::SetUserSelectedCupSidekick(eSidekickID sidekick)
{
    mCurrentCup->mUserSelectedSidekick = sidekick;
}

/**
 * Offset/Address/Size: 0x951C | 0x8017EBC0 | size: 0x14
 */
eUserGameResult GameInfoManager::GetResultsOfLastUserGame() const
{
    return mUserLastResults[mCurrentMode];
}

/**
 * Offset/Address/Size: 0x9508 | 0x8017EBAC | size: 0x14
 */
void GameInfoManager::SetResultsOfLastUserGame(eUserGameResult result)
{
    mUserLastResults[mCurrentMode] = result;
}

/**
 * Offset/Address/Size: 0x94FC | 0x8017EBA0 | size: 0xC
 */
s16 GameInfoManager::GetCurrentRoundNumber() const
{
    return mCurrentCup->mRoundNumber;
}

/**
 * Offset/Address/Size: 0x9300 | 0x8017E9A4 | size: 0x1FC
 */
/**
 * TODO: 97.17% match - register allocation diffs (r30 vs r3/r0) in simple return paths
 * and comparison operand register swap at 0x1C4-0x1CC. Likely -inline deferred flag issue.
 */
s16 GameInfoManager::GetNextRoundNumber(short roundParam) const
{
    s16 round;

    if (roundParam == -7)
    {
        round = mCurrentCup->mRoundNumber;
    }
    else
    {
        round = roundParam;
    }

    if (round == -6)
    {
        round = 0;
    }
    else if (round == -4)
    {
        round = -3;
    }
    else if (round == -3)
    {
        round = -2;
    }
    else if (round == -2)
    {
        if (mCurrentMode == GM_BOWSER_CUP && !CheckUnlockStatus(isSuperTeamUnlocked, mUserInfo.mTrophies[0], 3))
        {
            round = -1;
        }
        else
        {
            round = -5;
        }
    }
    else if (round == -1)
    {
        round = -5;
    }
    else
    {
        if ((mCurrentMode == GM_BOWSER_CUP || mCurrentMode == GM_SUPER_BOWSER_CUP) && mDoingKnockout && round == mPreviousCup->GetNumRounds() - 1)
        {
            round = -3;
        }
        else if (round + 1 == mCurrentCup->GetNumRounds())
        {
            round = -5;
        }
        else
        {
            round++;
        }
    }

    return round;
}

/**
 * Offset/Address/Size: 0x91E8 | 0x8017E88C | size: 0x118
 */
s16 GameInfoManager::GetPreviousRoundNumber(short roundParam) const
{
    eGameModes mode;

    if (roundParam == -7)
    {
        roundParam = mCurrentCup->mRoundNumber;
    }

    mode = mCurrentMode;

    if ((roundParam == -5) && (!mDoingKnockout))
    {
        roundParam = mCurrentCup->GetNumRounds() - 1;
    }
    else if ((roundParam == -5) && (mDoingKnockout))
    {
        roundParam = -2;
    }
    else if ((roundParam == -3) && (mode == GM_BOWSER_CUP || mode == GM_SUPER_BOWSER_CUP))
    {
        roundParam = mPreviousCup->GetNumRounds() - 1;
    }
    else if ((roundParam == -3) && (mode != GM_BOWSER_CUP && mode != GM_SUPER_BOWSER_CUP))
    {
        roundParam = -4;
    }
    else if (roundParam == -2)
    {
        roundParam = -3;
    }
    else if (roundParam == -1)
    {
        roundParam = -2;
    }
    else
    {
        roundParam--;
    }
    return roundParam;
}

/**
 * Offset/Address/Size: 0x9180 | 0x8017E824 | size: 0x68
 */
signed short GameInfoManager::GetFirstRoundNumber() const
{
    if ((mCurrentMode == GM_TOURNAMENT) && (mCustomTournamentInfo.m_tournMode == TM_KNOCKOUT))
    {
        s16 result;
        u16 numRounds = mCurrentCup->GetNumRounds();
        result = -4;
        if (numRounds == 2)
        {
            result = -3;
        }
        return (s16)result;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x90AC | 0x8017E750 | size: 0xD4
 */
u16 GameInfoManager::GetNumGamesPerRound(int round) const
{
    unsigned short returnValue;

    if (round == -4)
    {
        return 4;
    }

    if (round == -3)
    {
        return 2;
    }

    if (round == -2 || round == -1)
    {
        return 1;
    }

    if (round == -5)
    {
        if (mDoingKnockout)
        {
            return 1;
        }
    }

    if (mDoingKnockout)
    {
        returnValue = mPreviousCup->GetNumTeams() >> 1;
    }
    else
    {
        unsigned short temp;
        if (mCurrentMode == GM_BOWSER_CUP || mCurrentMode == GM_SUPER_BOWSER_CUP)
        {
            temp = 8;
        }
        else
        {
            temp = mCurrentCup->GetNumTeams();
        }
        returnValue = temp >> 1;
    }
    return returnValue;
}
/**
 * Offset/Address/Size: 0x90A0 | 0x8017E744 | size: 0xC
 */
eTeamID GameInfoManager::GetUserSelectedCupTeam() const
{
    return mCurrentCup->mUserSelectedTeam;
}

/**
 * Offset/Address/Size: 0x9088 | 0x8017E72C | size: 0x18
 */
void GameInfoManager::SetStadium(eStadiumID stadiumID)
{
    mGameInfo[mCurrentMode]->mStadiumIndex = stadiumID;
}

/**
 * Offset/Address/Size: 0x8C28 | 0x8017E2CC | size: 0x460
 */
eStadiumID GameInfoManager::PickStadium(bool isLastRound, eStadiumID excludeStadium) const
{
    eStadiumID returnValue;
    eStadiumID lastRoundStadium;

    if (g_e3_Build)
    {
        return STAD_WARIO_STADIUM;
    }

    lastRoundStadium = STAD_INVALID;

    bool kongaUnlocked = CheckUnlockStatus(isKongaUnlocked, mUserInfo.mTrophies[0], 0);
    bool yoshiUnlocked = CheckUnlockStatus(isYoshiUnlocked, mUserInfo.mTrophies[0], 1);
    bool forbiddenUnlocked = CheckUnlockStatus(isForbiddenUnlocked, mUserInfo.mTrophies[0], 2);
    bool superStadiumUnlocked = CheckUnlockStatus(isSuperStadUnlocked, mUserInfo.mTrophies[0], 3);

    eGameModes mode = mCurrentMode;
    if (mode == GM_TOURNAMENT)
    {
        isLastRound = false;
    }

    switch (mCurrentMode)
    {
    case GM_MUSHROOM_CUP:
        lastRoundStadium = STAD_PEACH_TOAD_STADIUM;
        break;
    case GM_FLOWER_CUP:
        lastRoundStadium = STAD_MARIO_STADIUM;
        break;
    case GM_STAR_CUP:
        lastRoundStadium = STAD_WARIO_STADIUM;
        break;
    case GM_SUPER_MUSHROOM_CUP:
        lastRoundStadium = STAD_YOSHI_STADIUM;
        break;
    case GM_SUPER_FLOWER_CUP:
        lastRoundStadium = STAD_DK_DAISY;
        break;
    case GM_SUPER_STAR_CUP:
        lastRoundStadium = STAD_FORBIDDEN_DOME;
        break;
    case GM_BOWSER_CUP:
    case GM_SUPER_BOWSER_CUP:
        lastRoundStadium = STAD_SUPER_STADIUM;
        break;
    default:
        break;
    }

    if (isLastRound)
    {
        return lastRoundStadium;
    }

    while (true)
    {
        returnValue = (eStadiumID)nlRandom(7, &nlDefaultSeed);

        if (returnValue == lastRoundStadium)
        {
            bool shouldRepick;
            eGameModes mode2 = mCurrentMode;

            if (mode2 < GM_SUPER_MUSHROOM_CUP)
            {
                if (mode2 >= GM_MUSHROOM_CUP)
                {
                    shouldRepick = true;
                }
                else
                {
                    shouldRepick = false;
                }
            }
            else
            {
                shouldRepick = false;
            }

            if (shouldRepick == false)
            {
                if (mode2 < GM_TOURNAMENT)
                {
                    if (mode2 >= GM_SUPER_MUSHROOM_CUP)
                    {
                        shouldRepick = true;
                    }
                    else
                    {
                        shouldRepick = false;
                    }
                }
                else
                {
                    shouldRepick = false;
                }
            }

            if (shouldRepick)
            {
                continue;
            }
        }

        if (returnValue == excludeStadium)
        {
            continue;
        }

        if ((u32)returnValue <= STAD_PEACH_TOAD_STADIUM)
        {
            break;
        }

        if (returnValue == STAD_WARIO_STADIUM)
        {
            break;
        }

        if (returnValue == STAD_DK_DAISY)
        {
            if (kongaUnlocked)
            {
                break;
            }
        }

        if (returnValue == STAD_YOSHI_STADIUM)
        {
            if (yoshiUnlocked)
            {
                break;
            }
        }

        if (returnValue == STAD_FORBIDDEN_DOME)
        {
            if (forbiddenUnlocked)
            {
                break;
            }
        }

        if (returnValue != STAD_SUPER_STADIUM)
        {
            continue;
        }

        if (superStadiumUnlocked == false)
        {
            continue;
        }

        break;
    }

    return returnValue;
}

/**
 * Offset/Address/Size: 0x8C08 | 0x8017E2AC | size: 0x20
 */
s16 GameInfoManager::GetPlayingSide(unsigned short padnumber) const
{
    return mGameInfo[mCurrentMode]->mPadSides[padnumber];
}

/**
 * Offset/Address/Size: 0x8BE8 | 0x8017E28C | size: 0x20
 */
void GameInfoManager::SetPlayingSide(unsigned short padnumber, short side)
{
    mGameInfo[mCurrentMode]->mPadSides[padnumber] = side;
}

/**
 * Offset/Address/Size: 0x8B38 | 0x8017E1DC | size: 0xB0
 */
u16 GameInfoManager::GetNumPlayers() const
{
    BasicGameInfo* const* pInfo = &mGameInfo[mCurrentMode];
    u16 count = 0;

    for (int i = 0; i < 4; i++)
    {
        if ((*pInfo)->mPadSides[(u16)i] != -1)
        {
            count++;
        }
    }

    return count;
}

/**
 * Offset/Address/Size: 0x8B10 | 0x8017E1B4 | size: 0x28
 */
void GameInfoManager::ResetPlayingSides()
{
    BasicGameInfo* gameInfo = mGameInfo[mCurrentMode];
    gameInfo->mPadSides[0] = -1;
    gameInfo->mPadSides[1] = -1;
    gameInfo->mPadSides[2] = -1;
    gameInfo->mPadSides[3] = -1;
}

/**
 * Offset/Address/Size: 0x8638 | 0x8017DCDC | size: 0x4D8
 */
void GameInfoManager::SetupRoundRobinSchedule(eTeamID*, eSidekickID*)
{
}

/**
 * Offset/Address/Size: 0x81CC | 0x8017D870 | size: 0x46C
 */
/**
 * Offset/Address/Size: 0x7EF0 + 0x380 | 0x8017D870 | size: 0x46C
 * TODO: 99.89% match - first loop body uses r5/r4/r0 register assignment order for
 * sidekick/team loads instead of target r4/r0/r5 order.
 */
static const int BOWSER_KNOCKOUT_ORDER[4] = { 0, 3, 1, 2 };

unsigned char GameInfoManager::SetupBowserKnockout()
{
    int teamIndices[8];
    eSidekickID sidekicks[9];
    unsigned char returnValue = 0;
    int numPreviousTeams;
    int numTeams;
    StatsTracker* pStatsTracker;

    mLastHumanStadium = STAD_INVALID;

    for (int i = 0; i < (u16)mPreviousCup->GetNumTeams() / 2; i++)
    {
        BasicGameInfo* g = mPreviousCup->GetGameInfo(0, i);
        eTeamID index = g->mTeamIndex[0];
        sidekicks[index] = g->mSidekickIndex[0];
        index = g->mTeamIndex[1];
        sidekicks[index] = g->mSidekickIndex[1];
    }

    *mCurrentCup->GetRoundResults(0) = 1;
    *mCurrentCup->GetRoundResults(1) = 1;
    *mCurrentCup->GetRoundResults(2) = 1;

    mCurrentCup->mUserSelectedTeam = mPreviousCup->mUserSelectedTeam;
    mCurrentCup->mUserSelectedSidekick = mPreviousCup->mUserSelectedSidekick;

    pStatsTracker = nlSingleton<StatsTracker>::s_pInstance;
    numTeams = mCurrentCup->GetNumTeams();
    numPreviousTeams = mPreviousCup->GetNumTeams();

    pStatsTracker->GetSortedTeamStats(mPreviousCup->GetTeamStats(0), numPreviousTeams, teamIndices, numTeams);

    for (int i = 0; i < (u16)mCurrentCup->GetNumTeams(); i++)
    {
        TeamStats* pSourceStats = mPreviousCup->GetTeamStats(teamIndices[BOWSER_KNOCKOUT_ORDER[i]]);
        TeamStats* pDestStats = mCurrentCup->GetTeamStats(i);
        *pDestStats = *pSourceStats;

        if (mCurrentCup->GetTeamStats(i)->mTeamIndex == mCurrentCup->mUserSelectedTeam)
        {
            returnValue = 1;
        }
    }

    BasicGameInfo* g = mCurrentCup->GetGameInfo(0, 0);
    g->mTeamIndex[0] = TEAM_MARIO;
    g->mTeamIndex[1] = TEAM_LUIGI;
    g->mSidekickIndex[0] = SK_TOAD;
    g->mSidekickIndex[1] = SK_KOOPA;
    g->mFinalScore[1] = 0;
    g->mFinalScore[0] = 0;
    g->mPadSides[0] = -1;
    g->mPadSides[1] = -1;
    g->mPadSides[2] = -1;
    g->mPadSides[3] = -1;
    g->mStadiumIndex = STAD_MARIO_STADIUM;

    eTeamID home = mCurrentCup->GetTeamStats(0)->mTeamIndex;
    eTeamID away = mCurrentCup->GetTeamStats(1)->mTeamIndex;
    g->mTeamIndex[0] = home;
    g->mTeamIndex[1] = away;
    g->mSidekickIndex[0] = sidekicks[home];
    g->mSidekickIndex[1] = sidekicks[away];
    g->mStadiumIndex = PickStadium(true, STAD_INVALID);

    g = mCurrentCup->GetGameInfo(0, 1);
    g->mTeamIndex[0] = TEAM_MARIO;
    g->mTeamIndex[1] = TEAM_LUIGI;
    g->mSidekickIndex[0] = SK_TOAD;
    g->mSidekickIndex[1] = SK_KOOPA;
    g->mFinalScore[1] = 0;
    g->mFinalScore[0] = 0;
    g->mPadSides[0] = -1;
    g->mPadSides[1] = -1;
    g->mPadSides[2] = -1;
    g->mPadSides[3] = -1;
    g->mStadiumIndex = STAD_MARIO_STADIUM;

    home = mCurrentCup->GetTeamStats(2)->mTeamIndex;
    away = mCurrentCup->GetTeamStats(3)->mTeamIndex;
    g->mTeamIndex[0] = home;
    g->mTeamIndex[1] = away;
    g->mSidekickIndex[0] = sidekicks[home];
    g->mSidekickIndex[1] = sidekicks[away];
    g->mStadiumIndex = PickStadium(true, STAD_INVALID);

    mCurrentCup->mCupStarted = true;

    return returnValue;
}

extern eStadiumID PickStadium__15GameInfoManagerCFb10eStadiumID(const GameInfoManager*, bool, eStadiumID);

/**
 * Offset/Address/Size: 0x7EF0 | 0x8017D594 | size: 0x2DC
 */
void GameInfoManager::SetupTournamentKnockout(eTeamID* pTeamIDs, eSidekickID* pSidekickIDs)
{
    int zero = 0;
    int numGames;
    s16 roundParam = mCurrentCup->mRoundNumber;

    if (roundParam == -4)
    {
        numGames = 4;
    }
    else if (roundParam == -3)
    {
        numGames = 2;
    }
    else if (roundParam == -2 || roundParam == -1)
    {
        numGames = 1;
    }
    else if (roundParam == -5 && mDoingKnockout)
    {
        numGames = 1;
    }
    else
    {
        if (mDoingKnockout)
        {
            numGames = mPreviousCup->GetNumTeams() >> 1;
        }
        else
        {
            u16 temp;

            if (mCurrentMode == GM_BOWSER_CUP || mCurrentMode == GM_SUPER_BOWSER_CUP)
            {
                temp = 8;
            }
            else
            {
                temp = mCurrentCup->GetNumTeams();
            }

            numGames = temp >> 1;
        }
    }

    mCurrentCup->mCupStarted = true;
    int numGamesCount = numGames;
    mCurrentCup->mGameNumber = zero;
    mLastHumanStadium = STAD_INVALID;

    int numTeams = (u16)mCurrentCup->GetNumTeams();

    *mCurrentCup->GetRoundResults(0) = 1;
    *mCurrentCup->GetRoundResults(1) = 1;
    *mCurrentCup->GetRoundResults(2) = 1;

    eSidekickID* pSidekickInfo = pSidekickIDs;
    eTeamID* pTeamInfo = pTeamIDs;
    eTeamID* pTeamStatsID = pTeamIDs;

    for (int i = 0; i < numGamesCount; i++)
    {
        BasicGameInfo* pGameInfo = mCurrentCup->GetGameInfo(0, i);

        pGameInfo->mTeamIndex[0] = TEAM_MARIO;
        pGameInfo->mTeamIndex[1] = TEAM_LUIGI;
        pGameInfo->mSidekickIndex[0] = SK_TOAD;
        pGameInfo->mSidekickIndex[1] = SK_KOOPA;
        pGameInfo->mFinalScore[1] = zero;
        pGameInfo->mFinalScore[0] = zero;
        pGameInfo->mPadSides[0] = -1;
        pGameInfo->mPadSides[1] = -1;
        pGameInfo->mPadSides[2] = -1;
        pGameInfo->mPadSides[3] = -1;
        pGameInfo->mStadiumIndex = STAD_MARIO_STADIUM;

        {
            eTeamID team1 = pTeamInfo[1];
            eSidekickID sidekick0 = pSidekickInfo[0];
            eSidekickID sidekick1 = pSidekickInfo[1];
            eTeamID team0 = pTeamInfo[0];

            pGameInfo->mTeamIndex[0] = team0;
            pGameInfo->mTeamIndex[1] = team1;
            pGameInfo->mSidekickIndex[0] = sidekick0;
            pGameInfo->mSidekickIndex[1] = sidekick1;
        }

        {
            eStadiumID pickedStadium = PickStadium__15GameInfoManagerCFb10eStadiumID(this, false, mLastHumanStadium);
            pGameInfo->mStadiumIndex = pickedStadium;

            u16 humanTeams = mCurrentCup->mHumanTeams;
            if ((humanTeams & (1 << pGameInfo->mTeamIndex[0])) || (humanTeams & (1 << pGameInfo->mTeamIndex[1])))
            {
                mLastHumanStadium = pickedStadium;
            }
        }

        pTeamInfo += 2;
        pSidekickInfo += 2;
    }

    TeamStats* pStats = mCurrentCup->GetTeamStats(0);
    pTeamStatsID = pTeamIDs;

    for (int i = 0; i < numTeams; i++)
    {
        eTeamID team = *pTeamStatsID;

        memset(&pStats->mPlayerTotalStats, zero, 0x34);
        pStats->mPlayerTotalStats.mRecordType.mTeamID = team;
        pStats->mPlayerTotalStats.mType = TYPE_TEAM;
        pStats->mTeamIndex = team;
        pStats->mNumWins = zero;
        pStats->mNumLosses = zero;
        pStats->mNumOTLosses = zero;
        pStats->mNumPoints = zero;

        pTeamStatsID++;
        pStats++;
    }
}

/**
 * Offset/Address/Size: 0x78D8 | 0x8017CF7C | size: 0x618
 */
unsigned char GameInfoManager::SetupKnockoutRound(short)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x7534 | 0x8017CBD8 | size: 0x3A4
 * TODO: 99.61% match - dnmflags r29 vs r27 register allocation diff cascades
 * to flag extraction regs and inner temp regs (away r6 vs r8). All diffs register-only.
 */
unsigned char GameInfoManager::DetermineNextMatchups(int dnmflags)
{
    int round = mCurrentCup->mRoundNumber;

    if (round != -4)
    {
        if (round != -3)
        {
            if ((u32)(round + 2) > 1)
            {
                if (round != -5 || !mDoingKnockout)
                {
                    if (mDoingKnockout)
                    {
                        mPreviousCup->GetNumTeams();
                    }
                    else if (mCurrentMode != GM_BOWSER_CUP && mCurrentMode != GM_SUPER_BOWSER_CUP)
                    {
                        mCurrentCup->GetNumTeams();
                    }
                }
            }
        }
    }

    int userPad = mMainUserPadNumber;

    while (round != -5)
    {
        u16 numRounds = mCurrentCup->GetNumRounds();

        if (round == -4)
        {
            round = (u16)numRounds - 3;
        }

        if (round == -3)
        {
            round = (u16)numRounds - 2;
        }

        if (round == -2 || round == -1)
        {
            round = (u16)numRounds - 1;
        }

        BasicGameInfo* gameinfo = mCurrentCup->GetGameInfo(round, mCurrentCup->mGameNumber);
        eTeamID home = gameinfo->mTeamIndex[0];
        eTeamID away = gameinfo->mTeamIndex[1];
        mGameInfo[mCurrentMode] = gameinfo;

        if (dnmflags & 0x1)
        {
            u16 humanTeams = mCurrentCup->mHumanTeams;
            if ((humanTeams & (1 << home)) || (humanTeams & (1 << away)))
            {
                mGameInfo[mCurrentMode]->mPadSides[(u16)userPad] = (home != mCurrentCup->mUserSelectedTeam);
                return 1;
            }
        }

        if (dnmflags & 0x10)
        {
            nlSingleton<StatsTracker>::s_pInstance->SetBasicGameInfoPointer(gameinfo, true);
            nlSingleton<StatsTracker>::s_pInstance->SimulateGame();
            nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();
        }

        if (!(dnmflags & 0x2))
        {
            break;
        }

        mCurrentCup->mGameNumber++;

        if (mCurrentCup->mGameNumber == GetNumGamesPerRound(mCurrentCup->mRoundNumber) && (dnmflags & 0x4))
        {
            IncreaseRoundNumber();
        }

        if (mCurrentCup->mGameNumber == GetNumGamesPerRound(mCurrentCup->mRoundNumber))
        {
            if (dnmflags & 0x8)
            {
                break;
            }

            round = mCurrentCup->mRoundNumber;
        }
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x71F4 | 0x8017C898 | size: 0x340
 */
void GameInfoManager::IncreaseRoundNumber()
{
    mCurrentCup->mRoundNumber = GetNextRoundNumber(mCurrentCup->mRoundNumber);
    mDidRoundJustEnd = true;

    s16 round = mCurrentCup->mRoundNumber;

    do
    {
        if (round != -3)
            if (round != -2)
                if (round != -1)
                    break;
        if (!SetupKnockoutRound(round))
        {
            if (mCurrentCup->mRoundNumber == -3)
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_ELIMINATED_QUARTER;
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
                SetupKnockoutRound(-2);
                mCurrentCup->mRoundNumber = -2;
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
            }
            else if (mCurrentCup->mRoundNumber == -2)
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_ELIMINATED_SEMI;
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
            }

            mCurrentCup->mRoundNumber = -5;
        }
    } while (false);

    if (mCurrentCup->mRoundNumber == -5)
    {
        mCurrentCup->mCupStarted = false;
    }

    if (mCurrentCup->mRoundNumber == -5 && !mDoingKnockout)
    {
        if (mCurrentMode == GM_BOWSER_CUP)
        {
            mPreviousCup = mCurrentCup;
            mCurrentCup = &mBowserCupKnockout;
            mCurrentCup->mUserSelectedTeam = mPreviousCup->mUserSelectedTeam;
            mCurrentCup->mUserSelectedSidekick = mPreviousCup->mUserSelectedSidekick;
            mCurrentCup->mHumanTeams = mPreviousCup->mHumanTeams;
            mCurrentCup->mRoundNumber = -3;
            mCurrentCup->mCupSettings = mPreviousCup->mCupSettings;

            unsigned char bowserResult = SetupBowserKnockout();
            mDoingKnockout = true;
            if (bowserResult)
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_PLAYOFF_QUALIFIES;
            }
            else
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_DOES_NOT_PLAYOFF_QUALIFY;
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
                mCurrentCup->mRoundNumber = -2;
                SetupKnockoutRound(-2);
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
                mCurrentCup->mRoundNumber = -5;
            }
        }

        if (mCurrentMode == GM_SUPER_BOWSER_CUP)
        {
            mPreviousCup = mCurrentCup;
            mCurrentCup = &mSuperBowserCupKnockout;
            mCurrentCup->mUserSelectedTeam = mPreviousCup->mUserSelectedTeam;
            mCurrentCup->mUserSelectedSidekick = mPreviousCup->mUserSelectedSidekick;
            mCurrentCup->mHumanTeams = mPreviousCup->mHumanTeams;
            mCurrentCup->mRoundNumber = -3;
            mCurrentCup->mCupSettings = mPreviousCup->mCupSettings;

            unsigned char superResult = SetupBowserKnockout();
            mDoingKnockout = true;
            if (superResult)
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_PLAYOFF_QUALIFIES;
            }
            else
            {
                mUserLastResults[mCurrentMode] = RESULT_USER_DOES_NOT_PLAYOFF_QUALIFY;
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
                mCurrentCup->mRoundNumber = -2;
                SetupKnockoutRound(-2);
                nlSingleton<StatsTracker>::s_pInstance->SimulateRemainingGames();
                mCurrentCup->mRoundNumber = -5;
            }
        }
    }

    mCurrentCup->mGameNumber = 0;
}

/**
 * Offset/Address/Size: 0x70D8 | 0x8017C77C | size: 0x11C
 */
void GameInfoManager::IncreaseGameNumber(bool shouldIncreaseRound)
{
    mCurrentCup->mGameNumber++;

    s16 round = mCurrentCup->mRoundNumber;
    u16 maxGames;

    if (round == -4)
    {
        maxGames = 4;
    }
    else if (round == -3)
    {
        maxGames = 2;
    }
    else if (round == -2 || round == -1)
    {
        maxGames = 1;
    }
    else if (round == -5 && mDoingKnockout)
    {
        maxGames = 1;
    }
    else if (mDoingKnockout)
    {
        maxGames = mPreviousCup->GetNumTeams() >> 1;
    }
    else
    {
        u16 temp;
        if (mCurrentMode == GM_BOWSER_CUP || mCurrentMode == GM_SUPER_BOWSER_CUP)
        {
            temp = 8;
        }
        else
        {
            temp = mCurrentCup->GetNumTeams();
        }
        maxGames = temp >> 1;
    }

    if (mCurrentCup->mGameNumber == maxGames && shouldIncreaseRound)
    {
        IncreaseRoundNumber();
    }
}

/**
 * Offset/Address/Size: 0x6E20 | 0x8017C4C4 | size: 0x2B8
 */
int GameInfoManager::GetNumHumanTeams()
{
    int numHumanTeams = 0;
    int i = 0;

    while (i < GetNumPlayingTeams())
    {
        eTeamID teamIndex = GetTeamStatsByIndex(i).mTeamIndex;
        if (mCurrentCup->mHumanTeams & (1 << teamIndex))
        {
            numHumanTeams++;
        }

        i++;
    }

    return numHumanTeams;
}

/**
 * Offset/Address/Size: 0x6D70 | 0x8017C414 | size: 0xB0
 */
BaseCup* GameInfoManager::GetCup(GameInfoManager::eGameModes mode)
{
    BaseCup* result = NULL;

    switch (mode)
    {
    case GM_FRIENDLY:
        result = &mMushroomCupSeries;
        break;
    case GM_MUSHROOM_CUP:
        result = &mFlowerCupSeries;
        break;
    case GM_FLOWER_CUP:
        result = &mStarCupSeries;
        break;
    case GM_STAR_CUP:
        if (mBowserCupSeries.mRoundNumber == -5 && mBowserCupKnockout.mRoundNumber != -5)
        {
            result = &mBowserCupKnockout;
        }
        else
        {
            result = &mBowserCupSeries;
        }
        break;
    case GM_BOWSER_CUP:
        result = &mSuperMushroomCupSeries;
        break;
    case GM_SUPER_MUSHROOM_CUP:
        result = &mSuperFlowerCupSeries;
        break;
    case GM_SUPER_FLOWER_CUP:
        result = &mSuperStarCupSeries;
        break;
    case GM_SUPER_STAR_CUP:
        if (mSuperBowserCupSeries.mRoundNumber == -5 && mSuperBowserCupKnockout.mRoundNumber != -5)
        {
            result = &mSuperBowserCupKnockout;
        }
        else
        {
            result = &mSuperBowserCupSeries;
        }
        break;
    case GM_TOURNAMENT:
        result = mCustomTournamentInfo.m_cup;
        break;
    default:
        break;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x6908 | 0x8017BFAC | size: 0x468
 */
bool GameInfoManager::IsUserQualified(GameInfoManager::eGameModes mode) const
{
    bool qualified = false;

    switch (mode)
    {
    case GM_FLOWER_CUP:
        qualified = mUserInfo.mIsFlowerCupUnlocked || GetConfigBool(Config::Global(), "givealltrophies", false);
        break;

    case GM_STAR_CUP:
        qualified = mUserInfo.mIsStarCupUnlocked;

        if (qualified == false)
        {
            qualified = GetConfigBool(Config::Global(), "givealltrophies", false);
        }

        break;

    case GM_BOWSER_CUP:
        qualified = GetConfigBool(Config::Global(), "givealltrophies", false);

        if (qualified)
        {
            qualified = true;
        }
        else
        {
            qualified = (mUserInfo.mTrophies[0] >> 0) & 0x1;
        }

        if (qualified)
        {
            qualified = GetConfigBool(Config::Global(), "givealltrophies", false);

            if (qualified)
            {
                qualified = true;
            }
            else
            {
                qualified = (mUserInfo.mTrophies[0] >> 1) & 0x1;
            }
        }

        if (qualified)
        {
            qualified = GetConfigBool(Config::Global(), "givealltrophies", false);

            if (qualified)
            {
                qualified = true;
            }
            else
            {
                qualified = (mUserInfo.mTrophies[0] >> 2) & 0x1;
            }
        }

        break;

    case GM_NUM_MODES:
        qualified = GetConfigBool(Config::Global(), "givealltrophies", false);

        if (qualified)
        {
            qualified = true;
        }
        else
        {
            qualified = (mUserInfo.mTrophies[0] >> 3) & 0x1;
        }

        break;

    default:
        break;
    }

    return qualified;
}

/**
 * Offset/Address/Size: 0x6794 | 0x8017BE38 | size: 0x174
 */
void GameInfoManager::SetMode(GameInfoManager::eGameModes mode)
{
    mCurrentMode = mode;
    mCupMatchRequirement = RESULT_INVALID;
    mIsInStrikers101Mode = false;
    mDidRoundJustEnd = false;
    mUserLastResults[mode] = RESULT_INVALID;

    switch (mCurrentMode)
    {
    case GM_FRIENDLY:
        mCurrentCup = &mMushroomCupSeries;
        mDoingKnockout = false;
        return;

    case GM_MUSHROOM_CUP:
        mCurrentCup = &mFlowerCupSeries;
        mDoingKnockout = false;
        return;

    case GM_FLOWER_CUP:
        mCurrentCup = &mStarCupSeries;
        mDoingKnockout = false;
        return;

    case GM_STAR_CUP:
        mCurrentCup = &mBowserCupSeries;
        mDoingKnockout = false;

        if (mCurrentCup->mRoundNumber == -5 && mBowserCupKnockout.mRoundNumber != -5)
        {
            mPreviousCup = mCurrentCup;
            mCurrentCup = &mBowserCupKnockout;
            mDoingKnockout = true;
        }

        return;

    case GM_BOWSER_CUP:
        mCurrentCup = &mSuperMushroomCupSeries;
        mDoingKnockout = false;
        return;

    case GM_SUPER_MUSHROOM_CUP:
        mCurrentCup = &mSuperFlowerCupSeries;
        mDoingKnockout = false;
        return;

    case GM_SUPER_FLOWER_CUP:
        mCurrentCup = &mSuperStarCupSeries;
        mDoingKnockout = false;
        return;

    case GM_SUPER_STAR_CUP:
        mCurrentCup = &mSuperBowserCupSeries;
        mDoingKnockout = false;

        if (mCurrentCup->mRoundNumber == -5 && mSuperBowserCupKnockout.mRoundNumber != -5)
        {
            mPreviousCup = mCurrentCup;
            mCurrentCup = &mSuperBowserCupKnockout;
            mDoingKnockout = true;
        }

        return;

    case GM_TOURNAMENT:
        mCurrentCup = mCustomTournamentInfo.m_cup;

        if (mCustomTournamentInfo.m_tournMode == TM_LEAGUE)
        {
            mDoingKnockout = false;
        }
        else
        {
            mDoingKnockout = true;
        }

        if (!mCurrentCup->mCupStarted)
        {
            mCurrentCup->mUserSelectedTeam = TEAM_INVALID;
        }

        return;

    default:
        mCurrentCup = NULL;
        return;
    }
}

/**
 * Offset/Address/Size: 0x6664 | 0x8017BD08 | size: 0x130
 */
unsigned long GameInfoManager::GetMemoryCardDataSize() const
{
    unsigned long size = mMushroomCupSeries.GetSaveDataSize() + sizeof(UserInfo);
    size += mFlowerCupSeries.GetSaveDataSize();
    size += mStarCupSeries.GetSaveDataSize();
    size += mBowserCupSeries.GetSaveDataSize();
    size += mBowserCupKnockout.GetSaveDataSize();
    size += mSuperMushroomCupSeries.GetSaveDataSize();
    size += mSuperFlowerCupSeries.GetSaveDataSize();
    size += mSuperStarCupSeries.GetSaveDataSize();
    size += mSuperBowserCupSeries.GetSaveDataSize();
    size += mSuperBowserCupKnockout.GetSaveDataSize();
    size += mCustomTournamentInfo.GetSaveDataSize();
    return size;
}

/**
 * Offset/Address/Size: 0x642C | 0x8017BAD0 | size: 0x238
 */
void GameInfoManager::GetMemoryCardData(void* pData)
{
    memcpy(pData, &mUserInfo, sizeof(UserInfo));
    pData = (u8*)pData + sizeof(UserInfo);

    mMushroomCupSeries.SerializeData(pData);
    pData = (u8*)pData + mMushroomCupSeries.GetSaveDataSize();

    mFlowerCupSeries.SerializeData(pData);
    pData = (u8*)pData + mFlowerCupSeries.GetSaveDataSize();

    mStarCupSeries.SerializeData(pData);
    pData = (u8*)pData + mStarCupSeries.GetSaveDataSize();

    mBowserCupSeries.SerializeData(pData);
    pData = (u8*)pData + mBowserCupSeries.GetSaveDataSize();

    mBowserCupKnockout.SerializeData(pData);
    pData = (u8*)pData + mBowserCupKnockout.GetSaveDataSize();

    mSuperMushroomCupSeries.SerializeData(pData);
    pData = (u8*)pData + mSuperMushroomCupSeries.GetSaveDataSize();

    mSuperFlowerCupSeries.SerializeData(pData);
    pData = (u8*)pData + mSuperFlowerCupSeries.GetSaveDataSize();

    mSuperStarCupSeries.SerializeData(pData);
    pData = (u8*)pData + mSuperStarCupSeries.GetSaveDataSize();

    mSuperBowserCupSeries.SerializeData(pData);
    pData = (u8*)pData + mSuperBowserCupSeries.GetSaveDataSize();

    mSuperBowserCupKnockout.SerializeData(pData);
    pData = (u8*)pData + mSuperBowserCupKnockout.GetSaveDataSize();

    mCustomTournamentInfo.SerializeData(pData);
    mCustomTournamentInfo.GetSaveDataSize();
}

/**
 * Offset/Address/Size: 0x61F8 | 0x8017B89C | size: 0x234
 */
void GameInfoManager::SetMemoryCardData(void* pData)
{
    memcpy(&mUserInfo, pData, sizeof(UserInfo));
    pData = (u8*)pData + sizeof(UserInfo);

    mMushroomCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mMushroomCupSeries.GetSaveDataSize();

    mFlowerCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mFlowerCupSeries.GetSaveDataSize();

    mStarCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mStarCupSeries.GetSaveDataSize();

    mBowserCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mBowserCupSeries.GetSaveDataSize();

    mBowserCupKnockout.DeserializeData(pData);
    pData = (u8*)pData + mBowserCupKnockout.GetSaveDataSize();

    mSuperMushroomCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mSuperMushroomCupSeries.GetSaveDataSize();

    mSuperFlowerCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mSuperFlowerCupSeries.GetSaveDataSize();

    mSuperStarCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mSuperStarCupSeries.GetSaveDataSize();

    mSuperBowserCupSeries.DeserializeData(pData);
    pData = (u8*)pData + mSuperBowserCupSeries.GetSaveDataSize();

    mSuperBowserCupKnockout.DeserializeData(pData);
    pData = (u8*)pData + mSuperBowserCupKnockout.GetSaveDataSize();

    mCustomTournamentInfo.DeserializeData(pData);
    mCustomTournamentInfo.GetSaveDataSize();
}

/**
 * Offset/Address/Size: 0x61DC | 0x8017B880 | size: 0x1C
 */
u8 GameInfoManager::CheckSaveIDChanged(void* pData)
{
    return ((UserInfo*)pData)->mSaveID != mUserInfo.mSaveID;
}

/**
 * Offset/Address/Size: 0x60C0 | 0x8017B764 | size: 0x11C
 */
bool GameInfoManager::HasTrophy(eTrophyType trophyType) const
{
    if (GetConfigBool(Config::Global(), "givealltrophies", false))
    {
        return true;
    }

    u8 trophyValue = mUserInfo.mTrophies[trophyType / 8];
    int bit = trophyType % 8;
    return (trophyValue & (1 << bit)) != 0;
}

/**
 * Offset/Address/Size: 0x5F60 | 0x8017B604 | size: 0x160
 */
eMilestoneColour GameInfoManager::GetMilestoneLevel(eTrophyType trophy) const
{
    eMilestoneColour returnValue = INVALID_MILESTONE_COLOUR;

    switch (trophy)
    {
    case TROPHY_VETERAN_CUP:
        if (mUserInfo.mNumGamesPlayed < 25)
            returnValue = MILESTONE_BLACK;
        else if (mUserInfo.mNumGamesPlayed < 50)
            returnValue = MILESTONE_BRONZE;
        else if (mUserInfo.mNumGamesPlayed < 100)
            returnValue = MILESTONE_SILVER;
        else
            returnValue = MILESTONE_GOLD;
        break;
    case TROPHY_SNIPER_CUP:
        if (mUserInfo.mNumGoalsScored < 75)
            returnValue = MILESTONE_BLACK;
        else if (mUserInfo.mNumGoalsScored < 150)
            returnValue = MILESTONE_BRONZE;
        else if (mUserInfo.mNumGoalsScored < 300)
            returnValue = MILESTONE_SILVER;
        else
            returnValue = MILESTONE_GOLD;
        break;
    case TROPHY_STRIKER_CUP:
        if (mUserInfo.mNumSTSAttempts < 25)
            returnValue = MILESTONE_BLACK;
        else if (mUserInfo.mNumSTSAttempts < 50)
            returnValue = MILESTONE_BRONZE;
        else if (mUserInfo.mNumSTSAttempts < 100)
            returnValue = MILESTONE_SILVER;
        else
            returnValue = MILESTONE_GOLD;
        break;
    case TROPHY_TACTICIAN_CUP:
        if (mUserInfo.mNumPerfectPasses < 75)
            returnValue = MILESTONE_BLACK;
        else if (mUserInfo.mNumPerfectPasses < 150)
            returnValue = MILESTONE_BRONZE;
        else if (mUserInfo.mNumPerfectPasses < 300)
            returnValue = MILESTONE_SILVER;
        else
            returnValue = MILESTONE_GOLD;
        break;
    case TROPHY_PARAMEDIC_CUP:
        if (mUserInfo.mNumHits < 250)
            returnValue = MILESTONE_BLACK;
        else if (mUserInfo.mNumHits < 500)
            returnValue = MILESTONE_BRONZE;
        else if (mUserInfo.mNumHits < 1000)
            returnValue = MILESTONE_SILVER;
        else
            returnValue = MILESTONE_GOLD;
        break;
    }

    return returnValue;
}

/**
 * Offset/Address/Size: 0x5F38 | 0x8017B5DC | size: 0x28
 */
bool GameInfoManager::IsInRegularCupMode() const
{
    switch (mCurrentMode)
    {
    case GM_MUSHROOM_CUP:
    case GM_FLOWER_CUP:
    case GM_STAR_CUP:
    case GM_BOWSER_CUP:
        return true;
    default:
        return false;
    }
}

/**
 * Offset/Address/Size: 0x5F10 | 0x8017B5B4 | size: 0x28
 */
bool GameInfoManager::IsInSuperCupMode() const
{
    switch (mCurrentMode)
    {
    case GM_SUPER_MUSHROOM_CUP:
    case GM_SUPER_FLOWER_CUP:
    case GM_SUPER_STAR_CUP:
    case GM_SUPER_BOWSER_CUP:
        return true;
    default:
        return false;
    }
}

/**
 * Offset/Address/Size: 0x5EC0 | 0x8017B564 | size: 0x50
 */
bool GameInfoManager::IsInCupMode() const
{
    return IsInRegularCupMode() || IsInSuperCupMode();
}

/**
 * Offset/Address/Size: 0x5E5C | 0x8017B500 | size: 0x64
 */
bool GameInfoManager::IsInCupOrTournamentMode() const
{
    return IsInTournamentMode() || IsInCupMode();
}

/**
 * Offset/Address/Size: 0x5E48 | 0x8017B4EC | size: 0x14
 */
bool GameInfoManager::IsInDemoMode() const
{
    return mCurrentMode == GM_DEMO;
}

/**
 * Offset/Address/Size: 0x5E38 | 0x8017B4DC | size: 0x10
 */
bool GameInfoManager::IsInFriendlyMode() const
{
    return mCurrentMode == GM_FRIENDLY;
}

/**
 * Offset/Address/Size: 0x5E24 | 0x8017B4C8 | size: 0x14
 */
bool GameInfoManager::IsInTournamentMode() const
{
    return mCurrentMode == GM_TOURNAMENT;
}

/**
 * Offset/Address/Size: 0x5E1C | 0x8017B4C0 | size: 0x8
 */
AudioSettings& GameInfoManager::GetAudioOptions()
{
    return mUserInfo.mAudioOptions;
}

/**
 * Offset/Address/Size: 0x5DE0 | 0x8017B484 | size: 0x3C
 */
const GameplaySettings& GameInfoManager::GetGameplayOptions() const
{
    if (mUseCurGameSettings)
    {
        return mCurGameGameplayOptions;
    }

    if (mCurrentMode == GM_FRIENDLY || mCurrentMode == GM_DEMO)
    {
        return mUserInfo.mGameplayOptions;
    }

    return mCurrentCup->mCupSettings;
}

/**
 * Offset/Address/Size: 0x5DD8 | 0x8017B47C | size: 0x8
 */
const PowerupSettings& GameInfoManager::GetPowerupOptions() const
{
    FORCE_DONT_INLINE;
    return mUserInfo.mPowerupOptions;
}

/**
 * Offset/Address/Size: 0x5040 | 0x8017A6E4 | size: 0xD98
 */
void GameInfoManager::OnPreGameState()
{
}

/**
 * Offset/Address/Size: 0x500C | 0x8017A6B0 | size: 0x34
 */
void GameInfoManager::OnPostGameState()
{
    mUseCurGameSettings = false;
    mUserInfo.mAudioOptions.ApplySettings(true, false);
}

/**
 * Offset/Address/Size: 0x4E7C | 0x8017A520 | size: 0x190
 * TODO: 95.75% match - persistent r4/r5 register swap in the unrolled pad-side loop,
 *       plus one remaining lwzx/stw ordering difference in the DifficultyMap writeback.
 */
void GameInfoManager::ApplyDifficultySettings()
{
    static eDifficultyID DifficultyMap[5][2];
    unsigned char humansOnSide[2] = { 0, 0 };
    int i;
    GameplaySettings::eSkillLevel skillLevel;

    for (i = 0; i < 4; i++)
    {
        s16 padSide = mGameInfo[mCurrentMode]->mPadSides[(unsigned short)i];
        if (padSide == 0)
        {
            humansOnSide[0] = 1;
        }
        else if (padSide == 1)
        {
            humansOnSide[1] = 1;
        }
    }

    if (mIsInStrikers101Mode)
    {
        skillLevel = GameplaySettings::TRAINING;
    }
    else
    {
        GameplaySettings* settings;
        if (mUseCurGameSettings)
        {
            settings = &mCurGameGameplayOptions;
        }
        else if (mCurrentMode == GM_FRIENDLY || mCurrentMode == GM_DEMO)
        {
            settings = &mUserInfo.mGameplayOptions;
        }
        else
        {
            settings = &mCurrentCup->mCupSettings;
        }
        skillLevel = settings->SkillLevel;
    }

    mCurrentDifficulty[0] = DifficultyMap[skillLevel][humansOnSide[0] ? 0 : 1];
    mCurrentDifficulty[1] = DifficultyMap[skillLevel][humansOnSide[1] ? 0 : 1];
}

/**
 * Offset/Address/Size: 0x4E14 | 0x8017A4B8 | size: 0x68
 */
eTrophyType GameInfoManager::GetTrophyTypeByCurrentMode() const
{
    eTrophyType mode = INVALID_TROPHY;

    switch (mCurrentMode)
    {
    case GM_MUSHROOM_CUP:
        mode = TROPHY_MUSHROOM_CUP;
        break;
    case GM_FLOWER_CUP:
        mode = TROPHY_FLOWER_CUP;
        break;
    case GM_STAR_CUP:
        mode = TROPHY_STAR_CUP;
        break;
    case GM_BOWSER_CUP:
        mode = TROPHY_BOWSER_CUP;
        break;
    case GM_SUPER_MUSHROOM_CUP:
        mode = TROPHY_SUPER_MUSHROOM_CUP;
        break;
    case GM_SUPER_FLOWER_CUP:
        mode = TROPHY_SUPER_FLOWER_CUP;
        break;
    case GM_SUPER_STAR_CUP:
        mode = TROPHY_SUPER_STAR_CUP;
        break;
    case GM_SUPER_BOWSER_CUP:
        mode = TROPHY_SUPER_BOWSER_CUP;
        break;
    }

    return mode;
}

/**
 * Offset/Address/Size: 0x4DFC | 0x8017A4A0 | size: 0x18
 */
bool GameInfoManager::IsPossibleCupMatch() const
{
    return mCupMatchRequirement != RESULT_INVALID;
}

/**
 * Offset/Address/Size: 0x38E8 | 0x80178F8C | size: 0x1514
 */
void GameInfoManager::OnPreCupGameState()
{
}

/**
 * Offset/Address/Size: 0x256C | 0x80177C10 | size: 0x137C
 */
void GameInfoManager::OnPostCupGameState()
{
}

static eTrophyType MILESTONES[5] = {
    TROPHY_VETERAN_CUP,
    TROPHY_SNIPER_CUP,
    TROPHY_STRIKER_CUP,
    TROPHY_TACTICIAN_CUP,
    TROPHY_PARAMEDIC_CUP,
};

/**
 * Offset/Address/Size: 0x23FC | 0x80177AA0 | size: 0x170
 * TODO: 97.8% match - MWCC emits duplicated li r3,0 in nested/outer else paths instead of tail-merging to branch-shared false assignment
 */
void GameInfoManager::DetermineNextCupScreen()
{
    int i = 0;
    while (i < 5)
    {
        if (mDisplayTrophy[i + 1] == 1)
        {
            MilestoneTrophyScene* scene = (MilestoneTrophyScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MILESTONE_TROPHY, SCREEN_NOTHING, false);
            scene->CreateTrophyScene(MILESTONES[i], ButtonComponent::BS_A_ONLY, true);
            mDisplayTrophy[i + 1] = false;
            Audio::gWorldSFX.Play(Audio::WORLDSFX_FE_ACCEPT_WARIO, 100.0f, -1.0f, true, 100.0f);
            return;
        }
        i++;
    }

    if (mCurrentCup->mRoundNumber == -5)
    {
        TimeStampCupEnd();
    }

    bool isSuper;
    mDisplayTrophy[0] = (isSuper = true);
    if (mCurrentMode < GM_TOURNAMENT)
    {
        if (mCurrentMode >= GM_SUPER_MUSHROOM_CUP)
        {
        }
        else
        {
            isSuper = false;
        }
    }
    else
    {
        isSuper = false;
    }

    SceneList nextScene = isSuper ? SCENE_SUPER_CUP_STANDINGS_ANIM : SCENE_CUP_STANDINGS_ANIM;
    if (mCurrentCup->mRoundNumber == -1)
    {
        nextScene = SCENE_CUP_SUPER_TEAM;
    }
    if (nextScene != SCENE_CUP_SUPER_TEAM)
    {
        CupHubScene* hub = (CupHubScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(nextScene, SCREEN_NOTHING, false);
        hub->mDoAutoSave = true;
    }
    else
    {
        nlSingleton<GameSceneManager>::s_pInstance->Push(nextScene, SCREEN_NOTHING, false);
    }
}

/**
 * Offset/Address/Size: 0x1DA8 | 0x8017744C | size: 0x654
 */
signed char GameInfoManager::DetermineUserPlacement(Spoil*)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x196C | 0x80177010 | size: 0x43C
 * TODO: 84.71% match - stmw r26 vs r27, stack frame 0x80 vs 0x90.
 * Compiler puts userTeam in callee-saved r26 instead of spilling to stack.
 * Likely register allocation heuristic difference in decomp.me context.
 */
void GameInfoManager::TimeStampCupEnd()
{
    eTrophyType trophy = INVALID_TROPHY;

    switch (mCurrentMode)
    {
    case GM_MUSHROOM_CUP:
        trophy = TROPHY_MUSHROOM_CUP;
        break;
    case GM_FLOWER_CUP:
        trophy = TROPHY_FLOWER_CUP;
        break;
    case GM_STAR_CUP:
        trophy = TROPHY_STAR_CUP;
        break;
    case GM_BOWSER_CUP:
        trophy = TROPHY_BOWSER_CUP;
        break;
    case GM_SUPER_MUSHROOM_CUP:
        trophy = TROPHY_SUPER_MUSHROOM_CUP;
        break;
    case GM_SUPER_FLOWER_CUP:
        trophy = TROPHY_SUPER_FLOWER_CUP;
        break;
    case GM_SUPER_STAR_CUP:
        trophy = TROPHY_SUPER_STAR_CUP;
        break;
    case GM_SUPER_BOWSER_CUP:
        trophy = TROPHY_SUPER_BOWSER_CUP;
        break;
    default:
        break;
    }

    Spoil* pSpoil = &mUserInfo.mSpoils[trophy];
    OSCalendarTime calendar;
    signed char userPlace = 0;
    eTeamID userTeam = TEAM_INVALID;
    GameplaySettings::eSkillLevel skill = GameplaySettings::ROOKIE;
    CupRecord record;

    memset(&calendar, 0, sizeof(calendar));
    OSTicksToCalendarTime(OSGetTime(), &calendar);

    userTeam = mCurrentCup->mUserSelectedTeam;

    userPlace = DetermineUserPlacement(pSpoil);

    skill = mCurrentCup->mCupSettings.SkillLevel;

    record.mDate = calendar;
    record.mPlace = userPlace;
    record.mTeam = userTeam;
    record.mDifficulty = skill;

    if (pSpoil->mNumRecords < 10)
    {
        pSpoil->mNumRecords++;
    }

    for (int i = pSpoil->mNumRecords - 1; i > 0; --i)
    {
        pSpoil->mCupHistory[i] = pSpoil->mCupHistory[i - 1];
    }

    pSpoil->mCupHistory[0] = record;

    if (pSpoil->mNumWins > 999)
    {
        pSpoil->mNumWins = 999;
    }

    if (pSpoil->mNumLosses > 999)
    {
        pSpoil->mNumLosses = 999;
    }

    if (pSpoil->mNumCupWins > 999)
    {
        pSpoil->mNumCupWins = 999;
    }

    pSpoil->mCurrentChamp = FindWinningTeam();

    pSpoil->mIsCPUChamp = (mCurrentCup->mUserSelectedTeam != pSpoil->mCurrentChamp);

    if (mCurrentCup->mUserSelectedTeam == pSpoil->mCurrentChamp)
    {
        pSpoil->mNumCupWins++;

        if (pSpoil->mNumWins > 999)
        {
            pSpoil->mNumWins = 999;
        }

        if (pSpoil->mNumLosses > 999)
        {
            pSpoil->mNumLosses = 999;
        }

        if (pSpoil->mNumCupWins > 999)
        {
            pSpoil->mNumCupWins = 999;
        }
    }
}

/**
 * Offset/Address/Size: 0x13E4 | 0x80176A88 | size: 0x588
 */
eTeamID GameInfoManager::FindWinningTeam()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x12F8 | 0x8017699C | size: 0xEC
 */
bool GameInfoManager::IsKongaUnlocked() const
{
    return CheckUnlockStatus(isKongaUnlocked, mUserInfo.mTrophies[0], 0);
}

/**
 * Offset/Address/Size: 0x120C | 0x801768B0 | size: 0xEC
 */
bool GameInfoManager::IsYoshiUnlocked() const
{
    return CheckUnlockStatus(isYoshiUnlocked, mUserInfo.mTrophies[0], 1);
}

/**
 * Offset/Address/Size: 0x1120 | 0x801767C4 | size: 0xEC
 */
bool GameInfoManager::IsForbiddenUnlocked() const
{
    return CheckUnlockStatus(isForbiddenUnlocked, mUserInfo.mTrophies[0], 2);
}

/**
 * Offset/Address/Size: 0x1034 | 0x801766D8 | size: 0xEC
 */
bool GameInfoManager::IsSuperStadiumUnlocked() const
{
    return CheckUnlockStatus(isSuperStadUnlocked, mUserInfo.mTrophies[0], 3);
}

/**
 * Offset/Address/Size: 0xF48 | 0x801765EC | size: 0xEC
 */
bool GameInfoManager::IsSuperTeamUnlocked() const
{
    return CheckUnlockStatus(isSuperTeamUnlocked, mUserInfo.mTrophies[0], 3);
}

/**
 * Offset/Address/Size: 0xF24 | 0x801765C8 | size: 0x24
 */
bool GameInfoManager::IsSuperCupModeUnlocked() const
{
    return IsUserQualified(GM_NUM_MODES);
}

/**
 * Offset/Address/Size: 0xF1C | 0x801765C0 | size: 0x8
 */
bool GameInfoManager::IsLegendSkillUnlocked() const
{
    return true;
}

/**
 * Offset/Address/Size: 0xE30 | 0x801764D4 | size: 0xEC
 */
bool GameInfoManager::IsAllSTSCheatUnlocked() const
{
    return CheckUnlockStatus(isAllSTSUnlocked, mUserInfo.mTrophies[0], 4);
}

/**
 * Offset/Address/Size: 0xD44 | 0x801763E8 | size: 0xEC
 */
bool GameInfoManager::IsTiltCheatUnlocked() const
{
    return CheckUnlockStatus(isTiltUnlocked, mUserInfo.mTrophies[0], 5);
}

/**
 * Offset/Address/Size: 0xC58 | 0x801762FC | size: 0xEC
 */
bool GameInfoManager::IsGlassJawGoalieUnlocked() const
{
    return CheckUnlockStatus(isGoalieUnlocked, mUserInfo.mTrophies[0], 6);
}

/**
 * Offset/Address/Size: 0xB6C | 0x80176210 | size: 0xEC
 */
bool GameInfoManager::IsUnlimtedPowerupsUnlocked() const
{
    return CheckUnlockStatus(isUnlimitedUnlocked, mUserInfo.mTrophies[0], 7);
}

/**
 * Offset/Address/Size: 0xA80 | 0x80176124 | size: 0xEC
 */
bool GameInfoManager::IsCustomShellsUnlocked() const
{
    return CheckUnlockStatus(isShellsUnlocked, mUserInfo.mTrophies[1], 0);
}

/**
 * Offset/Address/Size: 0x994 | 0x80176038 | size: 0xEC
 */
bool GameInfoManager::IsCustomEnhanceUnlocked() const
{
    return CheckUnlockStatus(isEnhanceUnlocked, mUserInfo.mTrophies[1], 1);
}

/**
 * Offset/Address/Size: 0x8A8 | 0x80175F4C | size: 0xEC
 */
bool GameInfoManager::IsCustomGiantUnlocked() const
{
    return CheckUnlockStatus(isGiantUnlocked, mUserInfo.mTrophies[1], 4);
}

/**
 * Offset/Address/Size: 0x7BC | 0x80175E60 | size: 0xEC
 */
bool GameInfoManager::IsCustomExplosiveUnlocked() const
{
    return CheckUnlockStatus(isExplosiveUnlocked, mUserInfo.mTrophies[1], 2);
}

/**
 * Offset/Address/Size: 0x6D0 | 0x80175D74 | size: 0xEC
 */
bool GameInfoManager::IsCustomFreezingUnlocked() const
{
    return CheckUnlockStatus(isFreezingUnlocked, mUserInfo.mTrophies[1], 3);
}

/**
 * Offset/Address/Size: 0x41C | 0x80175AC0 | size: 0x2B4
 * TODO: 98.41% match - still missing the redundant second mode check/load in the
 * prologue, and one inlined GetNumGamesPerRound path still reloads mCurrentCup.
 */
bool GameInfoManager::HasHumanGameBeenPlayed() const
{
    if (mCurrentMode != GM_TOURNAMENT)
    {
        return true;
    }

    eGameModes mode;
    BaseCup* currentCup;
    int currentRound;
    int currentGame;
    int round;
    int game;

    currentRound = mCurrentCup->mRoundNumber;
    currentGame = mCurrentCup->mGameNumber;

    round = GetFirstRoundNumber();

    currentCup = mCurrentCup;
    mode = mCurrentMode;
    game = 0;

    while (round != -5)
    {
        if ((round == currentRound) && (game == currentGame))
        {
            return false;
        }

        BaseCup* cup = currentCup;
        s16 lookupRound = round;

        if ((mode == GM_BOWSER_CUP) || (mode == GM_SUPER_BOWSER_CUP))
        {
            if (lookupRound == -3)
            {
                lookupRound = 0;
            }
            else if ((lookupRound == -2) || (lookupRound == -5) || (lookupRound == -1))
            {
                lookupRound = 1;
            }
            else if (mDoingKnockout)
            {
                cup = mPreviousCup;
            }
        }
        else
        {
            if (lookupRound == -4)
            {
                lookupRound = currentCup->GetNumRounds() - 3;
            }
            else if (lookupRound == -3)
            {
                lookupRound = currentCup->GetNumRounds() - 2;
            }
            else if (lookupRound == -2)
            {
                lookupRound = currentCup->GetNumRounds() - 1;
            }
        }

        BasicGameInfo* gameInfo = cup->GetGameInfo(lookupRound, (u16)game);
        u16 humanTeams = currentCup->mHumanTeams;

        if ((humanTeams & (1 << gameInfo->mTeamIndex[0])) || (humanTeams & (1 << gameInfo->mTeamIndex[1])))
        {
            return true;
        }

        game++;

        if (game == GetNumGamesPerRound(round))
        {
            game = 0;
            round = GetNextRoundNumber(round);
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x2FC | 0x801759A0 | size: 0x120
 */
void GameInfoManager::SetRoundResult(bool inOvertime, int winningSide)
{
    BaseCup* cup = mCurrentCup;
    eGameModes mode = mCurrentMode;
    int roundNum = cup->mRoundNumber;
    BasicGameInfo* gameInfo = mGameInfo[mode];

    if (gameInfo == NULL)
    {
        winningSide = -1;
    }
    else
    {
        winningSide = (int)gameInfo->mTeamIndex[(s16)winningSide];
    }

    eTeamID userTeam = cup->mUserSelectedTeam;
    bool userWon = ((eTeamID)winningSide == userTeam);

    if (mode == GM_BOWSER_CUP || mode == GM_SUPER_BOWSER_CUP)
    {
        if (mDoingKnockout)
        {
            if (roundNum == -3)
            {
                roundNum = 0;
            }
            else if (roundNum == -2 || roundNum == -1)
            {
                roundNum = 1;
            }
        }
    }

    if (userWon)
    {
        *cup->GetRoundResults(roundNum) = 0;
    }
    else if (inOvertime)
    {
        *cup->GetRoundResults(roundNum) = 2;
    }
    else
    {
        *cup->GetRoundResults(roundNum) = 1;
    }
}

/**
 * Offset/Address/Size: 0x29C | 0x80175940 | size: 0x60
 */
bool GameInfoManager::IsStunnedGoaliesOn() const
{
    if (mIsInStrikers101Mode)
    {
        return false;
    }

    bool useCheatSettings;
    eGameModes currentMode = mCurrentMode;
    if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
    {
        useCheatSettings = false;
        if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
        {
            useCheatSettings = true;
        }
        if (useCheatSettings)
        {
            return mUserInfo.mCheatOptions.mStunnedGoalies;
        }
        return false;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x23C | 0x801758E0 | size: 0x60
 */
bool GameInfoManager::IsInfinitePowerupsOn() const
{
    if (mIsInStrikers101Mode)
    {
        return false;
    }

    bool useCheatSettings;
    eGameModes currentMode = mCurrentMode;
    if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
    {
        useCheatSettings = false;
        if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
        {
            useCheatSettings = true;
        }
        if (useCheatSettings)
        {
            return mUserInfo.mCheatOptions.mInfinitePowerups;
        }
        return false;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x1DC | 0x80175880 | size: 0x60
 */
bool GameInfoManager::IsTiltingFieldOn() const
{
    if (mIsInStrikers101Mode)
    {
        return false;
    }

    bool useCheatSettings;
    eGameModes currentMode = mCurrentMode;
    if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
    {
        useCheatSettings = false;
        if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
        {
            useCheatSettings = true;
        }
        if (useCheatSettings)
        {
            return mUserInfo.mCheatOptions.mCheatTBD1Enabled;
        }
        return false;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x17C | 0x80175820 | size: 0x60
 */
bool GameInfoManager::IsPerfectStrikesOn() const
{
    if (mIsInStrikers101Mode)
    {
        return false;
    }

    bool useCheatSettings;
    eGameModes currentMode = mCurrentMode;
    if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
    {
        useCheatSettings = false;
        if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
        {
            useCheatSettings = true;
        }
        if (useCheatSettings)
        {
            return mUserInfo.mCheatOptions.mCheatTBD2Enabled;
        }
        return false;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x14C | 0x801757F0 | size: 0x30
 */
bool GameInfoManager::IsBowserAttackEnabled() const
{
    if (g_e3_Build)
    {
        return false;
    }

    if (mIsInStrikers101Mode)
    {
        return false;
    }

    return mCurGameGameplayOptions.BowserAttackEnabled;
}

/**
 * Offset/Address/Size: 0xF8 | 0x8017579C | size: 0x54
 */
GameplaySettings::eSkillLevel GameInfoManager::GetSkillLevel()
{
    if (mIsInStrikers101Mode)
        return GameplaySettings::TRAINING;

    const GameplaySettings::eSkillLevel* p;

    if (mUseCurGameSettings)
    {
        p = &mCurGameGameplayOptions.SkillLevel;
    }
    else
    {
        if ((mCurrentMode == GM_FRIENDLY) || (mCurrentMode == GM_DEMO))
        {
            p = &mUserInfo.mGameplayOptions.SkillLevel;
        }
        else
        {
            p = &mCurrentCup->mCupSettings.SkillLevel;
        }
    }

    return *p;
}

/**
 * Offset/Address/Size: 0x60 | 0x80175704 | size: 0x98
 */
eDifficultyID GameInfoManager::GetSkillLevelAsDifficultyID()
{
    eDifficultyID skillToDifficulty[5] = {
        DIFF_BRAINDEAD,
        DIFF_EASY,
        DIFF_MEDIUM,
        DIFF_HARD,
        DIFF_VERYHARD
    };

    GameplaySettings::eSkillLevel level;
    if (mIsInStrikers101Mode)
    {
        level = GameplaySettings::TRAINING;
    }
    else
    {
        GameplaySettings* pSettings;
        if (mUseCurGameSettings)
        {
            pSettings = &mCurGameGameplayOptions;
        }
        else if (mCurrentMode == GM_FRIENDLY || mCurrentMode == GM_DEMO)
        {
            pSettings = &mUserInfo.mGameplayOptions;
        }
        else
        {
            pSettings = &mCurrentCup->mCupSettings;
        }
        level = pSettings->SkillLevel;
    }

    return skillToDifficulty[level];
}

/**
 * Offset/Address/Size: 0x0 | 0x801756A4 | size: 0x60
 */
CustomPowerups GameInfoManager::GetCustomPowerups() const
{
    if (mIsInStrikers101Mode)
    {
        return CP_OFF;
    }

    bool useCheatSettings;
    eGameModes currentMode = mCurrentMode;
    if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
    {
        useCheatSettings = false;
        if ((currentMode == GM_FRIENDLY) || (currentMode == GM_TOURNAMENT))
        {
            useCheatSettings = true;
        }
        if (useCheatSettings)
        {
            return mUserInfo.mCheatOptions.mCustomPowerups;
        }
        return CP_OFF;
    }
    return CP_OFF;
}
