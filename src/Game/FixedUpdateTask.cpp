#include "Game/FixedUpdateTask.h"

#include "Game/Ball.h"
#include "Game/BasicStadium.h"
#include "Game/CharacterTemplate.h"
#include "Game/Field.h"
#include "Game/FlickDetection.h"
#include "Game/Game.h"
#include "Game/Physics/Physics.h"
#include "Game/Render/NetMesh.h"
#include "Game/ReplayManager.h"
#include "Game/Team.h"
#include "NL/platpad.h"

extern float g_fFixedUpdateTick;
extern PhysicsWorld* g_PhysicsWorld;

class SidelineExplodableManager
{
public:
    static void Update(float);
};

static bool g_bRunSimAndRenderInLockStep;
static float g_fSimulationTick = g_fFixedUpdateTick;

float FixedUpdateTask::mAccumulatedDeltaT;
float FixedUpdateTask::mSimulationTime;
float FixedUpdateTask::mfFrameLockTime;

/**
 * Offset/Address/Size: 0x2D8 | 0x8016E608 | size: 0x30
 */
FixedUpdateTask::FixedUpdateTask()
{
    mAccumulatedDeltaT = g_fFixedUpdateTick;
    mSimulationTime = 0.f;
    mfFrameLockTime = 0.f;
}

/**
 * Offset/Address/Size: 0x2CC | 0x8016E5FC | size: 0xC
 */
const char* FixedUpdateTask::GetName()
{
    return "Game Fixed Update";
}

/**
 * Offset/Address/Size: 0x0 | 0x8016E330 | size: 0x280
 * TODO: 97.75% match - remaining diffs are register allocation/scheduling:
 * g_pCharacters/g_pTeams base register assignment, fixedTick f30/f1 load order,
 * and NetMesh update argument materialization order.
 */
void FixedUpdateTask::Run(float dt)
{
    if (nlTaskManager::m_pInstance->m_CurrState == 2)
    {
        cTeam** pTeams = g_pTeams;
        cCharacter** pCharacter;
        int i;
        float simulationTick;

        mAccumulatedDeltaT += dt * mTimeScale;

        while (g_bRunSimAndRenderInLockStep || mAccumulatedDeltaT >= g_fFixedUpdateTick)
        {
            UseFixedUpdatePad();

            simulationTick = g_fFixedUpdateTick;
            UpdatePlatPad(simulationTick);
            cPadManager::Update(simulationTick);
            FlickDetection::Update();

            mAccumulatedDeltaT -= g_fFixedUpdateTick;
            if (g_bRunSimAndRenderInLockStep)
            {
                mAccumulatedDeltaT = 0.0f;
            }

            simulationTick = g_fSimulationTick;
            mSimulationTime += simulationTick;
            g_pGame->PreUpdate(simulationTick);
            g_pGame->Update(simulationTick);
            BasicStadium::GetCurrentStadium()->mpNPCManager->UpdateAINPCs(g_fSimulationTick);

            simulationTick = g_fSimulationTick;
            pCharacter = g_pCharacters;
            for (i = 0; i < 10; i++)
            {
                (*pCharacter)->PrePhysicsUpdate(simulationTick);
                pCharacter++;
            }

            PhysicsUpdate(g_PhysicsWorld, g_fSimulationTick);

            simulationTick = g_fSimulationTick;
            pCharacter = g_pCharacters;
            for (i = 0; i < 10; i++)
            {
                (*pCharacter)->PostPhysicsUpdate();
                pCharacter++;
            }

            g_pBall->PostPhysicsUpdate(simulationTick);

            if (NetMesh::s_bAnimatedNetMeshEnabled)
            {
                bool doGoalieNetTestPosX = true;
                float goalieX = (float)fabs(g_pTeams[0]->GetGoalie()->m_v3Position.f.x);
                if (goalieX > cField::GetGoalLineX(1U))
                {
                }
                else
                {
                    goalieX = (float)fabs(pTeams[1]->GetGoalie()->m_v3Position.f.x);
                    if (goalieX > cField::GetGoalLineX(1U))
                    {
                    }
                    else
                    {
                        doGoalieNetTestPosX = false;
                    }
                }

                NetMesh::spPositiveXNetMesh->Update(g_fSimulationTick, g_pBall->m_v3Position, g_pBall->m_v3PrevPosition, doGoalieNetTestPosX, g_pBall->m_pPhysicsBall);
                NetMesh::spNegativeXNetMesh->Update(g_fSimulationTick, g_pBall->m_v3Position, g_pBall->m_v3PrevPosition, doGoalieNetTestPosX, g_pBall->m_pPhysicsBall);
            }

            SidelineExplodableManager::Update(g_fSimulationTick);
            ReplayManager::Instance()->GrabSnapshot();

            if (g_bRunSimAndRenderInLockStep)
            {
                break;
            }
        }
    }

    UseDefaultPad();
    UpdatePlatPad(dt);
    cPadManager::Update(dt);
    FlickDetection::Update();
}

/**
 * Offset/Address/Size: 0x2C4 | 0x8016E5F4 | size: 0x8
 */
float FixedUpdateTask::GetPhysicsUpdateTick()
{
    return g_fSimulationTick;
}

/**
 * Offset/Address/Size: 0x280 | 0x8016E5B0 | size: 0x44
 */
void FixedUpdateTask::DecrementFrameLock(float f)
{
    float temp_f1 = mfFrameLockTime - f;
    mfFrameLockTime = temp_f1;
    if (temp_f1 < 0.f)
    {
        nlTaskManager::SetNextState(2);
        mfFrameLockTime = 0.f;
    }
}
