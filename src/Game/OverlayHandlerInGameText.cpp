#include "Game/OverlayHandlerInGameText.h"
#include "Game/BaseSceneHandler.h"
#include "Game/FE/FEPresentation.h"
#include "Game/FE/tlSlide.h"
#include "Game/OverlayManager.h"
#include "NL/nlSingleton.h"
#include "NL/nlTask.h"
#include "Game/FE/feInput.h"

#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feTemplates.h"

#include "types.h"

extern FEInput* g_pFEInput;

class GameInfoManager;
class StatsTracker;

extern "C" int GetTeam__15GameInfoManagerCFs(void*, short);
extern "C" unsigned long GetLOCTeamName__F7eTeamID(int);
extern "C" void Track__12StatsTrackerF12ePlayerStatsiiiiii(int, int, int, int, int, int, int);

template <typename To, typename From>
To LexicalCast(const From&);

template <typename StringType, typename ValueType>
StringType Format(const StringType&, const ValueType&);

template <typename StringType, typename T1, typename T2>
StringType Format(const StringType&, const T1&, const T2&);

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

        operator unsigned long() const { return hash; }
    };

    LOCHeader* m_pFile;
    StringLookup* m_LookupTable;
    unsigned short* m_FirstString;
    int m_CurrentLanguage;
};

template <>
nlLocalization::StringLookup* nlBSearch<nlLocalization::StringLookup, unsigned long>(
    const unsigned long&, nlLocalization::StringLookup*, int);

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

struct GameLocal
{
    char _padding[0x24];
    int m_eGameState;
};
extern GameLocal* g_pGame;

struct StatsTrackerAccessor
{
    char _padding[0x4B4];
    int mNumGamesWon[2];
};

struct GameInfoModeAccessor
{
    char _padding[0x4954];
    int mCurrentMode;
};

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
    return r;
}

/**
 * Offset/Address/Size: 0x678 | 0x800FC780 | size: 0x15C
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x5F4 | 0x800FC6FC | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x570 | 0x800FC678 | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x538 | 0x800FC640 | size: 0x38
 */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
//{
// }

