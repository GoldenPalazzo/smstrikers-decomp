#define MEMFUN_NO_DECL
#define BIND_NO_DECL
#define FUNCTION1_SPLIT_BODIES
#include "Game/SH/SHSpoils.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feInput.h"
#include "Game/SH/SHCupTrophy.h"
#include "Game/SH/SHMilestoneTrophy.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

typedef Detail::MemFunImpl<void, void (SpoilsScene::*)(SpoilsScene::eSpoils)> MemFunImpl_Spoils_t;
typedef BindExp2<void, MemFunImpl_Spoils_t, SpoilsScene*, SpoilsScene::eSpoils> BindExp2_Spoils_t;
typedef Function1<void, TLComponentInstance*>::FunctorImpl<BindExp2_Spoils_t> FunctorImpl_Spoils_t;

#include "NL/nlMemFunBody.h"
#include "NL/nlBindBody.h"

s32 SpoilsScene::mLastSelectedIndex = 0;

namespace DoubleHighlite
{
static const char* SLIDE_IN = "in";
static const char* SLIDE_OUT = "out";
} // namespace DoubleHighlite

typedef TLInstance* (*FindInstByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
typedef TLInstance* (*FindInstByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

/**
 * Offset/Address/Size: 0xACC | 0x800D19BC | size: 0xA4
 */
SpoilsScene::SpoilsScene()
{
}

/**
 * Offset/Address/Size: 0xA28 | 0x800D1918 | size: 0xA4
 */
SpoilsScene::~SpoilsScene()
{
}

/**
 * Offset/Address/Size: 0x6B4 | 0x800D15A4 | size: 0x374
 */
void SpoilsScene::Update(float dt)
{
    extern s32 mLastSelectedIndex__11SpoilsScene;

    BaseSceneHandler::Update(dt);
    mButtons.CentreButtons();

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        int currentIndex = mMenuItems.mCurrentIndex;
        int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mTag;

        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            if (mMenuItems.mMenuItems[currentIndex].mDisabled == 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;

                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[0].mFunctor)(type);
                }
            }
        }

        mLastSelectedIndex__11SpoilsScene = mMenuItems.mCurrentIndex;
        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, true);
        mLastSelectedIndex__11SpoilsScene = 0;
        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int wrapFlag = flags & 1;
        int currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex - 1;

    loop_up:
        if (wrapFlag)
        {
            if (newIndex < 0)
            {
                newIndex = mMenuItems.mNumItemsAdded - 1;
            }
        }
        else
        {
            if (newIndex < 0)
            {
                return;
            }
        }

        if (flags & 2)
        {
            if (mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex--;
                goto loop_up;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }

        return;
    }

    if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        int flags = mMenuItems.mFlags;
        int wrapFlag = flags & 1;
        int currentIndex = mMenuItems.mCurrentIndex;
        int newIndex = currentIndex + 1;

    loop_down:
        if (wrapFlag)
        {
            newIndex = newIndex % mMenuItems.mNumItemsAdded;
        }
        else
        {
            if (newIndex >= mMenuItems.mNumItemsAdded)
            {
                return;
            }
        }

        if (flags & 2)
        {
            if (mMenuItems.mMenuItems[newIndex].mDisabled)
            {
                newIndex++;
                goto loop_down;
            }
        }

        {
            int tag = mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[currentIndex].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[currentIndex].mCallbacks[2].mFunctor)(type);
                }
            }
        }

        mMenuItems.mCurrentIndex = newIndex;

        {
            int selIdx = mMenuItems.mCurrentIndex;
            int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
            if (((u32)((-tag) | tag) >> 31) > 0)
            {
                TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
                if (tag == FREE_FUNCTION)
                {
                    mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
                }
                else
                {
                    (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
                }
            }
        }
    }
}

static inline MenuItem<TLComponentInstance>* SpoilsItemAt(MenuList<TLComponentInstance>& menu, int idx)
{
    return &menu.mMenuItems[idx] - 1;
}

/**
 * Offset/Address/Size: 0xC4 | 0x800D0FB4 | size: 0x5F0
 */
