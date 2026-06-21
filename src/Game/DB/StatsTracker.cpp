#define BASICSTRING_INLINE_ERASE
#include "Game/DB/StatsTracker.h"
#include "Game/AI/FielderActions.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/World/WorldLoader.h"
#include "NL/nlFormat.h"

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

typedef FormatImpl<NLString> NLFormatImpl;

/**
 * Offset/Address/Size: 0x1DAC | 0x80188948 | size: 0xD74
 * TODO: 96.56% match - reconstructed-template wall: marker checks emit indexed
 * lbzx vs target base+disp lbz, and insert lacks a null-check branch.
 */
template <>
NLFormatImpl& NLFormatImpl::operator% <const char*>(const char* const& t)
{
    NLString insert = LexicalCast<NLString, const char*>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != (char)'{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 1] - '0' != mCurrentPos)
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 2] != (char)'}')
            continue;

        mString.erase(&mString[i], &mString[i + 3]);
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        char* insertBegin = &insert[0];
        char* insertEndCow = &insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? &insert.m_data->mData[insert.m_data->mSize - 1] : (char*)0);
    }

    mCurrentPos++;
    return *this;
}

/**
 * Offset/Address/Size: 0x1038 | 0x80187BD4 | size: 0xD74
 * TODO: 96.56% match - reconstructed-template wall: marker checks emit indexed
 * lbzx vs target base+disp lbz, and insert lacks a null-check branch.
 */
template <>
NLFormatImpl& NLFormatImpl::operator% <float>(const float& t)
{
    NLString insert = LexicalCast<NLString, float>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != (char)'{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 1] - '0' != mCurrentPos)
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        if (mString[i + 2] != (char)'}')
            continue;

        mString.erase(&mString[i], &mString[i + 3]);
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        char* insertBegin = &insert[0];
        char* insertEndCow = &insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? &insert.m_data->mData[insert.m_data->mSize - 1] : (char*)0);
    }

    mCurrentPos++;
    return *this;
}

/**
 * Offset/Address/Size: 0xEDC | 0x80187A78 | size: 0x15C
 */
template <typename StringType, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline StringType Format(
    const StringType& format,
    const T1& value1,
    const T2& value2,
    const T3& value3,
    const T4& value4,
    const T5& value5,
    const T6& value6,
    const T7& value7);

template <>
inline BasicString<char, Detail::TempStringAllocator>
Format<BasicString<char, Detail::TempStringAllocator>,
    const char*,
    const char*,
    const char*,
    const char*,
    const char*,
    float,
    float>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const char* const& value1,
    const char* const& value2,
    const char* const& value3,
    const char* const& value4,
    const char* const& value5,
    const float& value6,
    const float& value7)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl(data);

    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)(((((((impl % value1) % value2) % value3) % value4) % value5) % value6) % value7));
}

void StatsTracker_Format7_stub()
{
    BasicString<char, Detail::TempStringAllocator> s;
    const char* p = "";
    float f = 0.0f;
    Format(s, p, p, p, p, p, f, f);
}

// /**
//  * Offset/Address/Size: 0x168 | 0x80186D04 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<int>(const int&)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80186B9C | size: 0x168
 */
template <typename StringType, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline StringType Format(
    const StringType& format,
    const T1& value1,
    const T2& value2,
    const T3& value3,
    const T4& value4,
    const T5& value5,
    const T6& value6,
    const T7& value7,
    const T8& value8);

template <>
inline BasicString<char, Detail::TempStringAllocator>
Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int, int>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const int& value1,
    const int& value2,
    const int& value3,
    const int& value4,
    const int& value5,
    const int& value6,
    const int& value7,
    const int& value8)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl(data);

    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)((((((((impl % value1) % value2) % value3) % value4) % value5) % value6) % value7) % value8));
}

void StatsTracker_Format8_stub()
{
    typedef BasicString<char, Detail::TempStringAllocator> (*FmtFnI8)(
        const BasicString<char, Detail::TempStringAllocator>&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&);
    volatile FmtFnI8 fn = &Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int, int>;
    (void)fn;
}

/**
 * Offset/Address/Size: 0x2B20 | 0x801896BC | size: 0x15C
 */
template <>
inline BasicString<char, Detail::TempStringAllocator>
Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const int& value1,
    const int& value2,
    const int& value3,
    const int& value4,
    const int& value5,
    const int& value6,
    const int& value7)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl(data);

    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)(((((((impl % value1) % value2) % value3) % value4) % value5) % value6) % value7));
}

