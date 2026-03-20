#include "Game/SH/SHChooseCaptains.h"

#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/GameSceneManager.h"
#include "NL/nlConfig.h"

extern bool g_e3_Build;

/**
 * Offset/Address/Size: 0x0 | 0x800D8554 | size: 0x2C
 */
void IChooseCaptain::SetPhaseReady(int homeaway)
{
    mComponentState[homeaway].SetCurrentPhase(PHASE_READY);
}

// /**
//  * Offset/Address/Size: 0xAB0 | 0x800D83F8 | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xA2C | 0x800D8374 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x9F4 | 0x800D833C | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x898 | 0x800D81E0 | size: 0x15C
//  */
// void FEFinder<TLInstance, 5>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x814 | 0x800D815C | size: 0x84
//  */
// void FEFinder<TLInstance, 5>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x7DC | 0x800D8124 | size: 0x38
//  */
// void FEFinder<TLInstance, 5>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800D7FC8 | size: 0x15C
//  */
// void FEFinder<TLInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800D7F90 | size: 0x38
//  */
// void FEFinder<TLInstance, 3>::Find<TLInstance>(TLInstance*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800D7E34 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800D7DB0 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800D7D78 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800D7C1C | size: 0x15C
//  */
// void FEFinder<TLInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800D7B98 | size: 0x84
//  */
// void FEFinder<TLInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800D7B60 | size: 0x38
//  */
// void FEFinder<TLInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800D7A04 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800D7980 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D7948 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0xE64 | 0x800D78AC | size: 0x9C
 */
#pragma inline_depth(8)
ChooseCaptainsSceneV2::ChooseCaptainsSceneV2(ChooseCaptainsSceneV2::SceneType sceneType)
    : mSceneType(sceneType)
    , mDesiredSceneType(sceneType)
    , mTicker(NULL)
    , mMoveForwardFrameDelay(-1)
{
}

/**
 * Offset/Address/Size: 0xD20 | 0x800D7768 | size: 0x144
 */
#pragma inline_depth(8)
ChooseCaptainsSceneV2::~ChooseCaptainsSceneV2()
{
    FEScrollText* ticker = mTicker;

    if (ticker != NULL)
    {
        if (ticker != NULL)
        {
            if ((char*)ticker + 0x21C)
            {
                volatile FEScrollText* vticker = ticker;
                if ((char*)vticker + 0x21C)
                {
                    if (ticker->m_messageFinishedCB.mTag == FUNCTOR)
                    {
                        delete ticker->m_messageFinishedCB.mFunctor;
                    }
                    ticker->m_messageFinishedCB.mTag = EMPTY;
                }
            }

            if ((char*)ticker + 4)
            {
                BasicStringData<unsigned short>* data = ticker->m_message.m_data;
                if (data != NULL)
                {
                    if (--data->mRefCount == 0)
                    {
                        if (data != NULL)
                        {
                            if (data != NULL)
                            {
                                delete[] data->mData;
                            }
                            if (data != NULL)
                            {
                                nlFree(data);
                            }
                        }
                    }
                }
            }

            ::operator delete(ticker);
        }
    }
}

/**
 * Offset/Address/Size: 0xC28 | 0x800D7670 | size: 0xF8
 */
#pragma inline_depth(0)
void ChooseCaptainsSceneV2::SceneCreated()
{
    mChooseCaptain.Initialize("art/fe/LoadingScreensUI.res", "art/fe/LoadingScreensSidekicksUI.res");
    mChooseCaptain.SceneCreated(m_pFEPresentation);

    if (mDesiredSceneType == SceneType_1)
    {
        mChooseCaptain.SetPhaseReady(0);
        mChooseCaptain.SetPhaseReady(1);
    }
    else
    {
        eFEINPUT_PAD pad = nlSingleton<GameInfoManager>::s_pInstance->mMainUserPadNumber;
        if (g_pFEInput->IsConnected(pad))
        {
            mChooseCaptain.PushPlayer(pad, -1);
        }

        GameInfoManager* gim = nlSingleton<GameInfoManager>::s_pInstance;
        gim->mCurGameGameplayOptions.SkillLevel = gim->mUserInfo.mGameplayOptions.SkillLevel;
        gim->mCurGameGameplayOptions.GameTime = gim->mUserInfo.mGameplayOptions.GameTime;
        gim->mCurGameGameplayOptions.PowerUps = gim->mUserInfo.mGameplayOptions.PowerUps;
        gim->mCurGameGameplayOptions.Shoot2Score = gim->mUserInfo.mGameplayOptions.Shoot2Score;
        gim->mCurGameGameplayOptions.BowserAttackEnabled = gim->mUserInfo.mGameplayOptions.BowserAttackEnabled;
        gim->mCurGameGameplayOptions.RumbleEnabled = gim->mUserInfo.mGameplayOptions.RumbleEnabled;
    }

    BindChooseSideInstances();
    CreateTicker();
    ChangeSceneType(mDesiredSceneType);
}
/**
 * Offset/Address/Size: 0xBE0 | 0x800D7628 | size: 0x48
 */