void SpoilsScene::SceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();
    char menuname[64];
    MenuItem<TLComponentInstance>* item;

    for (int i = 0; i < SPOILS_NUM_CHOICES; i++)
    {
        nlSNPrintf(menuname, 64, "MENU ITEM%d", i + 2);

        TLInstance* instance = FEFinder<TLInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)));
        TLComponentInstance* compinstance = (TLComponentInstance*)instance;

        compinstance->SetActiveSlide(i == SpoilsScene::mLastSelectedIndex ? DoubleHighlite::SLIDE_IN : DoubleHighlite::SLIDE_OUT);

        int idx = mMenuItems.mNumItemsAdded;
        item = SpoilsItemAt(mMenuItems, idx);
        mMenuItems.mMenuItems[idx].mType = compinstance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<TLComponentInstance*> callback1;
            callback1.mTag = FREE_FUNCTION;
            callback1.mFreeFunction = DoubleHighlite::OpenItem;
            *(Function<TLComponentInstance*>*)&item[1].mCallbacks[1] = callback1;
        }

        {
            Function<TLComponentInstance*> callback2;
            callback2.mTag = FREE_FUNCTION;
            callback2.mFreeFunction = DoubleHighlite::CloseItem;
            *(Function<TLComponentInstance*>*)&item[1].mCallbacks[2] = callback2;
        }

        {
            BindExp2_Spoils_t bind = Bind<void, MemFunImpl_Spoils_t, SpoilsScene*, SpoilsScene::eSpoils>(
                MemFun<SpoilsScene, void, SpoilsScene::eSpoils>(&SpoilsScene::ShowSpoils),
                this,
                (SpoilsScene::eSpoils)i);

            Function<TLComponentInstance*> callback0;
            callback0.mTag = FUNCTOR;
            callback0.mFunctor = new ((FunctorImpl_Spoils_t*)nlMalloc(sizeof(FunctorImpl_Spoils_t), 8, false))
                FunctorImpl_Spoils_t(bind);
            *(Function<TLComponentInstance*>*)&item[1].mCallbacks[0] = callback0;
        }

        FindComponent(compinstance->GetActiveSlide(), "highlite");

        if (i == SpoilsScene::mLastSelectedIndex)
        {
            DoubleHighlite::TempDisableSound();
            {
                int tag = item[1].mCallbacks[1].mTag;
                if (((u32)((-tag) | tag) >> 31) > 0)
                {
                    TLComponentInstance* type = item[1].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        item[1].mCallbacks[1].mFreeFunction(type);
                    }
                    else
                    {
                        (*item[1].mCallbacks[1].mFunctor)(type);
                    }
                }
            }
        }
        else
        {
            {
                int tag = item[1].mCallbacks[2].mTag;
                if (((u32)((-tag) | tag) >> 31) > 0)
                {
                    TLComponentInstance* type = item[1].mType;
                    if (tag == FREE_FUNCTION)
                    {
                        item[1].mCallbacks[2].mFreeFunction(type);
                    }
                    else
                    {
                        (*item[1].mCallbacks[2].mFunctor)(type);
                    }
                }
            }
            TLSlide* slide = compinstance->GetActiveSlide();
            compinstance->Update(1.0f + (slide->m_start + slide->m_duration));
        }
    }

    mMenuItems.mFlags = 1;
    mMenuItems.mCurrentIndex = SpoilsScene::mLastSelectedIndex;

    {
        int selIdx = mMenuItems.mCurrentIndex;
        int tag = mMenuItems.mMenuItems[selIdx].mCallbacks[1].mTag;
        if (((u32)((-tag) | tag) >> 31) > 0)
        {
            TLComponentInstance* type = mMenuItems.mMenuItems[selIdx].mType;
            if (tag == FREE_FUNCTION)
            {
                mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFreeFunction(type);
            }
            else
            {
                (*mMenuItems.mMenuItems[selIdx].mCallbacks[1].mFunctor)(type);
            }
        }
    }

    TLComponentInstance* buttonComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")));
    mButtons.mButtonInstance = buttonComponent;

    mButtons.SetState(ButtonComponent::BS_A_AND_B);
}

/**
 * Offset/Address/Size: 0x0 | 0x800D0EF0 | size: 0xC4
 */
void SpoilsScene::ShowSpoils(SpoilsScene::eSpoils spoils)
{
    if (spoils == SPOILS_CUP)
    {
        CupTrophyScene* pScene = (CupTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_CUP_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(TROPHY_MUSHROOM_CUP, ButtonComponent::BS_B_ONLY, false);
    }
    else if (spoils == SPOILS_SUPER_CUP)
    {
        CupTrophyScene* pScene = (CupTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_CUP_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(TROPHY_SUPER_MUSHROOM_CUP, ButtonComponent::BS_B_ONLY, false);
    }
    else
    {
        MilestoneTrophyScene* pScene = (MilestoneTrophyScene*)GameSceneManager::GetInstance()->Push(SCENE_MILESTONE_TROPHY, SCREEN_FORWARD, true);
        pScene->CreateTrophyScene(NUM_BATTLE_TROPHIES, ButtonComponent::BS_B_ONLY, false);
    }
}
