#include "Game/Effects/EmissionController.h"
#include "Game/Effects/ParticleSystem.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/SAnim/pnSAnimController.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlMemory.h"
#include "types.h"

extern int numLingeringSystems;

struct EmissionFunctor
{
    virtual ~EmissionFunctor() { };
    virtual void Invoke(EmissionController&) = 0;
    virtual FunctorBase* Clone() const = 0;
};

struct UserEffectSpecClone : public UserEffectSpec
{
    virtual UserEffectSpec* Clone() = 0;
};

struct EmptyCallback
{
    enum Tag mTag;
    union
    {
        void (*mFreeFunction)(EmissionController&);
        Function1<void, EmissionController&>::FunctorBase* mFunctor;
    };
};

/**
 * Offset/Address/Size: 0xF2C | 0x801F881C | size: 0x104
 */
EmissionController::EmissionController(EffectsGroup* pEffectsGroup, unsigned short id, eGLView view)
{
    m_nextNode = nullptr;
    m_prevNode = nullptr;
    m_Systems.m_headNode = nullptr;
    m_Systems.m_tailNode = nullptr;
    m_Systems.m_numNodes = 0;
    mUpdateCallback.mTag = EMPTY;
    mFinishedCallback.mTag = EMPTY;

    m_GlView = view;
    m_Replaying = false;
    m_Age = 0.0f;
    m_ReplayDeltaTime = 0.0f;
    m_Id = id;
    m_bPoseErrorDisplayed = false;
    m_pGroup = pEffectsGroup;
    m_Mirror.f.x = 1.0f;
    m_Mirror.f.y = 1.0f;
    m_Mirror.f.z = 1.0f;
    m_uUserData = 0;

    InitializeSystemsFromGroup();

    m_fGround = 0.015625f;
    m_aFacing = 0;
    m_vPosition.f.x = 0.0f;
    m_vPosition.f.y = 0.0f;
    m_vPosition.f.z = 0.0f;

    m_vDirection.f.x = 0.0f;
    m_vDirection.f.y = 0.0f;
    m_vDirection.f.z = 1.0f;

    m_vVelocity.f.x = 0.0f;
    m_vVelocity.f.y = 0.0f;
    m_vVelocity.f.z = 0.0f;
    m_pPose = nullptr;
    m_pAnimController = nullptr;
    m_uJointIDOverride = 0;
    m_bVisible = true;
    m_bDisabled = false;

    if (m_pGroup->m_isLingering != 0)
    {
        numLingeringSystems++;
    }

    EmissionManager::KillOldest(numLingeringSystems - 12, true);
}

/**
 * Offset/Address/Size: 0xD84 | 0x801F8674 | size: 0x1A8
 * TODO: 99.62% match - remaining register allocation differences in user effect clone loop (r29/r30 swap for pUserSpecs/ofs).
 * Target: r30=pUserSpecs, r29=ofs, r27=i. Current: r28=pUserSpecs, r30=ofs, r27=i.
 * Likely `-inline deferred` environment difference; exhaustive permutations tested.
 */
