#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Physics/PhysicsPlane.h"
#include "Game/Physics/PhysicsGroundPlane.h"
#include "Game/Physics/Physics.h"
#include "Game/FixedUpdateTask.h"
#include "Game/AI/AiUtil.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListSlotPool.h"

SlotPool<BallCacheInfo> BallCacheInfo::mBallCacheInfoSlotPool;
nlDLListSlotPool<BallCacheInfo*> FakeBallWorld::mBallCacheList;

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
 * TODO: 92.86% match - remaining diffs are FPR/GPR allocation drift
 *       (fDistance lives in f27 vs target f26, pBall in r29 vs r27,
 *       cascading across both loops) and minor instruction scheduling
 *       on the v3Zero copy and pre-loop distance computation.
 */
float FakeBallWorld::GetPredictedPosAtDistance(float fDistance, nlVector3& v3Position, nlVector3& v3Velocity)
{
    extern nlVector3 v3Zero;

    cBall* pBall = mpPredictWorld->mpBall;

    float speedSq = pBall->m_v3Velocity.f.x * pBall->m_v3Velocity.f.x
                  + pBall->m_v3Velocity.f.y * pBall->m_v3Velocity.f.y
                  + pBall->m_v3Velocity.f.z * pBall->m_v3Velocity.f.z;

    if (speedSq < 0.0001f)
    {
        v3Position = pBall->m_v3Position;
        v3Velocity = v3Zero;
        return -1.0f;
    }

    if (!GetPredictedBallPosition(0.0f, v3Position, v3Velocity))
    {
        return -1.5f;
    }

    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    float fDistanceTargetSq = fDistance * fDistance;
    float fSimulationTime = FixedUpdateTask::mSimulationTime;

    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pHead = *ppHead;

    if (pHead != NULL)
    {
        DLListEntry<BallCacheInfo*>* pListEntry = nlDLRingGetStart(pHead);
        BallCacheInfo* pNext = pListEntry->m_data;
        DLListEntry<BallCacheInfo*>* pHeadRef = *ppHead;

        float dx0 = pNext->mv3Position.f.x - pBall->m_v3Position.f.x;
        float dy0 = pNext->mv3Position.f.y - pBall->m_v3Position.f.y;
        float dz0 = pNext->mv3Position.f.z - pBall->m_v3Position.f.z;
        float fDistanceNextSq = dx0 * dx0 + dy0 * dy0 + dz0 * dz0;

        while (!nlDLRingIsEnd(pHeadRef, pListEntry))
        {
            if (nlDLRingIsEnd(pHeadRef, pListEntry) || pListEntry == NULL)
                pListEntry = NULL;
            else
                pListEntry = pListEntry->m_next;

            BallCacheInfo* pPrev = pNext;
            pNext = pListEntry->m_data;
            float fDistancePrevSq = fDistanceNextSq;

            float dx = pNext->mv3Position.f.x - pBall->m_v3Position.f.x;
            float dy = pNext->mv3Position.f.y - pBall->m_v3Position.f.y;
            float dz = pNext->mv3Position.f.z - pBall->m_v3Position.f.z;
            fDistanceNextSq = dx * dx + dy * dy + dz * dz;

            if (fDistanceNextSq > fDistanceTargetSq)
            {
                float sqrtPrev = nlSqrt(fDistancePrevSq, true);
                float sqrtNext = nlSqrt(fDistanceNextSq, true);
                float fPercent = (fDistance - sqrtPrev) / (sqrtNext - sqrtPrev);
                float fTime = Interpolate(pPrev->mfTime, pNext->mfTime, fPercent) - fSimulationTime;

                float fInvPercent = 1.0f - fPercent;
                v3Position.f.x = fPercent * pNext->mv3Position.f.x + fInvPercent * pPrev->mv3Position.f.x;
                v3Position.f.y = fPercent * pNext->mv3Position.f.y + fInvPercent * pPrev->mv3Position.f.y;
                v3Position.f.z = fPercent * pNext->mv3Position.f.z + fInvPercent * pPrev->mv3Position.f.z;
                v3Velocity.f.x = fPercent * pNext->mv3LinearVelocity.f.x + fInvPercent * pPrev->mv3LinearVelocity.f.x;
                v3Velocity.f.y = fPercent * pNext->mv3LinearVelocity.f.y + fInvPercent * pPrev->mv3LinearVelocity.f.y;
                v3Velocity.f.z = fPercent * pNext->mv3LinearVelocity.f.z + fInvPercent * pPrev->mv3LinearVelocity.f.z;

                return fTime;
            }

            if (fDistanceNextSq <= fDistancePrevSq)
            {
                v3Position = pPrev->mv3Position;
                v3Velocity = pPrev->mv3LinearVelocity;
                return -2.0f;
            }
        }
    }

    DLListEntry<BallCacheInfo*>* pLastEntry = nlDLRingGetEnd(*ppHead);
    BallCacheInfo* pCurCache = pLastEntry->m_data;

    SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;
    float fMaxTime = 6.0f + fSimulationTime;
    nlDLListSlotPool<BallCacheInfo*>* pCacheList = &mBallCacheList;

    float dx0 = pCurCache->mv3Position.f.x - pBall->m_v3Position.f.x;
    float dy0 = pCurCache->mv3Position.f.y - pBall->m_v3Position.f.y;
    float dz0 = pCurCache->mv3Position.f.z - pBall->m_v3Position.f.z;
    float fDistanceCurSq = dx0 * dx0 + dy0 * dy0 + dz0 * dz0;

    while (mfLastCacheTime < fMaxTime)
    {
        BallCacheInfo* pLastCache = pCurCache;
        float fDistanceLastSq = fDistanceCurSq;

        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);

        mfLastCacheTime += fPhysicsTick;
        BallCacheInfo* pNewInfo = NULL;
        PhysicsObject* pPhysObj = mpPredictWorld->mpPhysicsBall;

        if (pBCIPool->m_FreeList == NULL)
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&BallCacheInfo::mBallCacheInfoSlotPool, sizeof(BallCacheInfo));
        if (pBCIPool->m_FreeList != NULL)
        {
            pNewInfo = (BallCacheInfo*)pBCIPool->m_FreeList;
            pBCIPool->m_FreeList = pBCIPool->m_FreeList->m_next;
        }

        pNewInfo->mfTime = mfLastCacheTime;
        pNewInfo->mv3Position = pPhysObj->GetPosition();
        pNewInfo->mv3LinearVelocity = pPhysObj->GetLinearVelocity();

        DLListEntry<BallCacheInfo*>* pNewEntry = NULL;
        if (pCacheList->m_Allocator.m_FreeList == NULL)
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&mBallCacheList, sizeof(DLListEntry<BallCacheInfo*>));
        if (pCacheList->m_Allocator.m_FreeList != NULL)
        {
            pNewEntry = (DLListEntry<BallCacheInfo*>*)pCacheList->m_Allocator.m_FreeList;
            pCacheList->m_Allocator.m_FreeList = pCacheList->m_Allocator.m_FreeList->m_next;
        }

        if (pNewEntry != NULL)
        {
            pNewEntry->m_next = NULL;
            pNewEntry->m_prev = NULL;
            pNewEntry->m_data = pNewInfo;
        }

        nlDLRingAddEnd(ppHead, pNewEntry);

        pCurCache = pNewInfo;

        float dx = pNewInfo->mv3Position.f.x - pBall->m_v3Position.f.x;
        float dy = pNewInfo->mv3Position.f.y - pBall->m_v3Position.f.y;
        float dz = pNewInfo->mv3Position.f.z - pBall->m_v3Position.f.z;
        fDistanceCurSq = dx * dx + dy * dy + dz * dz;

        if (fDistanceCurSq > fDistanceTargetSq)
        {
            float sqrtLast = nlSqrt(fDistanceLastSq, true);
            float sqrtCur = nlSqrt(fDistanceCurSq, true);
            float fPercent = (fDistance - sqrtLast) / (sqrtCur - sqrtLast);
            float fTime = Interpolate(pLastCache->mfTime, pNewInfo->mfTime, fPercent) - fSimulationTime;

            float fInvPercent = 1.0f - fPercent;
            v3Position.f.x = fPercent * pNewInfo->mv3Position.f.x + fInvPercent * pLastCache->mv3Position.f.x;
            v3Position.f.y = fPercent * pNewInfo->mv3Position.f.y + fInvPercent * pLastCache->mv3Position.f.y;
            v3Position.f.z = fPercent * pNewInfo->mv3Position.f.z + fInvPercent * pLastCache->mv3Position.f.z;
            v3Velocity.f.x = fPercent * pNewInfo->mv3LinearVelocity.f.x + fInvPercent * pLastCache->mv3LinearVelocity.f.x;
            v3Velocity.f.y = fPercent * pNewInfo->mv3LinearVelocity.f.y + fInvPercent * pLastCache->mv3LinearVelocity.f.y;
            v3Velocity.f.z = fPercent * pNewInfo->mv3LinearVelocity.f.z + fInvPercent * pLastCache->mv3LinearVelocity.f.z;

            return fTime;
        }

        if (fDistanceCurSq <= fDistanceLastSq)
        {
            v3Position = pLastCache->mv3Position;
            v3Velocity = pLastCache->mv3LinearVelocity;
            return -3.0f;
        }
    }

    v3Position = pCurCache->mv3Position;
    v3Velocity = pCurCache->mv3LinearVelocity;
    return -4.0f;
}

