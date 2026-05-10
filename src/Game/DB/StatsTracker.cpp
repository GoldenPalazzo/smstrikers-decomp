#include "Game/DB/StatsTracker.h"
#include "Game/AI/FielderActions.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/World/WorldLoader.h"

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
void StatsTracker::eventHandler(Event* event, void*)
{
    struct GoalScoredDataExt
    {
        GoalScoredData data;
        int sideOfInterest;
    };

    switch (event->m_uEventID)
    {
    case 5:
    {
        GoalScoredData* data;
        cPlayer* assistplayer;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x18A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (GoalScoredData*)&event->m_data;
            }
        }

        assistplayer = data->pAssister;

        s_pInstance->TrackStat(
            STATS_GOALS_FOR,
            data->uTeamIndex,
            data->pScorer->m_ID,
            (assistplayer != 0) ? assistplayer->m_ID : -1,
            data->uGoalType,
            data->uNumGoalsScored,
            ((GoalScoredDataExt*)data)->sideOfInterest);

        if (g_pGame != 0)
        {
            float gameDuration = g_pGame->m_fGameDuration;
            if (g_pGame->GetGameTime() >= gameDuration)
            {
                TeamStats awayStats = s_pInstance->mCurrentTeamStats[0];
                TeamStats homeStats = s_pInstance->mCurrentTeamStats[1];
                if ((int)awayStats.mPlayerTotalStats.mNumGoalsFor != (int)homeStats.mPlayerTotalStats.mNumGoalsFor)
                {
                    s_pInstance->TrackWinner(-1);
                }
            }
        }
        break;
    }

    case 0x3D:
    {
        PenaltyData* data;
        cPlayer* fouler;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x152)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (PenaltyData*)&event->m_data;
            }
        }

        fouler = (cPlayer*)data->pFouler;
        s_pInstance->TrackStat(STATS_FOULS,
            fouler->m_pTeam->m_nSide,
            fouler->m_ID,
            0,
            0,
            0,
            0);
        break;
    }

    case 0x14:
    {
        ShotAtGoalData* data;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x14A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (ShotAtGoalData*)&event->m_data;
            }
        }

        s_pInstance->TrackStat(STATS_SHOTS_ON_GOAL,
            data->pShooter->m_pTeam->m_nSide,
            data->pShooter->m_ID,
            0,
            0,
            0,
            0);
        break;
    }

    case 0xE:
    {
        PassBallData* data;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x131)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (PassBallData*)&event->m_data;
            }
        }

        s_pInstance->TrackStat(STATS_PASSES_MADE,
            data->pPasser->m_pTeam->m_nSide,
            data->pPasser->m_ID,
            data->mPasserControllerID,
            0,
            0,
            0);
        break;
    }

    case 0xD:
    {
        ReceiveBallData* data;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x121)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (ReceiveBallData*)&event->m_data;
            }
        }

        if (data->eResult == RECEIVEBALL_PASS_COMPLETE)
        {
            s_pInstance->TrackStat(STATS_PASSES_RECEIVED,
                data->pReceiver->m_pTeam->m_nSide,
                data->pReceiver->m_ID,
                0,
                0,
                0,
                0);
        }
        else if (data->eResult == RECEIVEBALL_PASS_INTERCEPT)
        {
            s_pInstance->TrackStat(STATS_PASSES_INTERCEPTED,
                data->pReceiver->m_pTeam->m_nSide,
                data->pReceiver->m_ID,
                0,
                0,
                0,
                0);
        }
        break;
    }

    case 0x55:
    {
        CollisionPowerupStatsData* data;
        cPlayer* thrower;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x104)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (CollisionPowerupStatsData*)&event->m_data;
            }
        }

        thrower = (cPlayer*)data->pThrower;
        s_pInstance->TrackStat(STATS_POWERUPS_HIT,
            thrower->m_pTeam->m_nSide,
            thrower->m_ID,
            data->nThrowerPadID,
            0,
            0,
            0);
        break;
    }

    case 0x19:
    {
        PlayerAttackData* data;
        cPlayer* attacker;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x19A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (PlayerAttackData*)&event->m_data;
            }
        }

        attacker = (cPlayer*)data->pAttacker;
        s_pInstance->TrackStat(STATS_STEALS,
            attacker->m_pTeam->m_nSide,
            attacker->m_ID,
            data->nAttackerPadID,
            0,
            0,
            0);
        break;
    }

    case 0x45:
    {
        PassBallData* data;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x131)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (PassBallData*)&event->m_data;
            }
        }

        s_pInstance->TrackStat(STATS_PERFECT_PASSES,
            data->pPasser->m_pTeam->m_nSide,
            data->pPasser->m_ID,
            data->mPasserControllerID,
            0,
            0,
            0);
        break;
    }

    case 0x40:
    {
        ShotAtGoalData* data;
        s32 id = event->m_data.GetID();

        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            data = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x14A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                data = 0;
            }
            else
            {
                data = (ShotAtGoalData*)&event->m_data;
            }
        }

        s_pInstance->TrackStat(STATS_STS_ATTEMPTS,
            data->pShooter->m_pTeam->m_nSide,
            data->pShooter->m_ID,
            0,
            0,
            0,
            0);
        break;
    }

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x3EF0 | 0x80185450 | size: 0x840
 */