void EmissionController::InitializeSystemsFromGroup()
{
    EffectsSpec* pEndSpec;
    EffectsSpec* pSpec;
    EffectsSpec* pSpecs;
    pSpecs = m_pGroup->m_specs;
    pSpec = pSpecs;
    pEndSpec = pSpecs + m_pGroup->m_numSpecs;

    while (pSpec < pEndSpec)
    {
        if (pSpec->m_eAttach == FXBind_Joint && pSpec->m_uJointID == 0xFFFFFFFF)
        {
            pSpec++;
            continue;
        }

        EffectsTerrainSpec* pTerrain = pSpec->m_pTerrainSpec;
        u8 bUseThisSpec = true;

        if (pTerrain != NULL && fxGetTerrain != NULL)
        {
            if (!pTerrain->HasTerrain(fxGetTerrain()))
            {
                bUseThisSpec = false;
            }
        }

        if (bUseThisSpec)
        {
            ParticleSystem* pSys = new (nlMalloc(sizeof(ParticleSystem), 8, false)) ParticleSystem(pSpec->m_pTemplate, pSpec);

            pSys->m_fDelay = pSpec->m_fDelay;
            m_Systems.Append(pSys);
        }

        pSpec++;
    }

    m_fGround = 0.015625f;
    m_aFacing = 0;
    m_vPosition.f.x = 0.0f;
    m_vPosition.f.y = 0.0f;
    m_vPosition.f.z = 0.0f;
    m_vDirection.f.x = 0.0f;
    m_vDirection.f.y = 0.0f;
    m_vDirection.f.z = 1.0f;
    m_vVelocity.f.x = 0.0f;
    m_vVelocity.f.y = 0.0f;
    m_vVelocity.f.z = 0.0f;
    m_pPose = NULL;
    m_pAnimController = NULL;
    m_pUserEffects = NULL;
    m_nUserEffects = m_pGroup->m_userSpecs;

    if (m_nUserEffects > 0)
    {
        int i;
        int ofs;
        UserEffectSpec** pUserSpecs = m_pGroup->m_userSpecsPtr;
        m_pUserEffects = (UserEffectSpec**)nlMalloc(m_nUserEffects * sizeof(UserEffectSpec*), 8, false);

        i = 0;
        ofs = 0;
        while (i < m_nUserEffects)
        {
            m_pUserEffects[ofs] = ((UserEffectSpecClone*)pUserSpecs[0])->Clone();
            pUserSpecs++;
            i++;
            ofs++;
        }
    }
}

/**
 * Offset/Address/Size: 0xB30 | 0x801F8420 | size: 0x254
 */
EmissionController::~EmissionController()
{
    if (mFinishedCallback.mTag != EMPTY)
    {
        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction(*this);
        }
        else
        {
            ((EmissionFunctor*)mFinishedCallback.mFunctor)->Invoke(*this);
        }

        EmptyCallback empty;
        empty.mTag = EMPTY;

        if (mFinishedCallback.mTag == FUNCTOR)
        {
            delete mFinishedCallback.mFunctor;
        }

        mFinishedCallback.mTag = EMPTY;
        mFinishedCallback.mTag = empty.mTag;

        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction = empty.mFreeFunction;
        }
        else if (mFinishedCallback.mTag == FUNCTOR)
        {
            mFinishedCallback.mFunctor = empty.mFunctor->Clone();
        }

        if (empty.mTag == FUNCTOR)
        {
            delete empty.mFunctor;
        }
        *(volatile int*)&empty.mTag = EMPTY;
    }

    while (m_Systems.m_headNode != NULL)
    {
        ParticleSystem* pSys = (ParticleSystem*)m_Systems.Remove();
        delete pSys;
    }

    if (m_pUserEffects != NULL)
    {
        int i = 0;
        int ofs = 0;
        while (i < m_nUserEffects)
        {
            delete m_pUserEffects[ofs];
            ofs++;
            i++;
        }
        delete[] m_pUserEffects;
    }

    if (m_pGroup->m_isLingering != 0)
    {
        numLingeringSystems--;
    }
}

/**
 * Offset/Address/Size: 0xB14 | 0x801F8404 | size: 0x1C
 */
void EmissionController::SetPosition(const nlVector3& pos)
{
    m_vPosition = pos;
}

/**
 * Offset/Address/Size: 0xB0C | 0x801F83FC | size: 0x8
 */
const nlVector3& EmissionController::GetPosition() const
{
    return m_vPosition;
}

/**
 * Offset/Address/Size: 0xAF0 | 0x801F83E0 | size: 0x1C
 */
void EmissionController::SetDirection(const nlVector3& dir)
{
    m_vDirection = dir;
}

/**
 * Offset/Address/Size: 0xAD4 | 0x801F83C4 | size: 0x1C
 */
void EmissionController::SetVelocity(const nlVector3& velocity)
{
    m_vVelocity = velocity;
}

/**
 * Offset/Address/Size: 0xACC | 0x801F83BC | size: 0x8
 */
void EmissionController::SetPoseAccumulator(const cPoseAccumulator& pose)
{
    m_pPose = &pose;
}

/**
 * Offset/Address/Size: 0xAC4 | 0x801F83B4 | size: 0x8
 */
void EmissionController::SetAnimController(const cPN_SAnimController& animController)
{
    m_pAnimController = &animController;
}

/**
 * Offset/Address/Size: 0x988 | 0x801F8278 | size: 0x13C
 */
