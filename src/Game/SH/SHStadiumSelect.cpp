#include "Game/SH/SHStadiumSelect.h"

#include "Game/FE/FEAudio.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feInput.h"
#include "Game/GameInfo.h"
#include "Game/GameSceneManager.h"
#include "NL/gl/glStruct.h"
#include "NL/nlPrint.h"
#include "types.h"

static const eStadiumID STADIUM_ORDER[7] = {
    STAD_MARIO_STADIUM,
    STAD_PEACH_TOAD_STADIUM,
    STAD_WARIO_STADIUM,
    STAD_DK_DAISY,
    STAD_YOSHI_STADIUM,
    STAD_FORBIDDEN_DOME,
    STAD_SUPER_STADIUM,
};

static const StadiumEntry StadiumEntries[8] = {
    { STAD_PEACH_TOAD_STADIUM, "fe/mainv2/targas/stadium_peach" },
    { STAD_MARIO_STADIUM, "fe/mainv2/targas/stadium_mario" },
    { STAD_WARIO_STADIUM, "fe/mainv2/targas/stadium_wario" },
    { STAD_DK_DAISY, "fe/mainv2/targas/stadium_dk" },
    { STAD_YOSHI_STADIUM, "fe/mainv2/targas/stadium_yoshi" },
    { STAD_FORBIDDEN_DOME, "fe/mainv2/targas/stadium_dome" },
    { STAD_SUPER_STADIUM, "fe/mainv2/targas/stadium_super" },
    { (eStadiumID)7, "fe/mainv2/targas/stadium_LOCKED" },
};

static unsigned long StadiumDescriptions[7] = {
    0xE2FCB05C,
    0x065E30EC,
    0xEF23A6A3,
    0xEF484926,
    0xF045B5A2,
    0x618EAFC3,
    0xEFDC5FC5,
};

static inline bool IsStadiumUnlocked(eStadiumID sid)
{
    switch (sid)
    {
    case STAD_MARIO_STADIUM:
        return true;
    case STAD_PEACH_TOAD_STADIUM:
        return true;
    case STAD_DK_DAISY:
        return GameInfoManager::Instance()->IsKongaUnlocked();
    case STAD_WARIO_STADIUM:
        return true;
    case STAD_YOSHI_STADIUM:
        return GameInfoManager::Instance()->IsYoshiUnlocked();
    case STAD_SUPER_STADIUM:
        return GameInfoManager::Instance()->IsSuperStadiumUnlocked();
    case STAD_FORBIDDEN_DOME:
        return GameInfoManager::Instance()->IsForbiddenUnlocked();
    default:
        return true;
    }
}

static inline int WrapStadiumIndex(int index)
{
    if (index < 0)
    {
        index += 7;
    }
    return index % 7;
}

/**
 * Offset/Address/Size: 0x1400 | 0x800D9980 | size: 0xF4
 */
#pragma dont_inline reset
StadiumSelectSceneV2::StadiumSelectSceneV2()
    : mTempTextureBuffer(NULL)
    , mTempTextureBufferSize(0)
    , mStadiumIndex(0)
    , mCurrentMenuList(NULL)
    , mLastDirection(DIR_LEFT)
{
}

/**
 * Offset/Address/Size: 0x1230 | 0x800D97B0 | size: 0x1D0
 */
StadiumSelectSceneV2::~StadiumSelectSceneV2()
{
    for (int i = 0; i < 7; i++)
    {
        delete mImages[i];
    }

    if (mTempTextureBuffer != NULL)
    {
        delete[] mTempTextureBuffer;
    }

    delete m_pTicker;

    m_pTicker = NULL;
}

/**
 * Offset/Address/Size: 0x7D4 | 0x800D8D54 | size: 0xA5C
 */
