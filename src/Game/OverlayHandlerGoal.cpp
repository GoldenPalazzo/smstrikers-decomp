#include "Game/OverlayHandlerGoal.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/Goalie.h"
#include "Game/Team.h"
#include "NL/nlBundleFile.h"
#include "NL/nlFormat.h"

template <typename T, typename Key>
T* nlBSearch(const Key&, T*, int);

struct LOCHeader
{
    char Thumbprint[4];
    unsigned long Version;
    unsigned long Language;
    unsigned long StringCount;
    unsigned long Flags;
};

class nlLocalization
{
public:
    struct StringLookup
    {
        unsigned long hash;
        unsigned long StringOffset;
    };

    LOCHeader* m_pFile;
    StringLookup* m_LookupTable;
    unsigned short* m_FirstString;
    int m_CurrentLanguage;
};

template <>
nlLocalization::StringLookup* nlBSearch<nlLocalization::StringLookup, unsigned long>(const unsigned long&, nlLocalization::StringLookup*, int);

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];
extern cTeam* g_pTeams[];

void MakeTextBoxReallyWide(TLTextInstance&);
unsigned long GetLOCCharacterName(eTeamID, bool, bool);
unsigned long GetLOCTrophyName(eTrophyType);
unsigned long GetLOCTeamName(eTeamID);

static inline const unsigned short* LookupLocHash(unsigned long key)
{
    nlLocalization* loc = g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }

    nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
    if (entry)
    {
        return loc->m_FirstString + entry->StringOffset;
    }

    return MissingLocString;
}
extern "C" double ceil(double);
extern "C" double floor(double);

/**
 * Offset/Address/Size: 0x106C | 0x80104868 | size: 0xCF0
 */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
//{
// }

/**
 * Offset/Address/Size: 0xF40 | 0x8010473C | size: 0x12C
 */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32], BasicString<unsigned short, Detail::TempStringAllocator>>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[32], const BasicString<unsigned short, Detail::TempStringAllocator>&)
//{
// }

/**
 * Offset/Address/Size: 0xE1C | 0x80104618 | size: 0x124
 */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16], unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[16], const unsigned short(&)[16])
//{
// }

/**
 * Offset/Address/Size: 0x12C | 0x80103928 | size: 0xCF0
 */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x801037FC | size: 0x12C
 */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, unsigned short[32], unsigned short[32]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short(&)[32], const unsigned short(&)[32])
//{
// }

/**
 * Offset/Address/Size: 0xBC | 0x8010337C | size: 0x1E4
 */
// void BasicString<char, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x801032C0 | size: 0xBC
 */
// void BasicString<char, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&) const
//{
// }

/**
 * Offset/Address/Size: 0x3150 | 0x801031C0 | size: 0x100
 */
GoalOverlay::GoalOverlay()
    : BaseOverlayHandler(0x110, POSITION_BOTTOM)
{
    mEventHandler = NULL;
    mIsCreated = false;
    mIsInOvertime = false;

    mEventHandler = g_pEventManager->AddEventHandler(eventHandler, this, 1);

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInFriendlyMode() || nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode())
    {
        mHasSniperCup = true;
    }
    else
    {
        mHasSniperCup = nlSingleton<GameInfoManager>::s_pInstance->HasTrophy((eTrophyType)9);
    }

    mCaptainGoals[0] = 0;
    mCaptainGoals[1] = 0;
    mSidekickGoals[0] = 0;
    mSidekickGoals[1] = 0;
}

/**
 * Offset/Address/Size: 0x30B8 | 0x80103128 | size: 0x98
 */
GoalOverlay::~GoalOverlay()
{
    if (mEventHandler != nullptr)
    {
        g_pEventManager->RemoveEventHandler(mEventHandler);
        mEventHandler = nullptr;
    }
}

/**
 * Offset/Address/Size: 0x30AC | 0x8010311C | size: 0xC
 */
void GoalOverlay::SceneCreated()
{
    mIsCreated = true;
}

/**
 * Offset/Address/Size: 0x305C | 0x801030CC | size: 0x50
 */
void GoalOverlay::Update(float dt)
{
    BaseSceneHandler::Update(dt);

    if (!mIsInOvertime)
    {
        if (g_pGame->m_eGameState == GS_OVERTIME)
        {
            mIsInOvertime = true;
        }
    }
}

/**
 * Offset/Address/Size: 0x2F44 | 0x80102FB4 | size: 0x118
 */
