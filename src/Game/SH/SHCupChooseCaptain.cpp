#include "Game/SH/SHCupChooseCaptain.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "Game/FE/feHelpFuncs.h"

static unsigned long CAPTAIN_DESCRIPTIONS[] = {
    0xFF68ABBA,
    0xE2D37C19,
    0x000465BA,
    0x000BAD38,
    0x0043DF21,
    0x330C3072,
    0x00C0A242,
    0x00EC84AC,
    0x69BFAF9D,
};

// /**
//  * Offset/Address/Size: 0x4EC | 0x800DF878 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800DF7F4 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800DF7BC | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800DF660 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800DF5DC | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800DF5A4 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800DF448 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800DF3C4 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800DF38C | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x237C | 0x800DF248 | size: 0x144
 */
CupChooseCaptainSceneV2::CupChooseCaptainSceneV2(bool isSuperCup)
    : mSidekickMiniHead(NULL)
    , mCurrentCaptain((eTeamID)3)
    , mCurrentSK((eSidekickID)0)
    , mState(CUP_STATE_CAPTAIN)
    , mAnimationDelay(0.75f)
    , mSoundDelay(0.0f)
    , mRemainingSoundDelay(0.0f)
    , mTicker(NULL)
    , mIsSuperCup(isSuperCup)
    , mCupStartString()
{
    const char* asyncPath = mIsSuperCup ? "art/fe/SuperCupLoadingScreensUI.res" : "art/fe/CupLoadingScreensUI.res";

    AsyncImage* image = new (0x20, true) AsyncImage(asyncPath, NULL);
    mCaptainImageMain = image;

    image = new (0x20, true) AsyncImage(asyncPath, NULL);
    mCaptainImageBG = image;

    image = new (0x20, true) AsyncImage(asyncPath, NULL);
    mCaptainImageFlash = image;
}

/**
 * Offset/Address/Size: 0x2168 | 0x800DF034 | size: 0x214
 */
CupChooseCaptainSceneV2::~CupChooseCaptainSceneV2()
{
    delete mCaptainImageMain;
    delete mCaptainImageBG;
    delete mCaptainImageFlash;
    delete mCaptainGrid;
    delete mSKGrid;

    if (mTicker)
    {
        delete mTicker;
    }
}

/**
 * Offset/Address/Size: 0x1654 | 0x800DE520 | size: 0xB14
 */
void CupChooseCaptainSceneV2::SceneCreated()
{
}

/**
 * Offset/Address/Size: 0x109C | 0x800DDF68 | size: 0x5B8
 */