void StadiumSelectSceneV2::SceneCreated()
{
    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();

    for (int i = 0; i < 7; i++)
    {

        char buf[64];
        nlSNPrintf(buf, 64, "stadium_%c", i + 'A');

        TLImageInstance* img = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            pres->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash(buf)));
        AsyncImage* asyncImg = new (nlMalloc(0x1C, 0x20, true)) AsyncImage("art/fe/StadiumsUI.res", NULL);
        mImages[i] = asyncImg;
        mImages[i]->mImageInstance = img;
    }

    mStadiumIndex = 0;

    {

        TLTextInstance* tickerText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            pres->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("TickerText")));

        gl_ScreenInfo* screenInfo = glGetScreenInfo();
        FEScrollText* ticker = new (nlMalloc(0x22C, 0x8, false)) FEScrollText(tickerText, 0, screenInfo->ScreenWidth + 50);
        m_pTicker = ticker;
    }

    m_pTicker->SetDisplayMessage(StadiumDescriptions[mStadiumIndex]);

    eStadiumID sid;
    int loadIdx;

    {
        int idx;
        if ((idx = mStadiumIndex - 4) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[6]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex - 3) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[3]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex - 2) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[2]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex - 1) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[0]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[1]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex + 1) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[4]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    {
        int idx;
        if ((idx = mStadiumIndex + 2) < 0)
        {
            idx += 7;
        }
        idx = idx % 7;
        sid = STADIUM_ORDER[idx];
        loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
        mImages[5]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);
    }

    mLastDirection = DIR_LEFT;
    mTempTextureBuffer = NULL;
    mTempTextureBufferSize = -1;

    {

        struct SlideHolder
        {
            TLSlide* p;
        } holder;
        holder.p = m_pFEPresentation->m_currentSlide;

        TLTextInstance* nameText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            holder.p,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("stadiumname")));

        eStadiumID currentStadium = StadiumEntries[mStadiumIndex].stadiumID;
        bool isUnlocked = true;
        switch (currentStadium)
        {
        case STAD_DK_DAISY:
            isUnlocked = GameInfoManager::Instance()->IsKongaUnlocked();
            break;
        case STAD_YOSHI_STADIUM:
            isUnlocked = GameInfoManager::Instance()->IsYoshiUnlocked();
            break;
        case STAD_FORBIDDEN_DOME:
            isUnlocked = GameInfoManager::Instance()->IsForbiddenUnlocked();
            break;
        case STAD_SUPER_STADIUM:
            isUnlocked = GameInfoManager::Instance()->IsSuperStadiumUnlocked();
            break;
        }

        if (isUnlocked)
        {
            nameText->m_LocStrId = GetStadiumStringID(StadiumEntries[mStadiumIndex].stadiumID);
            nameText->m_OverloadFlags |= 0x8;
        }
        else
        {
            nameText->m_LocStrId = 0x2A68AC55;
            nameText->m_OverloadFlags |= 0x8;
        }
    }

    {

        TLComponentInstance* dayNightComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            pres->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("day night")));
        dayNightComp->m_bVisible = false;
        pres->SetActiveSlide("RIGHT");
    }

    {

        TLComponentInstance* dayNightComp2 = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            pres->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("day night")));
        dayNightComp2->m_bVisible = false;
        pres->SetActiveSlide("LEFT");
    }

    {

        TLComponentInstance* buttonsComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("buttons")));
        mButtons.mButtonInstance = buttonsComp;
        mButtons.SetState(ButtonComponent::BS_A_AND_B);
    }
}

/**
 * Offset/Address/Size: 0x210 | 0x800D8790 | size: 0x5C4
 */
