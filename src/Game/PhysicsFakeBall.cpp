#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Physics/PhysicsPlane.h"
#include "Game/Physics/PhysicsGroundPlane.h"
#include "Game/Physics/Physics.h"
#include "Game/FixedUpdateTask.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListSlotPool.h"

class SimpleCollisionSpace : public CollisionSpace
{
public:
    SimpleCollisionSpace(PhysicsWorld*);
    virtual ~SimpleCollisionSpace() { };
};

/**
 * Offset/Address/Size: 0x60 | 0x8013744C | size: 0x38
 */
ContactType FakePhysicsBall::Contact(PhysicsObject* object, dContact* contact, int arg)
{
    if (mWorld.mbHitSuccess)
    {
        return NO_CONTACT;
    }

    return PhysicsBall::Contact(object, contact, arg);
}

/**
 * Offset/Address/Size: 0x98 | 0x80137484 | size: 0x3EC
 * TODO: 90.27% match - remaining diffs are MWCC register allocation:
 *       GPR shift (stmw r23 vs r22, all callee-saved GPRs +2),
 *       FPR swap (f27/f28/f29 assignments for fPlayerReach/fMaxTime/playerPosX),
 *       and pre-loop instruction ordering for bciPool/cacheList address computation.
 */
bool FakeBallWorld::FindBallIntercept(const nlVector3& v3PlayerPos, float fPlayerReach, float fPlayerSpeed, nlVector3& v3InterceptPos, nlVector3& v3InterceptVel, float& fInterceptTime, float& fClosestDist, float fMaxTime)
{
    fInterceptTime = 0.0f;
    fClosestDist = 100000.0f;
    unsigned char bDone = 0;

    float fPlayerDistPerTick = fPlayerSpeed * FixedUpdateTask::GetPhysicsUpdateTick();

    nlVector3 v3NewBallPos;
    nlVector3 v3NewBallVel;
    nlVector3 v3TempVel;
    nlVector3 v3TempPos;
    GetPredictedBallPosition(0.0f, v3TempPos, v3TempVel);

    struct BallCacheIterator
    {
        DLListEntry<BallCacheInfo*>* m_head;
        DLListEntry<BallCacheInfo*>* m_current;
        BallCacheIterator(DLListEntry<BallCacheInfo*>* head, DLListEntry<BallCacheInfo*>* current)
            : m_head(head)
            , m_current(current)
        {
        }
    };

    static BallCacheIterator iter(mBallCacheList.m_Head, nlDLRingGetStart(mBallCacheList.m_Head));

    iter.m_current = nlDLRingGetStart(mBallCacheList.m_Head);
    iter.m_head = mBallCacheList.m_Head;
    mpCacheIterator = reinterpret_cast<nlDLListIterator<DLListEntry<BallCacheInfo*> >*>(&iter);

    if (mpCacheIterator->m_current != NULL)
    {
        if (nlDLRingIsEnd(mpCacheIterator->m_head, mpCacheIterator->m_current) || iter.m_current == NULL)
        {
            iter.m_current = NULL;
        }
        else
        {
            iter.m_current = iter.m_current->m_next;
        }
    }

    float fPlayerDistanceFromStartingPoint = fPlayerReach;
    float playerPosX = v3PlayerPos.f.x;
    float playerPosY = v3PlayerPos.f.y;

    SlotPool<BallCacheInfo>* bciPool = &BallCacheInfo::mBallCacheInfoSlotPool;
    nlDLListSlotPool<BallCacheInfo*>* cacheList = &mBallCacheList;

    while (!bDone)
    {
        nlDLListIterator<DLListEntry<BallCacheInfo*> >* cacheIter = mpCacheIterator;

        if (cacheIter->m_current != NULL)
        {
            BallCacheInfo* info = cacheIter->m_current->m_data;
            v3NewBallPos = info->mv3Position;
            v3NewBallVel = info->mv3LinearVelocity;

            if (nlDLRingIsEnd(cacheIter->m_head, cacheIter->m_current) || cacheIter->m_current == NULL)
            {
                cacheIter->m_current = NULL;
            }
            else
            {
                cacheIter->m_current = cacheIter->m_current->m_next;
            }
        }
        else
        {
            float tick = FixedUpdateTask::GetPhysicsUpdateTick();
            FakeBallWorld* predictWorld = mpPredictWorld;
            PhysicsUpdate(predictWorld->mpPhysicsWorld, tick);

            predictWorld = mpPredictWorld;
            mfLastCacheTime += tick;
            BallCacheInfo* newInfo = NULL;
            PhysicsObject* physObj = predictWorld->mpPhysicsBall;

            if (bciPool->m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)bciPool, sizeof(BallCacheInfo));
            }
            if (bciPool->m_FreeList != NULL)
            {
                newInfo = (BallCacheInfo*)bciPool->m_FreeList;
                bciPool->m_FreeList = bciPool->m_FreeList->m_next;
            }

            newInfo->mfTime = mfLastCacheTime;
            newInfo->mv3Position = physObj->GetPosition();
            newInfo->mv3LinearVelocity = physObj->GetLinearVelocity();

            DLListEntry<BallCacheInfo*>* newEntry = NULL;
            if (cacheList->m_Allocator.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&cacheList->m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
            }
            if (cacheList->m_Allocator.m_FreeList != NULL)
            {
                newEntry = (DLListEntry<BallCacheInfo*>*)cacheList->m_Allocator.m_FreeList;
                cacheList->m_Allocator.m_FreeList = cacheList->m_Allocator.m_FreeList->m_next;
            }

            if (newEntry != NULL)
            {
                newEntry->m_next = NULL;
                newEntry->m_prev = NULL;
                newEntry->m_data = newInfo;
            }

            nlDLRingAddEnd(&cacheList->m_Head, newEntry);

            v3NewBallPos = newInfo->mv3Position;
            v3NewBallVel = newInfo->mv3LinearVelocity;
        }

        float dx = v3NewBallPos.f.x - playerPosX;
        float dy = v3NewBallPos.f.y - playerPosY;
        float dist = nlSqrt(dx * dx + dy * dy, true);
        float adjustedDist = fabsf(dist - fPlayerDistanceFromStartingPoint);

        if (adjustedDist >= fClosestDist)
        {
            bDone = 1;
        }
        else
        {
            v3InterceptPos = v3NewBallPos;
            v3InterceptVel = v3NewBallVel;
            fClosestDist = adjustedDist;
        }

        fPlayerDistanceFromStartingPoint += fPlayerDistPerTick;
        fInterceptTime += FixedUpdateTask::GetPhysicsUpdateTick();

        if (fInterceptTime >= fMaxTime)
        {
            bDone = 1;
        }
    }

    return fInterceptTime < fMaxTime;
}

