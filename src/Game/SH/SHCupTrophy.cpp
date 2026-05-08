#include "Game/SH/SHCupTrophy.h"

#include "Game/FE/feFinder.h"
#include "Game/FE/tlComponentInstance.h"

#include "NL/nlFunktion.h"
#include "NL/nlMemory.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x800CDAC8 | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>(Detail::MemFunImpl<void, void
// (CupTrophyScene::*)()>,
//                                                                                        CupTrophyScene* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CDAAC | size: 0x1C
//  */
// void MemFun<CupTrophyScene, void>(void (CupTrophyScene::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0xFB4 | 0x800CD994 | size: 0x118
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32]>(
//     const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short (&)[32])
// {
// }

// /**
//  * Offset/Address/Size: 0xE5C | 0x800CD83C | size: 0x158
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16], unsigned short[16], unsigned short[16],
//             const unsigned short*, const unsigned short*>(const BasicString<unsigned short, Detail::TempStringAllocator>&,
//                                                           const unsigned short (&)[16], const unsigned short (&)[16],
//                                                           const unsigned short (&)[16], const unsigned short* const&,
//                                                           const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x16C | 0x800CCB4C | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator% <const unsigned short*>(const unsigned short*
// const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CC9E0 | size: 0x16C
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16], unsigned short[16], unsigned short[16],
//             const unsigned short*, const unsigned short*, const unsigned short*>(
//     const BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short (&)[16], const unsigned short (&)[16],
//     const unsigned short (&)[16], const unsigned short* const&, const unsigned short* const&, const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CC984 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CC8DC | size: 0x78
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupTrophyScene::*)()>, CupTrophyScene*>>::Clone() const
// -- moved after type definitions below

// /**
//  * Offset/Address/Size: 0xBC | 0x800CC6F8 | size: 0x1E4
//  */
// void BasicString<char, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(
//     const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CC63C | size: 0xBC
//  */
// void BasicString<char, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(
//     const BasicString<char, Detail::TempStringAllocator>&) const
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800CC5B8 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                              unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800CC580 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                             InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800CC424 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800CC3A0 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800CC368 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800CC20C | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800CC188 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800CC150 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                  InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800CBFF4 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800CBF70 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CBF38 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x27B0 | 0x800CBE64 | size: 0xD4
 */
CupTrophyScene::CupTrophyScene()
    : mTrophy((eTrophyType)-1)
    , mCreated(false)
    , mIsNew(false)
    , mFirstSlideChange(true)
    , mButtonState((ButtonComponent::ButtonState)(*(int*)&unk_gap[121] = 0, *(int*)&unk_gap[123] = 0,
          *(bool*)&unk_gap[127] = true, 0))
{
    CupTrophyScene* self = this;
    const char* asyncPath = "art/fe/TrophiesUI.res";
    AsyncImage* image = new (nlMalloc(0x1C, 0x20, true)) AsyncImage(asyncPath, NULL);
    *(AsyncImage**)&self->unk_gap[125] = image;
}

/**
 * Offset/Address/Size: 0x270C | 0x800CBDC0 | size: 0xA4
 */
CupTrophyScene::~CupTrophyScene()
{
    delete *(AsyncImage**)&unk_gap[125];
}

static const char* CUP_TROPHY_TEXT_NAME = "CUP TITLE";
static const char* CUP_TROPHY_IMAGE_NAME = "Trophy";
static const char* CUP_TROPHY_TOTAL_NAME = "TOTAL CUPS";

static const char* TROPHY_TEXTURE_FILENAMES[13] = {
    "fe/trophies/cups_mushroom",
    "fe/trophies/cups_flower",
    "fe/trophies/cups_star",
    "fe/trophies/cups_bowser",
    "fe/trophies/cups_super_mushroom",
    "fe/trophies/cups_super_flower",
    "fe/trophies/cups_super_star",
    "fe/trophies/cups_super_bowser",
    "fe/trophies/cups_veteran",
    "fe/trophies/cups_sniper",
    "fe/trophies/cups_striker",
    "fe/trophies/cups_super_team",
    "fe/trophies/cups_lakitu",
};

static const nlColour TROPHY_BLACK_CUP = { 0x00, 0x00, 0x00, 0xFF };

struct SpoilNumCupWinsView
{
    unsigned char mPad[0x20E];
    unsigned short mNumCupWins;
};

extern unsigned long GetLOCTrophyName(eTrophyType);
extern "C" bool HasTrophy__15GameInfoManagerCF11eTrophyType(const void*, eTrophyType);