void StatsTracker_Format7i_stub()
{
    typedef BasicString<char, Detail::TempStringAllocator> (*FmtFnI7)(
        const BasicString<char, Detail::TempStringAllocator>&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&,
        const int&);
    volatile FmtFnI7 fn = &Format<BasicString<char, Detail::TempStringAllocator>, int, int, int, int, int, int, int>;
    (void)fn;
}

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

static inline void ResetTeamStats(TeamStats& cur, const TeamStats& cum)
{
    eTeamID teamIdx = cum.mTeamIndex;
    memset(&cur.mPlayerTotalStats, 0, sizeof(PlayerStats));
    cur.mPlayerTotalStats.mRecordType.mTeamID = teamIdx;
    cur.mPlayerTotalStats.mType = TYPE_TEAM;
    cur.mTeamIndex = teamIdx;
    cur.mNumWins = 0;
    cur.mNumLosses = 0;
    cur.mNumOTLosses = 0;
    cur.mNumPoints = 0;
}

/**
 * Offset/Address/Size: 0x4FD0 | 0x80186530 | size: 0xF8
 */
void StatsTracker::ResetCurrentStats()
{
    int i;

    mIsOvertime = false;
    mHasGameEnded = false;

    ResetTeamStats(mCurrentTeamStats[0], mCumulativeTeamStats[0]);
    ResetTeamStats(mCurrentTeamStats[1], mCumulativeTeamStats[1]);

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
    u32 i;

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
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_GOALS_AGAINST, ((homeaway == 0) ? 1 : 0), playerindex, param2, 0, 0, 0);
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
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_LOSS, ((homeaway == 0) ? 1 : 0), 0, 0, 0, 0, 0);

        gameInfoManager = nlSingleton<GameInfoManager>::Instance();
        if (gameInfoManager->IsInCupMode() == 1)
        {
            if (gameInfoManager->mCupMatchRequirement != RESULT_INVALID)
            {
                teamID = mBasicGameInfo->mTeamIndex[(short)homeaway];
                if (teamID == gameInfoManager->GetUserSelectedCupTeam())
                {
                    AwardCup(RESULT_USER_WINS);
                    break;
                }
                else
                {
                    AwardCup(RESULT_USER_LOSES);
                    break;
                }
            }
        }
        if (gameInfoManager->IsInTournamentMode() == 1 && gameInfoManager->GetNumHumanTeams() == 1)
        {
            teamID = mBasicGameInfo->mTeamIndex[(short)homeaway];
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
        nlSingleton<StatsTracker>::s_pInstance->TrackStat(STATS_OT_LOSS, ((homeaway == 0) ? 1 : 0), 0, 0, 0, 0, 0);

        gameInfoManager = nlSingleton<GameInfoManager>::Instance();
        if (gameInfoManager->IsInCupMode() == 1)
        {
            if (gameInfoManager->mCupMatchRequirement != RESULT_INVALID)
            {
                teamID = mBasicGameInfo->mTeamIndex[(short)homeaway];
                if (teamID == gameInfoManager->GetUserSelectedCupTeam())
                {
                    AwardCup(RESULT_USER_OT_WINS);
                    break;
                }
                else
                {
                    AwardCup(RESULT_USER_OT_LOSES);
                    break;
                }
            }
        }
        if (gameInfoManager->IsInTournamentMode() == 1 && gameInfoManager->GetNumHumanTeams() == 1)
        {
            teamID = mBasicGameInfo->mTeamIndex[(short)homeaway];
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
 * TODO: 95.79% match - r29/r31 swap in unrolled init loop, extra clrlwi after compare assignments, r10/r11 source/index swaps in stat loop and copy tail
 */
static inline int CompareInt(eSortOrder sortOrder, int a, int b)
{
    if (sortOrder == SORT_DESCENDING)
    {
        return a < b;
    }
    return b < a;
}

void StatsTracker::GetSortedStats(PlayerStats* source, int numsource, int* dest, int numelements, ePlayerStats statType, eSortOrder sortOrder)
{
    int tempsorted[66];
    int i;

    for (i = 0; i < numsource; i++)
    {
        tempsorted[i] = i;
    }

    unsigned char swapped;
    do
    {
        swapped = 0;
        for (i = 0; i < numsource; i++)
        {
            int nexti = i + 1;
            if (nexti >= numsource)
                break;

            unsigned char doswap = 0;

            switch (statType)
            {
            case STATS_SHOTS_ON_GOAL:
            {
                unsigned short a = source[tempsorted[nexti]].mNumShotsOnGoal;
                unsigned short b = source[tempsorted[i]].mNumShotsOnGoal;
                doswap = CompareInt(sortOrder, b, a);
                break;
            }
            case STATS_GOALS_FOR:
            {
                unsigned short a = source[tempsorted[nexti]].mNumGoalsFor;
                unsigned short b = source[tempsorted[i]].mNumGoalsFor;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_GOALS_AGAINST:
            {
                unsigned short a = source[tempsorted[nexti]].mNumGoalsAgainst;
                unsigned short b = source[tempsorted[i]].mNumGoalsAgainst;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_ASSISTS:
            {
                unsigned short a = source[tempsorted[nexti]].mNumAssists;
                unsigned short b = source[tempsorted[i]].mNumAssists;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_FOULS:
            {
                unsigned short a = source[tempsorted[nexti]].mNumFouls;
                unsigned short b = source[tempsorted[i]].mNumFouls;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_WIN:
            case STATS_OT_WIN:
            case STATS_LOSS:
            case STATS_OT_LOSS:
                break;
            case STATS_POWERUPS_USED:
            {
                unsigned short a = source[tempsorted[nexti]].mNumPowerupsUsed;
                unsigned short b = source[tempsorted[i]].mNumPowerupsUsed;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_POWERUPS_HIT:
            {
                unsigned short a = source[tempsorted[nexti]].mNumPowerupsHit;
                unsigned short b = source[tempsorted[i]].mNumPowerupsHit;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_GOALS_FOR_STS:
            {
                unsigned short a = source[tempsorted[nexti]].mNumShootToScoreGoals;
                unsigned short b = source[tempsorted[i]].mNumShootToScoreGoals;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_PASSES_MADE:
            {
                unsigned short a = source[tempsorted[nexti]].mNumPassesMade;
                unsigned short b = source[tempsorted[i]].mNumPassesMade;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_PASSES_RECEIVED:
            {
                unsigned short a = source[tempsorted[nexti]].mNumPassesMade;
                unsigned short b = source[tempsorted[i]].mNumPassesMade;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_PASSES_INTERCEPTED:
            {
                unsigned short a = source[tempsorted[nexti]].mNumPassesIntercepted;
                unsigned short b = source[tempsorted[i]].mNumPassesIntercepted;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_POSSESION_TIME:
            {
                unsigned long a = source[tempsorted[nexti]].mBallPossessionTime;
                unsigned long b = source[tempsorted[i]].mBallPossessionTime;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_GAMES_PLAYED:
            {
                unsigned short a = source[tempsorted[nexti]].mNumGamesPlayed;
                unsigned short b = source[tempsorted[i]].mNumGamesPlayed;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_STEALS:
            {
                unsigned short a = source[tempsorted[nexti]].mNumSteals;
                unsigned short b = source[tempsorted[i]].mNumSteals;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_BUTTON_PRESSES:
            {
                unsigned long a = source[tempsorted[nexti]].mNumButtonPresses;
                unsigned long b = source[tempsorted[i]].mNumButtonPresses;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            case STATS_HITS_MADE:
            {
                unsigned short a = source[tempsorted[nexti]].mNumHitsMade;
                unsigned short b = source[tempsorted[i]].mNumHitsMade;
                doswap = CompareInt(sortOrder, a, b);
                break;
            }
            }

            if (doswap)
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

static inline void AccumulateUserStats(PlayerStats* us, const PlayerStats& current)
{
    us->mNumShotsOnGoal += current.mNumShotsOnGoal;
    us->mNumGoalsFor += current.mNumGoalsFor;
    us->mNumGoalsAgainst = current.mNumGoalsAgainst;
    us->mNumAssists += current.mNumAssists;
    us->mNumFouls += current.mNumFouls;
    us->mNumPowerupsUsed += current.mNumPowerupsUsed;
    us->mNumPowerupsHit += current.mNumPowerupsHit;
    us->mNumShootToScoreGoals += current.mNumShootToScoreGoals;
    us->mNumPassesMade += current.mNumPassesMade;
    us->mNumPassesReceived += current.mNumPassesReceived;
    us->mNumPassesIntercepted += current.mNumPassesIntercepted;
    us->mNumHitsMade += current.mNumHitsMade;
    us->mNumSteals += current.mNumSteals;
    us->mBallPossessionTime += current.mBallPossessionTime;
    us->mNumButtonPresses += current.mNumButtonPresses;
    us->mNumGoalsOneTimers += current.mNumGoalsOneTimers;
    us->mNumSTSAttempts += current.mNumSTSAttempts;
    us->mNumPerfectPasses += current.mNumPerfectPasses;
    us->mNumGamesPlayed = current.mNumGamesPlayed;
}

/**
 * Offset/Address/Size: 0x2E00 | 0x80184360 | size: 0x540
 */
void StatsTracker::CompileEndOfGameStats()
{
    int homeAwayIndex[2] = { -1, -1 };
    eTeamID homeid = mBasicGameInfo->mTeamIndex[0];
    eTeamID awayid = mBasicGameInfo->mTeamIndex[1];
    GameInfoManager* gameInfoMgr = nlSingleton<GameInfoManager>::Instance();
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

        if (mBasicGameInfo->mFinalScore[(short)homeaway] != -5)
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

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mBallPossessionTime;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mBallPossessionTime += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumButtonPresses;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumButtonPresses += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumSTSAttempts;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumSTSAttempts += tempStat;

            tempStat = mCurrentTeamStats[homeaway].mPlayerTotalStats.mNumPerfectPasses;
            gameInfoMgr->pGetTeamStatsByIndex(team)->mPlayerTotalStats.mNumPerfectPasses += tempStat;

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
            mBasicGameInfo->mFinalScore[(short)homeaway] = 0;
        }

        tempStat = mCurrentTeamStats[homeaway].mNumWins;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumWins += tempStat;
        tempStat = mCurrentTeamStats[homeaway].mNumLosses;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumLosses += tempStat;
        tempStat = mCurrentTeamStats[homeaway].mNumOTLosses;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumOTLosses += tempStat;
        tempStat = mCurrentTeamStats[homeaway].mNumPoints;
        gameInfoMgr->pGetTeamStatsByIndex(team)->mNumPoints += tempStat;
    }

    for (int i = 0; i < 4; i++)
    {
        AccumulateUserStats(&nlSingleton<GameInfoManager>::s_pInstance->mUserStats[i], mCurrentUserStats[i]);
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

    extern const char* STATS_FILE;
    extern unsigned long fwrite(const void*, unsigned long, unsigned long, void*);

    if (gameDuration <= 0.0f)
    {
        gameDuration = (float)nlSingleton<GameInfoManager>::s_pInstance->GetGameplayOptions().GameTime;
    }

    if (filename == 0)
    {
        filename = STATS_FILE;
    }

    FILE* pFile = fopen(filename, "r");
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
        int i;

        header.AppendInPlace("H Captain, ");
        header.AppendInPlace("H Sidekick, ");
        header.AppendInPlace("A Captain, ");
        header.AppendInPlace("A Sidekick, ");
        header.AppendInPlace("Stadium, ");
        header.AppendInPlace("Game Time, ");
        header.AppendInPlace("Actual Time, ");
        BasicStringInternal* homeAwayData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (homeAwayData != 0)
        {
            const char* homeAwayText = "H ";
            homeAwayData->mData = 0;
            homeAwayData->mSize = 0;
            homeAwayData->mCapacity = 0;

            const char* p = homeAwayText;
            while ((signed char)*p++ != 0)
            {
                homeAwayData->mSize++;
            }

            homeAwayData->mSize++;
            homeAwayData->mData = (char*)nlMalloc((homeAwayData->mSize + 1) * sizeof(char), 8, true);
            homeAwayData->mCapacity = homeAwayData->mSize;
            for (int copyIndex = 0; copyIndex < homeAwayData->mSize; copyIndex++)
            {
                homeAwayData->mData[copyIndex] = *homeAwayText++;
            }
            homeAwayData->mRefCount = 1;
        }

        BasicString<char, Detail::TempStringAllocator> homeAway(homeAwayData);
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
            homeAwayData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
            if (homeAwayData != 0)
            {
                const char* awayText = "A ";
                homeAwayData->mData = 0;
                homeAwayData->mSize = 0;
                homeAwayData->mCapacity = 0;

                const char* p = awayText;
                while ((signed char)*p++ != 0)
                {
                    homeAwayData->mSize++;
                }

                homeAwayData->mSize++;
                homeAwayData->mData = (char*)nlMalloc((homeAwayData->mSize + 1) * sizeof(char), 8, true);
                homeAwayData->mCapacity = homeAwayData->mSize;
                for (int copyIndex = 0; copyIndex < homeAwayData->mSize; copyIndex++)
                {
                    homeAwayData->mData[copyIndex] = *awayText++;
                }
                homeAwayData->mRefCount = 1;
            }

            homeAway = BasicString<char, Detail::TempStringAllocator>(homeAwayData);
        }

        header.AppendInPlace("\n");

        unsigned long len;
        if (header.m_data != 0)
        {
            len = (unsigned long)(header.m_data->mSize - 1);
        }
        else
        {
            len = 0UL;
        }
        fwrite(header.c_str(), 1, len, pFile);
    }

    int numHumans[2] = { 0, 0 };
    int i;
    for (i = 0; i < 4; i++)
    {
        if (nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i) == 0)
        {
            numHumans[0]++;
        }
        else if (nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)i) == 1)
        {
            numHumans[1]++;
        }
    }

    BasicString<char, Detail::TempStringAllocator> stats;
    Config& cfg = Config::Global();
    TagValuePair& tvp = cfg.FindTvp("allcaptains");
    bool allCaptains;
    if (tvp.tag == NULL)
    {
        cfg.Set("allcaptains", false);
        allCaptains = false;
    }
    else if (tvp.type == _BOOL)
    {
        allCaptains = LexicalCast<bool, bool>(tvp.value.b);
    }
    else if (tvp.type == _INT)
    {
        allCaptains = LexicalCast<bool, int>(tvp.value.i);
    }
    else if (tvp.type == _FLOAT)
    {
        allCaptains = LexicalCast<bool, float>(tvp.value.f);
    }
    else if (tvp.type == _STRING)
    {
        allCaptains = LexicalCast<bool, const char*>(tvp.value.s);
    }
    else
    {
        allCaptains = false;
    }

    if (allCaptains)
    {
        BasicStringInternal* fmtData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (fmtData != 0)
        {
            const char* fmtText = "{0},{1},{2},{3},{4},{5},{6},";
            fmtData->mData = 0;
            fmtData->mSize = 0;
            fmtData->mCapacity = 0;

            const char* p = fmtText;
            while ((signed char)*p++ != 0)
            {
                fmtData->mSize++;
            }

            fmtData->mSize++;
            fmtData->mData = (char*)nlMalloc((fmtData->mSize + 1) * sizeof(char), 8, true);
            fmtData->mCapacity = fmtData->mSize;
            for (int copyIndex = 0; copyIndex < fmtData->mSize; copyIndex++)
            {
                fmtData->mData[copyIndex] = *fmtText++;
            }
            fmtData->mRefCount = 1;
        }
        BasicString<char, Detail::TempStringAllocator> formatTemplate(fmtData);

        stats = Format(formatTemplate,
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0)),
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0)),
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1)),
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1)),
            TheWorldLoader.GetStadiumFilename(nlSingleton<GameInfoManager>::s_pInstance->GetStadium()),
            gameTime,
            gameDuration);
    }
    else
    {
        BasicStringInternal* fmtData2 = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (fmtData2 != 0)
        {
            const char* fmtText2 = "{0},{1},{2},{3},{4},{5},{6},";
            fmtData2->mData = 0;
            fmtData2->mSize = 0;
            fmtData2->mCapacity = 0;

            const char* p = fmtText2;
            while ((signed char)*p++ != 0)
            {
                fmtData2->mSize++;
            }

            fmtData2->mSize++;
            fmtData2->mData = (char*)nlMalloc((fmtData2->mSize + 1) * sizeof(char), 8, true);
            fmtData2->mCapacity = fmtData2->mSize;
            for (int copyIndex = 0; copyIndex < fmtData2->mSize; copyIndex++)
            {
                fmtData2->mData[copyIndex] = *fmtText2++;
            }
            fmtData2->mRefCount = 1;
        }
        BasicString<char, Detail::TempStringAllocator> formatTemplate(fmtData2);

        stats = Format(formatTemplate,
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0)),
            GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0)),
            GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1)),
            GetSidekickName(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1)),
            TheWorldLoader.GetStadiumFilename(nlSingleton<GameInfoManager>::s_pInstance->GetStadium()),
            gameTime,
            gameDuration);
    }

    int team;
    for (team = 0; team < 2; team++)
    {
        int possession = (int)(mCurrentTeamStats[team].mPlayerTotalStats.mBallPossessionTime / 100);
        int difficulty = (int)nlSingleton<GameInfoManager>::s_pInstance->mCurrentDifficulty[team];

        BasicStringInternal* npData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (npData != 0)
        {
            const char* npText = "{0},";
            npData->mData = 0;
            npData->mSize = 0;
            npData->mCapacity = 0;
            const char* p = npText;
            while ((signed char)*p++ != 0)
            {
                npData->mSize++;
            }
            npData->mSize++;
            npData->mData = (char*)nlMalloc((npData->mSize + 1) * sizeof(char), 8, true);
            npData->mCapacity = npData->mSize;
            for (int copyIndex = 0; copyIndex < npData->mSize; copyIndex++)
            {
                npData->mData[copyIndex] = *npText++;
            }
            npData->mRefCount = 1;
        }
        BasicString<char, Detail::TempStringAllocator> numPlayersTemplate(npData);
        BasicString<char, Detail::TempStringAllocator> statsAfterPlayers = stats.Append(Format(numPlayersTemplate, numHumans[team]));

        BasicStringInternal* fsData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (fsData != 0)
        {
            const char* fsText = "{0},{1},{2},{3},{4},{5},{6},";
            fsData->mData = 0;
            fsData->mSize = 0;
            fsData->mCapacity = 0;
            const char* p = fsText;
            while ((signed char)*p++ != 0)
            {
                fsData->mSize++;
            }
            fsData->mSize++;
            fsData->mData = (char*)nlMalloc((fsData->mSize + 1) * sizeof(char), 8, true);
            fsData->mCapacity = fsData->mSize;
            for (int copyIndex = 0; copyIndex < fsData->mSize; copyIndex++)
            {
                fsData->mData[copyIndex] = *fsText++;
            }
            fsData->mRefCount = 1;
        }
        BasicString<char, Detail::TempStringAllocator> firstStatsTemplate(fsData);
        BasicString<char, Detail::TempStringAllocator> statsAfterFirst = statsAfterPlayers.Append(Format(firstStatsTemplate,
            difficulty,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumShotsOnGoal,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsFor,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumShootToScoreGoals,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumAssists,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumFouls,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsUsed));

        BasicStringInternal* ssData = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (ssData != 0)
        {
            const char* ssText = "{0},{1},{2},{3},{4},{5},{6},{7}";
            ssData->mData = 0;
            ssData->mSize = 0;
            ssData->mCapacity = 0;
            const char* p = ssText;
            while ((signed char)*p++ != 0)
            {
                ssData->mSize++;
            }
            ssData->mSize++;
            ssData->mData = (char*)nlMalloc((ssData->mSize + 1) * sizeof(char), 8, true);
            ssData->mCapacity = ssData->mSize;
            for (int copyIndex = 0; copyIndex < ssData->mSize; copyIndex++)
            {
                ssData->mData[copyIndex] = *ssText++;
            }
            ssData->mRefCount = 1;
        }
        BasicString<char, Detail::TempStringAllocator> secondStatsTemplate(ssData);
        stats = statsAfterFirst.Append(Format(secondStatsTemplate,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPowerupsHit,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesMade,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesReceived,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumPassesIntercepted,
            possession,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumSteals,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumHitsMade,
            (int)mCurrentTeamStats[team].mPlayerTotalStats.mNumGoalsOneTimers));

        if (team == 0)
        {
            stats = stats.Append(",");
        }
    }

    stats.AppendInPlace("\n");
    {
        unsigned long len;
        if (stats.m_data != 0)
        {
            len = (unsigned long)(stats.m_data->mSize - 1);
        }
        else
        {
            len = 0UL;
        }
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

    BasicStringInternal* data = outputData;
    if (data != 0)
    {
        data->mRefCount++;
        data = outputData;
    }
    else
    {
        data = 0;
    }
    BasicStringInternal* toFree = outputData;
    retainedData = data;

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

void StatsTracker_stub()
{
    BasicString<char, Detail::TempStringAllocator> s;
    const char* p = "";
    Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, const char*, const char*>(s, p, p, p, p, p);
}
