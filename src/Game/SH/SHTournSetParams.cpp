#include "Game/SH/SHTournSetParams.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "NL/nlPrint.h"

typedef void FnTLComponentInstanceCb(TLComponentInstance*);

namespace SingleHighlite
{
void OpenItem(TLComponentInstance*);
void CloseItem(TLComponentInstance*);
void TempDisableSound();
} // namespace SingleHighlite

extern nlColour SubMenuHighliteColour;
extern nlColour SubMenuUnhighliteColour;

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

        menuItem = sml->AddItem(slideHash, slidenum);
    } while (++slidenum);

    list = mSlideMenuLists[menuitem];
    list->SetItem(startindex);

    if (wraps)
    {
        mSlideMenuLists[menuitem]->SetFlag(1);
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

        menuItem = mMenuItems.AddItem(instance);

        menuItem->SetCallback(ON_HIGHLIGHT, SingleHighlite::OpenItem);
        menuItem->SetCallback(ON_UNHIGHLIGHT, SingleHighlite::CloseItem);

        if (i == 0)
        {
            SingleHighlite::TempDisableSound();
        }

        menuItem->RunCallback((i == 0) ? ON_HIGHLIGHT : ON_UNHIGHLIGHT);
    }

    mMenuItems.SetFlag(3);

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

    mSlideMenuLists[1]->SetFlag(3);

    instance = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("numbers2")));
    BuildSubMenuList(2, instance, true, 0);

    SlideMenuList* slideMenuList = mSlideMenuLists[mMenuItems.GetActiveItemIndex()];
    if (slideMenuList != NULL)
    {
        TLInstance* inst;
        TLInstance* head;
        TLSlide* slide;
        TLSlide* firstSlide;
        TLComponentInstance* comp = slideMenuList->GetComponentInstance();
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
void TournSetParamsScene::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    SlideMenuList* slideMenuList;

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        SlideMenuList* list = mSlideMenuLists[0];
        CustomTournament* customTourn = &nlSingleton<GameInfoManager>::s_pInstance->mCustomTournamentInfo;
        SlideMenuItem* item = list->GetMenuItem()->GetType();
        m_isLeagueMode = !item->GetUserEnumType();

        list = mSlideMenuLists[1];
        item = list->GetMenuItem()->GetType();
        m_numTeams = item->GetUserEnumType() + 3;

        list = mSlideMenuLists[2];
        item = list->GetMenuItem()->GetType();
        m_numGames = (item->GetUserEnumType() == 0) ? 1 : 2;

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
        slideMenuList = mSlideMenuLists[mMenuItems.GetActiveItemIndex()];
        if (slideMenuList != NULL)
        {
            TLInstance* inst;
            TLInstance* firstChild;
            TLSlide* currentSlide;
            TLSlide* startSlide;
            TLComponentInstance* comp;
            unsigned long hash;
            comp = slideMenuList->GetComponentInstance();
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

        if (mMenuItems.PreviousItem() == RES_OK)
        {
            slideMenuList = mSlideMenuLists[mMenuItems.GetActiveItemIndex()];
            if (slideMenuList != NULL)
            {
                TLComponentInstance* comp;
                TLInstance* firstChild;
                TLInstance* inst;
                TLSlide* startSlide;
                TLSlide* currentSlide;
                unsigned long hash;
                comp = slideMenuList->GetComponentInstance();
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
        slideMenuList = mSlideMenuLists[mMenuItems.GetActiveItemIndex()];
        if (slideMenuList != NULL)
        {
            TLComponentInstance* comp;
            TLInstance* firstChild;
            TLInstance* inst;
            TLSlide* startSlide;
            TLSlide* currentSlide;
            unsigned long hash;
            comp = slideMenuList->GetComponentInstance();
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

        if (mMenuItems.NextItem() == RES_OK)
        {
            slideMenuList = mSlideMenuLists[mMenuItems.GetActiveItemIndex()];
            if (slideMenuList != NULL)
            {
                TLComponentInstance* comp;
                TLInstance* firstChild;
                TLInstance* inst;
                TLSlide* startSlide;
                TLSlide* currentSlide;
                unsigned long hash;
                comp = slideMenuList->GetComponentInstance();
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
        int menuIndex = mMenuItems.GetActiveItemIndex();
        slideMenuList = mSlideMenuLists[menuIndex];
        if (slideMenuList != NULL)
        {
            MenuResult res = slideMenuList->PreviousItem();

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
        int menuIndex = mMenuItems.GetActiveItemIndex();
        slideMenuList = mSlideMenuLists[menuIndex];
        if (slideMenuList != NULL)
        {
            MenuResult res = slideMenuList->NextItem();

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
    SlideMenuList* slideMenu = mSlideMenuLists[0];
    MenuItem<SlideMenuItem>* cur0 = slideMenu->GetMenuItem();
    m_isLeagueMode = !cur0->GetType()->GetUserEnumType();
    if (!m_isLeagueMode)
    {
        MenuItem<TLComponentInstance>* gamesItem = mMenuItems.GetMenuItem(2);
        gamesItem->SetDisabledFlag(true);
        gamesItem->GetType()->m_bVisible = false;

        mSlideMenuLists[1]->SetItem(1);
        mSlideMenuLists[2]->SetItem(0);

        mSlideMenuLists[2]->GetMenuItem()->SetDisabledFlag(true);
        mSlideMenuLists[2]->GetComponentInstance()->m_bVisible = false;

        bool activestatetable[6] = { true, false, true, true, true, false };
        for (int i = 0; i < 6; i++)
        {
            mSlideMenuLists[1]->GetMenuItem(i)->SetDisabledFlag(activestatetable[i]);
        }
    }
    else
    {
        MenuItem<TLComponentInstance>* gamesItem = mMenuItems.GetMenuItem(2);
        gamesItem->SetDisabledFlag(false);
        gamesItem->GetType()->m_bVisible = true;

        mSlideMenuLists[1]->SetItem(0);
        mSlideMenuLists[2]->SetItem(0);

        mSlideMenuLists[2]->GetMenuItem()->SetDisabledFlag(false);
        mSlideMenuLists[2]->GetComponentInstance()->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->GetNumItemsAdded(); i++)
        {
            mSlideMenuLists[1]->GetMenuItem(i)->SetDisabledFlag(false);
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800DF9D4 | size: 0x6B0
 */
void TournSetParamsScene::InitializeMenu()
{
    if (!m_isLeagueMode)
    {
        mSlideMenuLists[0]->PreviousItem();

        MenuItem<TLComponentInstance>* gamesItem = mMenuItems.GetMenuItem(2);
        gamesItem->SetDisabledFlag(true);
        gamesItem->GetType()->m_bVisible = false;

        if (m_numTeams == 4)
        {
            mSlideMenuLists[1]->SetItem(1);
        }
        else if (m_numTeams == 8)
        {
            mSlideMenuLists[1]->SetItem(5);
        }

        mSlideMenuLists[2]->SetItem(0);

        mSlideMenuLists[2]->GetMenuItem()->SetDisabledFlag(true);
        mSlideMenuLists[2]->GetComponentInstance()->m_bVisible = false;

        bool activestatetable[6] = { true, false, true, true, true, false };
        for (int i = 0; i < 6; i++)
        {
            mSlideMenuLists[1]->GetMenuItem(i)->SetDisabledFlag(activestatetable[i]);
        }
    }
    else
    {
        MenuItem<TLComponentInstance>* gamesItem = mMenuItems.GetMenuItem(2);
        gamesItem->SetDisabledFlag(false);
        gamesItem->GetType()->m_bVisible = true;

        int numTeamsSelection = m_numTeams - 3;
        mSlideMenuLists[1]->SetItem(numTeamsSelection);

        int numGamesSelection = m_numGames - 1;
        mSlideMenuLists[2]->SetItem(numGamesSelection);

        mSlideMenuLists[2]->GetMenuItem()->SetDisabledFlag(false);
        mSlideMenuLists[2]->GetComponentInstance()->m_bVisible = true;

        for (int i = 0; i < mSlideMenuLists[1]->GetNumItemsAdded(); i++)
        {
            mSlideMenuLists[1]->GetMenuItem(i)->SetDisabledFlag(false);
        }
    }
}