/**
 * Offset/Address/Size: 0x3DC | 0x800FC4E4 | size: 0x15C
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::_Find<TLInstance>(
    TLInstance* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->pChildren, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->pChildren, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x2D4 | 0x800FC3DC | size: 0x84
 */
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::_Find<FEPresentation>(
    FEPresentation* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLSlide>(pTopLevel->m_slides, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLSlide>(CastToSomeType<TLSlide>(pTopLevel->m_slides, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}

/**
 * Offset/Address/Size: 0x358 | 0x800FC460 | size: 0x84
 */
#pragma dont_inline on
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::_Find<TLSlide>(
    TLSlide* pTopLevel, const unsigned long Level1, const unsigned long Level2,
    const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
{
    void* pChild = FindItemByHashID<TLInstance>(pTopLevel->m_instances, Level1);
    if (pChild == 0)
        return 0;
    if (Level2 == 0)
        return (TLInstance*)pChild;
    return _Find<TLInstance>(CastToSomeType<TLInstance>(pTopLevel->m_instances, pChild), Level2, Level3, Level4, Level5, Level6, 0);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x29C | 0x800FC3A4 | size: 0x38
 */
#pragma inline_depth(0)
template <>
template <>
TLInstance* FEFinder<TLInstance, 3>::Find<FEPresentation>(
    FEPresentation* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3,
    InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}
#pragma inline_depth()

/**
 * Offset/Address/Size: 0x140 | 0x800FC248 | size: 0x15C
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0xBC | 0x800FC1C4 | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x38 | 0x800FC140 | size: 0x84
 */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FC108 | size: 0x38
 */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x800FC098 | size: 0x70
 */
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
    unsigned long key = 0x8C4280A4;
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

    WideString unformatted(data);
    WideString formatted(Format(unformatted, scoreLeftWideString, scoreRightWideString));

    FEPresentation* presentation = this->m_pFEScene->m_pFEPackage->GetPresentation();
    TLTextInstance* pTextInstance;
    const char* WINNER_SLIDE_NAME = IGTTable[this->mCurrentSlideName].mSlideName;

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

        int winningSide = (scoreRight >> 31)
                        + ((unsigned int)scoreLeft >> 31)
                        + ((unsigned int)scoreRight >= (unsigned int)scoreLeft);

        int winningTeam = GetTeam__15GameInfoManagerCFs(
            nlSingleton<GameInfoManager>::s_pInstance,
            (short)winningSide);

        unsigned long teamNameStringID = GetLOCTeamName__F7eTeamID(winningTeam);
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

        BasicStringData<unsigned short>* winnerData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
        if (winnerData)
        {
            winnerData->mData = 0;
            winnerData->mSize = 0;
            winnerData->mCapacity = 0;

            const unsigned short* ptr = winnerLocString;
            while (*ptr++)
            {
                winnerData->mSize++;
            }

            winnerData->mSize++;
            winnerData->mData = (unsigned short*)nlMalloc((winnerData->mSize + 1) * 2, 8, true);
            winnerData->mCapacity = winnerData->mSize;

            int i = 0;
            int j = 0;
            while (i < winnerData->mSize)
            {
                *(unsigned short*)((char*)winnerData->mData + j) = *winnerLocString;
                i++;
                winnerLocString++;
                j += 2;
            }

            winnerData->mRefCount = 1;
        }

        WideString winnerNameWideString(winnerData);

        if (winningTeam == 3)
        {
            static const unsigned short SPACE_WCS[2] = { 0x20, 0x0 };

            const unsigned short* spaceLocString = SPACE_WCS;

            BasicStringData<unsigned short>* spaceData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
            if (spaceData)
            {
                spaceData->mData = 0;
                spaceData->mSize = 0;
                spaceData->mCapacity = 0;

                const unsigned short* ptr = spaceLocString;
                while (*ptr++)
                {
                    spaceData->mSize++;
                }

                spaceData->mSize++;
                spaceData->mData = (unsigned short*)nlMalloc((spaceData->mSize + 1) * 2, 8, true);
                spaceData->mCapacity = spaceData->mSize;

                int i = 0;
                int j = 0;
                while (i < spaceData->mSize)
                {
                    *(unsigned short*)((char*)spaceData->mData + j) = *spaceLocString;
                    i++;
                    spaceLocString++;
                    j += 2;
                }

                spaceData->mRefCount = 1;
            }

            WideString space(spaceData);
            winnerNameWideString = space.Append(winnerNameWideString);
        }

        key = 0x8611A152;

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

        BasicStringData<unsigned short>* nameData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
        if (nameData)
        {
            nameData->mData = 0;
            nameData->mSize = 0;
            nameData->mCapacity = 0;

            const unsigned short* ptr = formatLocString;
            while (*ptr++)
            {
                nameData->mSize++;
            }

            nameData->mSize++;
            nameData->mData = (unsigned short*)nlMalloc((nameData->mSize + 1) * 2, 8, true);
            nameData->mCapacity = nameData->mSize;

            int i = 0;
            int j = 0;
            while (i < nameData->mSize)
            {
                *(unsigned short*)((char*)nameData->mData + j) = *formatLocString;
                i++;
                formatLocString++;
                j += 2;
            }

            nameData->mRefCount = 1;
        }

        WideString unformattedName(nameData);
        WideString formattedName(Format(unformattedName, winnerNameWideString.c_str()));

        volatile InlineHasher hNameSlideB, hNameSlideA;
        volatile InlineHasher hNameLayerB, hNameLayerA;
        volatile InlineHasher hNameB, hNameA;

        findInst.byValue = FEFinder<TLInstance, 3>::Find<FEPresentation>;

        h1.m_Hash = 0;
        h3.m_Hash = 0;
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

        int team = GetTeam__15GameInfoManagerCFs(
            nlSingleton<GameInfoManager>::s_pInstance,
            0);

        volatile InlineHasher hFaceSlideB, hFaceSlideA;
        volatile InlineHasher hFaceLayerB, hFaceLayerA;
        volatile InlineHasher hFaceB, hFaceA;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

        h5.m_Hash = 0;
        h3.m_Hash = 0;
        h1.m_Hash = 0;

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

        team = GetTeam__15GameInfoManagerCFs(
            nlSingleton<GameInfoManager>::s_pInstance,
            1);

        h5.m_Hash = 0;
        h3.m_Hash = 0;
        h1.m_Hash = 0;

        hash = nlStringLowerHash("right_face");
        hFaceA.m_Hash = hash;
        hFaceB.m_Hash = hash;

        hash = nlStringLowerHash(OVERLAY_HANDLER_LAYER_NAME);
        hFaceLayerA.m_Hash = hash;
        hFaceLayerB.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_SLIDE_NAME);
        hFaceSlideA.m_Hash = hash;
        hFaceSlideB.m_Hash = hash;

        pComponentInstance = findComp.byRef(
            presentation,
            (InlineHasher&)hFaceSlideB,
            (InlineHasher&)hFaceLayerB,
            (InlineHasher&)hFaceB,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        pComponentInstance->SetActiveSlide(TEAM_SLIDE_NAMES[team]);

        if (((GameInfoModeAccessor*)nlSingleton<GameInfoManager>::s_pInstance)->mCurrentMode != 0)
        {
            if (g_pGame->m_eGameState == 5)
            {
                Track__12StatsTrackerF12ePlayerStatsiiiiii(7, winningSide, 0, scoreLeft, scoreRight, 0, 0);
            }
            else
            {
                Track__12StatsTrackerF12ePlayerStatsiiiiii(6, winningSide, 0, scoreLeft, scoreRight, 0, 0);
            }
        }
        else
        {
            ((StatsTrackerAccessor*)nlSingleton<StatsTracker>::s_pInstance)->mNumGamesWon[winningSide]++;
        }
    }

    memcpy(mScoresBuffer, formatted.c_str(), 0x40);
    pTextInstance->SetString(mScoresBuffer);
}
