#define BASICSTRING_NESTED_DATA_CTOR
#define BASICSTRING_OUTLINE_CTOR

#include "Game/SH/SHChooseSides.h"

#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/FE/feManager.h"
#include "Game/FE/fePopupMenu.h"
#include "Game/Game.h"
#include "Game/GameInfo.h"
#include "Game/GameRenderTask.h"
#include "Game/GameSceneManager.h"
#include "Game/OverlayManager.h"
#include "Game/SH/SHPause.h"
#include "Game/Team.h"
#include "NL/nlColour.h"
#include "NL/nlConfig.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/gl/glStruct.h"

extern bool g_e3_Build;

static inline void GetAllSides();
static inline void SetAllSides(IChooseSide& cs);
static inline eDifficultyID GetDifficulty0(GameInfoManager* gameInfo);
static inline eDifficultyID GetDifficulty1(GameInfoManager* gameInfo);

// /**
//  * Offset/Address/Size: 0x0 | 0x800C7D40 | size: 0xEC
//  */
// void BasicString<char, Detail::TempStringAllocator>::BasicString(const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0xCC8 | 0x800C7BE4 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xC44 | 0x800C7B60 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xC0C | 0x800C7B28 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xAB0 | 0x800C79CC | size: 0x15C
//  */
// void FEFinder<TLInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0xA2C | 0x800C7948 | size: 0x84
//  */
// void FEFinder<TLInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x9F4 | 0x800C7910 | size: 0x38
//  */
// void FEFinder<TLInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x898 | 0x800C77B4 | size: 0x15C
//  */
// void FEFinder<TLInstance, 5>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x814 | 0x800C7730 | size: 0x84
//  */
// void FEFinder<TLInstance, 5>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x7DC | 0x800C76F8 | size: 0x38
//  */
// void FEFinder<TLInstance, 5>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x680 | 0x800C759C | size: 0x15C
//  */
// void FEFinder<TLInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800C7564 | size: 0x38
//  */
// void FEFinder<TLInstance, 3>::Find<TLInstance>(TLInstance*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800C7408 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800C7384 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800C734C | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800C71F0 | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800C716C | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800C7134 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800C6FD8 | size: 0x15C
//  */
// void FEFinder<TLInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800C6F54 | size: 0x84
//  */
// void FEFinder<TLInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800C6F1C | size: 0x38
//  */
// void FEFinder<TLInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x21C8 | 0x800C6D88 | size: 0x194
 */
SHChooseSides2::SHChooseSides2(SHChooseSides2::eCSContext context)
    : BaseSceneHandler()
    , mContext(context)
    , m_pTicker(NULL)
    , mSoundDelay(0.0f)
    , mNextScene(SCENE_INVALID)
    , mBackScene(SCENE_INVALID)
    , mChooseSide()
    , mProceedDelay(-1)
    , mButtons()
{
    switch (mContext)
    {
    case FRIENDLY:
        if (g_e3_Build)
        {
            mNextScene = SCENE_SUPER_LOADING;
            mBackScene = SCENE_MAIN_MENU;
        }
        else
        {
            mNextScene = SCENE_STADIUM_SELECT;
            mBackScene = SCENE_CHOOSE_CAPTAINS;
        }
        break;
    case CUP:
        mNextScene = SCENE_SUPER_LOADING;
        mBackScene = SCENE_CUP_STANDINGS;
        break;
    case SUPERCUP:
        mNextScene = SCENE_SUPER_LOADING;
        mBackScene = SCENE_SUPER_CUP_STANDINGS;
        break;
    case TOURNAMENT:
        mNextScene = SCENE_SUPER_LOADING;
        mBackScene = SCENE_TOURNAMENT_STANDINGS;
        break;
    case PAUSE:
        mNextScene = SCENE_INVALID;
        mBackScene = IGSCENE_PAUSE;
        break;
    }

    mChooseSide.mPlayingSides[0] = -1;
    mChooseSide.mPlayingSides[1] = -1;
    mChooseSide.mPlayingSides[2] = -1;
    mChooseSide.mPlayingSides[3] = -1;

    if (mContext == PAUSE)
    {
        mChooseSide.mContext = CONTEXT_PAUSE;
    }
    else
    {
        mChooseSide.mContext = CONTEXT_FE;
    }

    mAsyncImage[0][0] = NULL;
    mAsyncImage[1][0] = NULL;
    mAsyncImage[0][1] = NULL;
    mAsyncImage[1][1] = NULL;
    mAsyncImage[0][2] = NULL;
    mAsyncImage[1][2] = NULL;
}

