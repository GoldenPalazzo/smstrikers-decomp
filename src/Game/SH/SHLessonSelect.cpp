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

extern int sRowOffset;
extern int sCurrentRow;

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
    FEAudio::EnableSounds(false);

    FEPresentation* presentation = m_pFEPresentation;

    typedef Detail::MemFunImpl<void, void (LessonSelectScene::*)()> MemFunImpl_LessonSelect_t;
    typedef BindExp1<void, MemFunImpl_LessonSelect_t, LessonSelectScene*> BindExp1_LessonSelect_t;
    typedef Function1<void, TLComponentInstance*>::FunctorImpl<BindExp1_LessonSelect_t> FunctorImpl_LessonSelect_t;

    for (int i = 0; i < 4; i++)
    {
        char menuname[64];
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 1);

        TLComponentInstance* compinstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        compinstance->SetActiveSlide(i == 0 ? "in" : "out");

        if (mDoSlideIn)
        {
            MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
            menuItem->mType = compinstance;
            mMenuItems.mNumItemsAdded++;

            {
                Function<TLComponentInstance*> openFunction;
                openFunction.mTag = FREE_FUNCTION;
                openFunction.mFreeFunction = DoubleHighlite::OpenItem;
                menuItem->mCallbacks[ON_HIGHLIGHT] = openFunction;
            }

            {
                Function<TLComponentInstance*> closeFunction;
                closeFunction.mTag = FREE_FUNCTION;
                closeFunction.mFreeFunction = DoubleHighlite::CloseItem;
                menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunction;
            }

            {
                BindExp1_LessonSelect_t bind = Bind<void, MemFunImpl_LessonSelect_t, LessonSelectScene*>(
                    MemFun<LessonSelectScene, void>(&LessonSelectScene::StartLesson), this);

                FunctorImpl_LessonSelect_t* impl = new ((FunctorImpl_LessonSelect_t*)nlMalloc(sizeof(FunctorImpl_LessonSelect_t), 8, false))
                    FunctorImpl_LessonSelect_t(bind);

                Function<TLComponentInstance*> applyFunction;
                applyFunction.mTag = FUNCTOR;
                applyFunction.mFunctor = impl;
                menuItem->mCallbacks[ON_APPLY] = applyFunction;
            }

            if (i == 0)
            {
                DoubleHighlite::TempDisableSound();
            }

            menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
        }

        if (i == sCurrentRow)
        {
            DoubleHighlite::TempDisableSound();
            DoubleHighlite::OpenItem(compinstance);
        }
        else
        {
            DoubleHighlite::CloseItem(compinstance);
        }

        TLSlide* slide = compinstance->GetActiveSlide();
        compinstance->Update(slide->m_start + slide->m_duration);
    }

    DoubleHighlite::TempDisableSound();

    mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].ApplyAction(ON_UNHIGHLIGHT);
    mMenuItems.mCurrentIndex = sCurrentRow - sRowOffset;
    mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].ApplyAction(ON_HIGHLIGHT);

    for (int i = 0; i < 4; i++)
    {
        UpdateRow(i, false);
    }

    TLComponentInstance* arrows = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("ARROWS")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mUpArrow = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        arrows->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("arrow")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mDownArrow = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
        arrows->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("arrow2")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    if (sCurrentRow == 0)
    {
        mUpArrow->m_bVisible = false;
        mDownArrow->m_bVisible = true;
    }
    else if (sCurrentRow == 11)
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = false;
    }
    else
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = true;
    }

    mButtons.mButtonInstance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));

    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    FEAudio::EnableSounds(true);
}

extern int sRowOffset;
extern int sCurrentRow;

/**
 * Offset/Address/Size: 0x7E0 | 0x8010B630 | size: 0x6B8
 * TODO: 90.70% match - callback tag checks and up/down navigation branch layout
 * differ; back action scene-start-animation byte store is still missing.
 */
