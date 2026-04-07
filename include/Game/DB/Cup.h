#ifndef _CUP_H_
#define _CUP_H_

#include "types.h"
#include "Game/Team.h"
#include "Game/DB/UserOptions.h"
#include "Game/DB/StatsTracker.h"

enum eSidekickID
{
    SK_MYSTERY = -2,
    SK_INVALID = -1,
    SK_TOAD = 0,
    SK_KOOPA = 1,
    SK_HAMMERBROS = 2,
    SK_BIRDO = 3,
    NUM_SIDEKICKS = 4,
};

enum eStadiumID
{
    STAD_INVALID = -1,
    STAD_MARIO_STADIUM = 0,
    STAD_PEACH_TOAD_STADIUM = 1,
    STAD_DK_DAISY = 2,
    STAD_WARIO_STADIUM = 3,
    STAD_YOSHI_STADIUM = 4,
    STAD_SUPER_STADIUM = 5,
    STAD_FORBIDDEN_DOME = 6,
    MAX_STADIUMS = 7,
};

struct BasicGameInfo
{
    /* 0x00 */ eTeamID mTeamIndex[2];
    /* 0x08 */ eSidekickID mSidekickIndex[2];
    /* 0x10 */ eStadiumID mStadiumIndex;
    /* 0x14 */ s16 mPadSides[4];
    /* 0x1C */ s16 mFinalScore[2];

    BasicGameInfo()
    {
        mTeamIndex[0] = TEAM_MARIO;
        mTeamIndex[1] = TEAM_LUIGI;
        mSidekickIndex[0] = SK_TOAD;
        mSidekickIndex[1] = SK_KOOPA;
        mFinalScore[1] = 0;
        mFinalScore[0] = 0;
        mPadSides[0] = -1;
        mPadSides[1] = -1;
        mPadSides[2] = -1;
        mPadSides[3] = -1;
        mStadiumIndex = STAD_MARIO_STADIUM;
    }
}; // total size: 0x20

struct CupRecord
{
    CupRecord()
    {
        memset(&mDate, 0, sizeof(mDate));
        mPlace = 0;
        mTeam = TEAM_INVALID;
        mDifficulty = (GameplaySettings::eSkillLevel)1;
    }

    /* 0x00 */ OSCalendarTime mDate;
    /* 0x28 */ signed char mPlace;
    /* 0x2C */ eTeamID mTeam;
    /* 0x30 */ GameplaySettings::eSkillLevel mDifficulty;
}; // total size: 0x34

struct BaseCup
{
    /* 0x0, */ eTeamID mUserSelectedTeam;
    /* 0x4, */ eSidekickID mUserSelectedSidekick;
    /* 0x8, */ s16 mRoundNumber;
    /* 0xA, */ s16 mGameNumber;
    /* 0xC, */ u16 mHumanTeams;
    /* 0xE, */ bool mCupStarted;
    /* 0x10 */ GameplaySettings mCupSettings;

    BaseCup()
        : mUserSelectedTeam(TEAM_INVALID)
        , mUserSelectedSidekick(SK_INVALID)
        , mRoundNumber(0)
        , mGameNumber(0)
        , mHumanTeams(0)
        , mCupStarted(false)
        , mCupSettings()
    {
    }

    virtual BasicGameInfo* GetGameInfo(int team, int round) = 0;
    virtual TeamStats* GetTeamStats(int team) = 0;
    virtual u16 GetNumTeams() = 0;
    virtual u16 GetNumRounds() = 0;
    virtual signed char* GetRoundResults(int round) = 0;
    virtual void* SerializeData(void* dst) const
    {
        memcpy(dst, &mUserSelectedTeam, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mUserSelectedSidekick, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mRoundNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mGameNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mHumanTeams, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mCupStarted, 1);
        dst = (u8*)dst + 1;
        memcpy(dst, &mCupSettings, sizeof(mCupSettings));
        return (u8*)dst + sizeof(mCupSettings);
    };
    virtual void* DeserializeData(void* src)
    {
        memcpy(&mUserSelectedTeam, src, 4);
        src = (u8*)src + 4;
        memcpy(&mUserSelectedSidekick, src, 4);
        src = (u8*)src + 4;
        memcpy(&mRoundNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mGameNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mHumanTeams, src, 2);
        src = (u8*)src + 2;
        memcpy(&mCupStarted, src, 1);
        src = (u8*)src + 1;
        memcpy(&mCupSettings, src, sizeof(mCupSettings));
        src = (u8*)src + sizeof(mCupSettings);
        return src;
    };
    virtual int GetSaveDataSize() const { return 0x1B; };
}; // total size: 0x20

