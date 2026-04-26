#include "Game/SH/SHSaveLoad.h"

#include "dolphin/card.h"
#include "Game/Audio/AudioLoader.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "Game/DB/SaveLoad.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feInput.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/FE/feSceneManager.h"
#include "Game/FE/tlTextInstance.h"
#include "Game/ResetTask.h"
#include "Game/SH/SHMoviePlayer.h"
#include "Game/Sys/gcmemcard.h"
#include "NL/nlConfig.h"
#include "types.h"

enum eSaveLoad
{
    ST_INVALID = -1,
    ST_SAVE = 0,
    ST_LOAD = 1,
    ST_GAMESAVEIDTEST = 2,
    ST_DELETE = 3,
    ST_FORMAT = 4,
    ST_ASK_SAVE = 5,
    ST_ASK_LOAD = 6,
    ST_CHECKING = 7,
    ST_ABOUT_AUTOSAVE = 8,
    ST_CONFIRM_FORMAT = 9,
    ST_SHOULD_LOAD_OR_SAVE = 10,
};

extern int gSceneTypeStackDepth;
extern enum eSaveLoad gSceneTypeStack[4];
extern bool gSaveLoadStarted;
extern bool gSaveLoadFinished;
extern bool gCallbackMade;
extern float gSceneTime;
extern bool gIgnoreMinWait;
extern bool gContinueWithoutOperation;
extern bool gSaveLoadEnabled;
extern bool g_e3_Build;
extern long gResult;
extern float gRetryTimerDelay;

// extern bool s_InitDone__7MemCard;
// extern declared in gcmemcard.h as: extern MemCard* g_MemCards[2];
extern u8 WasCardRemoved;
extern u8 PreviousNoCardInSlotState;

/**
 * Offset/Address/Size: 0x28D0 | 0x800B2E58 | size: 0x14
 */
bool DidContinueWithoutOperation()
{
    return (gContinueWithoutOperation == true);
}

/**
 * Offset/Address/Size: 0x28C4 | 0x800B2E4C | size: 0xC
 */
void ResetEnableSaveLoadFlag()
{
    gSaveLoadEnabled = true;
}

/**
 * Offset/Address/Size: 0x28B4 | 0x800B2E3C | size: 0x10
 */
void SaveLoadCallback(long result)
{
    gResult = result;
    gCallbackMade = true;
}

/**
 * Offset/Address/Size: 0x2878 | 0x800B2E00 | size: 0x3C
 */