/**
 * Offset/Address/Size: 0x201C | 0x800C6BDC | size: 0x1AC
 */
SHChooseSides2::~SHChooseSides2()
{
    FEScrollText* ticker = m_pTicker;

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

    for (int i = 0; i < 3; i++)
    {
        if (mAsyncImage[0][i] != NULL)
        {
            delete mAsyncImage[0][i];
        }

        if (mAsyncImage[1][i] != NULL)
        {
            delete mAsyncImage[1][i];
        }
    }

    if (mContext == PAUSE)
    {
        g_bRenderWorld = true;
    }
}

/**
 * Offset/Address/Size: 0xCFC | 0x800C58BC | size: 0x1320
 */
void SHChooseSides2::SceneCreated()
{
    BindChooseSideInstances();

    TLSlide* currentSlide = m_pFEPresentation->m_currentSlide;

    TLTextInstance* scrollText = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("TickerText")));

    const gl_ScreenInfo* screenInfo = glGetScreenInfo();

    FEScrollText* ticker = new (nlMalloc(0x22C, 8, false)) FEScrollText(scrollText, 0, screenInfo->ScreenWidth + 0x32);
    m_pTicker = ticker;

    if (mContext == PAUSE)
    {
        m_pTicker->SetDisplayMessage(0x53B23764);
    }
    else
    {
        m_pTicker->SetDisplayMessage(0xACCB0957);
    }

    if (mContext != PAUSE)
    {
        const char* filename = "art/fe/LoadingScreensUI.res";

        for (int i = 0; i < 3; i++)
        {
            mAsyncImage[0][i] = new (nlMalloc(0x1C, 8, false)) AsyncImage(filename, NULL);
            mAsyncImage[1][i] = new (nlMalloc(0x1C, 8, false)) AsyncImage(filename, NULL);
        }

        TLComponentInstance* captaincomponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("LEFT_CAPT")));

        TLSlide* captainactiveslide = captaincomponent->GetActiveSlide();

        mAsyncImage[0][0]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_L")));

        mAsyncImage[0][2]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_L_WHITE")));

        mAsyncImage[0][1]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_L_OUT")));

        mSoundDelay = (captainactiveslide->m_start + captainactiveslide->m_duration) / 2.0f;

        captaincomponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("RIGHT_CAPT")));

        captainactiveslide = captaincomponent->GetActiveSlide();

        mAsyncImage[1][0]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_R")));

        mAsyncImage[1][2]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_R_WHITE")));

        mAsyncImage[1][1]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            captainactiveslide,
            InlineHasher(nlStringLowerHash("CAPT_R_OUT")));

        char mainfilename[128] = { };
        char outlinefilename[128] = { };
        char flashfilename[128] = { };

        for (int i = 0; i < 2; i++)
        {
            eTeamID teamid = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_0, mainfilename, 0x80, teamid, i);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_1, outlinefilename, 0x80, teamid, i);
            CaptainSidekickFilename::Build(CaptainSidekickFilename::TYPE_2, flashfilename, 0x80, teamid, i);

            mAsyncImage[i][0]->QueueLoad(mainfilename, true);
            mAsyncImage[i][1]->QueueLoad(outlinefilename, true);
            mAsyncImage[i][2]->QueueLoad(flashfilename, true);
        }

        TLComponentInstance* pNameComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("CAPTAIN_NAME_LEFT")));

        TLComponentInstance* pIconComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            pNameComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("COMPONENT")));

        pIconComp->SetActiveSlide(GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)0)));

        pNameComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("CAPTAIN_NAME_RIGHT")));

        pIconComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            pNameComp->GetActiveSlide(),
            InlineHasher(nlStringLowerHash("COMPONENT")));

        pIconComp->SetActiveSlide(GetTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)1)));
    }
    else
    {
        const char* filename = "art/fe/CaptainIconsUI.res";

        AsyncImage* image0 = new (nlMalloc(0x1C, 8, false)) AsyncImage(filename, NULL);
        mAsyncImage[0][0] = image0;

        mAsyncImage[0][0]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("HOME_AWAY")));

        AsyncImage* image1 = new (nlMalloc(0x1C, 8, false)) AsyncImage(filename, NULL);
        mAsyncImage[1][0] = image1;

        mAsyncImage[1][0]->mImageInstance = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("HOME_AWAY2")));

