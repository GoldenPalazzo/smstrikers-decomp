#include "Game/OverlayHandlerInGameText.h"
#include "Game/BaseSceneHandler.h"
#include "Game/DB/StatsTracker.h"
#include "Game/FE/FEPresentation.h"
#include "Game/FE/tlSlide.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/OverlayManager.h"
#include "NL/nlLocalization.h"
#include "NL/nlSingleton.h"
#include "NL/nlTask.h"
#include "Game/FE/feInput.h"

#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feTemplates.h"

#include "types.h"

extern FEInput* g_pFEInput;

template <typename StringType, typename ValueType>
StringType Format(const StringType&, const ValueType&);

template <typename StringType, typename T1, typename T2>
StringType Format(const StringType&, const T1&, const T2&);

template <typename T, typename Key>
T* nlBSearch(const Key&, T*, int);

unsigned long GetLOCTeamName(eTeamID);

template <>
nlLocalization::StringLookup* nlBSearch<nlLocalization::StringLookup, unsigned long>(
    const unsigned long&, nlLocalization::StringLookup*, int);

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

static char* TEAM_SLIDE_NAMES[8] = {
    "DAISY",
    "DK",
    "LUIGI",
    "MARIO",
    "PEACH",
    "WALUIGI",
    "WARIO",
    "YOSHI"
}; // size: 0x20, address: 0x802BFE60

static const char* OVERLAY_HANDLER_LAYER_NAME = "Layer"; // size: 0x4, address: 0x80395EAC

static const struct InGameTextEntry IGTTable[8] = {
    { SLIDE_NAME_TEXT_GOAL, "GOAL!", 0 },
    { SLIDE_NAME_TEXT_KICKOFF, "KICKOFF!", 0 },
    { SLIDE_NAME_TEXT_WINNER, "WINNER!", 1 },
    { SLIDE_NAME_TEXT_PAUSE, "Pause", 1 },
    { SLIDE_NAME_TEXT_TIE, "TIE!", 1 },
    { SLIDE_NAME_TEXT_LOADING, "LOADING...", 1 },
    { SLIDE_NAME_TEXT_SHOOT, "Shoot!", 2 },
    { SLIDE_NAME_TEXT_REPLAY, "REPLAY", 16 }
}; // size: 0x60, address: 0x802AD8E0

/**
 * Offset/Address/Size: 0xBC | 0x800FC998 | size: 0x208
 */
/*
void BasicString<unsigned short, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<unsigned short, Detail::TempStringAllocator>&)
{
}
*/
/**
 * Offset/Address/Size: 0x0 | 0x800FC8DC | size: 0xBC
 */
template <>
template <>
BasicString<unsigned short, Detail::TempStringAllocator>
BasicString<unsigned short, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& rhs) const
{
    BasicString r(*this);
    r.AppendInPlace(rhs);
    BasicStringData<unsigned short>* data = r.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }
    return BasicString(data);
}

/**
 * Offset/Address/Size: 0x2D4 | 0x800FC3DC | size: 0x84
 */
#pragma dont_inline on
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x358 | 0x800FC460 | size: 0x84
 */
#pragma dont_inline on
#pragma dont_inline reset

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void OverlayHandlerInGameText_stub()
{
    void (*volatile forceTrack)(ePlayerStats, int, int, int, int, int, int) = &StatsTracker::Track;
    (void)forceTrack;
    TLInstance* (*volatile forceFind)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher) = &FEFinder<TLInstance, 3>::Find<FEPresentation>;
    (void)forceFind;
}

/**
 * Offset/Address/Size: 0xF44 | 0x800FBFF0 | size: 0xA8
 */
InGameTextOverlay::InGameTextOverlay()
    : BaseOverlayHandler(2, POSITION_ALL)
{
    mCurrentSlideName = SLIDE_NAME_INVALID;
    mPendingSlideName = SLIDE_NAME_INVALID;
    this->SetVisible(false);
}

/**
 * Offset/Address/Size: 0xED8 | 0x800FBF84 | size: 0x6C
 */
#pragma inline_depth(16)
InGameTextOverlay::~InGameTextOverlay()
{
}

/**
 * Offset/Address/Size: 0xE1C | 0x800FBEC8 | size: 0xBC
 */
void InGameTextOverlay::SetSlide(OverlaySlideName slideName)
{
    this->mPendingSlideName = slideName;
    if (this->mCurrentSlideName != this->mPendingSlideName)
    {
        this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[this->mPendingSlideName].mSlideName);
        TLSlide* CurrentSlide = this->m_pFEScene->m_pFEPackage->GetPresentation()->m_currentSlide;
        if (CurrentSlide != NULL)
        {
            CurrentSlide->m_time = 0.0f;
            CurrentSlide->m_start = 0.0f;
            CurrentSlide->Update(0.0f);
        }
        if (mCurrentSlideName != SLIDE_NAME_INVALID)
        {
            this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[mCurrentSlideName].mSlideName);
        }
    }
}

