#define BIND_NO_DECL
#include "Game/SH/SHTournSetParams.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "NL/nlPrint.h"
#undef BIND_NO_DECL

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg)
{
    BindExp1<R, F, A> result;
    result.mFuncPtr = fn;
    result.mArg = arg;
    return result;
}

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)());

typedef void FnTLComponentInstanceCb(TLComponentInstance*);

namespace SingleHighlite
{
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
void TempDisableSound();
} // namespace SingleHighlite

extern nlColour SubMenuHighliteColour;
extern nlColour SubMenuUnhighliteColour;

static inline MenuItem<TLComponentInstance>* TournSetParamsMenuItemAt(MenuList<TLComponentInstance>& menu, int idx)
{
    return &menu.mMenuItems[idx];
}

static inline MenuItem<SlideMenuItem>* TournSetParamsSlideMenuItemAt(SlideMenuList* menu, int idx)
{
    MenuItem<SlideMenuItem>* items = menu->mMenuItems;
    return &items[idx];
}

static inline void TournSetParamsInstallSlideCallback(MenuItem<SlideMenuItem>* menuItem, SlideMenuList* const& sml)
{
    typedef Detail::MemFunImpl<void, void (SlideMenuList::*)()> MemFunImpl_SML;
    typedef BindExp1<void, MemFunImpl_SML, SlideMenuList*> BindExp1_SML;
    typedef void FnSlideMenuItemCb(SlideMenuItem*);

    BindExp1_SML bind = Bind<void>(MemFun<SlideMenuList, void>(&SlideMenuList::SetSlide), sml);
    Function<FnSlideMenuItemCb> callback(bind);
    menuItem->mCallbacks[1] = callback;
}

// /**
//  * Offset/Address/Size: 0xBC | 0x800E1D48 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800E1CC4 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800E1C8C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x21F0 | 0x800E1BC4 | size: 0xC8
 */
TournSetParamsScene::TournSetParamsScene()
    : BaseSceneHandler()
    , mMenuItems()
    , m_isLeagueMode(true)
    , m_numTeams(3)
    , m_numGames(1)
    , mButtons()
{
    mSlideMenuLists[0] = NULL;
    mSlideMenuLists[1] = NULL;
    mSlideMenuLists[2] = NULL;
}

/**
 * Offset/Address/Size: 0x2104 | 0x800E1AD8 | size: 0xEC
 */
TournSetParamsScene::~TournSetParamsScene()
{
    for (int i = 0; i < 3; i++)
    {
        delete mSlideMenuLists[i];
    }
}

/**
 * Offset/Address/Size: 0x1CD0 | 0x800E16A4 | size: 0x434
 * TODO: 99.76% match - r29/r30 register swap for this+offset vs memcpy-temp/menuItem.
 */
