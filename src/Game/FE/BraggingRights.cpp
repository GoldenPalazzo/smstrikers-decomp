#include "Game/FE/BraggingRights.h"
#include "Game/DB/StatsTracker.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feMusic.h"
#include "Game/FE/feScrollText.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "Game/SH/SHMainMenu.h"
#include "Game/SH/SHSaveLoad.h"
#include "NL/gl/glStruct.h"
#include "NL/nlColour.h"
#include "NL/nlMath.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

static char* TEXT_NAMES[] = { "PLAYER", "PLAYER2", "PLAYER3", "PLAYER4", "PLAYER5" };
extern u32 CONTROLLER_TEXT[4];
extern u8 PAD_COLOURS[4][3];

namespace SingleHighlite
{
extern const char* SLIDE_IN;
extern const char* SLIDE_OUT;
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
} // namespace SingleHighlite

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
        operator unsigned long() const { return hash; }
    };

    LOCHeader* m_pFile;
    StringLookup* m_LookupTable;
    unsigned short* m_FirstString;
    int m_CurrentLanguage;
};

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];
extern nlLocalization::StringLookup* nlBSearch(const unsigned long&, nlLocalization::StringLookup*, int);

template <typename StringType, typename T1, typename T2>
StringType Format(const StringType& format, const T1& value1, const T2& value2);

static const unsigned short* LookupLocHash(unsigned long hash)
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

// /**
//  * Offset/Address/Size: 0xE10 | 0x800D6924 | size: 0x124
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32], unsigned short[32]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[32], const unsigned short(&)[32])
// {
// }

// /**
//  * Offset/Address/Size: 0x120 | 0x800D5C34 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D5B14 | size: 0x120
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, unsigned short[16]>(const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short* const&, const unsigned short(&)[16])
// {
// }

// /**
//  * Offset/Address/Size: 0x570 | 0x800D5ADC | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x538 | 0x800D5AA4 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x3DC | 0x800D5948 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x358 | 0x800D58C4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800D5840 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x29C | 0x800D5808 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x140 | 0x800D56AC | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800D5628 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800D55A4 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D556C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x34C8 | 0x800D54C4 | size: 0xA8
 */
BraggingRightsOverlay::BraggingRightsOverlay()
    : mIsTournamentScene(0)
{
}

/**
 * Offset/Address/Size: 0x3368 | 0x800D5364 | size: 0x160
 */
BraggingRightsOverlay::~BraggingRightsOverlay()
{
    if (mTicker != NULL)
    {
        delete mTicker;
    }
    nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode();
}

/**
 * Offset/Address/Size: 0x2E3C | 0x800D4E38 | size: 0x52C
 * TODO: 85.15% scratch match (87.61% full build). Added MenuItem::ApplyAction
 *       for correct callback dispatch pattern. Remaining diff: stmw r25 vs r26
 *       register shift from -inline deferred vs -inline auto context difference.
 */
void BraggingRightsOverlay::SceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    TLComponentInstance* pTickerComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Component")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    TLTextInstance* scrollText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pTickerComponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Group")),
        InlineHasher(nlStringLowerHash("TickerText")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    const gl_ScreenInfo* screenInfo = glGetScreenInfo();

    mTicker = new (nlMalloc(sizeof(FEScrollText), 8, false))
        FEScrollText(scrollText, 0, screenInfo->ScreenWidth + 50);

    char menuname[64];

    for (int i = 0; i < 5; i++)
    {
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        instance->SetActiveSlide((i == 0) ? SingleHighlite::SLIDE_IN : SingleHighlite::SLIDE_OUT);

        MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
        menuItem->mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<TLComponentInstance*> openFunc;
            openFunc.mTag = FREE_FUNCTION;
            openFunc.mFreeFunction = SingleHighlite::OpenItem;
            menuItem->mCallbacks[1] = openFunc;
        }
        {
            Function<TLComponentInstance*> closeFunc;
            closeFunc.mTag = FREE_FUNCTION;
            closeFunc.mFreeFunction = SingleHighlite::CloseItem;
            menuItem->mCallbacks[2] = closeFunc;
        }

        menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
    }

    mMenuItems.mFlags = 1;

    mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode())
    {
        mIsTournamentScene = 1;
        TournamentSceneCreated();
        mButtons.SetState(ButtonComponent::BS_A_AND_B);
    }
    else
    {
        IngameSceneCreated();
        mButtons.SetState(ButtonComponent::BS_A_ONLY);
    }

    ChangeTicker(0);
}