void ContinueWithoutSavingCB()
{
    gSceneTypeStackDepth = 1;
    SaveLoadScene* instance = SaveLoadScene::mInstance;
    gIgnoreMinWait = true;
    gSaveLoadFinished = true;
    gSaveLoadStarted = true;
    gSaveLoadEnabled = false;
    gContinueWithoutOperation = true;
    SaveLoadScene::mLastSaveLoadSuccess = false;
    if (instance->m_displayText != nullptr)
    {
        instance->m_displayText->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x2840 | 0x800B2DC8 | size: 0x38
 */
void ContinueWithoutLoadingCB()
{
    gSceneTypeStackDepth = 1;
    SaveLoadScene* instance = SaveLoadScene::mInstance;
    gIgnoreMinWait = true;
    gSaveLoadFinished = true;
    gSaveLoadStarted = true;
    gContinueWithoutOperation = true;
    SaveLoadScene::mLastSaveLoadSuccess = false;
    if (instance->m_displayText != nullptr)
    {
        instance->m_displayText->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x27DC | 0x800B2D64 | size: 0x64
 */
void ContinueLoadingCB()
{
    gCallbackMade = false;
    int stackIndex = --gSceneTypeStackDepth;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    ResetTask::s_resetPaused = (gSceneTypeStack[stackIndex] == 0);
    gSceneTypeStack[gSceneTypeStackDepth++] = ST_LOAD;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    gCallbackMade = false;
    gSceneTime = 0.0f;
    ResetTask::s_resetPaused = false;
}

/**
 * Offset/Address/Size: 0x25E0 | 0x800B2B68 | size: 0x1FC
 */
void RetryCB()
{
    gSceneTypeStackDepth = 0;

    switch (SaveLoadScene::mInstance->mSaveLoadMode)
    {
    case SaveLoadScene::SLM_AT_BOOT:
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_SHOULD_LOAD_OR_SAVE;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SaveLoadScene::SLM_SAVING:
        gSaveLoadStarted = false;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_SAVE;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = true;

        gSceneTypeStack[gSceneTypeStackDepth++] = ST_GAMESAVEIDTEST;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SaveLoadScene::SLM_LOADING:
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_LOAD;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SaveLoadScene::SLM_ASK_BEFORE_SAVING:
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_ASK_SAVE;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SaveLoadScene::SLM_ASK_BEFORE_LOADING:
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_ASK_LOAD;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;
    }

    gSceneTypeStack[gSceneTypeStackDepth++] = ST_CHECKING;
    gCallbackMade = false;
    ResetTask::s_resetPaused = false;
    gSaveLoadStarted = true;
    gSaveLoadFinished = true;
    gIgnoreMinWait = false;
    gSceneTime = 0.0f;
    gContinueWithoutOperation = false;

    if (SaveLoadScene::mInstance->m_displayText != nullptr)
    {
        SaveLoadScene::mInstance->m_displayText->m_bVisible = true;
    }

    gRetryTimerDelay = 1.0f;
    SaveLoadScene::mInstance->SceneCreated();
}

/**
 * Offset/Address/Size: 0x25A0 | 0x800B2B28 | size: 0x40
 */
void DeleteFileCB()
{
    gSceneTypeStack[gSceneTypeStackDepth++] = ST_DELETE;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    gCallbackMade = false;
    gSceneTime = 0.0f;
    ResetTask::s_resetPaused = false;
}

/**
 * Offset/Address/Size: 0x2540 | 0x800B2AC8 | size: 0x60
 */
void FormatConfirmCB()
{
    SaveLoad::RememberCurrentMemCardSerialID(0);
    gSceneTypeStack[gSceneTypeStackDepth++] = ST_CONFIRM_FORMAT;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    gCallbackMade = false;
    ResetTask::s_resetPaused = false;
    gSceneTime = 999.9f;
}

/**
 * Offset/Address/Size: 0x2500 | 0x800B2A88 | size: 0x40
 */
void FormatCB()
{
    gSceneTypeStack[gSceneTypeStackDepth++] = ST_FORMAT;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    gCallbackMade = false;
    gSceneTime = 0.0f;
    ResetTask::s_resetPaused = false;
}

/**
 * Offset/Address/Size: 0x24E0 | 0x800B2A68 | size: 0x20
 */
void ManageMemCardCB()
{
    ResetTask::s_ResetMode = 1;
    ResetTask::s_ResetState = (ResetTask::s_ResetState == RS_RUNNING) ? RS_STARTRESET : ResetTask::s_ResetState;
}

/**
 * Offset/Address/Size: 0x247C | 0x800B2A04 | size: 0x64
 */
void OverwriteFileAndContinueCB()
{
    gCallbackMade = false;
    int stackIndex = --gSceneTypeStackDepth;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    ResetTask::s_resetPaused = (gSceneTypeStack[stackIndex] == 0);
    gSceneTypeStackDepth = stackIndex + 1;
    gSceneTypeStack[stackIndex] = ST_SAVE;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    gCallbackMade = false;
    gSceneTime = 0.0f;
    ResetTask::s_resetPaused = true;
}

/**
 * Offset/Address/Size: 0x2420 | 0x800B29A8 | size: 0x5C
 */
void CreateFileAndSaveCB()
{
    gCallbackMade = false;
    int stackIndex = --gSceneTypeStackDepth;
    SaveLoadScene* instance = SaveLoadScene::mInstance;
    gSaveLoadStarted = false;
    gSaveLoadFinished = false;
    ResetTask::s_resetPaused = (gSceneTypeStack[stackIndex] == 0);
    instance->SetupForAboutAutoSave();
}

/**
 * Offset/Address/Size: 0x15E4 | 0x800B1B6C | size: 0xE3C
 */
void CheckResults()
{
}

/**
 * Offset/Address/Size: 0x1408 | 0x800B1990 | size: 0x1DC
 * TODO: 94.1% match - callback setup still keeps gSceneTypeStack temp in r7 and
 * call symbol remains Create(..., Function&, Function&) vs target by-value mangling.
 */
bool PushNoCardMessage()
{
    MemCard* memCard = g_MemCards[0];
    s32 result = CARDProbeEx(memCard->m_Slot, &memCard->m_CardInfo.CardSize, &memCard->m_CardInfo.SectorSize);

    if (result != CARD_RESULT_READY)
    {
        memCard->m_State = IS_IDLE;
        memCard->m_CardState = CS_IDLE;
    }

    if (result == CARD_RESULT_NOCARD || WasCardRemoved)
    {
        BaseSceneHandler* handler;
        if (nlSingleton<GameSceneManager>::s_pInstance->mCurrentStackDepth != 0)
        {
            handler = nlSingleton<GameSceneManager>::s_pInstance->mBaseSceneHandlerStack[nlSingleton<GameSceneManager>::s_pInstance->mCurrentStackDepth - 1];
        }
        else
        {
            handler = NULL;
        }

        if (nlSingleton<GameSceneManager>::s_pInstance->GetSceneType(handler) == SCENE_POPUP_MENU)
        {
            if (((FEPopupMenu*)handler)->mType == POPUP_NO_MEMCARD)
            {
                WasCardRemoved = 0;
                return false;
            }

            if (!nlSingleton<FESceneManager>::s_pInstance->AreAllScenesValid())
            {
                return false;
            }

            nlSingleton<GameSceneManager>::s_pInstance->Pop();
            nlSingleton<FESceneManager>::s_pInstance->ForceImmediateStackProcessing();
        }

        FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

        s32 prevOp = gSceneTypeStack[gSceneTypeStackDepth - 1];

        Function<FnVoidVoid> option1;
        Function<FnVoidVoid> option2;
        option1.mTag = FREE_FUNCTION;
        option1.mFreeFunction = RetryCB;
        option2.mTag = FREE_FUNCTION;
        option2.mFreeFunction = ContinueWithoutSavingCB;
        if (prevOp == ST_LOAD)
        {
            option2.mFreeFunction = ContinueWithoutLoadingCB;
        }

        popup->Create(POPUP_NO_MEMCARD, option1, option2);

        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0x11C8 | 0x800B1750 | size: 0x240
 */
SaveLoadScene::SaveLoadScene(SaveLoadScene::eSaveLoadMode saveLoadMode)
{
    m_displayText = NULL;
    mNextScene = SCENE_INVALID;
    mIsAutoSaving = false;
    mIsFirstTimeCreateFile = true;
    mButtonComponent = NULL;
    mSaveLoadMode = saveLoadMode;

    gSceneTypeStackDepth = 0;

    int savingorloading = SCENE_SAVE;

    switch (mSaveLoadMode)
    {
    case SLM_SAVING:
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_SAVE;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = true;

        gSceneTypeStack[gSceneTypeStackDepth++] = ST_GAMESAVEIDTEST;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SLM_LOADING:
        savingorloading = SCENE_LOAD;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_LOAD;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SLM_ASK_BEFORE_SAVING:
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_ASK_SAVE;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SLM_ASK_BEFORE_LOADING:
        savingorloading = SCENE_LOAD;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_ASK_LOAD;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;

    case SLM_AT_BOOT:
        savingorloading = SCENE_LOAD;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_SHOULD_LOAD_OR_SAVE;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = false;
        break;
    }

    gSceneTypeStack[gSceneTypeStackDepth++] = ST_CHECKING;
    gCallbackMade = false;
    ResetTask::s_resetPaused = false;
    gSaveLoadStarted = true;
    gSaveLoadFinished = true;
    gIgnoreMinWait = false;
    gSceneTime = 0.0f;
    gRetryTimerDelay = 1.0f;
    gContinueWithoutOperation = false;

    g_pFEInput->PushExclusiveInputLock(this, savingorloading);

    mInstance = this;
}

/**
 * Offset/Address/Size: 0x1124 | 0x800B16AC | size: 0xA4
 */
SaveLoadScene::~SaveLoadScene()
{
    extern bool mIsFirstTimeAboutIPL__13SaveLoadScene;

    g_pFEInput->PopExclusiveInputLock(this);
    mInstance = NULL;

    if (mButtonComponent != NULL)
    {
        delete mButtonComponent;
        mButtonComponent = NULL;
    }

    mIsFirstTimeAboutIPL__13SaveLoadScene = false;
}

/**
 * Offset/Address/Size: 0xF0C | 0x800B1494 | size: 0x218
 * TODO: 98.5% match - first hasher block remains at sp+0x38 (target sp+0x2C)
 * and function body is still shifted by +0x14 bytes in current codegen
 */
void SaveLoadScene::SceneCreated()
{
    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();
    pres->SetActiveSlide("Slide1");

    typedef TLTextInstance* (*FindTextByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    volatile InlineHasher hB, hA;
    volatile InlineHasher h9, h8;
    volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;

    unsigned long hash;
    hash = nlStringLowerHash("Text");
    h6.m_Hash = hash;
    h7.m_Hash = hash;

    hash = nlStringLowerHash("Layer");
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash("Slide1");
    hA.m_Hash = hash;
    hB.m_Hash = hash;

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;
    findText.byValue = FEFinder<TLTextInstance, 3>::Find<FEPresentation>;

    m_displayText = findText.byRef(
        pres,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    if (mIsAutoSaving)
    {
        pres->SetActiveSlide("Slide2");
    }

    typedef TLSlide* (*FindSlideByValue)(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLSlide* (*FindSlideByRef)(FEPresentation*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    volatile InlineHasher gB, gA;
    volatile InlineHasher g9, g8, g7, g6, g4, g2, g0;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g2.m_Hash = 0;
    h3.m_Hash = 0;
    g4.m_Hash = 0;
    h5.m_Hash = 0;
    g6.m_Hash = 0;
    g7.m_Hash = 0;
    g8.m_Hash = 0;
    g9.m_Hash = 0;

    hash = nlStringLowerHash("Slide3");
    gA.m_Hash = hash;
    gB.m_Hash = hash;

    union
    {
        FindSlideByValue byValue;
        FindSlideByRef byRef;
    } findSlide;
    findSlide.byValue = FEFinder<TLSlide, 0>::Find<FEPresentation>;

    mAboutAutoSaveSlide = findSlide.byRef(
        pres,
        (InlineHasher&)gB,
        (InlineHasher&)g9,
        (InlineHasher&)g7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    TLTextInstance* text = m_displayText;
    if (text != NULL)
    {
        eSaveLoad prevOp = gSceneTypeStack[gSceneTypeStackDepth - 1];
        switch (prevOp)
        {
        case ST_SAVE:
        case ST_GAMESAVEIDTEST:
            text->m_LocStrId = 0xCF941DC9;
            text->m_OverloadFlags |= 0x8;
            break;
        case ST_LOAD:
            text->m_LocStrId = 0xFAA420FA;
            text->m_OverloadFlags |= 0x8;
            break;
        case ST_FORMAT:
        case ST_CONFIRM_FORMAT:
            text->m_LocStrId = 0x81D26163;
            text->m_OverloadFlags |= 0x8;
            break;
        case ST_DELETE:
            text->m_LocStrId = 0x1A7FDB2D;
            text->m_OverloadFlags |= 0x8;
            break;
        case ST_ASK_SAVE:
        case ST_ASK_LOAD:
        case ST_CHECKING:
        case ST_SHOULD_LOAD_OR_SAVE:
            text->m_LocStrId = 0xE8E70E54;
            text->m_OverloadFlags |= 0x8;
            break;
        case ST_ABOUT_AUTOSAVE:
            break;
        case (eSaveLoad)11:
            text->m_LocStrId = 0xF501447B;
            text->m_OverloadFlags |= 0x8;
            break;
        }
    }
}

/**
 * Offset/Address/Size: 0x8C8 | 0x800B0E50 | size: 0x644
 */
void SaveLoadScene::Update(float)
{
}

/**
 * Offset/Address/Size: 0x7D4 | 0x800B0D5C | size: 0xF4
 */
bool SaveLoadScene::IsIOEnabled()
{
    if (!gSaveLoadEnabled)
    {
        return false;
    }

    if (g_e3_Build)
    {
        return false;
    }

    return GetConfigBool(Config::Global(), "DisableMemCard", false) != true;
}

/**
 * Offset/Address/Size: 0x6D8 | 0x800B0C60 | size: 0xFC
 */
void SaveLoadScene::SetupForAboutAutoSave()
{
    typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindByValue byValue;
        FindByRef byRef;
    } findFunc;

    m_pFEPresentation->SetActiveSlide(mAboutAutoSaveSlide);

    if (mButtonComponent == NULL)
    {
        ButtonComponent* ptr = new ((u8*)nlMalloc(sizeof(ButtonComponent), 8, false)) ButtonComponent();
        mButtonComponent = ptr;

        volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;
        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash;
        hash = nlStringLowerHash("buttons");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash("Layer");
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        findFunc.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        mButtonComponent->mButtonInstance = findFunc.byRef(
            mAboutAutoSaveSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);
    }

    mButtonComponent->SetState(ButtonComponent::BS_A_ONLY);
    mButtonComponent->CentreButtons();

    TLComponentInstance* inst = mButtonComponent->mButtonInstance;
    if (inst != NULL)
    {
        inst->m_bVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x600 | 0x800B0B88 | size: 0xD8
 */
void SaveLoadScene::UpdateForAboutToSaveSlide()
{
    if (PushNoCardMessage())
    {
        SceneCreated();
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, 0x100, false, NULL))
    {
        SceneCreated();

        gSceneTypeStackDepth = 0;
        gSceneTypeStack[gSceneTypeStackDepth++] = ST_SAVE;
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        gCallbackMade = false;
        gSceneTime = 0.0f;
        ResetTask::s_resetPaused = true;
    }

    if (mButtonComponent != NULL)
    {
        TLComponentInstance* inst = mButtonComponent->mButtonInstance;
        if (inst != NULL)
        {
            inst->m_bVisible = true;
        }
    }
}

/**
 * Offset/Address/Size: 0x224 | 0x800B07AC | size: 0x3DC
 * TODO: 97.94% match - r0/r3 register swap in FORMAT case (4 diffs), likely -inline deferred flag difference
 */
void SaveLoadScene::HandleSaveLoadFinishedResult()
{
    eSaveLoad sceneType = gSceneTypeStack[gSceneTypeStackDepth - 1];

    switch (sceneType)
    {
    case ST_SAVE:
    case ST_FORMAT:
    case ST_DELETE:
    case ST_CONFIRM_FORMAT:
    {
        if (sceneType == ST_FORMAT)
        {
            int depth = *(volatile int*)&gSceneTypeStackDepth;
            gSaveLoadStarted = false;
            int stackIndex = depth - 1;
            eSaveLoad prevScene = gSceneTypeStack[stackIndex];
            gSaveLoadFinished = false;
            gSceneTypeStackDepth = stackIndex;
            ResetTask::s_resetPaused = (prevScene == 0);
            gSceneTypeStackDepth = stackIndex + 1;
            gSceneTypeStack[stackIndex] = (eSaveLoad)SCENE_CUP_BACKGROUND;
            gSaveLoadStarted = false;
            gSaveLoadFinished = false;
            gCallbackMade = false;
            gSceneTime = 0.0f;
            ResetTask::s_resetPaused = false;
        }
        else
        {
            int stackIndex = --gSceneTypeStackDepth;
            gSaveLoadStarted = false;
            eSaveLoad prevScene = gSceneTypeStack[stackIndex];
            gSaveLoadFinished = false;
            if (prevScene == 0)
            {
                ResetTask::s_resetPaused = true;
            }
            else
            {
                ResetTask::s_resetPaused = false;
            }
        }
        break;
    }

    case ST_LOAD:
    {
        int stackIndex = --gSceneTypeStackDepth;
        gSaveLoadStarted = false;
        eSaveLoad prevScene = gSceneTypeStack[stackIndex];
        gSaveLoadFinished = false;
        if (prevScene == 0)
        {
            ResetTask::s_resetPaused = true;
        }
        else
        {
            ResetTask::s_resetPaused = false;
        }
        break;
    }

    case ST_GAMESAVEIDTEST:
    {
        int stackIndex = --gSceneTypeStackDepth;
        gSaveLoadStarted = false;
        eSaveLoad prevScene = gSceneTypeStack[stackIndex];
        gSaveLoadFinished = false;
        if (prevScene == 0)
        {
            ResetTask::s_resetPaused = true;
        }
        else
        {
            ResetTask::s_resetPaused = false;
        }
        break;
    }

    case ST_ASK_SAVE:
    case ST_ASK_LOAD:
    {
        if (mIsAutoSaving)
        {
            nlSingleton<GameSceneManager>::s_pInstance->Pop();
        }
        else
        {
            if (gContinueWithoutOperation)
            {
                SaveLoadScene::mLastSaveLoadSuccess = false;
                if (mNextScene == SCENE_OPTIONS)
                {
                    if (sceneType != ST_FORMAT)
                    {
                        if (gResult != -8 && gResult != -9)
                        {
                            gSaveLoadEnabled = true;
                        }
                    }
                }
            }
            else
            {
                bool success = (gResult == 0);
                SaveLoadScene::mLastSaveLoadSuccess = success;
                if (success)
                {
                    gSaveLoadEnabled = true;
                }
            }

            BaseSceneHandler* scene = nlSingleton<GameSceneManager>::s_pInstance->Push(mNextScene, SCREEN_NOTHING, true);

            if (m_displayText != NULL)
            {
                m_displayText->m_bVisible = false;
            }

            if (gSceneTypeStack[gSceneTypeStackDepth - 1] == SCENE_MARIO_BACKGROUND)
            {
                if (mNextScene != SCENE_LEGAL)
                {
                    int oldMode = nlSingleton<GameInfoManager>::s_pInstance->mCurGameAudioSettings.Mode;
                    AudioSettings& opts = nlSingleton<GameInfoManager>::s_pInstance->GetAudioOptions();
                    bool playMusic = false;
                    if (oldMode == 2 || opts.Mode == 2)
                    {
                        playMusic = true;
                    }
                    nlSingleton<GameInfoManager>::s_pInstance->mUserInfo.mAudioOptions.ForceApplySettings(false);
                    if (playMusic)
                    {
                        AudioLoader::PlayFEMenuMusic();
                    }
                }
            }

            if (mNextScene == SCENE_MOVIE_PLAYER)
            {
                ((MoviePlayerScene*)scene)->SetMovieDetails("credits.thp", true, false);
                ((MoviePlayerScene*)scene)->mNextScene = SCENE_TITLE;
            }
        }
        break;
    }

    case ST_ABOUT_AUTOSAVE:
    {
        int stackIndex = --gSceneTypeStackDepth;
        eSaveLoad prevScene = gSceneTypeStack[stackIndex];
        gSaveLoadStarted = false;
        gSaveLoadFinished = false;
        ResetTask::s_resetPaused = (prevScene == 0);
        gSceneTime = 0.0f;
        gIgnoreMinWait = false;
        break;
    }

    case ST_CHECKING:
    {
        gSaveLoadFinished = false;
        int stackIndex = --gSceneTypeStackDepth;
        eSaveLoad prevScene = gSceneTypeStack[stackIndex];
        gSaveLoadStarted = false;
        ResetTask::s_resetPaused = (prevScene == 0);
        gSceneTime = 999.9f;
        gSaveLoadFinished = false;
        break;
    }

    case ST_SHOULD_LOAD_OR_SAVE:
    {
        if (gResult == 0)
        {
            int depth = gSceneTypeStackDepth;
            gSaveLoadStarted = false;
            int stackIndex = depth - 1;
            eSaveLoad prevScene = gSceneTypeStack[stackIndex];
            gSaveLoadFinished = false;
            gSceneTypeStackDepth = stackIndex;
            ResetTask::s_resetPaused = (prevScene == 0);
            gSaveLoadFinished = false;
            gSceneTypeStackDepth = stackIndex + 1;
            gSceneTypeStack[stackIndex] = ST_LOAD;
            gSaveLoadStarted = false;
            gCallbackMade = false;
            gSceneTime = 0.0f;
            ResetTask::s_resetPaused = false;
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(mNextScene, SCREEN_NOTHING, true);
        }
        gSaveLoadFinished = false;
        break;
    }

    case (eSaveLoad)11:
    default:
        break;
    }

    SaveLoad().FreeAllCallbackMemory();
}

/**
 * Offset/Address/Size: 0xB0 | 0x800B0638 | size: 0x174
 */
void SaveLoadScene::StartSaveNow()
{
    if (mInstance == NULL)
    {
        return;
    }

    eSaveLoad sceneType = gSceneTypeStack[gSceneTypeStackDepth - 1];
    if (sceneType != ST_SAVE)
    {
        return;
    }

    if (gSaveLoadStarted)
    {
        return;
    }

    if (mInstance->m_pFEPresentation->m_currentSlide == mInstance->mAboutAutoSaveSlide)
    {
        return;
    }

    gSaveLoadStarted = true;
    gSaveLoadFinished = false;
    gCallbackMade = false;

    switch (sceneType)
    {
    case ST_SAVE:
        gResult = SaveLoad::StartSave(0, SaveLoadCallback);
        break;
    case ST_LOAD:
        gResult = SaveLoad::StartLoad(0, SaveLoadCallback, true, false);
        break;
    case ST_GAMESAVEIDTEST:
        gResult = SaveLoad::StartMemoryCardIDCheck(0, SaveLoadCallback);
        break;
    case ST_DELETE:
        gResult = SaveLoad::StartDelete(0, SaveLoadCallback);
        break;
    case ST_FORMAT:
        gResult = SaveLoad::StartFormat(0, SaveLoadCallback);
        break;
    case ST_ASK_SAVE:
        gResult = SaveLoad::StartFileExistsCheck(0, SaveLoadCallback);
        break;
    case ST_ASK_LOAD:
        gResult = SaveLoad::StartLoad(0, SaveLoadCallback, false, false);
        break;
    case ST_CHECKING:
        gSaveLoadFinished = true;
        gCallbackMade = false;
        break;
    case ST_ABOUT_AUTOSAVE:
        gResult = SaveLoad::StartFileExistsCheck(0, SaveLoadCallback);
        break;
    case ST_CONFIRM_FORMAT:
        break;
    case ST_SHOULD_LOAD_OR_SAVE:
        break;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x800B0588 | size: 0xB0
 */
void SaveLoadScene::UpdateCardRemovedFlag()
{
    if (!MemCard::s_InitDone)
    {
        return;
    }

    MemCard* memCard = g_MemCards[0];

    s32 result = CARDProbeEx(memCard->m_Slot, &memCard->m_CardInfo.CardSize, &memCard->m_CardInfo.SectorSize);
    if (result != 0)
    {
        memCard->m_State = IS_IDLE;
        memCard->m_CardState = CS_IDLE;
    }

    if (result == CARD_RESULT_NOCARD)
    {
        // Reload pointer in case it changed
        memCard = g_MemCards[0];

        result = CARDProbeEx(memCard->m_Slot, &memCard->m_CardInfo.CardSize, &memCard->m_CardInfo.SectorSize);
        if (result != 0)
        {
            memCard->m_State = IS_IDLE;
            memCard->m_CardState = CS_IDLE;
        }

        u8 currentNoCardState = (result == CARD_RESULT_NOCARD) ? 1 : 0;
        if (PreviousNoCardInSlotState != currentNoCardState)
        {
            WasCardRemoved = 1;
        }
    }
}
