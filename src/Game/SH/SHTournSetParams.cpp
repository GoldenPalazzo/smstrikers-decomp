#include "Game/SH/SHTournSetParams.h"

#include "Game/FE/feFinder.h"
#include "NL/nlPrint.h"

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*fn)());

extern char __vt__13SlideMenuItem[];

typedef void FnTLComponentInstanceCb(TLComponentInstance*);

namespace SingleHighlite
{
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
void TempDisableSound();
} // namespace SingleHighlite

extern nlColour SubMenuHighliteColour;

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
 * TODO: 90.38% match - remaining diffs are r27/r28/r29 register allocation swap,
 * MenuItem address folding (+4 base offset), and minor stack layout differences.
 */
void TournSetParamsScene::BuildSubMenuList(int menuitem, TLComponentInstance* compinstance, bool wraps, int startindex)
{
    extern int nlSNPrintf(char*, unsigned long, const char*, ...);
    typedef Detail::MemFunImpl<void, void (SlideMenuList::*)()> MemFunImpl_SML;
    typedef BindExp1<void, MemFunImpl_SML, SlideMenuList*> BindExp1_SML;

    SlideMenuList* list = new (nlMalloc(sizeof(SlideMenuList), 8, false)) SlideMenuList(compinstance);
    mSlideMenuLists[menuitem] = list;

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

        SlideMenuItem* item = (SlideMenuItem*)nlMalloc(sizeof(SlideMenuItem), 8, true);
        if (item != NULL)
        {
            TLComponentInstance* comp = sml->mComponentInstance;
            *(char**)item = __vt__13SlideMenuItem;
            item->mSlideMenuHash = (unsigned long)-1;
            item->mComponentInstance = comp;
            item->mUserEnumType = slidenum;
        }
        item->mSlideMenuHash = slideHash;

        MenuItem<SlideMenuItem>* menuItem = &sml->mMenuItems[sml->mNumItemsAdded];
        menuItem->mType = item;
        sml->mNumItemsAdded++;

        {
            BindExp1_SML bind = Bind<void, MemFunImpl_SML, SlideMenuList*>(
                MemFun<SlideMenuList, void>(&SlideMenuList::SetSlide), sml);
            Function<SlideMenuItem*> callback(bind);
            menuItem->mCallbacks[1] = callback;
        }
    } while (++slidenum);

    list = mSlideMenuLists[menuitem];
    MenuItem<SlideMenuItem>* menuItem = &list->mMenuItems[list->mCurrentIndex];
    menuItem->mCallbacks[2](menuItem->mType);

    list->mCurrentIndex = startindex;

    menuItem = &list->mMenuItems[list->mCurrentIndex];
    menuItem->mCallbacks[1](menuItem->mType);

    if (wraps)
    {
        list->mFlags = 1;
    }
}

/**
 * Offset/Address/Size: 0x168C | 0x800E1060 | size: 0x644
 * TODO: 94.42% match - r30/r31 register swap for this/presentation,
 * stack frame 0x100 vs 0x140, Function<R(P)> constructor dead store,
 * ApplyAction inline re-evaluates ternary action index.
 */