void GoalOverlay::eventHandler(Event* event, void* param)
{
    GoalOverlay* pGoal = (GoalOverlay*)param;

    if (event->m_uEventID == 5)
    {
        GoalScoredData* data;
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

        bool isCaptainS2S = (data->uGoalType == 6);
        int playerIndex;

        if (data->uGoalType == 5)
        {
            playerIndex = data->pLastTouch[data->uTeamIndex]->m_ID;
        }
        else
        {
            playerIndex = data->pScorer->m_ID;
        }

        pGoal->UpdateGoalInfo((int)data->uTeamIndex, playerIndex, isCaptainS2S, (int)data->uNumGoalsScored);
    }
    else if (event->m_uEventID == 3)
    {
        pGoal->mCaptainGoals[0] = 0;
        pGoal->mCaptainGoals[1] = 0;
        pGoal->mSidekickGoals[0] = 0;
        pGoal->mSidekickGoals[1] = 0;
        pGoal->mIsInOvertime = false;
    }
}

/**
 * Offset/Address/Size: 0x19A8 | 0x80101A18 | size: 0x159C
 */
void GoalOverlay::UpdateGoalInfo(int homeAway, int playerIndex, bool isCaptainS2S, int numGoals)
{
    TLTextInstance* pText;
    GameInfoManager* gameInfo;
    bool isSuperTeam;
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Name")));

    eTeamID team = (eTeamID)(gameInfo = nlSingleton<GameInfoManager>::s_pInstance)->GetTeam((short)homeAway);
    nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);
    nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);

    float time = g_pGame->GetGameTime();
    StatsTracker* stats = nlSingleton<StatsTracker>::s_pInstance;
    if (!stats->mIsOvertime)
    {
        float fGameDuration = g_pGame->m_pGameTweaks->fGameDuration;
        if (time > fGameDuration)
        {
            time = fGameDuration;
        }
    }

    float remainingTime = g_pGame->m_pGameTweaks->fGameDuration - time;
    unsigned long minutes = (unsigned long)(time / 60.0f);

    float fSeconds = time - (float)(minutes * 60);
    int wholeSeconds;
    if (remainingTime >= 30.0)
    {
        float roundedSeconds = (float)ceil((double)fSeconds);
        wholeSeconds = (int)roundedSeconds;
    }
    else
    {
        float roundedSeconds = (float)floor((double)fSeconds);
        wholeSeconds = (int)roundedSeconds;
    }
    unsigned long seconds = (unsigned long)wholeSeconds;

    if (seconds == 60)
    {
        seconds = 0;
        minutes = minutes + 1;
    }

    BasicString<char, Detail::TempStringAllocator> minutesString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(minutes));

    BasicString<char, Detail::TempStringAllocator> secondsString;
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;
    BasicString<unsigned short, Detail::TempStringAllocator> unformatted;

    int oldScore[2] = {
        mCaptainGoals[0] + mSidekickGoals[0],
        mCaptainGoals[1] + mSidekickGoals[1]
    };

    isSuperTeam = (nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)homeAway) == 8);
    if (isSuperTeam)
    {
        playerIndex = 0;
    }

    unsigned long teamNameID = GetLOCTeamName((eTeamID)team);
    pText->m_LocStrId = teamNameID;
    pText->m_OverloadFlags |= 8;

    if (seconds < 10)
    {
        secondsString = BasicString<char, Detail::TempStringAllocator>("0");
        secondsString = secondsString.Append(
            LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(seconds));
    }
    else
    {
        secondsString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(seconds);
    }

    unsigned short minutesWideString[16];
    unsigned short secondsWideString[16];
    nlStrToWcs(minutesString.c_str(), minutesWideString, 32);
    nlStrToWcs(secondsString.c_str(), secondsWideString, 32);

    unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x04E76F8B));
    formatted = Format(unformatted, minutesWideString, secondsWideString);

    memcpy(mClockBuffer, formatted.c_str(), 0x40);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Time")));

    pText->SetString(mClockBuffer);
    pText->m_bVisible = true;

    if (playerIndex > 0)
    {
        mSidekickGoals[homeAway] += numGoals;
    }
    else
    {
        mCaptainGoals[homeAway] += numGoals;
    }

    int scoreLeft = mCaptainGoals[0] + mSidekickGoals[0],
        scoreRight = mCaptainGoals[1] + mSidekickGoals[1];

    if (mIsInOvertime)
    {
        formatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0xAD90B5E0));
    }
    else
    {
        if (!mHasSniperCup && gameInfo->HasTrophy((eTrophyType)9) == 1)
        {
            formatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x25801546));
            mHasSniperCup = true;
        }
        else if (isCaptainS2S == 1)
        {
            formatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x831AAC58));
        }
        else if (oldScore[0] == 0 && oldScore[1] == 0)
        {
            formatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x80675849));
        }
        else if (scoreLeft == scoreRight)
        {
            formatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x43AB49F3));
        }
        else if ((oldScore[0] >= oldScore[1] && scoreLeft < scoreRight) || (oldScore[1] >= oldScore[0] && scoreRight < scoreLeft))
        {
            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x78446837));

            unsigned long captainKey = GetLOCTeamName(team);
            const unsigned short* captainString = LookupLocHash(captainKey);
            BasicString<unsigned short, Detail::TempStringAllocator> captain(captainString);
            formatted = Format(unformatted, captain);
        }
        else if (playerIndex == 0 && mCaptainGoals[homeAway] == 3 && !isSuperTeam)
        {
            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0xD8976F68));

            BasicString<unsigned short, Detail::TempStringAllocator> captain(
                LookupLocHash(GetLOCTeamName((eTeamID)team)));
            formatted = Format(unformatted, captain);
        }
        else if (playerIndex == 0)
        {
            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x3DE2ABC1));

            BasicString<unsigned short, Detail::TempStringAllocator> captain(
                LookupLocHash(GetLOCTeamName((eTeamID)team)));

            BasicString<char, Detail::TempStringAllocator> numGoalsString(
                LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mCaptainGoals[homeAway]));
            unsigned short goalsWideString[32];
            nlStrToWcs(numGoalsString.c_str(), goalsWideString, 32);

            formatted = Format(unformatted, goalsWideString, captain);
        }
        else
        {
            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x091F7BA8));

            BasicString<unsigned short, Detail::TempStringAllocator> captain(
                LookupLocHash(GetLOCTeamName((eTeamID)team)));

            BasicString<char, Detail::TempStringAllocator> numGoalsString(
                LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mSidekickGoals[homeAway]));
            unsigned short goalsWideString[32];
            nlStrToWcs(numGoalsString.c_str(), goalsWideString, 32);

            formatted = Format(unformatted, goalsWideString, captain);
        }
    }

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Description")));

    MakeTextBoxReallyWide(*pText);
    pText->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0x1590 | 0x80101600 | size: 0x418
 * TODO: 99.56% match - loc-string/data pointer saved-register swaps remain.
 */