void LessonSelectScene::Update(float fDeltaT)
{
    if (mStartAnimAtEnd)
    {
        m_pFEPresentation->m_fadeDuration = 999.9f;
        mStartAnimAtEnd = false;
    }

    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_APPLY].mTag;
        if (tag)
        {
            if (!mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mDisabled)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_APPLY].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_APPLY].mFunctor)(type);
                }
            }
        }

        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_STRIKERS_101_PAUSE, SCREEN_BACK, true);
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
        sRowOffset = 0;
        sCurrentRow = 0;
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        FEAudio::EnableSounds(false);

        int flags = mMenuItems.mFlags;
        int currentIndex = mMenuItems.mCurrentIndex;
        int wrapBit = flags & 1;
        int skipDisabled = flags & 2;
        int result = currentIndex - 1;

        while (true)
        {
            if (wrapBit)
            {
                if (result < 0)
                {
                    result = mMenuItems.mNumItemsAdded - 1;
                }
            }
            else
            {
                if (result < 0)
                {
                    result = RES_NOT_CHANGED;
                    break;
                }
            }

            if (skipDisabled)
            {
                if (mMenuItems.mMenuItems[result].mDisabled)
                {
                    result = result - 1;
                    continue;
                }
            }

            {
                int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mFunctor)(type);
                    }
                }
            }

            mMenuItems.mCurrentIndex = result;

            {
                int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                    }
                }
            }

            result = RES_OK;
            break;
        }

        FEAudio::EnableSounds(true);

        sCurrentRow = sRowOffset + mMenuItems.mCurrentIndex;
        bool updatearrows = true;

        if (result == RES_NOT_CHANGED && sRowOffset > 0)
        {
            sRowOffset = sRowOffset - 1;
            sCurrentRow = sRowOffset + mMenuItems.mCurrentIndex;

            int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
            if (tag)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                }
            }
        }
        else
        {
            if (result == RES_NOT_CHANGED)
            {
                updatearrows = false;
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            }
            else if (result == RES_OK)
            {
                int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                    }
                }
            }
        }

        if (updatearrows)
        {
            for (int i = 0; i < 4; i++)
            {
                UpdateRow(i, false);
            }
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        FEAudio::EnableSounds(false);

        int flags = mMenuItems.mFlags;
        int currentIndex = mMenuItems.mCurrentIndex;
        int wrapBit = flags & 1;
        int skipDisabled = flags & 2;
        int result = currentIndex + 1;

        while (true)
        {
            if (wrapBit)
            {
                int numItems = mMenuItems.mNumItemsAdded;
                result = result % numItems;
            }
            else
            {
                if (result >= mMenuItems.mNumItemsAdded)
                {
                    result = RES_NOT_CHANGED;
                    break;
                }
            }

            if (skipDisabled)
            {
                if (mMenuItems.mMenuItems[result].mDisabled)
                {
                    result = result + 1;
                    continue;
                }
            }

            {
                int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[currentIndex].mCallbacks[ON_UNHIGHLIGHT].mFunctor)(type);
                    }
                }
            }

            mMenuItems.mCurrentIndex = result;

            {
                int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                    }
                }
            }

            result = RES_OK;
            break;
        }

        FEAudio::EnableSounds(true);

        sCurrentRow = sRowOffset + mMenuItems.mCurrentIndex;
        bool updatearrows = true;

        if (result == RES_NOT_CHANGED && (sRowOffset + 3) < 11)
        {
            sRowOffset = sRowOffset + 1;
            sCurrentRow = sRowOffset + mMenuItems.mCurrentIndex;

            int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
            if (tag)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                }
            }
        }
        else
        {
            if (result == RES_NOT_CHANGED)
            {
                updatearrows = false;
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            }
            else if (result == RES_OK)
            {
                int tag = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mTag;
                if (tag)
                {
                    TLComponentInstance* type = mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFreeFunction(type);
                    }
                    else
                    {
                        (*mMenuItems.mMenuItems[mMenuItems.mCurrentIndex].mCallbacks[ON_HIGHLIGHT].mFunctor)(type);
                    }
                }
            }
        }

        if (updatearrows)
        {
            for (int i = 0; i < 4; i++)
            {
                UpdateRow(i, false);
            }
        }
    }

    if (sCurrentRow == 0)
    {
        mUpArrow->m_bVisible = false;
        mDownArrow->m_bVisible = true;
    }
    else if (sCurrentRow == 11)
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = false;
    }
    else
    {
        mUpArrow->m_bVisible = true;
        mDownArrow->m_bVisible = true;
    }
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