/**
 * Offset/Address/Size: 0x484 | 0x80137870 | size: 0x1E4
 * TODO: 99.75% match - remaining diff is register allocation for iterator
 *       (r28/r29 swap) in the early cache-hit branch.
 */
void FakeBallWorld::GetNextBallPosition(nlVector3& v3BallPos)
{
    if (mpCacheIterator->m_current != NULL)
    {
        DLListEntry<BallCacheInfo*>* entry = mpCacheIterator->m_current;
        BallCacheInfo* info = entry->m_data;
        v3BallPos = info->mv3Position;

        nlDLListIterator<DLListEntry<BallCacheInfo*> >* iter = mpCacheIterator;
        if (nlDLRingIsEnd(iter->m_head, iter->m_current) || iter->m_current == NULL)
        {
            iter->m_current = NULL;
        }
        else
        {
            iter->m_current = iter->m_current->m_next;
        }
        return;
    }

    float tick = FixedUpdateTask::GetPhysicsUpdateTick();
    FakeBallWorld* predictWorld = mpPredictWorld;
    PhysicsUpdate(predictWorld->mpPhysicsWorld, tick);

    SlotPool<BallCacheInfo>* bciPool = &BallCacheInfo::mBallCacheInfoSlotPool;
    predictWorld = mpPredictWorld;
    mfLastCacheTime += tick;
    BallCacheInfo* newInfo = NULL;
    PhysicsObject* physObj = predictWorld->mpPhysicsBall;

    if (bciPool->m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)bciPool, sizeof(BallCacheInfo));
    }
    if (bciPool->m_FreeList != NULL)
    {
        newInfo = (BallCacheInfo*)bciPool->m_FreeList;
        bciPool->m_FreeList = bciPool->m_FreeList->m_next;
    }

    newInfo->mfTime = mfLastCacheTime;
    newInfo->mv3Position = physObj->GetPosition();

    newInfo->mv3LinearVelocity = physObj->GetLinearVelocity();
    nlDLListSlotPool<BallCacheInfo*>* cacheList = &mBallCacheList;
    DLListEntry<BallCacheInfo*>* newEntry = NULL;

    if (cacheList->m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&cacheList->m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
    }
    if (cacheList->m_Allocator.m_FreeList != NULL)
    {
        newEntry = (DLListEntry<BallCacheInfo*>*)cacheList->m_Allocator.m_FreeList;
        cacheList->m_Allocator.m_FreeList = cacheList->m_Allocator.m_FreeList->m_next;
    }

    if (newEntry != NULL)
    {
        newEntry->m_next = NULL;
        newEntry->m_prev = NULL;
        newEntry->m_data = newInfo;
    }

    nlDLRingAddEnd(&mBallCacheList.m_Head, newEntry);

    v3BallPos = newInfo->mv3Position;
}