#pragma inline_depth(1)
        BasicString<char, Detail::TempStringAllocator> iconfilename[2];

        for (int i = 0; i < 2; i++)
        {
            eTeamID teamid = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i);
            switch (teamid)
            {
            case TEAM_DAISY:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_daisy");
                break;
            case TEAM_DONKEYKONG:
                if (i == 0)
                {
                    iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_dk");
                }
                else
                {
                    iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_dk_r");
                }
                break;
            case TEAM_LUIGI:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_luigi");
                break;
            case TEAM_MARIO:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_mario");
                break;
            case TEAM_PEACH:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_peach");
                break;
            case TEAM_WALUIGI:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_waluigi");
                break;
            case TEAM_WARIO:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_wario");
                break;
            case TEAM_YOSHI:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_yoshi");
                break;
            case TEAM_MYSTERY:
                iconfilename[i] = BasicString<char, Detail::TempStringAllocator>("fe/captain_icons/captain_icons_super");
                break;
            default:
                break;
            }

            mAsyncImage[i][0]->QueueLoad(iconfilename[i].c_str(), true);
        }
#pragma inline_depth()
    }

    TLComponentInstance* captainnamecomponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CAPTAIN_NAME_LEFT")));

    TLTextInstance* captaintext = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        captainnamecomponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("CAPTAIN_NAME")));

    captaintext->m_LocStrId = GetLOCTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)0));
    captaintext->m_OverloadFlags |= 8;

    captainnamecomponent = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CAPTAIN_NAME_RIGHT")));

    captaintext = FEFinder<TLTextInstance, 3>::Find<TLSlide>(
        captainnamecomponent->GetActiveSlide(),
        InlineHasher(nlStringLowerHash("CAPTAIN_NAME")));

    captaintext->m_LocStrId = GetLOCTeamName(nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)1));
    captaintext->m_OverloadFlags |= 8;

    TLComponentInstance* buttons = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
        currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("buttons")));

    if (buttons != NULL)
    {
        mButtons.mButtonInstance = buttons;

        if (mContext == PAUSE)
        {
            mButtons.SetState(ButtonComponent::BS_B_ONLY);
        }
        else
        {
            mButtons.SetState(ButtonComponent::BS_A_AND_B);
        }
    }

    GameInfoManager* const gim = nlSingleton<GameInfoManager>::s_pInstance;

    if (gim->IsInCupMode())
    {
        TLComponentInstance* sidesComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            currentSlide,
            InlineHasher(nlStringLowerHash("Layer")),
            InlineHasher(nlStringLowerHash("choose_side")));

        int lockedSide = (!(gim->GetUserSelectedCupTeam() - gim->GetTeam(0))) ? 1 : 0;

        for (int i = 0; i < 4; i++)
        {
            char arrowName[64];
            nlSNPrintf(arrowName, 64, "arrows%d", i + 1);

            TLComponentInstance* arrowComp = FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
                sidesComp->GetActiveSlide(),
                InlineHasher(nlStringLowerHash("Group")),
                InlineHasher(nlStringLowerHash(arrowName)));

            TLImageInstance* arrow1 = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
                arrowComp->GetActiveSlide(),
                InlineHasher(nlStringLowerHash("p1arrows1")),
                InlineHasher(nlStringLowerHash("arrow")));

            TLImageInstance* arrow2 = FEFinder<TLImageInstance, 2>::Find<TLSlide>(
                arrowComp->GetActiveSlide(),
                InlineHasher(nlStringLowerHash("p1arrows1")),
                InlineHasher(nlStringLowerHash("arrow2")));

            if (lockedSide == 0)
            {
                arrow1->m_bVisible = true;
                arrow2->m_bVisible = false;
            }
            else if (lockedSide == 1)
            {
                arrow1->m_bVisible = false;
                arrow2->m_bVisible = true;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x7BC | 0x800C537C | size: 0x540
 * TODO: 98.66% match - extra mr instruction in prologue (lwz r4/mr r29,r4/mr r3,r4
 * instead of lwz r29/mr r3,r29), rlwinm mask (24,31) vs srwi (5,31)
 */
void SHChooseSides2::UpdateChooseSideComponent(float fDeltaT)
{
    eFEINPUT_PAD pad = FE_ALL_PADS;
    GameInfoManager* gim = nlSingleton<GameInfoManager>::Instance();

    UpdateResult result;
    if (gim->IsInCupMode())
    {
        eTeamID cupTeam = gim->GetUserSelectedCupTeam();
        result = mChooseSide.Update(fDeltaT, &pad, (!(cupTeam - gim->GetTeam(0))) ? 1 : 0);
    }
    else
    {
        result = mChooseSide.Update(fDeltaT, &pad, -1);
    }

    switch (result)
    {
    case UPDATE_GO_FORWARD:
    {
        if (mContext == PAUSE)
        {
            if (mChooseSide.AllPlayersReady() || mChooseSide.AllPluggedInAreReady() || GetConfigBool(Config::Global(), "no_humans", false))
            {
                FEAudio::PlayAnimAudioEvent("sfx_back", false);

                if (mContext == PAUSE)
                {
                    GetAllSides();
                }
                SetAllSides(mChooseSide);
                if (mContext == PAUSE)
                {
                    g_pTeams[0]->UpdateControllers();
                    g_pTeams[1]->UpdateControllers();
                    nlSingleton<GameInfoManager>::s_pInstance->ApplyDifficultySettings();
                    g_pGame->SetDifficulty(
                        GetDifficulty0(nlSingleton<GameInfoManager>::s_pInstance),
                        GetDifficulty1(nlSingleton<GameInfoManager>::s_pInstance),
                        (eDifficultyID)3);
                }

                nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_PAUSE, SCREEN_BACK, true);
                break;
            }
        }

        if (mChooseSide.mPlayerReady[pad])
        {
            if (mChooseSide.AllPlayersReady())
            {
                mProceedDelay = 2;
                TLInstance* continueInst = mChooseSide.mInstanceTable[16];
                if (continueInst != NULL)
                {
                    continueInst->m_bVisible = false;
                }
            }
        }
        else
        {
            FEAudio::PlayAnimAudioEvent("sfx_accept", false);

            if (mChooseSide.AllPluggedInAreReady())
            {
                mProceedDelay = 2;
                TLInstance* continueInst = mChooseSide.mInstanceTable[16];
                if (continueInst != NULL)
                {
                    continueInst->m_bVisible = false;
                }
            }
            else
            {
                if (mContext != PAUSE)
                {
                    if (!mChooseSide.AtLeastOnePlayerReady())
                    {
                        FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);
                        popup->Create(POPUP_NO_SIDES_CHOSEN);
                    }
                }
            }
        }
        break;
    }

    case UPDATE_GO_BACK:
    {
        FEAudio::PlayAnimAudioEvent("sfx_back", false);

        if (mContext == PAUSE)
        {
            if (mChooseSide.AllControllersAreCentred())
            {
                if (!GetConfigBool(Config::Global(), "no_humans", false))
                {
                    FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<OverlayManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);
                    popup->Create(POPUP_NO_SIDES_CHOSEN);
                    break;
                }
            }

            if (mContext == PAUSE)
            {
                GetAllSides();
            }
            SetAllSides(mChooseSide);
            if (mContext == PAUSE)
            {
                g_pTeams[0]->UpdateControllers();
                g_pTeams[1]->UpdateControllers();
                nlSingleton<GameInfoManager>::s_pInstance->ApplyDifficultySettings();
                g_pGame->SetDifficulty(
                    GetDifficulty0(nlSingleton<GameInfoManager>::s_pInstance),
                    GetDifficulty1(nlSingleton<GameInfoManager>::s_pInstance),
                    (eDifficultyID)3);
            }

            if (nlSingleton<GameInfoManager>::s_pInstance->mIsInStrikers101Mode)
            {
                nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_STRIKERS_101_PAUSE, SCREEN_BACK, true);
            }
            else
            {
                PauseMenuScene* scene = (PauseMenuScene*)nlSingleton<OverlayManager>::s_pInstance->Push(IGSCENE_PAUSE, SCREEN_BACK, true);
                scene->mStartAnimAtEnd = true;
            }
        }
        else
        {
            nlSingleton<GameSceneManager>::s_pInstance->Push(mBackScene, SCREEN_BACK, true);
        }
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0x350 | 0x800C4F10 | size: 0x46C
 * TODO: 71.81% match - MWCC keeps InlineHasher(0) value in callee-saved register (r27)
 * instead of using li r0,0 per Find call. Causes stmw r23 (9 regs) vs target r24 (8 regs),
 * register-based zero stores vs target li+stw pattern, and fewer zero stores per call.
 * Inline InlineHasher(0) gives correct zero-store pattern but 0x1F0 frame (target 0x180).
 */