void GoalOverlay::SetHighlightNumber(int highlightNumber)
{
    SetWinnerTitle();

    TLTextInstance* text = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Description")));

    MakeTextBoxReallyWide(*text);

    if (highlightNumber == 0)
    {
        text->SetStringId("HIGHLIGHTS1");
        return;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(LookupLocHash(0xF3DDE99C));

    BasicString<char, Detail::TempStringAllocator> highlightString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(highlightNumber + 1));
    unsigned short highlightWideString[16];
    nlStrToWcs(highlightString.c_str(), highlightWideString, 16);

    BasicString<unsigned short, Detail::TempStringAllocator> formatted(
        Format(unformatted, highlightWideString));

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);
    text->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0xDA4 | 0x80100E14 | size: 0x7EC
 */
void GoalOverlay::DoMatchEndOverlay()
{
    SetWinnerTitle();

    GameInfoManager* const gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;
    eTeamID winner = TEAM_INVALID;
    unsigned char isFinalGame = false;
    int scoreLeft;
    int scoreRight;

    if (gameInfo->IsInCupMode())
    {
        int round = gameInfo->GetCurrentRoundNumber();
        GameInfoManager::eGameModes mode = gameInfo->mCurrentMode;

        if (mode == GameInfoManager::GM_BOWSER_CUP && ((round == -2 && gameInfo->IsSuperTeamUnlocked()) || round == -1))
        {
            isFinalGame = true;
            winner = gameInfo->FindWinningTeam();
        }
        else if (mode == GameInfoManager::GM_BOWSER_CUP && round == -2 && !gameInfo->IsSuperTeamUnlocked())
        {
            winner = gameInfo->FindWinningTeam();
            if (winner != gameInfo->GetUserSelectedCupTeam())
            {
                isFinalGame = true;
            }
        }
        else if (mode == GameInfoManager::GM_SUPER_BOWSER_CUP && round == -2)
        {
            isFinalGame = true;
            winner = gameInfo->FindWinningTeam();
        }
        else if (mode != GameInfoManager::GM_BOWSER_CUP && mode != GameInfoManager::GM_SUPER_BOWSER_CUP)
        {
            if (round == (u16)gameInfo->GetNumRounds() - 1)
            {
                isFinalGame = true;
                winner = gameInfo->FindWinningTeam();
            }
        }
    }

    if (isFinalGame && (winner == gameInfo->GetTeam(0) || winner == gameInfo->GetTeam(1)))
    {
        BasicString<unsigned short, Detail::TempStringAllocator> unformatted(LookupLocHash(0x736E7F17));
        eTrophyType cup = nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();

        formatted = Format(unformatted, LookupLocHash(GetLOCTeamName(winner)), LookupLocHash(GetLOCTrophyName(cup)));
    }
    else
    {
        scoreLeft = g_pTeams[0]->m_nScore;
        scoreRight = g_pTeams[1]->m_nScore;

        eTeamID winnerID = scoreLeft > scoreRight
            ? nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0)
            : nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);
        eTeamID loserID = scoreLeft < scoreRight
            ? nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0)
            : nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);

        BasicString<unsigned short, Detail::TempStringAllocator> unformatted(LookupLocHash(0x09B4BC7C));

        formatted = Format(unformatted, LookupLocHash(GetLOCCharacterName(winnerID, true, false)), LookupLocHash(GetLOCCharacterName(loserID, true, false)));
    }

    TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Description")));

    MakeTextBoxReallyWide(*pText);

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);
    mDescriptionBuffer[127] = 0;
    pText->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0x6E8 | 0x80100758 | size: 0x6BC
 * TODO: 99.54% match - remaining saved-register swaps between object pointer, localization string pointer, and wide string data.
 */
