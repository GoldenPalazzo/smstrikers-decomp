#include "Game/SH/SHChooseCaptains.h"

#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/feTemplates.h"
#include "Game/GameSceneManager.h"
#include "Game/main.h"
#include "NL/nlConfig.h"
#include "NL/nlPrint.h"

/**
 * Offset/Address/Size: 0xE64 | 0x800D78AC | size: 0x9C
 */
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
    FORCE_DONT_INLINE;
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

    if (pCurrentSlide->m_time >= 1.0)
    {
        mTicker->Update(fDeltaT);
    }

    if (pCurrentSlide->m_time <= 1.15 && mSceneType == SceneType_1)
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
                TagValuePair& tvp = cfg.FindTvp("no_humans");
                bool allReady;
                if (tvp.tag == NULL)
                {
                    cfg.Set("no_humans", false);
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

static inline FEPresentation* GetScenePresentation(ChooseCaptainsSceneV2* pScene)
{
    return pScene->m_pFEPresentation;
}

/**
 * Offset/Address/Size: 0xE4 | 0x800D6B2C | size: 0x47C
 * TODO: 99.79% match - the SetAssetColour argument address materializes one
 *       slot earlier than target, between li 0xff and the first pad colour load
 */
void ChooseCaptainsSceneV2::BindChooseSideInstances()
{
    FEPresentation* pPres = GetScenePresentation(this);
    nlColour colour;

    TLComponentInstance* chooseSideComponent = (TLComponentInstance*)FEFinder<TLInstance, 4>::Find<TLSlide>(
        pPres->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CHOOSE_SIDE")));

    TLSlide* activeSlide = chooseSideComponent->GetActiveSlide();

    for (int i = 0; i < 4; i++)
    {
        char tempString[64];

        nlSNPrintf(tempString, 64, "controller%d", i + 1);
        mChooseSide.mInstanceTable[i] = FEFinder<TLInstance, 5>::Find<TLSlide>(
            activeSlide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempString)));

        mChooseSide.mInstanceTable[i + 4] = FEFinder<TLInstance, 3>::Find<TLInstance>(
            mChooseSide.mInstanceTable[i],
            InlineHasher(nlStringLowerHash("ready")));

        if (mChooseSide.mInstanceTable[i + 4] != NULL)
        {
            mChooseSide.mInstanceTable[i + 4]->m_bVisible = false;
        }

        nlSNPrintf(tempString, 64, "arrows%d", i + 1);
        mChooseSide.mInstanceTable[i + 12] = (TLInstance*)FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            activeSlide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempString)));

        nlSNPrintf(tempString, 64, "p%d", i + 1);
        mChooseSide.mInstanceTable[i + 8] = (TLInstance*)FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            activeSlide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempString)));

        // colour.c[0] = PAD_COLOURS[0][0];
        // colour.c[1] = PAD_COLOURS[i][1];
        // colour.c[2] = PAD_COLOURS[i][2];
        // colour.c[3] = 0xFF;
        nlColourSet(colour, PAD_COLOURS[i][0], PAD_COLOURS[i][1], PAD_COLOURS[i][2], 0xFF);
        mChooseSide.mInstanceTable[i + 8]->SetAssetColour(colour);
    }

    TLInstance* object = FEFinder<TLInstance, 2>::Find<TLSlide>(
        activeSlide,
        InlineHasher(nlStringLowerHash("group")),
        InlineHasher(nlStringLowerHash("homex")));

    mChooseSide.mControllerDestPos[0] = object->GetAssetPosition().f.x;
    object->m_bVisible = false;

    object = FEFinder<TLInstance, 2>::Find<TLSlide>(
        activeSlide,
        InlineHasher(nlStringLowerHash("group")),
        InlineHasher(nlStringLowerHash("awayx")));

    mChooseSide.mControllerDestPos[1] = object->GetAssetPosition().f.x;
    object->m_bVisible = false;

    mChooseSide.mControllerDestPos[2] = mChooseSide.mInstanceTable[0]->GetAssetPosition().f.x;

    mChooseSide.mInstanceTable[16] = FEFinder<TLInstance, 4>::Find<TLSlide>(
        pPres->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("continue")));

    mChooseSide.ResetAndPositionControllers(mChooseCaptain.mNumTotalPushedPlayers != 1);
}

/**
 * Offset/Address/Size: 0x0 | 0x800D6A48 | size: 0xE4
 */
void ChooseCaptainsSceneV2::CreateTicker()
{
    FORCE_DONT_INLINE;

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
    mTicker = new (nlMalloc(sizeof(FEScrollText), 8, false)) FEScrollText(textInstance, 0, screenInfo->ScreenWidth + 0x32);
}

#pragma inline_depth(8)