struct TLTextLocView
{
    unsigned char mPad0[0x80];
    unsigned long mLocStrId;
    unsigned char mPad1[0x0C];
    unsigned long mOverloadFlags;
};

class GameInfoManager;

struct CupTrophySpoilView
{
    unsigned char mPad[0x208];
    unsigned char mNumRecords;
    unsigned char mTailPad[0x0F];
};

template <typename To, typename From>
To LexicalCast(const From&);

template <typename StringType, typename ValueType>
StringType Format(const StringType& format, const ValueType& value);

template <typename T, typename Key>
T* nlBSearch(const Key& key, T* base, int count);

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

class TLTextInstance
{
public:
    void SetString(const unsigned short* utf16);
};

extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

/**
 * Offset/Address/Size: 0x1F54 | 0x800CB608 | size: 0x7B8
 * TODO: 75.83% match - stack/register allocation drift in finder/localization setup and text string materialization.
 */
void CupTrophyScene::SceneCreated()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompSlideByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompSlideByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompPresByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompPresByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    *(int*)&unk_gap[121] = 0;
    *(int*)&unk_gap[123] = 0;

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    char* gameInfoBase = (char*)nlSingleton<GameInfoManager>::s_pInstance;
    gameInfoBase += mTrophy * 0x218;
    Spoil* pSpoil = (Spoil*)(gameInfoBase + 0x2F24);

    TLTextInstance* pText;
    {
        union
        {
            FindTextByValue byValue;
            FindTextByRef byRef;
        } findText;

        volatile InlineHasher hLayerA, hLayerB;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher h7, h6;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(CUP_TROPHY_TEXT_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        pText = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    ((TLTextLocView*)pText)->mLocStrId = GetLOCTrophyName(mTrophy);
    ((TLTextLocView*)pText)->mOverloadFlags |= 0x8;

    TLImageInstance* pTrophyImage;
    {
        union
        {
            FindImageByValue byValue;
            FindImageByRef byRef;
        } findImage;

        volatile InlineHasher hLayerA, hLayerB;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher h7, h6;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(CUP_TROPHY_IMAGE_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        pTrophyImage = findImage.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    (*(AsyncImage**)&unk_gap[125])->mImageInstance = pTrophyImage;
    (*(AsyncImage**)&unk_gap[125])->QueueLoad(TROPHY_TEXTURE_FILENAMES[(int)mTrophy], *(bool*)&unk_gap[127]);
    *(bool*)&unk_gap[127] = false;

    {
        union
        {
            FindCompSlideByValue byValue;
            FindCompSlideByRef byRef;
        } findComp;

        volatile InlineHasher hLayerA, hLayerB;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher h7, h6;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("ARROWS2");
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        TLComponentInstance* arrowComp = findComp.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        arrowComp->m_bVisible = ((CupTrophySpoilView*)pSpoil)->mNumRecords > 3;
    }

    if (mIsNew)
    {
        union
        {
            FindCompSlideByValue byValue;
            FindCompSlideByRef byRef;
        } findComp;

        volatile InlineHasher hLayerA, hLayerB;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher h7, h6;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("ARROWS");
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        TLComponentInstance* pComp = findComp.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        pComp->m_bVisible = false;
        ((unsigned char*)nlSingleton<GameInfoManager>::s_pInstance)[0x4C39] = 0;
    }

    if (HasTrophy__15GameInfoManagerCF11eTrophyType(nlSingleton<GameInfoManager>::s_pInstance, mTrophy))
    {
        nlColour trophyColour = ((FELibObject*)pTrophyImage->m_component)->GetColour();
        pTrophyImage->SetAssetColour(trophyColour);
    }
    else
    {
        pTrophyImage->SetAssetColour(TROPHY_BLACK_CUP);
    }

    SetWinRecord(*pSpoil);
    SetLossRecord(*pSpoil);
    SetHistory(*pSpoil);

    if (!mButtons.mAlreadyCentred)
    {
        union
        {
            FindCompPresByValue byValue;
            FindCompPresByRef byRef;
        } findComp;

        volatile InlineHasher hInB, hInA;
        volatile InlineHasher hLayerB, hLayerA;
        volatile InlineHasher hButtonsB, hButtonsA;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("buttons");
        hButtonsA.m_Hash = hash;
        hButtonsB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        hash = nlStringLowerHash("IN");
        hInA.m_Hash = hash;
        hInB.m_Hash = hash;

        mButtons.mButtonInstance = findComp.byRef(
            presentation,
            (InlineHasher&)hInA,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hButtonsA,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mButtons.SetState(mButtonState);
    }

    if (!mButtons2.mAlreadyCentred)
    {
        union
        {
            FindCompPresByValue byValue;
            FindCompPresByRef byRef;
        } findComp;

        volatile InlineHasher hInB, hInA;
        volatile InlineHasher hLayerB, hLayerA;
        volatile InlineHasher hButtonsB, hButtonsA;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("buttons");
        hButtonsA.m_Hash = hash;
        hButtonsB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        hash = nlStringLowerHash("IN2");
        hInA.m_Hash = hash;
        hInB.m_Hash = hash;

        mButtons2.mButtonInstance = findComp.byRef(
            presentation,
            (InlineHasher&)hInA,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hButtonsA,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        mButtons2.SetState(mButtonState);
    }

    BasicString<char, Detail::TempStringAllocator> timesWon = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>((int)((SpoilNumCupWinsView&)(*pSpoil)).mNumCupWins);

    unsigned short timesWonWide[32];
    nlStrToWcs(timesWon.c_str(), timesWonWide, 32);

    unsigned long locHash = 0x103642ED;
    nlLocalization* loc = g_pLocalization;
    const unsigned short* locString;

    if (loc->m_LookupTable == 0)
    {
        locString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch(locHash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry != 0)
        {
            locString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            locString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = locString;
        while (*ptr++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *locString;
            i++;
            locString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(data);
    BasicString<unsigned short, Detail::TempStringAllocator> formatted = Format(unformatted, timesWonWide);

    memcpy((void*)((char*)this + 0x62C), formatted.c_str(), 0x100);

    {
        union
        {
            FindTextByValue byValue;
            FindTextByRef byRef;
        } findText;

        volatile InlineHasher hLayerA, hLayerB;
        volatile InlineHasher hNameB, hNameA;
        volatile InlineHasher h7, h6;
        volatile InlineHasher h5, h4, h3, h2, h1, h0;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(CUP_TROPHY_TOTAL_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        TLTextInstance* pTotal = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        pTotal->SetString((unsigned short*)((char*)this + 0x62C));
    }
}

class GameInfoManager
{
public:
    unsigned char _pad[0x4C58];
    unsigned int mUnlockedTriggers;
};

class GameSceneManager
{
public:
    virtual ~GameSceneManager();
    virtual void* Push(int, int, bool);
};

enum ePopupMenu
{
    POPUP_UNLOCKED_FLOWER_CUP = 32,
    POPUP_UNLOCKED_STAR_CUP = 33,
    POPUP_UNLOCKED_BOWSER_CUP = 34,
    POPUP_UNLOCKED_SUPER_CUPS = 35,
    POPUP_UNLOCKED_KONGA_STADIUM = 37,
    POPUP_UNLOCKED_YOSHI_STADIUM = 38,
    POPUP_UNLOCKED_FORBIDDEN_STADIUM = 39,
    POPUP_UNLOCKED_SUPER_STADIUM = 40,
    POPUP_UNLOCKED_LEGEND_DIFFICULTY = 41,
    POPUP_UNLOCKED_SUPER_TEAM = 42,
    POPUP_UNLOCKED_CHEAT_GOALIE = 43,
    POPUP_UNLOCKED_CHEAT_INFINITE = 44,
    POPUP_UNLOCKED_CHEAT_TILT = 45,
    POPUP_UNLOCKED_ALL_STS = 46,
    NUM_POPUP_MENUS = 47,
};

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*)());

class FEPopupMenu
{
public:
    void Create(ePopupMenu, Function<FnVoidVoid>&);
};

typedef Detail::MemFunImpl<void, void (CupTrophyScene::*)()> MemFunImpl_CupTrophyScene_v;
typedef BindExp1<void, MemFunImpl_CupTrophyScene_v, CupTrophyScene*> BindExp1_vfmfcp;
typedef Function0<void>::FunctorImpl<BindExp1_vfmfcp> FunctorImpl_vfmfcp;

/**
 * Offset/Address/Size: 0x78 | 0x800CC954 | size: 0x30
 */
template <>
void Function0<void>::FunctorImpl<BindExp1_vfmfcp>::operator()()
{
    (mBind.mArg->*(mBind.mFuncPtr.mMemFun))();
}

/**
 * Offset/Address/Size: 0x1D70 | 0x800CB424 | size: 0x1E4
 * TODO: 99.88% match - only minor instruction/label diffs remain around PopupMap/Create relocation.
 */
void CupTrophyScene::HandleUnlockedTriggers()
{
    static const ePopupMenu PopupMap[] = {
        POPUP_UNLOCKED_KONGA_STADIUM,
        POPUP_UNLOCKED_YOSHI_STADIUM,
        POPUP_UNLOCKED_FORBIDDEN_STADIUM,
        POPUP_UNLOCKED_SUPER_CUPS,
        POPUP_UNLOCKED_SUPER_TEAM,
        POPUP_UNLOCKED_LEGEND_DIFFICULTY,
        POPUP_UNLOCKED_SUPER_STADIUM,
        POPUP_UNLOCKED_ALL_STS,
        POPUP_UNLOCKED_CHEAT_TILT,
        POPUP_UNLOCKED_CHEAT_INFINITE,
        POPUP_UNLOCKED_FLOWER_CUP,
        POPUP_UNLOCKED_STAR_CUP,
        POPUP_UNLOCKED_BOWSER_CUP,
        POPUP_UNLOCKED_CHEAT_GOALIE,
        NUM_POPUP_MENUS,
    };

    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    int i = 0;
    FEPopupMenu* popup;

    while (i < 0x20)
    {
        if ((gameInfo->mUnlockedTriggers & (1 << i)) != 0)
        {
            popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(0x1B, 0, false);
            BindExp1_vfmfcp bind = Bind<void, MemFunImpl_CupTrophyScene_v, CupTrophyScene*>(
                MemFun<CupTrophyScene, void>(&CupTrophyScene::HandleUnlockedTriggers), this);

            {
                Function<FnVoidVoid> callback;
                callback.mTag = FUNCTOR;

                FunctorImpl_vfmfcp* functor = new ((FunctorImpl_vfmfcp*)nlMalloc(sizeof(FunctorImpl_vfmfcp), 8, false))
                    FunctorImpl_vfmfcp(bind);

                callback.mFunctor = functor;
                popup->Create(PopupMap[i], callback);
            }

            gameInfo->mUnlockedTriggers &= ((unsigned int)-2 << i);
            return;
        }

        i++;
    }

    nlSingleton<GameSceneManager>::s_pInstance->Push(0x2E, 0, true);
}

/**
 * Offset/Address/Size: 0x1970 | 0x800CB024 | size: 0x400
 */
enum eFEINPUT_PAD
{
    FE_ALL_PADS = 8,
};

struct FEInput
{
    bool JustPressed(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
    bool IsAutoPressed(eFEINPUT_PAD, int, bool, eFEINPUT_PAD*);
};

extern FEInput* g_pFEInput;
extern "C" void HandleUnlockedTriggers__14CupTrophySceneFv(CupTrophyScene*);

void CupTrophyScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mButtons2.CentreButtons();
    (*(AsyncImage**)&unk_gap[125])->Update(true);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    char* pBase = (char*)nlSingleton<GameInfoManager>::s_pInstance;
    bool canAccept = true;
    int buttonState = mButtonState;
    pBase += mTrophy * 0x218;
    Spoil* pSpoil = (Spoil*)(pBase + 0x2F24);

    if (buttonState != ButtonComponent::BS_A_AND_B && buttonState != ButtonComponent::BS_A_ONLY)
    {
        canAccept = false;
    }

    bool canBack = true;
    if (buttonState != ButtonComponent::BS_A_AND_B && buttonState != ButtonComponent::BS_B_ONLY)
    {
        canBack = false;
    }

    TLSlide* slide = presentation->m_currentSlide;
    if (presentation->m_fadeDuration < slide->m_start + slide->m_duration)
    {
        return;
    }

    if (mIsNew == true && canAccept)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            if (nlSingleton<GameInfoManager>::s_pInstance->mUnlockedTriggers != 0)
            {
                HandleUnlockedTriggers__14CupTrophySceneFv(this);
            }
            else
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(0x2E, 1, true);
            }

            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            return;
        }
    }

    if (mIsNew == false && canBack)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(0x1C, 2, true);
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
            return;
        }
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        if (((CupTrophySpoilView*)pSpoil)->mNumRecords > 3)
        {
            char* pBase2 = (char*)nlSingleton<GameInfoManager>::s_pInstance;
            pBase2 += mTrophy * 0x218;
            Spoil* pSpoil = (Spoil*)(pBase2 + 0x2F24);
            unsigned char numRecords = ((CupTrophySpoilView*)pSpoil)->mNumRecords;

            if (numRecords > 1)
            {
                if (mRow < 2 && mRow < (int)(numRecords - 1))
                {
                    mRow += 1;
                    FEAudio::PlayAnimAudioEvent("sfx_option_scroll_down", false);
                }
                else if (numRecords > 3 && mScrollOffset < (int)(numRecords - 3))
                {
                    mScrollOffset += 1;
                    FEAudio::PlayAnimAudioEvent("sfx_option_scroll_down", false);
                }
                else
                {
                    FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                }

                SetHistory(*pSpoil);
                return;
            }
        }
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        unsigned char numRecords = ((CupTrophySpoilView*)pSpoil)->mNumRecords;
        if (numRecords > 3)
        {
            if (numRecords > 1)
            {
                if (mRow > 0)
                {
                    mRow -= 1;
                    FEAudio::PlayAnimAudioEvent("sfx_option_scroll_up", false);
                }
                else if (mScrollOffset > 0)
                {
                    mScrollOffset -= 1;
                    FEAudio::PlayAnimAudioEvent("sfx_option_scroll_up", false);
                }
                else
                {
                    FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                }

                SetHistory(*pSpoil);
                return;
            }
        }
    }

    if (mIsNew == false)
    {
        if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xC, true, NULL))
        {
            if (mTrophy == mLastTrophy)
            {
                mTrophy = mFirstTrophy;
            }
            else
            {
                mTrophy = (eTrophyType)((int)mTrophy + 1);
            }

            ChangeSlides();
            FEAudio::PlayAnimAudioEvent("sfx_cup_toggle_right", false);
            return;
        }
    }

    if (mIsNew == false)
    {
        if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xB, true, NULL))
        {
            if (mTrophy == mFirstTrophy)
            {
                mTrophy = mLastTrophy;
            }
            else
            {
                mTrophy = (eTrophyType)((int)mTrophy - 1);
            }

            ChangeSlides();
            FEAudio::PlayAnimAudioEvent("sfx_cup_toggle_left", false);
        }
    }
}

