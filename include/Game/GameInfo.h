#ifndef _GAMEINFO_H_
#define _GAMEINFO_H_

#include "types.h"
#include "dolphin/os.h"

#include "Game/FE/feInput.h"

#include "NL/nlSingleton.h"
#include "NL/nlConfig.h"

#include "Game/Team.h"
#include "Game/TrophyInfo.h"
#include "Game/GameTweaks.h"

#include "Game/DB/UserOptions.h"
#include "Game/DB/CustomTournament.h"
#include "Game/DB/StatsTracker.h"
#include "Game/DB/Cup.h"

struct Spoil
{
    Spoil()
        : mNumRecords(0)
        , mNumWins(0)
        , mNumLosses(0)
        , mNumCupWins(0)
        , mCurrentChamp(TEAM_INVALID)
        , mIsCPUChamp(false)
    {
    }

    /* 0x000 */ CupRecord mCupHistory[10];
    /* 0x208 */ unsigned char mNumRecords;
    /* 0x20A */ unsigned short mNumWins;
    /* 0x20C */ unsigned short mNumLosses;
    /* 0x20E */ unsigned short mNumCupWins;
    /* 0x210 */ eTeamID mCurrentChamp;
    /* 0x214 */ bool mIsCPUChamp;
}; // total size: 0x218

struct UserInfo
{
    UserInfo()
        : mSaveID(0)
        , mIsFlowerCupUnlocked(false)
        , mIsStarCupUnlocked(false)
        , mNumGamesPlayed(0)
        , mNumGoalsScored(0)
        , mNumSTSAttempts(0)
        , mNumPerfectPasses(0)
        , mNumHits(0)
    {
        memset(mTrophies, 0, sizeof(mTrophies));
        mBowserCupFinalRound.mTeamIndex[0] = (eTeamID)3;
        mBowserCupFinalRound.mTeamIndex[1] = (eTeamID)2;
        mBowserCupFinalRound.mSidekickIndex[0] = (eSidekickID)0;
        mBowserCupFinalRound.mSidekickIndex[1] = (eSidekickID)1;
        mBowserCupFinalRound.mFinalScore[1] = 0;
        mBowserCupFinalRound.mFinalScore[0] = 0;
        mBowserCupFinalRound.mPadSides[0] = -1;
        mBowserCupFinalRound.mPadSides[1] = -1;
        mBowserCupFinalRound.mPadSides[2] = -1;
        mBowserCupFinalRound.mPadSides[3] = -1;
        mBowserCupFinalRound.mStadiumIndex = (eStadiumID)0;
    }

    /* 0x0000 */ unsigned long mSaveID;
    /* 0x0004 */ AudioSettings mAudioOptions;
    /* 0x0024 */ VisualSettings mVisualOptions;
    /* 0x0030 */ GameplaySettings mGameplayOptions;
    /* 0x003C */ PowerupSettings mPowerupOptions;
    /* 0x0044 */ CheatSettings mCheatOptions;
    /* 0x004C */ unsigned char mTrophies[2];
    /* 0x0050 */ Spoil mSpoils[8];
    /* 0x1110 */ bool mIsFlowerCupUnlocked;
    /* 0x1111 */ bool mIsStarCupUnlocked;
    /* 0x1112 */ unsigned short mNumGamesPlayed;
    /* 0x1114 */ unsigned short mNumGoalsScored;
    /* 0x1116 */ unsigned short mNumSTSAttempts;
    /* 0x1118 */ unsigned short mNumPerfectPasses;
    /* 0x111A */ unsigned short mNumHits;
    /* 0x111C */ BasicGameInfo mBowserCupFinalRound;
}; // total size: 0x113C