/**
 * Offset/Address/Size: 0x29A8 | 0x800D49A4 | size: 0x494
 * TODO: 88.11% match - stack frame 0x200 vs 0x1E0 and register shift by 1
 *       due to explicit PlayerStats temp needed for word-load copy pattern
 *       (-inline deferred vs -inline auto difference)
 */
void BraggingRightsOverlay::IngameSceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    StatsTracker* tracker = nlSingleton<StatsTracker>::s_pInstance;
    PlayerStats userStats[4];
    for (int i = 0; i < 4; i++)
    {
        PlayerStats temp = tracker->mCumulativeUserStats[i];
        userStats[i] = temp;
    }

    int highestTieBreaker[5];

    for (int award = 0; award < 5; award++)
    {
        mHighestStats[award] = -1;
        highestTieBreaker[award] = -1;

        for (int user = 0; user < 4; user++)
        {
            short side = nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((unsigned short)user);
            if (side == -1)
                continue;

            int mainStat;
            int tieBreaker;

            switch (award)
            {
            case 0:
                mainStat = userStats[user].mNumShotsOnGoal;
                if (mainStat == 0)
                {
                    tieBreaker = 0;
                }
                else
                {
                    tieBreaker = (int)(100.0f * (float)userStats[user].mNumGoalsFor / (float)mainStat);
                }
                break;
            case 1:
                mainStat = userStats[user].mNumHitsMade;
                tieBreaker = userStats[user].mNumFouls;
                break;
            case 2:
            {
                unsigned short steals = userStats[user].mNumSteals;
                tieBreaker = steals;
                mainStat = userStats[user].mNumPassesIntercepted + steals;
                break;
            }
            case 3:
                mainStat = userStats[user].mNumPowerupsUsed;
                tieBreaker = userStats[user].mNumPowerupsHit;
                break;
            case 4:
                mainStat = userStats[user].mNumButtonPresses;
                tieBreaker = nlRandom(100, &nlDefaultSeed);
                break;
            }

            if (mainStat > mHighestStats[award]
                || (mainStat == mHighestStats[award] && tieBreaker > highestTieBreaker[award]))
            {
                mHighestStats[award] = mainStat;
                highestTieBreaker[award] = tieBreaker;
                mAwardWinners[award] = user;
            }
        }

        TLTextInstance* pText = FEFinder<TLTextInstance, 3>::Find(
            presentation, InlineHasher(nlStringLowerHash("Slide1")), InlineHasher(nlStringLowerHash("Layer")), InlineHasher(nlStringLowerHash(TEXT_NAMES[award])), InlineHasher(0), InlineHasher(0), InlineHasher(0));

        if (mHighestStats[award] == 0)
        {
            pText->m_LocStrId = 0x0316C79C;
            pText->m_OverloadFlags |= 0x8;
        }
        else
        {
            pText->m_LocStrId = CONTROLLER_TEXT[mAwardWinners[award]];
            pText->m_OverloadFlags |= 0x8;

            nlColour colour;
            colour.c[0] = PAD_COLOURS[mAwardWinners[award]][0];
            colour.c[1] = PAD_COLOURS[mAwardWinners[award]][1];
            colour.c[2] = PAD_COLOURS[mAwardWinners[award]][2];
            colour.c[3] = 0xFF;

            pText->SetAssetColour(colour);
        }
    }

    TLTextInstance* pPlacement = FEFinder<TLTextInstance, 3>::Find(
        presentation, InlineHasher(nlStringLowerHash("Slide1")), InlineHasher(nlStringLowerHash("Layer")), InlineHasher(nlStringLowerHash("Placement")), InlineHasher(0), InlineHasher(0), InlineHasher(0));
    pPlacement->m_bVisible = 0;

    TLTextInstance* pPlacementTitle = FEFinder<TLTextInstance, 3>::Find(
        presentation, InlineHasher(nlStringLowerHash("Slide1")), InlineHasher(nlStringLowerHash("Layer")), InlineHasher(nlStringLowerHash("Placement_Title")), InlineHasher(0), InlineHasher(0), InlineHasher(0));
    pPlacementTitle->m_bVisible = 0;
}

