#include "Game/FE/Overlay/OverlayHandlerWinner.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/FE/Overlay/OverlayHandlerSummary.h"
#include "Game/OverlayManager.h"

static char* WINNER_TEXTURES[9][3] = {
    { "fe/winners/daisy_action", "fe/winners/daisy_action_OUTLINE", "fe/winners/daisy_action_WHITE" },
    { "fe/winners/dk_action", "fe/winners/dk_action_OUTLINE", "fe/winners/dk_action_WHITE" },
    { "fe/winners/luigi_action", "fe/winners/luigi_action_OUTLINE", "fe/winners/luigi_action_WHITE" },
    { "fe/winners/mario_action", "fe/winners/mario_action_OUTLINE", "fe/winners/mario_action_WHITE" },
    { "fe/winners/peach_action", "fe/winners/peach_action_OUTLINE", "fe/winners/peach_action_WHITE" },
    { "fe/winners/waluigi_action", "fe/winners/waluigi_action_OUTLINE", "fe/winners/waluigi_action_WHITE" },
    { "fe/winners/wario_action", "fe/winners/wario_action_OUTLINE", "fe/winners/wario_action_WHITE" },
    { "fe/winners/yoshi_action", "fe/winners/yoshi_action_OUTLINE", "fe/winners/yoshi_action_WHITE" },
    { "fe/winners/mario_action", "fe/winners/mario_action_OUTLINE", "fe/winners/mario_action_WHITE" },
};

static const char* WINNER_HANDLER_LAYER_NAME = "Layer";

// /**
//  * Offset/Address/Size: 0x4EC | 0x80106C80 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x80106BFC | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x80106BC4 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x80106A68 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x801069E4 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x801069AC | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x80106850 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x801067CC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80106794 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x10C0 | 0x8010672C | size: 0x68
 */
WinnerOverlay::WinnerOverlay()
    : BaseOverlayHandler(-1, POSITION_ALL)
{
    mInputDelay = 1.0f;
    mDoingOutTransition = false;
    mWinningTeam = TEAM_INVALID;
}

/**
 * Offset/Address/Size: 0xFE4 | 0x80106650 | size: 0xDC
 */
WinnerOverlay::~WinnerOverlay()
{
    delete mWinnerActionWhite;
    delete mWinnerAction;
    delete mWinnerActionOutline;
}

/**
 * Offset/Address/Size: 0x304 | 0x80105970 | size: 0xCE0
 * TODO: 75.07% match - stack layout/register allocation still diverges in
 * localization string construction and UI setup paths.
 */
template <typename To, typename From>
To LexicalCast(const From&);

template <typename StringType, typename ValueType>
StringType Format(const StringType&, const ValueType&);

template <typename StringType, typename ValueType1, typename ValueType2>
StringType Format(const StringType&, const ValueType1&, const ValueType2&);

class GameInfoManager;

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

extern "C" int GetTeam__15GameInfoManagerCFs(void*, short);
extern "C" bool IsInDemoMode__15GameInfoManagerCFv(void*);
extern "C" unsigned long GetLOCTeamName__F7eTeamID(int);

