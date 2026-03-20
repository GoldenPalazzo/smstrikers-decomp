#include "Game/Formation.h"

#include "Game/MathHelpers.h"
#include "Game/AI/AiUtil.h"
#include "Game/AI/Fielder.h"
#include "Game/AI/FuzzyVariant.h"
#include "Game/AI/Scripts/FormationScript.h"
#include "Game/Game.h"
#include "Game/Team.h"
#include "Game/FormationDefines.h"
#include "Game/Sys/debug.h"
#include "NL/nlMath.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"

extern cTeam* g_pTeams[];

static nlVector3 v3Zero = { 0.0f, 0.0f, 0.0f };
static const float g_fFormationCaptainPosBonus[4] = { 0.55f, 0.5f, 0.0f, 0.0f };

FormationSet* FormationManager::m_FormationSetArray = nullptr;
int FormationManager::m_NumFormationSets = 0;

static inline void ClearFormationEvals(FormationEval** pp);

/**
 * Offset/Address/Size: 0x5C | 0x8003ADB4 | size: 0x5C
 */
FormationDefensive::~FormationDefensive()
{
}

/**
 * Offset/Address/Size: 0x0 | 0x8003AD58 | size: 0x5C
 */
FormationOffensive::~FormationOffensive()
{
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8003ACEC | size: 0x6C
//  */
// FormationSet::~FormationSet()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x8003ACA4 | size: 0x48
 */
FormationEval::~FormationEval()
{
}

/**
 * Offset/Address/Size: 0x29F4 | 0x8003AC44 | size: 0x60
 */
FormationManager::FormationManager(cTeam* pTeam)
{
    m_pTeam = pTeam;
    m_tSelectFormationsTimer.SetSeconds(0.0f);
    m_pFormations[0] = nullptr;
    m_pFormations[1] = nullptr;
    m_pFormations[2] = nullptr;
    m_CachedPositions[0].bCacheIsValid = false;
    m_CachedPositions[1].bCacheIsValid = false;
    m_CachedPositions[2].bCacheIsValid = false;
    m_CachedPositions[3].bCacheIsValid = false;
    m_tSelectFormationsTimer.m_uPackedTime = 0;
}

/**
 * Offset/Address/Size: 0x2974 | 0x8003ABC4 | size: 0x80
 */
FormationManager::~FormationManager()
{
    ClearFormationEvals((FormationEval**)this);
}

/**
 * Offset/Address/Size: 0x281C | 0x8003AA6C | size: 0x158
 * TODO: 96.3% match - MWCC register allocation: r26/r28/r29 three-way swap
 * for g_pTeams pointer, outer loop i, inner loop j. All instructions match.
 * Likely -inline deferred context issue; repo builds at 97.9%.
 */
void FormationManager::LoadFormationSets()
{
    int numFormationSets = 0;
    FormationSet* pFormationSets = FormationSet::LoadFormationSets("FormationSets.ini", numFormationSets);

    if (numFormationSets == 0)
    {
        if (m_FormationSetArray != nullptr)
        {
            tDebugPrintManager::Print(DC_AI, "Reload of FormationSets.ini failed, but we\'ve got some loaded already.");
        }
        return;
    }

    if (g_pGame != nullptr)
    {
        if (g_pTeams[0] != nullptr && g_pTeams[1] != nullptr)
        {
            for (s32 i = 0; i < 2; i++)
            {
                FormationEval** pp = (FormationEval**)g_pTeams[i]->m_pFormationManager;
                for (s32 j = 0; j < 3; j++)
                {
                    FormationEval* pEval = pp[1];
                    delete pEval;
                    *++pp = nullptr;
                }
            }
        }
    }

    delete[] m_FormationSetArray;
    m_FormationSetArray = pFormationSets;
    m_NumFormationSets = numFormationSets;

    if (g_pGame != nullptr)
    {
        if (g_pTeams[0] != nullptr && g_pTeams[1] != nullptr)
        {
            for (s32 i = 0; i < 2; i++)
            {
                g_pTeams[i]->m_pFormationManager->ChooseNewFormations();
            }
        }
    }
}

static inline void ClearFormationEvals(FormationEval** pp)
{
    for (s32 j = 0; j < 3; j++)
    {
        FormationEval* pEval = pp[1];
        delete pEval;
        *++pp = nullptr;
    }
}

/**
 * Offset/Address/Size: 0x275C | 0x8003A9AC | size: 0xC0
 */
void FormationManager::UnloadFormationSets()
{
    s32 i;

    if (g_pGame != nullptr)
    {
        if (g_pTeams[0] != nullptr && g_pTeams[1] != nullptr)
        {
            for (i = 0; i < 2; i++)
            {
                ClearFormationEvals((FormationEval**)g_pTeams[i]->m_pFormationManager);
            }
        }
    }

    delete[] m_FormationSetArray;
    m_FormationSetArray = nullptr;
    m_NumFormationSets = 0;
}

/**
 * Offset/Address/Size: 0x26E4 | 0x8003A934 | size: 0x78
 */
FormationSpec* FormationManager::GetFormationSpec(eFormation formation)
{
    FORCE_DONT_INLINE;
    FormationSpec* result = nullptr;
    int offset = 0;
    eFormation id = (eFormation)(int)this;
    int i = 0;

    while (i < m_NumFormationSets)
    {
        result = m_FormationSetArray[i].GetFormationSpecFromID(id);
        if (result != nullptr)
        {
            break;
        }
        offset += 0x10;
        i++;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x2558 | 0x8003A7A8 | size: 0x18C
 */
void FormationManager::Update(float dt)
{
    if (!g_pGame->IsGameplayOrOvertime())
    {
        return;
    }

    m_tSelectFormationsTimer.Countdown(dt, 0.0f);

    if (m_tSelectFormationsTimer.m_uPackedTime == 0)
    {
        ChooseNewFormations();
        float randomTime = nlRandomf(0.2f, &nlDefaultSeed);
        m_tSelectFormationsTimer.SetSeconds(10.0f + randomTime);
    }

    m_v2AIFielderCenter.f.x = 0.0f;
    m_v2AIFielderCenter.f.y = 0.0f;

    for (int i = 0; i < 4; i++)
    {
        cFielder* pFielder = m_pTeam->GetFielder(i);
        float newX = m_v2AIFielderCenter.f.x + pFielder->m_v3Position.f.x;
        float newY = m_v2AIFielderCenter.f.y + pFielder->m_v3Position.f.y;
        m_v2AIFielderCenter.f.x = newX;
        m_v2AIFielderCenter.f.y = newY;
    }

    float cx = m_v2AIFielderCenter.f.x * 0.25f;
    float cy = m_v2AIFielderCenter.f.y * 0.25f;
    m_v2AIFielderCenter.f.x = cx;
    m_v2AIFielderCenter.f.y = cy;

    if (m_pTeam->m_nSide == AWAY)
    {
        m_v2AIFielderCenter.f.x = -m_v2AIFielderCenter.f.x;
        m_v2AIFielderCenter.f.y = -m_v2AIFielderCenter.f.y;
    }

    for (int i = 0; i < 3; i++)
    {
        if (m_pFormations[i] != nullptr)
        {
            m_pFormations[i]->Update(dt);
        }
    }

    m_CachedPositions[0].bCacheIsValid = false;
    m_CachedPositions[1].bCacheIsValid = false;
    m_CachedPositions[2].bCacheIsValid = false;
    m_CachedPositions[3].bCacheIsValid = false;
}

/**
 * Offset/Address/Size: 0x24B8 | 0x8003A708 | size: 0xA0
 */
void FormationManager::ChooseNewFormations()
{
    FORCE_DONT_INLINE;
    s32 defensiveFormation;
    s32 offensiveFormation;
    s32 ballFormationSet;

    defensiveFormation = Fuzzy::GetBestDefensiveFormation(m_pTeam).mData.i;
    offensiveFormation = Fuzzy::GetBestOffensiveFormation(m_pTeam).mData.i;
    ballFormationSet = Fuzzy::GetBestBallFormationSet(m_pTeam).mData.i;

    SetNewFormationEval(FTYPE_DEFENSIVE, (eFormation)defensiveFormation);
    SetNewFormationEval(FTYPE_OFFENSIVE, (eFormation)offensiveFormation);
    SetNewFormationEval(FTYPE_BALLPOSITION, (eFormationSet)ballFormationSet);
}

/**
 * Offset/Address/Size: 0x23DC | 0x8003A62C | size: 0xDC
 */
void FormationManager::SetNewFormationEval(eFormationType formType, eFormation formation)
{
    FORCE_DONT_INLINE;
    FormationEval* pFormation;

    if (formation == FORMATION_NONE)
    {
        pFormation = m_pFormations[formType];
        delete pFormation;
        m_pFormations[formType] = nullptr;
    }
    else
    {
        pFormation = m_pFormations[formType];
        if (pFormation != nullptr)
        {
            if ((s32)formation == pFormation->m_pFormationSpec->m_ID)
            {
                return;
            }
        }
        delete pFormation;
        m_pFormations[formType] = FormationEval::Create(this, formType, FSET_NONE, formation);
    }
}

/**
 * Offset/Address/Size: 0x2300 | 0x8003A550 | size: 0xDC
 */
void FormationManager::SetNewFormationEval(eFormationType formType, eFormationSet formSet)
{
    FORCE_DONT_INLINE;
    FormationEval* pFormation;

    if (formSet == FSET_NONE)
    {
        pFormation = m_pFormations[formType];
        delete pFormation;
        m_pFormations[formType] = nullptr;
    }
    else
    {
        pFormation = m_pFormations[formType];
        if (pFormation != nullptr)
        {
            FormationSet* pFormationSet = *(FormationSet**)((u8*)pFormation + 0x28);
            if ((s32)formSet == pFormationSet->m_ID)
            {
                return;
            }
        }
        delete pFormation;
        m_pFormations[formType] = FormationEval::Create(this, formType, formSet, FORMATION_NONE);
    }
}

/**
 * Offset/Address/Size: 0x202C | 0x8003A27C | size: 0x2D4
 * TODO: 92.1% match - register allocation off by 1 (r19-r28 vs r20-r29)
 */
bool FormationManager::CalculateFielderPosition(nlVector3& v3DestPosition, cFielder* pFielder, bool bInPosition, float fBallPosFormationWeight)
{
    int id = pFielder->m_ID;
    int offset = id << 4;
    CachedPosition* cached = (CachedPosition*)((u8*)&m_CachedPositions[0] + offset);

    // Check if we have a cached position
    if (cached->bCacheIsValid)
    {
        v3DestPosition = cached->vPosition;
        return cached->bInPosition;
    }

    // Initialize local variables
    float fWeights[3];
    nlVector3 v3FormationPosition[2][3];
    nlVector3 v3FutureDesiredPosition = v3Zero;
    float fFielderInPosition = 0.0f;

    // First loop: get weights and positions from each formation
    for (int i = 0; i < 3; i++)
    {
        FormationEval* pFormation = m_pFormations[i];
        if (pFormation != nullptr && pFormation->m_pFormationSpec != nullptr)
        {
            fWeights[i] = pFormation->GetWeight();
            pFormation->CalculateDesiredLocation(v3FormationPosition[1][i], pFielder, true);
            pFormation->CalculateDesiredLocation(v3FormationPosition[0][i], pFielder, false);
        }
        else
        {
            v3FormationPosition[0][i] = v3Zero;
            v3FormationPosition[1][i] = v3Zero;
            fWeights[i] = 0.0f;
        }
    }

    // Ensure ball position weight doesn't go below 0
    if (fWeights[2] < 0.0f)
    {
        fWeights[2] = 1.0f;
    }

    // Scale weights by fBallPosFormationWeight
    float scaledBallWeight = fWeights[2] * fBallPosFormationWeight;
    float remainingWeight = 1.0f - scaledBallWeight;
    fWeights[2] = scaledBallWeight;
    fWeights[0] *= remainingWeight;
    fWeights[1] *= remainingWeight;

    // Second loop: calculate final position
    for (int i = 0; i < 3; i++)
    {
        FormationEval* pFormation = m_pFormations[i];
        if (pFormation != nullptr && pFormation->m_pFormationSpec != nullptr)
        {
            nlVector3 pos = v3FormationPosition[0][i];
            v3FutureDesiredPosition.f.x += fWeights[i] * v3FormationPosition[1][i].f.x;
            v3FutureDesiredPosition.f.y += fWeights[i] * v3FormationPosition[1][i].f.y;
            v3FutureDesiredPosition.f.z += fWeights[i] * v3FormationPosition[1][i].f.z;
            float result = pFormation->IsFielderInPosition(pFielder, pos, bInPosition);
            fFielderInPosition += fWeights[i] * result;
        }
    }

    v3DestPosition = v3FutureDesiredPosition;
    bool inPos = fFielderInPosition >= 0.5f;

    // Cache the result
    cached->vPosition = v3DestPosition;
    cached->bInPosition = inPos;
    cached->bCacheIsValid = true;

    return inPos;
}

/**
 * Offset/Address/Size: 0x1E00 | 0x8003A050 | size: 0x22C
 * TODO: 95.3% match - r26/r27/r30 circular register swap for counter/formID/offset in search loop
 */
FormationEval::FormationEval(FormationManager* pMgr, eFormationType type, const FormationSpec* spec)
{
    m_SortTimer.SetSeconds(0.0f);
    m_SortTimer.m_uPackedTime = 0;
    m_pFormationManager = pMgr;
    m_pKeyPlayer = NULL;
    m_pFormationSpec = spec;
    m_eFormationType = type;
    m_iFielderFormationPos[0] = 0;
    m_iFielderFormationPos[1] = 1;
    m_iFielderFormationPos[2] = 2;
    m_iFielderFormationPos[3] = 3;
}

FormationDefensive::FormationDefensive(FormationManager* pMgr, eFormationType type, const FormationSpec* spec)
    : FormationEval(pMgr, type, spec)
{
}

FormationOffensive::FormationOffensive(FormationManager* pMgr, eFormationType type, const FormationSpec* spec)
    : FormationEval(pMgr, type, spec)
{
}

FormationBallPosition::FormationBallPosition(FormationManager* pMgr, eFormationType type, const FormationSet* set)
    : FormationEval(pMgr, type, NULL)
{
    m_pNextClosestFormation = NULL;
    m_pFormationSet = set;
}

FormationBallPosition::FormationBallPosition(FormationManager* pMgr, eFormationType type, const FormationSpec* spec)
    : FormationEval(pMgr, type, spec)
{
    m_pNextClosestFormation = NULL;
    m_pFormationSet = NULL;
}

FormationEval* FormationEval::Create(FormationManager* pManager, eFormationType formType, eFormationSet formSetID, eFormation formID)
{
    int i;
    FormationEval* result = NULL;
    FormationSet* pFormationSet = NULL;
    const FormationSpec* pFormationSpec = NULL;

    if (formSetID != FSET_NONE)
    {
        pFormationSet = &FormationManager::m_FormationSetArray[formSetID];
    }

    if (formType != (eFormationType)-1)
    {
        const FormationSpec* spec = NULL;
        for (i = 0; i < FormationManager::m_NumFormationSets; i++)
        {
            spec = FormationManager::m_FormationSetArray[i].GetFormationSpecFromID(formID);
            if (spec != NULL)
            {
                break;
            }
        }
        pFormationSpec = spec;
    }

    switch (formType)
    {
    case FTYPE_DEFENSIVE:
        result = new (nlMalloc(sizeof(FormationDefensive), 8, false)) FormationDefensive(pManager, formType, pFormationSpec);
        break;
    case FTYPE_OFFENSIVE:
        result = new (nlMalloc(sizeof(FormationOffensive), 8, false)) FormationOffensive(pManager, formType, pFormationSpec);
        break;
    case FTYPE_BALLPOSITION:
        result = new (nlMalloc(sizeof(FormationBallPosition), 8, false)) FormationBallPosition(pManager, formType, pFormationSet);
        break;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x1D40 | 0x80039F90 | size: 0xC0
 */
void FormationEval::Update(float fDeltaT)
{
    m_pKeyPlayer = NULL;

    char buff[32];
    nlSNPrintf(buff, 31, "FormationEvalUpdate%d_%d", m_eFormationType, m_pFormationManager->m_pTeam->m_nSide);
    nlStringHash(buff);

    if (m_SortTimer.Countdown(fDeltaT, 0.0f))
    {
        SortPlayers(NULL);
        float randomTime = nlRandomf(0.2f, &nlDefaultSeed);
        m_SortTimer.SetSeconds(0.4f + randomTime);
    }
}

/**
 * Offset/Address/Size: 0x1D24 | 0x80039F74 | size: 0x1C
 */
float FormationEval::GetWeight()
{
    if (m_pFormationSpec != NULL)
    {
        return 1.0f;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x1A14 | 0x80039C64 | size: 0x310
 * TODO: 94.18% match - stack frame/register allocation and loop-counter hoist
 * diffs around nested permutation loops (MWCC promotes i_pos array loads to
 * callee-saved regs r29/r30/r31 instead of using volatile r0).
 */
#pragma opt_common_subs off
void FormationEval::AssignPositionsToFielders(unsigned int* pFielderPosAssignments, float (*fFielderToPositionDistance)[4])
{
    float fBestDistance = 100000000000.0f;
    cPlayer* pKeyPlayer = GetKeyPlayer();
    cTeam* pTeam = m_pFormationManager->m_pTeam;
    int aiAssignedPos[4];

    for (aiAssignedPos[0] = 0; aiAssignedPos[0] < 4; aiAssignedPos[0]++)
    {
        float fDistance = fFielderToPositionDistance[0][aiAssignedPos[0]];

        for (aiAssignedPos[1] = 0; aiAssignedPos[1] < 4; aiAssignedPos[1]++)
        {
            if (aiAssignedPos[1] == aiAssignedPos[0])
            {
                continue;
            }

            float fDistancePos1 = fDistance;
            fDistance += fFielderToPositionDistance[1][aiAssignedPos[1]];

            for (aiAssignedPos[2] = 0; aiAssignedPos[2] < 4; aiAssignedPos[2]++)
            {
                if (aiAssignedPos[2] == aiAssignedPos[1])
                {
                    continue;
                }

                if (aiAssignedPos[2] == aiAssignedPos[0])
                {
                    continue;
                }

                float fDistancePos2 = fDistance;
                fDistance += fFielderToPositionDistance[2][aiAssignedPos[2]];

                for (aiAssignedPos[3] = 0; aiAssignedPos[3] < 4; aiAssignedPos[3]++)
                {
                    if (aiAssignedPos[3] == aiAssignedPos[2])
                    {
                        continue;
                    }

                    if (aiAssignedPos[3] == aiAssignedPos[1])
                    {
                        continue;
                    }

                    if (aiAssignedPos[3] == aiAssignedPos[0])
                    {
                        continue;
                    }

                    float fDistancePos3 = fDistance;
                    fDistance += fFielderToPositionDistance[3][aiAssignedPos[3]];

                    float fCaptainPosScore = 0.0f;
                    float fCaptainPosCount = fCaptainPosScore;
                    int* piAssignedPos = aiAssignedPos;

                    for (int iFielder = 0; iFielder < 4; iFielder++)
                    {
                        if (iFielder == pKeyPlayer->m_ID)
                        {
                            if (*piAssignedPos == m_pFormationSpec->m_iKeyIndex)
                            {
                                fCaptainPosScore += 0.5f;
                                fCaptainPosCount += 1.0f;
                            }
                        }

                        cFielder* pFielder = pTeam->GetFielder(iFielder);
                        if (pFielder->IsCaptain() && pFielder->GetGlobalPad() == NULL)
                        {
                            for (int iPos = 0; iPos < 4; iPos++)
                            {
                                if (iPos == *piAssignedPos)
                                {
                                    if (g_fFormationCaptainPosBonus[iPos] > 0.0f)
                                    {
                                        fCaptainPosCount += 1.0f;
                                        fCaptainPosScore += 1.0f - g_fFormationCaptainPosBonus[iPos];
                                        break;
                                    }
                                }
                            }
                        }

                        piAssignedPos++;
                    }

                    float fCaptainPosMult = 0.0f;
                    if (fCaptainPosCount > 0.0f)
                    {
                        fCaptainPosMult = fCaptainPosScore / fCaptainPosCount;
                    }

                    if (fCaptainPosMult > 0.0f)
                    {
                        fDistance *= fCaptainPosMult;
                    }

                    if (fDistance < fBestDistance)
                    {
                        fBestDistance = fDistance;
                        pFielderPosAssignments[0] = aiAssignedPos[0];
                        pFielderPosAssignments[1] = aiAssignedPos[1];
                        pFielderPosAssignments[2] = aiAssignedPos[2];
                        pFielderPosAssignments[3] = aiAssignedPos[3];
                    }

                    fDistance = fDistancePos3;
                }

                fDistance = fDistancePos2;
            }

            fDistance = fDistancePos1;
        }
    }
}
#pragma opt_common_subs on

/**
 * Offset/Address/Size: 0x1798 | 0x800399E8 | size: 0x27C
 * TODO: 97.30% match - remaining diffs are this/team/pFielder GPR swap, prologue mr order, and center-add float register allocation cascade
 */
void FormationEval::SortPlayers(const nlVector2* v2Center)
{
    FormationEval* const self = this;
    const nlVector2* pCenter = v2Center;
    float fFielderToPositionDistance[4][4];
    nlVector2 av2FormationPositions[4];
    nlVector3 av3FielderAILocs[4];
    nlVector2* pFormPositions;
    int i;
    nlVector2 v2CenterOfPlayers;
    int i_fielder;
    int i_pos;

    if (self->m_pFormationSpec == NULL)
    {
        return;
    }

    cTeam* team = self->m_pFormationManager->m_pTeam;
    memset(fFielderToPositionDistance, 0, sizeof(fFielderToPositionDistance));

    for (i = 0; i < 4; i++)
    {
        cFielder* pFielder = team->GetFielder(i);
        if (pFielder->GetGlobalPad() != NULL)
        {
            f32 fx = 0.4f * pFielder->m_v3Velocity.f.x + pFielder->m_v3Position.f.x;
            f32 fy = 0.4f * pFielder->m_v3Velocity.f.y + pFielder->m_v3Position.f.y;
            f32 fz = 0.4f * pFielder->m_v3Velocity.f.z + pFielder->m_v3Position.f.z;
            av3FielderAILocs[i].f.x = fx;
            av3FielderAILocs[i].f.y = fy;
            av3FielderAILocs[i].f.z = fz;
        }
        else
        {
            f32 fx = 0.15f * pFielder->m_v3Velocity.f.x + pFielder->m_v3Position.f.x;
            f32 fy = 0.15f * pFielder->m_v3Velocity.f.y + pFielder->m_v3Position.f.y;
            f32 fz = 0.15f * pFielder->m_v3Velocity.f.z + pFielder->m_v3Position.f.z;
            av3FielderAILocs[i].f.x = fx;
            av3FielderAILocs[i].f.y = fy;
            av3FielderAILocs[i].f.z = fz;
        }

        if (team->m_nSide != 0)
        {
            f32 nx = -av3FielderAILocs[i].f.x;
            f32 ny = -av3FielderAILocs[i].f.y;
            av3FielderAILocs[i].f.x = nx;
            av3FielderAILocs[i].f.y = ny;
            av3FielderAILocs[i].f.z = 0.0f;
        }
    }

    if (pCenter != NULL)
    {
        v2CenterOfPlayers = *pCenter;
    }
    else
    {
        v2CenterOfPlayers = self->m_pFormationManager->m_v2AIFielderCenter;
        v2CenterOfPlayers.f.y -= self->m_pFormationSpec->m_v2Center.f.y;
        v2CenterOfPlayers.f.x -= self->m_pFormationSpec->m_v2Center.f.x;
    }

    av2FormationPositions[0].f.y = self->m_pFormationSpec->m_Positions[0].m_Location.f.y + v2CenterOfPlayers.f.y;
    av2FormationPositions[1].f.y = self->m_pFormationSpec->m_Positions[1].m_Location.f.y + v2CenterOfPlayers.f.y;
    av2FormationPositions[2].f.y = self->m_pFormationSpec->m_Positions[2].m_Location.f.y + v2CenterOfPlayers.f.y;
    av2FormationPositions[3].f.y = self->m_pFormationSpec->m_Positions[3].m_Location.f.y + v2CenterOfPlayers.f.y;

    av2FormationPositions[0].f.x = self->m_pFormationSpec->m_Positions[0].m_Location.f.x + v2CenterOfPlayers.f.x;
    av2FormationPositions[1].f.x = self->m_pFormationSpec->m_Positions[1].m_Location.f.x + v2CenterOfPlayers.f.x;
    av2FormationPositions[2].f.x = self->m_pFormationSpec->m_Positions[2].m_Location.f.x + v2CenterOfPlayers.f.x;
    av2FormationPositions[3].f.x = self->m_pFormationSpec->m_Positions[3].m_Location.f.x + v2CenterOfPlayers.f.x;

    pFormPositions = av2FormationPositions;
    self->GetKeyPlayer();

    for (i_fielder = 0; i_fielder < 4; i_fielder++)
    {
        team->GetFielder(i_fielder);
        for (i_pos = 0; i_pos < 4; i_pos++)
        {
            float dy = pFormPositions[i_pos].f.y - av3FielderAILocs[i_fielder].f.y;
            float dx = pFormPositions[i_pos].f.x - av3FielderAILocs[i_fielder].f.x;
            fFielderToPositionDistance[i_fielder][i_pos] = nlSqrt(dx * dx + dy * dy, true);
        }
    }

    self->AssignPositionsToFielders(self->m_iFielderFormationPos, fFielderToPositionDistance);
}

/**
 * Offset/Address/Size: 0x1630 | 0x80039880 | size: 0x168
 */
cPlayer* FormationEval::GetKeyPlayer()
{
    if (m_pKeyPlayer != NULL)
    {
        return m_pKeyPlayer;
    }

    cTeam* team = m_pFormationManager->m_pTeam;
    cPlayer* otherCarrier;
    cPlayer* pKeyPlayer;

    {
        cPlayer* temp = Fuzzy::GetStrategicBallCarrier(team).mData.pPlayer;
        pKeyPlayer = temp;
    }

    if (pKeyPlayer == NULL)
    {
        pKeyPlayer = g_pBall->GetOwnerGoalie();
    }

    if (pKeyPlayer == NULL)
    {
        otherCarrier = Fuzzy::GetStrategicBallCarrier(team->GetOtherTeam()).mData.pPlayer;

        if (otherCarrier != NULL)
        {
            for (int i_fielder = 0; i_fielder < 4; i_fielder++)
            {
                cFielder* pFielder = team->GetFielder(i_fielder);
                if (pFielder->m_pMark == (cFielder*)otherCarrier)
                {
                    pKeyPlayer = pFielder;
                    break;
                }
            }
        }
        else
        {
            pKeyPlayer = Fuzzy::GetBestBallInterceptor(team).mData.pPlayer;
        }
    }

    if (pKeyPlayer == NULL)
    {
        for (int i_fielder = 0; i_fielder < 4; i_fielder++)
        {
            if (team->GetFielder(i_fielder)->GetGlobalPad() != NULL)
            {
                pKeyPlayer = team->GetFielder(i_fielder);
                break;
            }
        }
    }

    if (pKeyPlayer == NULL)
    {
        if (m_eFormationType == FTYPE_DEFENSIVE)
        {
            pKeyPlayer = team->GetRearMostFielder();
        }
        else if (m_eFormationType == FTYPE_OFFENSIVE)
        {
            pKeyPlayer = team->GetFrontMostFielder();
        }
        else
        {
            pKeyPlayer = team->GetStriker();
        }
    }

    m_pKeyPlayer = pKeyPlayer;
    return pKeyPlayer;
}

/**
 * Offset/Address/Size: 0x1488 | 0x800396D8 | size: 0x1A8
 */
void FormationEval::GetKeyPositions(cFielder* pFielder, nlVector3& v3KeyAIPosition, nlVector3* pKeyFormationAIPosition, bool bExtrapolate)
{
    cPlayer* keyPlayer = GetKeyPlayer();

    if (keyPlayer == pFielder)
    {
        FormationManager* mgr = m_pFormationManager;
        f32 x = mgr->m_v2AIFielderCenter.f.x;
        f32 y = mgr->m_v2AIFielderCenter.f.y;
        v3KeyAIPosition.f.x = x;
        v3KeyAIPosition.f.y = y;
        v3KeyAIPosition.f.z = 0.0f;

        if (pKeyFormationAIPosition != NULL)
        {
            const FormationSpec* spec = m_pFormationSpec;
            f32 cx = spec->m_v2Center.f.x;
            f32 cy = spec->m_v2Center.f.y;
            pKeyFormationAIPosition->f.x = cx;
            pKeyFormationAIPosition->f.y = cy;
            pKeyFormationAIPosition->f.z = 0.0f;
        }
    }
    else
    {
        if (bExtrapolate)
        {
            f32 t = 0.5f;
            f32 rz = keyPlayer->m_v3Position.f.z + t * keyPlayer->m_v3Velocity.f.z;
            f32 rx = keyPlayer->m_v3Position.f.x + t * keyPlayer->m_v3Velocity.f.x;
            f32 ry = keyPlayer->m_v3Position.f.y + t * keyPlayer->m_v3Velocity.f.y;
            v3KeyAIPosition.f.x = rx;
            v3KeyAIPosition.f.y = ry;
            v3KeyAIPosition.f.z = rz;
        }
        else
        {
            v3KeyAIPosition = keyPlayer->m_v3Position;
        }

        if (pFielder->m_pTeam->m_nSide != 0)
        {
            f32 nx = -v3KeyAIPosition.f.x;
            f32 ny = -v3KeyAIPosition.f.y;
            v3KeyAIPosition.f.x = nx;
            v3KeyAIPosition.f.y = ny;
            v3KeyAIPosition.f.z = 0.0f;
        }

        if (pKeyFormationAIPosition != NULL)
        {
            if (m_pFormationSpec->m_iKeyIndex > -1)
            {
                nlVector2& keyLoc = m_pFormationSpec->GetKeyLocation();
                f32 lx = keyLoc.f.x;
                f32 ly = keyLoc.f.y;
                pKeyFormationAIPosition->f.x = lx;
                pKeyFormationAIPosition->f.y = ly;
                pKeyFormationAIPosition->f.z = 0.0f;
            }
            else
            {
                pKeyFormationAIPosition->f.x = 1.0f;
                pKeyFormationAIPosition->f.y = 0.0f;
                pKeyFormationAIPosition->f.z = 0.0f;

                if (keyPlayer->m_eClassType != GOALIE)
                {
                    u32 posIndex = m_iFielderFormationPos[keyPlayer->m_ID];
                    const FormationPos* pPos = &m_pFormationSpec->m_Positions[posIndex];
                    f32 px = pPos->m_Location.f.x;
                    f32 py = pPos->m_Location.f.y;
                    pKeyFormationAIPosition->f.x = px;
                    pKeyFormationAIPosition->f.y = py;
                    pKeyFormationAIPosition->f.z = 0.0f;
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1330 | 0x80039580 | size: 0x158
 * TODO: 98.78% match - float register allocation: X clamp uses f4/f5/f6 shifted by +1
 * (target: f5=minX, f6=clampX, f4=maxX; actual: f4=minX, f5=clampX, f6=maxX).
 * Y clamp uses f0/f3 swapped (target: f0=minY, f3=clampY; actual: f3=minY, f0=clampY).
 * All 16 diffs are register-only (r markers).
 */
void FormationEval::CalculateDesiredLocation(nlVector3& destPosition, cFielder* pFielder, bool bExtrapolate)
{
    nlVector3 v3KeyAIPosition;
    nlVector3 v3KeyFormationAIPosition;
    nlVector2 v2FormationMin;
    nlVector2 v2FormationMax;

    GetKeyPositions(pFielder, v3KeyAIPosition, &v3KeyFormationAIPosition, bExtrapolate);

    m_pFormationSpec->CalculateExtents(v2FormationMin, v2FormationMax, *(nlVector2*)&v3KeyFormationAIPosition);

    u32 posIndex = m_iFielderFormationPos[pFielder->m_ID];
    const FormationSpec* pSpec = m_pFormationSpec;

    f32 maxX = v2FormationMax.f.x;
    f32 minX = v2FormationMin.f.x;

    const FormationPos* pPos = &pSpec->m_Positions[posIndex];

    f32 locY = v3KeyFormationAIPosition.f.y;
    f32 locX = v3KeyFormationAIPosition.f.x;

    f32 posLocY = pPos->m_Location.f.y;
    f32 posLocX = pPos->m_Location.f.x;

    f32 dy = posLocY - locY;
    f32 dx = posLocX - locX;

    f32 clampX = v3KeyAIPosition.f.x;
    clampX = (clampX >= minX) ? clampX : minX;
    clampX = (clampX <= maxX) ? clampX : maxX;

    destPosition.f.x = dx + clampX;

    f32 maxY = v2FormationMax.f.y;
    f32 clampY = v3KeyAIPosition.f.y;
    f32 minY = v2FormationMin.f.y;

    clampY = (clampY >= minY) ? clampY : minY;
    clampY = (clampY <= maxY) ? clampY : maxY;

    destPosition.f.y = dy + clampY;
    f32 zero = 0.0f;
    destPosition.f.z = zero;

    if (pFielder->m_pTeam->m_nSide != 0)
    {
        nlVec3Set(destPosition, -destPosition.f.x, -destPosition.f.y, zero);
    }
}

/**
 * Offset/Address/Size: 0x1150 | 0x800393A0 | size: 0x1E0
 */
float FormationEval::IsFielderInPosition(cFielder* pFielder, nlVector3 v3Pos, bool bExtended)
{
    if (m_pFormationSpec == NULL)
    {
        return 0.0f;
    }

    nlVector3 v3DesiredPos;
    GetKeyPositions(pFielder, v3DesiredPos, NULL, false);

    if (pFielder->m_pTeam->m_nSide != 0)
    {
        f32 negY = -v3DesiredPos.f.y;
        f32 negX = -v3DesiredPos.f.x;
        v3DesiredPos.f.z = 0.0f;
        v3DesiredPos.f.x = negX;
        v3DesiredPos.f.y = negY;
    }

    f32 dx = pFielder->m_v3Position.f.x - v3DesiredPos.f.x;
    f32 dy = pFielder->m_v3Position.f.y - v3DesiredPos.f.y;
    f32 distToDesired = nlSqrt(dx * dx + dy * dy, true);

    f32 dx2 = pFielder->m_v3Position.f.x - v3Pos.f.x;
    f32 dy2 = pFielder->m_v3Position.f.y - v3Pos.f.y;
    f32 distToTarget = nlSqrt(dx2 * dx2 + dy2 * dy2, true);

    f32 normalizedDist = NormalizeVal(distToDesired, g_pGame->m_pGameTweaks->vGetInPositionKeyFielderDist);

    f32 inDist = Interpolate(
        g_pGame->m_pGameTweaks->vGetInPositionInRadius.f.x,
        g_pGame->m_pGameTweaks->vGetInPositionInRadius.f.y,
        normalizedDist);

    f32 outDist = Interpolate(
        g_pGame->m_pGameTweaks->vGetInPositionOutRadius.f.x,
        g_pGame->m_pGameTweaks->vGetInPositionOutRadius.f.y,
        normalizedDist);

    f32 result = 0.0f;
    if (distToTarget <= inDist)
    {
        result = 1.0f;
    }
    else if (bExtended && distToTarget <= outDist)
    {
        result = nlMinEquals(nlMaxEquals(1.0f - distToTarget / outDist, 0.0f), 1.0f);
    }

    return result;
}

struct FormationPositionThresholds
{
    float fInRadius;
    float fOutUpField;
    float fOutDownField;
    float fOutLateral;
};

extern FormationPositionThresholds g_aDefensiveFormationThresholds[4];

/**
 * Offset/Address/Size: 0xE70 | 0x800390C0 | size: 0x2E0
 * TODO: 91.90% match - remaining diffs are MWCC floating-point register allocation around fInPosition in the near-zero branch.
 */
float FormationDefensive::IsFielderInPosition(cFielder* pFielder, nlVector3 v3DesiredPosition, bool bInPosition)
{
    nlVector3 v3KeyPosition;
    GetKeyPositions(pFielder, v3KeyPosition, NULL, false);

    if (pFielder->m_pTeam->m_nSide != 0)
    {
        float negY = -v3KeyPosition.f.y;
        float negX = -v3KeyPosition.f.x;
        v3KeyPosition.f.z = 0.0f;
        v3KeyPosition.f.x = negX;
        v3KeyPosition.f.y = negY;
    }

    float dx = pFielder->m_v3Position.f.x - v3KeyPosition.f.x;
    float dy = pFielder->m_v3Position.f.y - v3KeyPosition.f.y;
    float fPercent = nlSqrt(dx * dx + dy * dy, true);

    fPercent = NormalizeVal(fPercent, g_pGame->m_pGameTweaks->vGetInPositionKeyFielderDist);

    nlVector3 v3FielderPos = pFielder->m_v3Position;
    nlVector3 v3NetLocation = pFielder->GetAIOffNetLocation(NULL);
    const FormationPositionThresholds* pPositionThresholds = &g_aDefensiveFormationThresholds[pFielder->m_eRole];

    float distY = v3FielderPos.f.y - v3DesiredPosition.f.y;
    float distX = v3FielderPos.f.x - v3DesiredPosition.f.x;

    float fInPosition = 0.0f;

    if (nlSqrt(distX * distX + distY * distY, true) <= pPositionThresholds->fInRadius)
    {
        fInPosition = 1.0f;
    }
    else if ((double)((float)fabs(fPercent - fInPosition) <= 0.00001f) != (double)fInPosition)
    {
        if (pFielder == GetKeyPlayer())
        {
            fInPosition = 1.0f;
        }
        else
        {
            fInPosition = 0.0f;
        }
    }
    else if (bInPosition)
    {
        float fUpScore = ((v3DesiredPosition.f.x - v3FielderPos.f.x) * AIsgn(v3NetLocation.f.x))
                       / (fPercent * pPositionThresholds->fOutUpField);

        float fDownScore = ((v3FielderPos.f.x - v3DesiredPosition.f.x) * AIsgn(v3NetLocation.f.x))
                         / (fPercent * pPositionThresholds->fOutDownField);

        float fLateralScore = (float)fabs(v3FielderPos.f.y - v3DesiredPosition.f.y)
                            / (fPercent * pPositionThresholds->fOutLateral);

        float fScore = nlMaxEquals(fUpScore, 0.0f);
        fScore += nlMaxEquals(fDownScore, 0.0f);
        fScore += nlMaxEquals(fLateralScore, 0.0f);
        fScore *= 0.33333334f;
        fScore = nlMinEquals(nlMaxEquals(fScore, 0.0f), 1.0f);
        fInPosition = 1.0f - fScore;
    }

    return fInPosition;
}

/**
 * Offset/Address/Size: 0xE40 | 0x80039090 | size: 0x30
 */
float FormationDefensive::GetWeight()
{
    return NormalizeVal(m_pFormationManager->m_v2AIFielderCenter.f.x, 25.0f, -25.0f);
}

/**
 * Offset/Address/Size: 0xB60 | 0x80038DB0 | size: 0x2E0
 */
float FormationOffensive::IsFielderInPosition(cFielder* pFielder, nlVector3 v3DesiredPosition, bool bInPosition)
{
    nlVector3 v3KeyPosition;
    GetKeyPositions(pFielder, v3KeyPosition, NULL, false);

    if (pFielder->m_pTeam->m_nSide != 0)
    {
        float negY = -v3KeyPosition.f.y;
        float negX = -v3KeyPosition.f.x;
        v3KeyPosition.f.z = 0.0f;
        v3KeyPosition.f.x = negX;
        v3KeyPosition.f.y = negY;
    }

    float dx = pFielder->m_v3Position.f.x - v3KeyPosition.f.x;
    float dy = pFielder->m_v3Position.f.y - v3KeyPosition.f.y;
    float fPercent = nlSqrt(dx * dx + dy * dy, true);

    fPercent = NormalizeVal(fPercent, g_pGame->m_pGameTweaks->vGetInPositionKeyFielderDist);

    nlVector3 v3FielderPos = pFielder->m_v3Position;
    nlVector3 v3NetLocation = pFielder->GetAIOffNetLocation(NULL);
    const FormationPositionThresholds* pPositionThresholds = &g_aDefensiveFormationThresholds[pFielder->m_eRole];

    float distY = v3FielderPos.f.y - v3DesiredPosition.f.y;
    float distX = v3FielderPos.f.x - v3DesiredPosition.f.x;

    float fInPosition = 0.0f;

    if (nlSqrt(distX * distX + distY * distY, true) <= pPositionThresholds->fInRadius)
    {
        fInPosition = 1.0f;
    }
    else if ((double)((float)fabs(fPercent - fInPosition) <= 0.00001f) != (double)fInPosition)
    {
        if (pFielder == GetKeyPlayer())
        {
            fInPosition = 1.0f;
        }
        else
        {
            fInPosition = 0.0f;
        }
    }
    else if (bInPosition)
    {
        float fUpScore = ((v3DesiredPosition.f.x - v3FielderPos.f.x) * AIsgn(v3NetLocation.f.x))
                       / (fPercent * pPositionThresholds->fOutUpField);

        float fDownScore = ((v3FielderPos.f.x - v3DesiredPosition.f.x) * AIsgn(v3NetLocation.f.x))
                         / (fPercent * pPositionThresholds->fOutDownField);

        float fLateralScore = (float)fabs(v3FielderPos.f.y - v3DesiredPosition.f.y)
                            / (fPercent * pPositionThresholds->fOutLateral);

        float fScore = (nlMaxEquals(fUpScore, 0.0f) + nlMaxEquals(fDownScore, 0.0f) + nlMaxEquals(fLateralScore, 0.0f)) * 0.33333334f;
        fScore = nlMinEquals(nlMaxEquals(fScore, 0.0f), 1.0f);
        fInPosition = 1.0f - fScore;
    }

    return fInPosition;
}

/**
 * Offset/Address/Size: 0xB30 | 0x80038D80 | size: 0x30
 */
float FormationOffensive::GetWeight()
{
    return NormalizeVal(m_pFormationManager->m_v2AIFielderCenter.f.x, -25.0f, 25.0f);
}

/**
 * Offset/Address/Size: 0xA98 | 0x80038CE8 | size: 0x98
 */
FormationBallPosition::~FormationBallPosition()
{
    if (m_pNextClosestFormation != NULL)
    {
        delete m_pNextClosestFormation;
        m_pNextClosestFormation = NULL;
    }
}

/**
 * Offset/Address/Size: 0x930 | 0x80038B80 | size: 0x168
 */
void FormationBallPosition::Update(float fDeltaT)
{
    nlVector3 v3AIBallLoc;

    m_pKeyPlayer = NULL;

    if (m_SortTimer.Countdown(fDeltaT, 0.0f))
    {
        if (g_pBall->m_pOwner != NULL)
        {
            v3AIBallLoc = g_pBall->m_pOwner->m_v3Position;
        }
        else if (g_pBall->m_pPassTarget != NULL)
        {
            v3AIBallLoc = g_pBall->m_pPassTarget->m_v3Position;
        }
        else
        {
            v3AIBallLoc = g_pBall->m_v3Position;
        }

        if (m_pFormationManager->m_pTeam->m_nSide != 0)
        {
            float negY = -v3AIBallLoc.f.y;
            float negX = -v3AIBallLoc.f.x;
            v3AIBallLoc.f.z = 0.0f;
            v3AIBallLoc.f.x = negX;
            v3AIBallLoc.f.y = negY;
        }

        nlVector2 vAIBallLoc = *(const nlVector2*)&v3AIBallLoc;

        SelectClosestBallFormations(vAIBallLoc);

        if (m_pFormationSpec != NULL)
        {
            SortPlayers((const nlVector2*)&v3Zero);
        }

        if (m_pNextClosestFormation != NULL)
        {
            m_pNextClosestFormation->SortPlayers((const nlVector2*)&v3Zero);
        }

        float fRandom = nlRandomf(0.2f, &nlDefaultSeed);
        m_SortTimer.SetSeconds(0.4f + fRandom);
    }
}

/**
 * Offset/Address/Size: 0x618 | 0x80038868 | size: 0x318
 */
/**
 * Offset/Address/Size: 0x618 | 0x80038868 | size: 0x318
 * TODO: 98.28% match - 4 register diffs (f1/f2 swap in loop body at offsets 80-8c),
 *       likely -inline deferred vs -inline auto difference
 */
bool FormationBallPosition::SelectClosestBallFormations(const nlVector2& v2AIBallLoc)
{
    FormationSpec* pClosest[2] = { NULL, NULL };
    float fDist[2] = { 3.402823466e+38f, 3.402823466e+38f };
    s32 i = 0;
    float ballX, ballY;
    ballY = v2AIBallLoc.f.y;
    ballX = v2AIBallLoc.f.x;

    for (; i < m_pFormationSet->m_NumFormationDefs; i++)
    {
        FormationSpec* pSpec = m_pFormationSet->GetFormationSpec(i);
        nlVector2& keyLoc = pSpec->GetKeyLocation();
        float dy = ballY - keyLoc.f.y;
        float dx = ballX - keyLoc.f.x;
        float dist = nlSqrt(dx * dx + dy * dy, true);

        if (dist < fDist[0])
        {
            fDist[1] = fDist[0];
            pClosest[1] = pClosest[0];
            pClosest[0] = pSpec;
            fDist[0] = dist;
        }
        else if (dist < fDist[1])
        {
            pClosest[1] = pSpec;
            fDist[1] = dist;
        }
    }

    bool bChanged = false;

    FormationSpec* pClosestFormation = pClosest[0];
    if (pClosestFormation != m_pFormationSpec)
    {
        float fInDist = pClosestFormation->m_InRadius;
        if (m_pFormationSpec == NULL)
        {
            fInDist = pClosestFormation->m_OutRadius;
        }

        nlVector2& keyLoc = pClosestFormation->GetKeyLocation();
        float dy = v2AIBallLoc.f.y - keyLoc.f.y;
        float dx = v2AIBallLoc.f.x - keyLoc.f.x;
        float dist = nlSqrt(dx * dx + dy * dy, true);

        if (dist <= fInDist)
        {
            m_pFormationSpec = pClosestFormation;
            bChanged = true;
        }
    }

    if (m_pNextClosestFormation != NULL)
    {
        FormationSpec* pNextSpec = (FormationSpec*)m_pNextClosestFormation->m_pFormationSpec;
        float fOutDist = pNextSpec->m_OutRadius;
        nlVector2& keyLoc = pNextSpec->GetKeyLocation();
        float dy = v2AIBallLoc.f.y - keyLoc.f.y;
        float dx = v2AIBallLoc.f.x - keyLoc.f.x;
        float dist = nlSqrt(dx * dx + dy * dy, true);

        if (dist > fOutDist)
        {
            delete m_pNextClosestFormation;
            m_pNextClosestFormation = NULL;
            goto done;
        }
    }

    if (pClosest[1] != NULL)
    {
        FormationSpec* pCurrentNextSpec;
        if (m_pNextClosestFormation != NULL)
        {
            pCurrentNextSpec = (FormationSpec*)m_pNextClosestFormation->m_pFormationSpec;
        }
        else
        {
            pCurrentNextSpec = NULL;
        }

        if (pCurrentNextSpec != m_pFormationSpec)
        {
            float fInDist = pClosest[1]->m_InRadius;
            if (pCurrentNextSpec == NULL)
            {
                fInDist = pClosest[1]->m_OutRadius;
            }

            nlVector2& keyLoc = pClosest[1]->GetKeyLocation();
            float dy = v2AIBallLoc.f.y - keyLoc.f.y;
            float dx = v2AIBallLoc.f.x - keyLoc.f.x;
            float dist = nlSqrt(dx * dx + dy * dy, true);

            if (dist > fInDist)
            {
                goto done;
            }
        }

        if (m_pNextClosestFormation == NULL)
        {
            m_pNextClosestFormation = new (nlMalloc(sizeof(FormationBallPosition), 8, false)) FormationBallPosition(m_pFormationManager, FTYPE_BALLPOSITION, pClosest[1]);
            bChanged = true;
        }
        else if (m_pNextClosestFormation->m_pFormationSpec != pClosest[1])
        {
            m_pNextClosestFormation->m_pFormationSpec = pClosest[1];
            bChanged = true;
        }
    }

done:
    return bChanged;
}

/**
 * Offset/Address/Size: 0x2C0 | 0x80038510 | size: 0x358
 * TODO: 99.34% match - remaining diffs are MWCC floating-point register allocation in clamp blocks:
 * X clamp: f4/f6 swap for clampX/maxX + cror flag direction; Y clamp: f0/f3 swap + stack offset swap.
 */
void FormationBallPosition::CalculateDesiredLocation(nlVector3& destPosition, cFielder* pFielder, bool bExtrapolate)
{
    nlVector3 v3NextPosition;

    {
        nlVector3 v3KeyFormationAIPosition;
        nlVector3 v3KeyAIPosition;
        nlVector2 v2FormationMax;
        nlVector2 v2FormationMin;

        if (m_pFormationSpec == NULL)
        {
            destPosition = pFielder->m_v3Position;
            return;
        }

        GetKeyPositions(pFielder, v3KeyAIPosition, &v3KeyFormationAIPosition, bExtrapolate);

        m_pFormationSpec->CalculateExtents(v2FormationMin, v2FormationMax, *(nlVector2*)&v3KeyFormationAIPosition);

        u32 posIndex = m_iFielderFormationPos[pFielder->m_ID];
        const FormationSpec* pSpec = m_pFormationSpec;

        f32 maxX = v2FormationMax.f.x;
        f32 minX = v2FormationMin.f.x;

        const FormationPos* pPos = &pSpec->m_Positions[posIndex];

        f32 locY = v3KeyFormationAIPosition.f.y;
        f32 locX = v3KeyFormationAIPosition.f.x;

        f32 posLocY = pPos->m_Location.f.y;
        f32 posLocX = pPos->m_Location.f.x;

        f32 dy = posLocY - locY;
        f32 dx = posLocX - locX;

        f32 clampX = v3KeyAIPosition.f.x;
        clampX = (minX <= clampX) ? clampX : minX;
        clampX = (maxX >= clampX) ? clampX : maxX;

        destPosition.f.x = dx + clampX;

        f32 maxY = v2FormationMax.f.y;
        f32 clampY = v3KeyAIPosition.f.y;
        f32 minY = v2FormationMin.f.y;

        clampY = (clampY >= minY) ? clampY : minY;
        clampY = (clampY <= maxY) ? clampY : maxY;

        f32 zero = 0.0f;
        destPosition.f.y = dy + clampY;
        destPosition.f.z = zero;

        if (pFielder->m_pTeam->m_nSide != 0)
        {
            nlVec3Set(destPosition, -destPosition.f.x, -destPosition.f.y, zero);
        }
    }

    if (m_pNextClosestFormation != NULL && m_pNextClosestFormation->m_pFormationSpec != NULL)
    {
        m_pNextClosestFormation->CalculateDesiredLocation(v3NextPosition, pFielder, bExtrapolate);

        f32 blend = 1.0f;

        if (m_pNextClosestFormation != NULL)
        {
            const FormationSpec* pNextSpec = m_pNextClosestFormation->m_pFormationSpec;
            if (pNextSpec != NULL)
            {
                const FormationSpec* pSpec = m_pFormationSpec;
                nlVector3 v3AIBallLoc;

                if (g_pBall->m_pOwner != NULL)
                {
                    v3AIBallLoc = g_pBall->m_pOwner->m_v3Position;
                }
                else if (g_pBall->m_pPassTarget != NULL)
                {
                    v3AIBallLoc = g_pBall->m_pPassTarget->m_v3Position;
                }
                else
                {
                    v3AIBallLoc = g_pBall->m_v3Position;
                }

                if (m_pFormationManager->m_pTeam->m_nSide != 0)
                {
                    f32 negY = -v3AIBallLoc.f.y;
                    f32 negX = -v3AIBallLoc.f.x;
                    v3AIBallLoc.f.z = 0.0f;
                    v3AIBallLoc.f.x = negX;
                    v3AIBallLoc.f.y = negY;
                }

                nlVector2 vAIBallLoc = *(const nlVector2*)&v3AIBallLoc;
                nlVector2& keyLoc = pSpec->GetKeyLocation();
                f32 dyA = vAIBallLoc.f.y - keyLoc.f.y;
                f32 dxA = vAIBallLoc.f.x - keyLoc.f.x;
                f32 dist = nlSqrt(dxA * dxA + dyA * dyA, true);

                nlVector2& nextKeyLoc = pNextSpec->GetKeyLocation();
                f32 dyB = vAIBallLoc.f.y - nextKeyLoc.f.y;
                f32 dxB = vAIBallLoc.f.x - nextKeyLoc.f.x;
                f32 nextDist = nlSqrt(dxB * dxB + dyB * dyB, true);

                blend = nextDist / (dist + nextDist);
            }
        }

        f32 invBlend = 1.0f - blend;
        destPosition.f.x = invBlend * v3NextPosition.f.x + blend * destPosition.f.x;
        destPosition.f.y = invBlend * v3NextPosition.f.y + blend * destPosition.f.y;
        destPosition.f.z = invBlend * v3NextPosition.f.z + blend * destPosition.f.z;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80038250 | size: 0x2C0
 * TODO: 99.89% match - f29/f30 swap for fBlend register allocation in inner blend computation
 */
float FormationBallPosition::GetWeight()
{
    nlVector3 v3AIBallLoc;

    if (g_pBall->m_pOwner != NULL)
    {
        v3AIBallLoc = g_pBall->m_pOwner->m_v3Position;
    }
    else if (g_pBall->m_pPassTarget != NULL)
    {
        v3AIBallLoc = g_pBall->m_pPassTarget->m_v3Position;
    }
    else
    {
        v3AIBallLoc = g_pBall->m_v3Position;
    }

    if (m_pFormationManager->m_pTeam->m_nSide != 0)
    {
        float negY = -v3AIBallLoc.f.y;
        float negX = -v3AIBallLoc.f.x;
        v3AIBallLoc.f.z = 0.0f;
        v3AIBallLoc.f.x = negX;
        v3AIBallLoc.f.y = negY;
    }

    nlVector2 vAIBallLoc = *(const nlVector2*)&v3AIBallLoc;
    nlVector2& keyLoc = m_pFormationSpec->GetKeyLocation();
    float dy = vAIBallLoc.f.y - keyLoc.f.y;
    float dx = vAIBallLoc.f.x - keyLoc.f.x;
    float dist = nlSqrt(dx * dx + dy * dy, true);
    float fWeight = NormalizeVal(dist, m_pFormationSpec->m_OutRadius, 0.0f);

    if (m_pNextClosestFormation != NULL)
    {
        const FormationSpec* pNextSpec = m_pNextClosestFormation->m_pFormationSpec;
        if (pNextSpec != NULL)
        {
            float fBlend = 1.0f;

            if (m_pNextClosestFormation != NULL)
            {
                if (pNextSpec != NULL)
                {
                    const FormationSpec* pSpec = m_pFormationSpec;
                    nlVector3 v3AIBallLoc2;

                    if (g_pBall->m_pOwner != NULL)
                    {
                        v3AIBallLoc2 = g_pBall->m_pOwner->m_v3Position;
                    }
                    else if (g_pBall->m_pPassTarget != NULL)
                    {
                        v3AIBallLoc2 = g_pBall->m_pPassTarget->m_v3Position;
                    }
                    else
                    {
                        v3AIBallLoc2 = g_pBall->m_v3Position;
                    }

                    if (m_pFormationManager->m_pTeam->m_nSide != 0)
                    {
                        float negY = -v3AIBallLoc2.f.y;
                        float negX = -v3AIBallLoc2.f.x;
                        v3AIBallLoc2.f.z = 0.0f;
                        v3AIBallLoc2.f.x = negX;
                        v3AIBallLoc2.f.y = negY;
                    }

                    nlVector2 vAIBallLoc2 = *(const nlVector2*)&v3AIBallLoc2;
                    nlVector2& keyLoc2 = pSpec->GetKeyLocation();
                    float dy2 = vAIBallLoc2.f.y - keyLoc2.f.y;
                    float dx2 = vAIBallLoc2.f.x - keyLoc2.f.x;
                    fBlend = nlSqrt(dx2 * dx2 + dy2 * dy2, true);

                    nlVector2& nextKeyLoc = pNextSpec->GetKeyLocation();
                    float dy3 = vAIBallLoc2.f.y - nextKeyLoc.f.y;
                    float dx3 = vAIBallLoc2.f.x - nextKeyLoc.f.x;
                    float fDist2 = nlSqrt(dx3 * dx3 + dy3 * dy3, true);

                    fBlend = fDist2 / (fBlend + fDist2);
                }
            }

            float fNextWeight = m_pNextClosestFormation->GetWeight();
            fWeight = fWeight * fBlend + fNextWeight * (1.0f - fBlend);
        }
    }

    return fWeight;
}
