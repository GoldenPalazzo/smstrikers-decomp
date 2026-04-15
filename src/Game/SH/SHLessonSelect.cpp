#include "Game/SH/SHLessonSelect.h"
#include "Game/OverlayManager.h"
#include "Game/SH/SHLesson.h"
#include "Game/FE/feNSNMessenger.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/FEAudio.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/tlSlide.h"
#include "Game/FE/tlTextInstance.h"

#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/nlLexicalCast.h"
#include "NL/nlBasicString.h"

// /**
//  * Offset/Address/Size: 0x0 | 0x8010CE0C | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (LessonSelectScene::*)()>, LessonSelectScene*>(
//     Detail::MemFunImpl<void, void (LessonSelectScene::*)()>, LessonSelectScene* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010CDF0 | size: 0x1C
//  */
// void MemFun<LessonSelectScene, void>(void (LessonSelectScene::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010CD94 | size: 0x5C
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (LessonSelectScene::*)()>, LessonSelectScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x8010CC38 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x8010CBB4 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x8010CB7C | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                  InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x8010CA20 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x8010C99C | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x8010C964 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x8010C808 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x8010C784 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010C74C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x78 | 0x8010C71C | size: 0x30
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (LessonSelectScene::*)()>, LessonSelectScene*>>::operator()(TLComponentInstance*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8010C6A4 | size: 0x78
//  */
// void Function1<void, TLComponentInstance*>::FunctorImpl<
//     BindExp1<void, Detail::MemFunImpl<void, void (LessonSelectScene::*)()>, LessonSelectScene*>>::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x1830 | 0x8010C680 | size: 0x24
 */
void LessonTickerDoneCB()
{
    SetTickerLesson(-1);
}

/**
 * Offset/Address/Size: 0x1774 | 0x8010C5C4 | size: 0xBC
 */
LessonSelectScene::LessonSelectScene()
    : BaseSceneHandler()
    , mMenuItems()
    , mDoSlideIn(true)
    , mStartAnimAtEnd(false)
    , mButtons()
    , mUpArrow(NULL)
    , mDownArrow(NULL)
{
}

/**
 * Offset/Address/Size: 0x16D0 | 0x8010C520 | size: 0xA4
 */
LessonSelectScene::~LessonSelectScene()
{
}

/**
 * Offset/Address/Size: 0xE98 | 0x8010BCE8 | size: 0x838
 */
void LessonSelectScene::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x7E0 | 0x8010B630 | size: 0x6B8
 */
void LessonSelectScene::Update(float)
{
}

extern int sRowOffset;
extern int sCurrentRow;

/**
 * Offset/Address/Size: 0x334 | 0x8010B184 | size: 0x4AC
 * TODO: 96.1% match - stack frame 0x1B0 vs target 0x160; compiler does not reuse
 * InlineHasher arg copy stack slots across FEFinder::Find calls
 */
void LessonSelectScene::UpdateRow(int onScreenRow, bool playsound)
{
    if (!playsound)
    {
        FEAudio::EnableSounds(false);
    }

    int currentRow = onScreenRow + sRowOffset;
    FEPresentation* presentation = m_pFEPresentation;
    char menuname[64];
    nlSNPrintf(menuname, 64, "MENU ITEM%d", onScreenRow + 1);

    TLComponentInstance* pComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash(menuname)),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pComp->SetActiveSlide("IN");
    TLSlide* slide = pComp->GetActiveSlide();
    pComp->Update(slide->m_start + slide->m_duration);

    TLTextInstance* pText1 = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pComp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Text")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    TLTextInstance* pNameText1 = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pComp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("pauseresume")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    pComp->SetActiveSlide("OUT");
    slide = pComp->GetActiveSlide();
    pComp->Update(slide->m_start + slide->m_duration);

    TLTextInstance* pText2 = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pComp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("Text")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    TLTextInstance* pNameText2 = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        pComp->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("pauseresume")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (onScreenRow == sCurrentRow - sRowOffset)
    {
        pComp->SetActiveSlide("IN");
        if (!playsound)
        {
            DoubleHighlite::TempDisableSound();
        }
        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].ApplyAction(ON_HIGHLIGHT);
    }

    BasicString<char, Detail::TempStringAllocator> rowString = LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(currentRow + 1);

    unsigned short* numBuf = mNumberBuffers[onScreenRow];
    nlStrToWcs(rowString.c_str(), numBuf, 16);

    pText1->SetString(numBuf);
    pText2->SetString(numBuf);

    char lessonTitleName[64];
    nlSNPrintf(lessonTitleName, 64, "TUTORIAL_INSTRUCTION_TITLE_%d", currentRow + 1);
    pNameText1->SetStringId(lessonTitleName);
    pNameText2->SetStringId(lessonTitleName);

    if (!playsound)
    {
        FEAudio::EnableSounds(true);
    }
}