#pragma inline_depth(8)
void SHChooseSides2::BindChooseSideInstances()
{
    InlineHasher zH(0);
    FEPresentation* pPres = m_pFEPresentation;

    TLComponentInstance* choosesidecomponent = (TLComponentInstance*)FEFinder<TLInstance, 4>::Find<TLSlide>(
        pPres->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("CHOOSE_SIDE")),
        zH,
        zH,
        zH,
        zH);

    TLSlide* activeslide = choosesidecomponent->GetActiveSlide();
    unsigned char* pPadColour = (unsigned char*)PAD_COLOURS;

    for (int i = 0; i < 4; i++)
    {
        char tempstring[64];
        nlSNPrintf(tempstring, 64, "controller%d", i + 1);

        mChooseSide.mInstanceTable[i] = FEFinder<TLInstance, 5>::Find<TLSlide>(
            activeslide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempstring)),
            zH,
            zH,
            zH,
            zH);

        mChooseSide.mInstanceTable[i + 4] = FEFinder<TLInstance, 3>::Find<TLInstance>(
            mChooseSide.mInstanceTable[i],
            InlineHasher(nlStringLowerHash("ready")),
            zH,
            zH,
            zH,
            zH,
            zH);

        if (mChooseSide.mInstanceTable[i + 4])
        {
            mChooseSide.mInstanceTable[i + 4]->m_bVisible = false;
        }

        nlSNPrintf(tempstring, 64, "arrows%d", i + 1);

        mChooseSide.mInstanceTable[i + 12] = (TLInstance*)FEFinder<TLComponentInstance, 4>::Find<TLSlide>(
            activeslide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempstring)),
            zH,
            zH,
            zH,
            zH);

        nlSNPrintf(tempstring, 64, "p%d", i + 1);

        mChooseSide.mInstanceTable[i + 8] = (TLInstance*)FEFinder<TLTextInstance, 3>::Find<TLSlide>(
            activeslide,
            InlineHasher(nlStringLowerHash("group")),
            InlineHasher(nlStringLowerHash(tempstring)),
            zH,
            zH,
            zH,
            zH);

        nlColour colour;
        colour.c[0] = pPadColour[0];
        colour.c[1] = pPadColour[1];
        colour.c[2] = pPadColour[2];
        colour.c[3] = 0xFF;
        mChooseSide.mInstanceTable[i + 8]->SetAssetColour(colour);

        pPadColour += 3;
    }

    TLInstance* object = FEFinder<TLInstance, 2>::Find<TLSlide>(
        activeslide,
        InlineHasher(nlStringLowerHash("group")),
        InlineHasher(nlStringLowerHash("homex")),
        zH,
        zH,
        zH,
        zH);
    mChooseSide.mControllerDestPos[0] = object->GetAssetPosition().f.x;
    object->m_bVisible = false;

    object = FEFinder<TLInstance, 2>::Find<TLSlide>(
        activeslide,
        InlineHasher(nlStringLowerHash("group")),
        InlineHasher(nlStringLowerHash("awayx")),
        zH,
        zH,
        zH,
        zH);
    mChooseSide.mControllerDestPos[1] = object->GetAssetPosition().f.x;
    object->m_bVisible = false;

    mChooseSide.mControllerDestPos[2] = mChooseSide.mInstanceTable[0]->GetAssetPosition().f.x;

    mChooseSide.mInstanceTable[16] = FEFinder<TLInstance, 4>::Find<TLSlide>(
        pPres->m_currentSlide,
        InlineHasher(nlStringLowerHash("Layer")),
        InlineHasher(nlStringLowerHash("continue")),
        zH,
        zH,
        zH,
        zH);

    mChooseSide.ResetAndPositionControllers(false);
}
#pragma inline_depth()