void StatsTracker::TrackStat(ePlayerStats stat, int homeaway, int playerindex, int param0, int param1, int param2, int param3)
{
    cTeam* team;
    cGlobalPad* pad;
    GameInfoManager* gameInfoManager;
    eTeamID teamID;
    int padIndex;
    u8 hasPad;
    int i;

    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        AddStat(STATS_SHOTS_ON_GOAL, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_SHOTS_ON_GOAL;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_SHOTS_ON_GOAL:
        if (hasPad)
        {
            AddUserStatByPad(STATS_SHOTS_ON_GOAL, padIndex, 1);
        }
        break;
    case STATS_GOALS_FOR:
        AddStat(STATS_GOALS_FOR, homeaway, playerindex, param2);
        AddUserStatByPad(STATS_GOALS_FOR, param3, param2);
        if (param1 == 6 || param1 == 2)
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR_STS, homeaway, playerindex, param2, param3, 0, 0);
        }
        else if (param1 == 1)
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_FOR_ONE_TIMERS, homeaway, playerindex, param2, param3, 0, 0);
        }
        if (param0 >= 0)
        {
            nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_ASSISTS, homeaway, param0, 0, 0, 0, 0);
        }
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_AGAINST, homeaway == 0, playerindex, param2, 0, 0, 0);
        break;
    case STATS_GOALS_AGAINST:
        AddStat(STATS_GOALS_AGAINST, homeaway, -1, param0);
        for (i = 0; i < 5; i++)
        {
            team = g_pTeams[homeaway];
            if (team != NULL)
            {
                pad = team->GetPlayer(i)->GetGlobalPad();
                if (pad != NULL)
                {
                    padIndex = pad->m_padIndex;
                    hasPad = 1;
                    goto HavePad_GOALS_AGAINST;
                }
            }
            padIndex = -1;
            hasPad = 0;

        HavePad_GOALS_AGAINST:
            if (hasPad)
            {
                AddUserStatByPad(STATS_GOALS_AGAINST, padIndex, param0);
            }
        }
        break;
    case STATS_GOALS_FOR_STS:
        AddStat(STATS_GOALS_FOR_STS, homeaway, playerindex, param0);
        AddUserStatByPad(STATS_GOALS_FOR_STS, param1, param0);
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        AddStat(STATS_GOALS_FOR_ONE_TIMERS, homeaway, playerindex, param0);
        AddUserStatByPad(STATS_GOALS_FOR_ONE_TIMERS, param1, param0);
        break;
    case STATS_ASSISTS:
        AddStat(STATS_ASSISTS, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_ASSISTS;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_ASSISTS:
        if (hasPad)
        {
            AddUserStatByPad(STATS_ASSISTS, padIndex, 1);
        }
        break;
    case STATS_FOULS:
        AddStat(STATS_FOULS, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_FOULS;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_FOULS:
        if (hasPad)
        {
            AddUserStatByPad(STATS_FOULS, padIndex, 1);
        }
        break;
    case STATS_WIN:
        mIsUserCupWinner = false;
        AddStat(STATS_WIN, homeaway, -1, 1);
        mBasicGameInfo->mFinalScore[0] = param0;
        mBasicGameInfo->mFinalScore[1] = param1;
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_LOSS, homeaway == 0, 0, 0, 0, 0, 0);

        gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
        if (gameInfoManager->IsInCupMode())
        {
            if (gameInfoManager->mCupMatchRequirement != RESULT_INVALID)
            {
                teamID = mBasicGameInfo->mTeamIndex[homeaway];
                if (teamID == gameInfoManager->GetUserSelectedCupTeam())
                {
                    AwardCup(RESULT_USER_WINS);
                }
                else
                {
                    AwardCup(RESULT_USER_LOSES);
                }
            }
        }
        else if (gameInfoManager->IsInTournamentMode() && gameInfoManager->GetNumHumanTeams() == 1)
        {
            teamID = mBasicGameInfo->mTeamIndex[homeaway];
            if (teamID == gameInfoManager->GetUserSelectedCupTeam() && gameInfoManager->mCustomTournamentInfo.m_tournMode == TM_KNOCKOUT)
            {
                gameInfoManager->SetResultsOfLastUserGame(RESULT_CUP_WIN);
            }
        }
        break;
    case STATS_OT_WIN:
        mIsUserCupWinner = false;
        AddStat(STATS_OT_WIN, homeaway, -1, 1);
        mBasicGameInfo->mFinalScore[0] = param0;
        mBasicGameInfo->mFinalScore[1] = param1;
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_OT_LOSS, homeaway == 0, 0, 0, 0, 0, 0);

        gameInfoManager = nlSingleton<GameInfoManager>::s_pInstance;
        if (gameInfoManager->IsInCupMode())
        {
            if (gameInfoManager->mCupMatchRequirement != RESULT_INVALID)
            {
                teamID = mBasicGameInfo->mTeamIndex[homeaway];
                if (teamID == gameInfoManager->GetUserSelectedCupTeam())
                {
                    AwardCup(RESULT_USER_OT_WINS);
                }
                else
                {
                    AwardCup(RESULT_USER_OT_LOSES);
                }
            }
        }
        else if (gameInfoManager->IsInTournamentMode() && gameInfoManager->GetNumHumanTeams() == 1)
        {
            teamID = mBasicGameInfo->mTeamIndex[homeaway];
            if (teamID == gameInfoManager->GetUserSelectedCupTeam() && gameInfoManager->mCustomTournamentInfo.m_tournMode == TM_KNOCKOUT)
            {
                gameInfoManager->SetResultsOfLastUserGame(RESULT_CUP_WIN);
            }
        }
        break;
    case STATS_LOSS:
        AddStat(STATS_LOSS, homeaway, -1, 1);
        break;
    case STATS_OT_LOSS:
        AddStat(STATS_OT_LOSS, homeaway, -1, 1);
        break;
    case STATS_POWERUPS_USED:
        AddStat(STATS_POWERUPS_USED, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_POWERUPS_USED;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_POWERUPS_USED:
        if (hasPad)
        {
            AddUserStatByPad(STATS_POWERUPS_USED, padIndex, 1);
        }
        break;
    case STATS_POWERUPS_HIT:
        AddStat(STATS_POWERUPS_HIT, homeaway, playerindex, 1);
        AddUserStatByPad(STATS_POWERUPS_HIT, param0, 1);
        break;
    case STATS_PASSES_MADE:
        AddStat(STATS_PASSES_MADE, homeaway, playerindex, 1);
        AddUserStatByPad(STATS_PASSES_MADE, param0, 1);
        break;
    case STATS_PASSES_RECEIVED:
        AddStat(STATS_PASSES_RECEIVED, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_PASSES_RECEIVED;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_PASSES_RECEIVED:
        if (hasPad)
        {
            AddUserStatByPad(STATS_PASSES_RECEIVED, padIndex, 1);
        }
        break;
    case STATS_PASSES_INTERCEPTED:
        AddStat(STATS_PASSES_INTERCEPTED, homeaway, playerindex, 1);
        break;
    case STATS_POSSESION_TIME:
        AddStat(STATS_POSSESION_TIME, homeaway, playerindex, param0);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_POSSESION_TIME;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_POSSESION_TIME:
        if (hasPad)
        {
            AddUserStatByPad(STATS_POSSESION_TIME, padIndex, param0);
        }
        break;
    case STATS_GAMES_PLAYED:
        break;
    case STATS_STEALS:
        AddStat(STATS_STEALS, homeaway, playerindex, 1);
        AddUserStatByPad(STATS_STEALS, param0, 1);
        break;
    case STATS_BUTTON_PRESSES:
        AddStat(STATS_BUTTON_PRESSES, homeaway, playerindex, param1);
        AddUserStatByPad(STATS_BUTTON_PRESSES, param0, param1);
        break;
    case STATS_HITS_MADE:
        AddStat(STATS_HITS_MADE, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_HITS_MADE;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_HITS_MADE:
        if (hasPad)
        {
            AddUserStatByPad(STATS_HITS_MADE, padIndex, 1);
        }
        break;
    case STATS_STS_ATTEMPTS:
        AddStat(STATS_STS_ATTEMPTS, homeaway, playerindex, 1);
        team = g_pTeams[homeaway];
        if (team != NULL)
        {
            pad = team->GetPlayer(playerindex)->GetGlobalPad();
            if (pad != NULL)
            {
                padIndex = pad->m_padIndex;
                hasPad = 1;
                goto HavePad_STS_ATTEMPTS;
            }
        }
        padIndex = -1;
        hasPad = 0;

    HavePad_STS_ATTEMPTS:
        if (hasPad)
        {
            AddUserStatByPad(STATS_STS_ATTEMPTS, padIndex, 1);
        }
        break;
    case STATS_PERFECT_PASSES:
        AddStat(STATS_PERFECT_PASSES, homeaway, playerindex, 1);
        AddUserStatByPad(STATS_PERFECT_PASSES, param0, 1);
        break;
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x3708 | 0x80184C68 | size: 0x7E8
 * TODO: 56.94% match - compiler version issue: target uses GC/2.0 which generates
 * xor/srawi/and/subf/srwi for unsigned comparison, but decomp.me only has
 * mwcc_247_92 (GC/1.3.2) which generates subf/srwi. This 3-instruction difference
 * per comparison (x32 branches) accounts for ~19% of the function.
 * Also: register allocation differs (r29 not used as callee-saved for shouldSwap).
 */
void StatsTracker::GetSortedStats(PlayerStats* source, int numsource, int* dest, int numelements, ePlayerStats statType, eSortOrder sortOrder)
{
    int tempsorted[66];
    int i;

    for (i = 0; i < numsource; i++)
    {
        tempsorted[i] = i;
    }

    unsigned char swapped;
    unsigned char shouldSwap;
    do
    {
        swapped = 0;
        for (i = 0; i < numsource; i++)
        {
            if (i + 1 >= numsource)
                break;

            shouldSwap = 0;

            switch (statType)
            {
            case STATS_SHOTS_ON_GOAL:
            {
                unsigned short a = source[tempsorted[i]].mNumShotsOnGoal;
                unsigned short b = source[tempsorted[i + 1]].mNumShotsOnGoal;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_GOALS_FOR:
            {
                unsigned short a = source[tempsorted[i]].mNumGoalsFor;
                unsigned short b = source[tempsorted[i + 1]].mNumGoalsFor;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_GOALS_AGAINST:
            {
                unsigned short a = source[tempsorted[i]].mNumGoalsAgainst;
                unsigned short b = source[tempsorted[i + 1]].mNumGoalsAgainst;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_GOALS_FOR_STS:
            {
                unsigned short a = source[tempsorted[i]].mNumShootToScoreGoals;
                unsigned short b = source[tempsorted[i + 1]].mNumShootToScoreGoals;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_ASSISTS:
            {
                unsigned short a = source[tempsorted[i]].mNumAssists;
                unsigned short b = source[tempsorted[i + 1]].mNumAssists;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_FOULS:
            {
                unsigned short a = source[tempsorted[i]].mNumFouls;
                unsigned short b = source[tempsorted[i + 1]].mNumFouls;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_WIN:
            case STATS_OT_WIN:
            case STATS_LOSS:
            case STATS_OT_LOSS:
                break;
            case STATS_POWERUPS_USED:
            {
                unsigned short a = source[tempsorted[i]].mNumPowerupsUsed;
                unsigned short b = source[tempsorted[i + 1]].mNumPowerupsUsed;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_POWERUPS_HIT:
            {
                unsigned short a = source[tempsorted[i]].mNumPowerupsHit;
                unsigned short b = source[tempsorted[i + 1]].mNumPowerupsHit;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_PASSES_MADE:
            {
                unsigned short a = source[tempsorted[i]].mNumPassesMade;
                unsigned short b = source[tempsorted[i + 1]].mNumPassesMade;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_PASSES_RECEIVED:
            {
                unsigned short a = source[tempsorted[i]].mNumPassesMade;
                unsigned short b = source[tempsorted[i + 1]].mNumPassesMade;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_PASSES_INTERCEPTED:
            {
                unsigned short a = source[tempsorted[i]].mNumPassesIntercepted;
                unsigned short b = source[tempsorted[i + 1]].mNumPassesIntercepted;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_POSSESION_TIME:
            {
                unsigned long a = source[tempsorted[i]].mBallPossessionTime;
                unsigned long b = source[tempsorted[i + 1]].mBallPossessionTime;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_GAMES_PLAYED:
            {
                unsigned short a = source[tempsorted[i]].mNumGamesPlayed;
                unsigned short b = source[tempsorted[i + 1]].mNumGamesPlayed;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_STEALS:
            {
                unsigned short a = source[tempsorted[i]].mNumSteals;
                unsigned short b = source[tempsorted[i + 1]].mNumSteals;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_BUTTON_PRESSES:
            {
                unsigned long a = source[tempsorted[i]].mNumButtonPresses;
                unsigned long b = source[tempsorted[i + 1]].mNumButtonPresses;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            case STATS_HITS_MADE:
            {
                unsigned short a = source[tempsorted[i]].mNumHitsMade;
                unsigned short b = source[tempsorted[i + 1]].mNumHitsMade;
                if (sortOrder == SORT_DESCENDING)
                    shouldSwap = a < b;
                else
                    shouldSwap = b < a;
                break;
            }
            }

            if (shouldSwap)
            {
                int temp = tempsorted[i + 1];
                tempsorted[i + 1] = tempsorted[i];
                tempsorted[i] = temp;
                swapped = 1;
            }
        }
    } while (swapped);

    for (i = 0; i < numelements; i++)
    {
        dest[i] = tempsorted[i];
    }
}

static inline unsigned int IsHumanControlled(u16 humanTeams, eTeamID teamIdx)
{
    return (humanTeams & (1 << teamIdx)) != 0;
}

/**
 * Offset/Address/Size: 0x3340 | 0x801848A0 | size: 0x3C8
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
            int b = source[tempsorted[i + 1]].mNumPoints;
            unsigned int humanA = IsHumanControlled(humanTeams, source[tempsorted[i]].mTeamIndex);
            unsigned int humanB = IsHumanControlled(humanTeams, source[tempsorted[i + 1]].mTeamIndex);
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
/**
 * TODO: 92.4% match - MWCC strength-reduces mFinalScore[homeaway] index
 * into a persistent register accumulator (extra callee-saved register r24-r31
 * instead of r25-r31), causing all register assignments to shift by 1.
 * Target computes mFinalScore index inline (extsh+slwi+addi) each iteration.
 */
void StatsTracker::CompileEndOfGameStats()
{
    int homeAwayIndex[2] = { 0, 0 };
    eTeamID homeid = mBasicGameInfo->mTeamIndex[0];
    eTeamID awayid = mBasicGameInfo->mTeamIndex[1];
    GameInfoManager* gameInfoMgr = nlSingleton<GameInfoManager>::s_pInstance;
    int numTeams = gameInfoMgr->GetNumPlayingTeams();

    for (int i = 0; i < numTeams; i++)
    {
        if (gameInfoMgr->GetTeamStatsByIndex(i).mTeamIndex == homeid)
        {
            homeAwayIndex[0] = i;
        }
        if (gameInfoMgr->GetTeamStatsByIndex(i).mTeamIndex == awayid)
        {
            homeAwayIndex[1] = i;
        }
    }

    int tempStat;
    for (int homeaway = 0; homeaway < 2; homeaway++)
    {
        int team = homeAwayIndex[homeaway];

        if (mBasicGameInfo->mFinalScore[homeaway] != -5)
        {
            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumShotsOnGoal;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumShotsOnGoal += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumGoalsFor;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumGoalsFor += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumGoalsAgainst;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumGoalsAgainst += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumAssists;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumAssists += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumFouls;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumFouls += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPowerupsUsed;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPowerupsUsed += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPowerupsHit;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPowerupsHit += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumShootToScoreGoals;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumShootToScoreGoals += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPassesMade;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPassesMade += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPassesReceived;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPassesReceived += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPassesIntercepted;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPassesIntercepted += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumHitsMade;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumHitsMade += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumSteals;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumSteals += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumGoalsOneTimers;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumGoalsOneTimers += tempStat;

            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mBallPossessionTime += mCurrentTeamStats[homeaway].mPlayerTotalStats.mBallPossessionTime;

            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumButtonPresses += mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumButtonPresses;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumSTSAttempts;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumSTSAttempts += tempStat;

            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPerfectPasses += mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPerfectPasses;

            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumGamesPlayed++;

            if (nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
            {
                if (gameInfoMgr->pGetTeamStatsByIndex(team)->mTeamIndex == nlSingleton<GameInfoManager>::s_pInstance->GetUserSelectedCupTeam())
                {
                    AddMilestoneUserStat(STATS_GAMES_PLAYED, 1);
                    AddMilestoneUserStat(STATS_HITS_MADE, mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumHitsMade);
                    AddMilestoneUserStat(STATS_GOALS_FOR, mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumGoalsFor);
                    AddMilestoneUserStat(STATS_PERFECT_PASSES, mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPerfectPasses);
                    AddMilestoneUserStat(STATS_STS_ATTEMPTS, mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumSTSAttempts);
                }
            }
        }
        else
        {
            mBasicGameInfo->mFinalScore[homeaway] = 0;
        }

        tempStat = mCurrentTeamStats[homeaway].mNumWins;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumWins += tempStat;
        tempStat = mCurrentTeamStats[homeaway].mNumLosses;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumLosses += tempStat;
        tempStat = mCurrentTeamStats[homeaway].mNumOTLosses;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumOTLosses += tempStat;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumPoints += mCurrentTeamStats[homeaway].mNumPoints;
    }

    for (int i = 0; i < 4; i++)
    {
        PlayerStats* us = &nlSingleton<GameInfoManager>::s_pInstance->mUserStats[i];
        us->mNumShotsOnGoal += mCurrentUserStats[i].mNumShotsOnGoal;
        us->mNumGoalsFor += mCurrentUserStats[i].mNumGoalsFor;
        us->mNumGoalsAgainst = mCurrentUserStats[i].mNumGoalsAgainst;
        us->mNumAssists += mCurrentUserStats[i].mNumAssists;
        us->mNumFouls += mCurrentUserStats[i].mNumFouls;
        us->mNumPowerupsUsed += mCurrentUserStats[i].mNumPowerupsUsed;
        us->mNumPowerupsHit += mCurrentUserStats[i].mNumPowerupsHit;
        us->mNumShootToScoreGoals += mCurrentUserStats[i].mNumShootToScoreGoals;
        us->mNumPassesMade += mCurrentUserStats[i].mNumPassesMade;
        us->mNumPassesReceived += mCurrentUserStats[i].mNumPassesReceived;
        us->mNumPassesIntercepted += mCurrentUserStats[i].mNumPassesIntercepted;
        us->mNumHitsMade += mCurrentUserStats[i].mNumHitsMade;
        us->mNumSteals += mCurrentUserStats[i].mNumSteals;
        us->mBallPossessionTime += mCurrentUserStats[i].mBallPossessionTime;
        us->mNumButtonPresses += mCurrentUserStats[i].mNumButtonPresses;
        us->mNumGoalsOneTimers += mCurrentUserStats[i].mNumGoalsOneTimers;
        us->mNumSTSAttempts += mCurrentUserStats[i].mNumSTSAttempts;
        us->mNumPerfectPasses += mCurrentUserStats[i].mNumPerfectPasses;
        us->mNumGamesPlayed = mCurrentUserStats[i].mNumGamesPlayed;
    }
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
void StatsTracker::AddStat(ePlayerStats stat, int team, int player, int value)
{
    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        mCurrentPlayerStats[team][player].mNumShotsOnGoal += value;
        break;
    case STATS_GOALS_FOR:
        mCurrentPlayerStats[team][player].mNumGoalsFor += value;
        break;
    case STATS_GOALS_FOR_STS:
        mCurrentPlayerStats[team][player].mNumShootToScoreGoals += value;
        break;
    case STATS_GOALS_AGAINST:
    {
        int start = (player == -1) ? 0 : player;
        int end = (player == -1) ? 5 : start + 1;
        for (int i = start; i < end; i++)
        {
            mCurrentPlayerStats[team][i].mNumGoalsAgainst += value;
        }
        break;
    }
    case STATS_ASSISTS:
        mCurrentPlayerStats[team][player].mNumAssists += value;
        break;
    case STATS_FOULS:
        mCurrentPlayerStats[team][player].mNumFouls += value;
        break;
    case STATS_POWERUPS_USED:
        mCurrentPlayerStats[team][player].mNumPowerupsUsed += value;
        break;
    case STATS_POWERUPS_HIT:
        mCurrentPlayerStats[team][player].mNumPowerupsHit += value;
        break;
    case STATS_PASSES_MADE:
        mCurrentPlayerStats[team][player].mNumPassesMade += value;
        break;
    case STATS_PASSES_RECEIVED:
        mCurrentPlayerStats[team][player].mNumPassesReceived += value;
        break;
    case STATS_PASSES_INTERCEPTED:
        mCurrentPlayerStats[team][player].mNumPassesIntercepted += value;
        break;
    case STATS_POSSESION_TIME:
        mCurrentPlayerStats[team][player].mBallPossessionTime += value;
        break;
    case STATS_STEALS:
        mCurrentPlayerStats[team][player].mNumSteals += value;
        break;
    case STATS_HITS_MADE:
        mCurrentPlayerStats[team][player].mNumHitsMade += value;
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        mCurrentPlayerStats[team][player].mNumGoalsOneTimers += value;
        break;
    case STATS_STS_ATTEMPTS:
        mCurrentPlayerStats[team][player].mNumSTSAttempts += value;
        break;
    case STATS_PERFECT_PASSES:
        mCurrentPlayerStats[team][player].mNumPerfectPasses += value;
        break;
    default:
        break;
    }

    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumShotsOnGoal += value;
        break;
    case STATS_GOALS_FOR:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsFor += value;
        break;
    case STATS_GOALS_FOR_STS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumShootToScoreGoals += value;
        break;
    case STATS_GOALS_AGAINST:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsAgainst += value;
        break;
    case STATS_ASSISTS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumAssists += value;
        break;
    case STATS_FOULS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumFouls += value;
        break;
    case STATS_WIN:
        mCurrentTeamStats[team].mNumWins++;
        mCurrentTeamStats[team].mNumPoints += 3;
        break;
    case STATS_OT_WIN:
        mCurrentTeamStats[team].mNumWins++;
        mCurrentTeamStats[team].mNumPoints += 3;
        break;
    case STATS_LOSS:
        mCurrentTeamStats[team].mNumLosses++;
        break;
    case STATS_OT_LOSS:
        mCurrentTeamStats[team].mNumOTLosses++;
        mCurrentTeamStats[team].mNumPoints++;
        break;
    case STATS_POWERUPS_USED:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsUsed += value;
        break;
    case STATS_POWERUPS_HIT:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsHit += value;
        break;
    case STATS_PASSES_MADE:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesMade += value;
        break;
    case STATS_PASSES_RECEIVED:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesReceived += value;
        break;
    case STATS_PASSES_INTERCEPTED:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesIntercepted += value;
        break;
    case STATS_POSSESION_TIME:
        mCurrentTeamStats[team].mPlayerTotalStats.mBallPossessionTime += value;
        break;
    case STATS_STEALS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumSteals += value;
        break;
    case STATS_HITS_MADE:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumHitsMade += value;
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsOneTimers += value;
        break;
    case STATS_BUTTON_PRESSES:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumButtonPresses += value;
        break;
    case STATS_STS_ATTEMPTS:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumSTSAttempts += value;
        break;
    case STATS_PERFECT_PASSES:
        mCurrentTeamStats[team].mPlayerTotalStats.mNumPerfectPasses += value;
        break;
    default:
        break;
    }

    switch (stat)
    {
    case STATS_SHOTS_ON_GOAL:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumShotsOnGoal += value;
        break;
    case STATS_GOALS_FOR:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumGoalsFor += value;
        break;
    case STATS_GOALS_FOR_STS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumShootToScoreGoals += value;
        break;
    case STATS_GOALS_AGAINST:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumGoalsAgainst += value;
        break;
    case STATS_ASSISTS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumAssists += value;
        break;
    case STATS_FOULS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumFouls += value;
        break;
    case STATS_WIN:
        mCumulativeTeamStats[team].mNumWins++;
        mCumulativeTeamStats[team].mNumPoints += 3;
        break;
    case STATS_OT_WIN:
        mCumulativeTeamStats[team].mNumWins++;
        mCumulativeTeamStats[team].mNumPoints += 3;
        break;
    case STATS_LOSS:
        mCumulativeTeamStats[team].mNumLosses++;
        break;
    case STATS_OT_LOSS:
        mCumulativeTeamStats[team].mNumPoints++;
        break;
    case STATS_POWERUPS_USED:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPowerupsUsed += value;
        break;
    case STATS_POWERUPS_HIT:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPowerupsHit += value;
        break;
    case STATS_PASSES_MADE:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPassesMade += value;
        break;
    case STATS_PASSES_RECEIVED:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPassesReceived += value;
        break;
    case STATS_PASSES_INTERCEPTED:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPassesIntercepted += value;
        break;
    case STATS_POSSESION_TIME:
        mCumulativeTeamStats[team].mPlayerTotalStats.mBallPossessionTime += value;
        break;
    case STATS_STEALS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumSteals += value;
        break;
    case STATS_HITS_MADE:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumHitsMade += value;
        break;
    case STATS_GOALS_FOR_ONE_TIMERS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumGoalsOneTimers += value;
        break;
    case STATS_STS_ATTEMPTS:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumSTSAttempts += value;
        break;
    case STATS_PERFECT_PASSES:
        mCumulativeTeamStats[team].mPlayerTotalStats.mNumPerfectPasses += value;
        break;
    default:
        break;
    }
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
void StatsTracker::WriteStats(float gameTime, float gameDuration, const char* filename)
{
    unsigned char firstTime = 1;
    FILE* pFile;

    extern const char* STATS_FILE;
    extern unsigned long fwrite(const void*, unsigned long, unsigned long, void*);
    extern BasicString<char, Detail::TempStringAllocator> Format(
        const BasicString<char, Detail::TempStringAllocator>&,
        const char* const&,
        const char* const&,
        const char* const&,
        const char* const&,
        const char* const&,
        const float&,
        const float&);
    extern BasicString<char, Detail::TempStringAllocator> Format(
        const BasicString<char, Detail::TempStringAllocator>&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&);
    extern BasicString<char, Detail::TempStringAllocator> Format(
        const BasicString<char, Detail::TempStringAllocator>&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&);

    if (gameDuration <= 0.0f)
    {
        gameDuration = (float)nlSingleton<GameInfoManager>::s_pInstance->GetGameplayOptions().GameTime;
    }

    if (filename == 0)
    {
        filename = STATS_FILE;
    }

    pFile = fopen(filename, "r");
    if (pFile != 0)
    {
        firstTime = 0;
        fclose(pFile);
    }

    pFile = fopen(filename, firstTime ? "wt" : "at");
    if (pFile == 0)
    {
        return;
    }

    if (firstTime)
    {
        BasicString<char, Detail::TempStringAllocator> header;
        BasicString<char, Detail::TempStringAllocator> homeAway("H ");
        int i;

        header.AppendInPlace("H Captain, ");
        header.AppendInPlace("H Sidekick, ");
        header.AppendInPlace("A Captain, ");
        header.AppendInPlace("A Sidekick, ");
        header.AppendInPlace("Stadium, ");
        header.AppendInPlace("Game Time, ");
        header.AppendInPlace("Actual Time, ");

        for (i = 0; i < 2; i++)
        {
            header.AppendInPlace(homeAway.Append("Num Players, "));
            header.AppendInPlace(homeAway.Append("Difficulty, "));
            header.AppendInPlace(homeAway.Append("Shots On Goal, "));
            header.AppendInPlace(homeAway.Append("Goals For, "));
            header.AppendInPlace(homeAway.Append("ShootToScoreGoals, "));
            header.AppendInPlace(homeAway.Append("Assists, "));
            header.AppendInPlace(homeAway.Append("Fouls, "));
            header.AppendInPlace(homeAway.Append("Powerups Used, "));
            header.AppendInPlace(homeAway.Append("Powerups Hit, "));
            header.AppendInPlace(homeAway.Append("Passes Made, "));
            header.AppendInPlace(homeAway.Append("Passes Received, "));
            header.AppendInPlace(homeAway.Append("Passes Intercepted, "));
            header.AppendInPlace(homeAway.Append("Possession Time, "));
            header.AppendInPlace(homeAway.Append("Steals, "));
            header.AppendInPlace(homeAway.Append("Hits Made, "));
            header.AppendInPlace(homeAway.Append("Goals One Timers, "));
            homeAway = "A ";
        }

        header.AppendInPlace("\n");

        unsigned long len = header.size() > 0 ? (unsigned long)(header.size() - 1) : 0UL;
        fwrite(header.c_str(), 1, len, pFile);
    }

    int numHumans[2] = { 0, 0 };
    int i;
    for (i = 0; i < 4; i++)
    {
        short side = nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i);
        if (side == 0)
        {
            numHumans[0]++;
        }
        else if (side == 1)
        {
            numHumans[1]++;
        }
    }

    BasicString<char, Detail::TempStringAllocator> stats;
    bool allCaptains = Config::Global().Get("allcaptains", false);

    if (allCaptains)
    {
        BasicString<char, Detail::TempStringAllocator> formatTemplate("{0},{1},{2},{3},{4},{5},{6},");
        const char* stadiumName = TheWorldLoader.GetStadiumFilename(nlSingleton<GameInfoManager>::s_pInstance->GetStadium());
        const char* awayTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
        const char* homeTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));

        stats = Format(formatTemplate,
            homeTeamName,
            homeTeamName,
            awayTeamName,
            awayTeamName,
            stadiumName,
            gameTime,
            gameDuration);
    }
    else
    {
        BasicString<char, Detail::TempStringAllocator> formatTemplate("{0},{1},{2},{3},{4},{5},{6},");
        const char* stadiumName = TheWorldLoader.GetStadiumFilename(nlSingleton<GameInfoManager>::s_pInstance->GetStadium());
        const char* awaySidekickName = GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1));
        const char* awayTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
        const char* homeSidekickName = GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0));
        const char* homeTeamName = GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));

        stats = Format(formatTemplate,
            homeTeamName,
            homeSidekickName,
            awayTeamName,
            awaySidekickName,
            stadiumName,
            gameTime,
            gameDuration);
    }

    for (int team = 0; team < 2; team++)
    {
        int possession = (int)(mCurrentTeamStats[team].mPlayerTotalStats.mBallPossessionTime / 100);
        int difficulty = (int)nlSingleton<GameInfoManager>::s_pInstance->mCurrentDifficulty[team];

        BasicString<char, Detail::TempStringAllocator> numPlayersTemplate("{0},");
        BasicString<char, Detail::TempStringAllocator> statsAfterPlayers = stats.Append(Format(numPlayersTemplate, numHumans[team]));

        BasicString<char, Detail::TempStringAllocator> firstStatsTemplate("{0},{1},{2},{3},{4},{5},{6},");
        BasicString<char, Detail::TempStringAllocator> statsAfterFirst = statsAfterPlayers.Append(Format(firstStatsTemplate,
            difficulty,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumShotsOnGoal,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsFor,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumShootToScoreGoals,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumAssists,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumFouls,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsUsed));

        BasicString<char, Detail::TempStringAllocator> secondStatsTemplate("{0},{1},{2},{3},{4},{5},{6},{7}");
        BasicString<char, Detail::TempStringAllocator> fullStats = statsAfterFirst.Append(Format(secondStatsTemplate,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsHit,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesMade,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesReceived,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesIntercepted,
            possession,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumSteals,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumHitsMade,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsOneTimers));

        stats = fullStats;

        if (team == 0)
        {
            stats = stats.Append(",");
        }
    }

    stats.AppendInPlace("\n");
    {
        unsigned long len = stats.size() > 0 ? (unsigned long)(stats.size() - 1) : 0UL;
        fwrite(stats.c_str(), 1, len, pFile);
    }

    fclose(pFile);
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

    stadiumName = TheWorldLoader.GetStadiumFilename(
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
