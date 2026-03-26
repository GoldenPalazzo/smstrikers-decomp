#include "Game/SH/SHCupHub.h"

#include "Game/GameSceneManager.h"
#include "Game/BaseGameSceneManager.h"
#include "Game/FE/feFinder.h"
#include "Game/FE/feHelpFuncs.h"
#include "Game/GameInfo.h"
#include "NL/nlMath.h"
#include "NL/nlMemory.h"
#include "NL/nlPrint.h"

static const char* CUP_HUB_LAYER_NAME;
static nlColour HUB_COLOUR_HIGHLIGHT;
static char* HUBstandingsRowNames[10];
static unsigned char gHubLeagueMovementSoundIsPlaying;
static unsigned char gHubKnockoutMovementSoundIsPlaying;

namespace Audio
{
enum eWorldSFX
{
    WORLDSFX_DUMMY = 0,
};

class cWorldSFX : public cGameSFX
{
public:
    void Stop(eWorldSFX, cGameSFX::StopFlag);
};

extern cWorldSFX gWorldSFX;
} // namespace Audio

extern "C" eTeamID FindWinningTeam__15GameInfoManagerFv(GameInfoManager*);
extern "C" BasicGameInfo* GetMatchupInfo__15GameInfoManagerCFsUs(const GameInfoManager*, short, unsigned short);

class CupTrophyScene
{
public:
    void CreateTrophyScene(eTrophyType, ButtonComponent::ButtonState, bool);
};

namespace Detail
{
template <typename R, typename F>
struct MemFunImpl
{
    F mFuncPtr;
};
} // namespace Detail

template <typename T, typename R>
Detail::MemFunImpl<R, void (T::*)()> MemFun(void (T::*)());

template <typename R, typename F, typename A>
BindExp1<R, F, A> Bind(F fn, const A& arg);

enum ePopupMenu
{
    POPUP_TOURNEY_OVER = 14,
};

class FEPopupMenu
{
public:
    void Create(ePopupMenu, Function<FnVoidVoid>&, Function<FnVoidVoid>&);
    static void Nothing();
};

typedef Detail::MemFunImpl<void, void (CupHubScene::*)()> MemFunImpl_CupHubScene_v;
typedef BindExp1<void, MemFunImpl_CupHubScene_v, CupHubScene*> BindExp1_vfmfcp;
typedef Function0<void>::FunctorImpl<BindExp1_vfmfcp> FunctorImpl_vfmfcp;

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1F90 | size: 0x38
//  */
// void Bind<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>(Detail::MemFunImpl<void, void (CupHubScene::*)()>,
// CupHubScene* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1F74 | size: 0x1C
//  */
// void MemFun<CupHubScene, void>(void (CupHubScene::*)())
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1F18 | size: 0x5C
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1EB4 | size: 0x64
//  */
// TeamStats::TeamStats()
// {
// }

// /**
//  * Offset/Address/Size: 0x78 | 0x800F1E84 | size: 0x30
//  */
// void Function0<void>::FunctorImpl<BindExp1<void, Detail::MemFunImpl<void, void (CupHubScene::*)()>, CupHubScene*>>::operator()()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x800F1E0C | size: 0x78
 * Clone() generated from inline definition in NL/nlFunction.h
 */

