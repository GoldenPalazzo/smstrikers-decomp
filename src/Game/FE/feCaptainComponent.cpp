#include "Game/FE/feCaptainComponent.h"
#include "Game/FE/feAsyncImage.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/tlSlide.h"

extern bool g_e3_Build;

// /**
//  * Offset/Address/Size: 0x9C | 0x800C1658 | size: 0x9C
//  */
// void IGridComponent<eTeamID>::RebindHighliteComponent(const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800C15BC | size: 0x9C
//  */
// void IGridComponent<eSidekickID>::RebindHighliteComponent(const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800C1460 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800C13DC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800C13A4 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800C1248 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800C11C4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800C118C | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800C1030 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800C0FAC | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800C0F74 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1104 | 0x800C08A8 | size: 0x6CC
 */
void IChooseCaptain::ComponentState::GotoNextPhase()
{
    ICaptainGridComponent* captaingrid;
    ISidekickGridComponent* sidekickgrid;
    ISidekickGridComponent* sidekickgrid2;
    eTeamID chosenteam;
    char filenameC2[0x80];
    char filenameC1[0x80];
    char filenameC0[0x80];
    char filenameS2[0x80];
    char filenameS1[0x80];
    char filenameS0[0x80];

    switch (mCurrentPhase)
    {
    case PHASE_CHOOSING_CAPTAIN:
        captaingrid = mParent->mCaptainGridComponents[mHomeAway];

        if (!captaingrid->mMapMenu->IsSelectedItemActive())
        {
            FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            break;
        }

        captaingrid->mParentComponent->SetActiveSlide("OUT");
        captaingrid->mParentComponent->Update(0.0f);
        captaingrid->RebuildInstanceTable();
        captaingrid->mMapMenu->UpdateAllItems();
        captaingrid->RebindHighliteComponent("HIGHLIGHT");
        captaingrid->mHighliteComponent->m_bVisible = false;

        {
            const char* eventName = "sfx_character_group_right_exit";
            if (mHomeAway == 0)
            {
                eventName = "sfx_character_group_left_exit";
            }
            FEAudio::PlayAnimAudioEvent(eventName, false);
        }

        mParent->mHomeAwayTeam[mHomeAway] = captaingrid->GetSelectedItem();

        {
            FEMapMenu* mapmenu = mParent->mCaptainGridComponents[mHomeAway ^ 1]->mMapMenu;
            mapmenu->SetItemActive(captaingrid->mMapMenu->GetSelectedItem(), false);
        }

        chosenteam = captaingrid->GetSelectedItem();
        if (chosenteam != TEAM_MYSTERY)
        {
            sidekickgrid = mParent->mSidekickGridComponents[mHomeAway];
            sidekickgrid->mParentComponent->SetActiveSlide("IN");
            sidekickgrid->mParentComponent->Update(0.0f);
            sidekickgrid->RebuildInstanceTable();
            sidekickgrid->mMapMenu->UpdateAllItems();
            sidekickgrid->RebindHighliteComponent("HIGHLIGHT");
            sidekickgrid->mHighliteComponent->m_bVisible = false;
            sidekickgrid->mHighliteVisibilityAtAnimEnd = true;
            sidekickgrid->SetVisibleInstanceTable(true);
            sidekickgrid->mParentComponent->m_bVisible = true;
            mCurrentPhase = PHASE_CHOOSING_SIDEKICK;

            {
                const char* eventName = "sfx_character_group_right_enter";
                if (mHomeAway == 0)
                {
                    eventName = "sfx_character_group_left_enter";
                }
                FEAudio::PlayAnimAudioEvent(eventName, false);
            }

            mParent->mNameComponents[mHomeAway].mComponent->SetActiveSlide("Slide2");
            mParent->mNameComponents[mHomeAway].mComponent->Update(0.0f);
            mParent->mNameComponents[mHomeAway].SetCaptainName(GetLOCCharacterName((eTeamID)mParent->mHomeAwayTeam[mHomeAway], false, false));
            mParent->mNameComponents[mHomeAway].SetCaptainLogo(GetTeamName((eTeamID)mParent->mHomeAwayTeam[mHomeAway]));
            mParent->mNameComponents[mHomeAway].SetSidekickName(GetLOCSidekickName(sidekickgrid->GetSelectedItem()));

            if (g_e3_Build)
            {
                mParent->mSidekickGridComponents[mHomeAway]->MoveHighlightToTarget((eSidekickID)(mHomeAway != 0));
                mParent->mNameComponents[mHomeAway].SetSidekickName(GetLOCSidekickName(sidekickgrid->GetSelectedItem()));
                GotoNextPhase();
            }
        }
        else
        {
            int homeaway = mHomeAway;
            IChooseCaptain* parent = mParent;
            int teamID = parent->mHomeAwayTeam[homeaway];

            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_0, filenameC0, 0x80, teamID, homeaway);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_1, filenameC1, 0x80, teamID, homeaway);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_2, filenameC2, 0x80, teamID, homeaway);

            parent->mAsyncImage[homeaway][0]->QueueLoad(filenameC0, false);
            parent->mAsyncImage[homeaway][1]->QueueLoad(filenameC1, false);
            parent->mAsyncImage[homeaway][2]->QueueLoad(filenameC2, false);

            parent->mDidSwapCaptains[homeaway] = false;
            mCurrentPhase = PHASE_READY;
        }

        FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
        mParent->mLastCaptainSelectSoundStrPlayed[mHomeAway] = (char*)FECharacterSound::PlayCaptainName((eTeamID)mParent->mHomeAwayTeam[mHomeAway]);
        break;

    case PHASE_CHOOSING_SIDEKICK:
        sidekickgrid2 = mParent->mSidekickGridComponents[mHomeAway];

        sidekickgrid2->mParentComponent->SetActiveSlide("OUT");
        sidekickgrid2->mParentComponent->Update(0.0f);
        sidekickgrid2->RebuildInstanceTable();
        sidekickgrid2->mMapMenu->UpdateAllItems();
        sidekickgrid2->RebindHighliteComponent("HIGHLIGHT");
        sidekickgrid2->mHighliteComponent->m_bVisible = false;

        {
            const char* eventName = "sfx_character_group_right_exit";
            if (mHomeAway == 0)
            {
                eventName = "sfx_character_group_left_exit";
            }
            FEAudio::PlayAnimAudioEvent(eventName, false);
        }

        if (g_e3_Build)
        {
            sidekickgrid2->SetVisibleInstanceTable(false);
        }

        mParent->mHomeAwaySidekicks[mHomeAway] = sidekickgrid2->GetSelectedItem();
        {
            int homeaway = mHomeAway;
            IChooseCaptain* parent = mParent;
            int teamID = parent->mHomeAwayTeam[homeaway];

            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_0, filenameS0, 0x80, teamID, homeaway);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_1, filenameS1, 0x80, teamID, homeaway);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_2, filenameS2, 0x80, teamID, homeaway);

            parent->mAsyncImage[homeaway][0]->QueueLoad(filenameS0, false);
            parent->mAsyncImage[homeaway][1]->QueueLoad(filenameS1, false);
            parent->mAsyncImage[homeaway][2]->QueueLoad(filenameS2, false);

            parent->mDidSwapCaptains[homeaway] = false;
            parent->StartSidekickMiniHead(homeaway, (eSidekickID)parent->mHomeAwaySidekicks[homeaway]);
        }
        mCurrentPhase = PHASE_READY;

        FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);

        if (!g_e3_Build)
        {
            if (mParent->mLastCaptainSelectSoundStrPlayed[mHomeAway] != NULL)
            {
                FEAudio::StopAnimAudioEvent(mParent->mLastCaptainSelectSoundStrPlayed[mHomeAway]);
                mParent->mLastCaptainSelectSoundStrPlayed[mHomeAway] = NULL;
            }

            FECharacterSound::PlaySidekickName((eSidekickID)mParent->mHomeAwaySidekicks[mHomeAway]);
        }
        break;

    case PHASE_READY:
        if (mParent->mCaptainGridComponents[mHomeAway]->GetSelectedItem() != TEAM_MYSTERY)
        {
            mParent->mSidekickComponents[mHomeAway]->m_bVisible = false;
        }

        FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
        break;

    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0xA48 | 0x800C01EC | size: 0x6BC
 */