/**
 * Offset/Address/Size: 0x23EC | 0x800D43E8 | size: 0x5BC
 */
void BraggingRightsOverlay::TournamentSceneCreated()
{
}

/**
 * Offset/Address/Size: 0x1EA0 | 0x800D3E9C | size: 0x54C
 */
void BraggingRightsOverlay::Update(float)
{
}

/**
 * Offset/Address/Size: 0x15A4 | 0x800D35A0 | size: 0x8FC
 */
void BraggingRightsOverlay::ChangeTicker(int tickerRow)
{
    static unsigned long DETAIL_NAMES[6][2] = {
        { 0x1583A4AF, 0x779A26F3 },
        { 0x14C01771, 0x1EF8D3F5 },
        { 0x361BDF85, 0x88E8F289 },
        { 0x311076D6, 0x994F54FA },
        { 0xB3863DA8, 0x2C4EE80C },
        { 0x724E4E75, 0xF19F2F79 },
    };

    BasicString<char, Detail::TempStringAllocator> statString
        = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(mHighestStats[tickerRow]);
    unsigned short statWideString[16];
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    nlStrToWcs(statString.c_str(), statWideString, 16);

    if (mHighestStats[tickerRow] != 0)
    {
        if (mIsTournamentScene)
        {
            eTeamID winningTeam = nlSingleton<GameInfoManager>::s_pInstance->GetTeamStatsByIndex((unsigned short)mAwardWinners[tickerRow]).mTeamIndex;

            if (tickerRow == 4)
            {
                tickerRow++;
            }

            const unsigned short* detailFormat = LookupLocHash(DETAIL_NAMES[tickerRow][0]);
            const unsigned short* winnerName = LookupLocHash(GetLOCCharacterName(winningTeam, false, false));

            formatted = Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, unsigned short[16]>(
                BasicString<unsigned short, Detail::TempStringAllocator>(detailFormat), winnerName, statWideString);
        }
        else
        {
            const unsigned short* detailFormat = LookupLocHash(DETAIL_NAMES[tickerRow][0]);
            const unsigned short* winnerName = LookupLocHash(CONTROLLER_TEXT[mAwardWinners[tickerRow]]);

            formatted = Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, unsigned short[16]>(
                BasicString<unsigned short, Detail::TempStringAllocator>(detailFormat), winnerName, statWideString);
        }
    }
    else
    {
        const unsigned short* locString = LookupLocHash(0xBA05BFBD);
        formatted = BasicString<unsigned short, Detail::TempStringAllocator>(locString);
    }

    memcpy(mBuffer, formatted.c_str(), sizeof(mBuffer));

    BasicString<unsigned short, Detail::TempStringAllocator> message(mBuffer);
    mTicker->SetDisplayMessage(message);
}

/**
 * Offset/Address/Size: 0x153C | 0x800D3538 | size: 0x68
 */
BraggingRightsScene::BraggingRightsScene()
{
}

/**
 * Offset/Address/Size: 0x14C4 | 0x800D34C0 | size: 0x78
 */
BraggingRightsScene::~BraggingRightsScene()
{
}

/**
 * Offset/Address/Size: 0x2E4 | 0x800D22E0 | size: 0x11E0
 */
