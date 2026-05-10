#include "Game/SH/SHMilestoneTrophy.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "NL/nlBSearch.h"
#include "NL/nlLocalization.h"

template <typename StringType>
class FormatImpl
{
public:
    StringType mString;
    int mCurrentPos;

    FormatImpl(BasicStringData<char>* data)
        : mString((BasicStringData<unsigned short>*)data)
        , mCurrentPos(0)
    {
    }

    operator StringType() const;

    template <typename T>
    FormatImpl& operator%(const T& t);
};

template <typename StringType, typename T1>
StringType Format(const StringType& format, const T1& value1);

template <typename StringType, typename T1, typename T2>
StringType Format(const StringType& format, const T1& value1, const T2& value2);

template <typename StringType, typename T1, typename T2, typename T3>
StringType Format(const StringType& format, const T1& value1, const T2& value2, const T3& value3);

/**
 * Offset/Address/Size: 0x1058 | 0x800D0DD8 | size: 0x118
 * TODO: 98.36% match - return copy null/data path still stores through r4 instead of reloading through r0.
 */
template <>
BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[128]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[128])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> > impl((BasicStringData<char>*)data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(impl % (const unsigned short*)value));
}

/**
 * Offset/Address/Size: 0xF44 | 0x800D0CC4 | size: 0x114
 * TODO: 98.33% match - return copy stores null/data via r4 instead of stack reload into r0.
 */
template <>
BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short* const& value)
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> > impl((BasicStringData<char>*)data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(impl % value));
}

/**
 * Offset/Address/Size: 0xE20 | 0x800D0BA0 | size: 0x124
 * TODO: 98.36% match - return copy null/data path still stores through r4 instead of reloading through r0.
 */
template <>
BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[128], unsigned short[128]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[128],
    const unsigned short (&value2)[128])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> > impl((BasicStringData<char>*)data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((impl % (const unsigned short*)value) % (const unsigned short*)value2));
}

// /**
//  * Offset/Address/Size: 0x130 | 0x800CFEB0 | size: 0xCF0
//  */
// void FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator>>::operator%<const unsigned short*>(const unsigned short* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CFD80 | size: 0x130
//  */
// void Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16], unsigned short[16], unsigned short[16]>(const
// BasicString<unsigned short, Detail::TempStringAllocator>&, const unsigned short(&)[16], const unsigned short(&)[16], const unsigned
// short(&)[16])
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800CFCC4 | size: 0xBC
 */
static void SHMilestoneTrophy_Append_stub()
{
    BasicString<char, Detail::TempStringAllocator> s;
    s.Append("");
}

// /**
//  * Offset/Address/Size: 0x5A8 | 0x800CFB68 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x524 | 0x800CFAE4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800CFAAC | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x390 | 0x800CF950 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x30C | 0x800CF8CC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800CF894 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800CF810 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800CF7D8 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800CF67C | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800CF5F8 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800CF5C0 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

struct TLTextInstance;
struct TLImageInstance;

struct TLTextLocView
{
    unsigned char mPad0[0x80];
    unsigned long mLocStrId;
    unsigned char mPad1[0x0C];
    unsigned long mOverloadFlags;
};

struct TLInstanceView
{
    unsigned char mPad0[0x0C];
    void* m_component;
    unsigned char mPad1[0x6E];
    bool m_bVisible;
};

extern unsigned long GetLOCTrophyName(eTrophyType);
extern "C" void SetString__14TLTextInstanceFPCUs(TLTextInstance*, const unsigned short*);
extern "C" const nlColour& GetColour__11FELibObjectCFv(void*);
extern "C" void SetAssetColour__10TLInstanceFRC8nlColour(void*, const nlColour&);
extern nlLocalization* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