/**
 * Offset/Address/Size: 0x668 | 0x80137A54 | size: 0xC8
 * TODO: 99.4% match - remaining diffs are static-local symbol numbering
 *       (iter/init labels) and float literal label numbering.
 */
void FakeBallWorld::ResetBallIterator()
{
    nlVector3 v3Position;
    nlVector3 v3Velocity;

    GetPredictedBallPosition(0.0f, v3Position, v3Velocity);

    struct BallCacheIterator
    {
        DLListEntry<BallCacheInfo*>* m_head;
        DLListEntry<BallCacheInfo*>* m_current;

        BallCacheIterator(DLListEntry<BallCacheInfo*>* head, DLListEntry<BallCacheInfo*>* current)
            : m_head(head)
            , m_current(current)
        {
        }
    };

    static BallCacheIterator iter(mBallCacheList.m_Head, nlDLRingGetStart(mBallCacheList.m_Head));

    iter.m_current = nlDLRingGetStart(mBallCacheList.m_Head);
    iter.m_head = mBallCacheList.m_Head;
    mpCacheIterator = reinterpret_cast<nlDLListIterator<DLListEntry<BallCacheInfo*> >*>(&iter);

    if (mpCacheIterator->m_current != NULL)
    {
        if (nlDLRingIsEnd(mpCacheIterator->m_head, mpCacheIterator->m_current) || iter.m_current == NULL)
        {
            iter.m_current = NULL;
        }
        else
        {
            iter.m_current = iter.m_current->m_next;
        }
    }
}

/**
 * Offset/Address/Size: 0x730 | 0x80137B1C | size: 0x80
 */
FakePhysicsBall::~FakePhysicsBall()
{
}

/**
 * Offset/Address/Size: 0x7B0 | 0x80137B9C | size: 0x600
 */
void FakeBallWorld::GetPredictedPosAtDistance(float, nlVector3&, nlVector3&)
{
}

/**
 * Offset/Address/Size: 0xDB0 | 0x8013819C | size: 0x3FC
 * TODO: 87.03% match - remaining diffs are MWCC register allocation drift and
 *       speed-squared instruction shape (fmadds vs fmuls/fadds) in the opening block.
 */
