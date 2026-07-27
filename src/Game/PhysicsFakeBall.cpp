#include "Game/Physics/PhysicsFakeBall.h"
#include "Game/Physics/PhysicsPlane.h"
#include "Game/Physics/PhysicsGroundPlane.h"
#include "Game/Physics/Physics.h"
#include "Game/Physics/PhysicsAIBall.h"
#include "Game/Player.h"
#include "Game/FixedUpdateTask.h"
#include "Game/AI/AiUtil.h"
#include "NL/nlDLRing.h"
#include "NL/nlDLListContainer.h"

static const nlVector3 v3Zero = { 0.f, 0.f, 0.f };

static f32 CANT_COLLIDE = *(f32*)__float_max;

SlotPool<BallCacheInfo> BallCacheInfo::mBallCacheInfoSlotPool(16, 16);
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

static inline BallCacheInfo* AddCacheEntry(PhysicsBall* pPhysicsBall)
{
    DLListEntry<BallCacheInfo*>* pNewEntry;
    BallCacheInfo* pNewInfo = NULL;
    if (BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&BallCacheInfo::mBallCacheInfoSlotPool, sizeof(BallCacheInfo));
    }
    if (BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList != NULL)
    {
        pNewInfo = (BallCacheInfo*)BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
        BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList->next;
    }
    pNewInfo->mfTime = FakeBallWorld::mfLastCacheTime;
    pNewInfo->mv3Position = ((PhysicsObject*)pPhysicsBall)->GetPosition();
    pNewInfo->mv3LinearVelocity = ((PhysicsObject*)pPhysicsBall)->GetLinearVelocity();
    pNewEntry = NULL;
    if (FakeBallWorld::mBallCacheList.m_Allocator.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)&FakeBallWorld::mBallCacheList.m_Allocator, sizeof(DLListEntry<BallCacheInfo*>));
    }
    if (FakeBallWorld::mBallCacheList.m_Allocator.m_FreeList != NULL)
    {
        pNewEntry = (DLListEntry<BallCacheInfo*>*)FakeBallWorld::mBallCacheList.m_Allocator.m_FreeList;
        FakeBallWorld::mBallCacheList.m_Allocator.m_FreeList = FakeBallWorld::mBallCacheList.m_Allocator.m_FreeList->next;
    }
    if (pNewEntry != NULL)
    {
        pNewEntry->m_next = NULL;
        pNewEntry->m_prev = NULL;
        pNewEntry->entry = pNewInfo;
    }
    nlDLRingAddEnd(&FakeBallWorld::mBallCacheList.m_Head, pNewEntry);
    return pNewInfo;
}

static inline void GetNextBallPosVelInline(nlVector3& v3BallPos, nlVector3& v3BallVel)
{
    nlDLListIterator<BallCacheInfo*>* cacheIter = FakeBallWorld::mpCacheIterator;

    if (cacheIter->m_Curr != NULL)
    {
        BallCacheInfo* info = cacheIter->m_Curr->entry;
        v3BallPos = info->mv3Position;
        v3BallVel = info->mv3LinearVelocity;

        if (nlDLRingIsEnd(cacheIter->m_Head, cacheIter->m_Curr) || cacheIter->m_Curr == NULL)
        {
            cacheIter->m_Curr = NULL;
        }
        else
        {
            cacheIter->m_Curr = cacheIter->m_Curr->m_next;
        }
        return;
    }

    float tick = FixedUpdateTask::GetPhysicsUpdateTick();
    PhysicsUpdate(FakeBallWorld::mpPredictWorld->mpPhysicsWorld, tick);
    FakeBallWorld::mfLastCacheTime += tick;

    BallCacheInfo* newInfo = AddCacheEntry((PhysicsBall*)FakeBallWorld::mpPredictWorld->mpPhysicsBall);

    v3BallPos = newInfo->mv3Position;
    v3BallVel = newInfo->mv3LinearVelocity;
}