void GoalOverlay::SetWinnerTitle()
{
    int scoreLeft = g_pTeams[0]->m_nScore;
    int scoreRight = g_pTeams[1]->m_nScore;

    BasicString<char, Detail::TempStringAllocator> scoreLeftString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(scoreLeft));
    BasicString<char, Detail::TempStringAllocator> scoreRightString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(scoreRight));

    unsigned short scoreLeftWideString[32];
    unsigned short scoreRightWideString[32];

    nlStrToWcs(scoreLeftString.c_str(), scoreLeftWideString, 32);
    nlStrToWcs(scoreRightString.c_str(), scoreRightWideString, 32);

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(LookupLocHash(0x4543196B));
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    eTeamID winningTeam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((scoreLeft <= scoreRight) ? 1 : 0);

    if (scoreLeft > scoreRight)
    {
        formatted = Format(unformatted, LookupLocHash(GetLOCTeamName(winningTeam)), scoreLeftWideString, scoreRightWideString);
    }
    else
    {
        formatted = Format(unformatted, LookupLocHash(GetLOCTeamName(winningTeam)), scoreRightWideString, scoreLeftWideString);
    }

    memcpy(mScoresBuffer, formatted.c_str(), 0x100);

    TLTextInstance* pText;

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Name")));

    pText->SetString(mScoresBuffer);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Time")));

    pText->m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x20 | 0x80100090 | size: 0x6C8
 * TODO: 99.47% match - remaining r29/r30 register swap in temporary wide string construction.
 */
void GoalOverlay::DoCupWinOverlay()
{
    eTeamID winners = (eTeamID)nlSingleton<GameInfoManager>::s_pInstance->GetUserSelectedCupTeam();

    BasicString<unsigned short, Detail::TempStringAllocator> formatted(
        Format(BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0xB49CF8B5)),
            LookupLocHash(GetLOCCharacterName(winners, true, false))));

    memcpy(mScoresBuffer, formatted.c_str(), 0x100);

    TLTextInstance* pText;

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Name")));

    pText->SetString(mScoresBuffer);

    eTrophyType cup = (eTrophyType)nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();

    formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(LookupLocHash(0x4E704897)),
        LookupLocHash(GetLOCTrophyName(cup)));

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Description")));

    MakeTextBoxReallyWide(*pText);
    pText->SetString(mDescriptionBuffer);

    pText = FEFinder<TLTextInstance, 3>::Find<FEPresentation>(
        m_pFEPresentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Time")));

    pText->m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x0 | 0x80100070 | size: 0x20
 */
void GoalOverlay::Restart()
{
    if (mIsCreated)
    {
        m_pFEPresentation->m_fadeDuration = m_pFEPresentation->m_currentSlide->m_start;
    }
}