/**
 * Offset/Address/Size: 0xDB0 | 0x8013819C | size: 0x3FC
 * TODO: 87.03% match - remaining diffs are MWCC register allocation drift and
 *       speed-squared instruction shape (fmadds vs fmuls/fadds) in the opening block.
 */
/**
 * Offset/Address/Size: 0x3DC | 0x8013819C | size: 0x3FC
 * TODO: 91.27% match - FPR allocation: f31=fHeight instead of target f29, cascading GPR/FPR register swaps
 */
float FakeBallWorld::GetPredictedHeightLimitTime(float fHeight, float fMinTime, nlVector3& v3ContactPoint, nlVector3& v3ContactVelocity, bool bDownOnly)
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
        return fMinTime;
    }

    bool freeball = GetPredictedBallPosition(fMinTime, v3ContactPoint, v3ContactVelocity);
    if (v3ContactPoint.f.z <= fHeight)
    {
        if (!bDownOnly || v3ContactVelocity.f.z <= 0.0f)
        {
            return fMinTime;
        }
    }

    if (!freeball)
    {
        return -2.0f;
    }

    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    float fSimulationTime = FixedUpdateTask::mSimulationTime;
    float fLastZVel = 0.0f;
    float fTestTime = fSimulationTime + fMinTime;

    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(*ppHead);
    DLListEntry<BallCacheInfo*>* pHead = *ppHead;

    while (pEntry)
    {
        BallCacheInfo* pCur = pEntry->m_data;

        if (pCur->mfTime >= fTestTime)
        {
            float zPos = pCur->mv3Position.f.z;
            float zVel = pCur->mv3LinearVelocity.f.z;

            if ((zPos <= fHeight && (!bDownOnly || zVel <= 0.0f)) || (fLastZVel < 0.0f && zVel > 0.0f))
            {
                v3ContactPoint = pCur->mv3Position;
                v3ContactVelocity = pCur->mv3LinearVelocity;
                return pCur->mfTime - fSimulationTime;
            }

            fLastZVel = zVel;
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
        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);
        mfLastCacheTime += fPhysicsTick;

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

        DLListEntry<BallCacheInfo*>* pNewEntry = NULL;

        if (mBallCacheList.m_Allocator.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&mBallCacheList.m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
        }
        if (mBallCacheList.m_Allocator.m_FreeList)
        {
            pNewEntry = (DLListEntry<BallCacheInfo*>*)mBallCacheList.m_Allocator.m_FreeList;
            mBallCacheList.m_Allocator.m_FreeList = mBallCacheList.m_Allocator.m_FreeList->m_next;
        }

        if (pNewEntry)
        {
            pNewEntry->m_next = NULL;
            pNewEntry->m_prev = NULL;
            pNewEntry->m_data = pNewInfo;
        }

        nlDLRingAddEnd(ppHead, pNewEntry);

        float zPos = pNewInfo->mv3Position.f.z;
        float zVel = pNewInfo->mv3LinearVelocity.f.z;

        if ((zPos <= fHeight && (!bDownOnly || zVel <= 0.0f)) || (fLastZVel < 0.0f && zVel > 0.0f))
        {
            v3ContactPoint = pNewInfo->mv3Position;
            v3ContactVelocity = pNewInfo->mv3LinearVelocity;
            return pNewInfo->mfTime - fSimulationTime;
        }

        fLastZVel = zVel;
    }

    return -1.0f;
}