/**
 * Offset/Address/Size: 0x98 | 0x80137484 | size: 0x3EC
 */
bool FakeBallWorld::FindBallIntercept(const nlVector3& v3PlayerPos, float fPlayerReach, float fPlayerSpeed, nlVector3& v3InterceptPos, nlVector3& v3InterceptVel, float& fInterceptTime, float& fClosestDist, float fMaxTime)
{
    fInterceptTime = 0.0f;
    fClosestDist = 10000.0f;
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
    mpCacheIterator = reinterpret_cast<nlDLListIterator<BallCacheInfo*>*>(&iter);

    if (mpCacheIterator->m_Curr != NULL)
    {
        if (nlDLRingIsEnd(mpCacheIterator->m_Head, mpCacheIterator->m_Curr) || iter.m_current == NULL)
        {
            iter.m_current = NULL;
        }
        else
        {
            iter.m_current = iter.m_current->m_next;
        }
    }

    float fPlayerDistanceFromStartingPoint = fPlayerReach;

    while (!bDone)
    {
        GetNextBallPosVelInline(v3NewBallPos, v3NewBallVel);

        fPlayerDistanceFromStartingPoint += fPlayerDistPerTick;

        float dx = v3NewBallPos.f.x - v3PlayerPos.f.x;
        float dy = v3NewBallPos.f.y - v3PlayerPos.f.y;
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
 */
void FakeBallWorld::GetNextBallPosition(nlVector3& v3BallPos)
{
    if (mpCacheIterator->m_Curr != NULL)
    {
        DLListEntry<BallCacheInfo*>* entry = mpCacheIterator->m_Curr;
        BallCacheInfo* info = entry->entry;
        v3BallPos = info->mv3Position;

        nlDLListIterator<BallCacheInfo*>* iter = mpCacheIterator;
        if (nlDLRingIsEnd(iter->m_Head, iter->m_Curr) || iter->m_Curr == NULL)
        {
            iter->m_Curr = NULL;
        }
        else
        {
            iter->m_Curr = iter->m_Curr->m_next;
        }
        return;
    }

    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    FakeBallWorld* predictWorld = mpPredictWorld;
    PhysicsUpdate(predictWorld->mpPhysicsWorld, fPhysicsTick);

    predictWorld = mpPredictWorld;
    mfLastCacheTime += fPhysicsTick;

    BallCacheInfo* newInfo = AddCacheEntry((PhysicsBall*)predictWorld->mpPhysicsBall);

    v3BallPos = newInfo->mv3Position;
}

/**
 * Offset/Address/Size: 0x668 | 0x80137A54 | size: 0xC8
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
    mpCacheIterator = reinterpret_cast<nlDLListIterator<BallCacheInfo*>*>(&iter);

    if (mpCacheIterator->m_Curr != NULL)
    {
        if (nlDLRingIsEnd(mpCacheIterator->m_Head, mpCacheIterator->m_Curr) || iter.m_current == NULL)
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
 * TODO: 98.92% match - output refs use r24/r25 instead of target r23/r24;
 *       cache traversal GPR allocation differs.
 */
float FakeBallWorld::GetPredictedPosAtDistance(float fDistance, nlVector3& v3Position, nlVector3& v3Velocity)
{
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

    float fSimulationTime;
    float fDistanceTargetSq;
    float fMaxTime;
    float fPhysicsTick;
    fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    fDistanceTargetSq = fDistance * fDistance;
    fSimulationTime = FixedUpdateTask::mSimulationTime;

    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pHead = *ppHead;

    if (pHead != NULL)
    {
        DLListEntry<BallCacheInfo*>* pListEntry = nlDLRingGetStart(pHead);
        DLListEntry<BallCacheInfo*>* pHeadRef = *ppHead;
        BallCacheInfo* pPrev;
        BallCacheInfo* pNext = pListEntry->entry;

        float fDistanceNextSq = nlGetLengthSquared3D(pNext->mv3Position.f.x - pBall->m_v3Position.f.x, pNext->mv3Position.f.y - pBall->m_v3Position.f.y, pNext->mv3Position.f.z - pBall->m_v3Position.f.z);

        while (!nlDLRingIsEnd(pHeadRef, pListEntry))
        {
            if (nlDLRingIsEnd(pHeadRef, pListEntry) || pListEntry == NULL)
                pListEntry = NULL;
            else
                pListEntry = pListEntry->m_next;

            pPrev = pNext;
            pNext = pListEntry->entry;
            float fDistancePrevSq = fDistanceNextSq;

            fDistanceNextSq = nlGetLengthSquared3D(pNext->mv3Position.f.x - pBall->m_v3Position.f.x, pNext->mv3Position.f.y - pBall->m_v3Position.f.y, pNext->mv3Position.f.z - pBall->m_v3Position.f.z);

            if (fDistanceNextSq > fDistanceTargetSq)
            {
                float sqrtPrev = nlSqrt(fDistancePrevSq, true);
                float sqrtNext = nlSqrt(fDistanceNextSq, true);
                float fPercent = (fDistance - sqrtPrev) / (sqrtNext - sqrtPrev);
                float fTime = Interpolate(pPrev->mfTime, pNext->mfTime, fPercent) - fSimulationTime;

                float fInvPercent = 1.0f - fPercent;
                v3Position.f.x = fInvPercent * pPrev->mv3Position.f.x + fPercent * pNext->mv3Position.f.x;
                v3Position.f.y = fInvPercent * pPrev->mv3Position.f.y + fPercent * pNext->mv3Position.f.y;
                v3Position.f.z = fInvPercent * pPrev->mv3Position.f.z + fPercent * pNext->mv3Position.f.z;
                v3Velocity.f.x = fInvPercent * pPrev->mv3LinearVelocity.f.x + fPercent * pNext->mv3LinearVelocity.f.x;
                v3Velocity.f.y = fInvPercent * pPrev->mv3LinearVelocity.f.y + fPercent * pNext->mv3LinearVelocity.f.y;
                v3Velocity.f.z = fInvPercent * pPrev->mv3LinearVelocity.f.z + fPercent * pNext->mv3LinearVelocity.f.z;

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
    BallCacheInfo* pCurCache = pLastEntry->entry;

    fMaxTime = 6.0f + fSimulationTime;

    float fDistanceCurSq = nlGetLengthSquared3D(pCurCache->mv3Position.f.x - pBall->m_v3Position.f.x, pCurCache->mv3Position.f.y - pBall->m_v3Position.f.y, pCurCache->mv3Position.f.z - pBall->m_v3Position.f.z);

    while (mfLastCacheTime < fMaxTime)
    {
        BallCacheInfo* pLastCache = pCurCache;
        float fDistanceLastSq = fDistanceCurSq;

        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);

        mfLastCacheTime += fPhysicsTick;
        BallCacheInfo* pNewInfo = AddCacheEntry((PhysicsBall*)mpPredictWorld->mpPhysicsBall);

        pCurCache = pNewInfo;

        fDistanceCurSq = nlGetLengthSquared3D(pNewInfo->mv3Position.f.x - pBall->m_v3Position.f.x, pNewInfo->mv3Position.f.y - pBall->m_v3Position.f.y, pNewInfo->mv3Position.f.z - pBall->m_v3Position.f.z);

        if (fDistanceCurSq > fDistanceTargetSq)
        {
            float sqrtLast = nlSqrt(fDistanceLastSq, true);
            float sqrtCur = nlSqrt(fDistanceCurSq, true);
            float fPercent = (fDistance - sqrtLast) / (sqrtCur - sqrtLast);
            float fTime = Interpolate(pLastCache->mfTime, pNewInfo->mfTime, fPercent) - fSimulationTime;

            float fInvPercent = 1.0f - fPercent;
            v3Position.f.x = fInvPercent * pLastCache->mv3Position.f.x + fPercent * pNewInfo->mv3Position.f.x;
            v3Position.f.y = fInvPercent * pLastCache->mv3Position.f.y + fPercent * pNewInfo->mv3Position.f.y;
            v3Position.f.z = fInvPercent * pLastCache->mv3Position.f.z + fPercent * pNewInfo->mv3Position.f.z;
            v3Velocity.f.x = fInvPercent * pLastCache->mv3LinearVelocity.f.x + fPercent * pNewInfo->mv3LinearVelocity.f.x;
            v3Velocity.f.y = fInvPercent * pLastCache->mv3LinearVelocity.f.y + fPercent * pNewInfo->mv3LinearVelocity.f.y;
            v3Velocity.f.z = fInvPercent * pLastCache->mv3LinearVelocity.f.z + fPercent * pNewInfo->mv3LinearVelocity.f.z;

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
 * Offset/Address/Size: 0x3DC | 0x8013819C | size: 0x3FC
 */
float FakeBallWorld::GetPredictedHeightLimitTime(float fHeight, float fMinTime, nlVector3& v3ContactPoint, nlVector3& v3ContactVelocity, bool bDownOnly)
{
    cBall* pBall = mpPredictWorld->mpBall;

    float speedSq = pBall->m_v3Velocity.f.x * pBall->m_v3Velocity.f.x
                  + pBall->m_v3Velocity.f.y * pBall->m_v3Velocity.f.y
                  + pBall->m_v3Velocity.f.z * pBall->m_v3Velocity.f.z;

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
    const float fSimulationTime = FixedUpdateTask::mSimulationTime;
    float fTestTime = fSimulationTime + fMinTime;
    float fLastZVel = 0.0f;

    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(*ppHead);
    DLListEntry<BallCacheInfo*>* pHeadRef = *ppHead;
    DLListEntry<BallCacheInfo*>* pListEntry = pEntry;

    while (pListEntry)
    {
        BallCacheInfo* pCur = pListEntry->entry;

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
        }

        fLastZVel = pCur->mv3LinearVelocity.f.z;

        if (nlDLRingIsEnd(pHeadRef, pListEntry) || pListEntry == NULL)
        {
            pListEntry = NULL;
        }
        else
        {
            pListEntry = pListEntry->m_next;
        }
    }

    fTestTime = fSimulationTime + 6.0f;

    while (mfLastCacheTime < fTestTime)
    {
        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);
        mfLastCacheTime += fPhysicsTick;

        BallCacheInfo* pNewInfo = AddCacheEntry((PhysicsBall*)mpPredictWorld->mpPhysicsBall);

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

    float fSimulationTime;
    float fDistanceNext;
    float fMaxTime;
    float fPhysicsTick = FixedUpdateTask::GetPhysicsUpdateTick();
    fSimulationTime = FixedUpdateTask::mSimulationTime;
    DLListEntry<BallCacheInfo*>** ppHead = &mBallCacheList.m_Head;
    DLListEntry<BallCacheInfo*>* pHead = *ppHead;

    if (pHead != NULL)
    {
        DLListEntry<BallCacheInfo*>* pEntry = nlDLRingGetStart(pHead);
        DLListEntry<BallCacheInfo*>* pHeadRef = *ppHead;
        DLListEntry<BallCacheInfo*>* pListEntry = pEntry;
        BallCacheInfo* pPrev;
        BallCacheInfo* pNext = pEntry->entry;

        fDistanceNext = pNext->mv3Position.f.x * v4Plane.f.x
                      + pNext->mv3Position.f.y * v4Plane.f.y
                      + pNext->mv3Position.f.z * v4Plane.f.z
                      - v4Plane.f.w;

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

            pPrev = pNext;
            pNext = pListEntry->entry;
            float fDistancePrev = fDistanceNext;

            float fDistanceNew = pNext->mv3Position.f.x * v4Plane.f.x
                               + pNext->mv3Position.f.y * v4Plane.f.y
                               + pNext->mv3Position.f.z * v4Plane.f.z
                               - v4Plane.f.w;
            fDistanceNext = fDistanceNew;

            if (fDistanceNew < 0.0f)
            {
                float fPercent = fDistancePrev / (fDistancePrev - fDistanceNew);
                float fTime = Interpolate(pPrev->mfTime, pNext->mfTime, fPercent) - fSimulationTime;

                float fInvPercent = 1.0f - fPercent;
                v3ContactPoint.f.x = fInvPercent * pPrev->mv3Position.f.x + fPercent * pNext->mv3Position.f.x;
                v3ContactPoint.f.y = fInvPercent * pPrev->mv3Position.f.y + fPercent * pNext->mv3Position.f.y;
                v3ContactPoint.f.z = fInvPercent * pPrev->mv3Position.f.z + fPercent * pNext->mv3Position.f.z;
                v3ContactVelocity.f.x = fInvPercent * pPrev->mv3LinearVelocity.f.x + fPercent * pNext->mv3LinearVelocity.f.x;
                v3ContactVelocity.f.y = fInvPercent * pPrev->mv3LinearVelocity.f.y + fPercent * pNext->mv3LinearVelocity.f.y;
                v3ContactVelocity.f.z = fInvPercent * pPrev->mv3LinearVelocity.f.z + fPercent * pNext->mv3LinearVelocity.f.z;

                return fTime;
            }

            if (fDistanceNew >= fDistancePrev)
            {
                v3ContactPoint = pPrev->mv3Position;
                v3ContactVelocity = pPrev->mv3LinearVelocity;
                return -3.0f;
            }
        }
    }

    DLListEntry<BallCacheInfo*>* pLastEntry = nlDLRingGetEnd(*ppHead);
    BallCacheInfo* pCurCache = pLastEntry->entry;

    float fDistanceCur = pCurCache->mv3Position.f.x * v4Plane.f.x
                       + pCurCache->mv3Position.f.y * v4Plane.f.y
                       + pCurCache->mv3Position.f.z * v4Plane.f.z
                       - v4Plane.f.w;

    fMaxTime = 6.0f + fSimulationTime;

    while (mfLastCacheTime < fMaxTime)
    {
        BallCacheInfo* pLastCache = pCurCache;
        float fDistanceLast = fDistanceCur;

        PhysicsUpdate(mpPredictWorld->mpPhysicsWorld, fPhysicsTick);

        mfLastCacheTime += fPhysicsTick;
        BallCacheInfo* pNewInfo = AddCacheEntry((PhysicsBall*)mpPredictWorld->mpPhysicsBall);

        pCurCache = pNewInfo;

        float fDistanceNewCache = pNewInfo->mv3Position.f.x * v4Plane.f.x
                                + pNewInfo->mv3Position.f.y * v4Plane.f.y
                                + pNewInfo->mv3Position.f.z * v4Plane.f.z
                                - v4Plane.f.w;
        fDistanceCur = fDistanceNewCache;

        if (fDistanceNewCache < 0.0f)
        {
            float fPercent = fDistanceLast / (fDistanceLast - fDistanceNewCache);
            float fTime = Interpolate(pLastCache->mfTime, pNewInfo->mfTime, fPercent) - fSimulationTime;

            float fInvPercent = 1.0f - fPercent;
            v3ContactPoint.f.x = fInvPercent * pLastCache->mv3Position.f.x + fPercent * pNewInfo->mv3Position.f.x;
            v3ContactPoint.f.y = fInvPercent * pLastCache->mv3Position.f.y + fPercent * pNewInfo->mv3Position.f.y;
            v3ContactPoint.f.z = fInvPercent * pLastCache->mv3Position.f.z + fPercent * pNewInfo->mv3Position.f.z;
            v3ContactVelocity.f.x = fInvPercent * pLastCache->mv3LinearVelocity.f.x + fPercent * pNewInfo->mv3LinearVelocity.f.x;
            v3ContactVelocity.f.y = fInvPercent * pLastCache->mv3LinearVelocity.f.y + fPercent * pNewInfo->mv3LinearVelocity.f.y;
            v3ContactVelocity.f.z = fInvPercent * pLastCache->mv3LinearVelocity.f.z + fPercent * pNewInfo->mv3LinearVelocity.f.z;

            return fTime;
        }

        if (fDistanceNewCache >= fDistanceLast)
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

static inline void ClearBallCacheInline(SlotPool<BallCacheInfo>* pBCIPool)
{
    if (FakeBallWorld::mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(FakeBallWorld::mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* end = FakeBallWorld::mBallCacheList.m_Head;
        DLListEntry<BallCacheInfo*>* current = start;
        while (current != NULL)
        {
            BallCacheInfo* data = current->entry;
            ((SlotPoolEntry*)data)->next = pBCIPool->m_FreeList;
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
            FakeBallWorld::mBallCacheList.m_Head, (BallCacheListBase*)&FakeBallWorld::mBallCacheList, (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
        FakeBallWorld::mBallCacheList.m_Head = NULL;
    }
    FakeBallWorld::mfLastCacheTime = -1.0f;
}

/**
 * Offset/Address/Size: 0x1768 | 0x80138B54 | size: 0x6C0
 * TODO: 98.05% match - cache cleanup/final traversal register allocation and callback label selection still diverge
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
        ClearBallCacheInline(&BallCacheInfo::mBallCacheInfoSlotPool);
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
            BallCacheInfo* pCur = pEntry->entry;
            if (fSimTime >= pCur->mfTime)
            {
                if (pLast != NULL)
                {
                    DLListEntry<BallCacheInfo*>* removed = nlDLRingRemoveStart(pHeadRef);
                    BallCacheInfo** ppLast = &pLast;
                    if (ppLast != NULL)
                    {
                        *ppLast = removed->entry;
                    }
                    ((SlotPoolEntry*)removed)->next = pCacheList->m_Allocator.m_FreeList;
                    pCacheList->m_Allocator.m_FreeList = (SlotPoolEntry*)removed;
                    ((SlotPoolEntry*)pLast)->next = pBCIPool->m_FreeList;
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
                float distSq = nlGetLengthSquared3D(pCur->mv3Position.f.x - mpPredictWorld->mpBall->m_v3Position.f.x, pCur->mv3Position.f.y - mpPredictWorld->mpBall->m_v3Position.f.y, pCur->mv3Position.f.z - mpPredictWorld->mpBall->m_v3Position.f.z);
                if (!(distSq > 0.0025f))
                {
                    break;
                }
                ClearBallCacheInline(pBCIPool);
                break;
            }
        }
    }

    float fTargetTime = fSimTime + fDeltaTime;

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
        AddCacheEntry((PhysicsBall*)mpPredictWorld->mpPhysicsBall);
    }

    DLListEntry<BallCacheInfo*>** pHeadRef = &mBallCacheList.m_Head;
    float overshoot = mfLastCacheTime - fTargetTime;
    BallCacheInfo* pPrev;
    BallCacheInfo* pNext;
    if (fDeltaTime < overshoot)
    {
        DLListEntry<BallCacheInfo*>* pStartEntry = nlDLRingGetStart(*pHeadRef);
        pNext = pStartEntry->entry;
        DLListEntry<BallCacheInfo*>* pListEntry = pStartEntry;
        DLListEntry<BallCacheInfo*>* pHead = *pHeadRef;
        pPrev = pNext;
        while (!nlDLRingIsEnd(pHead, pListEntry) && pNext->mfTime < fTargetTime)
        {
            pPrev = pNext;
            if (nlDLRingIsEnd(pHead, pListEntry) || pListEntry == NULL)
            {
                pListEntry = NULL;
            }
            else
            {
                pListEntry = pListEntry->m_next;
            }
            pNext = pListEntry->entry;
        }
    }
    else
    {
        DLListEntry<BallCacheInfo*>* pEndEntry = nlDLRingGetEnd(*pHeadRef);
        pNext = pEndEntry->entry;
        DLListEntry<BallCacheInfo*>* pListEntry = pEndEntry;
        DLListEntry<BallCacheInfo*>* pHead = *pHeadRef;
        pPrev = pNext;
        while (!nlDLRingIsStart(pHead, pListEntry) && pPrev->mfTime >= fTargetTime)
        {
            pNext = pPrev;
            if (nlDLRingIsStart(pHead, pListEntry))
            {
                pListEntry = NULL;
            }
            else
            {
                pListEntry = pListEntry->m_prev;
            }
            pPrev = pListEntry->entry;
        }
    }

    if (pPrev != pNext && fTargetTime > pPrev->mfTime)
    {
        float fPercent = (fTargetTime - pPrev->mfTime) / (pNext->mfTime - pPrev->mfTime);

        if (fPercent < 1.0f)
        {
            float fNextTerm = fPercent * pNext->mv3Position.f.x;
            float fInvPercent = 1.0f - fPercent;
            v3Position.f.x = fNextTerm + fInvPercent * pPrev->mv3Position.f.x;
            v3Position.f.y = fPercent * pNext->mv3Position.f.y + fInvPercent * pPrev->mv3Position.f.y;
            v3Position.f.z = fPercent * pNext->mv3Position.f.z + fInvPercent * pPrev->mv3Position.f.z;
            v3Velocity.f.x = fPercent * pNext->mv3LinearVelocity.f.x + fInvPercent * pPrev->mv3LinearVelocity.f.x;
            v3Velocity.f.y = fPercent * pNext->mv3LinearVelocity.f.y + fInvPercent * pPrev->mv3LinearVelocity.f.y;
            v3Velocity.f.z = fPercent * pNext->mv3LinearVelocity.f.z + fInvPercent * pPrev->mv3LinearVelocity.f.z;
        }
        else
        {
            v3Position = pNext->mv3Position;
            v3Velocity = pNext->mv3LinearVelocity;
        }
    }
    else
    {
        v3Position = pPrev->mv3Position;
        v3Velocity = pPrev->mv3LinearVelocity;
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
            BallCacheInfo* data = current->entry;
            ((SlotPoolEntry*)data)->next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
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

static inline void DestroyBallCacheInline()
{
    if (FakeBallWorld::mBallCacheList.m_Head != NULL)
    {
        DLListEntry<BallCacheInfo*>* start = nlDLRingGetStart(FakeBallWorld::mBallCacheList.m_Head);
        DLListEntry<BallCacheInfo*>* current = start;
        DLListEntry<BallCacheInfo*>* end = FakeBallWorld::mBallCacheList.m_Head;

        while (current != NULL)
        {
            BallCacheInfo* data = current->entry;
            ((SlotPoolEntry*)data)->next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
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
            FakeBallWorld::mBallCacheList.m_Head,
            (BallCacheListBase*)&FakeBallWorld::mBallCacheList,
            (void (BallCacheListBase::*)(DLListEntry<BallCacheInfo*>*))&BallCacheListBase::DeleteEntry);
        FakeBallWorld::mBallCacheList.m_Head = NULL;
    }

    FakeBallWorld::mfLastCacheTime = -1.0f;
}

/**
 * Offset/Address/Size: 0x1F14 | 0x80139300 | size: 0x198
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

    DestroyBallCacheInline();
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
            BallCacheInfo* data = current->entry;
            ((SlotPoolEntry*)data)->next = BallCacheInfo::mBallCacheInfoSlotPool.m_FreeList;
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
