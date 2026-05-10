#include "Game/SH/SHLesson.h"
#include "Game/SH/SHLessonSelect.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feInput.h"
#include "Game/FE/feManager.h"
#include "Game/FE/tlComponentInstance.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/OverlayManager.h"
#include "Game/SH/SHMoviePlayer.h"
#include "NL/nlPrint.h"
#include "NL/nlSingleton.h"
#include "types.h"

extern FEInput* g_pFEInput;

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

template <typename T, typename Key>
T* nlBSearch(const Key& key, T* pBase, int count);

struct InlineHasher
{
    unsigned long m_Hash;
    InlineHasher() { }
    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
};

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U* slide, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6);
};

static inline const unsigned short* LookupLocString(const char* stringId)
{
    nlLocalization* loc = g_pLocalization;
    unsigned long key = nlStringLowerHash(stringId);

    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }

    if (nlLocalization::StringLookup* entry = nlBSearch<nlLocalization::StringLookup, unsigned long>(key, loc->m_LookupTable, (int)loc->m_pFile->StringCount))
    {
        return loc->m_FirstString + entry->StringOffset;
    }

    return MissingLocString;
}

int LessonScene::mLessonIndex = -1;
// /**
//  * Offset/Address/Size: 0x2D4 | 0x8010ACF4 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x8010AC70 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x8010AC38 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x8010AADC | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8010AA58 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010AA20 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x508 | 0x8010A9B4 | size: 0x6C
 */
LessonScene::LessonScene()
    : mHUDScene(nullptr)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x490 | 0x8010A93C | size: 0x78
 */
LessonScene::~LessonScene()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x1DC | 0x8010A688 | size: 0x2B4
 */
void LessonScene::SceneCreated()
{
    char title[64];
    char body[64];
    TLTextInstance* titletextinstance;
    TLTextInstance* bodytextinstance;
    TLComponentInstance* buttonComponent;

    nlSNPrintf(title, 0x40, "LOC_TUTORIAL_INSTRUCTION_TITLE_%d", mLessonIndex);
    nlSNPrintf(body, 0x40, "LOC_TUTORIAL_INSTRUCTION_BODY_%d", mLessonIndex);

    titletextinstance = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("title")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    bodytextinstance = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("body")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (LookupLocString(title) != 0)
    {
        titletextinstance->SetStringId(title);
    }

    if (LookupLocString(body) != 0)
    {
        bodytextinstance->SetStringId(body);
    }

    buttonComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        m_pFEPresentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mButtons.mButtonInstance = buttonComponent;
    mButtons.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
    buttonComponent->m_bVisible = false;
}

/**
 * Offset/Address/Size: 0x8 | 0x8010A4B4 | size: 0x1D4
 */
void LessonScene::Update(float fDeltaT)
{
    MoviePlayerScene* scene;
    char filename[128];

    BaseSceneHandler::Update(fDeltaT);
    this->mButtons.CentreButtons();
    if (nlSingleton<OverlayManager>::s_pInstance->mCurrentStackDepth != 0)
    {
        scene = (MoviePlayerScene*)nlSingleton<OverlayManager>::s_pInstance->mBaseSceneHandlerStack[nlSingleton<OverlayManager>::s_pInstance->mCurrentStackDepth - 1];
    }
    else
    {
        scene = nullptr;
    }

    if (scene == (MoviePlayerScene*)this)
    {
        if (this->mButtons.mButtonInstance)
        {
            this->mButtons.mButtonInstance->m_bVisible = true;
        }
    }
    else
    {
        if (this->mButtons.mButtonInstance)
        {
            this->mButtons.mButtonInstance->m_bVisible = false;
        }
    }
    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, nullptr))
    {
        MoviePlayerScene* movieScene = (MoviePlayerScene*)nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_LESSON_MOVIE_PLAYER, SCREEN_FORWARD, false);
        nlSNPrintf(filename, 0x80, "movies/lesson%d.thp", mLessonIndex);
        movieScene->SetMovieDetails(filename, true, false);
        movieScene->mNextScene = IGSCENE_LESSON;
        movieScene->mPushWithPop = false;
        g_pFEInput->PushExclusiveInputLock(movieScene, -1);
        FEAudio::PlayAnimAudioEvent("sfx_accept", NULL);
        return;
    }
    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, nullptr))
    {
        LessonSelectScene* lessonScene = (LessonSelectScene*)nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_LESSON_SELECT, SCREEN_BACK, true);
        lessonScene->mStartAnimAtEnd = true;
        FEAudio::PlayAnimAudioEvent("sfx_back", NULL);
        return;
    }
    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x800, false, nullptr))
    {
        FrontEnd::ExitMenuState();
        FEAudio::PlayAnimAudioEvent("sfx_back", NULL);
        FEAudio::PlayAnimAudioEvent("sfx_screen_back", NULL);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8010A4AC | size: 0x8
 */
void LessonScene::SetLesson(int index)
{
    LessonScene::mLessonIndex = index;
}