void TournSetParamsScene::BuildSubMenuList(int menuitem, TLComponentInstance* compinstance, bool wraps, int startindex)
{
    SlideMenuList* list = new (nlMalloc(sizeof(SlideMenuList), 8, false)) SlideMenuList(compinstance);
    mSlideMenuLists[menuitem] = list;

    MenuItem<SlideMenuItem>* menuItem;
    char slidename[64] = { 0 };

    int slidenum = 0;
    do
    {
        nlSNPrintf(slidename, 64, "Slide%d", slidenum + 1);
        compinstance->SetActiveSlide(slidename);

        if (compinstance->GetActiveSlide() == NULL)
        {
            break;
        }

        unsigned long slideHash = compinstance->GetActiveSlide()->m_hash;

        SlideMenuList* sml = mSlideMenuLists[menuitem];

        SlideMenuItem* item = new (nlMalloc(sizeof(SlideMenuItem), 8, true)) SlideMenuItem(sml->mComponentInstance, slidenum);
        item->mSlideMenuHash = slideHash;

        menuItem = TournSetParamsSlideMenuItemAt(sml, sml->mNumItemsAdded);
        menuItem->mType = item;
        sml->mNumItemsAdded++;

        TournSetParamsInstallSlideCallback(menuItem, sml);
    } while (++slidenum);

    list = mSlideMenuLists[menuitem];
    menuItem = &list->mMenuItems[list->mCurrentIndex];
    int tag = menuItem->mCallbacks[2].mTag;
    if (((u32)((-tag) | tag) >> 31) > 0)
    {
        SlideMenuItem* type = menuItem->mType;
        if (tag == FREE_FUNCTION)
            menuItem->mCallbacks[2].mFreeFunction(type);
        else
            (*menuItem->mCallbacks[2].mFunctor)(type);
    }

    list->mCurrentIndex = startindex;

    menuItem = &list->mMenuItems[list->mCurrentIndex];
    tag = menuItem->mCallbacks[1].mTag;
    if (((u32)((-tag) | tag) >> 31) > 0)
    {
        SlideMenuItem* type = menuItem->mType;
        if (tag == FREE_FUNCTION)
            menuItem->mCallbacks[1].mFreeFunction(type);
        else
            (*menuItem->mCallbacks[1].mFunctor)(type);
    }

    if (wraps)
    {
        mSlideMenuLists[menuitem]->mFlags = 1;
    }
}

/**
 * Offset/Address/Size: 0x168C | 0x800E1060 | size: 0x644
 * TODO: 99.74% match - first submenu call uses a direct instance argument instead of a temporary move.
 */
void TournSetParamsScene::SceneCreated()
{
    MenuItem<TLComponentInstance>* menuItem;
    TLComponentInstance* instance;
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    char menuname[16] = { 0 };

    for (int i = 0; i < 3; i++)
    {
        nlSNPrintf(menuname, 16, "MENU ITEM%d", i + 1);

        instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)));

        int numAdded = mMenuItems.mNumItemsAdded;
        menuItem = TournSetParamsMenuItemAt(mMenuItems, numAdded);
        mMenuItems.mMenuItems[numAdded].mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<TLComponentInstance*> openFunc;
            openFunc.mTag = FREE_FUNCTION;
            openFunc.mFreeFunction = SingleHighlite::OpenItem;
            *(Function<TLComponentInstance*>*)&menuItem->mCallbacks[ON_HIGHLIGHT] = openFunc;
        }

        {
            Function<TLComponentInstance*> closeFunc;
            closeFunc.mTag = FREE_FUNCTION;
            closeFunc.mFreeFunction = SingleHighlite::CloseItem;
            *(Function<TLComponentInstance*>*)&menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunc;
        }

        if (i == 0)
        {
            SingleHighlite::TempDisableSound();
        }

        menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
    }

    mMenuItems.mFlags = 3;

    TLSlide* currentSlide = presentation->m_currentSlide;

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CHOICES")));
    BuildSubMenuList(0, instance, true, 0);

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("numbers")));
    BuildSubMenuList(1, instance, true, 0);

    mSlideMenuLists[1]->mFlags = 3;

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("numbers2")));
    BuildSubMenuList(2, instance, true, 0);

    SlideMenuList* slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
    if (slideMenuList != NULL)
    {
        TLInstance* inst;
        TLInstance* head;
        TLSlide* slide;
        TLSlide* firstSlide;
        TLComponentInstance* comp = slideMenuList->mComponentInstance;
        if (comp != NULL)
        {
            if (comp->GetActiveSlide() != NULL)
            {
                firstSlide = comp->GetActiveSlide();
                slide = firstSlide;
                do
                {
                    comp->SetActiveSlide(slide);
                    head = comp->GetActiveSlide()->m_instances;
                    inst = head;
                    if (inst != NULL)
                    {
                        do
                        {
                            if (inst->m_type == TLAT_TEXT)
                            {
                                inst->SetAssetColour(SubMenuHighliteColour);
                            }
                            else if (inst->m_type == TLAT_IMAGE)
                            {
                                unsigned long hash = inst->m_hash;
                                if (hash != nlStringLowerHash("white_box"))
                                {
                                    inst->SetAssetColour(SubMenuHighliteColour);
                                }
                            }
                            inst = inst->m_next;
                        } while (inst != head);
                    }
                    slide = slide->m_next;
                } while (slide != firstSlide);

                comp->SetActiveSlide(firstSlide);
            }
        }
    }

    InitializeMenu();

    TLComponentInstance* buttonComponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        presentation->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")));
    mButtons.mButtonInstance = buttonComponent;
    mButtons.SetState(ButtonComponent::BS_A_AND_B);
}