void ChooseCaptainsSceneV2::ChangeSceneType(ChooseCaptainsSceneV2::SceneType sceneType)
{
    mSceneType = sceneType;
    switch (mSceneType)
    {
    case 0:
        ResetForCHOOSECAPTAINS();
        break;
    case 1:
        ResetForCHOOSESIDES();
        break;
    }
}

/**
 * Offset/Address/Size: 0xA7C | 0x800D74C4 | size: 0x164
 */
void ChooseCaptainsSceneV2::ResetForCHOOSECAPTAINS()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union FindUnion
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    };

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    volatile unsigned long hB2, hA2;
    volatile unsigned long h92, h82, h72, h62, h52, h42, h32, h22, h12, h02;

    FindUnion findComp;
    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0 = 0;
    h1 = 0;
    h2 = 0;
    h3 = 0;
    h4 = 0;
    h5 = 0;
    h6 = 0;
    h7 = 0;

    unsigned long hash = nlStringLowerHash("CHOOSE_SIDE");
    h8 = hash;
    h9 = hash;

    hash = nlStringLowerHash("Layer");
    hB = hash;
    hA = hash;

    TLComponentInstance* comp = findComp.byRef(m_pFEPresentation->m_currentSlide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

    comp->m_bVisible = false;
    mTicker->SetDisplayMessage((unsigned long)0x4B67A61F);

    h02 = 0;
    h1 = 0;
    h22 = 0;
    h3 = 0;
    h42 = 0;
    h5 = 0;
    h62 = 0;
    h7 = 0;

    FindUnion findComp2;
    findComp2.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    hash = nlStringLowerHash("buttons");
    h82 = hash;
    h92 = hash;

    hash = nlStringLowerHash("Layer");
    hB2 = hash;
    hA2 = hash;

    TLComponentInstance* buttonsComp = findComp2.byRef(m_pFEPresentation->m_currentSlide, (InlineHasher&)hB2, (InlineHasher&)h92, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

    mButtons.mButtonInstance = buttonsComp;
    mButtons.SetState(ButtonComponent::BS_A_AND_B);

    mChooseCaptain.MoveHighlightToCurrentCaptain(0);
    mChooseCaptain.SetupNameComponentToCurrentCaptain(0);
    mChooseCaptain.MoveHighlightToCurrentCaptain(1);

    if (mChooseCaptain.mNumTotalPushedPlayers > 1)
    {
        mChooseCaptain.SetupNameComponentToCurrentCaptain(1);
    }
}

/**
 * Offset/Address/Size: 0x930 | 0x800D7378 | size: 0x14C
 */
void ChooseCaptainsSceneV2::ResetForCHOOSESIDES()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union FindUnion
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    };

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    volatile unsigned long hB2, hA2;
    volatile unsigned long h92, h82, h72, h62, h52, h42, h32, h22, h12, h02;

    FindUnion findComp;
    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    h0 = 0;
    h1 = 0;
    h2 = 0;
    h3 = 0;
    h4 = 0;
    h5 = 0;
    h6 = 0;
    h7 = 0;

    unsigned long hash = nlStringLowerHash("CHOOSE_SIDE");
    h8 = hash;
    h9 = hash;

    hash = nlStringLowerHash("Layer");
    hB = hash;
    hA = hash;

    TLComponentInstance* comp = findComp.byRef(m_pFEPresentation->m_currentSlide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

    comp->m_bVisible = true;

    mTicker->SetDisplayMessage((unsigned long)0x53B23764);

    if (!nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode && !g_e3_Build)
    {
        FindUnion findComp2;
        findComp2.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        h02 = 0;
        h1 = 0;
        h22 = 0;
        h3 = 0;
        h42 = 0;
        h5 = 0;
        h62 = 0;
        h7 = 0;

        hash = nlStringLowerHash("buttons");
        h82 = hash;
        h92 = hash;

        hash = nlStringLowerHash("Layer");
        hB2 = hash;
        hA2 = hash;

        TLComponentInstance* buttonsComp = findComp2.byRef(m_pFEPresentation->m_currentSlide, (InlineHasher&)hB2, (InlineHasher&)h92, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

        buttonsComp->m_bVisible = false;
        mButtons.mButtonInstance = buttonsComp;
        mButtons.SetState(ButtonComponent::BS_A_AND_B_AND_Y);
    }
}

/**
 * Offset/Address/Size: 0x560 | 0x800D6FA8 | size: 0x3D0
 * TODO: 99.59% match - single extra unreachable branch from -inline deferred vs -inline auto
 */
void ChooseCaptainsSceneV2::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();
    mChooseCaptain.UpdateSound(fDeltaT);
    FEPresentation* pPresentation = m_pFEScene->m_pFEPackage->GetPresentation();
    TLSlide* pCurrentSlide = pPresentation->m_currentSlide;
    if (mMoveForwardFrameDelay > 0)
    {
        mMoveForwardFrameDelay--;
        if (mMoveForwardFrameDelay != 0)
            return;
        if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        {
            nlSingleton<GameSceneManager>::s_pInstance->PushLoadingScene(true);
            nlSingleton<GameInfoManager>::s_pInstance->SetStadium((eStadiumID)0);
        }
        else if (g_e3_Build)
        {
            nlSingleton<GameSceneManager>::s_pInstance->PushLoadingScene(true);
            nlSingleton<GameInfoManager>::s_pInstance->SetStadium((eStadiumID)3);
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_STADIUM_SELECT, SCREEN_FORWARD, true);
        }
        mChooseSide.SaveChanges();
        FrontEnd::SetControllerState();
        mMoveForwardFrameDelay = -1;
        return;
    }
    if (pCurrentSlide->m_time >= 0.5)
    {
        mTicker->Update(fDeltaT);
    }
    if (pCurrentSlide->m_time <= 1.5 && mSceneType == SceneType_1)
    {
        mChooseCaptain.UpdateAsyncImages();
        return;
    }
    switch (mSceneType)
    {
    case SceneType_0:
        switch (mChooseCaptain.Update(fDeltaT))
        {
        case UPDATE_GO_BACK:
            nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_BACK, false);
            return;
        case UPDATE_GO_FORWARD:
            ChangeSceneType(SceneType_1);
            mChooseSide.ResetAndPositionControllers(true);
        default:
            return;
        }
    case SceneType_1:
    {
        eFEINPUT_PAD buttonPressed = FE_ALL_PADS;
        if (!nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
        {
            if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x800, false, NULL))
            {
                if (!g_e3_Build)
                {
                    nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_QUICK_GAMEPLAY_OPTIONS, SCREEN_FORWARD, false);
                    return;
                }
            }
        }
        UpdateResult sideResult = mChooseSide.Update(fDeltaT, &buttonPressed, -1);
        switch (sideResult)
        {
        case UPDATE_GO_BACK:
            ChangeSceneType(SceneType_0);
            mChooseSide.SaveChanges();
            mChooseCaptain.ResetPushPlayerData();
            mChooseCaptain.PushPlayerWithGameInfoDB();
            mChooseCaptain.SetupForLastPhase(buttonPressed);
            break;
        case UPDATE_GO_FORWARD:
        {
            if (mChooseSide.AllPlayersReady() || mChooseSide.AllPluggedInAreReady())
            {
                goto readyPath;
            }
            {
                Config& cfg = Config::Global();
                TagValuePair& tvp = cfg.FindTvp("allReady");
                bool allReady;
                if (tvp.tag == NULL)
                {
                    cfg.Set("allReady", false);
                    allReady = false;
                }
                else if (tvp.type == _BOOL)
                {
                    allReady = LexicalCast<bool, bool>(tvp.value.b);
                }
                else if (tvp.type == _INT)
                {
                    allReady = LexicalCast<bool, int>(tvp.value.i);
                }
                else if (tvp.type == _FLOAT)
                {
                    allReady = LexicalCast<bool, float>(tvp.value.f);
                }
                else if (tvp.type == _STRING)
                {
                    allReady = LexicalCast<bool, const char*>(tvp.value.s);
                }
                else
                {
                    allReady = false;
                }
                if (allReady)
                {
                readyPath:
                    TLInstance* inst = mChooseSide.mInstanceTable[16];
                    if (inst != NULL)
                    {
                        inst->m_bVisible = false;
                    }
                    mMoveForwardFrameDelay = 2;
                    return;
                }
            }
            if (!mChooseSide.AtLeastOnePlayerReady())
            {
                ((FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false))->Create(POPUP_NO_SIDES_CHOSEN);
            }
            break;
        }
        }
        mChooseCaptain.UpdateAsyncImages();
        return;
    }
    }
}