/**
 * Offset/Address/Size: 0x191C | 0x800CAFD0 | size: 0x54
 */
void CupTrophyScene::CreateTrophyScene(eTrophyType trophy, ButtonComponent::ButtonState buttonState, bool isNew)
{
    mTrophy = trophy;
    mIsNew = isNew;
    mCreated = 1;

    if (!mIsNew)
    {
        if (mTrophy < 4)
        {
            mFirstTrophy = TROPHY_MUSHROOM_CUP;
            mLastTrophy = TROPHY_BOWSER_CUP;
        }
        else
        {
            mFirstTrophy = TROPHY_SUPER_MUSHROOM_CUP;
            mLastTrophy = TROPHY_SUPER_BOWSER_CUP;
        }
    }

    mButtonState = buttonState;
}

struct SpoilNumWinsView
{
    unsigned char mPad[0x20A];
    unsigned short mNumWins;
};

struct SpoilNumLossesView
{
    unsigned char mPad[0x20C];
    unsigned short mNumLosses;
};

static const char* CUP_FIRST_TEXT_NAME_LEFT = "FIRST WON TIME";

/**
 * Offset/Address/Size: 0x1524 | 0x800CABD8 | size: 0x3F8
 * TODO: 94.57% match - register allocation (this=r28/r30, locString=r31/r28) and stack offset
 * shift are -inline deferred artifacts; file compiled with -inline deferred but scratch uses -inline auto
 */
