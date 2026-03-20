#include "Game/Team.h"

#include "Game/AI/Fielder.h"
#include "Game/Audio/WorldAudio.h"
#include "Game/GameInfo.h"
#include "Game/OverlayHandlerHUD.h"
#include "Game/OverlayManager.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Powerups.h"
#include "Game/Ball.h"
#include "Game/Game.h"
#include "Game/BasicStadium.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Sys/audio.h"

#include "Game/AI/Scripts/ScriptQuestions.h"

#include "NL/nlMath.h"
#include <stdlib.h>

cTeam* g_pTeams[2] = { NULL, NULL };

/**
 * Offset/Address/Size: 0x1CF0 | 0x8006609C | size: 0x138
 */
cTeam::cTeam(int side)
{
    m_nSide = side;
    m_nScore = 0;
    m_nCurrentPowerUp = 0;
    mfPowerupMeter = 0.0f;
    mfPowerupTimer = 0.0f;
    meCurrentTeamStyle = TEAM_STYLE_MODERATE;

    mtTeamStyleTimer.SetSeconds(0.0f);
    mtMarkTimer.SetSeconds(0.0f);
    mtRoleTimer.SetSeconds(0.0f);
    mtDefensiveZoneTimer.SetSeconds(0.0f);
    mbHasToggledPowerup = false;
    mtBallInterceptTimer.SetSeconds(0.0f);

    m_ePowerupList[0].nnumOfPowerups = 0;
    m_ePowerupList[0].eType = POWER_UP_NONE;
    m_ePowerupList[1].nnumOfPowerups = 0;
    m_ePowerupList[1].eType = POWER_UP_NONE;

    for (int i = 0; i < 5; i++)
    {
        m_pPlayers[i] = NULL;
    }

    for (int i = 0; i < 4; i++)
    {
        m_pAIOrderedFielders[i] = NULL;
        m_pBallInterceptOrderedFielders[i] = NULL;
    }

    m_pNet = new (nlMalloc(sizeof(cNet), 8, false)) cNet(side);
    m_pFormationManager = new (nlMalloc(sizeof(FormationManager), 8, false)) FormationManager(this);
}

/**
 * Offset/Address/Size: 0x1C8C | 0x80066038 | size: 0x64
 */
cTeam::~cTeam()
{
    delete m_pNet;
    delete m_pFormationManager;
}

/**
 * Offset/Address/Size: 0x1C1C | 0x80065FC8 | size: 0x70
 */
void cTeam::SetDifficulty(eDifficultyID difficulty)
{
    if (difficulty < 0)
    {
        difficulty = DIFF_MEDIUM;
    }

    SkillTweaks::GetSkillTweaks(m_nSide)->Init(difficulty, false);
    SkillTweaks::GetSkillTweaks(m_nSide)->HookupTweakeables(m_nSide); // actually a dead function
}

/**
 * Offset/Address/Size: 0x1C00 | 0x80065FAC | size: 0x1C
 */
void cTeam::ClearAllPowerUps()
{
    m_ePowerupList[0].eType = POWER_UP_NONE;
    m_ePowerupList[0].nnumOfPowerups = 0;
    m_ePowerupList[1].eType = POWER_UP_NONE;
    m_ePowerupList[1].nnumOfPowerups = 0;
}

/**
 * Offset/Address/Size: 0x1B94 | 0x80065F40 | size: 0x6C
 */
void cTeam::ClearCurrentPowerUp()
{
    if (m_ePowerupList[0].eType == POWER_UP_NONE)
    {
        m_ePowerupList[1].eType = POWER_UP_NONE;
        m_ePowerupList[1].nnumOfPowerups = 0;
        mbHasToggledPowerup = false;
        return;
    }

    m_ePowerupList[0].eType = POWER_UP_NONE;
    m_ePowerupList[0].nnumOfPowerups = 0;

    if (m_ePowerupList[1].eType != POWER_UP_NONE)
    {
        if (!mbHasToggledPowerup)
        {
            m_ePowerupList[0].eType = m_ePowerupList[1].eType;
            m_ePowerupList[0].nnumOfPowerups = m_ePowerupList[1].nnumOfPowerups;
            m_ePowerupList[1].eType = POWER_UP_NONE;
            m_ePowerupList[1].nnumOfPowerups = 0;
        }
    }
    else
    {
        mbHasToggledPowerup = false;
    }
}