class GameInfoManager : public nlSingleton<GameInfoManager>
{
public:
    enum eGameModes
    {
        GM_INVALID = -1,
        GM_FRIENDLY = 0,
        GM_MUSHROOM_CUP = 1,
        GM_FLOWER_CUP = 2,
        GM_STAR_CUP = 3,
        GM_BOWSER_CUP = 4,
        GM_SUPER_MUSHROOM_CUP = 5,
        GM_SUPER_FLOWER_CUP = 6,
        GM_SUPER_STAR_CUP = 7,
        GM_SUPER_BOWSER_CUP = 8,
        GM_TOURNAMENT = 9,
        GM_DEMO = 10,
        GM_NUM_MODES = 11,
    };

public:
    GameInfoManager();
    virtual ~GameInfoManager();

    eTeamID GetTeam(short homeaway) const;
    void SetTeam(short homeaway, eTeamID teamid);
    eSidekickID GetSidekick(short homeaway) const;
    void SetSidekick(short homeaway, eSidekickID sidekickid);
    u16 GetNumPlayingTeams() const;
    u16 GetNumRounds() const;
    TeamStats GetTeamStatsByIndex(unsigned short index);
    TeamStats* pGetTeamStatsByIndex(unsigned short index);
    void SetPreviousTeamStats();
    eStadiumID GetStadium() const;
    void GetMatchupInfo(short, unsigned short) const;
    void SetUserSelectedCupTeam(eTeamID);
    void SetUserSelectedCupSidekick(eSidekickID);
    eUserGameResult GetResultsOfLastUserGame() const;
    void SetResultsOfLastUserGame(eUserGameResult);
    s16 GetCurrentRoundNumber() const;
    s16 GetNextRoundNumber(short) const;
    s16 GetPreviousRoundNumber(short) const;
    signed short GetFirstRoundNumber() const;
    u16 GetNumGamesPerRound(int) const;
    eTeamID GetUserSelectedCupTeam() const;
    void SetStadium(eStadiumID);
    eStadiumID PickStadium(bool, eStadiumID) const;
    short GetPlayingSide(unsigned short) const;
    void SetPlayingSide(unsigned short, short);
    u16 GetNumPlayers() const;
    void ResetPlayingSides();
    void SetupRoundRobinSchedule(eTeamID*, eSidekickID*);
    unsigned char SetupBowserKnockout();
    void SetupTournamentKnockout(eTeamID*, eSidekickID*);
    unsigned char SetupKnockoutRound(short);
    unsigned char DetermineNextMatchups(int);
    void IncreaseRoundNumber();
    void IncreaseGameNumber(bool);
    int GetNumHumanTeams();
    BaseCup* GetCup(GameInfoManager::eGameModes);
    bool IsUserQualified(GameInfoManager::eGameModes) const;
    void SetMode(GameInfoManager::eGameModes);
    unsigned long GetMemoryCardDataSize() const;
    void GetMemoryCardData(void*);
    void SetMemoryCardData(void*);
    u8 CheckSaveIDChanged(void*);
    bool HasTrophy(eTrophyType) const;
    eMilestoneColour GetMilestoneLevel(eTrophyType) const;
    bool IsInRegularCupMode() const;
    bool IsInSuperCupMode() const;
    bool IsInCupMode() const;
    bool IsInCupOrTournamentMode() const;
    bool IsInDemoMode() const;
    bool IsInFriendlyMode() const;
    bool IsInTournamentMode() const;
    AudioSettings& GetAudioOptions();
    const GameplaySettings& GetGameplayOptions() const;
    const PowerupSettings& GetPowerupOptions() const;
    void OnPreGameState();
    void OnPostGameState();
    void ApplyDifficultySettings();
    eTrophyType GetTrophyTypeByCurrentMode() const;
    bool IsPossibleCupMatch() const;
    void OnPreCupGameState();
    void OnPostCupGameState();
    void DetermineNextCupScreen();
    signed char DetermineUserPlacement(Spoil*);
    void TimeStampCupEnd();
    eTeamID FindWinningTeam();
    bool IsKongaUnlocked() const;
    bool IsYoshiUnlocked() const;
    bool IsForbiddenUnlocked() const;
    bool IsSuperStadiumUnlocked() const;
    bool IsSuperTeamUnlocked() const;
    bool IsSuperCupModeUnlocked() const;
    bool IsLegendSkillUnlocked() const;
    bool IsAllSTSCheatUnlocked() const;
    bool IsTiltCheatUnlocked() const;
    bool IsGlassJawGoalieUnlocked() const;
    bool IsUnlimtedPowerupsUnlocked() const;
    bool IsCustomShellsUnlocked() const;
    bool IsCustomEnhanceUnlocked() const;
    bool IsCustomGiantUnlocked() const;
    bool IsCustomExplosiveUnlocked() const;
    bool IsCustomFreezingUnlocked() const;
    bool HasHumanGameBeenPlayed() const;
    void SetRoundResult(bool, int);
    bool IsStunnedGoaliesOn() const;
    bool IsInfinitePowerupsOn() const;
    bool IsTiltingFieldOn() const;
    bool IsPerfectStrikesOn() const;
    bool IsBowserAttackEnabled() const;
    GameplaySettings::eSkillLevel GetSkillLevel();
    eDifficultyID GetSkillLevelAsDifficultyID();
    CustomPowerups GetCustomPowerups() const;