// /**
//  * Offset/Address/Size: 0x680 | 0x800F1D88 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<FEPresentation>(FEPresentation*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x800F1D50 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<FEPresentation>(FEPresentation*, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher, InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x4EC | 0x800F1BF4 | size: 0x15C
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x468 | 0x800F1B70 | size: 0x84
//  */
// void FEFinder<TLComponentInstance, 4>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x430 | 0x800F1B38 | size: 0x38
//  */
// void FEFinder<TLComponentInstance, 4>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0x2D4 | 0x800F19DC | size: 0x15C
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x250 | 0x800F1958 | size: 0x84
//  */
// void FEFinder<TLTextInstance, 3>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x218 | 0x800F1920 | size: 0x38
//  */
// void FEFinder<TLTextInstance, 3>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x800F17C4 | size: 0x15C
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLInstance>(TLInstance*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned
// long, unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x38 | 0x800F1740 | size: 0x84
//  */
// void FEFinder<TLImageInstance, 2>::_Find<TLSlide>(TLSlide*, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long,
// unsigned long)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x800F1708 | size: 0x38
//  */
// void FEFinder<TLImageInstance, 2>::Find<TLSlide>(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher,
// InlineHasher)
// {
// }

/**
 * Offset/Address/Size: 0x72C8 | 0x800F1024 | size: 0x6E4
 */
CupHubScene::CupHubScene(bool, bool)
{
}

/**
 * Offset/Address/Size: 0x7224 | 0x800F0F80 | size: 0xA4
 */
CupHubScene::~CupHubScene()
{
    delete mCaptainImage;
}

/**
 * Offset/Address/Size: 0x71C0 | 0x800F0F1C | size: 0x64
 */
void CupHubScene::SceneCreated()
{
    LoadCaptainImage();
    eHubState state = mHubState;
    switch (state)
    {
    case HUB_LEAGUE:
    case HUB_BOWSER_TRANSITION:
        CreateLeague();
        break;
    case HUB_KNOCKOUT2:
    case HUB_KNOCKOUT4:
    case HUB_KNOCKOUT8:
        CreateKnockout();
        break;
    default:
        break;
    }
}

/**
 * Offset/Address/Size: 0x69A4 | 0x800F0700 | size: 0x81C
 */
void CupHubScene::Update(float)
{
}

/**
 * Offset/Address/Size: 0x670C | 0x800F0468 | size: 0x298
 * TODO: 99.97% match - remaining i diff on the MemFun literal-label load pair (@1204/@562).
 */
void CupHubScene::EndCup()
{
    CupHubScene* self = this;

    if (nlSingleton<GameInfoManager>::s_pInstance->mDisplayTrophy[0] && nlSingleton<GameInfoManager>::s_pInstance->IsInCupMode())
    {
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();

        CupTrophyScene* trophyScene = (CupTrophyScene*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_TROPHY, SCREEN_FORWARD, false);

        eTrophyType trophyType = nlSingleton<GameInfoManager>::s_pInstance->GetTrophyTypeByCurrentMode();
        trophyScene->CreateTrophyScene(trophyType, ButtonComponent::BS_A_ONLY, true);
    }
    else if (nlSingleton<GameInfoManager>::s_pInstance->IsInTournamentMode())
    {
        if (nlSingleton<GameInfoManager>::s_pInstance->GetNumHumanTeams() > 1)
        {
            nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
            nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_TOURNEY_BRAG, SCREEN_FORWARD, false);
        }
        else
        {
            FEPopupMenu* popup = (FEPopupMenu*)nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_POPUP_MENU, SCREEN_NOTHING, false);

            BindExp1_vfmfcp bind = Bind<void, MemFunImpl_CupHubScene_v, CupHubScene*>(
                MemFun<CupHubScene, void>(&CupHubScene::ReturnToMainMenu), self);

            {
                Function<FnVoidVoid> yes;
                yes.mTag = FUNCTOR;

                FunctorImpl_vfmfcp* functor = new ((FunctorImpl_vfmfcp*)nlMalloc(sizeof(FunctorImpl_vfmfcp), 8, false)) FunctorImpl_vfmfcp(bind);
                yes.mFunctor = functor;

                Function<FnVoidVoid> no;
                no.mTag = FREE_FUNCTION;
                no.mFreeFunction = FEPopupMenu::Nothing;

                popup->Create(POPUP_TOURNEY_OVER, yes, no);
            }
        }
    }
    else
    {
        nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
        nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_CUP_BRAG, SCREEN_FORWARD, false);
    }
}

/**
 * Offset/Address/Size: 0x66C8 | 0x800F0424 | size: 0x44
 */
void CupHubScene::ReturnToMainMenu()
{
    nlSingleton<GameSceneManager>::s_pInstance->PopEntireStack();
    nlSingleton<GameSceneManager>::s_pInstance->Push(SCENE_MAIN_MENU, SCREEN_FORWARD, false);
}

/**
 * Offset/Address/Size: 0x5F08 | 0x800EFC64 | size: 0x7C0
 */
unsigned char CupHubScene::UpdateDisplayedStat()
{
    FORCE_DONT_INLINE;
    return false;
}

/**
 * Offset/Address/Size: 0x4E34 | 0x800EEB90 | size: 0x10D4
 */
void CupHubScene::CreateLeague()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x4018 | 0x800EDD74 | size: 0xE1C
 */