/**
 * Offset/Address/Size: 0x1A4C | 0x80065DF8 | size: 0x148
 */
#pragma push
#pragma opt_propagation off
void cTeam::TogglePowerup(bool bToggle)
{
    static bool bAudioToggleSwitch = true;

    PowerUpTeamType tmp = m_ePowerupList[1];
    m_ePowerupList[1] = m_ePowerupList[0];
    m_ePowerupList[0] = tmp;

    if (!bToggle)
    {
        unsigned int alwaysZero = 0;
        if (alwaysZero == 0)
        {
            if ((tmp.eType != POWER_UP_NONE) || (m_ePowerupList[1].eType != POWER_UP_NONE))
            {
                if (bAudioToggleSwitch)
                {
                    Audio::gWorldSFX.Play(Audio::WORLDSFX_FILTER_START, 100.0f, -1.0f, true, 100.0f);
                }
                else
                {
                    Audio::gWorldSFX.Play(Audio::WORLDSFX_FILTER_END, 100.0f, -1.0f, true, 100.0f);
                }
                bAudioToggleSwitch = !bAudioToggleSwitch;
            }
        }
    }

    OverlayManager* pOverlayManager = OverlayManager::s_pInstance;
    HUDOverlay* pHUDOverlay = (HUDOverlay*)pOverlayManager->GetScene(OVERLAY_HUD);
    pHUDOverlay->SwapPowerUps(m_nSide);

    if (m_ePowerupList[1].eType != POWER_UP_NONE || m_ePowerupList[0].eType == POWER_UP_NONE)
    {
        mbHasToggledPowerup = true;
    }
    else
    {
        mbHasToggledPowerup = false;
    }
}
#pragma pop

/**
 * Offset/Address/Size: 0x1910 | 0x80065CBC | size: 0x13C
 */
bool cTeam::IncrementPowerupMeter(float fIncrement)
{
    int nResult = -1;
    mfPowerupMeter += fIncrement;
    if (mfPowerupMeter >= 1.0f)
    {
        mfPowerupMeter -= 1.0f;
        nResult = PowerupBase::AwardPowerup(this);
    }
    if (nResult < 0)
    {
        bool bHasEmptySlot = false;
        for (int i = 0; i < 2; i++)
        {
            if (GetPowerUpByIndex(i).eType == POWER_UP_NONE)
            {
                bHasEmptySlot = true;
            }
        }
        if (!bHasEmptySlot)
        {
            return false;
        }
    }
    return true;
}

/**
 * Offset/Address/Size: 0x18D0 | 0x80065C7C | size: 0x40
 */
PowerUpTeamType cTeam::GetCurrentPowerUp() const
{
    if (m_ePowerupList[0].eType == POWER_UP_NONE)
    {
        return m_ePowerupList[1];
    }
    return m_ePowerupList[0];
}

/**
 * Offset/Address/Size: 0x18BC | 0x80065C68 | size: 0x14
 */
bool cTeam::IsCurrentNoPowerup() const
{
    return m_ePowerupList[0].eType == POWER_UP_NONE;
}

/**
 * Offset/Address/Size: 0x18A8 | 0x80065C54 | size: 0x14
 */
bool cTeam::IsCurrentMushroom() const
{
    return m_ePowerupList[0].eType == POWER_UP_MUSHROOM;
}

/**
 * Offset/Address/Size: 0x1894 | 0x80065C40 | size: 0x14
 */
bool cTeam::IsCurrentStar() const
{
    return m_ePowerupList[0].eType == POWER_UP_STAR;
}