void EmissionController::Die()
{
    ParticleSystem* p = (ParticleSystem*)m_Systems.m_headNode;
    while (p != NULL)
    {
        p->Die();
        p = (ParticleSystem*)p->m_nextNode;
    }

    if (mFinishedCallback.mTag != EMPTY)
    {
        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction(*this);
        }
        else
        {
            ((EmissionFunctor*)mFinishedCallback.mFunctor)->Invoke(*this);
        }

        EmptyCallback empty;
        empty.mTag = EMPTY;

        if (mFinishedCallback.mTag == FUNCTOR)
        {
            delete mFinishedCallback.mFunctor;
        }

        mFinishedCallback.mTag = EMPTY;
        mFinishedCallback.mTag = empty.mTag;

        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction = empty.mFreeFunction;
        }
        else if (mFinishedCallback.mTag == FUNCTOR)
        {
            mFinishedCallback.mFunctor = empty.mFunctor->Clone();
        }

        if (empty.mTag == FUNCTOR)
        {
            delete empty.mFunctor;
        }
        *(volatile int*)&empty.mTag = EMPTY;
    }
}

/**
 * Offset/Address/Size: 0x924 | 0x801F8214 | size: 0x64
 */
float EmissionController::GetRemainingTime() const
{
    float maxTime = 0.0f;
    efBaseNode* node = m_Systems.m_headNode;

    while (node != nullptr)
    {
        ParticleSystem* system = (ParticleSystem*)node;
        float remainingTime = system->GetRemainingTime();
        if (remainingTime > maxTime)
        {
            maxTime = remainingTime;
        }
        node = node->m_nextNode;
    }

    return maxTime;
}

/**
 * Offset/Address/Size: 0x918 | 0x801F8208 | size: 0xC
 */
bool EmissionController::IsLingering() const
{
    return m_pGroup->m_isLingering;
}

/**
 * Offset/Address/Size: 0x32C | 0x801F7C1C | size: 0x5EC
 * TODO: 96.97% match - register allocation diffs (r26/r30 numSys, r27/r29 numDel, r29/r26 pSpec,
 * r30/r27 pNext, r20/r24 pTerrain, f1/f2 float swap).
 */