/**
 * Offset/Address/Size: 0xCB0 | 0x800FBD5C | size: 0x16C
 */
void InGameTextOverlay::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    if (this->mCurrentSlideName != this->mPendingSlideName)
    {
        this->mCurrentSlideName = this->mPendingSlideName;
        this->m_pFEScene->m_pFEPackage->GetPresentation()->SetActiveSlide(IGTTable[this->mCurrentSlideName].mSlideName);
        this->mVisibilityMask = IGTTable[this->mCurrentSlideName].mTaskVisibility;
        if (this->mVisibilityMask & nlTaskManager::m_pInstance->m_CurrState)
        {
            if (mWasLastVisible)
            {
                this->SetVisible(true);
            }
        }
        else
        {
            mWasLastVisible = m_bVisible;
            this->SetVisible(false);
        }

        switch (this->mCurrentSlideName)
        {
        case SLIDE_NAME_TEXT_WINNER:
            DisplayFinalScore();
            break;
        }
    }
    if (this->mCurrentSlideName == SLIDE_NAME_TEXT_WINNER && g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL) && m_bVisible)
    {
        nlSingleton<OverlayManager>::s_pInstance->SetVisible(OVERLAY_TEXT, false, false);
        SummaryOverlay* handler = (SummaryOverlay*)nlSingleton<OverlayManager>::s_pInstance->Push(OVERLAY_SUMMARY, SCREEN_NOTHING, false);
        handler->mButtonState = ButtonComponent::BS_A_ONLY;
    }
}

/**
 * Offset/Address/Size: 0xCAC | 0x800FBD58 | size: 0x4
 */
void InGameTextOverlay::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800FB0AC | size: 0xCAC
 */