template <u16 Teams, u16 Rounds>
struct Cup : public BaseCup
{
    virtual BasicGameInfo* GetGameInfo(int team, int round) { return &mGameInfo[team][round]; };
    virtual TeamStats* GetTeamStats(int team) { return &mTeamStats[team]; };
    virtual u16 GetNumTeams() { return Teams; };
    virtual u16 GetNumRounds() { return Rounds; };
    virtual signed char* GetRoundResults(int round) { return &mRoundResults[round]; };
    virtual void* SerializeData(void* dst) const
    {
        memcpy(dst, &mUserSelectedTeam, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mUserSelectedSidekick, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mRoundNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mGameNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mHumanTeams, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mCupStarted, 1);
        dst = (u8*)dst + 1;
        memcpy(dst, &mCupSettings, sizeof(mCupSettings));
        dst = (u8*)dst + sizeof(mCupSettings);
        memcpy(dst, mGameInfo, sizeof(mGameInfo));
        dst = (u8*)dst + sizeof(mGameInfo);
        memcpy(dst, mTeamStats, sizeof(mTeamStats));
        dst = (u8*)dst + sizeof(mTeamStats);
        memcpy(dst, mRoundResults, sizeof(mRoundResults));
        dst = (u8*)dst + sizeof(mRoundResults);
        return dst;
    }
    virtual void* DeserializeData(void* src)
    {
        memcpy(&mUserSelectedTeam, src, 4);
        src = (u8*)src + 4;
        memcpy(&mUserSelectedSidekick, src, 4);
        src = (u8*)src + 4;
        memcpy(&mRoundNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mGameNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mHumanTeams, src, 2);
        src = (u8*)src + 2;
        memcpy(&mCupStarted, src, 1);
        src = (u8*)src + 1;
        memcpy(&mCupSettings, src, sizeof(mCupSettings));
        src = (u8*)src + sizeof(mCupSettings);
        memcpy(mGameInfo, src, sizeof(mGameInfo));
        src = (u8*)src + sizeof(mGameInfo);
        memcpy(mTeamStats, src, sizeof(mTeamStats));
        src = (u8*)src + sizeof(mTeamStats);
        memcpy(mRoundResults, src, sizeof(mRoundResults));
        src = (u8*)src + sizeof(mRoundResults);
        return src;
    }
    virtual int GetSaveDataSize() const { return 0x1B + sizeof(mGameInfo) + sizeof(mTeamStats) + sizeof(mRoundResults); };

    BasicGameInfo mGameInfo[Rounds][Teams / 2];
    TeamStats mTeamStats[Teams];
    signed char mRoundResults[Rounds];
};

template <u16 Teams>
struct Knockout : public BaseCup
{
    virtual BasicGameInfo* GetGameInfo(int team, int round);
    virtual TeamStats* GetTeamStats(int team) { return &mTeamStats[team]; };
    virtual u16 GetNumTeams() { return Teams; };
    virtual u16 GetNumRounds() { return 2; };
    virtual signed char* GetRoundResults(int round) { return &mRoundResults[round]; };
    virtual void* SerializeData(void* dst) const
    {
        memcpy(dst, &mUserSelectedTeam, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mUserSelectedSidekick, 4);
        dst = (u8*)dst + 4;
        memcpy(dst, &mRoundNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mGameNumber, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mHumanTeams, 2);
        dst = (u8*)dst + 2;
        memcpy(dst, &mCupStarted, 1);
        dst = (u8*)dst + 1;
        memcpy(dst, &mCupSettings, sizeof(mCupSettings));
        dst = (u8*)dst + sizeof(mCupSettings);
        memcpy(dst, mGameInfo, sizeof(mGameInfo));
        dst = (u8*)dst + sizeof(mGameInfo);
        memcpy(dst, mTeamStats, sizeof(mTeamStats));
        dst = (u8*)dst + sizeof(mTeamStats);
        memcpy(dst, mRoundResults, sizeof(mRoundResults));
        dst = (u8*)dst + sizeof(mRoundResults);
        return dst;
    }
    virtual void* DeserializeData(void* src)
    {
        memcpy(&mUserSelectedTeam, src, 4);
        src = (u8*)src + 4;
        memcpy(&mUserSelectedSidekick, src, 4);
        src = (u8*)src + 4;
        memcpy(&mRoundNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mGameNumber, src, 2);
        src = (u8*)src + 2;
        memcpy(&mHumanTeams, src, 2);
        src = (u8*)src + 2;
        memcpy(&mCupStarted, src, 1);
        src = (u8*)src + 1;
        memcpy(&mCupSettings, src, sizeof(mCupSettings));
        src = (u8*)src + sizeof(mCupSettings);
        memcpy(mGameInfo, src, sizeof(mGameInfo));
        src = (u8*)src + sizeof(mGameInfo);
        memcpy(mTeamStats, src, sizeof(mTeamStats));
        src = (u8*)src + sizeof(mTeamStats);
        memcpy(mRoundResults, src, sizeof(mRoundResults));
        src = (u8*)src + sizeof(mRoundResults);
        return src;
    }
    virtual int GetSaveDataSize() const { return 0x1B + sizeof(mGameInfo) + sizeof(mTeamStats) + sizeof(mRoundResults); };

    BasicGameInfo mGameInfo[Teams - 1];
    TeamStats mTeamStats[Teams];
    signed char mRoundResults[3];
}; // total size: 0x184

#endif // _CUP_H_