static const char* MILESTONE_TROPHY_TEXT_NAME = "CUP TITLE";
static const char* MILESTONE_TROPHY_IMAGE_NAME = "TROPHY";
static const char* MILESTONE_TOTAL_TEXT_NAME = "TOTALS";
static const char* MILESTONE_STAT_TEXT_NAME = "THE TOTAL";
static const char* MILESTONE_DESCRIPTION_TEXT_NAME = "THE TOTAL2";

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
    "fe/trophies/cups_super_striker",
    "fe/trophies/cups_super_team",
    "fe/trophies/cups_lakitu",
};

static const nlColour TROPHY_BLACK_MILESTONE = { 0x00, 0x00, 0x00, 0xFF };

#define LOOKUP_LOC(_hash, _out)                                                                                                             \
    do                                                                                                                                      \
    {                                                                                                                                       \
        nlLocalization* _loc = g_pLocalization;                                                                                             \
        if (_loc->m_LookupTable == 0)                                                                                                       \
        {                                                                                                                                   \
            (_out) = LocalizationTableNotFound;                                                                                             \
        }                                                                                                                                   \
        else                                                                                                                                \
        {                                                                                                                                   \
            nlLocalization::StringLookup* _entry = nlBSearch((unsigned long)(_hash), _loc->m_LookupTable, (int)_loc->m_pFile->StringCount); \
            if (_entry != 0)                                                                                                                \
            {                                                                                                                               \
                (_out) = _loc->m_FirstString + _entry->StringOffset;                                                                        \
            }                                                                                                                               \
            else                                                                                                                            \
            {                                                                                                                               \
                (_out) = MissingLocString;                                                                                                  \
            }                                                                                                                               \
        }                                                                                                                                   \
    } while (0)

/**
 * Offset/Address/Size: 0x19F4 | 0x800CF4F4 | size: 0xCC
 */
MilestoneTrophyScene::MilestoneTrophyScene()
    : BaseSceneHandler()
    , mTrophy((eTrophyType)-1)
    , mCreated(false)
    , mIsNew(false)
    , mFirstSlideChange(true)
    , mDoBlockLoad(true)
    , mButtonState(ButtonComponent::BS_A_AND_B)
{
    const char* trophyImagePath = "art/fe/TrophiesUI.res";

    AsyncImage* pTrophyImage = new ((AsyncImage*)nlMalloc(0x1C, 0x20, true)) AsyncImage(trophyImagePath, NULL);
    mAsyncTrophy = pTrophyImage;
}

/**
 * Offset/Address/Size: 0x1950 | 0x800CF450 | size: 0xA4
 */
MilestoneTrophyScene::~MilestoneTrophyScene()
{
    delete mAsyncTrophy;
}

/**
 * Offset/Address/Size: 0x3CC | 0x800CDECC | size: 0x1584
 */
