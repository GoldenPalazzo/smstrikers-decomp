#include "Game/OverlayHandlerGoal.h"
#include "Game/FE/feNSNMessenger.h"
#include "Game/Game.h"
#include "Game/Goalie.h"
#include "NL/nlBundleFile.h"

struct InlineHasher
{
    InlineHasher() { }
    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
    unsigned long m_Hash;
};

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
};

template <typename To, typename From>
To LexicalCast(const From&);

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

void MakeTextBoxReallyWide(TLTextInstance&);
extern "C" void SetWinnerTitle__11GoalOverlayFv(GoalOverlay*);

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
 * Offset/Address/Size: 0x140 | 0x801036A0 | size: 0x15C
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0xBC | 0x8010361C | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x38 | 0x80103598 | size: 0x84
 */
// void FEFinder<TLTextInstance, 3>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
//{
// }

/**
 * Offset/Address/Size: 0x0 | 0x80103560 | size: 0x38
 */
// void FEFinder<TLTextInstance, 3>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
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
    : BaseOverlayHandler(2)
{
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
void GoalOverlay::UpdateGoalInfo(int, int, bool, int)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1590 | 0x80101600 | size: 0x418
 * TODO: 92.05% match - systematic register rotation by 1 (r28=this vs r27=this) and +0x0C stack offset shift, likely caused by volatile+union trick for InlineHasher by-ref calling under -inline deferred.
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

    SetWinnerTitle__11GoalOverlayFv(this);

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

    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = locString;
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
            *(unsigned short*)((char*)data->mData + j) = *locString;
            i++;
            locString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> format(data);

    int displayNumber = highlightNumber + 1;
    BasicString<char, Detail::TempStringAllocator> numberString(
        LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(displayNumber));
    unsigned short numberBuffer[16];
    nlStrToWcs(numberString.c_str(), numberBuffer, 16);

    BasicString<unsigned short, Detail::TempStringAllocator> formatted(
        Format(format, numberBuffer));

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);
    text->SetString(mDescriptionBuffer);
}

/**
 * Offset/Address/Size: 0xDA4 | 0x80100E14 | size: 0x7EC
 */
void GoalOverlay::DoMatchEndOverlay()
{
}

/**
 * Offset/Address/Size: 0x6E8 | 0x80100758 | size: 0x6BC
 */
void GoalOverlay::SetWinnerTitle()
{
}

/**
 * Offset/Address/Size: 0x20 | 0x80100090 | size: 0x6C8
 */
void GoalOverlay::DoCupWinOverlay()
{
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