void StadiumSelectSceneV2::Update(float dt)
{
    BaseSceneHandler::Update(dt);
    mButtons.CentreButtons();

    TLSlide* slide = m_pFEPresentation->m_currentSlide;
    if (!(slide->m_time >= slide->m_start + slide->m_duration))
    {
        goto update_images;
    }

    m_pTicker->Update(dt);
    {
        bool rightPressed = g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xC, true, NULL);
        bool leftPressed = g_pFEInput->IsAutoPressed(FE_ALL_PADS, 0xB, true, NULL);

        if (!rightPressed && !leftPressed)
        {
            goto check_a_button;
        }

        int newIndex;
        if (rightPressed)
        {
            if ((newIndex = mStadiumIndex + 1) < 0)
            {
                newIndex += 7;
            }
            newIndex %= 7;
        }
        else
        {
            if ((newIndex = mStadiumIndex - 1) < 0)
            {
                newIndex += 7;
            }
            newIndex %= 7;
        }
        mStadiumIndex = newIndex;

        m_pFEPresentation->SetActiveSlide(rightPressed ? "LEFT" : "RIGHT");

        {
            TLComponentInstance* buttonsComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
                m_pFEPresentation->m_currentSlide,
                InlineHasher(nlStringLowerHash("Layer")),
                InlineHasher(nlStringLowerHash("buttons")));
            mButtons.mButtonInstance = buttonsComp;
            mButtons.SetState(ButtonComponent::BS_A_AND_B);
        }

        if (rightPressed)
        {
            if (mLastDirection != DIR_LEFT)
            {
                ResetFromRight();
            }
            else
            {
                ResetFromLeft();
            }
        }
        else
        {
            if (mLastDirection != DIR_RIGHT)
            {
                ResetFromLeft();
            }
            else
            {
                ResetFromRight();
            }
        }

        mLastDirection = (Direction)(rightPressed == 0 ? 1 : 0);
        FEAudio::PlayAnimAudioEvent("sfx_toggle_stadium", false);

        eStadiumID sid = StadiumEntries[mStadiumIndex].stadiumID;

        TLTextInstance* nameText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            m_pFEPresentation->m_currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("stadiumname")));

        {
            bool isUnlocked = true;
            switch (sid)
            {
            case STAD_DK_DAISY:
                isUnlocked = GameInfoManager::Instance()->IsKongaUnlocked();
                break;
            case STAD_YOSHI_STADIUM:
                isUnlocked = GameInfoManager::Instance()->IsYoshiUnlocked();
                break;
            case STAD_FORBIDDEN_DOME:
                isUnlocked = GameInfoManager::Instance()->IsForbiddenUnlocked();
                break;
            case STAD_SUPER_STADIUM:
                isUnlocked = GameInfoManager::Instance()->IsSuperStadiumUnlocked();
                break;
            }

            if (isUnlocked)
            {
                nameText->m_LocStrId = GetStadiumStringID(sid);
                nameText->m_OverloadFlags |= 0x8;
            }
            else
            {
                nameText->m_LocStrId = 0x2A68AC55;
                nameText->m_OverloadFlags |= 0x8;
            }
        }

        {
            TLTextInstance* tickerText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
                m_pFEPresentation->m_currentSlide,
                InlineHasher(nlStringLowerHash("Layer")),
                InlineHasher(nlStringLowerHash("TickerText")));

            m_pTicker->ApplyNewTextInstancePointer(tickerText, 8000.0f, 100.0f);
        }

        {
            bool isUnlocked = true;
            switch (sid)
            {
            case STAD_DK_DAISY:
                isUnlocked = GameInfoManager::Instance()->IsKongaUnlocked();
                break;
            case STAD_YOSHI_STADIUM:
                isUnlocked = GameInfoManager::Instance()->IsYoshiUnlocked();
                break;
            case STAD_FORBIDDEN_DOME:
                isUnlocked = GameInfoManager::Instance()->IsForbiddenUnlocked();
                break;
            case STAD_SUPER_STADIUM:
                isUnlocked = GameInfoManager::Instance()->IsSuperStadiumUnlocked();
                break;
            }

            if (isUnlocked)
            {
                m_pTicker->SetDisplayMessage(StadiumDescriptions[mStadiumIndex]);
            }
            else
            {
                m_pTicker->SetDisplayMessage((unsigned long)0xDAA0A048);
            }
        }
        goto update_images;
    }

check_a_button:
    if (g_pFEInput->JustPressed(FE_ALL_PADS, PAD_BUTTON_A, false, NULL))
    {
        OnSelectStadium();
        return;
    }

    if (g_pFEInput->JustPressed(FE_ALL_PADS, PAD_BUTTON_B, false, NULL))
    {
        BaseSceneHandler* scene = GameSceneManager::Instance()->Push((SceneList)8, (ScreenMovement)2, true);
        *(unsigned long*)((char*)scene + 0x1B4) = 1;
        FEAudio::PlayAnimAudioEvent("sfx_back", false);
        return;
    }

