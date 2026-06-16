#include "Game/OverlayHandlerGoal.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feNSNMessenger.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/Goalie.h"
#include "Game/Team.h"
#include "NL/nlBundleFile.h"
#include "NL/nlFormat.h"

template <typename To, typename From>
To LexicalCast(const From& f);

template <typename StringType, typename ValueType>
StringType Format(const StringType&, const ValueType&);

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
struct StatsTrackerOvertimeAccessor
{
    char _pad[0x4C1];
    unsigned char mIsOvertime;
};

class StatsTracker;

extern "C" double ceil(double);
extern "C" double floor(double);

void OverlayHandlerGoal_stub()
{
    typedef BasicString<unsigned short, Detail::TempStringAllocator> WideBasicString;
    typedef WideBasicString (*FmtFn)(const WideBasicString&, const unsigned short (&)[32], const WideBasicString&);

    volatile FmtFn fn = &Format<WideBasicString, unsigned short[32], WideBasicString>;
    (void)fn;
}

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
    typedef TLTextInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    volatile InlineHasher hSlideB, hSlideA;
    volatile InlineHasher hLayerB, hLayerA;
    volatile InlineHasher hNameB, hNameA;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Name");
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    TLTextInstance* pText;
    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;
        pText = findComp.byRef(
            presentation,
            (InlineHasher&)hSlideB,
            (InlineHasher&)hLayerB,
            (InlineHasher&)hNameB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    eTeamID team = (eTeamID)gameInfo->GetTeam((short)homeAway);
    nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);
    nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);

    float gameTime = g_pGame->GetGameTime();
    StatsTrackerOvertimeAccessor* stats = (StatsTrackerOvertimeAccessor*)nlSingleton<StatsTracker>::s_pInstance;
    if (!stats->mIsOvertime)
    {
        float fGameDuration = g_pGame->m_pGameTweaks->fGameDuration;
        if (gameTime > fGameDuration)
        {
            gameTime = fGameDuration;
        }
    }

    float remainingTime = g_pGame->m_pGameTweaks->fGameDuration - gameTime;
    unsigned long minutes = (unsigned long)(gameTime / 60.0f);

    float fSeconds = gameTime - (float)(minutes * 60);
    unsigned long seconds;

    if (remainingTime >= 30.0)
    {
        seconds = (unsigned long)(int)(float)ceil((double)fSeconds);
    }
    else
    {
        seconds = (unsigned long)(int)(float)floor((double)fSeconds);
    }

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

    int oldScore[2] = { 0, 0 };
    oldScore[0] = mCaptainGoals[0] + mSidekickGoals[0];
    oldScore[1] = mCaptainGoals[1] + mSidekickGoals[1];

    bool isSuperTeam = (nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)homeAway) == 8);
    if (isSuperTeam)
    {
        playerIndex = 0;
    }

    unsigned long teamNameID = GetLOCTeamName((eTeamID)team);
    pText->m_LocStrId = teamNameID;
    pText->m_OverloadFlags |= 8;

    if (seconds < 10)
    {
        {
            BasicString<char, Detail::TempStringAllocator> zeroStr("0");
            secondsString = zeroStr;
        }
        BasicString<char, Detail::TempStringAllocator> secStr(
            LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(seconds));
        secondsString = secondsString.Append(secStr);
    }
    else
    {
        secondsString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(seconds);
    }

    unsigned short minutesWideString[16];
    unsigned short secondsWideString[16];
    nlStrToWcs(minutesString.c_str(), minutesWideString, 32);
    nlStrToWcs(secondsString.c_str(), secondsWideString, 32);

    unsigned long clockKey = 0x04E76F8B;
    nlLocalization* loc = g_pLocalization;

    const unsigned short* clockLocString;
    if (loc->m_LookupTable == 0)
    {
        clockLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(clockKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            clockLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            clockLocString = MissingLocString;
        }
    }

    unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(clockLocString);
    formatted = Format(unformatted, minutesWideString, secondsWideString);

    memcpy(mClockBuffer, formatted.c_str(), 0x40);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Time");
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;
        pText = findComp.byRef(
            presentation,
            (InlineHasher&)hSlideB,
            (InlineHasher&)hLayerB,
            (InlineHasher&)hNameB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

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

    int scoreLeft = mCaptainGoals[0] + mSidekickGoals[0];
    int scoreRight = mCaptainGoals[1] + mSidekickGoals[1];

    if (mIsInOvertime)
    {
        unsigned long overtimeKey = 0xAD90B5E0;
        loc = g_pLocalization;

        const unsigned short* locString;
        if (loc->m_LookupTable == 0)
        {
            locString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(overtimeKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                locString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                locString = MissingLocString;
            }
        }

        unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
    }
    else
    {
        if (!mHasSniperCup && gameInfo->HasTrophy((eTrophyType)9) == 1)
        {
            unsigned long sniperKey = 0x25801546;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(sniperKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
            mHasSniperCup = true;
        }
        else if (isCaptainS2S == 1)
        {
            unsigned long s2sKey = 0x831AAC58;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(s2sKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
        }
        else if (oldScore[0] == 0 && oldScore[1] == 0)
        {
            unsigned long zeroKey = 0x80675849;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(zeroKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
        }
        else if (scoreLeft == scoreRight)
        {
            unsigned long tiedKey = 0x43AB49F3;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(tiedKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
        }
        else if ((oldScore[0] < oldScore[1] || scoreLeft < scoreRight) && !(oldScore[1] < oldScore[0] && scoreRight >= scoreLeft))
        {
            unsigned long trailingKey = 0x78446837;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(trailingKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);

            unsigned long teamLocID = GetLOCTeamName((eTeamID)team);
            const unsigned short* teamLocString;

            loc = g_pLocalization;
            if (loc->m_LookupTable == 0)
            {
                teamLocString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    teamLocString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    teamLocString = MissingLocString;
                }
            }

            BasicString<unsigned short, Detail::TempStringAllocator> teamNameStr(teamLocString);
            formatted = Format(unformatted, teamNameStr);
        }
        else if (playerIndex == 0 && mCaptainGoals[homeAway] == 3 && !isSuperTeam)
        {
            unsigned long hatTrickKey = 0xD8976F68;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(hatTrickKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);

            unsigned long teamLocID = GetLOCTeamName((eTeamID)team);
            const unsigned short* teamLocString;

            loc = g_pLocalization;
            if (loc->m_LookupTable == 0)
            {
                teamLocString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    teamLocString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    teamLocString = MissingLocString;
                }
            }

            BasicString<unsigned short, Detail::TempStringAllocator> teamNameStr(teamLocString);
            formatted = Format(unformatted, teamNameStr);
        }
        else if (playerIndex == 0)
        {
            unsigned long captainKey = 0x3DE2ABC1;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(captainKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);

            unsigned long teamLocID = GetLOCTeamName((eTeamID)team);
            const unsigned short* teamLocString;

            loc = g_pLocalization;
            if (loc->m_LookupTable == 0)
            {
                teamLocString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    teamLocString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    teamLocString = MissingLocString;
                }
            }

            BasicString<unsigned short, Detail::TempStringAllocator> teamNameStr(teamLocString);

            BasicString<char, Detail::TempStringAllocator> numGoalsString(
                LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mCaptainGoals[homeAway]));
            unsigned short goalsWideString[32];
            nlStrToWcs(numGoalsString.c_str(), goalsWideString, 32);

            formatted = Format(unformatted, goalsWideString, teamNameStr);
        }
        else
        {
            unsigned long sidekickKey = 0x091F7BA8;
            loc = g_pLocalization;

            const unsigned short* locString;
            if (loc->m_LookupTable == 0)
            {
                locString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(sidekickKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    locString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    locString = MissingLocString;
                }
            }

            unformatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);

            unsigned long teamLocID = GetLOCTeamName((eTeamID)team);
            const unsigned short* teamLocString;

            loc = g_pLocalization;
            if (loc->m_LookupTable == 0)
            {
                teamLocString = LocalizationTableNotFound;
            }
            else
            {
                nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
                if (entry)
                {
                    teamLocString = loc->m_FirstString + entry->StringOffset;
                }
                else
                {
                    teamLocString = MissingLocString;
                }
            }

            BasicString<unsigned short, Detail::TempStringAllocator> teamNameStr(teamLocString);

            BasicString<char, Detail::TempStringAllocator> numGoalsString(
                LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mSidekickGoals[homeAway]));
            unsigned short goalsWideString[32];
            nlStrToWcs(numGoalsString.c_str(), goalsWideString, 32);

            formatted = Format(unformatted, goalsWideString, teamNameStr);
        }
    }

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Description");
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;
        pText = findComp.byRef(
            presentation,
            (InlineHasher&)hSlideB,
            (InlineHasher&)hLayerB,
            (InlineHasher&)hNameB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    MakeTextBoxReallyWide(*pText);
    pText->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0x1590 | 0x80101600 | size: 0x418
 * TODO: 97.89% match - +0x0C hasher stack slot offset and localization pointer register swaps remain.
 */
void GoalOverlay::SetHighlightNumber(int highlightNumber)
{
    typedef TLTextInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hSlideB, hSlideA;
    volatile InlineHasher hLayerB, hLayerA;
    volatile InlineHasher hDescB, hDescA;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;

    SetWinnerTitle();

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Description");
    hDescA.m_Hash = hash;
    hDescB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    TLTextInstance* text = findComp.byRef(
        m_pFEPresentation,
        (InlineHasher&)hSlideB,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hDescB,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    MakeTextBoxReallyWide(*text);

    if (highlightNumber == 0)
    {
        text->SetStringId("HIGHLIGHTS1");
        return;
    }

    const unsigned short* locString;
    unsigned long key = 0xF3DDE99C;
    nlLocalization* loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        locString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            locString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            locString = MissingLocString;
        }
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(locString);

    int highlight = highlightNumber + 1;
    BasicString<char, Detail::TempStringAllocator> highlightString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(highlight));
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
    typedef TLTextInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    SetWinnerTitle();

    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;
    unsigned char isFinalGame = false;
    eTeamID winner = TEAM_INVALID;

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
        const unsigned short* formatLocString;
        unsigned long key = 0x736E7F17;
        nlLocalization* loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            formatLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                formatLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                formatLocString = MissingLocString;
            }
        }

        BasicString<unsigned short, Detail::TempStringAllocator> unformatted(formatLocString);
        int cup = nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();

        unsigned long winnerLocID = GetLOCTeamName(winner);
        const unsigned short* winnerLocString;

        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            winnerLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(winnerLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                winnerLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                winnerLocString = MissingLocString;
            }
        }

        unsigned long trophyLocID = GetLOCTrophyName((eTrophyType)cup);
        const unsigned short* trophyLocString;

        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            trophyLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(trophyLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                trophyLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                trophyLocString = MissingLocString;
            }
        }

        formatted = Format(unformatted, winnerLocString, trophyLocString);
    }
    else
    {
        int scoreLeft = g_pTeams[0]->m_nScore;
        int scoreRight = g_pTeams[1]->m_nScore;

        int winnerID;
        int loserID;

        if (scoreLeft > scoreRight)
        {
            winnerID = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);
        }
        else
        {
            winnerID = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);
        }

        if (scoreLeft < scoreRight)
        {
            loserID = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);
        }
        else
        {
            loserID = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);
        }

        const unsigned short* formatLocString;
        unsigned long key = 0x09B4BC7C;
        nlLocalization* loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            formatLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                formatLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                formatLocString = MissingLocString;
            }
        }

        BasicString<unsigned short, Detail::TempStringAllocator> unformatted(formatLocString);

        unsigned long winnerLocID = GetLOCCharacterName((eTeamID)winnerID, true, false);
        const unsigned short* winnerLocString;

        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            winnerLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(winnerLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                winnerLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                winnerLocString = MissingLocString;
            }
        }

        unsigned long loserLocID = GetLOCCharacterName((eTeamID)loserID, true, false);
        const unsigned short* loserLocString;

        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            loserLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(loserLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                loserLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                loserLocString = MissingLocString;
            }
        }

        formatted = Format(unformatted, winnerLocString, loserLocString);
    }

    volatile InlineHasher hSlideB, hSlideA;
    volatile InlineHasher hLayerB, hLayerA;
    volatile InlineHasher hDescB, hDescA;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Description");
    hDescA.m_Hash = hash;
    hDescB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    TLTextInstance* pText = findComp.byRef(
        m_pFEPresentation,
        (InlineHasher&)hSlideB,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hDescB,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    MakeTextBoxReallyWide(*pText);

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);
    mDescriptionBuffer[127] = 0;
    pText->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0x6E8 | 0x80100758 | size: 0x6BC
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

    BasicStringData<unsigned short>* data;
    const unsigned short* formatLocString;
    unsigned long key = 0x4543196B;

    if (g_pLocalization->m_LookupTable == 0)
    {
        formatLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization* loc = g_pLocalization;
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            formatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            formatLocString = MissingLocString;
        }
    }

    data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = formatLocString;
        while (*ptr++)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = 0;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *formatLocString;
            i++;
            formatLocString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(data);
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    signed short teamIndex = (signed short)(scoreRight >> 31);
    teamIndex += (unsigned int)scoreLeft >> 31;
    teamIndex += scoreRight >= scoreLeft;
    int winningTeam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(teamIndex);

    if (scoreLeft > scoreRight)
    {
        unsigned long teamNameStringID = GetLOCTeamName((eTeamID)winningTeam);
        const unsigned short* winnerLocString;

        if (g_pLocalization->m_LookupTable == 0)
        {
            winnerLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization* loc = g_pLocalization;
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamNameStringID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                winnerLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                winnerLocString = MissingLocString;
            }
        }

        formatted = Format(unformatted, winnerLocString, scoreLeftWideString, scoreRightWideString);
    }
    else
    {
        unsigned long teamNameStringID = GetLOCTeamName((eTeamID)winningTeam);
        const unsigned short* winnerLocString;

        if (g_pLocalization->m_LookupTable == 0)
        {
            winnerLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization* loc = g_pLocalization;
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(teamNameStringID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                winnerLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                winnerLocString = MissingLocString;
            }
        }

        formatted = Format(unformatted, winnerLocString, scoreRightWideString, scoreLeftWideString);
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
 */
void GoalOverlay::DoCupWinOverlay()
{
    typedef TLTextInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hSlideB, hSlideA;
    volatile InlineHasher hLayerB, hLayerA;
    volatile InlineHasher hNameB, hNameA;
    volatile InlineHasher hDescB, hDescA;
    volatile InlineHasher hTimeB, hTimeA;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;
    TLTextInstance* pText;
    GoalOverlay* self = this;

    eTeamID winners = (eTeamID)nlSingleton<GameInfoManager>::s_pInstance->GetUserSelectedCupTeam();

    const unsigned short* formatLocString;
    unsigned long key = 0xB49CF8B5;
    nlLocalization* loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        formatLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            formatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            formatLocString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = formatLocString;
        while (*ptr++)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = 0;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *formatLocString;
            i++;
            formatLocString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    unsigned long winnerLocID = GetLOCCharacterName(winners, true, false);
    const unsigned short* winnerLocString;

    loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        winnerLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(winnerLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            winnerLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            winnerLocString = MissingLocString;
        }
    }

    BasicString<unsigned short, Detail::TempStringAllocator> formatted(
        Format(BasicString<unsigned short, Detail::TempStringAllocator>(data), winnerLocString));

    memcpy(self->mScoresBuffer, formatted.c_str(), 0x100);

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Name");
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    pText = findComp.byRef(
        self->m_pFEPresentation,
        (InlineHasher&)hSlideB,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hNameB,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    pText->SetString(self->mScoresBuffer);

    eTrophyType cup = (eTrophyType)nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();

    key = 0x4E704897;
    loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        formatLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            formatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            formatLocString = MissingLocString;
        }
    }

    data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = formatLocString;
        while (*ptr++)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = 0;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *formatLocString;
            i++;
            formatLocString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    unsigned long trophyLocID = GetLOCTrophyName(cup);
    const unsigned short* trophyLocString;

    loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        trophyLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(trophyLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            trophyLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            trophyLocString = MissingLocString;
        }
    }

    formatted = Format(BasicString<unsigned short, Detail::TempStringAllocator>(data), trophyLocString);

    memcpy(self->mDescriptionBuffer, formatted.c_str(), 0x100);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Description");
    hDescA.m_Hash = hash;
    hDescB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    pText = findComp.byRef(
        self->m_pFEPresentation,
        (InlineHasher&)hSlideB,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hDescB,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    MakeTextBoxReallyWide(*pText);
    pText->SetString(self->mDescriptionBuffer);

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    hash = nlStringLowerHash("Time");
    hTimeA.m_Hash = hash;
    hTimeB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hSlideA.m_Hash = hash;
    hSlideB.m_Hash = hash;

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    pText = findComp.byRef(
        self->m_pFEPresentation,
        (InlineHasher&)hSlideB,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hTimeB,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

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
