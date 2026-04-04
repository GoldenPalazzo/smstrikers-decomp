#include "Game/SH/SHTournSetParams.h"

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
 * TODO: 62.95% match - callback functor construction path still differs
 * (local SetSlideFunctor vs target Bind/MemFun FunctorImpl shape),
 * causing prologue/register allocation and callback setup mismatches.
 */
void TournSetParamsScene::BuildSubMenuList(int menuitem, TLComponentInstance* compinstance, bool wraps, int startindex)
{
    extern int nlSNPrintf(char*, unsigned long, const char*, ...);

    class SetSlideFunctor : public Function1<void, SlideMenuItem*>::FunctorBase
    {
    public:
        void (SlideMenuList::*mMemFun)();
        SlideMenuList* mList;

        SetSlideFunctor(SlideMenuList* list)
            : mMemFun(&SlideMenuList::SetSlide)
            , mList(list)
        {
        }

        virtual ~SetSlideFunctor()
        {
        }

        virtual void operator()(SlideMenuItem*)
        {
            (mList->*mMemFun)();
        }

        virtual Function1<void, SlideMenuItem*>::FunctorBase* Clone() const
        {
            return new ((SetSlideFunctor*)nlMalloc(sizeof(SetSlideFunctor), 8, false)) SetSlideFunctor(*this);
        }
    };

    SlideMenuList* list = (SlideMenuList*)nlMalloc(sizeof(SlideMenuList), 8, false);
    if (list != NULL)
    {
        new ((MenuList<SlideMenuItem>*)list) MenuList<SlideMenuItem>();
        list->mInputLocked = 0;
        list->mComponentInstance = compinstance;
    }
    mSlideMenuLists[menuitem] = list;

    char slidename[64] = { 0 };

    int slidenum = 0;
    while (true)
    {
        nlSNPrintf(slidename, 64, "Slide%d", slidenum + 1);
        compinstance->SetActiveSlide(slidename);

        if (compinstance->GetActiveSlide() == NULL)
        {
            break;
        }

        unsigned long slideHash = compinstance->GetActiveSlide()->m_hash;

        SlideMenuItem* item = (SlideMenuItem*)nlMalloc(sizeof(SlideMenuItem), 8, true);
        if (item != NULL)
        {
            item->mSlideMenuHash = (unsigned long)-1;
            item->mComponentInstance = list->mComponentInstance;
            item->mUserEnumType = slidenum;
        }
        item->mSlideMenuHash = slideHash;

        MenuItem<SlideMenuItem>* menuItem = &list->mMenuItems[list->mNumItemsAdded];
        menuItem->mType = item;
        list->mNumItemsAdded++;

        Function<SlideMenuItem*> callback;
        callback.mTag = FUNCTOR;
        callback.mFunctor = new ((SetSlideFunctor*)nlMalloc(sizeof(SetSlideFunctor), 8, false)) SetSlideFunctor(list);
        menuItem->mCallbacks[1] = callback;

        slidenum++;
    }

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
 */
void TournSetParamsScene::SceneCreated()
{
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
 */
void TournSetParamsScene::InitializeMenu()
{
}