/**
 * Offset/Address/Size: 0xBA0 | 0x800E0574 | size: 0xAEC
 * TODO: 99.31% match - remaining slide traversal register swaps
 */
#define CALL_MENU_CB_UPDATE_TOP(cur, action)                   \
    do                                                         \
    {                                                          \
        int tag = (cur)->mCallbacks[action].mTag;              \
        if (((u32)((-tag) | tag) >> 31) > 0)                   \
        {                                                      \
            TLComponentInstance* type = (cur)->mType;          \
            if (tag == FREE_FUNCTION)                          \
            {                                                  \
                (cur)->mCallbacks[action].mFreeFunction(type); \
            }                                                  \
            else                                               \
            {                                                  \
                (*(cur)->mCallbacks[action].mFunctor)(type);   \
            }                                                  \
        }                                                      \
    } while (0)

#define CALL_MENU_CB_UPDATE_SUB(cur, action)                   \
    do                                                         \
    {                                                          \
        int tag = (cur)->mCallbacks[action].mTag;              \
        if (((u32)((-tag) | tag) >> 31) > 0)                   \
        {                                                      \
            SlideMenuItem* type = (cur)->mType;                \
            if (tag == FREE_FUNCTION)                          \
            {                                                  \
                (cur)->mCallbacks[action].mFreeFunction(type); \
            }                                                  \
            else                                               \
            {                                                  \
                (*(cur)->mCallbacks[action].mFunctor)(type);   \
            }                                                  \
        }                                                      \
    } while (0)

void TournSetParamsScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    int newIndex;
    SlideMenuList* slideMenuList;

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        SlideMenuList* list = mSlideMenuLists[0];
        CustomTournament* customTourn = &nlSingleton<GameInfoManager>::s_pInstance->mCustomTournamentInfo;
        SlideMenuItem** itemPtr = &list->mMenuItems[list->mCurrentIndex].mType;
        SlideMenuItem* item = *itemPtr;
        m_isLeagueMode = !item->mUserEnumType;

        list = mSlideMenuLists[1];
        itemPtr = &list->mMenuItems[list->mCurrentIndex].mType;
        item = *itemPtr;
        m_numTeams = item->mUserEnumType + 3;

        list = mSlideMenuLists[2];
        itemPtr = &list->mMenuItems[list->mCurrentIndex].mType;
        item = *itemPtr;
        m_numGames = (item->mUserEnumType == 0) ? 1 : 2;

        customTourn->m_tournMode = (eTournamentMode)(m_isLeagueMode ? 0 : 1);
        customTourn->m_numTeams = m_numTeams;
        if (m_isLeagueMode)
        {
            customTourn->m_numGamesPerTeam = m_numGames;
        }

        customTourn->ConstructCup();
        nlSingleton<GameInfoManager>::s_pInstance->SetMode(GameInfoManager::GM_TOURNAMENT);
        nlSingleton<GameInfoManager>::s_pInstance->mCurrentCup->mCupSettings = nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mGameplayOptions;

        GameSceneManager::Instance()->Push(SCENE_CUP_OPTIONS_INITIAL_TOURN, SCREEN_FORWARD, true);

        FEAudio::PlayAnimAudioEvent("sfx_accept", false);
    }
    else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
    {
        GameSceneManager::Instance()->PopEntireStack();
        GameSceneManager::Instance()->Push(SCENE_MAIN_MENU, SCREEN_BACK, false);

        FEAudio::PlayAnimAudioEvent("sfx_back", false);
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xD, true, NULL))
    {
        slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
        if (slideMenuList != NULL)
        {
            TLInstance* inst;
            TLInstance* firstChild;
            TLSlide* currentSlide;
            TLSlide* startSlide;
            TLComponentInstance* comp;
            unsigned long hash;
            comp = slideMenuList->mComponentInstance;
            if (comp != NULL && comp->GetActiveSlide() != NULL)
            {
                startSlide = comp->GetActiveSlide();
                currentSlide = startSlide;

                do
                {
                    comp->SetActiveSlide(currentSlide);
                    firstChild = comp->GetActiveSlide()->m_instances;
                    inst = firstChild;

                    if (firstChild != NULL)
                    {
                        do
                        {
                            if (inst->m_type == TLAT_TEXT)
                            {
                                inst->SetAssetColour(SubMenuUnhighliteColour);
                            }
                            else if (inst->m_type == TLAT_IMAGE)
                            {
                                hash = inst->m_hash;
                                if (hash != nlStringLowerHash("white_box"))
                                {
                                    inst->SetAssetColour(SubMenuUnhighliteColour);
                                }
                            }

                            inst = inst->m_next;
                        } while (inst != firstChild);
                    }

                    currentSlide = currentSlide->m_next;
                } while (currentSlide != startSlide);

                comp->SetActiveSlide(startSlide);
            }
        }

        MenuResult res = RES_ERROR;
        int flags = mMenuItems.mFlags;
        int skipDisabled;
        int wrapList = flags & 1;
        skipDisabled = flags & 2;
        int oldIndex = mMenuItems.mCurrentIndex;
        newIndex = oldIndex - 1;

        while (true)
        {
            if (wrapList)
            {
                if (newIndex < 0)
                {
                    newIndex = mMenuItems.mNumItemsAdded - 1;
                }
            }
            else if (newIndex < 0)
            {
                res = RES_NOT_CHANGED;
                break;
            }

            if (skipDisabled)
            {
                if (mMenuItems.mMenuItems[newIndex].mDisabled)
                {
                    newIndex--;
                    continue;
                }
            }

            CALL_MENU_CB_UPDATE_TOP(&mMenuItems.mMenuItems[oldIndex], ON_UNHIGHLIGHT);
            mMenuItems.mCurrentIndex = newIndex;
            CALL_MENU_CB_UPDATE_TOP(&mMenuItems.mMenuItems[mMenuItems.mCurrentIndex], ON_HIGHLIGHT);
            res = RES_OK;
            break;
        }

        if (res == RES_OK)
        {
            slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
            if (slideMenuList != NULL)
            {
                TLComponentInstance* comp;
                TLInstance* firstChild;
                TLInstance* inst;
                TLSlide* startSlide;
                TLSlide* currentSlide;
                unsigned long hash;
                comp = slideMenuList->mComponentInstance;
                if (comp != NULL && comp->GetActiveSlide() != NULL)
                {
                    startSlide = comp->GetActiveSlide();
                    currentSlide = startSlide;

                    do
                    {
                        comp->SetActiveSlide(currentSlide);
                        firstChild = comp->GetActiveSlide()->m_instances;
                        inst = firstChild;

                        if (firstChild != NULL)
                        {
                            do
                            {
                                if (inst->m_type == TLAT_TEXT)
                                {
                                    inst->SetAssetColour(SubMenuHighliteColour);
                                }
                                else if (inst->m_type == TLAT_IMAGE)
                                {
                                    hash = inst->m_hash;
                                    if (hash != nlStringLowerHash("white_box"))
                                    {
                                        inst->SetAssetColour(SubMenuHighliteColour);
                                    }
                                }

                                inst = inst->m_next;
                            } while (inst != firstChild);
                        }

                        currentSlide = currentSlide->m_next;
                    } while (currentSlide != startSlide);

                    comp->SetActiveSlide(startSlide);
                }
            }
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xE, true, NULL))
    {
        slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
        if (slideMenuList != NULL)
        {
            TLComponentInstance* comp;
            TLInstance* firstChild;
            TLInstance* inst;
            TLSlide* startSlide;
            TLSlide* currentSlide;
            unsigned long hash;
            comp = slideMenuList->mComponentInstance;
            if (comp != NULL && comp->GetActiveSlide() != NULL)
            {
                startSlide = comp->GetActiveSlide();
                currentSlide = startSlide;

                do
                {
                    comp->SetActiveSlide(currentSlide);
                    firstChild = comp->GetActiveSlide()->m_instances;
                    inst = firstChild;

                    if (firstChild != NULL)
                    {
                        do
                        {
                            if (inst->m_type == TLAT_TEXT)
                            {
                                inst->SetAssetColour(SubMenuUnhighliteColour);
                            }
                            else if (inst->m_type == TLAT_IMAGE)
                            {
                                hash = inst->m_hash;
                                if (hash != nlStringLowerHash("white_box"))
                                {
                                    inst->SetAssetColour(SubMenuUnhighliteColour);
                                }
                            }

                            inst = inst->m_next;
                        } while (inst != firstChild);
                    }

                    currentSlide = currentSlide->m_next;
                } while (currentSlide != startSlide);

                comp->SetActiveSlide(startSlide);
            }
        }

        MenuResult res = RES_ERROR;
        int flags = mMenuItems.mFlags;
        int skipDisabled;
        int wrapList = flags & 1;
        skipDisabled = flags & 2;
        int oldIndex = mMenuItems.mCurrentIndex;
        newIndex = oldIndex + 1;

        while (true)
        {
            if (wrapList)
            {
                newIndex = newIndex % mMenuItems.mNumItemsAdded;
            }
            else if (newIndex >= mMenuItems.mNumItemsAdded)
            {
                res = RES_NOT_CHANGED;
                break;
            }

            if (skipDisabled)
            {
                if (mMenuItems.mMenuItems[newIndex].mDisabled)
                {
                    newIndex++;
                    continue;
                }
            }

            CALL_MENU_CB_UPDATE_TOP(&mMenuItems.mMenuItems[oldIndex], ON_UNHIGHLIGHT);
            mMenuItems.mCurrentIndex = newIndex;
            CALL_MENU_CB_UPDATE_TOP(&mMenuItems.mMenuItems[mMenuItems.mCurrentIndex], ON_HIGHLIGHT);
            res = RES_OK;
            break;
        }

        if (res == RES_OK)
        {
            slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
            if (slideMenuList != NULL)
            {
                TLComponentInstance* comp;
                TLInstance* firstChild;
                TLInstance* inst;
                TLSlide* startSlide;
                TLSlide* currentSlide;
                unsigned long hash;
                comp = slideMenuList->mComponentInstance;
                if (comp != NULL && comp->GetActiveSlide() != NULL)
                {
                    startSlide = comp->GetActiveSlide();
                    currentSlide = startSlide;

                    do
                    {
                        comp->SetActiveSlide(currentSlide);
                        firstChild = comp->GetActiveSlide()->m_instances;
                        inst = firstChild;

                        if (firstChild != NULL)
                        {
                            do
                            {
                                if (inst->m_type == TLAT_TEXT)
                                {
                                    inst->SetAssetColour(SubMenuHighliteColour);
                                }
                                else if (inst->m_type == TLAT_IMAGE)
                                {
                                    hash = inst->m_hash;
                                    if (hash != nlStringLowerHash("white_box"))
                                    {
                                        inst->SetAssetColour(SubMenuHighliteColour);
                                    }
                                }

                                inst = inst->m_next;
                            } while (inst != firstChild);
                        }

                        currentSlide = currentSlide->m_next;
                    } while (currentSlide != startSlide);

                    comp->SetActiveSlide(startSlide);
                }
            }
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xB, true, NULL))
    {
        int menuIndex = mMenuItems.mCurrentIndex;
        slideMenuList = mSlideMenuLists[menuIndex];
        if (slideMenuList != NULL)
        {
            MenuResult res = RES_ERROR;

            int flags = slideMenuList->mFlags;
            int skipDisabled;
            int wrapList = flags & 1;
            skipDisabled = flags & 2;
            int oldIndex = slideMenuList->mCurrentIndex;
            newIndex = oldIndex - 1;

            while (true)
            {
                if (wrapList)
                {
                    if (newIndex < 0)
                    {
                        newIndex = slideMenuList->mNumItemsAdded - 1;
                    }
                }
                else if (newIndex < 0)
                {
                    res = RES_NOT_CHANGED;
                    break;
                }

                if (skipDisabled)
                {
                    if (slideMenuList->mMenuItems[newIndex].mDisabled)
                    {
                        newIndex--;
                        continue;
                    }
                }

                MenuItem<SlideMenuItem>* oldItem = &slideMenuList->mMenuItems[oldIndex];
                CALL_MENU_CB_UPDATE_SUB(oldItem, ON_UNHIGHLIGHT);
                slideMenuList->mCurrentIndex = newIndex;
                MenuItem<SlideMenuItem>* newItem = &slideMenuList->mMenuItems[slideMenuList->mCurrentIndex];
                CALL_MENU_CB_UPDATE_SUB(newItem, ON_HIGHLIGHT);
                res = RES_OK;
                break;
            }

            switch (res)
            {
            case RES_OK:
                FEAudio::PlayAnimAudioEvent("sfx_option_scroll_left", false);
                if (menuIndex == 0)
                {
                    ApplyMenuDefaults();
                }
                break;
            case RES_NOT_CHANGED:
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                break;
            }
        }
    }
    else if (g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xC, true, NULL))
    {
        int menuIndex = mMenuItems.mCurrentIndex;
        slideMenuList = mSlideMenuLists[menuIndex];
        if (slideMenuList != NULL)
        {
            MenuResult res = RES_ERROR;

            int flags = slideMenuList->mFlags;
            int skipDisabled;
            int wrapList = flags & 1;
            skipDisabled = flags & 2;
            int oldIndex = slideMenuList->mCurrentIndex;
            newIndex = oldIndex + 1;

            while (true)
            {
                if (wrapList)
                {
                    newIndex = newIndex % slideMenuList->mNumItemsAdded;
                }
                else if (newIndex >= slideMenuList->mNumItemsAdded)
                {
                    res = RES_NOT_CHANGED;
                    break;
                }

                if (skipDisabled)
                {
                    if (slideMenuList->mMenuItems[newIndex].mDisabled)
                    {
                        newIndex++;
                        continue;
                    }
                }

                MenuItem<SlideMenuItem>* oldItem = &slideMenuList->mMenuItems[oldIndex];
                CALL_MENU_CB_UPDATE_SUB(oldItem, ON_UNHIGHLIGHT);
                slideMenuList->mCurrentIndex = newIndex;
                MenuItem<SlideMenuItem>* newItem = &slideMenuList->mMenuItems[slideMenuList->mCurrentIndex];
                CALL_MENU_CB_UPDATE_SUB(newItem, ON_HIGHLIGHT);
                res = RES_OK;
                break;
            }

            switch (res)
            {
            case RES_OK:
                FEAudio::PlayAnimAudioEvent("sfx_option_scroll_right", false);
                if (menuIndex == 0)
                {
                    ApplyMenuDefaults();
                }
                break;
            case RES_NOT_CHANGED:
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
                break;
            }
        }
    }
}