update_images:
    for (int i = 0; i < 7; i++)
    {
        mImages[i]->Update(true);
    }
}

/**
 * Offset/Address/Size: 0x108 | 0x800D8688 | size: 0x108
 */
void StadiumSelectSceneV2::ResetFromRight()
{
    FORCE_DONT_INLINE;
    if (mTempTextureBuffer == NULL)
    {
        mTempTextureBufferSize = mImages[0]->mTextureSize;
        mTempTextureBuffer = nlMalloc(mTempTextureBufferSize, 0x20, true);
    }

    memcpy(mTempTextureBuffer, mImages[5]->m_loadBuffer, mTempTextureBufferSize);

    mImages[5]->CopyFrom(mImages[4]);
    mImages[5]->SwapTextures();

    mImages[4]->CopyFrom(mImages[1]);
    mImages[4]->SwapTextures();

    mImages[1]->CopyFrom(mImages[0]);
    mImages[1]->SwapTextures();

    mImages[0]->CopyFrom(mImages[2]);
    mImages[0]->SwapTextures();

    mImages[2]->CopyFrom(mImages[3]);
    mImages[2]->SwapTextures();

    mImages[3]->CopyFrom(mImages[6]);
    mImages[3]->SwapTextures();

    mImages[6]->CopyFrom(mTempTextureBuffer, mTempTextureBufferSize);

    mImages[0]->SwapTextures();
    mImages[1]->SwapTextures();
    mImages[2]->SwapTextures();
}

/**
 * Offset/Address/Size: 0x0 | 0x800D8580 | size: 0x108
 */
void StadiumSelectSceneV2::ResetFromLeft()
{
    FORCE_DONT_INLINE;
    if (mTempTextureBuffer == NULL)
    {
        mTempTextureBufferSize = mImages[0]->mTextureSize;
        mTempTextureBuffer = nlMalloc(mTempTextureBufferSize, 0x20, true);
    }

    memcpy(mTempTextureBuffer, mImages[6]->m_loadBuffer, mTempTextureBufferSize);

    mImages[6]->CopyFrom(mImages[3]);
    mImages[6]->SwapTextures();

    mImages[3]->CopyFrom(mImages[2]);
    mImages[3]->SwapTextures();

    mImages[2]->CopyFrom(mImages[0]);
    mImages[2]->SwapTextures();

    mImages[0]->CopyFrom(mImages[1]);
    mImages[0]->SwapTextures();

    mImages[1]->CopyFrom(mImages[4]);
    mImages[1]->SwapTextures();

    mImages[4]->CopyFrom(mImages[5]);
    mImages[4]->SwapTextures();

    mImages[5]->CopyFrom(mTempTextureBuffer, mTempTextureBufferSize);

    mImages[0]->SwapTextures();
    mImages[1]->SwapTextures();
    mImages[2]->SwapTextures();
}
#pragma dont_inline reset

void StadiumSelectSceneV2::OnSelectStadium()
{
    eStadiumID stadium = StadiumEntries[mStadiumIndex].stadiumID;
    bool isUnlocked = true;
    switch (stadium)
    {
    case STAD_DK_DAISY:
        isUnlocked = GameInfoManager::Instance()->IsKongaUnlocked();
        break;
    case STAD_YOSHI_STADIUM:
        isUnlocked = GameInfoManager::Instance()->IsYoshiUnlocked();
        break;
    case STAD_FORBIDDEN_DOME:
        isUnlocked = GameInfoManager::Instance()->IsForbiddenUnlocked();
        break;
    case STAD_SUPER_STADIUM:
        isUnlocked = GameInfoManager::Instance()->IsSuperStadiumUnlocked();
        break;
    }

    if (isUnlocked)
    {
        GameSceneManager::Instance()->PushLoadingScene(true);
        GameInfoManager::Instance()->SetStadium(stadium);
        FEAudio::PlayAnimAudioEvent("sfx_accept_stadium", false);
    }
    else
    {
        FEAudio::PlayAnimAudioEvent("sfx_deny", false);
    }
}