/**
 * Offset/Address/Size: 0x11AC | 0x80138598 | size: 0x5BC
 * TODO: 93.34% match - remaining diffs are MWCC register allocation:
 *       GPR swap (r28/r31 ppHead/pNext, r24/r30 pHeadRef/pPrev),
 *       cascading FPR swaps for plane locals and tick/simTime.
 */
float FakeBallWorld::GetPredictedPlaneIntersectTime(const nlVector4& v4Plane, nlVector3& v3ContactPoint, nlVector3& v3ContactVelocity)
{
    cBall* pBall = mpPredictWorld->mpBall;

    float fDist = pBall->m_v3Position.f.x * v4Plane.f.x
                + pBall->m_v3Position.f.y * v4Plane.f.y
                + pBall->m_v3Position.f.z * v4Plane.f.z
                - v4Plane.f.w;

    if (fDist < 0.0f)
    {
        return -1.0f;
    }

    float fVelDot = pBall->m_v3Velocity.f.x * v4Plane.f.x
                  + pBall->m_v3Velocity.f.y * v4Plane.f.y
                  + pBall->m_v3Velocity.f.z * v4Plane.f.z;

    if (fVelDot >= 0.0f)
    {
        return -2.0f;
    }

    if (!GetPredictedBallPosition(0.0f, v3ContactPoint, v3ContactVelocity))
    {
        return -2.5f;
    }

    float planeX, planeY, planeZ, planeW;
    float fDistanceNext;
    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    float fSimulationTime = FixedUpdateTask::mSimulationTime;
    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pHead = *ppHead;

    if (pHead != NULL)
    {
        DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(pHead);
        BallCacheInfo* pNext = pEntry->m_data;
        DLListEntry<BallCacheInfo*>* pListEntry = pEntry;

        planeX = v4Plane.f.x;
        planeY = v4Plane.f.y;
        planeZ = v4Plane.f.z;
        planeW = v4Plane.f.w;

        fDistanceNext = pNext->mv3Position.f.x * planeX
                      + pNext->mv3Position.f.y * planeY
                      + pNext->mv3Position.f.z * planeZ
                      - planeW;

        DLListEntry<BallCacheInfo*>* pHeadRef = *ppHead;

        while (!nlDLRingIsEnd(pHeadRef, pListEntry))
        {
            if (nlDLRingIsEnd(pHeadRef, pListEntry) || pListEntry == NULL)
            {
                pListEntry = NULL;
            }
            else
            {
                pListEntry = pListEntry->m_next;
            }

            BallCacheInfo* pPrev = pNext;
            pNext = pListEntry->m_data;
            float fDistancePrev = fDistanceNext;

            fDistanceNext = pNext->mv3Position.f.x * planeX
                          + pNext->mv3Position.f.y * planeY
                          + pNext->mv3Position.f.z * planeZ
                          - planeW;

            if (fDistanceNext < 0.0f)
            {
                float fPercent = fDistancePrev / (fDistancePrev - fDistanceNext);
                float fTime = Interpolate(pPrev->mfTime, pNext->mfTime, fPercent) - fSimulationTime;

                float fInvPercent = 1.0f - fPercent;
                v3ContactPoint.f.x = fPercent * pNext->mv3Position.f.x + fInvPercent * pPrev->mv3Position.f.x;
                v3ContactPoint.f.y = fPercent * pNext->mv3Position.f.y + fInvPercent * pPrev->mv3Position.f.y;
                v3ContactPoint.f.z = fPercent * pNext->mv3Position.f.z + fInvPercent * pPrev->mv3Position.f.z;
                v3ContactVelocity.f.x = fPercent * pNext->mv3LinearVelocity.f.x + fInvPercent * pPrev->mv3LinearVelocity.f.x;
                v3ContactVelocity.f.y = fPercent * pNext->mv3LinearVelocity.f.y + fInvPercent * pPrev->mv3LinearVelocity.f.y;
                v3ContactVelocity.f.z = fPercent * pNext->mv3LinearVelocity.f.z + fInvPercent * pPrev->mv3LinearVelocity.f.z;

                return fTime;
            }

            if (fDistanceNext >= fDistancePrev)
            {
                v3ContactPoint = pPrev->mv3Position;
                v3ContactVelocity = pPrev->mv3LinearVelocity;
                return -3.0f;
            }
        }
    }

    nlDLListSlotPool<BallCacheInfo*>* pCacheList = &mBallCacheList;
    DLListEntry<BallCacheInfo*>* pLastEntry = nlDLRingGetEnd(*ppHead);
    BallCacheInfo* pCurCache = pLastEntry->m_data;

    SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;
    float fMaxTime = 6.0f + fSimulationTime;

    planeX = v4Plane.f.x;
    planeY = v4Plane.f.y;
    planeZ = v4Plane.f.z;
    planeW = v4Plane.f.w;

    float fDistanceCur = pCurCache->mv3Position.f.x * planeX
                       + pCurCache->mv3Position.f.y * planeY
                       + pCurCache->mv3Position.f.z * planeZ
                       - planeW;

    while (mfLastCacheTime < fMaxTime)
    {
        BallCacheInfo* pLastCache = pCurCache;
        float fDistanceLast = fDistanceCur;

        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);

        mfLastCacheTime += fPhysicsTick;
        BallCacheInfo* pNewInfo = NULL;
        PhysicsObject* pPhysObj = mpPredictWorld->mpPhysicsBall;

        if (pBCIPool->m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)pBCIPool, sizeof(BallCacheInfo));
        }
        if (pBCIPool->m_FreeList != NULL)
        {
            pNewInfo = (BallCacheInfo*)pBCIPool->m_FreeList;
            pBCIPool->m_FreeList = pBCIPool->m_FreeList->m_next;
        }

        pNewInfo->mfTime = mfLastCacheTime;
        pNewInfo->mv3Position = pPhysObj->GetPosition();
        pNewInfo->mv3LinearVelocity = pPhysObj->GetLinearVelocity();

        DLListEntry<BallCacheInfo*>* pNewEntry = NULL;
        if (pCacheList->m_Allocator.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&pCacheList->m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
        }
        if (pCacheList->m_Allocator.m_FreeList != NULL)
        {
            pNewEntry = (DLListEntry<BallCacheInfo*>*)pCacheList->m_Allocator.m_FreeList;
            pCacheList->m_Allocator.m_FreeList = pCacheList->m_Allocator.m_FreeList->m_next;
        }

        if (pNewEntry != NULL)
        {
            pNewEntry->m_next = NULL;
            pNewEntry->m_prev = NULL;
            pNewEntry->m_data = pNewInfo;
        }

        nlDLRingAddEnd(ppHead, pNewEntry);

        pCurCache = pNewInfo;

        fDistanceCur = pNewInfo->mv3Position.f.x * planeX
                     + pNewInfo->mv3Position.f.y * planeY
                     + pNewInfo->mv3Position.f.z * planeZ
                     - planeW;

        if (fDistanceCur < 0.0f)
        {
            float fPercent = fDistanceLast / (fDistanceLast - fDistanceCur);
            float fTime = Interpolate(pLastCache->mfTime, pNewInfo->mfTime, fPercent) - fSimulationTime;

            float fInvPercent = 1.0f - fPercent;
            v3ContactPoint.f.x = fPercent * pNewInfo->mv3Position.f.x + fInvPercent * pLastCache->mv3Position.f.x;
            v3ContactPoint.f.y = fPercent * pNewInfo->mv3Position.f.y + fInvPercent * pLastCache->mv3Position.f.y;
            v3ContactPoint.f.z = fPercent * pNewInfo->mv3Position.f.z + fInvPercent * pLastCache->mv3Position.f.z;
            v3ContactVelocity.f.x = fPercent * pNewInfo->mv3LinearVelocity.f.x + fInvPercent * pLastCache->mv3LinearVelocity.f.x;
            v3ContactVelocity.f.y = fPercent * pNewInfo->mv3LinearVelocity.f.y + fInvPercent * pLastCache->mv3LinearVelocity.f.y;
            v3ContactVelocity.f.z = fPercent * pNewInfo->mv3LinearVelocity.f.z + fInvPercent * pLastCache->mv3LinearVelocity.f.z;

            return fTime;
        }

        if (fDistanceCur >= fDistanceLast)
        {
            v3ContactPoint = pLastCache->mv3Position;
            v3ContactVelocity = pLastCache->mv3LinearVelocity;
            return -4.0f;
        }
    }

    v3ContactPoint = pCurCache->mv3Position;
    v3ContactVelocity = pCurCache->mv3LinearVelocity;
    return -5.0f;
}