void TournSetParamsScene::SceneCreated()
{
    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    char menuname[16] = { 0 };

    for (int i = 0; i < 3; i++)
    {
        nlSNPrintf(menuname, 16, "MENU ITEM%d", i + 1);

        TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            presentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(menuname)),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0),
            InlineHasher(0));

        MenuItem<TLComponentInstance>* menuItem = &mMenuItems.mMenuItems[mMenuItems.mNumItemsAdded];
        menuItem->mType = instance;
        mMenuItems.mNumItemsAdded++;

        {
            Function<FnTLComponentInstanceCb> openFunc;
            openFunc.mTag = FREE_FUNCTION;
            openFunc.mFreeFunction = SingleHighlite::OpenItem;
            menuItem->mCallbacks[ON_HIGHLIGHT] = openFunc;
        }

        {
            Function<FnTLComponentInstanceCb> closeFunc;
            closeFunc.mTag = FREE_FUNCTION;
            closeFunc.mFreeFunction = SingleHighlite::CloseItem;
            menuItem->mCallbacks[ON_UNHIGHLIGHT] = closeFunc;
        }

        if (i == 0)
        {
            SingleHighlite::TempDisableSound();
        }

        menuItem->ApplyAction((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
    }

    mMenuItems.mFlags = 3;

    TLSlide* currentSlide = presentation->m_currentSlide;

    TLComponentInstance* instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CHOICES")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    BuildSubMenuList(0, instance, true, 0);

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("numbers")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    BuildSubMenuList(1, instance, true, 0);

    mSlideMenuLists[1]->mFlags = 3;

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("numbers2")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    BuildSubMenuList(2, instance, true, 0);

    SlideMenuList* slideMenuList = mSlideMenuLists[mMenuItems.mCurrentIndex];
    if (slideMenuList != NULL)
    {
        TLComponentInstance* comp = slideMenuList->mComponentInstance;
        if (comp != NULL)
        {
            if (comp->GetActiveSlide() != NULL)
            {
                TLSlide* firstSlide = comp->GetActiveSlide();
                TLSlide* slide = firstSlide;
                do
                {
                    comp->SetActiveSlide(slide);
                    TLInstance* head = comp->GetActiveSlide()->m_instances;
                    TLInstance* inst = head;
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
        InlineHasher(nlStringLowerHash("buttons")),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0),
        InlineHasher(0));
    mButtons.mButtonInstance = buttonComponent;
    mButtons.SetState(ButtonComponent::BS_A_AND_B);
}

/**
 * Offset/Address/Size: 0xBA0 | 0x800E0574 | size: 0xAEC
 */
void TournSetParamsScene::Update(float)
{
}

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
 * TODO: 87.3% match on decomp.me - remaining diffs are compiler scheduling
 * (lwzx vs add+lwz, srwi vs rlwinm byte mask, loop instruction interleaving,
 * register allocation r5/r6/r7 swaps, stbx vs stb addressing mode).
 * These are likely resolved by the actual MWCC build with -inline deferred.
 */
void TournSetParamsScene::ApplyMenuDefaults()
{
    SlideMenuList* slideMenu = mSlideMenuLists[0];
    SlideMenuItem* currentItem = slideMenu->mMenuItems[slideMenu->mCurrentIndex].mType;
    m_isLeagueMode = (currentItem->mUserEnumType == 0);
    if (!m_isLeagueMode)
    {
        mMenuItems.mMenuItems[2].mDisabled = true;
        mMenuItems.mMenuItems[2].mType->m_bVisible = false;

        SlideMenuList* menu = mSlideMenuLists[1];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[2](cur->mType);
        menu->mCurrentIndex = 1;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[1](cur->mType);

        menu = mSlideMenuLists[2];
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[2](cur->mType);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[1](cur->mType);

        mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled = true;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = false;

        unsigned char activestatetable[6] = { 1, 0, 1, 1, 1, 0 };
        int i = 0;
        for (int row = 0; row < 2; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                MenuItem<SlideMenuItem>* item;
                if (i == ON_INVALID)
                {
                    item = &mSlideMenuLists[1]->mMenuItems[mSlideMenuLists[1]->mCurrentIndex];
                }
                else
                {
                    item = &mSlideMenuLists[1]->mMenuItems[i];
                }
                item->mDisabled = activestatetable[row * 3 + col];
                i++;
            }
        }
    }
    else
    {
        mMenuItems.mMenuItems[2].mDisabled = false;
        mMenuItems.mMenuItems[2].mType->m_bVisible = true;

        SlideMenuList* menu = mSlideMenuLists[1];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[2](cur->mType);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[1](cur->mType);

        menu = mSlideMenuLists[2];
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[2](cur->mType);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        cur->mCallbacks[1](cur->mType);

        mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled = false;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->mNumItemsAdded; i++)
        {
            MenuItem<SlideMenuItem>* item;
            if (i == ON_INVALID)
            {
                item = &mSlideMenuLists[1]->mMenuItems[mSlideMenuLists[1]->mCurrentIndex];
            }
            else
            {
                item = &mSlideMenuLists[1]->mMenuItems[i];
            }
            item->mDisabled = false;
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800DF9D4 | size: 0x6B0
 * TODO: 87.87% match - remaining diffs are r29/r30 register allocation swaps
 * in menu pointer/index handling and non-league preselection branch flow
 * offsets.
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
        SlideMenuList* menu = mSlideMenuLists[0];
        int flags = menu->mFlags;
        int wrapFlag = flags & 1;
        int skipDisabled = flags & 2;
        int currentIndex = menu->mCurrentIndex;
        int newIndex = currentIndex - 1;

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
            menu = mSlideMenuLists[1];
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = 1;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }
        else if (m_numTeams == 8)
        {
            menu = mSlideMenuLists[1];
            MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 2);
            menu->mCurrentIndex = 5;
            cur = &menu->mMenuItems[menu->mCurrentIndex];
            CALL_MENU_CB(cur, 1);
        }

        menu = mSlideMenuLists[2];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 2);
        menu->mCurrentIndex = 0;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 1);

        mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled = true;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = false;

        unsigned char activestatetable[6] = { 1, 0, 1, 1, 1, 0 };
        int i = 0;
        for (int row = 0; row < 2; row++)
        {
            for (int col = 0; col < 3; col++)
            {
                MenuItem<SlideMenuItem>* item;
                if (i == ON_INVALID)
                {
                    item = &mSlideMenuLists[1]->mMenuItems[mSlideMenuLists[1]->mCurrentIndex];
                }
                else
                {
                    item = &mSlideMenuLists[1]->mMenuItems[i];
                }
                item->mDisabled = activestatetable[row * 3 + col];
                i++;
            }
        }
    }
    else
    {
        mMenuItems.mMenuItems[2].mDisabled = false;
        mMenuItems.mMenuItems[2].mType->m_bVisible = true;

        SlideMenuList* menu = mSlideMenuLists[1];
        MenuItem<SlideMenuItem>* cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 2);
        menu->mCurrentIndex = m_numTeams - 3;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 1);

        menu = mSlideMenuLists[2];
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 2);
        menu->mCurrentIndex = m_numGames - 1;
        cur = &menu->mMenuItems[menu->mCurrentIndex];
        CALL_MENU_CB(cur, 1);

        mSlideMenuLists[2]->mMenuItems[mSlideMenuLists[2]->mCurrentIndex].mDisabled = false;
        mSlideMenuLists[2]->mComponentInstance->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->mNumItemsAdded; i++)
        {
            MenuItem<SlideMenuItem>* item;
            if (i == ON_INVALID)
            {
                item = &mSlideMenuLists[1]->mMenuItems[mSlideMenuLists[1]->mCurrentIndex];
            }
            else
            {
                item = &mSlideMenuLists[1]->mMenuItems[i];
            }
            item->mDisabled = false;
        }
    }

#undef CALL_MENU_CB
}