bool EmissionController::Update(float dt)
{
    if (m_bDisabled)
    {
        return true;
    }

    int numSys = 0;
    int numDel = 0;

    if (m_Replaying)
    {
        dt = m_ReplayDeltaTime;
    }
    else
    {
        m_Age += dt;
    }

    if (mUpdateCallback.mTag != EMPTY)
    {
        if (mUpdateCallback.mTag == FREE_FUNCTION)
        {
            mUpdateCallback.mFreeFunction(*this);
        }
        else
        {
            ((EmissionFunctor*)mUpdateCallback.mFunctor)->Invoke(*this);
        }
    }

    if (dt <= 0.0f)
    {
        return true;
    }

    ParticleSystem* pSys = (ParticleSystem*)m_Systems.m_headNode;

    while (pSys != NULL)
    {
        ParticleSystem* pNext = (ParticleSystem*)pSys->m_nextNode;

        pSys->m_aFacing = m_aFacing;

        EffectsSpec* pSpec = pSys->m_pSpec;
        EffectsTerrainSpec* pTerrain = pSpec->m_pTerrainSpec;

        if (pTerrain != NULL)
        {
            if (fxGetTerrain() != 0)
            {
                if (!pTerrain->HasTerrain(fxGetTerrain()))
                {
                    pSys = pNext;
                    continue;
                }
            }
        }

        numSys++;
        pSys->m_uLayer = pSpec->m_uLayer;

        nlVector3 pos = m_vPosition;
        nlVector3 vel = m_vVelocity;

        if (pSpec->m_eAttach == FXBind_Joint)
        {
            if (pSpec->m_eJointBinding == JB_Ascend && m_pPose != NULL)
            {
                u32 jointID = m_uJointIDOverride;
                float fAge = m_Age;
                vel.f.x = 0.0f;
                float fJointVelocity = pSpec->m_fJointVelocity;
                vel.f.y = 0.0f;
                vel.f.z = 0.0f;

                if (jointID == 0)
                {
                    jointID = pSpec->m_uJointID;
                }

                const cPoseAccumulator* pPose = m_pPose;
                float fDist = fAge * fJointVelocity;
                cSHierarchy* pHier = pPose->m_BaseSHierarchy;

                int jointIndex = pHier->GetNodeIndexByID(jointID);
                int parentIndex = pHier->GetParent(jointIndex);

                while (parentIndex != -1)
                {
                    const nlMatrix4& currentMat = pPose->GetNodeMatrix(jointIndex);
                    const nlMatrix4& parentMat = pPose->GetNodeMatrix(parentIndex);

                    float dy = parentMat.m[3][1] - currentMat.m[3][1];
                    float dx = parentMat.m[3][0] - currentMat.m[3][0];
                    float dz = parentMat.m[3][2] - currentMat.m[3][2];
                    float dist = nlSqrt(dx * dx + dy * dy + dz * dz, true);

                    if (dist >= fDist)
                    {
                        float ratio = fDist / dist;
                        float invRatio = 1.0f - ratio;
                        pos.f.x = ratio * parentMat.m[3][0] + invRatio * currentMat.m[3][0];
                        pos.f.y = ratio * parentMat.m[3][1] + invRatio * currentMat.m[3][1];
                        pos.f.z = ratio * parentMat.m[3][2] + invRatio * currentMat.m[3][2];
                        break;
                    }

                    fDist -= dist;
                    jointIndex = parentIndex;
                    parentIndex = pHier->GetParent(parentIndex);
                }

                if (parentIndex == -1)
                {
                    const nlMatrix4& mat = pPose->GetNodeMatrix(jointIndex);
                    pos.as_u32[0] = ((u32*)&mat.m[3][0])[0];
                    pos.as_u32[1] = ((u32*)&mat.m[3][0])[1];
                    pos.as_u32[2] = ((u32*)&mat.m[3][0])[2];
                }
            }
            else if (m_pPose != NULL)
            {
                u32 jointID = pSpec->m_uJointID;

                if (m_pAnimController != NULL && m_pAnimController->m_bMirror)
                {
                    cSHierarchy* pHier = m_pPose->m_BaseSHierarchy;
                    int nodeIndex = pHier->GetNodeIndexByID(jointID);
                    int mirroredIndex = pHier->GetMirroredNode(nodeIndex);
                    jointID = pHier->GetNodeID(mirroredIndex);
                }

                u32 finalJointID = m_uJointIDOverride;
                if (finalJointID == 0)
                {
                    finalJointID = jointID;
                }

                const nlMatrix4& mat = m_pPose->GetNodeMatrixByHashID(finalJointID);
                pos.as_u32[0] = ((u32*)&mat.m[3][0])[0];
                pos.as_u32[1] = ((u32*)&mat.m[3][0])[1];
                pos.as_u32[2] = ((u32*)&mat.m[3][0])[2];
            }
            else
            {
                if (!m_bPoseErrorDisplayed)
                {
                    EmissionManager::AddError("No Pose Buffer To Play Effect - playing at default position");
                    m_bPoseErrorDisplayed = true;
                }
            }
        }

        if (pSpec->m_bGround)
        {
            pos.f.z = m_fGround;
        }

        pos.f.z += pSpec->m_fOffset;

        pSys->m_vPosition = pos;
        pSys->m_vVelocity = vel;
        pSys->m_vForward = m_vDirection;
        pSys->m_Mirror = m_Mirror;

        pSys->UpdateCoordSys();

        pSys->m_bVisible = m_bVisible;

        if (!pSys->Update(dt))
        {
            m_Systems.Remove(pSys);
            delete pSys;
            numDel++;
        }

        pSys = pNext;
    }

    u8 isFinished = (numSys == numDel);

    if (isFinished && mFinishedCallback.mTag != EMPTY)
    {
        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction(*this);
        }
        else
        {
            ((EmissionFunctor*)mFinishedCallback.mFunctor)->Invoke(*this);
        }

        EmptyCallback empty;
        empty.mTag = EMPTY;

        if (mFinishedCallback.mTag == FUNCTOR)
        {
            delete mFinishedCallback.mFunctor;
        }

        mFinishedCallback.mTag = EMPTY;
        mFinishedCallback.mTag = empty.mTag;

        if (mFinishedCallback.mTag == FREE_FUNCTION)
        {
            mFinishedCallback.mFreeFunction = empty.mFreeFunction;
        }
        else if (mFinishedCallback.mTag == FUNCTOR)
        {
            mFinishedCallback.mFunctor = empty.mFunctor->Clone();
        }

        if (empty.mTag == FUNCTOR)
        {
            delete empty.mFunctor;
        }
        *(volatile int*)&empty.mTag = EMPTY;
    }

    if (m_nUserEffects > 0)
    {
        UserEffectInfo info;
        info.pv3Position = &m_vPosition;
        info.pv3Direction = &m_vDirection;

        int i = 0;
        int ofs = 0;
        while (i < m_nUserEffects)
        {
            m_pUserEffects[ofs]->Update(dt, &info);
            isFinished = isFinished & m_pUserEffects[ofs]->IsFinished();
            ofs++;
            i++;
        }
    }

    return !isFinished;
}