/**
 * Offset/Address/Size: 0xE4 | 0x800D6B2C | size: 0x47C
 */
void ChooseCaptainsSceneV2::BindChooseSideInstances()
{
    FORCE_DONT_INLINE;
}

extern "C" FEScrollText* __ct__12FEScrollTextFP14TLTextInstanceii(FEScrollText*, TLTextInstance*, int, int);

/**
 * Offset/Address/Size: 0x0 | 0x800D6A48 | size: 0xE4
 */
void ChooseCaptainsSceneV2::CreateTicker()
{
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    FEPresentation* pres = m_pFEPresentation;

    volatile unsigned long hB, hA;
    volatile unsigned long h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    h0 = 0;
    h1 = 0;
    h2 = 0;
    h3 = 0;
    h4 = 0;
    h5 = 0;
    h6 = 0;
    h7 = 0;

    unsigned long hash = nlStringLowerHash("TickerText");
    h8 = hash;
    h9 = hash;

    hash = nlStringLowerHash("Layer");
    hB = hash;
    hA = hash;

    TLTextInstance* textInstance = findText.byRef(pres->m_currentSlide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);

    gl_ScreenInfo* screenInfo = glGetScreenInfo();
    FEScrollText* ticker;
    if ((ticker = (FEScrollText*)nlMalloc(0x22C, 8, false)) != NULL)
    {
        ticker = __ct__12FEScrollTextFP14TLTextInstanceii(ticker, textInstance, 0, screenInfo->ScreenWidth + 0x32);
    }

    mTicker = ticker;
}