static inline void GetAllSides()
{
    for (int i = 0; i < 4; i++)
    {
        nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((u16)i);
    }
}

static inline void SetAllSides(IChooseSide& cs)
{
    for (int i = 0; i < 4; i++)
    {
        nlSingleton<GameInfoManager>::s_pInstance->SetPlayingSide((u16)i, (short)cs.mPlayingSides[i]);
    }
}

static inline eDifficultyID GetDifficulty0(GameInfoManager* gameInfo)
{
    return gameInfo->mCurrentDifficulty[0];
}

static inline eDifficultyID GetDifficulty1(GameInfoManager* gameInfo)
{
    return gameInfo->mCurrentDifficulty[1];
}

/**
 * Offset/Address/Size: 0x0 | 0x800C4BC0 | size: 0x350
 */
void SHChooseSides2::Update(float fDeltaT)
{
    BaseSceneHandler::Update(fDeltaT);
    mButtons.CentreButtons();

    if (mProceedDelay > 0)
    {
        mProceedDelay--;

        if (mProceedDelay == 0)
        {
            FEAudio::PlayAnimAudioEvent("choose_sides_proceed", false);

            if (mContext == PAUSE)
            {
                GetAllSides();
            }

            SetAllSides(mChooseSide);

            if (mContext == PAUSE)
            {
                g_pTeams[0]->UpdateControllers();
                g_pTeams[1]->UpdateControllers();
                nlSingleton<GameInfoManager>::s_pInstance->ApplyDifficultySettings();
                g_pGame->SetDifficulty(
                    GetDifficulty0(nlSingleton<GameInfoManager>::s_pInstance),
                    GetDifficulty1(nlSingleton<GameInfoManager>::s_pInstance),
                    (eDifficultyID)3);
            }

            FrontEnd::SetControllerState();

            if (mNextScene == SCENE_SUPER_LOADING)
            {
                nlSingleton<GameSceneManager>::s_pInstance->PushLoadingScene(true);
            }
            else
            {
                nlSingleton<GameSceneManager>::s_pInstance->Push(mNextScene, SCREEN_FORWARD, true);
            }

            mProceedDelay = -1;
        }
    }
    else
    {
        if (mSoundDelay > 0.0f && mContext != PAUSE)
        {
            mSoundDelay -= fDeltaT;

            if (mSoundDelay <= 0.0f)
            {
                mSoundDelay = 0.0f;

                for (int i = 0; i < 2; i++)
                {
                    eTeamID teamid = nlSingleton<GameInfoManager>::s_pInstance->GetTeam((short)i);
                    FECharacterSound::PlayCaptainSlideIn(teamid);
                }
            }
        }

        if (g_bRenderWorld && mContext == PAUSE)
        {
            g_bRenderWorld = false;
        }

        for (int i = 0; i < 3; i++)
        {
            if (mAsyncImage[0][i] != NULL)
            {
                bool swapresult = mAsyncImage[0][i]->Update(true);

                if (swapresult && mContext == PAUSE)
                {
                    mAsyncImage[0][i]->FreeLoadBuffer();
                }
            }

            if (mAsyncImage[1][i] != NULL)
            {
                bool swapresult = mAsyncImage[1][i]->Update(true);

                if (swapresult && mContext == PAUSE)
                {
                    mAsyncImage[1][i]->FreeLoadBuffer();
                }
            }
        }

        TLSlide* currentSlide = m_pFEPresentation->m_currentSlide;

        if (currentSlide->m_time < (currentSlide->m_start + currentSlide->m_duration))
        {
            TLInstance* instance;
            for (int i = 0; i < 4; i++)
            {
                instance = mChooseSide.mInstanceTable[i];

                if (g_pFEInput->IsConnected((eFEINPUT_PAD)i))
                {
                    instance->m_bVisible = true;
                    mChooseSide.PositionController(i, false, true);
                }
                else
                {
                    instance->m_bVisible = false;
                    mChooseSide.mPlayingSides[i] = -1;

                    instance = mChooseSide.mInstanceTable[i + 8];
                    if (instance != NULL)
                    {
                        instance->m_bVisible = true;
                    }

                    instance = mChooseSide.mInstanceTable[i + 12];
                    if (instance != NULL)
                    {
                        instance->m_bVisible = false;
                    }

                    mChooseSide.PositionController(i, false, false);
                    mChooseSide.SetReady(i, false);
                }
            }
        }
        else
        {
            m_pTicker->Update(fDeltaT);
            UpdateChooseSideComponent(fDeltaT);
        }
    }
}