/**
 * Offset/Address/Size: 0x183C | 0x80065BE8 | size: 0x58
 */
PowerUpTeamType cTeam::GetPowerUpByIndex(int index) const
{
    PowerUpTeamType result;
    if (index >= 0)
    {
        return m_ePowerupList[index];
    }
    result.eType = POWER_UP_NONE;
    result.nnumOfPowerups = 0;
    return result;
}

/**
 * Offset/Address/Size: 0x1824 | 0x80065BD0 | size: 0x18
 */
void cTeam::SetIsPowerUpNew(int index, bool bIsNew)
{
    if (index >= 0)
    {
        m_ePowerupList[index].bIsNew = bIsNew;
    }
}

#pragma push
#pragma optimization_level 0
static inline unsigned int FakeRuntimeZero()
{
    register unsigned int z = 0;
    return z;
}
#pragma pop

/**
 * Offset/Address/Size: 0x17D0 | 0x80065B7C | size: 0x54
 * TODO: 97.6% match - r0/r6 register swap (MWCC register allocator quirk)
 */
#pragma push
#pragma global_optimizer off
#pragma opt_propagation off
void cTeam::SetCurrentPowerUp(ePowerUpType eNewPowerUpType, int nnumOfPowerups)
{
    int a;
    unsigned char bGivenNewPowerup = 0;
    a = 1;

    if (m_ePowerupList[0].eType == POWER_UP_NONE && FakeRuntimeZero() == 0)
    {
        m_ePowerupList[0].eType = eNewPowerUpType;
        bGivenNewPowerup = 1;
        m_ePowerupList[0].nnumOfPowerups = nnumOfPowerups;
        m_ePowerupList[0].bIsNew = a;
    }

    if (m_ePowerupList[1].eType == POWER_UP_NONE && !bGivenNewPowerup)
    {
        m_ePowerupList[1].eType = eNewPowerUpType;
        m_ePowerupList[1].nnumOfPowerups = nnumOfPowerups;
        m_ePowerupList[1].bIsNew = a;
    }
}
#pragma pop

/**
 * Offset/Address/Size: 0x17B0 | 0x80065B5C | size: 0x20
 */
void cTeam::SetPlayer(cPlayer* pPlayer, int index)
{
    m_pPlayers[index] = (cFielder*)pPlayer;
    if (index < 4)
    {
        m_pAIOrderedFielders[index] = (cFielder*)pPlayer;
        m_pBallInterceptOrderedFielders[index] = (cFielder*)pPlayer;
    }
}

/**
 * Offset/Address/Size: 0x17A8 | 0x80065B54 | size: 0x8
 */
void cTeam::SetGoalie(Goalie* goalie)
{
    m_pPlayers[4] = (cFielder*)goalie;
}

/**
 * Offset/Address/Size: 0x17A0 | 0x80065B4C | size: 0x8
 */
Goalie* cTeam::GetGoalie()
{
    return (Goalie*)m_pPlayers[4];
}

/**
 * Offset/Address/Size: 0x1734 | 0x80065AE0 | size: 0x6C
 */
cPlayer* cTeam::GetControlledPlayer(cGlobalPad* pPad)
{
    cPlayer* res = nullptr;
    for (int i = 0; i < 5; i++)
    {
        if (m_pPlayers[i]->GetGlobalPad() == pPad)
        {
            res = (cPlayer*)m_pPlayers[i];
            break;
        }
    }
    return res;
}

/**
 * Offset/Address/Size: 0x16B4 | 0x80065A60 | size: 0x80
 */
int cTeam::GetNumAssignedControllers()
{
    int mySide, count;
    unsigned short i;
    short playingSide;

    count = 0;
    for (i = 0; i < 4; i++)
    {
        mySide = m_nSide;
        playingSide = GameInfoManager::Instance()->GetPlayingSide(i);
        if (playingSide == mySide)
        {
            count++;
        }
    }
    return count;
}