void BraggingRightsScene::SceneCreated()
{
    static char* CUP_BRAG_TEXT[5][4] = {
        { "Goals", "CUP_MILESTONEUPDATE_VETERAN", "Goals_Detail", "Goals_Detail2" },
        { "Hits", "CUP_MILESTONEUPDATE_SNIPER", "Hits_Detail", "Hits_Detail2" },
        { "Steals", "CUP_MILESTONEUPDATE_SUPERSTRIKER", "Steals_Detail", "Steals_Detail2" },
        { "Weapons", "CUP_MILESTONEUPDATE_TACTICIAN", "Weapons_Detail", "Weapons_Detail2" },
        { "Buttons2", "CUP_MILESTONEUPDATE_PARAMEDIC", "Buttons_Detail", "Buttons_Detail2" },
    };

    static int MILESTONE_GOALS[5] = { 100, 300, 100, 300, 1000 };
    static unsigned short ratioFormat[] = { '{', '0', '}', '/', '{', '1', '}', 0 };

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    GameInfoManager* info = nlSingleton<GameInfoManager>::s_pInstance;

    int wins;
    int losses;
    TLTextInstance* pText;
    int totalStats[5];
    int currentStats[5];
    TeamStats userStats;
    unsigned char complete[5] = { 0, 0, 0, 0, 0 };
    int i;

    for (i = 0; i < info->GetNumPlayingTeams(); i++)
    {
        TeamStats tempStats = info->GetTeamStatsByIndex((unsigned short)i);
        if (tempStats.mTeamIndex == info->GetUserSelectedCupTeam())
        {
            userStats = tempStats;
        }
    }

    totalStats[0] = info->mUserInfo.mNumGamesPlayed;
    currentStats[0] = userStats.mPlayerTotalStats.mNumGamesPlayed;
    if (totalStats[0] >= 100)
    {
        complete[0] = 1;
    }

    totalStats[1] = info->mUserInfo.mNumGoalsScored;
    currentStats[1] = userStats.mPlayerTotalStats.mNumGoalsFor;
    if (totalStats[1] >= 300)
    {
        complete[1] = 1;
    }

    totalStats[2] = info->mUserInfo.mNumSTSAttempts;
    currentStats[2] = userStats.mPlayerTotalStats.mNumSTSAttempts;
    if (totalStats[2] >= 100)
    {
        complete[2] = 1;
    }

    totalStats[3] = info->mUserInfo.mNumPerfectPasses;
    currentStats[3] = userStats.mPlayerTotalStats.mNumPerfectPasses;
    if (totalStats[3] >= 300)
    {
        complete[3] = 1;
    }

    totalStats[4] = info->mUserInfo.mNumHits;
    currentStats[4] = userStats.mPlayerTotalStats.mNumHitsMade;
    if (totalStats[4] >= 1000)
    {
        complete[4] = 1;
    }

    TLTextInstance* pTitleText = FEFinder<TLTextInstance, 3>::Find(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Title")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pTitleText->m_LocStrId = 0xB1839C7A;
    pTitleText->m_OverloadFlags |= 0x8;

    TLComponentInstance* buttonComponent = FEFinder<TLComponentInstance, 4>::Find(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    mButtons.mButtonInstance = buttonComponent;
    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    for (i = 0; i < 5; i++)
    {
        TLTextInstance* pLabelText = FEFinder<TLTextInstance, 3>::Find(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(CUP_BRAG_TEXT[i][0])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));
        pLabelText->SetStringId(CUP_BRAG_TEXT[i][1]);

        TLTextInstance* pStatText = FEFinder<TLTextInstance, 3>::Find(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(CUP_BRAG_TEXT[i][3])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        if (!complete[i])
        {
            BasicString<unsigned short, Detail::TempStringAllocator> unformatted(ratioFormat);
            BasicString<char, Detail::TempStringAllocator> statString
                = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(totalStats[i]);
            BasicString<char, Detail::TempStringAllocator> currentStatString
                = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(MILESTONE_GOALS[i]);
            unsigned short statWideString[32];
            unsigned short currentStatWideString[32];
            BasicString<unsigned short, Detail::TempStringAllocator> formatted;

            nlStrToWcs(statString.c_str(), statWideString, 32);
            nlStrToWcs(currentStatString.c_str(), currentStatWideString, 32);

            formatted = Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32], unsigned short[32]>(
                unformatted, statWideString, currentStatWideString);

            memcpy(mBuffer[i], formatted.c_str(), sizeof(mBuffer[i]));
            pStatText->SetString(mBuffer[i]);
        }
        else
        {
            BasicString<char, Detail::TempStringAllocator> statString
                = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(totalStats[i]);
            nlStrToWcs(statString.c_str(), mBuffer[i], 256);
            pStatText->SetString(mBuffer[i]);
        }

        pText = FEFinder<TLTextInstance, 3>::Find(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(CUP_BRAG_TEXT[i][2])),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        BasicString<char, Detail::TempStringAllocator> currentStatString
            = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(currentStats[i]);
        nlStrToWcs(currentStatString.c_str(), mThisCupBuffer[i], 256);
        pText->SetString(mThisCupBuffer[i]);
    }

    for (i = 0; i < info->GetNumPlayingTeams(); i++)
    {
        TeamStats userTeam = info->GetTeamStatsByIndex((unsigned short)i);
        if (userTeam.mTeamIndex == info->GetUserSelectedCupTeam())
        {
            wins = userTeam.mNumWins;
            losses = userTeam.mNumLosses + userTeam.mNumOTLosses;
            break;
        }
    }

    mUserPlace = info->DetermineUserPlacement(NULL);

    BasicString<char, Detail::TempStringAllocator> winString
        = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(wins);
    BasicString<char, Detail::TempStringAllocator> lossString
        = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(losses);
    unsigned short winWideString[32];
    unsigned short lossWideString[32];
    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(
        LookupLocHash(nlStringLowerHash("BRAG_RATIO")));
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    nlStrToWcs(winString.c_str(), winWideString, 32);
    nlStrToWcs(lossString.c_str(), lossWideString, 32);

    formatted = Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32], unsigned short[32]>(
        unformatted, winWideString, lossWideString);
    memcpy(mRatioBuffer, formatted.c_str(), sizeof(mRatioBuffer));

    pText = FEFinder<TLTextInstance, 3>::Find(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Win_Loss_Record")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    pText->SetString(mRatioBuffer);

    TLComponentInstance* pTemp = FEFinder<TLComponentInstance, 4>::Find(
        presentation,
        InlineHasher(nlStringLowerHash("Slide1")),
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("Placement")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pText = FEFinder<TLTextInstance, 3>::Find(
        pTemp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Placement")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (mUserPlace == -2)
    {
        pText->m_LocStrId = 0x813370B6;
        pText->m_OverloadFlags |= 0x8;
    }
    else
    {
        pText->m_LocStrId = GetLOCRank(mUserPlace);
        pText->m_OverloadFlags |= 0x8;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800D1FFC | size: 0x2E4
 */
void BraggingRightsScene::Update(float fDeltaT)
{
    FEPresentation* presentation;
    GameInfoManager* info;
    SaveLoadScene* scene;
    float fadeEndTime;

    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    fadeEndTime = presentation->m_currentSlide->m_start + presentation->m_currentSlide->m_duration;

    if (presentation->m_currentSlide->m_time < fadeEndTime)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL)
            || g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            presentation->m_fadeDuration = fadeEndTime;
        }
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        SceneList nextScene;

        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

        info = nlSingleton<GameInfoManager>::s_pInstance;
        nextScene = SCENE_MAIN_MENU;
        if (info->mCurrentMode == GameInfoManager::GM_BOWSER_CUP
            || info->mCurrentMode == GameInfoManager::GM_SUPER_BOWSER_CUP)
        {
            if (mUserPlace == 0)
            {
                nextScene = SCENE_CREDITS;
                FEMusic::StopStream();
            }
        }

        if (SaveLoadScene::IsIOEnabled())
        {
            scene = (SaveLoadScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SAVE, SCREEN_FORWARD, false);
            scene->mNextScene = nextScene;
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(nextScene, SCREEN_FORWARD, false);
        }

        SHMainMenu::mSnapMenuIntoPosition = false;
        SHMainMenu::mLastMenuItem = 0;
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        info = nlSingleton<GameInfoManager>::s_pInstance;
        if (info->mCurrentMode == GameInfoManager::GM_BOWSER_CUP)
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_STANDINGS_FINAL_ANIM, SCREEN_BACK, true);
        }
        else if (info->mCurrentMode == GameInfoManager::GM_SUPER_BOWSER_CUP)
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SUPER_CUP_STANDINGS_FINAL_ANIM, SCREEN_BACK, true);
        }
        else if (info->IsInTournamentMode() && info->mCustomTournamentInfo.m_tournMode == TM_KNOCKOUT)
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TOURNAMENT_STANDINGS_FINAL_ANIM, SCREEN_BACK, true);
        }
        else if (info->IsInRegularCupMode())
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_STANDINGS, SCREEN_BACK, true);
        }
        else if (info->IsInSuperCupMode())
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_SUPER_CUP_STANDINGS, SCREEN_BACK, true);
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TOURNAMENT_STANDINGS, SCREEN_BACK, true);
        }

        FEAudio::PlayAnimAudioEvent("sfx_back", false);
    }
}