void InGameTextOverlay::DisplayFinalScore()
{
    typedef BasicString<char, Detail::TempStringAllocator> NarrowString;
    typedef BasicString<unsigned short, Detail::TempStringAllocator> WideString;

    typedef TLTextInstance* (*FindTextByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLInstance* (*FindInstByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLInstance* (*FindInstByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindInstByValue byValue;
        FindInstByRef byRef;
    } findInst;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    int scoreLeft = g_pTeams[0]->m_nScore;
    int scoreRight = g_pTeams[1]->m_nScore;

    NarrowString scoreLeftString(LexicalCast<NarrowString, int>(scoreLeft));
    NarrowString scoreRightString(LexicalCast<NarrowString, int>(scoreRight));

    unsigned short scoreLeftWideString[32];
    unsigned short scoreRightWideString[32];

    nlStrToWcs(scoreLeftString.c_str(), scoreLeftWideString, 32);
    nlStrToWcs(scoreRightString.c_str(), scoreRightWideString, 32);

    const unsigned short* formatLocString;
    unsigned long scoreFormatKey = 0x8C4180A4;
    nlLocalization* loc = g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        formatLocString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(scoreFormatKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            formatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            formatLocString = MissingLocString;
        }
    }

    WideString unformatted(formatLocString);
    WideString formatted(Format(unformatted, scoreLeftWideString, scoreRightWideString));

    long winningSide;

    FEPresentation* presentation = this->m_pFEScene->m_pFEPackage->GetPresentation();
    TLTextInstance* pTextInstance;
    const char* WINNER_SLIDE_NAME = IGTTable[SLIDE_NAME_TEXT_WINNER].mSlideName;

    if (this->mCurrentSlideName == SLIDE_NAME_TEXT_WINNER)
    {
        volatile InlineHasher hSlideB, hSlideA;
        volatile InlineHasher hLayerB, hLayerA;
        volatile InlineHasher hScoreB, hScoreA;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        unsigned long hash;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;

        hash = nlStringLowerHash("Score");
        hScoreA.m_Hash = hash;
        hScoreB.m_Hash = hash;

        hash = nlStringLowerHash(OVERLAY_HANDLER_LAYER_NAME);
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_SLIDE_NAME);
        hSlideA.m_Hash = hash;
        hSlideB.m_Hash = hash;

        pTextInstance = findText.byRef(
            presentation,
            (InlineHasher&)hSlideB,
            (InlineHasher&)hLayerB,
            (InlineHasher&)hScoreB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        winningSide = (scoreLeft > scoreRight) ? 0 : 1;

        eTeamID winningTeam = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)winningSide);

        unsigned long teamNameStringID = GetLOCTeamName(winningTeam);
        const unsigned short* winnerLocString;

        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            winnerLocString = LocalizationTableNotFound;
        }
        else
        {
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

        WideString winnerNameWideString(winnerLocString);

        if (winningTeam == 3)
        {
            static const unsigned short SPACE_WCS[2] = { 0x20, 0x0 };

            WideString space(SPACE_WCS);
            winnerNameWideString = space.Append(winnerNameWideString);
        }

        unsigned long winnerFormatKey = 0x8610A152;
        loc = g_pLocalization;

        if (loc->m_LookupTable == 0)
        {
            formatLocString = LocalizationTableNotFound;
        }
        else
        {
            nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(winnerFormatKey, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
            if (entry)
            {
                formatLocString = loc->m_FirstString + entry->StringOffset;
            }
            else
            {
                formatLocString = MissingLocString;
            }
        }

        WideString unformattedName(formatLocString);
        WideString formattedName(Format(unformattedName, winnerNameWideString.c_str()));

        volatile InlineHasher hNameSlideB, hNameSlideA;
        volatile InlineHasher hNameLayerB, hNameLayerA;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher n5, n3, n1;

        findInst.byValue = FEFinder<TLInstance, 3>::Find<FEPresentation>;

        n1.m_Hash = 0;
        h1.m_Hash = 0;
        n3.m_Hash = 0;
        h3.m_Hash = 0;
        n5.m_Hash = 0;
        h5.m_Hash = 0;

        hash = nlStringLowerHash("name");
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash(OVERLAY_HANDLER_LAYER_NAME);
        hNameLayerA.m_Hash = hash;
        hNameLayerB.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_SLIDE_NAME);
        hNameSlideA.m_Hash = hash;
        hNameSlideB.m_Hash = hash;

        TLInstance* winnerNameInstance = findInst.byRef(
            presentation,
            (InlineHasher&)hNameSlideB,
            (InlineHasher&)hNameLayerB,
            (InlineHasher&)hNameB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        TLTextInstance* winnerNameTextInstance = (TLTextInstance*)winnerNameInstance;

        memcpy(mWinnerBuffer, formattedName.c_str(), 0x40);
        winnerNameTextInstance->SetString(mWinnerBuffer);

        eTeamID team = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);

        volatile InlineHasher hFaceSlideB, hFaceSlideA;
        volatile InlineHasher hFaceLayerB, hFaceLayerA;
        volatile InlineHasher hFaceB, hFaceA;
        volatile InlineHasher f5, f3, f1;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

        f1.m_Hash = 0;
        h1.m_Hash = 0;
        f3.m_Hash = 0;
        h3.m_Hash = 0;
        f5.m_Hash = 0;
        h5.m_Hash = 0;

        hash = nlStringLowerHash("left_face");
        hFaceA.m_Hash = hash;
        hFaceB.m_Hash = hash;

        hash = nlStringLowerHash(OVERLAY_HANDLER_LAYER_NAME);
        hFaceLayerA.m_Hash = hash;
        hFaceLayerB.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_SLIDE_NAME);
        hFaceSlideA.m_Hash = hash;
        hFaceSlideB.m_Hash = hash;

        TLComponentInstance* pComponentInstance = findComp.byRef(
            presentation,
            (InlineHasher&)hFaceSlideB,
            (InlineHasher&)hFaceLayerB,
            (InlineHasher&)hFaceB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        pComponentInstance->SetActiveSlide(TEAM_SLIDE_NAMES[team]);

        team = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);

        volatile InlineHasher hRFaceSlideB, hRFaceSlideA;
        volatile InlineHasher hRFaceLayerB, hRFaceLayerA;
        volatile InlineHasher hRFaceB, hRFaceA;
        volatile InlineHasher rf5, rf3, rf1;

        rf1.m_Hash = 0;
        h1.m_Hash = 0;
        rf3.m_Hash = 0;
        h3.m_Hash = 0;
        rf5.m_Hash = 0;
        h5.m_Hash = 0;

        hash = nlStringLowerHash("right_face");
        hRFaceA.m_Hash = hash;
        hRFaceB.m_Hash = hash;

        hash = nlStringLowerHash(OVERLAY_HANDLER_LAYER_NAME);
        hRFaceLayerA.m_Hash = hash;
        hRFaceLayerB.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_SLIDE_NAME);
        hRFaceSlideA.m_Hash = hash;
        hRFaceSlideB.m_Hash = hash;

        pComponentInstance = findComp.byRef(
            presentation,
            (InlineHasher&)hRFaceSlideB,
            (InlineHasher&)hRFaceLayerB,
            (InlineHasher&)hRFaceB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        pComponentInstance->SetActiveSlide(TEAM_SLIDE_NAMES[team]);

        if (nlSingleton<GameInfoManager>::s_pInstance->mCurrentMode != 0)
        {
            if (g_pGame->m_eGameState == GS_OVERTIME)
            {
                StatsTracker::Track(STATS_OT_WIN, winningSide, 0, scoreLeft, scoreRight, 0, 0);
            }
            else
            {
                StatsTracker::Track(STATS_WIN, winningSide, 0, scoreLeft, scoreRight, 0, 0);
            }
        }
        else
        {
            nlSingleton<StatsTracker>::s_pInstance->mNumGamesWon[winningSide]++;
        }
    }

    memcpy(mScoresBuffer, formatted.c_str(), 0x40);
    pTextInstance->SetString(mScoresBuffer);
}