/**
 * Offset/Address/Size: 0x16A4 | 0x80065A50 | size: 0x10
 */
cFielder* cTeam::GetFielder(int index)
{
    return m_pPlayers[index];
}

/**
 * Offset/Address/Size: 0x1694 | 0x80065A40 | size: 0x10
 */
cPlayer* cTeam::GetPlayer(int index)
{
    return (cPlayer*)m_pPlayers[index];
}

/**
 * Offset/Address/Size: 0x167C | 0x80065A28 | size: 0x18
 */
cTeam* cTeam::GetOtherTeam()
{
    return g_pTeams[m_nSide == HOME ? AWAY : HOME];
}

/**
 * Offset/Address/Size: 0x1660 | 0x80065A0C | size: 0x1C
 */
cNet* cTeam::GetOtherNet()
{
    return g_pTeams[m_nSide == HOME ? AWAY : HOME]->m_pNet;
}

/**
 * Offset/Address/Size: 0x15F8 | 0x800659A4 | size: 0x68
 */
void cTeam::PreUpdate(float dt)
{
    for (int i = 0; i < 5; i++)
    {
        m_pPlayers[i]->PreUpdate(dt);
    }
}

static inline void UpdateBallInterceptTime(cTeam* pTeam)
{
    for (int i = 0; i < 4; i++)
    {
        cFielder* pPlayer = pTeam->m_pPlayers[i];
        float speed = pPlayer->m_fActualSpeed;
        float runSpeed = pPlayer->m_pTweaks->fRunningSpeed;
        speed = (speed >= runSpeed) ? speed : runSpeed;

        nlVector3 v3LandingSpot;
        float landingTime = g_pBall->PredictLandingSpotAndTime(v3LandingSpot);
        float interceptTime;

        if (landingTime > 0.0f)
        {
            float y = v3LandingSpot.f.y - pPlayer->m_v3Position.f.y;
            float x = v3LandingSpot.f.x - pPlayer->m_v3Position.f.x;
            float y2 = y * y;
            float sum = (x * x) + y2;
            interceptTime = nlSqrt(sum, true) / speed;
        }
        else
        {
            cBall* pBall = g_pBall;
            int nFoundSolutions = 0;
            float fSolutions[2];

            interceptTime = 99999.0f;
            nlVector3* pBallPosition = &pBall->m_v3Position;
            nlVector3* pAIVelocity = pBall->GetAIVelocity();
            CalcInterceptXY(pPlayer->m_v3Position, speed, 0.0f, *pBallPosition, *pAIVelocity, nFoundSolutions, fSolutions);

            if (nFoundSolutions != 0)
            {
                if (nFoundSolutions == 2)
                {
                    float solution1 = fSolutions[1];
                    interceptTime = fSolutions[0];
                    if (interceptTime > solution1)
                    {
                        interceptTime = solution1;
                    }
                }
                else
                {
                    interceptTime = fSolutions[0];
                }
            }
        }

        pTeam->mfBallInterceptTimes[i] = interceptTime;
    }
}

/**
 * Offset/Address/Size: 0x132C | 0x800656D8 | size: 0x2CC
 * TODO: 96.35% match - remaining diffs are -inline deferred artifacts:
 *   r3/r4 swap, fcmpu order, stalling 1.0f CSE, loop counter r30/r28
 */