#undef CALL_MENU_CB_UPDATE_SUB
#undef CALL_MENU_CB_UPDATE_TOP

/**
 * Offset/Address/Size: 0xB90 | 0x800E0564 | size: 0x10
 */
void TournSetParamsScene::SetInitialParams(bool isLeagueMode, int numTeams, int numGames)
{
    m_isLeagueMode = isLeagueMode;
    m_numTeams = numTeams;
    m_numGames = numGames;
}

/**
 * Offset/Address/Size: 0x6B0 | 0x800E0084 | size: 0x4E0
 * TODO: 99.34% match - remaining diffs are r5/r6/r7 register allocation
 * swaps in the disabled-state loops.
 */
#define CALL_MENU_CB_APPLY(cur, action)                        \
    do                                                         \
    {                                                          \
        int tag = (cur)->mCallbacks[action].mTag;              \
        if (((u32)((-tag) | tag) >> 31) > 0)                   \
        {                                                      \
            SlideMenuItem* type = (cur)->mType;                \
            if (tag == FREE_FUNCTION)                          \
            {                                                  \
                (cur)->mCallbacks[action].mFreeFunction(type); \
            }                                                  \
            else                                               \
            {                                                  \
                (*(cur)->mCallbacks[action].mFunctor)(type);   \
            }                                                  \
        }                                                      \
    } while (0)