/**
 * Offset/Address/Size: 0x2E4 | 0x8010B134 | size: 0x50
 */
void LessonSelectScene::StartLesson()
{
    OverlayManager::Instance()->Push(IGSCENE_LESSON, SCREEN_FORWARD, true);
    LessonScene::SetLesson(sCurrentRow + 1);
    SetTickerLesson(sCurrentRow);
}

/**
 * Offset/Address/Size: 0x0 | 0x8010AE50 | size: 0x2E4
 * TODO: 99.59% scratch match - remaining i-diffs are SDA/string relocation offsets.
 */
void SetTickerLesson(int lesson)
{
    static unsigned char ResetHistory = 1;
    static signed char PreviousHistory[5];
    static signed char InsertPoint = 0;

    NSNMessengerScene* ticker;
    int randomlesson;
    char lessonTickerName[64];

    if (ResetHistory)
    {
        PreviousHistory[0] = -1;
        PreviousHistory[1] = -1;
        PreviousHistory[2] = -1;
        PreviousHistory[3] = -1;
        PreviousHistory[4] = -1;
        ResetHistory = 0;
    }

    BaseSceneHandler* scene = OverlayManager::s_pInstance->GetScene(OVERLAY_LESSON_TICKER);
    if (scene != 0)
    {
        scene = (BaseSceneHandler*)((char*)scene - 4);
    }

    ticker = (NSNMessengerScene*)(void*)scene;
    if (ticker == 0)
    {
        return;
    }

    if (lesson < 0)
    {
        do
        {
            randomlesson = nlRandom(12, &nlDefaultSeed);

            signed char* previous = PreviousHistory;
            for (int i = 0; i < 5; i++, previous++)
            {
                if (*previous == randomlesson)
                {
                    randomlesson = -1;
                    break;
                }
            }
        } while (randomlesson == -1);

        PreviousHistory[InsertPoint] = randomlesson;
        lesson = randomlesson;
        InsertPoint = (InsertPoint + 1) % 5;

        Function<FnVoidVoid> doneCB;
        doneCB.mTag = FREE_FUNCTION;
        doneCB.mFreeFunction = LessonTickerDoneCB;

        FEScrollText* scrollText = ticker->m_scrollText;
        if (scrollText != 0)
        {
            // *(Function<FnVoidVoid>*)&scrollText->mTag = doneCB;
            *(Function<FnVoidVoid>*)&scrollText->m_messageFinishedCB = doneCB;
        }
    }
    else
    {
        FEScrollText* scrollText = ticker->m_scrollText;
        if (scrollText != 0)
        {
            // Function<FnVoidVoid>* cb = (Function<FnVoidVoid>*)&scrollText->mTag;
            Function<FnVoidVoid>* cb = (Function<FnVoidVoid>*)&scrollText->m_messageFinishedCB;
            if (cb->mTag == FUNCTOR)
            {
                delete cb->mFunctor;
            }
            cb->mTag = EMPTY;
        }
    }

    nlSNPrintf(lessonTickerName, 64, "LOC_TUTORIAL_LESSON_%d", lesson + 1);
    ticker->EnableScrolling(true);
    ticker->SetDisplayMessage(lessonTickerName);
    ticker->OpenMessengerNow();
}