void CupChooseCaptainSceneV2::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    if (mCaptainImageMain->Update(true))
    {
        mNumImagesLoaded++;
    }
    if (mCaptainImageBG->Update(true))
    {
        mNumImagesLoaded++;
    }
    if (mCaptainImageFlash->Update(true))
    {
        mNumImagesLoaded++;
    }

    static const char* lastCaptainSelectSoundStrPlayed = 0;

    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();
    TLSlide* currentSlide = pres->m_currentSlide;

    if (mRemainingSoundDelay > 0.0f)
    {
        mRemainingSoundDelay -= fDeltaT;
        if (mRemainingSoundDelay <= 0.0f)
        {
            mRemainingSoundDelay = 0.0f;
            FECharacterSound::PlayCaptainSlideIn(mCurrentCaptain);
        }
    }

    if (currentSlide->m_time >= 1.0)
    {
        mTicker->Update(fDeltaT);
    }
    mCaptainGrid->UpdateSuperTeamIconState();

    if (mAnimationDelay > 0.0f)
    {
        mAnimationDelay -= fDeltaT;
        return;
    }

    switch (mState)
    {
    case CUP_STATE_CAPTAIN:
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            if (mCurrentCaptain != TEAM_MYSTERY)
            {
                ChangeState(mState, CUP_STATE_SK);
                FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
                lastCaptainSelectSoundStrPlayed = FECharacterSound::PlayCaptainName(mCurrentCaptain);
            }
            else if (mCurrentCaptain == TEAM_MYSTERY && nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
            {
                ChangeState(mState, CUP_STATE_READY);
                FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
                lastCaptainSelectSoundStrPlayed = FECharacterSound::PlayCaptainName(mCurrentCaptain);
            }
            else
            {
                FEAudio::PlayAnimAudioEvent("sfx_deny", false);
            }
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            if (mIsSuperCup)
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_SUPER, SCREEN_BACK, true);
            }
            else
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_OPTIONS_INITIAL_CUP, SCREEN_BACK, true);
            }
            FEAudio::PlayAnimAudioEvent("sfx_back", false);
        }
        else
        {
            mCaptainGrid->Update(FE_ALL_PADS);
            if (mCaptainGrid->mHasChangedSinceLastUpdate)
            {
                mCurrentCaptain = mCaptainGrid->GetSelectedItem();
                mComponents[0]->SetActiveSlide("Slide1");
                UpdateCaptainName();
                mComponents[0]->Update(0.0f);
            }
        }
        break;
    }
    case CUP_STATE_SK:
    {
        if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
        {
            ChangeState(mState, CUP_STATE_READY);
            FEAudio::PlayAnimAudioEvent("sfx_accept_no_screen_change", false);
            if (lastCaptainSelectSoundStrPlayed)
            {
                FEAudio::StopAnimAudioEvent(lastCaptainSelectSoundStrPlayed);
                lastCaptainSelectSoundStrPlayed = NULL;
            }
            FECharacterSound::PlaySidekickName(mCurrentSK);
        }
        else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
        {
            ChangeState(mState, CUP_STATE_CAPTAIN);
            FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
        }
        else
        {
            mSKGrid->Update(FE_ALL_PADS);
            if (mSKGrid->mHasChangedSinceLastUpdate)
            {
                mCurrentSK = mSKGrid->GetSelectedItem();
                mComponents[0]->SetActiveSlide("Slide2");
                UpdateSKName();
                mComponents[0]->Update(0.0f);
            }
        }
        break;
    }
    case CUP_STATE_READY:
    {
        if (mNumImagesLoaded != 3)
        {
            mComponents[2]->SetActiveSlide(mComponents[2]->GetActiveSlide());
            mComponents[2]->Update(0.0f);
            mComponents[4]->SetActiveSlide(mComponents[4]->GetActiveSlide());
            mComponents[4]->Update(0.0f);
            if (mCurrentCaptain != TEAM_MYSTERY)
            {
                StartSidekickMiniHead(mCurrentSK);
                mSidekickMiniHead->m_bVisible = true;
            }
            else
            {
                mSidekickMiniHead->m_bVisible = false;
            }
        }
        else
        {
            if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
            {
                GameInfoManager* const pGameInfo = nlSingleton<GameInfoManager>::s_pInstance;
                pGameInfo->SetUserSelectedCupTeam(mCurrentCaptain);
                pGameInfo->SetUserSelectedCupSidekick(mCurrentSK);
                CreateLineup();
                pGameInfo->IncreaseRoundNumber();
                pGameInfo->SetResultsOfLastUserGame(RESULT_CUP_START);
                nlSingleton<GameSceneManager>::s_pInstance->Pop();
                SceneList nextScene = mIsSuperCup ? SCENE_SUPER_CUP_STANDINGS : SCENE_CUP_STANDINGS;
                BaseSceneHandler* hubScene = nlSingleton<GameSceneManager>::s_pInstance->Push(nextScene, SCREEN_FORWARD, false);
                *(u8*)((u8*)hubScene + 0x226) = 1;
                FEAudio::PlayAnimAudioEvent("sfx_accept", false);
            }
            else if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x200, false, NULL))
            {
                if (mCurrentCaptain != TEAM_MYSTERY)
                {
                    ChangeState(mState, CUP_STATE_SK);
                }
                else
                {
                    ChangeState(mState, CUP_STATE_CAPTAIN);
                }
                FEAudio::PlayAnimAudioEvent("sfx_back_no_screen_change", false);
            }
        }
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0xE9C | 0x800DDD68 | size: 0x200
 */