void TournSetParamsScene::ApplyMenuDefaults()
{
    SlideMenuList* slideMenu = mSlideMenuLists[0];
    int idx = slideMenu->mCurrentIndex;
    MenuItem<SlideMenuItem>* cur0 = &slideMenu->mMenuItems[idx];
    m_isLeagueMode = !cur0->mType->mUserEnumType;
    if (!m_isLeagueMode)
    {
        mMenuItems.mMenuItems[2].mDisabled = true;
        mMenuItems.mMenuItems[2].mType->m_bVisible = false;

        SlideMenuList* menu = mSlideMenuLists[1];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 2);
        menu->mCurrentIndex = 1;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 1);

        menu = mSlideMenuLists[2];
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 2);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 1);

        ((unsigned char&)mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled) = true;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = false;

        unsigned char activestatetable[6] = { 1, 0, 1, 1, 1, 0 };
        for (int i = 0; i < 6; i++)
        {
            struct LoopState
            {
                SlideMenuList* list;
                MenuItem<SlideMenuItem>* item;
                unsigned char active;
            } state;
            state.active = activestatetable[i];
            state.list = mSlideMenuLists[1];
            if (i == ON_INVALID)
            {
                state.item = &state.list->mMenuItems[state.list->mCurrentIndex];
            }
            else
            {
                state.item = &state.list->mMenuItems[i];
            }
            ((unsigned char&)state.item->mDisabled) = state.active;
        }
    }
    else
    {
        mMenuItems.mMenuItems[2].mDisabled = false;
        mMenuItems.mMenuItems[2].mType->m_bVisible = true;

        SlideMenuList* menu = mSlideMenuLists[1];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 2);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 1);

        menu = mSlideMenuLists[2];
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 2);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB_APPLY(cur, 1);

        struct LeagueState
        {
            SlideMenuList* list;
            MenuItem<SlideMenuItem>* item;
            unsigned char active;
        } state;
        state.active = false;
        state.list = mSlideMenuLists[2];
        ((unsigned char&)state.list->mMenuItems[state.list->mCurrentIndex].mDisabled) = state.active;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->mNumItemsAdded; i++)
        {
            state.list = mSlideMenuLists[1];
            if (i == ON_INVALID)
            {
                state.item = &state.list->mMenuItems[state.list->mCurrentIndex];
            }
            else
            {
                state.item = &state.list->mMenuItems[i];
            }
            state.item->mDisabled = state.active;
        }
    }
}