float FakeBallWorld::GetPredictedHeightLimitTime(float fHeightLimit, float fTime, nlVector3& v3ContactPoint, nlVector3& v3ContactVelocity, bool bRequireDownward)
{
    extern nlVector3 v3Zero;

    cBall* pBall = mpPredictWorld->mpBall;

    float speedSq = pBall->m_v3Velocity.f.x * pBall->m_v3Velocity.f.x;
    speedSq += pBall->m_v3Velocity.f.y * pBall->m_v3Velocity.f.y;
    speedSq += pBall->m_v3Velocity.f.z * pBall->m_v3Velocity.f.z;

    if (speedSq < 0.0001f)
    {
        v3ContactPoint = pBall->m_v3Position;
        v3ContactVelocity = v3Zero;
        return fTime;
    }

    bool hasPredictedPos = GetPredictedBallPosition(fTime, v3ContactPoint, v3ContactVelocity);
    if (v3ContactPoint.f.z <= fHeightLimit)
    {
        if (bRequireDownward == false || v3ContactVelocity.f.z <= 0.0f)
        {
            return fTime;
        }
    }

    if (hasPredictedPos == false)
    {
        return -2.0f;
    }

    float fTick = FixedUpdateTask::GetPhysicsUpdateTick();
    float fSimulationTime = FixedUpdateTask::mSimulationTime;
    float fStartTime = fSimulationTime + fTime;
    float fPrevZVel = 0.0f;

    DLListEntry<BallCacheInfo*>* pHead = mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(pHead);

    while (pEntry)
    {
        BallCacheInfo* pInfo = pEntry->m_data;

        if (pInfo->mfTime >= fStartTime)
        {
            float zPos = pInfo->mv3Position.f.z;
            float zVel = pInfo->mv3LinearVelocity.f.z;

            if ((zPos <= fHeightLimit && (bRequireDownward == false || zVel <= 0.0f)) || (fPrevZVel < 0.0f && zVel > 0.0f))
            {
                v3ContactPoint = pInfo->mv3Position;
                v3ContactVelocity = pInfo->mv3LinearVelocity;
                return pInfo->mfTime - fSimulationTime;
            }

            fPrevZVel = zVel;
        }

        if (nlDLRingIsEnd(pHead, pEntry) || pEntry == NULL)
        {
            pEntry = NULL;
        }
        else
        {
            pEntry = pEntry->m_next;
        }
    }

    float fMaxTime = fSimulationTime + 6.0f;

    while (mfLastCacheTime < fMaxTime)
    {
        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fTick);
        mfLastCacheTime += fTick;

        SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;
        BallCacheInfo* pNewInfo = NULL;

        if (pBCIPool->m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)pBCIPool, sizeof(BallCacheInfo));
        }
        if (pBCIPool->m_FreeList)
        {
            pNewInfo = (BallCacheInfo*)pBCIPool->m_FreeList;
            pBCIPool->m_FreeList = pBCIPool->m_FreeList->m_next;
        }

        PhysicsObject* pPhysObj = mpPredictWorld->mpPhysicsBall;
        pNewInfo->mfTime = mfLastCacheTime;
        pNewInfo->mv3Position = pPhysObj->GetPosition();
        pNewInfo->mv3LinearVelocity = pPhysObj->GetLinearVelocity();

        nlDLListSlotPool<BallCacheInfo*>* pCacheList = &mBallCacheList;
        DLListEntry<BallCacheInfo*>* pNewEntry = NULL;

        if (pCacheList->m_Allocator.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&pCacheList->m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
        }
        if (pCacheList->m_Allocator.m_FreeList)
        {
            pNewEntry = (DLListEntry<BallCacheInfo*>*)pCacheList->m_Allocator.m_FreeList;
            pCacheList->m_Allocator.m_FreeList = pCacheList->m_Allocator.m_FreeList->m_next;
        }

        if (pNewEntry)
        {
            pNewEntry->m_next = NULL;
            pNewEntry->m_prev = NULL;
            pNewEntry->m_data = pNewInfo;
        }

        nlDLRingAddEnd(&pCacheList->m_Head, pNewEntry);

        float zPos = pNewInfo->mv3Position.f.z;
        float zVel = pNewInfo->mv3LinearVelocity.f.z;

        if ((zPos <= fHeightLimit && (bRequireDownward == false || zVel <= 0.0f)) || (fPrevZVel < 0.0f && zVel > 0.0f))
        {
            v3ContactPoint = pNewInfo->mv3Position;
            v3ContactVelocity = pNewInfo->mv3LinearVelocity;
            return pNewInfo->mfTime - fSimulationTime;
        }

        fPrevZVel = zVel;
    }

    return -1.0f;
}