void IChooseCaptain::ComponentState::GotoPreviousPhase()
{
    ICaptainGridComponent* gridcomponent;
    ICaptainGridComponent* captaingrid;
    ICaptainGridComponent* othercaptaingrid;
    ICaptainGridComponent* captaingrid2;
    ICaptainGridComponent* othercaptaingrid2;

    switch (mCurrentPhase)
    {
    case PHASE_CHOOSING_CAPTAIN:
        mCurrentPhase = PHASE_IDLE;

        gridcomponent = mParent->mCaptainGridComponents[mHomeAway];
        gridcomponent->mParentComponent->SetActiveSlide("OUT");
        gridcomponent->mParentComponent->Update(0.0f);
        gridcomponent->RebuildInstanceTable();
        gridcomponent->mMapMenu->UpdateAllItems();
        gridcomponent->RebindHighliteComponent("HIGHLIGHT");
        gridcomponent->mHighliteComponent->m_bVisible = false;

        mParent->mNameComponents[mHomeAway].SetCaptainName(0);
        mParent->mNameComponents[mHomeAway].SetCaptainLogo(NULL);
        mParent->mSidekickMiniHeadComponents[mHomeAway]->m_bVisible = false;

        FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", true);
        break;

    case PHASE_CHOOSING_SIDEKICK:
        mCurrentPhase = PHASE_CHOOSING_CAPTAIN;

        mParent->mSidekickGridComponents[mHomeAway]->mParentComponent->m_bVisible = false;

        captaingrid = mParent->mCaptainGridComponents[mHomeAway];
        captaingrid->mParentComponent->SetActiveSlide("SELECT");
        captaingrid->mParentComponent->Update(0.0f);
        captaingrid->RebuildInstanceTable();
        captaingrid->mMapMenu->UpdateAllItems();
        captaingrid->RebindHighliteComponent("HIGHLIGHT");
        captaingrid->mHighliteComponent->m_bVisible = true;
        captaingrid->mParentComponent->m_bVisible = true;
        captaingrid->MoveHighlightToTarget((eTeamID)mParent->mHomeAwayTeam[mHomeAway]);

        othercaptaingrid = mParent->mCaptainGridComponents[mHomeAway ^ 1];
        othercaptaingrid->RebuildInstanceTable();
        othercaptaingrid->SetAllItemsActive();
        captaingrid->RebuildInstanceTable();
        captaingrid->SetAllItemsActive();

        mParent->mNameComponents[mHomeAway].mComponent->SetActiveSlide("Slide1");
        mParent->mNameComponents[mHomeAway].mComponent->Update(0.0f);
        mParent->mNameComponents[mHomeAway].SetCaptainName(GetLOCCharacterName(captaingrid->GetSelectedItem(), false, false));
        mParent->mNameComponents[mHomeAway].SetCaptainLogo(GetTeamName(captaingrid->GetSelectedItem()));

        if (mParent->mComponentState[mHomeAway ^ 1].mCurrentPhase > PHASE_CHOOSING_CAPTAIN)
        {
            FEMapMenu* menu = captaingrid->mMapMenu;
            menu->SetItemActive(othercaptaingrid->mMapMenu->GetSelectedItem(), false);
        }

        FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
        break;

    case PHASE_READY:
        if (mParent->mCaptainGridComponents[mHomeAway]->GetSelectedItem() != (eTeamID)8 && mParent->mHomeAwayTeam[mHomeAway] != 8)
        {
            mCurrentPhase = PHASE_CHOOSING_SIDEKICK;

            gridcomponent = (ICaptainGridComponent*)mParent->mSidekickGridComponents[mHomeAway];
            gridcomponent->mParentComponent->SetActiveSlide("SELECT");
            gridcomponent->mParentComponent->Update(0.0f);
            gridcomponent->RebuildInstanceTable();
            gridcomponent->mMapMenu->UpdateAllItems();

            mParent->mSidekickGridComponents[mHomeAway]->RebindHighliteComponent("HIGHLIGHT");
            mParent->mSidekickGridComponents[mHomeAway]->mHighliteComponent->m_bVisible = true;
            mParent->mSidekickGridComponents[mHomeAway]->SetVisibleInstanceTable(true);
            mParent->mSidekickGridComponents[mHomeAway]->MoveHighlightToTarget((eSidekickID)mParent->mHomeAwaySidekicks[mHomeAway]);
            mParent->mSidekickGridComponents[mHomeAway]->mParentComponent->m_bVisible = true;

            mParent->mSidekickComponents[mHomeAway]->m_bVisible = false;
            mParent->mCaptainComponents[mHomeAway]->m_bVisible = false;
            mParent->mSidekickMiniHeadComponents[mHomeAway]->m_bVisible = false;

            if (g_e3_Build)
            {
                GotoPreviousPhase();
            }
        }
        else
        {
            mCurrentPhase = PHASE_CHOOSING_CAPTAIN;

            captaingrid2 = mParent->mCaptainGridComponents[mHomeAway];
            captaingrid2->mParentComponent->SetActiveSlide("SELECT");
            captaingrid2->mParentComponent->Update(0.0f);
            captaingrid2->RebuildInstanceTable();
            captaingrid2->mMapMenu->UpdateAllItems();
            captaingrid2->RebindHighliteComponent("HIGHLIGHT");
            captaingrid2->mHighliteComponent->m_bVisible = true;
            captaingrid2->mParentComponent->m_bVisible = true;
            captaingrid2->MoveHighlightToTarget((eTeamID)mParent->mHomeAwayTeam[mHomeAway]);
            mParent->mCaptainComponents[mHomeAway]->m_bVisible = false;

            othercaptaingrid2 = mParent->mCaptainGridComponents[mHomeAway ^ 1];
            othercaptaingrid2->RebuildInstanceTable();
            othercaptaingrid2->SetAllItemsActive();
            captaingrid2->RebuildInstanceTable();
            captaingrid2->SetAllItemsActive();

            mParent->mNameComponents[mHomeAway].mComponent->SetActiveSlide("Slide1");
            mParent->mNameComponents[mHomeAway].mComponent->Update(0.0f);
            mParent->mNameComponents[mHomeAway].SetCaptainName(GetLOCCharacterName(captaingrid2->GetSelectedItem(), false, false));
            mParent->mNameComponents[mHomeAway].SetCaptainLogo(GetTeamName(captaingrid2->GetSelectedItem()));

            if (mParent->mComponentState[mHomeAway ^ 1].mCurrentPhase > PHASE_CHOOSING_CAPTAIN)
            {
                FEMapMenu* menu = captaingrid2->mMapMenu;
                menu->SetItemActive(othercaptaingrid2->mMapMenu->GetSelectedItem(), false);
            }
        }

        FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
        break;

    case PHASE_IDLE:
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0xA20 | 0x800C01C4 | size: 0x28
 */
void IChooseCaptain::NameComponent::SetSidekickName(unsigned long id)
{
    SetTextName(mSidekickObjName, id);
}

/**
 * Offset/Address/Size: 0x1AC | 0x800BF950 | size: 0x874
 */
void IChooseCaptain::ComponentState::SetCurrentPhase(Phase phase)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xEC | 0x800BF890 | size: 0xC0
 */
void IChooseCaptain::NameComponent::SetTextName(const char* objname, unsigned long locstring)
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(objname);
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    TLTextInstance* textinstance = findText.byRef(
        mComponent->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (locstring != 0)
    {
        textinstance->m_LocStrId = locstring;
        textinstance->m_OverloadFlags |= 0x8u;
        textinstance->m_bVisible = true;
    }
    else
    {
        textinstance->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0xC4 | 0x800BF868 | size: 0x28
 */
void IChooseCaptain::NameComponent::SetCaptainName(unsigned long captainID)
{
    FORCE_DONT_INLINE;
    SetTextName(mCaptainObjName, captainID);
}

/**
 * Offset/Address/Size: 0x0 | 0x800BF7A4 | size: 0xC4
 */
void IChooseCaptain::NameComponent::SetCaptainLogo(const char* name)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("component");
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    TLComponentInstance* comp = findComp.byRef(
        mComponent->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (name != NULL)
    {
        comp->SetActiveSlide(name);
        comp->m_bVisible = true;
    }
    else
    {
        comp->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x1DF4 | 0x800BF790 | size: 0x14
 */
IChooseCaptain::IChooseCaptain()
{
    mIsSinglePlayerInput = true;
    mNumTotalPushedPlayers = 0;
}

/**
 * Offset/Address/Size: 0x1CE0 | 0x800BF67C | size: 0x114
 */
IChooseCaptain::~IChooseCaptain()
{
    int j;
    int i;
    IChooseCaptain* col;
    IChooseCaptain* row;

    for (i = 0, row = this; i < 2; i++, row = (IChooseCaptain*)((u8*)row + 0xC))
    {
        for (j = 0, col = row; j < 3; j++, col = (IChooseCaptain*)((u8*)col + 4))
        {
            delete col->mAsyncImage[0][0];
        }
    }

    delete mCaptainGridComponents[0];
    delete mCaptainGridComponents[1];
    delete mSidekickGridComponents[0];
    delete mSidekickGridComponents[1];
}

/**
 * Offset/Address/Size: 0x1B4C | 0x800BF4E8 | size: 0x194
 */
void IChooseCaptain::Initialize(const char* captainfilename, const char*)
{
    int i;

    for (i = 0; i < 2; i++)
    {
        mAsyncImage[i][0] = new (0x20, true) AsyncImage(captainfilename, NULL);
        mAsyncImage[i][1] = new (0x20, true) AsyncImage(captainfilename, NULL);
        mAsyncImage[i][2] = new (0x20, true) AsyncImage(captainfilename, NULL);
    }

    mAllPushedPlayers[0] = FE_ALL_PADS;
    mAllPushedPlayerSides[0] = -1;
    mAllPushedPlayers[1] = FE_ALL_PADS;
    mAllPushedPlayerSides[1] = -1;
    mAllPushedPlayers[2] = FE_ALL_PADS;
    mAllPushedPlayerSides[2] = -1;
    mAllPushedPlayers[3] = FE_ALL_PADS;
    mAllPushedPlayerSides[3] = -1;
    mNumTotalPushedPlayers = 0;

    mHomeAwayTeam[0] = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0);
    mHomeAwayTeam[1] = nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1);
    mHomeAwaySidekicks[0] = nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0);
    mHomeAwaySidekicks[1] = nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1);

    mDidSwapCaptains[1] = false;
    mDidSwapCaptains[0] = false;
    mDidSwapSidekicks[1] = false;
    mDidSwapSidekicks[0] = false;

    mComponentState[0].mCurrentPhase = PHASE_IDLE;
    mComponentState[0].mParent = this;
    mComponentState[0].mHomeAway = 0;
    mComponentState[1].mCurrentPhase = PHASE_IDLE;
    mComponentState[1].mParent = this;
    mComponentState[1].mHomeAway = 1;

    mCaptainSoundDelay[0] = 0.0f;
    mCaptainSoundDelay[1] = 0.0f;

    mLastCaptainSelectSoundStrPlayed[0] = NULL;
    mLastCaptainSelectSoundStrPlayed[1] = NULL;
}

/**
 * Offset/Address/Size: 0x1AB0 | 0x800BF44C | size: 0x9C
 */
void IChooseCaptain::UpdateSound(float dt)
{
    for (s32 i = 0; i < 2; i++)
    {
        if (mCaptainSoundDelay[i] > 0.0f)
        {
            mCaptainSoundDelay[i] -= dt;
            if (mCaptainSoundDelay[i] <= 0.0f)
            {
                mCaptainSoundDelay[i] = 0.0f;
                FECharacterSound::PlayCaptainSlideIn((eTeamID)mHomeAwayTeam[i]);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x141C | 0x800BEDB8 | size: 0x694
 */
UpdateResult IChooseCaptain::Update(float)
{
    CheckForDisconnectedHumanPlayers();
    FindAliveHumanPlayers();

    int numSide1;
    mIsSinglePlayerInput = numSide1 = 0;

    if (mNumTotalPushedPlayers == 1)
    {
        mIsSinglePlayerInput = true;
    }
    else
    {
        int numSide0 = numSide1;
        IChooseCaptain* p = this;

        for (int i = 0; i < mNumTotalPushedPlayers; i++)
        {
            if (p->mAllPushedPlayerSides[0] == 0)
            {
                numSide0++;
            }
            else if (p->mAllPushedPlayerSides[0] == 1)
            {
                numSide1++;
            }

            p = (IChooseCaptain*)((u8*)p + 4);
        }

        if (numSide0 == 0 || numSide1 == 0)
        {
            mIsSinglePlayerInput = true;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        eFEINPUT_PAD inputpad = (eFEINPUT_PAD)i;
        int side;

        if (mIsSinglePlayerInput)
        {
            if (mComponentState[0].mCurrentPhase < PHASE_READY)
            {
                side = 0;
            }
            else
            {
                side = 1;
            }
        }
        else
        {
            int j = 0;
            IChooseCaptain* p = this;

            for (; j < mNumTotalPushedPlayers; j++)
            {
                if (p->mAllPushedPlayers[0] == i)
                {
                    side = mAllPushedPlayerSides[j];
                    goto found_side;
                }

                p = (IChooseCaptain*)((u8*)p + 4);
            }

            side = -1;
        }

    found_side:
        if (side == -1)
        {
            continue;
        }

        if (g_pFEInput->JustPressed(inputpad, 0x200, false, NULL))
        {
            unsigned char goback = 0;

            switch (mComponentState[side].mCurrentPhase)
            {
            case PHASE_CHOOSING_CAPTAIN:
                if (mIsSinglePlayerInput)
                {
                    if (side == 1)
                    {
                        mComponentState[1].GotoPreviousPhase();
                        mComponentState[0].GotoPreviousPhase();
                    }
                    else
                    {
                        goback = true;
                    }
                }
                else
                {
                    goback = true;
                }
                break;

            case PHASE_CHOOSING_SIDEKICK:
            case PHASE_READY:
                mComponentState[side].GotoPreviousPhase();
                break;
            }

            if (goback)
            {
                FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
                return UPDATE_GO_BACK;
            }
        }
        else if (g_pFEInput->JustPressed(inputpad, 0x100, false, &inputpad))
        {
            unsigned char isdoneanimating = 1;

            switch (mComponentState[side].mCurrentPhase)
            {
            case PHASE_CHOOSING_CAPTAIN:
            {
                TLSlide* slide = mCaptainGridComponents[side]->mParentComponent->GetActiveSlide();

                if (slide == NULL)
                {
                    isdoneanimating = 1;
                }
                else if (slide->m_time >= slide->m_start + slide->m_duration)
                {
                    isdoneanimating = 1;
                }
                else
                {
                    isdoneanimating = 0;
                }
                break;
            }

            case PHASE_CHOOSING_SIDEKICK:
            {
                TLSlide* slide = mSidekickGridComponents[side]->mParentComponent->GetActiveSlide();

                if (slide == NULL)
                {
                    isdoneanimating = 1;
                }
                else if (slide->m_time >= slide->m_start + slide->m_duration)
                {
                    isdoneanimating = 1;
                }
                else
                {
                    isdoneanimating = 0;
                }
                break;
            }
            }

            if (isdoneanimating)
            {
                int side2;

                if (mIsSinglePlayerInput)
                {
                    if (mComponentState[0].mCurrentPhase < PHASE_READY)
                    {
                        side2 = 0;
                    }
                    else
                    {
                        side2 = 1;
                    }
                }
                else
                {
                    int j = 0;
                    IChooseCaptain* p = this;

                    for (; j < mNumTotalPushedPlayers; j++)
                    {
                        if (p->mAllPushedPlayers[0] == inputpad)
                        {
                            side2 = mAllPushedPlayerSides[j];
                            goto found_side2;
                        }

                        p = (IChooseCaptain*)((u8*)p + 4);
                    }

                    side2 = -1;
                }

            found_side2:
                mComponentState[side2].GotoNextPhase();

                if (mIsSinglePlayerInput && mComponentState[0].mCurrentPhase == PHASE_READY && mComponentState[1].mCurrentPhase == PHASE_IDLE)
                {
                    mComponentState[1].SetCurrentPhase(PHASE_CHOOSING_CAPTAIN);
                }
            }
        }
        else
        {
            switch (mComponentState[side].mCurrentPhase)
            {
            case PHASE_CHOOSING_CAPTAIN:
                mCaptainGridComponents[side]->Update(inputpad);
                if (mCaptainGridComponents[side]->mHasChangedSinceLastUpdate)
                {
                    mNameComponents[side].mComponent->SetActiveSlide("Slide1");
                    mNameComponents[side].mComponent->Update(0.0f);
                    mNameComponents[side].SetCaptainName(GetLOCCharacterName(mCaptainGridComponents[side]->GetSelectedItem(), false, true));
                    mNameComponents[side].SetCaptainLogo(GetTeamName(mCaptainGridComponents[side]->GetSelectedItem()));
                }
                break;

            case PHASE_CHOOSING_SIDEKICK:
                mSidekickGridComponents[side]->Update(inputpad);
                if (mSidekickGridComponents[side]->mHasChangedSinceLastUpdate)
                {
                    mNameComponents[side].mComponent->SetActiveSlide("Slide2");
                    mNameComponents[side].mComponent->Update(0.0f);
                    mNameComponents[side].SetCaptainName(GetLOCCharacterName((eTeamID)mHomeAwayTeam[side], false, false));
                    mNameComponents[side].SetSidekickName(GetLOCSidekickName(mSidekickGridComponents[side]->GetSelectedItem()));
                    mNameComponents[side].SetCaptainLogo(GetTeamName((eTeamID)mHomeAwayTeam[side]));
                }
                break;
            }
        }
    }

    if (mComponentState[0].mCurrentPhase == PHASE_READY && mComponentState[1].mCurrentPhase == PHASE_READY)
    {
        GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;

        gim->SetTeam(0, (eTeamID)mHomeAwayTeam[0]);
        gim->SetTeam(1, (eTeamID)mHomeAwayTeam[1]);
        gim->SetSidekick(0, (eSidekickID)mHomeAwaySidekicks[0]);
        gim->SetSidekick(1, (eSidekickID)mHomeAwaySidekicks[1]);

        IChooseCaptain* p = this;
        for (int i = 0; i < mNumTotalPushedPlayers; i++)
        {
            gim->SetPlayingSide((unsigned short)p->mAllPushedPlayers[0], (short)p->mAllPushedPlayerSides[0]);
            p = (IChooseCaptain*)((u8*)p + 4);
        }

        return UPDATE_GO_FORWARD;
    }

    for (int j = 0; j < 3; j++)
    {
        mAsyncImage[0][j]->Update(false);
        mAsyncImage[1][j]->Update(false);
    }

    for (int i = 0; i < 2; i++)
    {
        if (mComponentState[i].mCurrentPhase != PHASE_READY)
        {
            mCaptainSoundDelay[i] = 0.0f;
        }
        else
        {
            bool canswapcaptains = false;
            if (!mDidSwapCaptains[i])
            {
                if (mAsyncImage[i][0]->CanSwapTextures() && mAsyncImage[i][1]->CanSwapTextures() && mAsyncImage[i][2]->CanSwapTextures())
                {
                    canswapcaptains = true;
                }
            }

            if (canswapcaptains)
            {
                mCaptainComponents[i]->SetActiveSlide("Slide1");
                mCaptainComponents[i]->m_bVisible = true;
                mAsyncImage[i][0]->Update(true);
                mAsyncImage[i][1]->Update(true);
                mAsyncImage[i][2]->Update(true);
                mDidSwapCaptains[i] = true;
                mCaptainSoundDelay[i] = mCaptainSlideDurations[0];
            }
        }
    }

    return UPDATE_OK;
}

/**
 * Offset/Address/Size: 0x12F0 | 0x800BEC8C | size: 0x12C
 */
void IChooseCaptain::UpdateAsyncImages()
{
    int j;
    int i;
    bool canswapcaptains;

    for (j = 0; j < 3; j++)
    {
        mAsyncImage[0][j]->Update(false);
        mAsyncImage[1][j]->Update(false);
    }

    for (i = 0; i < 2; i++)
    {
        if (mComponentState[i].mCurrentPhase != PHASE_READY)
        {
            mCaptainSoundDelay[i] = 0.0f;
        }
        else
        {
            canswapcaptains = false;
            if (!mDidSwapCaptains[i])
            {
                if (mAsyncImage[i][0]->CanSwapTextures() && mAsyncImage[i][1]->CanSwapTextures() && mAsyncImage[i][2]->CanSwapTextures())
                {
                    canswapcaptains = true;
                }
            }

            if (canswapcaptains)
            {
                mCaptainComponents[i]->SetActiveSlide("Slide1");
                mCaptainComponents[i]->m_bVisible = true;
                mAsyncImage[i][0]->Update(true);
                mAsyncImage[i][1]->Update(true);
                mAsyncImage[i][2]->Update(true);
                mDidSwapCaptains[i] = true;
                mCaptainSoundDelay[i] = mCaptainSlideDurations[0];
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xAB0 | 0x800BE44C | size: 0x840
 */
void IChooseCaptain::SceneCreated(FEPresentation*)
{
}

/**
 * Offset/Address/Size: 0x890 | 0x800BE22C | size: 0x220
 * TODO: 96.7% match - remaining diffs are prologue r23/r31 register allocation and
 * float register ordering in the slide-duration setup path.
 * File uses -inline deferred but decomp.me tests with -inline auto.
 */
void IChooseCaptain::SetupCaptainComponent(TLComponentInstance* compinstance, int homeaway)
{
    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    mCaptainComponents[homeaway] = compinstance;

    TLSlide* slide = compinstance->GetActiveSlide();
    mCaptainSlideDurations[homeaway] = (slide->m_start + slide->m_duration) / 2.0f;

    {
        InlineHasher* p1 = (InlineHasher*)&h1;
        InlineHasher* p3 = (InlineHasher*)&h3;
        InlineHasher* p5 = (InlineHasher*)&h5;
        InlineHasher* p7 = (InlineHasher*)&h7;
        InlineHasher* p9 = (InlineHasher*)&h9;

        h0.m_Hash = 0;
        h2.m_Hash = 0;
        h4.m_Hash = 0;
        h6.m_Hash = 0;
        h1.m_Hash = 0;
        h3.m_Hash = 0;
        h5.m_Hash = 0;
        h7.m_Hash = 0;
        h8.m_Hash = 0;
        h9.m_Hash = 0;

        const char* name = (homeaway == 0) ? "CAPT_L" : "CAPT_R";

        unsigned long hash = nlStringLowerHash(name);
        hA.m_Hash = hash;
        hB.m_Hash = hash;

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
        mAsyncImage[homeaway][0]->mImageInstance = findImage.byRef(
            compinstance->GetActiveSlide(),
            *(InlineHasher*)&hB,
            *p9,
            *p7,
            *p5,
            *p3,
            *p1);
    }

    {
        volatile InlineHasher nB, nA, n8, n6, n4, n2, n0;

        InlineHasher* p1 = (InlineHasher*)&h1;
        InlineHasher* p3 = (InlineHasher*)&h3;
        InlineHasher* p5 = (InlineHasher*)&h5;
        InlineHasher* p7 = (InlineHasher*)&h7;
        InlineHasher* p9 = (InlineHasher*)&h9;

        n0.m_Hash = 0;
        n2.m_Hash = 0;
        h1.m_Hash = 0;
        h3.m_Hash = 0;
        n4.m_Hash = 0;
        h5.m_Hash = 0;
        n6.m_Hash = 0;
        h7.m_Hash = 0;
        n8.m_Hash = 0;
        h9.m_Hash = 0;

        const char* name = (homeaway == 0) ? "CAPT_L_OUT" : "CAPT_R_OUT";

        unsigned long hash = nlStringLowerHash(name);
        nA.m_Hash = hash;
        nB.m_Hash = hash;

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
        mAsyncImage[homeaway][1]->mImageInstance = findImage.byRef(
            compinstance->GetActiveSlide(),
            *(InlineHasher*)&nB,
            *p9,
            *p7,
            *p5,
            *p3,
            *p1);
    }

    {
        volatile InlineHasher mB, mA, m8, m6, m4, m2, m0;

        InlineHasher* p1 = (InlineHasher*)&h1;
        InlineHasher* p3 = (InlineHasher*)&h3;
        InlineHasher* p5 = (InlineHasher*)&h5;
        InlineHasher* p7 = (InlineHasher*)&h7;
        InlineHasher* p9 = (InlineHasher*)&h9;

        m0.m_Hash = 0;
        h1.m_Hash = 0;
        m2.m_Hash = 0;
        h3.m_Hash = 0;
        m4.m_Hash = 0;
        h5.m_Hash = 0;
        m6.m_Hash = 0;
        h7.m_Hash = 0;
        m8.m_Hash = 0;
        h9.m_Hash = 0;

        const char* name = (homeaway == 0) ? "CAPT_L_WHITE" : "CAPT_R_WHITE";

        unsigned long hash = nlStringLowerHash(name);
        mA.m_Hash = hash;
        mB.m_Hash = hash;

        findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
        mAsyncImage[homeaway][2]->mImageInstance = findImage.byRef(
            compinstance->GetActiveSlide(),
            *(InlineHasher*)&mB,
            *p9,
            *p7,
            *p5,
            *p3,
            *p1);
    }
}

/**
 * Offset/Address/Size: 0x70C | 0x800BE0A8 | size: 0x184
 * TODO: 94.4% match - r30/r31 allocation remains swapped across the sidekick index/homeaway offset path and source texture pointer lifetime.
 */
void IChooseCaptain::StartSidekickMiniHead(int homeaway, eSidekickID sidekick)
{
    static const char* SidekickImageNames[] = {
        "toad_minihead",
        "koopa_minihead",
        "hammerbros_minihead",
        "birdo_minihead",
    };

    static const char* SidekickDestImageNames[] = {
        "sidekick_minihead",
        "sidekick_minihead",
    };

    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    FETextureResource* sourceres;
    TLComponentInstance* component;
    TLSlide* activeslide;
    TLImageInstance* sourceimage;
    TLImageInstance* destimage;

    if (sidekick == SK_MYSTERY)
    {
        mSidekickMiniHeadComponents[homeaway]->m_bVisible = false;
        return;
    }

    component = mSidekickMiniHeadComponents[homeaway];
    activeslide = component->GetActiveSlide();
    component->SetActiveSlide(activeslide);
    component->Update(0.0f);
    component->m_bVisible = true;

    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

    TLComponentInstance* sourcecomp = mSidekickGridComponents[homeaway]->mParentComponent;

    h0.m_Hash = 0;
    h2.m_Hash = 0;
    h1.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;
    h8.m_Hash = 0;
    h9.m_Hash = 0;

    unsigned long hash = nlStringLowerHash(SidekickImageNames[sidekick]);
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    sourceimage = findImage.byRef(
        sourcecomp->GetActiveSlide(),
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);
    sourceres = sourceimage->m_pTextureResource;

    volatile InlineHasher g7, g6;
    volatile InlineHasher g4, g3, g2, g1, g0;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash(SidekickDestImageNames[homeaway]);
    g6.m_Hash = hash;
    g7.m_Hash = hash;

    destimage = findImage.byRef(
        component->GetActiveSlide(),
        (InlineHasher&)g7,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    destimage->m_component->pChildren = (TLSlide*)sourceres;
}

/**
 * Offset/Address/Size: 0x670 | 0x800BE00C | size: 0x9C
 */
void IChooseCaptain::CheckForDisconnectedHumanPlayers()
{
    for (int i = 0; i < 4; i++)
    {
        if (IsPlayerPushed(i))
        {
            if (!g_pFEInput->IsConnected((eFEINPUT_PAD)i))
            {
                PopPlayer((eFEINPUT_PAD)i);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x4E0 | 0x800BDE7C | size: 0x190
 * TODO: 99.95% match - remaining `bl` immediate mismatch on `SetCurrentPhase` call
 * (likely enum/signature mangling drift for `ComponentState::Phase`).
 */
void IChooseCaptain::FindAliveHumanPlayers()
{
    for (int i = 0; i < 4; i++)
    {
        eFEINPUT_PAD pad = (eFEINPUT_PAD)i;

        if (g_pFEInput->IsAutoPressed(pad, 0xB, true, NULL) || g_pFEInput->IsAutoPressed(pad, 0xC, true, NULL)
            || g_pFEInput->IsAutoPressed(pad, 0xD, true, NULL) || g_pFEInput->IsAutoPressed(pad, 0xE, true, NULL)
            || g_pFEInput->JustPressed(pad, 0x100, false, NULL))
        {
            int numPushedPlayers = mNumTotalPushedPlayers;

            if (!IsPlayerPushed(i))
            {
                int side = numPushedPlayers & 1;
                if (mComponentState[side].mCurrentPhase == PHASE_IDLE)
                {
                    mComponentState[side].SetCurrentPhase(PHASE_CHOOSING_CAPTAIN);
                }

                mAllPushedPlayers[mNumTotalPushedPlayers] = (eFEINPUT_PAD)i;
                if (side != -1)
                {
                    mAllPushedPlayerSides[mNumTotalPushedPlayers] = side;
                }
                else
                {
                    mAllPushedPlayerSides[mNumTotalPushedPlayers] = mNumTotalPushedPlayers & 1;
                }

                mNumTotalPushedPlayers++;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x3DC | 0x800BDD24 | size: 0x158
 * TODO: 96.2% match - first counting loop still has r3/r6 swap (numSide1 vs array traversal),
 * and prologue/branch layout differs around that loop due MWCC register reuse.
 */
void IChooseCaptain::SetupForLastPhase(eFEINPUT_PAD pad)
{
    int numSide0;
    mIsSinglePlayerInput = numSide0 = 0;

    if (mNumTotalPushedPlayers == 1)
    {
        mIsSinglePlayerInput = true;
    }
    else
    {
        int numSide1 = numSide0;
        for (int i = 0; i < mNumTotalPushedPlayers; i++)
        {
            if (mAllPushedPlayerSides[i] == 0)
            {
                numSide0++;
            }
            else if (mAllPushedPlayerSides[i] == 1)
            {
                numSide1++;
            }
        }
        if (numSide0 == 0 || numSide1 == 0)
        {
            mIsSinglePlayerInput = true;
        }
    }

    if (mIsSinglePlayerInput)
    {
        mComponentState[1].GotoPreviousPhase();
        return;
    }

    if (pad == FE_ALL_PADS)
    {
        mComponentState[0].GotoPreviousPhase();
        mComponentState[1].GotoPreviousPhase();
        return;
    }

    int side;
    if (mIsSinglePlayerInput)
    {
        if (mComponentState[0].mCurrentPhase < PHASE_READY)
        {
            side = 0;
        }
        else
        {
            side = 1;
        }
    }
    else
    {
        int i = 0;
        IChooseCaptain* p = this;
        for (; i < mNumTotalPushedPlayers; i++)
        {
            if (p->mAllPushedPlayers[0] == pad)
            {
                side = mAllPushedPlayerSides[i];
                goto found;
            }
            p = (IChooseCaptain*)((u8*)p + 4);
        }
        side = -1;
    }

found:
    if (side == -1)
    {
        mComponentState[1].GotoPreviousPhase();
    }
    else
    {
        mComponentState[side].GotoPreviousPhase();
    }
}

/**
 * Offset/Address/Size: 0x338 | 0x800BDCD4 | size: 0x50
 */
void IChooseCaptain::PushPlayer(eFEINPUT_PAD pad, int side)
{
    mAllPushedPlayers[mNumTotalPushedPlayers] = pad;
    if (side != -1)
    {
        mAllPushedPlayerSides[mNumTotalPushedPlayers] = side;
    }
    else
    {
        mAllPushedPlayerSides[mNumTotalPushedPlayers] = mNumTotalPushedPlayers & 1;
    }
    mNumTotalPushedPlayers++;
}

/**
 * Offset/Address/Size: 0x1DC | 0x800BDB78 | size: 0x15C
 * TODO: 98.4% match - still emits lbz reload for side-count init in else path,
 *       and mIsSinglePlayerInput=false stores from r0 (target uses r4)
 */
void IChooseCaptain::PopPlayer(eFEINPUT_PAD pad)
{
    int foundIndex = 0;
    int idx = 0;
    for (int i = 0; i < mNumTotalPushedPlayers; i++, idx++)
    {
        if (mAllPushedPlayers[i] == pad)
        {
            foundIndex = idx;
            break;
        }
    }

    for (int i = foundIndex; i < mNumTotalPushedPlayers - 1; i++)
    {
        mAllPushedPlayers[i] = mAllPushedPlayers[i + 1];
        mAllPushedPlayerSides[i] = mAllPushedPlayerSides[i + 1];
    }

    mNumTotalPushedPlayers--;
    mIsSinglePlayerInput = false;

    if (mNumTotalPushedPlayers == 1)
    {
        mIsSinglePlayerInput = true;
    }
    else
    {
        int side1Count = mIsSinglePlayerInput;
        int side0Count = side1Count;
        IChooseCaptain* p = this;
        for (int i = 0; i < mNumTotalPushedPlayers; i++)
        {
            if (p->mAllPushedPlayerSides[0] == 0)
            {
                side0Count++;
            }
            else if (p->mAllPushedPlayerSides[0] == 1)
            {
                side1Count++;
            }
            p = (IChooseCaptain*)((u8*)p + 4);
        }
        if (!side0Count || !side1Count)
        {
            mIsSinglePlayerInput = true;
        }
    }

    if (mNumTotalPushedPlayers != 0 && mIsSinglePlayerInput && mComponentState[1].mCurrentPhase != PHASE_READY)
    {
        if (mComponentState[0].mCurrentPhase != PHASE_READY)
        {
            mComponentState[1].SetCurrentPhase(PHASE_IDLE);
        }

        if (mNumTotalPushedPlayers == 1)
        {
            mAllPushedPlayerSides[0] = 0;
        }
    }
}

/**
 * Offset/Address/Size: 0x1A8 | 0x800BDB44 | size: 0x34
 */
void IChooseCaptain::ResetPushPlayerData()
{
    mAllPushedPlayers[0] = FE_ALL_PADS;
    mAllPushedPlayerSides[0] = -1;
    mAllPushedPlayers[1] = FE_ALL_PADS;
    mAllPushedPlayerSides[1] = -1;
    mAllPushedPlayers[2] = FE_ALL_PADS;
    mAllPushedPlayerSides[2] = -1;
    mAllPushedPlayers[3] = FE_ALL_PADS;
    mAllPushedPlayerSides[3] = -1;
    mNumTotalPushedPlayers = 0;
}

/**
 * Offset/Address/Size: 0xD8 | 0x800BDA74 | size: 0xD0
 */
void IChooseCaptain::PushPlayerWithGameInfoDB()
{
    int i;
    int side;

    for (i = 0; i < 4; i++)
    {
        side = nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide(i);
        if (g_pFEInput->IsConnected((eFEINPUT_PAD)i))
        {
            if (side != -1)
            {
                mAllPushedPlayers[mNumTotalPushedPlayers] = (eFEINPUT_PAD)i;
                if (side != -1)
                {
                    mAllPushedPlayerSides[mNumTotalPushedPlayers] = side;
                }
                else
                {
                    mAllPushedPlayerSides[mNumTotalPushedPlayers] = mNumTotalPushedPlayers & 1;
                }
                mNumTotalPushedPlayers++;
            }
        }
        else
        {
            nlSingleton<GameInfoManager>::s_pInstance->SetPlayingSide(i, -1);
        }
    }
}

/**
 * Offset/Address/Size: 0x8C | 0x800BDA28 | size: 0x4C
 */
void IChooseCaptain::MoveHighlightToCurrentCaptain(int which)
{
    if (which == 0 || which == 1)
    {
        mCaptainGridComponents[which]->MoveHighlightToTarget((eTeamID)mHomeAwayTeam[which]);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800BD99C | size: 0x8C
 */
void IChooseCaptain::SetupNameComponentToCurrentCaptain(int slot)
{
    if (slot == 0 || slot == 1)
    {
        mNameComponents[slot].SetCaptainName(GetLOCCharacterName((eTeamID)mHomeAwayTeam[slot], false, false));
        mNameComponents[slot].SetCaptainLogo(GetTeamName((eTeamID)mHomeAwayTeam[slot]));
    }
}

// REMOVE stub once real callers exist
void feCaptainComponent_stub()
{
    IGridComponent<eSidekickID>* p1 = 0;
    p1->RebindHighliteComponent(0);
    IGridComponent<eTeamID>* p2 = 0;
    p2->RebindHighliteComponent(0);
}