typedef DLListContainerBase<BallCacheInfo*, BasicSlotPool<DLListEntry<BallCacheInfo*> > > BallCacheListBase;

/**
 * Offset/Address/Size: 0x1768 | 0x80138B54 | size: 0x6C0
 * TODO: 81.05% match - r31/r30 register swap for v3Position/v3Velocity params cascades to all other register assignments
 */
bool FakeBallWorld::GetPredictedBallPosition(float fDeltaTime, nlVector3& v3Position, nlVector3& v3Velocity)
{
    cBall* pBall = mpPredictWorld->mpBall;

    if (pBall->m_pOwner != NULL)
    {
        v3Position = pBall->m_v3Position;
        v3Velocity = mpPredictWorld->mpBall->m_pOwner->m_v3Velocity;
        return false;
    }

    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    float fSimTime = FixedUpdateTask::mSimulationTime;

    if (mfLastCacheTime < fSimTime)
    {
        nlDLListSlotPool<BallCacheInfo*>* pCacheRef = &mBallCacheList;

        if (pCacheRef->m_Head != NULL)
        {
            DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(pCacheRef->m_Head);
            DLListEntry<BallCacheInfo*>* end = pCacheRef->m_Head;
            DLListEntry<BallCacheInfo*>* current = start;
            SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;

            while (current != NULL)
            {
                BallCacheInfo* data = current->m_data;
                ((SlotPoolEntry*)data)->m_next = pBCIPool->m_FreeList;
                pBCIPool->m_FreeList = (SlotPoolEntry*)data;

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
                pCacheRef->m_Head,
                (BallCacheListBase*)&mBallCacheList,
                (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
            pCacheRef->m_Head = NULL;
        }

        mfLastCacheTime = -1.0f;
    }
    else if (mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>** pHeadRef = &mBallCacheList.m_Head;
        BallCacheInfo* pLast = NULL;
        DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* pHead = mBallCacheList.m_Head;
        nlDLListSlotPool<BallCacheInfo*>* pCacheList = &mBallCacheList;
        SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;

        while (pEntry != NULL)
        {
            BallCacheInfo* pCur = pEntry->m_data;

            if (fSimTime >= pCur->mfTime)
            {
                if (pLast != NULL)
                {
                    DLListEntry<BallCacheInfo*>* removed = nlDLRingRemoveStart(pHeadRef);
                    BallCacheInfo** ppLast = &pLast;
                    if (ppLast != NULL)
                    {
                        *ppLast = removed->m_data;
                    }

                    ((SlotPoolEntry*)removed)->m_next = pCacheList->m_Allocator.m_FreeList;
                    pCacheList->m_Allocator.m_FreeList = (SlotPoolEntry*)removed;

                    ((SlotPoolEntry*)pLast)->m_next = pBCIPool->m_FreeList;
                    pBCIPool->m_FreeList = (SlotPoolEntry*)pLast;
                }
                pLast = pCur;

                if (nlDLRingIsEnd(pHead, pEntry) || pEntry == NULL)
                {
                    pEntry = NULL;
                }
                else
                {
                    pEntry = pEntry->m_next;
                }
            }
            else
            {
                if (pLast != NULL)
                {
                    if (fSimTime - pLast->mfTime < pCur->mfTime - fSimTime)
                    {
                        pCur = pLast;
                    }
                }

                cBall* pBallRef = mpPredictWorld->mpBall;
                float dy = pCur->mv3Position.f.y - pBallRef->m_v3Position.f.y;
                float dx = pCur->mv3Position.f.x - pBallRef->m_v3Position.f.x;
                float dz = pCur->mv3Position.f.z - pBallRef->m_v3Position.f.z;
                float distSq = dy * dy + dx * dx + dz * dz;

                if (distSq <= 0.0025f)
                {
                    break;
                }

                if (*pHeadRef != NULL)
                {
                    DLListEntry<BallCacheInfo*>* walkStart = nlDLRingGetStart(*pHeadRef);
                    DLListEntry<BallCacheInfo*>* walkHead = *pHeadRef;
                    DLListEntry<BallCacheInfo*>* walkEntry = walkStart;

                    while (walkEntry != NULL)
                    {
                        BallCacheInfo* data = walkEntry->m_data;
                        ((SlotPoolEntry*)data)->m_next = pBCIPool->m_FreeList;
                        pBCIPool->m_FreeList = (SlotPoolEntry*)data;

                        if (nlDLRingIsEnd(walkHead, walkEntry) || walkEntry == NULL)
                        {
                            walkEntry = NULL;
                        }
                        else
                        {
                            walkEntry = walkEntry->m_next;
                        }
                    }

                    nlWalkDLRing<DLListEntry<BallCacheInfo*>, BallCacheListBase>(
                        *pHeadRef,
                        (BallCacheListBase*)&mBallCacheList,
                        (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
                    *pHeadRef = NULL;
                }

                mfLastCacheTime = -1.0f;
                break;
            }
        }
    }

    float fTargetTime = fSimTime + fDeltaTime;
    nlDLListSlotPool<BallCacheInfo*>* pCacheList = &mBallCacheList;
    DLListEntry<BallCacheInfo*>** pHeadRef = &pCacheList->m_Head;
    SlotPool<BallCacheInfo>* pBCIPool = &BallCacheInfo::mBallCacheInfoSlotPool;

    while (mfLastCacheTime < fTargetTime)
    {
        if (mfLastCacheTime < fSimTime)
        {
            mpPredictWorld->mbHitSuccess = false;
            mpPredictWorld->mpPhysicsBall->CloneBall(*mpPredictWorld->mpBall->m_pPhysicsBall);
            mfLastCacheTime = fSimTime;
        }
        else
        {
            PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);
            mfLastCacheTime += fPhysicsTick;
        }

        BallCacheInfo* pNewInfo = NULL;
        PhysicsObject* pPhysObj = mpPredictWorld->mpPhysicsBall;

        if (pBCIPool->m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)pBCIPool, sizeof(BallCacheInfo));
        }
        if (pBCIPool->m_FreeList != NULL)
        {
            pNewInfo = (BallCacheInfo*)pBCIPool->m_FreeList;
            pBCIPool->m_FreeList = pBCIPool->m_FreeList->m_next;
        }

        pNewInfo->mfTime = mfLastCacheTime;
        pNewInfo->mv3Position = pPhysObj->GetPosition();
        pNewInfo->mv3LinearVelocity = pPhysObj->GetLinearVelocity();

        DLListEntry<BallCacheInfo*>* pNewEntry = NULL;

        if (pCacheList->m_Allocator.m_FreeList == NULL)
        {
            SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&pCacheList->m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
        }
        if (pCacheList->m_Allocator.m_FreeList != NULL)
        {
            pNewEntry = (DLListEntry<BallCacheInfo*>*)pCacheList->m_Allocator.m_FreeList;
            pCacheList->m_Allocator.m_FreeList = pCacheList->m_Allocator.m_FreeList->m_next;
        }

        if (pNewEntry != NULL)
        {
            pNewEntry->m_next = NULL;
            pNewEntry->m_prev = NULL;
            pNewEntry->m_data = pNewInfo;
        }

        nlDLRingAddEnd(pHeadRef, pNewEntry);
    }

    float overshoot = mfLastCacheTime - fTargetTime;
    BallCacheInfo* pPrev;
    BallCacheInfo* pNext;

    if (fDeltaTime < overshoot)
    {
        DLListEntry<BallCacheInfo*>* pStartEntry = nlDLRingGetStart(*pHeadRef);
        pNext = pStartEntry->m_data;
        DLListEntry<BallCacheInfo*>* pListEntry = pStartEntry;
        DLListEntry<BallCacheInfo*>* pHead = *pHeadRef;
        pPrev = pNext;

        while (!nlDLRingIsEnd(pHead, pListEntry))
        {
            if (pNext->mfTime >= fTargetTime)
            {
                break;
            }

            pPrev = pNext;

            if (nlDLRingIsEnd(pHead, pListEntry) || pListEntry == NULL)
            {
                pListEntry = NULL;
            }
            else
            {
                pListEntry = pListEntry->m_next;
            }
            pNext = pListEntry->m_data;
        }
    }
    else
    {
        DLListEntry<BallCacheInfo*>* pEndEntry = nlDLRingGetEnd(*pHeadRef);
        pNext = pEndEntry->m_data;
        DLListEntry<BallCacheInfo*>* pListEntry = pEndEntry;
        DLListEntry<BallCacheInfo*>* pHead = *pHeadRef;
        pPrev = pNext;

        while (!nlDLRingIsStart(pHead, pListEntry))
        {
            if (pPrev->mfTime < fTargetTime)
            {
                break;
            }

            pNext = pPrev;

            if (nlDLRingIsStart(pHead, pListEntry))
            {
                pListEntry = NULL;
            }
            else
            {
                pListEntry = pListEntry->m_prev;
            }
            pPrev = pListEntry->m_data;
        }
    }

    if (pNext == pPrev || fTargetTime <= pPrev->mfTime)
    {
        v3Position = pPrev->mv3Position;
        v3Velocity = pPrev->mv3LinearVelocity;
    }
    else
    {
        float fPercent = (fTargetTime - pPrev->mfTime) / (pNext->mfTime - pPrev->mfTime);

        if (fPercent >= 1.0f)
        {
            v3Position = pNext->mv3Position;
            v3Velocity = pNext->mv3LinearVelocity;
        }
        else
        {
            float fInvPercent = 1.0f - fPercent;

            v3Position.f.x = fPercent * pNext->mv3Position.f.x + fInvPercent * pPrev->mv3Position.f.x;
            v3Position.f.y = fPercent * pNext->mv3Position.f.y + fInvPercent * pPrev->mv3Position.f.y;
            v3Position.f.z = fPercent * pNext->mv3Position.f.z + fInvPercent * pPrev->mv3Position.f.z;

            v3Velocity.f.x = fPercent * pNext->mv3LinearVelocity.f.x + fInvPercent * pPrev->mv3LinearVelocity.f.x;
            v3Velocity.f.y = fPercent * pNext->mv3LinearVelocity.f.y + fInvPercent * pPrev->mv3LinearVelocity.f.y;
            v3Velocity.f.z = fPercent * pNext->mv3LinearVelocity.f.z + fInvPercent * pPrev->mv3LinearVelocity.f.z;
        }
    }

    return true;
}

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