#undef CALL_MENU_CB_APPLY

/**
 * Offset/Address/Size: 0x0 | 0x800DF9D4 | size: 0x6B0
 * TODO: 99.52% match - remaining diffs are r5/r6/r7 register allocation
 * swaps in the disabled-state loops.
 */
void TournSetParamsScene::InitializeMenu()
{
#define CALL_MENU_CB(cur, action)                              \
    do                                                         \
    {                                                          \
        int tag = (cur)->mCallbacks[action].mTag;              \
        if (((u32)((-tag) | tag) >> 31) > 0)                   \
        {                                                      \
            SlideMenuItem* type = (cur)->mType;                \
            if (tag == FREE_FUNCTION)                          \
            {                                                  \
                (cur)->mCallbacks[action].mFreeFunction(type); \
            }                                                  \
            else                                               \
            {                                                  \
                (*(cur)->mCallbacks[action].mFunctor)(type);   \
            }                                                  \
        }                                                      \
    } while (0)

    if (!m_isLeagueMode)
    {
        int newIndex;
        SlideMenuList* menu = mSlideMenuLists[0];
        int flags = menu->mFlags;
        int skipDisabled;
        int wrapFlag = flags & 1;
        skipDisabled = flags & 2;
        int currentIndex = menu->mCurrentIndex;
        newIndex = currentIndex - 1;

        while (true)
        {
            if (wrapFlag)
            {
                if (newIndex < 0)
                {
                    newIndex = menu->mNumItemsAdded - 1;
                }
            }
            else
            {
                if (newIndex < 0)
                {
                    break;
                }
            }

            if (skipDisabled)
            {
                if (menu->mMenuItems[newIndex].mDisabled)
                {
                    newIndex--;
                    continue;
                }
            }

            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[currentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = newIndex;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
            break;
        }

        mMenuItems.mMenuItems[2].mDisabled = true;
        mMenuItems.mMenuItems[2].mType->m_bVisible = false;

        if (m_numTeams == 4)
        {
            SlideMenuList* menu = mSlideMenuLists[1];
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = 1;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }
        else if (m_numTeams == 8)
        {
            SlideMenuList* menu = mSlideMenuLists[1];
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = 5;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }

        {
            SlideMenuList* menu = mSlideMenuLists[2];
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = 0;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }

        ((unsigned char&)mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled) = true;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = false;

        unsigned char activestatetable[6] = { 1, 0, 1, 1, 1, 0 };
        for (int i = 0; i < 6; i++)
        {
            struct LoopState
            {
                SlideMenuList* list;
                MenuItem<SlideMenuItem>* item;
                unsigned char active;
            } state;
            state.active = activestatetable[i];
            state.list = mSlideMenuLists[1];
            if (i == ON_INVALID)
            {
                state.item = &state.list->mMenuItems[state.list->mCurrentIndex];
            }
            else
            {
                state.item = &state.list->mMenuItems[i];
            }
            ((unsigned char&)state.item->mDisabled) = state.active;
        }
    }
    else
    {
        mMenuItems.mMenuItems[2].mDisabled = false;
        mMenuItems.mMenuItems[2].mType->m_bVisible = true;

        SlideMenuList* menu = mSlideMenuLists[1];
        int numTeamsSelection = m_numTeams - 3;
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 2);
        menu->mCurrentIndex = numTeamsSelection;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 1);

        {
            SlideMenuList* menu = mSlideMenuLists[2];
            int numGamesSelection = m_numGames - 1;
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = numGamesSelection;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }

        struct LeagueState
        {
            SlideMenuList* list;
            MenuItem<SlideMenuItem>* item;
            unsigned char active;
        } state;
        state.active = false;
        state.list = mSlideMenuLists[2];
        ((unsigned char&)state.list->mMenuItems[state.list->mCurrentIndex].mDisabled) = state.active;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->mNumItemsAdded; i++)
        {
            state.list = mSlideMenuLists[1];
            if (i == ON_INVALID)
            {
                state.item = &state.list->mMenuItems[state.list->mCurrentIndex];
            }
            else
            {
                state.item = &state.list->mMenuItems[i];
            }
            state.item->mDisabled = state.active;
        }
    }

#undef CALL_MENU_CB
}