void MilestoneTrophyScene::SceneCreated()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompSlideByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompSlideByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLComponentInstance* (*FindCompPresByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompPresByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    int statAccumulated = 0;
    int statNeeded = 0;
    int bronzeStat = 0;
    int silverStat = 0;
    int goldStat = 0;

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

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(MILESTONE_TROPHY_TEXT_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
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

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(MILESTONE_TROPHY_IMAGE_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
        pTrophyImage = findImage.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    mAsyncTrophy->mImageInstance = pTrophyImage;
    ((TLInstanceView*)pTrophyImage)->m_bVisible = true;

    const unsigned short* locString;
    LOOKUP_LOC(0x3C479468, locString);

    BasicString<unsigned short, Detail::TempStringAllocator> unformatted(locString);
    BasicString<unsigned short, Detail::TempStringAllocator> stat;
    BasicString<unsigned short, Detail::TempStringAllocator> unlockable;
    BasicString<unsigned short, Detail::TempStringAllocator> description;
    BasicString<unsigned short, Detail::TempStringAllocator> formatted;

    switch (mTrophy)
    {
    case TROPHY_VETERAN_CUP:
        statAccumulated = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mNumGamesPlayed;
        bronzeStat = 25;
        silverStat = 50;
        goldStat = 100;

        LOOKUP_LOC(0x8A5D9314, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            stat = tmp;
        }

        LOOKUP_LOC(0x759DD858, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unlockable = tmp;
        }
        break;

    case TROPHY_SNIPER_CUP:
        statAccumulated = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mNumGoalsScored;
        bronzeStat = 75;
        silverStat = 150;
        goldStat = 300;

        LOOKUP_LOC(0x49772A70, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            stat = tmp;
        }

        LOOKUP_LOC(0xE3FB84B4, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unlockable = tmp;
        }
        break;

    case TROPHY_STRIKER_CUP:
        statAccumulated = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mNumSTSAttempts;
        bronzeStat = 25;
        silverStat = 50;
        goldStat = 100;

        LOOKUP_LOC(0x593E7EE3, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            stat = tmp;
        }

        LOOKUP_LOC(0x1D5A2367, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unlockable = tmp;
        }
        break;

    case TROPHY_TACTICIAN_CUP:
        statAccumulated = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mNumPerfectPasses;
        bronzeStat = 75;
        silverStat = 150;
        goldStat = 300;

        LOOKUP_LOC(0x243FB12F, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            stat = tmp;
        }

        LOOKUP_LOC(0x1F42DEB3, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unlockable = tmp;
        }
        break;

    case TROPHY_PARAMEDIC_CUP:
        statAccumulated = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mNumHits;
        bronzeStat = 250;
        silverStat = 500;
        goldStat = 1000;

        LOOKUP_LOC(0xD9A2F4C5, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            stat = tmp;
        }

        LOOKUP_LOC(0xAB6BFAC9, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unlockable = tmp;
        }
        break;
    }

    formatted = Format(unformatted, stat.c_str());

    memcpy(mTotalBuffer, formatted.c_str(), 0x100);

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

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(MILESTONE_TOTAL_TEXT_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
        TLTextInstance* pTotal = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        SetString__14TLTextInstanceFPCUs(pTotal, mTotalBuffer);
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

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
        TLComponentInstance* arrows = findComp.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        arrows->m_bVisible = false;
    }

    eMilestoneColour levelReached = nlSingleton<GameInfoManager>::s_pInstance->GetMilestoneLevel(mTrophy);
    BasicString<char, Detail::TempStringAllocator> fileName(TROPHY_TEXTURE_FILENAMES[(int)mTrophy]);
    if (levelReached == MILESTONE_BLACK)
    {
        mAsyncTrophy->QueueLoad(fileName.c_str(), mDoBlockLoad);
        SetAssetColour__10TLInstanceFRC8nlColour((void*)pTrophyImage, TROPHY_BLACK_MILESTONE);

        statNeeded = bronzeStat;
        LOOKUP_LOC(0x138E19E5, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unformatted = tmp;
        }
    }
    else if (levelReached == MILESTONE_BRONZE)
    {
        fileName = fileName.Append("_bronze");
        mAsyncTrophy->QueueLoad(fileName.c_str(), mDoBlockLoad);
        SetAssetColour__10TLInstanceFRC8nlColour((void*)pTrophyImage, GetColour__11FELibObjectCFv(((TLInstanceView*)pTrophyImage)->m_component));

        statNeeded = silverStat;
        LOOKUP_LOC(0x3A916A4A, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unformatted = tmp;
        }
    }
    else if (levelReached == MILESTONE_SILVER)
    {
        fileName = fileName.Append("_silver");
        mAsyncTrophy->QueueLoad(fileName.c_str(), mDoBlockLoad);
        SetAssetColour__10TLInstanceFRC8nlColour((void*)pTrophyImage, GetColour__11FELibObjectCFv(((TLInstanceView*)pTrophyImage)->m_component));

        statNeeded = goldStat;
        LOOKUP_LOC(0x0AD790FB, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unformatted = tmp;
        }
    }
    else
    {
        mAsyncTrophy->QueueLoad(fileName.c_str(), mDoBlockLoad);
        SetAssetColour__10TLInstanceFRC8nlColour((void*)pTrophyImage, GetColour__11FELibObjectCFv(((TLInstanceView*)pTrophyImage)->m_component));
    }
    mDoBlockLoad = false;

    if (nlSingleton<GameInfoManager>::s_pInstance->HasTrophy(mTrophy))
    {
        BasicString<char, Detail::TempStringAllocator> accumulatedString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(statAccumulated);

        unsigned short accumulatedWideString[128];
        nlStrToWcs(accumulatedString.c_str(), accumulatedWideString, 128);

        LOOKUP_LOC(0x59B161FF, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            unformatted = tmp;
        }

        formatted = Format(unformatted, accumulatedWideString);

        memcpy(mStatBuffer, formatted.c_str(), 0x100);

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

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(MILESTONE_STAT_TEXT_NAME);
            hNameA.m_Hash = hash;
            hNameB.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hLayerA.m_Hash = hash;
            hLayerB.m_Hash = hash;

            findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
            TLTextInstance* pStat = findText.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hLayerA,
                (InlineHasher&)hNameA,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);
            SetString__14TLTextInstanceFPCUs(pStat, mStatBuffer);
        }

        LOOKUP_LOC(0xF8710578, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            description = tmp;
        }

        formatted = Format(description, unlockable);
    }
    else
    {
        BasicString<char, Detail::TempStringAllocator> accumulatedString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(statAccumulated);
        BasicString<char, Detail::TempStringAllocator> neededString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(statNeeded);

        unsigned short accumulatedWideString[128];
        unsigned short neededWideString[128];
        nlStrToWcs(accumulatedString.c_str(), accumulatedWideString, 128);
        nlStrToWcs(neededString.c_str(), neededWideString, 128);

        formatted = Format(unformatted, accumulatedWideString, neededWideString);

        memcpy(mStatBuffer, formatted.c_str(), 0x100);

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

            h0.m_Hash = 0;
            h1.m_Hash = 0;
            h2.m_Hash = 0;
            h3.m_Hash = 0;
            h4.m_Hash = 0;
            h5.m_Hash = 0;
            h6.m_Hash = 0;
            h7.m_Hash = 0;

            unsigned long hash = nlStringLowerHash(MILESTONE_STAT_TEXT_NAME);
            hNameA.m_Hash = hash;
            hNameB.m_Hash = hash;

            hash = nlStringLowerHash("Layer");
            hLayerA.m_Hash = hash;
            hLayerB.m_Hash = hash;

            findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
            TLTextInstance* pStat = findText.byRef(
                presentation->m_currentSlide,
                (InlineHasher&)hLayerA,
                (InlineHasher&)hNameA,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);
            SetString__14TLTextInstanceFPCUs(pStat, mStatBuffer);
        }

        BasicString<char, Detail::TempStringAllocator> bronzeString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(bronzeStat);
        BasicString<char, Detail::TempStringAllocator> silverString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(silverStat);
        BasicString<char, Detail::TempStringAllocator> goldString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(goldStat);

        unsigned short bronzeWideString[16];
        unsigned short silverWideString[16];
        unsigned short goldWideString[16];
        nlStrToWcs(bronzeString.c_str(), bronzeWideString, 16);
        nlStrToWcs(silverString.c_str(), silverWideString, 16);
        nlStrToWcs(goldString.c_str(), goldWideString, 16);

        LOOKUP_LOC(0x42FDAE95, locString);
        {
            BasicString<unsigned short, Detail::TempStringAllocator> tmp(locString);
            description = tmp;
        }

        formatted = Format(description, bronzeWideString, silverWideString, goldWideString);
    }

    memcpy(mDescriptionBuffer, formatted.c_str(), 0x100);

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

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash(MILESTONE_DESCRIPTION_TEXT_NAME);
        hNameA.m_Hash = hash;
        hNameB.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hLayerA.m_Hash = hash;
        hLayerB.m_Hash = hash;

        findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
        TLTextInstance* pDescription = findText.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hLayerA,
            (InlineHasher&)hNameA,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        SetString__14TLTextInstanceFPCUs(pDescription, mDescriptionBuffer);
    }

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

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;
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

        hash = nlStringLowerHash("change");
        hInA.m_Hash = hash;
        hInB.m_Hash = hash;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<FEPresentation>;
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
}