void CupTrophyScene::SetWinRecord(Spoil& spoil)
{
    typedef TLTextInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pres = m_pFEPresentation;
    BasicString<char, Detail::TempStringAllocator> winString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(((SpoilNumWinsView&)spoil).mNumWins);
    unsigned short winBuf[16];
    nlStrToWcs(winString.c_str(), winBuf, 16);

    unsigned long locHash = 0x49466bc6;
    nlLocalization* loc = g_pLocalization;
    const unsigned short* locString;

    if (loc->m_LookupTable == 0)
    {
        locString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch(locHash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry != 0)
        {
            locString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            locString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = locString;
        while (*ptr++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *locString;
            i++;
            locString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> msg(data);
    BasicString<unsigned short, Detail::TempStringAllocator> formattedResult = Format(msg, winBuf);

    memcpy(mFirstWinBuffer, formattedResult.c_str(), 0x100);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hLayerA, hLayerB;
    volatile InlineHasher hNameB, hNameA;
    volatile InlineHasher h7, h6;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    hash = nlStringLowerHash(CUP_FIRST_TEXT_NAME_LEFT);
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerB.m_Hash = hash;
    hLayerA.m_Hash = hash;

    TLTextInstance* text = findComp.byRef(
        pres->m_currentSlide,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hNameB,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    text->SetString(mFirstWinBuffer);
}

static const char* CUP_FIRST_TEXT_NAME_RIGHT = "FIRST WON TIME2";

/**
 * Offset/Address/Size: 0x112C | 0x800CA7E0 | size: 0x3F8
 * TODO: 93.04% match - register allocation (this=r28/r30, locString=r31/r28) and stack offset
 * shift are -inline deferred artifacts; file compiled with -inline deferred but scratch uses -inline auto
 */
void CupTrophyScene::SetLossRecord(Spoil& spoil)
{
    typedef TLTextInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pres = m_pFEPresentation;
    BasicString<char, Detail::TempStringAllocator> lossString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(((SpoilNumLossesView&)spoil).mNumLosses);
    unsigned short lossBuf[16];
    nlStrToWcs(lossString.c_str(), lossBuf, 16);

    unsigned long locHash = 0x720DF6F9;
    nlLocalization* loc = g_pLocalization;
    const unsigned short* locString;

    if (loc->m_LookupTable == 0)
    {
        locString = LocalizationTableNotFound;
    }
    else
    {
        nlLocalization::StringLookup* entry = nlBSearch(locHash, loc->m_LookupTable, (int)loc->m_pFile->StringCount);
        if (entry != 0)
        {
            locString = loc->m_FirstString + entry->StringOffset;
        }
        else
        {
            locString = MissingLocString;
        }
    }

    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = locString;
        while (*ptr++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (unsigned short*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)((char*)data->mData + j) = *locString;
            i++;
            locString++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> msg(data);
    BasicString<unsigned short, Detail::TempStringAllocator> formattedResult = Format(msg, lossBuf);

    memcpy(mHistoryBuffer, formattedResult.c_str(), 0x100);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hLayerA, hLayerB;
    volatile InlineHasher hNameB, hNameA;
    volatile InlineHasher h7, h6;
    volatile InlineHasher h5, h4, h3, h2, h1, h0;

    unsigned long hash;

    findComp.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    hash = nlStringLowerHash(CUP_FIRST_TEXT_NAME_RIGHT);
    hNameA.m_Hash = hash;
    hNameB.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    hLayerA.m_Hash = hash;
    hLayerB.m_Hash = hash;

    TLTextInstance* text = findComp.byRef(
        pres->m_currentSlide,
        (InlineHasher&)hLayerB,
        (InlineHasher&)hNameB,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    text->SetString(mHistoryBuffer);
}

/**
 * Offset/Address/Size: 0x170 | 0x800C9824 | size: 0xFBC
 */
void CupTrophyScene::SetHistory(Spoil&)
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800C96B4 | size: 0x170
 */
void CupTrophyScene::ChangeSlides()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pres = m_pFEPresentation;
    float starTime;
    unsigned long hash;
    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    if (mFirstSlideChange)
    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("star rotation");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        TLComponentInstance* starComp = findComp.byRef(
            pres->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        starTime = starComp->GetActiveSlide()->m_time;
    }

    pres->SetActiveSlide("IN2");
    pres->Update(0.0f);

    if (mFirstSlideChange)
    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;
        volatile InlineHasher g7, g6;
        volatile InlineHasher g5, g4, g3, g2, g1, g0;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        g0.m_Hash = 0;
        h1.m_Hash = 0;
        g1.m_Hash = 0;
        h3.m_Hash = 0;
        g2.m_Hash = 0;
        h5.m_Hash = 0;
        g3.m_Hash = 0;
        h7.m_Hash = 0;

        hash = nlStringLowerHash("star rotation");
        g4.m_Hash = hash;
        g5.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        g7.m_Hash = hash;
        g6.m_Hash = hash;

        TLComponentInstance* starComp = findComp.byRef(
            pres->m_currentSlide,
            (InlineHasher&)g7,
            (InlineHasher&)g5,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        starComp->Update(starTime);
        mFirstSlideChange = false;
    }

    SceneCreated();
}