void cTeam::Update(float dt)
{
    extern cTeam* g_pCurrentlyUpdatingTeam;
    extern void UpdateTeamAI__5cTeamFf(cTeam*, float);

    g_pCurrentlyUpdatingTeam = this;

    bool gameplayOrOvertime = false;
    if (g_pGame->m_eGameState == GS_GAMEPLAY || g_pGame->m_eGameState == GS_OVERTIME)
    {
        gameplayOrOvertime = true;
    }

    if (gameplayOrOvertime)
    {
        mfPowerupTimer -= dt;
        if (mfPowerupTimer < 0.0f)
        {
            mfPowerupTimer = 1.0f;
            if (nlSingleton<GameInfoManager>::s_pInstance->IsInfinitePowerupsOn())
            {
                PowerupBase::AwardPowerup(this);
            }
        }

        mtTeamStyleTimer.Countdown(dt, 0.0f);
        mtMarkTimer.Countdown(dt, 0.0f);
        mtRoleTimer.Countdown(dt, 0.0f);
        mtBallInterceptTimer.Countdown(dt, 0.0f);

        float offensive = Offensive(this);
        if (offensive != 0.0f && InOffensiveZone(g_pBall->m_v3Position, (eTeamSide)m_nSide) < 1.0f)
        {
            float stalling = Stalling(this);
            if (stalling < 1.0f)
            {
                mtDefensiveZoneTimer.Countup(dt, 1.0f);
            }
        }
        else
        {
            mtDefensiveZoneTimer.Countdown(0.5f * dt, 0.0f);
        }
    }

    UpdateBallInterceptTime(this);

    qsort(m_pBallInterceptOrderedFielders, 4, 4, BestAbleToInterceptBall);
    mtBallInterceptTimer.SetSeconds(0.0f);

    UpdateTeamAI__5cTeamFf(this, dt);

    for (int i = 0; i < 4; i++)
    {
        m_pAIOrderedFielders[i]->UpdatePlay(dt);
    }
}

/**
 * Offset/Address/Size: 0x1100 | 0x800654AC | size: 0x22C
 */