/**
 * Offset/Address/Size: 0x188 | 0x800CDC88 | size: 0x244
 */
void MilestoneTrophyScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mButtons2.CentreButtons();
    mAsyncTrophy->Update(true);

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    bool canAccept = true;
    if (mButtonState != ButtonComponent::BS_A_AND_B && mButtonState != ButtonComponent::BS_A_ONLY)
    {
        canAccept = false;
    }

    bool canBack = true;
    if (mButtonState != ButtonComponent::BS_A_AND_B && mButtonState != ButtonComponent::BS_B_ONLY)
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
            nlSingleton<GameSceneManager>::s_pInstance->Pop();
            nlSingleton<GameInfoManager>::s_pInstance->DetermineNextCupScreen();
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            return;
        }
    }

    if (mIsNew == false && canBack)
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TROPHY_ROOM, SCREEN_BACK, true);
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
            return;
        }
    }

    if (mIsNew == false && g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0x0C, true, NULL))
    {
        if (mTrophy == TROPHY_PARAMEDIC_CUP)
        {
            mTrophy = TROPHY_VETERAN_CUP;
        }
        else
        {
            mTrophy = (eTrophyType)((int)mTrophy + 1);
        }

        ChangeSlides();
        FEAudio::PlayAnimAudioEvent("sfx_milestone_scroll_right", false);
        return;
    }

    if (mIsNew == false && g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0x0B, true, NULL))
    {
        if (mTrophy == TROPHY_VETERAN_CUP)
        {
            mTrophy = TROPHY_PARAMEDIC_CUP;
        }
        else
        {
            mTrophy = (eTrophyType)((int)mTrophy - 1);
        }

        ChangeSlides();
        FEAudio::PlayAnimAudioEvent("sfx_milestone_scroll_left", false);
    }
}

