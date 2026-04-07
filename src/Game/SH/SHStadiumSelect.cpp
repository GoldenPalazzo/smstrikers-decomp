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

// /**
//  * Offset/Address/Size: 0x4EC | 0x800D9F60 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                      unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800D9EDC | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                   unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800D9EA4 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                  InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800D9D48 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                          unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800D9CC4 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                       unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800D9C8C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                      InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800D9B30 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long,
//                                                     unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800D9AAC | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
//                                                  unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800D9A74 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
//                                                 InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x1400 | 0x800D9980 | size: 0xF4
 */
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

    BasicStringData<unsigned short>* data;
    FEScrollText* ticker = m_pTicker;
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
            data = ticker->m_message.m_data;
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

    m_pTicker = NULL;
}

/**
 * Offset/Address/Size: 0x7D4 | 0x800D8D54 | size: 0xA5C
 * TODO: 80.31% match - volatile InlineHasher copy-back to parameter area before Find calls,
 * r26/r27 vs r27/r28 register allocation in loop, stack frame 0x10 too small
 */
/**
 * Offset/Address/Size: 0x384 | 0x800D8D54 | size: 0xA5C
 * TODO: 89.52% scratch match - r28/r29 register swap (format string vs img) and dead hasher
 * store elimination due to union byRef trick + non-volatile narrow-scope hashers.
 * File uses -inline deferred which may affect build match differently.
 */
void StadiumSelectSceneV2::SceneCreated()
{
    unsigned long hash;
    FEPresentation* pres = m_pFEScene->m_pFEPackage->GetPresentation();

    for (int i = 0; i < 7; i++)
    {
        typedef TLImageInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLImageInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findImg;
        findImg.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;

        char buf[64];
        nlSNPrintf(buf, 64, "stadium_%c", i + 'A');

        volatile InlineHasher hB, hA;
        volatile InlineHasher h9, h8;
        volatile InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash(buf);
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLImageInstance* img = findImg.byRef(
            pres->m_currentSlide, (InlineHasher&)hB, (InlineHasher&)h9, (InlineHasher&)h7, (InlineHasher&)h5, (InlineHasher&)h3, (InlineHasher&)h1);
        AsyncImage* asyncImg = new (nlMalloc(0x1C, 0x20, true)) AsyncImage("art/fe/StadiumsUI.res", NULL);
        mImages[i] = asyncImg;
        mImages[i]->mImageInstance = img;
    }

    mStadiumIndex = 0;

    {
        typedef TLTextInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLTextInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findTxt;
        findTxt.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        InlineHasher hB, hA;
        InlineHasher h9, h8;
        InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("TickerText");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLTextInstance* tickerText = findTxt.byRef(
            pres->m_currentSlide, hB, h9, h7, h5, h3, h1);

        gl_ScreenInfo* screenInfo = glGetScreenInfo();
        FEScrollText* ticker = new (nlMalloc(0x22C, 0x8, false)) FEScrollText(tickerText, 0, screenInfo->ScreenWidth + 50);
        m_pTicker = ticker;
    }

    m_pTicker->SetDisplayMessage(StadiumDescriptions[mStadiumIndex]);

    int idx;
    idx = WrapStadiumIndex(mStadiumIndex - 4);
    eStadiumID sid = STADIUM_ORDER[idx];
    int loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[6]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex - 3);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[3]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex - 2);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[2]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex - 1);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[0]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[1]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex + 1);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[4]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    idx = WrapStadiumIndex(mStadiumIndex + 2);
    sid = STADIUM_ORDER[idx];
    loadIdx = IsStadiumUnlocked(sid) ? idx : 7;
    mImages[5]->QueueLoad(StadiumEntries[loadIdx].imagePath, true);

    mLastDirection = DIR_LEFT;
    mTempTextureBuffer = NULL;
    mTempTextureBufferSize = -1;

    {
        typedef TLTextInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLTextInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findTxt;
        findTxt.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

        InlineHasher hB, hA;
        InlineHasher h9, h8;
        InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("stadiumname");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLTextInstance* nameText = findTxt.byRef(
            m_pFEPresentation->m_currentSlide, hB, h9, h7, h5, h3, h1);

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
        typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        InlineHasher hB, hA;
        InlineHasher h9, h8;
        InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("day night");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLComponentInstance* dayNightComp = findComp.byRef(
            pres->m_currentSlide, hB, h9, h7, h5, h3, h1);
        dayNightComp->m_bVisible = false;
        pres->SetActiveSlide("RIGHT");
    }

    {
        typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        InlineHasher hB, hA;
        InlineHasher h9, h8;
        InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("day night");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLComponentInstance* dayNightComp2 = findComp.byRef(
            pres->m_currentSlide, hB, h9, h7, h5, h3, h1);
        dayNightComp2->m_bVisible = false;
        pres->SetActiveSlide("LEFT");
    }

    {
        typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
        typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
        union
        {
            FindByValue byValue;
            FindByRef byRef;
        } findComp;
        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        InlineHasher hB, hA;
        InlineHasher h9, h8;
        InlineHasher h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("buttons");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hA.m_Hash = hash;
        hB.m_Hash = hash;
        TLComponentInstance* buttonsComp = findComp.byRef(
            m_pFEPresentation->m_currentSlide, hB, h9, h7, h5, h3, h1);
        mButtons.mButtonInstance = buttonsComp;
        mButtons.SetState(ButtonComponent::BS_A_AND_B);
    }
}

/**
 * Offset/Address/Size: 0x210 | 0x800D8790 | size: 0x5C4
 * TODO: 84.06% match - volatile InlineHasher copy-back to parameter area before Find calls,
 * stack offsets shifted +8 (11 vs 12 volatile slots), WrapStadiumIndex subf r4 vs r0 register
 */
void StadiumSelectSceneV2::Update(float dt)
{
    unsigned long hash;
    volatile InlineHasher hB;
    volatile InlineHasher h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

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
            newIndex = WrapStadiumIndex(mStadiumIndex + 1);
        }
        else
        {
            newIndex = WrapStadiumIndex(mStadiumIndex - 1);
        }
        mStadiumIndex = newIndex;

        m_pFEPresentation->SetActiveSlide(rightPressed ? "LEFT" : "RIGHT");

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("buttons");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hB.m_Hash = hash;
        {
            TLComponentInstance* buttonsComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
                m_pFEPresentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);
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

        mLastDirection = (Direction)(rightPressed == false);
        FEAudio::PlayAnimAudioEvent("sfx_toggle_stadium", false);

        eStadiumID sid = StadiumEntries[mStadiumIndex].stadiumID;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("stadiumname");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hB.m_Hash = hash;
        {
            TLTextInstance* nameText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
                m_pFEPresentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

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

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;
        hash = nlStringLowerHash("TickerText");
        h8.m_Hash = hash;
        h9.m_Hash = hash;
        hash = nlStringLowerHash("Layer");
        hB.m_Hash = hash;
        {
            TLTextInstance* tickerText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
                m_pFEPresentation->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);

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
        eStadiumID sid = StadiumEntries[mStadiumIndex].stadiumID;
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
            GameSceneManager::Instance()->PushLoadingScene(true);
            GameInfoManager::Instance()->SetStadium(sid);
            FEAudio::PlayAnimAudioEvent("sfx_accept_stadium", false);
        }
        else
        {
            FEAudio::PlayAnimAudioEvent("sfx_deny", false);
        }
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