void WinnerOverlay::SceneCreated()
{
    struct LOCHeaderLocal
    {
        char Thumbprint[4];
        unsigned long Version;
        unsigned long Language;
        unsigned long StringCount;
        unsigned long Flags;
    };

    struct StringLookupLocal
    {
        unsigned long hash;
        unsigned long StringOffset;

        operator unsigned long() const { return hash; }
    };

    struct LocalizationLocal
    {
        LOCHeaderLocal* m_pFile;
        StringLookupLocal* m_LookupTable;
        unsigned short* m_FirstString;
        int m_CurrentLanguage;
    };

    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    unsigned long hash;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;
    volatile InlineHasher gB, gA;
    volatile InlineHasher g5, g4, g3, g2, g1, g0;
    volatile InlineHasher fB, fA;
    volatile InlineHasher f5, f4, f3, f2, f1, f0;

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

    const unsigned short* formatLocString;
    unsigned long key = 0x8C4280A4;
    LocalizationLocal* loc = (LocalizationLocal*)g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        formatLocString = LocalizationTableNotFound;
    }
    else
    {
        StringLookupLocal* entry = nlBSearch<StringLookupLocal, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            formatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            formatLocString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* scoreFormatData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (scoreFormatData)
    {
        scoreFormatData->mData = 0;
        scoreFormatData->mSize = 0;
        scoreFormatData->mCapacity = 0;

        const unsigned short* ptr = formatLocString;
        while (*ptr++)
        {
            scoreFormatData->mSize++;
        }

        scoreFormatData->mSize++;
        scoreFormatData->mData = (unsigned short*)nlMalloc((scoreFormatData->mSize + 1) * 2, 8, true);
        scoreFormatData->mCapacity = scoreFormatData->mSize;

        int i = 0;
        int j = 0;
        while (i < scoreFormatData->mSize)
        {
            *(unsigned short*)((char*)scoreFormatData->mData + j) = *formatLocString;
            i++;
            formatLocString++;
            j += 2;
        }

        scoreFormatData->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(scoreFormatData);
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    if (scoreLeft > scoreRight)
    {
        formatted = Format(unformatted, scoreLeftWideString, scoreRightWideString);
    }
    else
    {
        formatted = Format(unformatted, scoreRightWideString, scoreLeftWideString);
    }

    memcpy(mScoresBuffer, formatted.c_str(), 0x40);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    presentation->SetActiveSlide("MENU IN2");

    void* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    short winnerSide = (short)((scoreRight >> 31) + ((unsigned int)scoreLeft >> 31) + ((unsigned int)scoreRight >= (unsigned int)scoreLeft));
    mWinningTeam = (eTeamID)GetTeam__15GameInfoManagerCFs(gameInfo, winnerSide);

    unsigned long winnerLocID = GetLOCTeamName__F7eTeamID(mWinningTeam);
    const unsigned short* winnerLocString;

    loc = (LocalizationLocal*)g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        winnerLocString = LocalizationTableNotFound;
    }
    else
    {
        StringLookupLocal* entry = nlBSearch<StringLookupLocal, unsigned long>(winnerLocID, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            winnerLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            winnerLocString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* winnerNameData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (winnerNameData)
    {
        winnerNameData->mData = 0;
        winnerNameData->mSize = 0;
        winnerNameData->mCapacity = 0;

        const unsigned short* ptr = winnerLocString;
        while (*ptr++)
        {
            winnerNameData->mSize++;
        }

        winnerNameData->mSize++;
        winnerNameData->mData = (unsigned short*)nlMalloc((winnerNameData->mSize + 1) * 2, 8, true);
        winnerNameData->mCapacity = winnerNameData->mSize;

        int i = 0;
        int j = 0;
        while (i < winnerNameData->mSize)
        {
            *(unsigned short*)((char*)winnerNameData->mData + j) = *winnerLocString;
            i++;
            winnerLocString++;
            j += 2;
        }

        winnerNameData->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> winnerNameWideString(winnerNameData);

    key = 0x8611A152;
    const unsigned short* winnerFormatLocString;

    loc = (LocalizationLocal*)g_pLocalization;

    if (loc->m_LookupTable == 0)
    {
        winnerFormatLocString = LocalizationTableNotFound;
    }
    else
    {
        StringLookupLocal* entry = nlBSearch<StringLookupLocal, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry)
        {
            winnerFormatLocString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            winnerFormatLocString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* winnerFormatData = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (winnerFormatData)
    {
        winnerFormatData->mData = 0;
        winnerFormatData->mSize = 0;
        winnerFormatData->mCapacity = 0;

        const unsigned short* ptr = winnerFormatLocString;
        while (*ptr++)
        {
            winnerFormatData->mSize++;
        }

        winnerFormatData->mSize++;
        winnerFormatData->mData = (unsigned short*)nlMalloc((winnerFormatData->mSize + 1) * 2, 8, true);
        winnerFormatData->mCapacity = winnerFormatData->mSize;

        int i = 0;
        int j = 0;
        while (i < winnerFormatData->mSize)
        {
            *(unsigned short*)((char*)winnerFormatData->mData + j) = *winnerFormatLocString;
            i++;
            winnerFormatLocString++;
            j += 2;
        }

        winnerFormatData->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformattedName(winnerFormatData);
    BasicString<unsigned short, Detail::TempStringAllocator> formattedName;

    formattedName = Format(unformattedName, winnerNameWideString.c_str());

    memcpy(mWinnerBuffer, formattedName.c_str(), 0x40);

    for (int i = 0; i < 2; i++)
    {
        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("Score");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        hA.m_Hash = hash;
        hB.m_Hash = hash;

        TLTextInstance* pTextInstance = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        pTextInstance->SetString(mScoresBuffer);

        g0.m_Hash = 0;
        h1.m_Hash = 0;
        g1.m_Hash = 0;
        h3.m_Hash = 0;
        g2.m_Hash = 0;
        h5.m_Hash = 0;
        g3.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("name");
        g4.m_Hash = hash;
        g5.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        gA.m_Hash = hash;
        gB.m_Hash = hash;

        TLTextInstance* winnerNameTextInstance = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)gB,
            (InlineHasher&)g5,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        winnerNameTextInstance->SetString(mWinnerBuffer);

        presentation->SetActiveSlide("MENU IN");
    }

    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    hash = nlStringLowerHash("action");
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLImageInstance* pImage = findImage.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mWinnerAction = new (nlMalloc(sizeof(AsyncImage), 0x20, true)) AsyncImage("art/fe/WinnersUI.res", NULL);
    mWinnerAction->mImageInstance = pImage;
    mWinnerAction->QueueLoad(WINNER_TEXTURES[mWinningTeam][0], false);

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;

    hash = nlStringLowerHash("action_OUTLINE");
    g4.m_Hash = hash;
    g5.m_Hash = hash;

    hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    pImage = findImage.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)gB,
        (InlineHasher&)g5,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mWinnerActionOutline = new (nlMalloc(sizeof(AsyncImage), 0x20, true)) AsyncImage("art/fe/WinnersUI.res", NULL);
    mWinnerActionOutline->mImageInstance = pImage;
    mWinnerActionOutline->QueueLoad(WINNER_TEXTURES[mWinningTeam][1], false);

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;

    hash = nlStringLowerHash("action_WHITE");
    f4.m_Hash = hash;
    f5.m_Hash = hash;

    hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    pImage = findImage.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)fB,
        (InlineHasher&)f5,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mWinnerActionWhite = new (nlMalloc(sizeof(AsyncImage), 0x20, true)) AsyncImage("art/fe/WinnersUI.res", NULL);
    mWinnerActionWhite->mImageInstance = pImage;
    mWinnerActionWhite->QueueLoad(WINNER_TEXTURES[mWinningTeam][2], false);

    if (IsInDemoMode__15GameInfoManagerCFv(gameInfo))
    {
        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        g0.m_Hash = 0;
        h1.m_Hash = 0;
        g1.m_Hash = 0;
        h3.m_Hash = 0;
        g2.m_Hash = 0;
        h5.m_Hash = 0;
        g3.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("buttons");
        g4.m_Hash = hash;
        g5.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        gA.m_Hash = hash;
        gB.m_Hash = hash;

        TLComponentInstance* pComp = findComp.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)gB,
            (InlineHasher&)g5,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        pComp->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8010566C | size: 0x304
 */
void WinnerOverlay::Update(float fDeltaT)
{
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    unsigned long hash;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;
    volatile InlineHasher gB, gA;
    volatile InlineHasher g5, g4, g3, g2, g1, g0;
    volatile InlineHasher fB, fA;
    volatile InlineHasher f5, f4, f3, f2, f1, f0;

    BaseSceneHandler::Update(fDeltaT);

    mWinnerAction->Update(true);
    mWinnerActionOutline->Update(true);
    mWinnerActionWhite->Update(true);

    mInputDelay -= fDeltaT;

    TLSlide* slide = m_pFEPresentation->m_currentSlide;

    if (mDoingOutTransition)
    {
        if (slide->m_time >= (slide->m_start + slide->m_duration))
        {
            SummaryOverlay* pSummary = (SummaryOverlay*)nlSingleton<OverlayManager>::s_pInstance->Push(OVERLAY_SUMMARY, SCREEN_NOTHING, true);
            pSummary->mButtonState = (ButtonComponent::ButtonState)1;
            return;
        }
    }

    if (!mDoingOutTransition)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL) == false)
        {
            return;
        }

        if (slide->m_time >= (slide->m_start + slide->m_duration))
        {
        }
        else
        {
            return;
        }

        if (mInputDelay <= 0.0)
        {
        }
        else
        {
            return;
        }

        mDoingOutTransition = true;
        m_pFEPresentation->SetActiveSlide("MENU IN2");

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("action");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        mWinnerAction->mImageInstance = findImage.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mWinnerAction->QueueLoad(WINNER_TEXTURES[mWinningTeam][0], false);

        g0.m_Hash = 0;
        h1.m_Hash = 0;
        g1.m_Hash = 0;
        h3.m_Hash = 0;
        g2.m_Hash = 0;
        h5.m_Hash = 0;
        g3.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("action_OUTLINE");
        g4.m_Hash = hash;
        g5.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        gB.m_Hash = hash;
        gA.m_Hash = hash;

        mWinnerActionOutline->mImageInstance = findImage.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)gB,
            (InlineHasher&)g5,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mWinnerActionOutline->QueueLoad(WINNER_TEXTURES[mWinningTeam][1], false);

        f0.m_Hash = 0;
        h1.m_Hash = 0;
        f1.m_Hash = 0;
        h3.m_Hash = 0;
        f2.m_Hash = 0;
        h5.m_Hash = 0;
        f3.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("action_WHITE");
        f4.m_Hash = hash;
        f5.m_Hash = hash;

        hash = nlStringLowerHash(WINNER_HANDLER_LAYER_NAME);
        fB.m_Hash = hash;
        fA.m_Hash = hash;

        mWinnerActionWhite->mImageInstance = findImage.byRef(
            m_pFEPresentation->m_currentSlide,
            (InlineHasher&)fB,
            (InlineHasher&)f5,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mWinnerActionWhite->QueueLoad(WINNER_TEXTURES[mWinningTeam][2], false);
    }
}
