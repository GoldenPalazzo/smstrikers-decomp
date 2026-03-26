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
 */
void TournSetParamsScene::ApplyMenuDefaults()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800DF9D4 | size: 0x6B0
 */
void TournSetParamsScene::InitializeMenu()
{
}