void CupHubScene::CreateBowserLeague()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x32FC | 0x800ED058 | size: 0xD1C
 */
void CupHubScene::CreateKnockout()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x2FF4 | 0x800ECD50 | size: 0x308
 * TODO: 99.15% match - persistent r29/r30 allocation swap around shouldStartSound/index pointers.
 */
unsigned char CupHubScene::UpdateLeague(float fDeltaT)
{
    unsigned char shouldStartSound = 0;

    mStatUpdateDelay += fDeltaT;

    if (mUpdatingStats)
    {
        if (mStatUpdateDelay >= 0.2)
        {
            mStatUpdateDelay = 0.0f;

            if (UpdateDisplayedStat())
            {
                return 0;
            }

            int i = 0;
            while (i < nlSingleton<GameInfoManager>::s_pInstance->GetNumPlayingTeams())
            {
                int oldRank = mOldRanks[mAllTeamStats[i].mTeamIndex];
                int newRank = mNewRanks[mAllTeamStats[i].mTeamIndex];

                if (oldRank != newRank)
                {
                    mRowMovement[oldRank] = (float)((oldRank - newRank) * 24);
                }

                i++;
            }

            mUpdatingStats = false;
            shouldStartSound = 1;
        }
        else
        {
            return 0;
        }
    }

    unsigned char shouldBreak = 0;
    unsigned int i = 0;

    while (i < 8)
    {
        if (mRowMovement[i] > 1.0)
        {
            mRowMovement[i] -= 1.0f;

            feVector3 position = mAnimComponents[i]->GetAssetPosition();
            mAnimComponents[i]->SetAssetPosition(position.f.x, position.f.y + 1.0f, position.f.z);

            shouldBreak = 1;
        }
        else if (mRowMovement[i] < -1.0)
        {
            mRowMovement[i] += 1.0f;

            feVector3 position = mAnimComponents[i]->GetAssetPosition();
            mAnimComponents[i]->SetAssetPosition(position.f.x, position.f.y - 1.0f, position.f.z);

            shouldBreak = 1;
        }

        i++;
    }

    if (shouldBreak)
    {
        if (shouldStartSound)
        {
            if (!gHubLeagueMovementSoundIsPlaying)
            {
                FEAudio::PlayAnimAudioEvent("sfx_hub_league_movement", true);
            }

            gHubLeagueMovementSoundIsPlaying = true;
        }

        return 0;
    }

    ColourUserRow();

    if (mHubState == HUB_BOWSER_TRANSITION)
    {
        CreateBowserLeague();
        ColourUserRow();
        mDoAnimations = false;
    }
    else
    {
        mDoAnimations = false;
        UpdateProgressIndicator();
    }

    if (gHubLeagueMovementSoundIsPlaying)
    {
        Audio::gWorldSFX.Stop((Audio::eWorldSFX)0x11, cGameSFX::SFX_STOP_FIRST);
    }

    gHubLeagueMovementSoundIsPlaying = false;

    if (nlSingleton<GameInfoManager>::s_pInstance->GetCurrentRoundNumber() != 0)
    {
        int i = 0;

        while (i < nlSingleton<GameInfoManager>::s_pInstance->GetNumPlayingTeams())
        {
            if (mNewRanks[mAllTeamStats[i].mTeamIndex] == 0)
            {
                FECharacterSound::PlayCaptainName(mAllTeamStats[i].mTeamIndex);
                break;
            }

            i++;
        }
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x2A64 | 0x800EC7C0 | size: 0x590
 */
void CupHubScene::UpdateKnockout8(float)
{
}

/**
 * Offset/Address/Size: 0x2600 | 0x800EC35C | size: 0x464
 * TODO: 98.43% match - function-scope volatile hashers allocated at different stack offsets
 * than target (0x68-0x74 vs 0x4c-0x64), plus SDA21 offset diffs for globals. All remaining
 * diffs are s/i (stack offset and immediate) only - no register or instruction diffs.
 */
s32 CupHubScene::UpdateKnockout4(float fDeltaT)
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    TLSlide* pSlide1 = mAnimComponents[mAnimatingKnockoutTeams[0]]->GetActiveSlide();
    TLSlide* pSlide2 = mAnimComponents[mAnimatingKnockoutTeams[1]]->GetActiveSlide();
    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;

    volatile InlineHasher h7, h5, h3, h1;

    if (mKnockoutLoserAnimations && mSlideSwitchDelay > 0.0f)
    {
        mSlideSwitchDelay -= fDeltaT;
        mAnimComponents[mAnimatingKnockoutTeams[0]]->SetActiveSlide("Eliminated");
        mAnimComponents[mAnimatingKnockoutTeams[1]]->SetActiveSlide("Eliminated");

        if (mSlideSwitchDelay <= 0.0f)
            FEAudio::PlayAnimAudioEvent("sfx_hub_knockout_elimination", false);

        return 0;
    }

    mSlideSwitchDelay = 0.0f;

    if ((pSlide1->m_time < (pSlide1->m_start + pSlide1->m_duration)) || (pSlide2->m_time < (pSlide2->m_start + pSlide2->m_duration)))
        return 0;

    if (mKnockoutLoserAnimations)
    {
        BasicGameInfo* pGame = GetMatchupInfo__15GameInfoManagerCFsUs(gameInfo, -3, 0);
        mAnimatingKnockoutTeams[0] = (pGame->mFinalScore[0] > pGame->mFinalScore[1]) ? 0 : 1;
        mAnimComponents[mAnimatingKnockoutTeams[0]]->SetActiveSlide("Move");
        mAnimComponents[mAnimatingKnockoutTeams[0]]->GetActiveSlide()->Update(0.0f);

        pGame = GetMatchupInfo__15GameInfoManagerCFsUs(gameInfo, -3, 1);
        mAnimatingKnockoutTeams[1] = (pGame->mFinalScore[0] > pGame->mFinalScore[1]) ? 2 : 3;
        mAnimComponents[mAnimatingKnockoutTeams[1]]->SetActiveSlide("Move");
        mAnimComponents[mAnimatingKnockoutTeams[1]]->GetActiveSlide()->Update(0.0f);

        FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

        volatile InlineHasher hB, hA, h9, h8, h6, h4, h2, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("vs");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash(CUP_HUB_LAYER_NAME);
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        union
        {
            FindCompByValue byValue;
            FindCompByRef byRef;
        } findComp;

        findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

        TLComponentInstance* vsComp = findComp.byRef(
            presentation->m_currentSlide,
            (InlineHasher&)hB,
            (InlineHasher&)h9,
            (InlineHasher&)h7,
            (InlineHasher&)h5,
            (InlineHasher&)h3,
            (InlineHasher&)h1);

        vsComp->SetActiveSlide("Slide2");

        mKnockoutLoserAnimations = false;

        if (!gHubKnockoutMovementSoundIsPlaying)
        {
            FEAudio::PlayAnimAudioEvent("sfx_hub_knockout_movment", true);
        }

        gHubKnockoutMovementSoundIsPlaying = true;
        return 0;
    }

    mDoAnimations = false;
    UpdateProgressIndicator();

    FEPresentation* presentation = m_pFEScene->m_pFEPackage->GetPresentation();

    volatile InlineHasher hB, hA, h9, h8, h6, h4, h2, h0;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long hash = nlStringLowerHash("message");
    h8.m_Hash = hash;
    h9.m_Hash = hash;

    hash = nlStringLowerHash(CUP_HUB_LAYER_NAME);
    hB.m_Hash = hash;
    hA.m_Hash = hash;

    union
    {
        FindCompByValue byValue;
        FindCompByRef byRef;
    } findComp;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    TLComponentInstance* pComp = findComp.byRef(
        presentation->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    pComp->SetActiveSlide("Slide1");
    pComp->Update(0.0f);
    pComp->m_bVisible = true;

    feVector3 position = pComp->GetAssetPosition();
    pComp->SetAssetPosition(position.f.x, 60.0f, position.f.z);

    volatile InlineHasher g7, g6, g5, g4, g3, g2, g1, g0;

    g0.m_Hash = 0;
    h1.m_Hash = 0;
    g1.m_Hash = 0;
    h3.m_Hash = 0;
    g2.m_Hash = 0;
    h5.m_Hash = 0;
    g3.m_Hash = 0;
    h7.m_Hash = 0;
    g4.m_Hash = 0;
    g5.m_Hash = 0;

    hash = nlStringLowerHash("Text");
    g6.m_Hash = hash;
    g7.m_Hash = hash;

    union
    {
        FindTextByValue byValue;
        FindTextByRef byRef;
    } findText;

    findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;

    TLTextInstance* pText = findText.byRef(
        pComp->GetActiveSlide(),
        (InlineHasher&)g7,
        (InlineHasher&)g5,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    pText->SetStringId("STANDINGS_FINAL");

    if (gHubKnockoutMovementSoundIsPlaying)
    {
        Audio::gWorldSFX.Stop((Audio::eWorldSFX)0x11, cGameSFX::SFX_STOP_FIRST);
    }

    gHubKnockoutMovementSoundIsPlaying = false;

    if (!mAllKnockoutAnimations && mPlayPopSound)
    {
        FEAudio::PlayAnimAudioEvent("sfx_standings_round_pop", false);
        mPlayPopSound = false;
    }
    return 1;
}

/**
 * Offset/Address/Size: 0x1F6C | 0x800EBCC8 | size: 0x694
 */
void CupHubScene::UpdateKnockout2(float)
{
}

/**
 * Offset/Address/Size: 0x1860 | 0x800EB5BC | size: 0x70C
 */
void CupHubScene::UpdateProgressIndicator()
{
    FORCE_DONT_INLINE;
}

/**
 * Erased (inlined into ColourUserRow)
 */
static unsigned char IsUserRow(eTeamID teamInRow)
{
    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    eTeamID userTeam = gameInfo->GetUserSelectedCupTeam();
    unsigned long teamMask = 1 << teamInRow;

    if ((gameInfo->mCurrentCup->mHumanTeams & teamMask) == 0)
        return 0;

    if ((gameInfo->GetNumHumanTeams() == 1) && (teamInRow == userTeam))
        return 1;

    return 0;
}

/**
 * Offset/Address/Size: 0x1698 | 0x800EB3F4 | size: 0x1C8
 * TODO: 99.1% match - extra mr r30,r0 for HUBstandingsRowNames address load (compiler scheduling artifact)
 */
void CupHubScene::ColourUserRow()
{
    typedef TLComponentInstance* (*FindCompByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindCompByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);
    typedef TLTextInstance* (*FindTextByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLTextInstance* (*FindTextByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    FEPresentation* pres = m_pFEPresentation;
    int standingsIndices[8];
    char** pRowName;
    int* pStandingsIndices;
    TLTextInstance* pTextInstance;
    int numTeams = nlSingleton<GameInfoManager>::s_pInstance->GetNumPlayingTeams();
    int row;

    nlSingleton<StatsTracker>::s_pInstance->GetSortedTeamStats(mAllTeamStats, numTeams, standingsIndices, numTeams);

    pStandingsIndices = standingsIndices;
    pRowName = HUBstandingsRowNames;

    for (row = 0; row < numTeams; row++)
    {
        volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

        h0.m_Hash = 0;
        h1.m_Hash = 0;
        h2.m_Hash = 0;
        h3.m_Hash = 0;
        h4.m_Hash = 0;
        h5.m_Hash = 0;
        h6.m_Hash = 0;
        h7.m_Hash = 0;

        unsigned long hash = nlStringLowerHash("@4212");
        h8.m_Hash = hash;
        h9.m_Hash = hash;

        hash = nlStringLowerHash(CUP_HUB_LAYER_NAME);
        hB.m_Hash = hash;
        hA.m_Hash = hash;

        TLComponentInstance* pComp;
        {
            union
            {
                FindCompByValue byValue;
                FindCompByRef byRef;
            } findComp;

            findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;
            pComp = findComp.byRef(
                pres->m_currentSlide,
                (InlineHasher&)hB,
                (InlineHasher&)h9,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);
        }

        TLSlide* pSlide = pComp->GetActiveSlide();

        volatile InlineHasher g7, g6, g5, g4, g3, g2, g1, g0;

        g0.m_Hash = 0;
        h1.m_Hash = 0;
        g1.m_Hash = 0;
        h3.m_Hash = 0;
        g2.m_Hash = 0;
        h5.m_Hash = 0;
        g3.m_Hash = 0;
        h7.m_Hash = 0;
        g4.m_Hash = 0;
        g5.m_Hash = 0;

        hash = nlStringLowerHash(*pRowName);
        g6.m_Hash = hash;
        g7.m_Hash = hash;

        {
            union
            {
                FindTextByValue byValue;
                FindTextByRef byRef;
            } findText;

            findText.byValue = FEFinder<TLTextInstance, 3>::Find<TLSlide>;
            pTextInstance = findText.byRef(
                pSlide,
                (InlineHasher&)g7,
                (InlineHasher&)g5,
                (InlineHasher&)h7,
                (InlineHasher&)h5,
                (InlineHasher&)h3,
                (InlineHasher&)h1);
        }

        if (row < numTeams)
        {
            eTeamID currentTeam = mAllTeamStats[*pStandingsIndices].mTeamIndex;
            if (IsUserRow(currentTeam))
            {
                pTextInstance->SetAssetColour(HUB_COLOUR_HIGHLIGHT);
                break;
            }
        }

        pRowName++;
        pStandingsIndices++;
    }
}

/**
 * Offset/Address/Size: 0x15C4 | 0x800EB320 | size: 0xD4
 */
void CupHubScene::HandleButtonComponent()
{
    typedef TLComponentInstance* (*FindByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLComponentInstance* (*FindByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    union
    {
        FindByValue byValue;
        FindByRef byRef;
    } findComp;

    findComp.byValue = FEFinder<TLComponentInstance, 4>::Find<TLSlide>;

    FEPresentation* pres = m_pFEPresentation;

    volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    h0.m_Hash = 0;
    h1.m_Hash = 0;
    h2.m_Hash = 0;
    h3.m_Hash = 0;
    h4.m_Hash = 0;
    h5.m_Hash = 0;
    h6.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long buttonHash = nlStringLowerHash("@4158");
    h8.m_Hash = buttonHash;
    h9.m_Hash = buttonHash;

    unsigned long layerHash = nlStringLowerHash(CUP_HUB_LAYER_NAME);
    hB.m_Hash = layerHash;
    hA.m_Hash = layerHash;

    TLComponentInstance* inst = findComp.byRef(
        pres->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    mButtons.mButtonInstance = inst;
    inst->m_bVisible = false;

    s16 roundNum = nlSingleton<GameInfoManager>::s_pInstance->GetCurrentRoundNumber();
    if (roundNum == -5)
    {
        mButtons.SetState(ButtonComponent::BS_A_ONLY);
    }
    else
    {
        mButtons.SetState(ButtonComponent::BS_A_AND_B);
    }
}

/**
 * Offset/Address/Size: 0x12EC | 0x800EB048 | size: 0x2D8
 * TODO: 99.59% match - remaining register allocation mismatch in round-index loop and current-cup pointer reuse
 */
void CupHubScene::SetRoundColours(eHubColour* coloursArray, int sizeOfArray)
{
    GameInfoManager* gameInfo = nlSingleton<GameInfoManager>::s_pInstance;
    int i;

    for (i = 0; i < sizeOfArray; i++)
    {
        coloursArray[i] = (eHubColour)2;
    }

    if (!gameInfo->IsInTournamentMode())
    {
        if (!gameInfo->mDoingKnockout)
        {
            int currentRound = (s16)gameInfo->GetCurrentRoundNumber();
            int firstRound = (s16)gameInfo->GetFirstRoundNumber();
            if (currentRound != firstRound)
            {
                int lastPlayedRound = (s16)gameInfo->GetPreviousRoundNumber((s16)currentRound);
                BaseCup* cup = gameInfo->mCurrentCup;
                eHubColour* pColour = coloursArray;
                int green = 1;
                int red = 0;
                int yellow = 3;
                int k;

                for (k = 0; k <= lastPlayedRound; k++)
                {
                    int roundResult = *cup->GetRoundResults(k);
                    if (roundResult == 0)
                    {
                        *pColour = (eHubColour)green;
                    }
                    else if (roundResult == 1)
                    {
                        *pColour = (eHubColour)red;
                    }
                    else if (roundResult == 2)
                    {
                        *pColour = (eHubColour)yellow;
                    }

                    pColour++;
                }
            }
        }
        else
        {
            eHubColour* pColour;
            BaseCup* cup = gameInfo->mPreviousCup;
            int numRounds = cup->GetNumRounds();
            pColour = coloursArray;
            int k = 0;
            int green = 1;
            int red = 0;
            int yellow = 3;

            while (k < numRounds)
            {
                int roundResult = *cup->GetRoundResults(k);
                if (roundResult == 0)
                {
                    *pColour = (eHubColour)green;
                }
                else if (roundResult == 1)
                {
                    *pColour = (eHubColour)red;
                }
                else if (roundResult == 2)
                {
                    *pColour = (eHubColour)yellow;
                }

                pColour++;
                k++;
            }

            cup = gameInfo->mCurrentCup;
            int round = gameInfo->GetCurrentRoundNumber();
            if (((u32)(round + 2) <= 1) || (round == -5))
            {
                int roundResult = *cup->GetRoundResults(0);
                if (roundResult == 0)
                {
                    coloursArray[numRounds] = (eHubColour)1;
                }
                else if ((roundResult == 1) || (roundResult == 2))
                {
                    coloursArray[numRounds] = (eHubColour)0;
                }
            }

            if (round == -5)
            {
                int roundResult = *cup->GetRoundResults(1);
                if (roundResult == 0)
                {
                    coloursArray[numRounds + 1] = (eHubColour)1;
                }
                else if ((roundResult == 1) || (roundResult == 2))
                {
                    coloursArray[numRounds + 1] = (eHubColour)0;
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1DC | 0x800E9F38 | size: 0x1110
 */
void CupHubScene::UpdateRoundMessage(bool)
{
}

/**
 * Offset/Address/Size: 0x0 | 0x800E9D5C | size: 0x1DC
 */
void CupHubScene::LoadCaptainImage()
{
    typedef TLImageInstance* (*FindImageByValue)(TLSlide*, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher, InlineHasher);
    typedef TLImageInstance* (*FindImageByRef)(TLSlide*, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&, InlineHasher&);

    volatile InlineHasher hB, hA, h9, h8, h7, h6, h5, h4, h3, h2, h1, h0;

    h0.m_Hash = 0;
    h2.m_Hash = 0;
    h4.m_Hash = 0;
    h6.m_Hash = 0;

    GameInfoManager* gameInfoMgr = *(GameInfoManager* volatile*)&nlSingleton<GameInfoManager>::s_pInstance;

    h1.m_Hash = 0;
    h3.m_Hash = 0;
    h5.m_Hash = 0;
    h7.m_Hash = 0;

    unsigned long imageHash = nlStringLowerHash("WALUIGI_L");
    h8.m_Hash = imageHash;
    h9.m_Hash = imageHash;

    unsigned long layerHash = nlStringLowerHash(CUP_HUB_LAYER_NAME);
    hB.m_Hash = layerHash;
    hA.m_Hash = layerHash;

    union
    {
        FindImageByValue byValue;
        FindImageByRef byRef;
    } findImage;
    findImage.byValue = FEFinder<TLImageInstance, 2>::Find<TLSlide>;
    TLImageInstance* imageInst = findImage.byRef(
        m_pFEPresentation->m_currentSlide,
        (InlineHasher&)hB,
        (InlineHasher&)h9,
        (InlineHasher&)h7,
        (InlineHasher&)h5,
        (InlineHasher&)h3,
        (InlineHasher&)h1);

    eTeamID teamId;
    if (gameInfoMgr->IsInCupMode() || gameInfoMgr->GetNumHumanTeams() == 1)
    {
        teamId = gameInfoMgr->GetUserSelectedCupTeam();
    }
    else
    {
        s16 roundNum = gameInfoMgr->GetCurrentRoundNumber();
        if (roundNum != -5)
        {
            u16 numTeams = gameInfoMgr->GetNumPlayingTeams();
            u32 randomResult = nlRandom(numTeams, &nlDefaultSeed);
            u16 randomIndex = (u16)randomResult;
            TeamStats stats = gameInfoMgr->GetTeamStatsByIndex(randomIndex);
            teamId = stats.mTeamIndex;
        }
        else
        {
            teamId = FindWinningTeam__15GameInfoManagerFv(gameInfoMgr);
        }
    }

    const char* teamName = GetTeamName(teamId);
    char buffer[0x80];
    nlSNPrintf(buffer, 0x80, "fe/cup_loadingscreens/%s_l", teamName);

    mCaptainImage->mImageInstance = imageInst;
    mCaptainImage->QueueLoad(buffer, true);
}