    /* 0x0004 */ GameplaySettings mCurGameGameplayOptions;
    /* 0x0010 */ BasicGameInfo* mGameInfo[11];
    /* 0x003C */ eUserGameResult mUserLastResults[11];
    /* 0x0068 */ eUserGameResult mCupMatchRequirement;
    /* 0x006C */ bool mDoingKnockout;
    /* 0x006D */ bool mDidRoundJustEnd;
    /* 0x0070 */ eDifficultyID mCurrentDifficulty[2];
    /* 0x0078 */ Cup<4, 3> mMushroomCupSeries;        // size 0x1E4
    /* 0x025C */ Cup<6, 5> mFlowerCupSeries;          // size 0x388
    /* 0x05E4 */ Cup<8, 7> mStarCupSeries;            // size 0x5A8
    /* 0x0B8C */ Cup<8, 7> mBowserCupSeries;          // size 0x5A8
    /* 0x1134 */ Knockout<4> mBowserCupKnockout;      // size 0x184
    /* 0x12B8 */ Cup<4, 6> mSuperMushroomCupSeries;   // size 0x2A8
    /* 0x1560 */ Cup<6, 10> mSuperFlowerCupSeries;    // size 0x56C
    /* 0x1ACC */ Cup<8, 14> mSuperStarCupSeries;      // size 0x930
    /* 0x23FC */ Cup<8, 14> mSuperBowserCupSeries;    // size 0x930
    /* 0x2D2C */ Knockout<4> mSuperBowserCupKnockout; // size 0x184
    /* 0x2EB0 */ int mCheckpointTeamPoints[9];
    /* 0x2ED4 */ UserInfo mUserInfo;
    /* 0x4010 */ CustomTournament mCustomTournamentInfo;
    /* 0x4954 */ eGameModes mCurrentMode;
    /* 0x4958 */ bool mDemoEnabled;
    /* 0x4959 */ bool mIsInStrikers101Mode;
    /* 0x495A */ bool mGoToChooseCaptains;
    /* 0x495C */ eFEINPUT_PAD mMainUserPadNumber;
    /* 0x4960 */ BaseCup* mCurrentCup;
    /* 0x4964 */ BaseCup* mPreviousCup;
    /* 0x4968 */ PlayerStats mUserStats[4];
    /* 0x4A38 */ TeamStats mPreviousTeamStats[8];
    /* 0x4C38 */ bool mUseCurGameSettings;
    /* 0x4C39 */ bool mDisplayTrophy[6];
    /* 0x4C40 */ eMilestoneColour mTrophyColourState[5];
    /* 0x4C54 */ u32 mPreGameUnlockedState;
    /* 0x4C58 */ u32 mUnlockedTriggers;
    /* 0x4C5C */ eStadiumID mLastHumanStadium;
    /* 0x4C60 */ AudioSettings mCurGameAudioSettings;
}; // total size: 0x4C80

#endif // _GAMEINFO_H_