/**
 * Offset/Address/Size: 0x288 | 0x801F7B78 | size: 0xA4
 */
void* fxLoadEntireFileHigh(const char* filename, unsigned long* fileSize)
{
    void* buffer = nullptr;
    u32 datasize = 0;

    nlFile* file = nlOpen(filename);
    if (file != nullptr)
    {
        unsigned int size;
        datasize = nlFileSize(file, &size);
        buffer = nlMalloc(size, 0x20, true);
        nlRead(file, buffer, datasize);
        nlClose(file);
    }

    if (fileSize != nullptr)
    {
        *fileSize = datasize;
    }

    return buffer;
}

/**
 * Offset/Address/Size: 0x150 | 0x801F7A40 | size: 0x138
 */
void EmissionController::Render()
{
    struct UserEffectInfo
    {
        nlVector3* pv3Position;
        nlVector3* pv3Direction;
    };

    if (m_bDisabled)
    {
        return;
    }

    ParticleSystem* sys = (ParticleSystem*)m_Systems.m_headNode;
    f32 lingerThreshold = 0.0f;

    while (sys != NULL)
    {
        EffectsSpec* spec = sys->m_pSpec;
        bool isLingering = (spec == NULL || spec->m_fLingerStart > lingerThreshold);

        s32 view;
        if (isLingering)
        {
            view = 0x12;
        }
        else
        {
            view = m_GlView;
        }

        if (sys->m_bVisible)
        {
            sys->RenderAllParticles((eGLView)view);
        }

        sys = (ParticleSystem*)sys->m_nextNode;
    }

    if (m_nUserEffects > 0)
    {
        UserEffectInfo info;
        info.pv3Position = &m_vPosition;
        info.pv3Direction = &m_vDirection;
        s32 i = 0;
        s32 j = 0;
        while (i < m_nUserEffects)
        {
            if (!m_pUserEffects[j]->IsFinished())
            {
                s8 view = m_GlView;
                m_pUserEffects[j]->Render((const nlVector3**)&info, (s32)view);
            }
            j++;
            i++;
        }
    }
}

/**
 * Offset/Address/Size: 0xA8 | 0x801F7998 | size: 0xA8
 */
void EmissionController::SetUpdateCallback(const Function1<void, EmissionController&>& callback)
{
    if (mUpdateCallback.mTag == FUNCTOR)
    {
        delete mUpdateCallback.mFunctor;
    }

    mUpdateCallback.mTag = EMPTY;
    mUpdateCallback.mTag = callback.mTag;

    if (mUpdateCallback.mTag == FREE_FUNCTION)
    {
        mUpdateCallback.mFreeFunction = callback.mFreeFunction;
    }
    else if (mUpdateCallback.mTag == FUNCTOR)
    {
        mUpdateCallback.mFunctor = callback.mFunctor->Clone();
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801F78F0 | size: 0xA8
 */
void EmissionController::SetFinishedCallback(const Function1<void, EmissionController&>& callback)
{
    if (mFinishedCallback.mTag == FUNCTOR)
    {
        delete mFinishedCallback.mFunctor;
    }

    mFinishedCallback.mTag = EMPTY;
    mFinishedCallback.mTag = callback.mTag;

    if (mFinishedCallback.mTag == FREE_FUNCTION)
    {
        mFinishedCallback.mFreeFunction = callback.mFreeFunction;
    }
    else if (mFinishedCallback.mTag == FUNCTOR)
    {
        mFinishedCallback.mFunctor = callback.mFunctor->Clone();
    }
}