void CupChooseCaptainSceneV2::UpdateCaptainName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

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

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, true);
    captainNameText->m_OverloadFlags |= 0x8;

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

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDescText = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == 8 && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDescText->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDescText->m_LocStrId = CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDescText->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* teamComp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    teamComp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0xC20 | 0x800DDAEC | size: 0x27C
 */
void CupChooseCaptainSceneV2::UpdateSKName()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    volatile InlineHasher iB, iA;
    volatile InlineHasher i4, i3, i2, i1, i0;

    volatile InlineHasher gB, gA;
    volatile InlineHasher g4, g3, g2, g1, g0;

    volatile InlineHasher fB, fA;
    volatile InlineHasher f4, f3, f2, f1, f0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
    TLSlide* slide = mComponents[0]->GetActiveSlide();

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

    unsigned long hash = nlStringLowerHash("CAPTAIN_NAME");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    TLTextInstance* captainNameText = findText.byRef(
        slide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    captainNameText->m_LocStrId = GetLOCCharacterName(mCurrentCaptain, false, false);
    captainNameText->m_OverloadFlags |= 0x8;

    i0.m_Hash = 0;
    h1.m_Hash = 0;
    i1.m_Hash = 0;
    h3.m_Hash = 0;
    i2.m_Hash = 0;
    h5.m_Hash = 0;
    i3.m_Hash = 0;
    h7.m_Hash = 0;
    i4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("SIDEKICK_NAME");
    iA.m_Hash = hash;
    iB.m_Hash = hash;

    TLTextInstance* sidekickNameText = findText.byRef(
        slide,
        (InlineHasher&)iB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    sidekickNameText->m_LocStrId = GetLOCSidekickName(mCurrentSK);
    sidekickNameText->m_OverloadFlags |= 0x8;

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

    hash = nlStringLowerHash("CAPTAIN_NAME2");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    TLTextInstance* captainDescText = findText.byRef(
        slide,
        (InlineHasher&)gB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mCurrentCaptain == 8 && !nlSingleton<GameInfoManager>::s_pInstance->IsSuperTeamUnlocked())
    {
        captainDescText->SetStringId("CUP_ATTR_MYSTERY_LOCKED");
    }
    else
    {
        captainDescText->m_LocStrId = CAPTAIN_DESCRIPTIONS[mCurrentCaptain];
        captainDescText->m_OverloadFlags |= 0x8;
    }

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    f0.m_Hash = 0;
    h1.m_Hash = 0;
    f1.m_Hash = 0;
    h3.m_Hash = 0;
    f2.m_Hash = 0;
    h5.m_Hash = 0;
    f3.m_Hash = 0;
    h7.m_Hash = 0;
    f4.m_Hash = 0;
    h9.m_Hash = 0;

    hash = nlStringLowerHash("COMPONENT");
    fA.m_Hash = hash;
    fB.m_Hash = hash;

    TLComponentInstance* teamComp = findComp.byRef(
        slide,
        (InlineHasher&)fB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    teamComp->SetActiveSlide(GetTeamName(mCurrentCaptain));
}

/**
 * Offset/Address/Size: 0x330 | 0x800DD1FC | size: 0x8F0
 */
void CupChooseCaptainSceneV2::ChangeState(CupChooseCaptainSceneV2::eCupCaptainState, CupChooseCaptainSceneV2::eCupCaptainState)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x15C | 0x800DD028 | size: 0x1D4
 * TODO: 98.8% match - remaining register allocation mismatch is in
 * numPlayingTeams/slot and lineup/sklineup traversal registers
 * (target r26/r31 + r29/r25/r27 vs current r31/r27 + r25/r26/r29).
 */
void CupChooseCaptainSceneV2::CreateLineup()
{
    int numPlayingTeams;
    u32 slot;
    GameInfoManager* const pGameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    eSidekickID chosenSidekick;
    eTeamID chosenCaptain;

    chosenCaptain = mCurrentCaptain;
    chosenSidekick = mCurrentSK;

    numPlayingTeams = pGameInfo->GetNumPlayingTeams();
    slot = nlRandom((u32)numPlayingTeams, &nlDefaultSeed);

    eTeamID lineup[8];
    eSidekickID sklineup[8];

    int i;
    for (i = 0; i < numPlayingTeams; i++)
    {
        lineup[i] = (eTeamID)-1;
        sklineup[i] = (eSidekickID)-1;
    }

    lineup[slot] = chosenCaptain;
    sklineup[slot] = chosenSidekick;

    for (i = 0; i < numPlayingTeams; i++)
    {
        if ((u32)i == slot)
        {
            continue;
        }

        eTeamID teamChoice;
        u8 notAlreadyChosen;
        do
        {
            if (pGameInfo->IsInSuperCupMode())
            {
                teamChoice = (eTeamID)nlRandom(9, &nlDefaultSeed);
            }
            else
            {
                teamChoice = (eTeamID)nlRandom(8, &nlDefaultSeed);
            }

            notAlreadyChosen = 1;
            for (int k = 0; k < numPlayingTeams; k++)
            {
                if (teamChoice == lineup[k])
                {
                    notAlreadyChosen = 0;
                    break;
                }
            }
        } while (!notAlreadyChosen);

        lineup[i] = teamChoice;
        sklineup[i] = (eSidekickID)nlRandom(4, &nlDefaultSeed);
    }

    pGameInfo->SetupRoundRobinSchedule(lineup, sklineup);
}

/**
 * Offset/Address/Size: 0x0 | 0x800DCECC | size: 0x15C
 * TODO: 97.6% match - remaining mismatch is the early SidekickImageNames load
 * scheduling (`lis` for the table base before `li r5, 0`) plus local symbol
 * labeling (`SidekickImageNames$3` vs `SidekickImageNames$1112`) in scratch.
 */
void CupChooseCaptainSceneV2::StartSidekickMiniHead(eSidekickID sidekickId)
{
    static const char* SidekickImageNames[] = {
        "toad_minihead",
        "koopa_minihead",
        "hammerbros_minihead",
        "birdo_minihead",
    };

    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    if (sidekickId == SK_MYSTERY)
    {
        mSidekickMiniHead->m_bVisible = false;
        return;
    }

    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
    TLComponentInstance* parentComp = mSKGrid->mParentComponent;

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

    const char** pNames = SidekickImageNames;
    unsigned long hash = nlStringLowerHash(pNames[sidekickId]);
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    FETextureResource* texRes = findImage.byRef(
                                             parentComp->GetActiveSlide(),
                                             (InlineHasher&)hB,
                                             (InlineHasher&)h9,
                                             (InlineHasher&)h7,
                                             (InlineHasher&)h5,
                                             (InlineHasher&)h3,
                                             (InlineHasher&)h1)
                                    ->m_pTextureResource;

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

    hash = nlStringLowerHash("sidekick_minihead");
    g7.m_Hash = hash;
    g6.m_Hash = hash;

    TLImageInstance* miniHead = findImage.byRef(
        mSidekickMiniHead->GetActiveSlide(),
        (InlineHasher&)g7,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    miniHead->m_component->pChildren = (TLSlide*)texRes;

    TLSlide* slide = mSidekickMiniHead->GetActiveSlide();
    mSidekickMiniHead->SetActiveSlide(slide);
    mSidekickMiniHead->Update(0.0f);
    mSidekickMiniHead->m_bVisible = true;
}