/**
 * Offset/Address/Size: 0x11AC | 0x80138598 | size: 0x5BC
 */
float FakeBallWorld::GetPredictedPlaneIntersectTime(const nlVector4& plane, nlVector3& v3ContactPoint, nlVector3& v3ContactVelocity)
{
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x1768 | 0x80138B54 | size: 0x6C0
 */
bool FakeBallWorld::GetPredictedBallPosition(float, nlVector3&, nlVector3&)
{
    FORCE_DONT_INLINE;
    return false;
}

typedef DLListContainerBase<BallCacheInfo*, BasicSlotPool<DLListEntry<BallCacheInfo*> > > BallCacheListBase;

/**
 * Offset/Address/Size: 0x1E28 | 0x80139214 | size: 0xEC
 */
void FakeBallWorld::InvalidateBallCache()
{
    if (mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* end = mBallCacheList.m_Head;
        DLListEntry<BallCacheInfo*>* current = start;

        while (current != NULL)
        {
            BallCacheInfo* data = current->m_data;
            ((SlotPoolEntry*)data)->m_next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
            BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList = (SlotPoolEntry*)data;

            if (nlDLRingIsEnd(end, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }
        }

        nlWalkDLRing<DLListEntry<BallCacheInfo*>, BallCacheListBase>(
            mBallCacheList.m_Head,
            (BallCacheListBase*)&mBallCacheList,
            (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
        mBallCacheList.m_Head = NULL;
    }

    mfLastCacheTime = -1.0f;
}

/**
 * Offset/Address/Size: 0x1F14 | 0x80139300 | size: 0x198
 * TODO: 99.66% match - world ptr in r30 vs r28 (MWCC register allocator quirk)
 */
void FakeBallWorld::Destroy()
{
    if (mpPredictWorld != NULL)
    {
        FakeBallWorld* world = mpPredictWorld;
        if (world != NULL)
        {
            delete world->mpPhysicsBall;
            delete world->mpGroundPlane;
            delete world->mpCollisionSpace;
            delete world->mpPhysicsWorld;
            delete world;
        }
        mpPredictWorld = NULL;
    }

    if (mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* end = mBallCacheList.m_Head;
        DLListEntry<BallCacheInfo*>* current = start;

        while (current != NULL)
        {
            BallCacheInfo* data = current->m_data;
            ((SlotPoolEntry*)data)->m_next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
            BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList = (SlotPoolEntry*)data;

            if (nlDLRingIsEnd(end, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }
        }

        nlWalkDLRing<DLListEntry<BallCacheInfo*>, BallCacheListBase>(
            mBallCacheList.m_Head,
            (BallCacheListBase*)&mBallCacheList,
            (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
        mBallCacheList.m_Head = NULL;
    }

    mfLastCacheTime = -1.0f;
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)&mBallCacheList, sizeof(DLListEntry<BallCacheInfo*>));
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)&BallCacheInfo::mBallCacheInfoSlotPool, sizeof(BallCacheInfo));
}

/**
 * Offset/Address/Size: 0x20AC | 0x80139498 | size: 0x1D8
 */
void FakeBallWorld::Init(cBall* pBall)
{
    if (mpPredictWorld == NULL)
    {
        FakeBallWorld* world = (FakeBallWorld*)nlMalloc(sizeof(FakeBallWorld), 8, false);
        if (world != NULL)
        {
            world->mpBall = pBall;

            world->mpPhysicsWorld = new (nlMalloc(sizeof(PhysicsWorld), 8, false)) PhysicsWorld();
            world->mpCollisionSpace = new (nlMalloc(sizeof(SimpleCollisionSpace), 8, false)) SimpleCollisionSpace(world->mpPhysicsWorld);

            world->mpPhysicsWorld->SetCFM(0.001f);

            world->mpGroundPlane = new (nlMalloc(sizeof(PhysicsGroundPlane), 8, false)) PhysicsGroundPlane(world->mpCollisionSpace);

            FakePhysicsBall* ball = new (nlMalloc(sizeof(FakePhysicsBall), 8, false)) FakePhysicsBall(world->mpCollisionSpace, world->mpPhysicsWorld, 0.1f, *world);
            world->mpPhysicsBall = ball;
        }
        mpPredictWorld = world;
    }

    if (mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* end = mBallCacheList.m_Head;
        DLListEntry<BallCacheInfo*>* current = start;

        while (current != NULL)
        {
            BallCacheInfo* data = current->m_data;
            ((SlotPoolEntry*)data)->m_next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
            BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList = (SlotPoolEntry*)data;

            if (nlDLRingIsEnd(end, current) || current == NULL)
            {
                current = NULL;
            }
            else
            {
                current = current->m_next;
            }
        }

        nlWalkDLRing<DLListEntry<BallCacheInfo*>, BallCacheListBase>(
            mBallCacheList.m_Head,
            (BallCacheListBase*)&mBallCacheList,
            (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
        mBallCacheList.m_Head = NULL;
    }

    mfLastCacheTime = -1.0f;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80139670 | size: 0x8
//  */
// int FakePhysicsBall::GetObjectType() const
// {
//     return 0x10;
// }

/**
 * Offset/Address/Size: 0x0 | 0x80139678 | size: 0x8
 */
int PhysicsPlane::GetObjectType() const
{
    return 0x6;
}

/**
 * Offset/Address/Size: 0x0 | 0x80139680 | size: 0x10
 */
// void DLListContainerBase<BallCacheInfo*, BasicSlotPool<DLListEntry<BallCacheInfo*>>>::DeleteEntry(DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80139690 | size: 0x3C
 * TODO: 96% match - stw LR save scheduling: target interleaves after first lwz, compiler places before.
 * Template instantiation from nlDLRing.h - codegen is functionally correct with a single instruction reorder.
 */
template void nlWalkDLRing<DLListEntry<BallCacheInfo*>, BallCacheListBase>(
    DLListEntry<BallCacheInfo*>* head,
    BallCacheListBase* callback,
    void (BallCacheListBase::*callbackFunc)(DLListEntry<BallCacheInfo*>*));

/**
 * Offset/Address/Size: 0x3C | 0x801396CC | size: 0x38
 */
// void nlDLRingRemoveStart<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>**)
// {
// }

/**
 * Offset/Address/Size: 0x74 | 0x80139704 | size: 0x24
 */
// void nlDLRingIsStart<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>*, DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0x98 | 0x80139728 | size: 0x20
 */
// void nlDLRingIsEnd<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>*, DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0xB8 | 0x80139748 | size: 0x10
 */
// void nlDLRingGetEnd<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0xC8 | 0x80139758 | size: 0x18
 */
// void nlDLRingGetStart<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0xE0 | 0x80139770 | size: 0x44
 */
// void nlDLRingRemove<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>**, DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0x124 | 0x801397B4 | size: 0x3C
 */
// void nlDLRingAddEnd<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>**, DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0x160 | 0x801397F0 | size: 0x38
 */
// void nlDLRingAddStart<DLListEntry<BallCacheInfo*>>(DLListEntry<BallCacheInfo*>**, DLListEntry<BallCacheInfo*>*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80139828 | size: 0x60
 */
// void nlWalkRing<DLListEntry<BallCacheInfo*>, DLListContainerBase<BallCacheInfo*,
// BasicSlotPool<DLListEntry<BallCacheInfo*>>>>(DLListEntry<BallCacheInfo*>*, DLListContainerBase<BallCacheInfo*,
// BasicSlotPool<DLListEntry<BallCacheInfo*>>>*, void (DLListContainerBase<BallCacheInfo*,
// BasicSlotPool<DLListEntry<BallCacheInfo*>>>::*)(DLListEntry<BallCacheInfo*>*))
// {
// }

/**
 * Offset/Address/Size: 0x12C | 0x80139954 | size: 0xC8
 */
// void nlDLListSlotPool<BallCacheInfo*>::~nlDLListSlotPool()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80139A1C | size: 0x64
 */
// void SlotPool<BallCacheInfo>::~SlotPool()
// {
// }