void cTeam::UpdateControllers()
{
    cAIPad* pAIPads = AIPadManager::mAIPads;

    for (int i = 0; i < 4; i++)
    {
        if (m_nSide != (s16)nlSingleton<GameInfoManager>::s_pInstance->GetPlayingSide((u16)i))
        {
            for (int j = 0; j < 5; j++)
            {
                if (m_pPlayers[j]->m_pController == &pAIPads[i])
                {
                    m_pPlayers[j]->SetAIPad(NULL);
                    break;
                }
            }
        }
        else
        {
            int playerIdx;
            for (playerIdx = 0; playerIdx < 5; playerIdx++)
            {
                if (m_pPlayers[playerIdx]->m_pController == &pAIPads[i])
                {
                    break;
                }
            }

            if (playerIdx == 5)
            {
                for (int j = 0; j < 5; j++)
                {
                    if (m_pPlayers[j]->m_pController == NULL)
                    {
                        m_pPlayers[j]->SetAIPad(&pAIPads[i]);
                        break;
                    }
                }
            }
        }
    }

    cPlayer* pOwner = g_pBall->m_pOwner;
    if (pOwner != NULL)
    {
        if (pOwner->m_eClassType == GOALIE)
        {
            if (g_pGame->IsGameplayOrOvertime() && !((Goalie*)pOwner)->mbNoUserControl)
            {
            }
            else
            {
                return;
            }
        }

        if (pOwner->m_pTeam != this)
        {
            return;
        }
        if (pOwner->m_pController != NULL)
        {
            return;
        }

        for (int j = 0; j < 5; j++)
        {
            if (m_pPlayers[j]->m_pController != NULL)
            {
                cAIPad* pPad = m_pPlayers[j]->m_pController;
                m_pPlayers[j]->SetAIPad(NULL);
                pOwner->SetAIPad(pPad);
                break;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xC98 | 0x80065044 | size: 0x468
 */
void cTeam::ResetCharacters()
{
}

/**
 * Offset/Address/Size: 0xAF4 | 0x80064EA0 | size: 0x1A4
 */
void cTeam::StopGameplayEffectsAndSounds()
{
    Audio::ActivateFilterOnAllCurrentSFX(false);
    Audio::SetPitchBendOnAllDialogueSFX(0x2000);

    s32 side = m_nSide;
    s32 playerIndex = 0;
    do
    {
        g_pTeams[side]->m_pPlayers[playerIndex]->StopPlayingAllTrackedSFX();
        playerIndex++;
    } while (playerIndex < 5);

    Audio::gWorldSFX.Stop(Audio::REPLAYSFX_CAMERA_ZOOM_OUT, cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xBD, cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xCE, cGameSFX::SFX_STOP_ALL);
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xCC, cGameSFX::SFX_STOP_FIRST);
    Audio::gStadGenSFX.Stop((Audio::eWorldSFX)0xBA, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x88, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x78, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x72, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x6C, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x66, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x5E, cGameSFX::SFX_STOP_FIRST);
    Audio::gPowerupSFX.Stop((Audio::eWorldSFX)0x8E, cGameSFX::SFX_STOP_FIRST);

    BasicStadium::GetCurrentStadium()->mpNPCManager->mpBowser->m_pCharacterSFX->StopPlayingAllTrackedSFX();

    s32 emissionIndex = 0;
    do
    {
        EmissionManager::Destroy((unsigned long)emissionIndex, nullptr);
        emissionIndex++;
    } while (emissionIndex < 10);
}

/**
 * Offset/Address/Size: 0xAD0 | 0x80064E7C | size: 0x24
 */
bool cTeam::CalculateFormationPosition(nlVector3& pos, cFielder* pFielder, bool bParam, float fParam)
{
    return m_pFormationManager->CalculateFielderPosition(pos, pFielder, bParam, fParam);
}

/**
 * Offset/Address/Size: 0xA68 | 0x80064E14 | size: 0x68
 */
int BestAbleToInterceptBall(const void* a, const void* b)
{
    cPlayer* playerA = *(cPlayer**)a;
    cPlayer* playerB = *(cPlayer**)b;

    float scoreA = AbleToInterceptBall(playerA);
    float scoreB = AbleToInterceptBall(playerB);

    if (scoreA == scoreB)
    {
        return 0;
    }

    if (scoreA > scoreB)
    {
        return -1;
    }

    return 1;
}

static inline void AbortAllFielderPlays(cTeam* pTeam)
{
    for (int i = 0; i < 4; i++)
    {
        pTeam->m_pAIOrderedFielders[i]->AbortPlay();
    }
}

/**
 * Offset/Address/Size: 0x6F0 | 0x80064A9C | size: 0x378
 * TODO: 99.75% match - pBallOwner r29/r28 register swap and fcmpu operand order
 */
void cTeam::UpdateTeamAI(float fDeltaT)
{
    if (mtTeamStyleTimer.m_uPackedTime == 0)
    {
        meCurrentTeamStyle = TEAM_STYLE_AGGRESSIVE;
        mtTeamStyleTimer.SetSeconds(1.0f);
        AbortAllFielderPlays(this);
    }

    m_pFormationManager->Update(fDeltaT);

    eSituation eLastSituation = mpCurrentSituation;
    cPlayer* pBallOwner = g_pBall->m_pOwner;

    if (pBallOwner == NULL)
    {
        pBallOwner = g_pBall->m_pPassTarget;
        if ((pBallOwner != NULL) && (pBallOwner->m_eClassType == FIELDER))
        {
            if (ReceivingPass((cFielder*)pBallOwner) != 0.0f)
            {
            }
            else
            {
                nlPrintf("cTeam::AssignSituation - caught bad pass case, with no proper receiver.\n");
                g_pBall->ClearPassTarget();
                pBallOwner = NULL;
            }
        }
    }

    if (pBallOwner != NULL)
    {
        if (pBallOwner->m_pTeam == this)
        {
            if (mpCurrentSituation != SITUATION_OFFENSE)
            {
                mpCurrentSituation = SITUATION_OFFENSE;
                meCurrentTeamStyle = TEAM_STYLE_AGGRESSIVE;
                mtTeamStyleTimer.SetSeconds(1.0f);
                AbortAllFielderPlays(this);
            }
        }
        else if (mpCurrentSituation != SITUATION_DEFENSE)
        {
            mpCurrentSituation = SITUATION_DEFENSE;
            meCurrentTeamStyle = TEAM_STYLE_AGGRESSIVE;
            mtTeamStyleTimer.SetSeconds(1.0f);
            AbortAllFielderPlays(this);
        }
    }
    else if (mpCurrentSituation != SITUATION_LOOSE)
    {
        mpCurrentSituation = SITUATION_LOOSE;
        meCurrentTeamStyle = TEAM_STYLE_AGGRESSIVE;
        mtTeamStyleTimer.SetSeconds(1.0f);
        AbortAllFielderPlays(this);
    }

    bool bSituationChanged = (eLastSituation != mpCurrentSituation);

    if ((mtRoleTimer.m_uPackedTime == 0) || bSituationChanged)
    {
        switch (mpCurrentSituation)
        {
        case SITUATION_OFFENSE:
            m_pAIOrderedFielders[0] = m_pPlayers[0];
            m_pAIOrderedFielders[1] = m_pPlayers[1];
            m_pAIOrderedFielders[2] = m_pPlayers[2];
            m_pAIOrderedFielders[3] = m_pPlayers[3];
            qsort(m_pAIOrderedFielders, 4, 4, MostOffensiveThreat);
            m_pAIOrderedFielders[0]->m_eRole = ROLE_STRIKER;
            m_pAIOrderedFielders[1]->m_eRole = ROLE_WINGER;
            m_pAIOrderedFielders[2]->m_eRole = ROLE_MIDFIELD;
            m_pAIOrderedFielders[3]->m_eRole = ROLE_DEFENCE;
            break;

        case SITUATION_DEFENSE:
            m_pAIOrderedFielders[0] = m_pPlayers[0];
            m_pAIOrderedFielders[1] = m_pPlayers[1];
            m_pAIOrderedFielders[2] = m_pPlayers[2];
            m_pAIOrderedFielders[3] = m_pPlayers[3];
            qsort(m_pAIOrderedFielders, 4, 4, MostDefensivePlayer);
            m_pAIOrderedFielders[0]->m_eRole = ROLE_STRIKER;
            m_pAIOrderedFielders[1]->m_eRole = ROLE_WINGER;
            m_pAIOrderedFielders[2]->m_eRole = ROLE_MIDFIELD;
            m_pAIOrderedFielders[3]->m_eRole = ROLE_DEFENCE;
            break;

        case SITUATION_LOOSE:
            m_pAIOrderedFielders[0] = m_pBallInterceptOrderedFielders[0];
            m_pAIOrderedFielders[1] = m_pBallInterceptOrderedFielders[1];
            m_pAIOrderedFielders[2] = m_pBallInterceptOrderedFielders[2];
            m_pAIOrderedFielders[3] = m_pBallInterceptOrderedFielders[3];
            m_pAIOrderedFielders[0]->m_eRole = ROLE_STRIKER;
            m_pAIOrderedFielders[1]->m_eRole = ROLE_WINGER;
            m_pAIOrderedFielders[2]->m_eRole = ROLE_MIDFIELD;
            m_pAIOrderedFielders[3]->m_eRole = ROLE_DEFENCE;
            break;
        }

        mtRoleTimer.SetSeconds(0.33f);
    }

    AssignMarks(bSituationChanged);
}

/**
 * Offset/Address/Size: 0x598 | 0x80064944 | size: 0x158
 */
int MostOffensiveThreat(const void* a, const void* b)
{
    cPlayer* playerA = *(cPlayer**)a;
    cPlayer* playerB = *(cPlayer**)b;

    const nlVector3& offNetLocA = playerA->GetAIOffNetLocation(NULL);

    float dxA = offNetLocA.f.x - playerA->m_v3Position.f.x;
    float dyA = offNetLocA.f.y - playerA->m_v3Position.f.y;
    float distA = nlSqrt(dxA * dxA + dyA * dyA, true);

    if (playerA->IsCaptain())
    {
        distA *= 0.96f;
    }

    float score_test = 0.0f;
    float strategicScoreA = StrategicBallOwner((cFielder*)playerA);
    if (strategicScoreA != score_test)
    {
        distA *= 0.92f;
    }

    const nlVector3& offNetLocB = playerB->GetAIOffNetLocation(NULL);

    float dxB = offNetLocB.f.x - playerB->m_v3Position.f.x;
    float dyB = offNetLocB.f.y - playerB->m_v3Position.f.y;
    float distB = nlSqrt(dxB * dxB + dyB * dyB, true);

    if (playerB->IsCaptain())
    {
        distB *= 0.96f;
    }

    float strategicScoreB = StrategicBallOwner((cFielder*)playerB);
    if (strategicScoreB != score_test)
    {
        distB *= 0.92f;
    }

    if (distA == distB)
    {
        return 0;
    }

    if (distA < distB)
    {
        return -1;
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x4D0 | 0x8006487C | size: 0xC8
 */
int MostDefensivePlayer(const void* a, const void* b)
{
    cPlayer* playerA = *(cPlayer**)a;
    cPlayer* playerB = *(cPlayer**)b;

    const nlVector3& netLocA = playerA->GetAIDefNetLocation(NULL);
    float dxA = netLocA.f.x - playerA->m_v3Position.f.x;
    float dyA = netLocA.f.y - playerA->m_v3Position.f.y;
    float distSqA = dxA * dxA + dyA * dyA;

    const nlVector3& netLocB = playerB->GetAIDefNetLocation(NULL);
    float dxB = netLocB.f.x - playerB->m_v3Position.f.x;
    float dyB = netLocB.f.y - playerB->m_v3Position.f.y;
    float distSqB = dxB * dxB + dyB * dyB;

    if (distSqA == distSqB)
    {
        return 0;
    }

    if (distSqA > distSqB)
    {
        return -1;
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x120 | 0x800644CC | size: 0x3B0
 */
void cTeam::AssignMarks(bool)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x118 | 0x800644C4 | size: 0x8
 */
cFielder* cTeam::GetCaptain()
{
    return m_pPlayers[0];
}

/**
 * Offset/Address/Size: 0x110 | 0x800644BC | size: 0x8
 */
cFielder* cTeam::GetStriker() const
{
    // return m_pStriker;
    return m_pAIOrderedFielders[0];
}

/**
 * Offset/Address/Size: 0x108 | 0x800644B4 | size: 0x8
 */
cFielder* cTeam::GetMidfield() const
{
    return m_pAIOrderedFielders[2];
}

/**
 * Offset/Address/Size: 0x100 | 0x800644AC | size: 0x8
 */
cFielder* cTeam::GetDefence() const
{
    return m_pAIOrderedFielders[3];
}

/**
 * Offset/Address/Size: 0x80 | 0x8006442C | size: 0x80
 */
cFielder* cTeam::GetFrontMostFielder()
{
    cFielder* p;
    cFielder* pRearFrontFielder = NULL;

    for (int i_fielder = 0; i_fielder < 4; i_fielder++)
    {
        p = m_pPlayers[i_fielder];
        if ((pRearFrontFielder == NULL) || (p->m_v3AIPosition.f.x > pRearFrontFielder->m_v3AIPosition.f.x))
        {
            pRearFrontFielder = p;
        }
    }

    return pRearFrontFielder;
}

/**
 * Offset/Address/Size: 0x0 | 0x800643AC | size: 0x80
 */
cFielder* cTeam::GetRearMostFielder()
{
    cFielder* p;
    cFielder* pRearMostFielder = NULL;

    for (int i = 0; i < 4; i++)
    {
        p = m_pPlayers[i];
        if ((pRearMostFielder == NULL) || (p->m_v3AIPosition.f.x < pRearMostFielder->m_v3AIPosition.f.x))
        {
            pRearMostFielder = p;
        }
    }

    return pRearMostFielder;
}