/**
 * Offset/Address/Size: 0x170 | 0x800CDC70 | size: 0x18
 */
void MilestoneTrophyScene::CreateTrophyScene(eTrophyType trophy, ButtonComponent::ButtonState state, bool isNew)
{
    mTrophy = trophy;
    mIsNew = isNew;
    mCreated = true;
    mButtonState = state;
}

/**
 * Offset/Address/Size: 0x0 | 0x800CDB00 | size: 0x170
 */
void MilestoneTrophyScene::ChangeSlides()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pres = m_pFEPresentation;
    f32 starTime;

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8;
    volatile unsigned long h7, h6, h5, h4, h3, h2, h1, h0;

    if (mFirstSlideChange)
    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;

        h0 = 0;
        h1 = 0;
        h2 = 0;
        h3 = 0;
        h4 = 0;
        h5 = 0;
        h6 = 0;
        h7 = 0;

        unsigned long hash = nlStringLowerHash("star rotation");
        h8 = hash;
        h9 = hash;

        hash = nlStringLowerHash("Layer");
        hB = hash;
        hA = hash;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
        TLSlide* slide = pres->m_currentSlide;
        TLComponentInstance* starComp = findComp.byRef(
            slide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
        starTime = starComp->GetActiveSlide()->m_time;
    }

    pres->SetActiveSlide("CHANGE");
    pres->Update(0.0f);

    if (mFirstSlideChange)
    {
        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;
        volatile unsigned long g7, g6;
        volatile unsigned long g5, g4, g3, g2, g1, g0;

        g0 = 0;
        h1 = 0;
        g1 = 0;
        h3 = 0;
        g2 = 0;
        h5 = 0;
        g3 = 0;
        h7 = 0;

        unsigned long hash = nlStringLowerHash("star rotation");
        g4 = hash;
        g5 = hash;

        hash = nlStringLowerHash("Layer");
        g7 = hash;
        g6 = hash;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
        TLSlide* slide = pres->m_currentSlide;
        TLComponentInstance* starComp = findComp.byRef(
            slide,
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
