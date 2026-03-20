#include "Game/DB/StatsTracker.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/GameInfo.h"

// /**
//  * Offset/Address/Size: 0x2C7C | 0x80189818 | size: 0x144
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, const char*, const char*>(const BasicString<char, Detail::TempStringAllocator>&, const char* const&, const char* const&, const char* const&, const char* const&, const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x2B20 | 0x801896BC | size: 0x15C
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int>(const BasicString<char, Detail::TempStringAllocator>&, const int&, const int&, const int&, const int&, const int&, const int&, const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1DAC | 0x80188948 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<const char*>(const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1038 | 0x80187BD4 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<float>(const float&)
// {
// }

// /**
//  * Offset/Address/Size: 0xEDC | 0x80187A78 | size: 0x15C
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, const char*, const char*, float, float>(const BasicString<char, Detail::TempStringAllocator>&, const char* const&, const char* const&, const char* const&, const char* const&, const char* const&, const float&, const float&)
// {
// }

// /**
//  * Offset/Address/Size: 0x168 | 0x80186D04 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<int>(const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80186B9C | size: 0x168
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int, int>(const BasicString<char, Detail::TempStringAllocator>&, const int&, const int&, const int&, const int&, const int&, const int&, const int&, const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x801869B8 | size: 0x1E4
//  */
// void BasicString<char, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801868FC | size: 0xBC
//  */
// void BasicString<char, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&) const
// {
// }

/**
 * Offset/Address/Size: 0x52FC | 0x8018685C | size: 0xA0
 */
StatsTracker::StatsTracker()
    : mBasicGameInfo(NULL)
    , mEventHandler(NULL)
{
    mIsUserCupWinner = false;
    mHasGameEnded = false;

    m_pSimulator = new (nlMalloc(0xB8, 8, false)) Simulator();
}

/**
 * Offset/Address/Size: 0x50C8 | 0x80186628 | size: 0x234
 */
void StatsTracker::SetBasicGameInfoPointer(BasicGameInfo* pGameInfo, bool initializeStats)
{
    eTeamID homeid;
    eTeamID awayid;
    eSidekickID homesk;
    eSidekickID awaysk;
    eCharacterClass characterClass;
    u32 i;
    int j;

    mBasicGameInfo = pGameInfo;
    homeid = mBasicGameInfo->mTeamIndex[0];
    awayid = mBasicGameInfo->mTeamIndex[1];
    homesk = mBasicGameInfo->mSidekickIndex[0];
    awaysk = mBasicGameInfo->mSidekickIndex[1];

    mIsUserCupWinner = false;
    mIsOvertime = false;
    mHasGameEnded = false;
    mNumConsecutiveGamesPlayed = 1;
    mNumGamesWon[0] = 0;
    mNumGamesWon[1] = 0;

    if (!initializeStats)
    {
        return;
    }

    memset(&mCurrentTeamStats[0].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCurrentTeamStats[0].mPlayerTotalStats.mRecordType.mTeamID = homeid;
    mCurrentTeamStats[0].mPlayerTotalStats.mType = TYPE_TEAM;
    mCurrentTeamStats[0].mTeamIndex = homeid;
    mCurrentTeamStats[0].mNumWins = 0;
    mCurrentTeamStats[0].mNumLosses = 0;
    mCurrentTeamStats[0].mNumOTLosses = 0;
    mCurrentTeamStats[0].mNumPoints = 0;

    memset(&mCurrentTeamStats[1].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCurrentTeamStats[1].mPlayerTotalStats.mRecordType.mTeamID = awayid;
    mCurrentTeamStats[1].mPlayerTotalStats.mType = TYPE_TEAM;
    mCurrentTeamStats[1].mTeamIndex = awayid;
    mCurrentTeamStats[1].mNumWins = 0;
    mCurrentTeamStats[1].mNumLosses = 0;
    mCurrentTeamStats[1].mNumOTLosses = 0;
    mCurrentTeamStats[1].mNumPoints = 0;

    memset(&mCumulativeTeamStats[0].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCumulativeTeamStats[0].mPlayerTotalStats.mRecordType.mTeamID = homeid;
    mCumulativeTeamStats[0].mPlayerTotalStats.mType = TYPE_TEAM;
    mCumulativeTeamStats[0].mTeamIndex = homeid;
    mCumulativeTeamStats[0].mNumWins = 0;
    mCumulativeTeamStats[0].mNumLosses = 0;
    mCumulativeTeamStats[0].mNumOTLosses = 0;
    mCumulativeTeamStats[0].mNumPoints = 0;

    memset(&mCumulativeTeamStats[1].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCumulativeTeamStats[1].mPlayerTotalStats.mRecordType.mTeamID = awayid;
    mCumulativeTeamStats[1].mPlayerTotalStats.mType = TYPE_TEAM;
    mCumulativeTeamStats[1].mTeamIndex = awayid;
    mCumulativeTeamStats[1].mNumWins = 0;
    mCumulativeTeamStats[1].mNumLosses = 0;
    mCumulativeTeamStats[1].mNumOTLosses = 0;
    mCumulativeTeamStats[1].mNumPoints = 0;

    characterClass = (eCharacterClass)ConvertToCharacterClass(homeid);
    memset(&mCurrentPlayerStats[0][0], 0, sizeof(PlayerStats));
    mCurrentPlayerStats[0][0].mRecordType.mCharacterClass = characterClass;
    mCurrentPlayerStats[0][0].mType = TYPE_CHARACTER;

    characterClass = (eCharacterClass)ConvertToCharacterClass(awayid);
    memset(&mCurrentPlayerStats[1][0], 0, sizeof(PlayerStats));
    mCurrentPlayerStats[1][0].mRecordType.mCharacterClass = characterClass;
    mCurrentPlayerStats[1][0].mType = TYPE_CHARACTER;

    i = 1;
    do
    {
        characterClass = (eCharacterClass)ConvertToCharacterClass(homesk);
        memset(&mCurrentPlayerStats[0][i], 0, sizeof(PlayerStats));
        mCurrentPlayerStats[0][i].mRecordType.mCharacterClass = characterClass;
        mCurrentPlayerStats[0][i].mType = TYPE_CHARACTER;

        characterClass = (eCharacterClass)ConvertToCharacterClass(awaysk);
        memset(&mCurrentPlayerStats[1][i], 0, sizeof(PlayerStats));
        mCurrentPlayerStats[1][i].mRecordType.mCharacterClass = characterClass;
        mCurrentPlayerStats[1][i].mType = TYPE_CHARACTER;

        i++;
    } while (i < 5);

    j = 0;
    do
    {
        memset(&mCurrentUserStats[j], 0, sizeof(PlayerStats));
        mCurrentUserStats[j].mRecordType.mControllerID = j;
        mCurrentUserStats[j].mType = TYPE_USER;

        memset(&mCumulativeUserStats[j], 0, sizeof(PlayerStats));
        mCumulativeUserStats[j].mRecordType.mControllerID = j;
        mCumulativeUserStats[j].mType = TYPE_USER;

        j++;
    } while (j < 4);
}

/**
 * Offset/Address/Size: 0x4FD0 | 0x80186530 | size: 0xF8
 * TODO: 94.2% match - r29/r30 register swap for this pointer vs teamIdx (MWCC register allocation quirk)
 */
void StatsTracker::ResetCurrentStats()
{
    eTeamID teamIdx;
    int i;

    mIsOvertime = false;
    mHasGameEnded = false;

    teamIdx = mCumulativeTeamStats[0].mTeamIndex;
    memset(&mCurrentTeamStats[0].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCurrentTeamStats[0].mPlayerTotalStats.mRecordType.mTeamID = teamIdx;
    mCurrentTeamStats[0].mPlayerTotalStats.mType = TYPE_TEAM;
    mCurrentTeamStats[0].mTeamIndex = teamIdx;
    mCurrentTeamStats[0].mNumWins = 0;
    mCurrentTeamStats[0].mNumLosses = 0;
    mCurrentTeamStats[0].mNumOTLosses = 0;
    mCurrentTeamStats[0].mNumPoints = 0;

    teamIdx = mCumulativeTeamStats[1].mTeamIndex;
    memset(&mCurrentTeamStats[1].mPlayerTotalStats, 0, sizeof(PlayerStats));
    mCurrentTeamStats[1].mPlayerTotalStats.mRecordType.mTeamID = teamIdx;
    mCurrentTeamStats[1].mPlayerTotalStats.mType = TYPE_TEAM;
    mCurrentTeamStats[1].mTeamIndex = teamIdx;
    mCurrentTeamStats[1].mNumWins = 0;
    mCurrentTeamStats[1].mNumLosses = 0;
    mCurrentTeamStats[1].mNumOTLosses = 0;
    mCurrentTeamStats[1].mNumPoints = 0;

    mNumConsecutiveGamesPlayed = mNumConsecutiveGamesPlayed + 1;

    i = 0;
    do
    {
        memset(&mCurrentUserStats[i], 0, sizeof(PlayerStats));
        mCurrentUserStats[i].mRecordType.mControllerID = i;
        mCurrentUserStats[i].mType = TYPE_USER;
        i++;
    } while (i < 4);
}

/**
 * Offset/Address/Size: 0x4F8C | 0x801864EC | size: 0x44
 */
void StatsTracker::CreateEventHandler()
{
    mEventHandler = g_pEventManager->AddEventHandler(eventHandler, NULL, 1);
}

/**
 * Offset/Address/Size: 0x4F48 | 0x801864A8 | size: 0x44
 */
void StatsTracker::DestroyEventHandler()
{
    if (mEventHandler)
    {
        g_pEventManager->RemoveEventHandler(mEventHandler);
        mEventHandler = NULL;
    }
}

/**
 * Offset/Address/Size: 0x4730 | 0x80185C90 | size: 0x818
 */
void StatsTracker::eventHandler(Event*, void*)
{
}

/**
 * Offset/Address/Size: 0x3EF0 | 0x80185450 | size: 0x840
 */
void StatsTracker::TrackStat(ePlayerStats, int, int, int, int, int, int)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x3708 | 0x80184C68 | size: 0x7E8
 */
void StatsTracker::GetSortedStats(PlayerStats*, int, int*, int, ePlayerStats, eSortOrder)
{
}

/**
 * Offset/Address/Size: 0x3340 | 0x801848A0 | size: 0x3C8
 * TODO: 99.73% match - 11 register-only diffs in e8-118 (r4/r6 swap for
 *       constant 1 vs slwi temp, MWCC allocator coloring choice).
 */
void StatsTracker::GetSortedTeamStats(TeamStats* source, int numsource, int* dest, int numelements)
{
    int tempsorted[8];
    for (int i = 0; i < numsource; i++)
    {
        tempsorted[i] = i;
    }
    unsigned char swapped;
    do
    {
        swapped = 0;
        for (int i = 0; i < numsource; i++)
        {
            if (i + 1 >= numsource)
                break;
            u16 humanTeams = nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup->mHumanTeams;
            int a = source[tempsorted[i]].mNumPoints;
            unsigned int humanA = (humanTeams & (1 << source[tempsorted[i]].mTeamIndex)) != 0;
            int b = source[tempsorted[i + 1]].mNumPoints;
            unsigned int humanB = (humanTeams & (1 << source[tempsorted[i + 1]].mTeamIndex)) != 0;
            if (a < b
                || (a == b && a != 0 && !humanA && humanB)
                || (a == b && a == 0 && humanA && !humanB))
            {
                int temp = tempsorted[i + 1];
                tempsorted[i + 1] = tempsorted[i];
                tempsorted[i] = temp;
                swapped = 1;
            }
            else if (a == b && humanA && humanB)
            {
                if (MoveTeamBUp(source[tempsorted[i]], source[tempsorted[i + 1]]))
                {
                    int temp = tempsorted[i + 1];
                    tempsorted[i + 1] = tempsorted[i];
                    tempsorted[i] = temp;
                    swapped = 1;
                }
            }
        }
    } while (swapped);
    for (int i = 0; i < numelements; i++)
    {
        dest[i] = tempsorted[i];
    }
}

/**
 * Offset/Address/Size: 0x2E00 | 0x80184360 | size: 0x540
 */
void StatsTracker::CompileEndOfGameStats()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x2B34 | 0x80184094 | size: 0x2CC
 */
void StatsTracker::SimulateRemainingGames()
{
    BaseCup* cup;
    int round;
    int numGames;
    int numRounds;
    eTeamID userTeam;
    int game;
    u16 gamesPerRound;
    u16 roundsTotal;

    round = nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup->mRoundNumber;
    gamesPerRound = nlSingleton<GameInfoManager>::s_pInstance->GetNumGamesPerRound(round);
    numGames = gamesPerRound;
    roundsTotal = nlSingleton<GameInfoManager>::s_pInstance->GetNumRounds();
    numRounds = roundsTotal;
    userTeam = nlSingleton<GameInfoManager>::s_pInstance->GetUserSelectedCupTeam();
    cup = nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup;

    m_pSimulator->InitializeStats();

    if (round == -4)
    {
        round = numRounds - 3;
    }

    if (round == -3)
    {
        round = numRounds - 2;
    }

    if (round == -2)
    {
        round = numRounds - 1;
    }

    if (round == -1)
    {
        return;
    }

    for (game = 0; game < numGames; game++)
    {
        if (userTeam == cup->GetGameInfo(round, game)->mTeamIndex[0])
        {
            continue;
        }

        if (userTeam == cup->GetGameInfo(round, game)->mTeamIndex[1])
        {
            continue;
        }

        SetBasicGameInfoPointer(cup->GetGameInfo(round, game), true);
        SimulateGame();

        if (nlSingleton<GameInfoManager>::s_pInstance->GetResultsOfLastUserGame() != RESULT_USER_DOES_NOT_PLAYOFF_QUALIFY)
        {
            CompileEndOfGameStats();
        }
    }
}

/**
 * Offset/Address/Size: 0x2974 | 0x80183ED4 | size: 0x1C0
 */
void StatsTracker::SimulateGame()
{
    int goalsFor = 0;
    int goalsAgainst = 0;
    int i;

    for (i = 0; i < 15; i++)
    {
        int rand = (int)nlRandom(100, &nlDefaultSeed);
        if (rand < 20)
        {
            goalsFor++;
            s_pInstance->TrackStat(STATS_GOALS_FOR, 0, 0, -1, 0, 1, 0);
        }
        else if (rand < 40)
        {
            goalsAgainst++;
            s_pInstance->TrackStat(STATS_GOALS_FOR, 1, 0, -1, 0, 1, 0);
        }
    }

    if (goalsFor == goalsAgainst)
    {
        if ((int)nlRandom(100, &nlDefaultSeed) < 50)
        {
            s_pInstance->TrackStat(STATS_GOALS_FOR, 0, 0, -1, 0, 1, 0);
            s_pInstance->TrackStat(STATS_OT_WIN, 0, 0, goalsFor + 1, goalsAgainst, 0, 0);
        }
        else
        {
            s_pInstance->TrackStat(STATS_GOALS_FOR, 1, 0, -1, 0, 1, 0);
            s_pInstance->TrackStat(STATS_OT_WIN, 1, 0, goalsFor, goalsAgainst + 1, 0, 0);
        }
    }
    else if (goalsFor > goalsAgainst)
    {
        s_pInstance->TrackStat(STATS_WIN, 0, 0, goalsFor, goalsAgainst, 0, 0);
    }
    else
    {
        s_pInstance->TrackStat(STATS_WIN, 1, 0, goalsFor, goalsAgainst, 0, 0);
    }
}

/**
 * Offset/Address/Size: 0x2200 | 0x80183760 | size: 0x774
 */
void StatsTracker::AddStat(ePlayerStats, int, int, int)
{
}

/**
 * Offset/Address/Size: 0x1E5C | 0x801833BC | size: 0x3A4
 */
void StatsTracker::AddUserStatByPad(ePlayerStats stat, int pad, int amount)
{
    if (pad < 0)
    {
        return;
    }

    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        mCurrentUserStats[pad].mNumShotsOnGoal += amount;
        break;
    case STATS_GOALS_FOR:
        mCurrentUserStats[pad].mNumGoalsFor += amount;
        break;
    case STATS_GOALS_FOR_STS:
        mCurrentUserStats[pad].mNumShootToScoreGoals += amount;
        break;
    case STATS_GOALS_AGAINST:
        mCurrentUserStats[pad].mNumGoalsAgainst += amount;
        break;
    case STATS_ASSISTS:
        mCurrentUserStats[pad].mNumAssists += amount;
        break;
    case STATS_FOULS:
        mCurrentUserStats[pad].mNumFouls += amount;
        break;
    case STATS_POWERUPS_USED:
        mCurrentUserStats[pad].mNumPowerupsUsed += amount;
        break;
    case STATS_POWERUPS_HIT:
        mCurrentUserStats[pad].mNumPowerupsHit += amount;
        break;
    case STATS_PASSES_MADE:
        mCurrentUserStats[pad].mNumPassesMade += amount;
        break;
    case STATS_PASSES_RECEIVED:
        mCurrentUserStats[pad].mNumPassesReceived += amount;
        break;
    case STATS_PASSES_INTERCEPTED:
        mCurrentUserStats[pad].mNumPassesIntercepted += amount;
        break;
    case STATS_POSSESION_TIME:
        mCurrentUserStats[pad].mBallPossessionTime += amount;
        break;
    case STATS_STEALS:
        mCurrentUserStats[pad].mNumSteals += amount;
        break;
    case STATS_BUTTON_PRESSES:
        mCurrentUserStats[pad].mNumButtonPresses += amount;
        break;
    case STATS_HITS_MADE:
        mCurrentUserStats[pad].mNumHitsMade += amount;
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        mCurrentUserStats[pad].mNumGoalsOneTimers += amount;
        break;
    case STATS_STS_ATTEMPTS:
        mCurrentUserStats[pad].mNumSTSAttempts += amount;
        break;
    case STATS_PERFECT_PASSES:
        mCurrentUserStats[pad].mNumPerfectPasses += amount;
        break;
    default:
        break;
    }

    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        mCumulativeUserStats[pad].mNumShotsOnGoal += amount;
        break;
    case STATS_GOALS_FOR:
        mCumulativeUserStats[pad].mNumGoalsFor += amount;
        break;
    case STATS_GOALS_FOR_STS:
        mCumulativeUserStats[pad].mNumShootToScoreGoals += amount;
        break;
    case STATS_GOALS_AGAINST:
        mCumulativeUserStats[pad].mNumGoalsAgainst += amount;
        break;
    case STATS_ASSISTS:
        mCumulativeUserStats[pad].mNumAssists += amount;
        break;
    case STATS_FOULS:
        mCumulativeUserStats[pad].mNumFouls += amount;
        break;
    case STATS_POWERUPS_USED:
        mCumulativeUserStats[pad].mNumPowerupsUsed += amount;
        break;
    case STATS_POWERUPS_HIT:
        mCumulativeUserStats[pad].mNumPowerupsHit += amount;
        break;
    case STATS_PASSES_MADE:
        mCumulativeUserStats[pad].mNumPassesMade += amount;
        break;
    case STATS_PASSES_RECEIVED:
        mCumulativeUserStats[pad].mNumPassesReceived += amount;
        break;
    case STATS_PASSES_INTERCEPTED:
        mCumulativeUserStats[pad].mNumPassesIntercepted += amount;
        break;
    case STATS_POSSESION_TIME:
        mCumulativeUserStats[pad].mBallPossessionTime += amount;
        break;
    case STATS_STEALS:
        mCumulativeUserStats[pad].mNumSteals += amount;
        break;
    case STATS_BUTTON_PRESSES:
        mCumulativeUserStats[pad].mNumButtonPresses += amount;
        break;
    case STATS_HITS_MADE:
        mCumulativeUserStats[pad].mNumHitsMade += amount;
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        mCumulativeUserStats[pad].mNumGoalsOneTimers += amount;
        break;
    case STATS_STS_ATTEMPTS:
        mCumulativeUserStats[pad].mNumSTSAttempts += amount;
        break;
    case STATS_PERFECT_PASSES:
        mCumulativeUserStats[pad].mNumPerfectPasses += amount;
        break;
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x1C88 | 0x801831E8 | size: 0x1D4
 */
void StatsTracker::AddMilestoneUserStat(ePlayerStats stat, int amount)
{
    GameInfoManager* pGIM = GameInfoManager::Instance();

    switch (stat)
    {
    case STATS_GOALS_FOR:
    {
        int val = pGIM->mUserInfo.mNumGoalsScored + amount;
        if (val >= 9999)
        {
            pGIM->mUserInfo.mNumGoalsScored = 9999;
        }
        else
        {
            pGIM->mUserInfo.mNumGoalsScored = val;
        }
        if (!pGIM->HasTrophy(TROPHY_SNIPER_CUP) && pGIM->mUserInfo.mNumGoalsScored >= 300)
        {
            pGIM->mUserInfo.mTrophies[1] |= 0x02;
        }
        break;
    }
    case STATS_HITS_MADE:
    {
        int val = pGIM->mUserInfo.mNumHits + amount;
        if (val >= 9999)
        {
            pGIM->mUserInfo.mNumHits = 9999;
        }
        else
        {
            pGIM->mUserInfo.mNumHits = val;
        }
        if (!pGIM->HasTrophy(TROPHY_PARAMEDIC_CUP) && pGIM->mUserInfo.mNumHits >= 1000)
        {
            pGIM->mUserInfo.mTrophies[1] |= 0x10;
        }
        break;
    }
    case STATS_PERFECT_PASSES:
    {
        int val = pGIM->mUserInfo.mNumPerfectPasses + amount;
        if (val >= 9999)
        {
            pGIM->mUserInfo.mNumPerfectPasses = 9999;
        }
        else
        {
            pGIM->mUserInfo.mNumPerfectPasses = val;
        }
        if (!pGIM->HasTrophy(TROPHY_TACTICIAN_CUP) && pGIM->mUserInfo.mNumPerfectPasses >= 300)
        {
            pGIM->mUserInfo.mTrophies[1] |= 0x08;
        }
        break;
    }
    case STATS_GAMES_PLAYED:
    {
        int val = pGIM->mUserInfo.mNumGamesPlayed + amount;
        if (val >= 9999)
        {
            pGIM->mUserInfo.mNumGamesPlayed = 9999;
        }
        else
        {
            pGIM->mUserInfo.mNumGamesPlayed = val;
        }
        if (!pGIM->HasTrophy(TROPHY_VETERAN_CUP) && pGIM->mUserInfo.mNumGamesPlayed >= 100)
        {
            pGIM->mUserInfo.mTrophies[1] |= 0x01;
        }
        break;
    }
    case STATS_STS_ATTEMPTS:
    {
        int val = pGIM->mUserInfo.mNumSTSAttempts + amount;
        if (val >= 9999)
        {
            pGIM->mUserInfo.mNumSTSAttempts = 9999;
        }
        else
        {
            pGIM->mUserInfo.mNumSTSAttempts = val;
        }
        if (!pGIM->HasTrophy(TROPHY_STRIKER_CUP) && pGIM->mUserInfo.mNumSTSAttempts >= 100)
        {
            pGIM->mUserInfo.mTrophies[1] |= 0x04;
        }
        break;
    }
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x1AE0 | 0x80183040 | size: 0x1A8
 */
#pragma push
#pragma bool off
void StatsTracker::TrackWinner(int forfeitSide)
{
    int scoreLeft;
    int scoreRight;
    int startScoreLeft;
    int startScoreRight;
    unsigned char wasForfeit;
    long winningSide;

    wasForfeit = 0;
    startScoreLeft = g_pTeams[0]->m_nScore;
    startScoreRight = g_pTeams[1]->m_nScore;
    scoreLeft = startScoreLeft;
    scoreRight = startScoreRight;

    if (forfeitSide == 0)
    {
        scoreLeft = -5;
        if (startScoreRight < 3)
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 1, 0, 0, 0, 3 - startScoreRight, 0);
            scoreRight = 3;
        }
        wasForfeit = 1;
    }
    else if (forfeitSide == 1)
    {
        scoreRight = -5;
        if (startScoreLeft < 3)
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR, 0, 0, 0, 0, 3 - startScoreLeft, 0);
            scoreLeft = 3;
        }
        wasForfeit = 1;
    }

    winningSide = scoreRight >= scoreLeft;

    if (!mHasGameEnded)
    {
        if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupOrTournamentMode())
        {
            if (mIsOvertime && !wasForfeit)
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_OT_WIN, winningSide, 0, scoreLeft, scoreRight, 0, 0);
                nlSingleton<GameInfoManager>::s_pInstance->SetRoundResult(1, winningSide);
            }
            else
            {
                nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_WIN, winningSide, 0, scoreLeft, scoreRight, 0, 0);
                nlSingleton<GameInfoManager>::s_pInstance->SetRoundResult(0, winningSide);
            }
            nlSingleton<StatsTracker>::s_pInstance->CompileEndOfGameStats();
        }
        else
        {
            nlSingleton<StatsTracker>::s_pInstance->mNumGamesWon[winningSide]++;
        }
        mHasGameEnded = 1;
    }
}
#pragma pop

/**
 * Offset/Address/Size: 0x540 | 0x80181AA0 | size: 0x15A0
 */
void StatsTracker::WriteStats(float, float, const char*)
{
}

/**
 * Offset/Address/Size: 0x418 | 0x80181978 | size: 0x128
 */
void StatsTracker::AwardCup(eUserGameResult gameResult)
{
    eUserGameResult neededResult = nlSingleton<GameInfoManager>::s_pInstance->mCupMatchRequirement;
    mIsUserCupWinner = false;

    if (neededResult == RESULT_USER_LOSES)
    {
        mIsUserCupWinner = false;
    }
    else if (neededResult == RESULT_CUP_WIN)
    {
        mIsUserCupWinner = true;
    }
    else if (neededResult == RESULT_USER_OT_LOSES)
    {
        if (gameResult == RESULT_USER_WINS || gameResult == RESULT_USER_OT_WINS || gameResult == RESULT_USER_OT_LOSES)
        {
            mIsUserCupWinner = true;
        }
        else
        {
            mIsUserCupWinner = false;
        }
    }
    else if (neededResult == RESULT_USER_OT_WINS)
    {
        if (gameResult == RESULT_USER_WINS || gameResult == RESULT_USER_OT_WINS)
        {
            mIsUserCupWinner = true;
        }
        else
        {
            mIsUserCupWinner = false;
        }
    }
    else if (neededResult == RESULT_USER_WINS)
    {
        if (gameResult == RESULT_USER_WINS)
        {
            mIsUserCupWinner = true;
        }
        else
        {
            mIsUserCupWinner = false;
        }
    }

    if (mIsUserCupWinner == true)
    {
        eTrophyType tourneyCup = nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();
        nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mTrophies[tourneyCup / 8] |= (1 << (tourneyCup % 8));
        nlSingleton<GameInfoManager>::s_pInstance->SetResultsOfLastUserGame(RESULT_CUP_WIN);
    }
}

/**
 * Offset/Address/Size: 0x148 | 0x801816A8 | size: 0x2D0
 */
template <typename StringType, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
void Format(StringType& result,
    const StringType& format,
    const Arg0& arg0,
    const Arg1& arg1,
    const Arg2& arg2,
    const Arg3& arg3,
    const Arg4& arg4);

extern "C" char* GetStadiumFilename__11WorldLoaderCF10eStadiumID(void*, eStadiumID);
extern "C" char TheWorldLoader;
extern "C" unsigned long fwrite(const void*, unsigned long, unsigned long, void*);

/**
 * Offset/Address/Size: 0x3DC | 0x801816A8 | size: 0x2D0
 * TODO: 95.61% match - remaining r29/r31 allocation in fopen/file and format-template paths
 */
void StatsTracker::WriteCurrentlyPlaying() const
{
    const char* formatTemplate;
    FILE* file = fopen("currently_playing.txt", "wt");
    if (file == 0)
    {
        return;
    }

    BasicStringInternal* formatData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (formatData != 0)
    {
        formatTemplate = "Home: {0} with {1}\nAway: {2} with {3}\nStadium: {4}\n";
        formatData->mData = 0;
        formatData->mSize = 0;
        formatData->mCapacity = 0;

        const char* p = formatTemplate;
        while ((s8)*p++ != 0)
        {
            formatData->mSize++;
        }

        formatData->mSize++;
        formatData->mData = (char*)nlMalloc(formatData->mSize + 1, 8, true);
        formatData->mCapacity = formatData->mSize;

        int i = 0;
        while (i < formatData->mSize)
        {
            formatData->mData[i] = *formatTemplate++;
            i++;
        }

        formatData->mRefCount = 1;
    }

    BasicStringInternal* retainedData;
    BasicStringInternal* formatDataTmp = formatData;
    const char* homeTeamName;
    const char* homeSidekickName;
    const char* awayTeamName;
    const char* awaySidekickName;
    const char* stadiumName;
    BasicStringInternal* outputData;

    stadiumName = GetStadiumFilename__11WorldLoaderCF10eStadiumID(
        &TheWorldLoader,
        nlSingleton<GameInfoManager>::s_pInstance->GetStadium());
    awaySidekickName = GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1));
    awayTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
    homeSidekickName = GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0));
    homeTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));

    Format(*(BasicString<char, Detail::TempStringAllocator>*)&outputData,
        *(BasicString<char, Detail::TempStringAllocator>*)&formatDataTmp,
        homeTeamName,
        homeSidekickName,
        awayTeamName,
        awaySidekickName,
        stadiumName);

    BasicStringInternal* toFree = outputData;
    if (outputData != 0)
    {
        outputData->mRefCount++;
        retainedData = outputData;
    }
    else
    {
        retainedData = 0;
    }

    if (toFree != 0)
    {
        if (--toFree->mRefCount == 0)
        {
            if (toFree != 0)
            {
                if (toFree != 0)
                {
                    delete[] toFree->mData;
                }
                if (toFree != 0)
                {
                    nlFree(toFree);
                }
            }
        }
    }

    toFree = formatData;
    if (toFree != 0)
    {
        if (--toFree->mRefCount == 0)
        {
            if (toFree != 0)
            {
                if (toFree != 0)
                {
                    delete[] toFree->mData;
                }
                if (toFree != 0)
                {
                    nlFree(toFree);
                }
            }
        }
    }

    s32 len;
    if (retainedData != 0)
    {
        len = retainedData->mSize - 1;
    }
    else
    {
        len = 0;
    }

    const char* str = ((BasicString<char, Detail::TempStringAllocator>*)&retainedData)->c_str();
    fwrite(str, 1, len, file);
    fclose(file);

    toFree = retainedData;
    if (toFree != 0)
    {
        if (--toFree->mRefCount == 0)
        {
            if (toFree != 0)
            {
                if (toFree != 0)
                {
                    delete[] toFree->mData;
                }
                if (toFree != 0)
                {
                    nlFree(toFree);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80181560 | size: 0x148
 */
bool StatsTracker::MoveTeamBUp(TeamStats b, TeamStats a)
{
    s32 bGoals = b.mPlayerTotalStats.mNumGoalsFor;
    s32 aGoals = a.mPlayerTotalStats.mNumGoalsFor;
    if (aGoals > bGoals)
        return true;
    if (bGoals > aGoals)
        return false;

    s32 bShots = b.mPlayerTotalStats.mNumShotsOnGoal;
    s32 aShots = a.mPlayerTotalStats.mNumShotsOnGoal;
    if (aShots > bShots)
        return true;
    if (bShots > aShots)
        return false;

    s32 bHits = b.mPlayerTotalStats.mNumHitsMade;
    s32 aHits = a.mPlayerTotalStats.mNumHitsMade;
    if (aHits > bHits)
        return true;
    if (bHits > aHits)
        return false;

    s32 bInterSteals = b.mPlayerTotalStats.mNumPassesIntercepted + b.mPlayerTotalStats.mNumSteals;
    s32 aInterSteals = a.mPlayerTotalStats.mNumPassesIntercepted + a.mPlayerTotalStats.mNumSteals;
    if (aInterSteals > bInterSteals)
        return true;
    if (bInterSteals > aInterSteals)
        return false;

    s32 bPU = b.mPlayerTotalStats.mNumPowerupsUsed;
    s32 aPU = a.mPlayerTotalStats.mNumPowerupsUsed;
    if (aPU > bPU)
        return true;
    if (bPU > aPU)
        return false;

    s32 bPP = b.mPlayerTotalStats.mNumPerfectPasses;
    s32 aPP = a.mPlayerTotalStats.mNumPerfectPasses;
    if (aPP > bPP)
        return true;
    if (bPP > aPP)
        return false;

    s32 bBP = b.mPlayerTotalStats.mNumButtonPresses;
    s32 aBP = a.mPlayerTotalStats.mNumButtonPresses;
    if (aBP > bBP)
        return true;
    if (bBP > aBP)
        return false;

    return (s32)a.mTeamIndex < (s32)b.mTeamIndex;
}
