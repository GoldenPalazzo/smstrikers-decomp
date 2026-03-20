#include "Game/SAnim/pnSAnimController.h"
#include "Game/SAnim.h"
#include "Game/SAnim/AnimRetargeter.h"

SlotPool<cPN_SAnimController> cPN_SAnimController::m_SAnimControllerSlotPool;

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
}

struct SAnimControllerDestructorChain
{
    SAnimControllerDestructorChain* next;
    void* destructor;
    void* object;
};

void SAnimControllerSlotPoolDtor(void* obj, int)
{
    ((SlotPool<cPN_SAnimController>*)obj)->~SlotPool<cPN_SAnimController>();
}

/**
 * Offset/Address/Size: 0x1064 | 0x801EB4B4 | size: 0x68
 * TODO: 99.23% match - relocation symbols differ for slot-pool destructor and
 * @185 registration chain.
 */
extern "C" void __sinit_pnSAnimController_cpp()
{
    static SAnimControllerDestructorChain chain;
    SlotPoolBase* pool = (SlotPoolBase*)&cPN_SAnimController::m_SAnimControllerSlotPool;

    __ct__12SlotPoolBaseFv(pool);
    pool->m_Initial = 0x10;
    SlotPoolBase::BaseAddNewBlock(pool, 0x54);
    pool->m_Delta = 0x10;
    __register_global_object(pool, (void*)SAnimControllerSlotPoolDtor, &chain);
}

/**
 * Offset/Address/Size: 0xDC0 | 0x801EB41C | size: 0x98
 */
cPN_SAnimController::cPN_SAnimController(cSAnim* pSAnim, const AnimRetarget* pAnimRetarget, ePlayMode playMode, void (*funcPlaybackSpeedCallback)(unsigned int, cPN_SAnimController*), unsigned int nPlaybackSpeedCallbackParam, bool bMirror)
    : cPoseNode()
{
    m_pSAnim = pSAnim;
    m_fTime = 0.0f;
    m_bMirror = bMirror;
    m_pAnimRetarget = pAnimRetarget;
    m_fPrevTime = 0.0f;
    m_ePlayMode = playMode;
    m_bIgnoreTriggers = false;
    m_funcPlaybackSpeedCallback = funcPlaybackSpeedCallback;
    m_nPlaybackSpeedCallbackParam = nPlaybackSpeedCallbackParam;
    m_fPlaybackSpeedScale = 1.0f;
    m_bIsSynchronized = false;
    m_pSynchronizedController = nullptr;
    m_funcSychronizedWeightCallback = nullptr;
    m_fSynchronizedWeight = 1.0f;
}

/**
 * Offset/Address/Size: 0xAF0 | 0x801EB14C | size: 0x2D0
 * TODO: 99.29% match - synchronized ratio block still differs in
 * f2/f3/f4/f7 register allocation.
 */
cPoseNode* cPN_SAnimController::Update(float dt)
{
    if (!m_bIsSynchronized)
    {
        if (m_funcPlaybackSpeedCallback != nullptr)
        {
            m_funcPlaybackSpeedCallback(m_nPlaybackSpeedCallbackParam, this);
        }

        float playbackSpeedScale = m_fPlaybackSpeedScale;

        if (m_pSynchronizedController != nullptr)
        {
            if (m_funcSychronizedWeightCallback != nullptr)
            {
                m_funcSychronizedWeightCallback(m_nSynchronizedWeightCallbackParam, this);
            }

            cPN_SAnimController* pSyncController = m_pSynchronizedController;
            float syncDuration = (float)pSyncController->m_pSAnim->m_nNumKeys / 30.0f;
            float thisDuration = (float)m_pSAnim->m_nNumKeys / 30.0f;
            float ratio = (thisDuration / m_fPlaybackSpeedScale)
                        / (syncDuration / pSyncController->m_fPlaybackSpeedScale);

            playbackSpeedScale *= m_fSynchronizedWeight * (ratio - 1.0f) + 1.0f;
        }

        m_fPrevTime = m_fTime;

        float duration = (float)m_pSAnim->m_nNumKeys / 30.0f;

        if (duration == 0.0f)
        {
            m_fTime = 1.0f;
        }
        else
        {
            m_fTime += (dt * playbackSpeedScale) / duration;
        }

        if (m_fTime > 1.0f)
        {
            switch (m_ePlayMode)
            {
            case PM_CYCLIC:
                do
                {
                    m_fTime -= 1.0f;
                } while (m_fTime > 1.0f);
                break;
            case PM_HOLD:
                m_fTime = 1.0f;
                break;
            }
        }
        else if (m_fTime < 0.0f)
        {
            switch (m_ePlayMode)
            {
            case PM_CYCLIC:
                do
                {
                    m_fTime += 1.0f;
                } while (m_fTime < 0.0f);
                break;
            case PM_HOLD:
                m_fTime = 0.0f;
                break;
            }
        }

        if (dt >= 0.0f && !m_bIgnoreTriggers)
        {
            cSAnimCallback* pCallback = m_pSAnim->m_pCallbackList;

            while (pCallback != nullptr)
            {
                float callbackTime = pCallback->m_fTime;

                if (callbackTime == 0.0f)
                {
                    extern float __float_min[];
                    callbackTime = __float_min[0];
                }

                float currTime = m_fTime;
                float prevTime = m_fPrevTime;
                bool shouldTrigger;

                if (currTime < prevTime)
                {
                    shouldTrigger = false;
                    if (callbackTime <= currTime || callbackTime > prevTime)
                    {
                        shouldTrigger = true;
                    }
                }
                else
                {
                    shouldTrigger = false;
                    if (callbackTime <= currTime && callbackTime > prevTime)
                    {
                        shouldTrigger = true;
                    }
                }

                if (shouldTrigger)
                {
                    pCallback->m_funcCallback(pCallback->m_nParam1);
                }

                pCallback = pCallback->next;
            }
        }

        if (m_pSynchronizedController != nullptr)
        {
            m_pSynchronizedController->UpdateSynchronized(m_fTime);
        }
    }

    return this;
}

/**
 * Offset/Address/Size: 0x7E8 | 0x801EAE44 | size: 0x308
 */
void cPN_SAnimController::UpdateSynchronized(float time)
{
    /**
     * TODO: 99.69% match - r30/r31 swap for this vs pCallback in first block.
     */
    cSAnimCallback* pCallback;

    m_fPrevTime = m_fTime;
    m_fTime = time;

    if (!m_bIgnoreTriggers)
    {
        pCallback = m_pSAnim->m_pCallbackList;

        while (pCallback != nullptr)
        {
            float callbackTime = pCallback->m_fTime;
            if (callbackTime == 0.0f)
            {
                extern float __float_min[];
                callbackTime = __float_min[0];
            }

            float currTime = m_fTime;
            float prevTime = m_fPrevTime;
            bool shouldTrigger;

            if (currTime < prevTime)
            {
                shouldTrigger = false;
                if (callbackTime <= currTime || callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }
            else
            {
                shouldTrigger = false;
                if (callbackTime <= currTime && callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }

            if (shouldTrigger)
            {
                pCallback->m_funcCallback(pCallback->m_nParam1);
            }

            pCallback = pCallback->next;
        }
    }

    cPN_SAnimController* pController = m_pSynchronizedController;
    if (pController == nullptr)
    {
        return;
    }

    pController->m_fPrevTime = pController->m_fTime;
    pController->m_fTime = time;

    if (!pController->m_bIgnoreTriggers)
    {
        pCallback = pController->m_pSAnim->m_pCallbackList;

        while (pCallback != nullptr)
        {
            float callbackTime = pCallback->m_fTime;
            if (callbackTime == 0.0f)
            {
                extern float __float_min[];
                callbackTime = __float_min[0];
            }

            float currTime = pController->m_fTime;
            float prevTime = pController->m_fPrevTime;
            bool shouldTrigger;

            if (currTime < prevTime)
            {
                shouldTrigger = false;
                if (callbackTime <= currTime || callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }
            else
            {
                shouldTrigger = false;
                if (callbackTime <= currTime && callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }

            if (shouldTrigger)
            {
                pCallback->m_funcCallback(pCallback->m_nParam1);
            }

            pCallback = pCallback->next;
        }
    }

    pController = pController->m_pSynchronizedController;
    if (pController == nullptr)
    {
        return;
    }

    pController->m_fPrevTime = pController->m_fTime;
    pController->m_fTime = time;

    if (!pController->m_bIgnoreTriggers)
    {
        pCallback = pController->m_pSAnim->m_pCallbackList;

        while (pCallback != nullptr)
        {
            float callbackTime = pCallback->m_fTime;
            if (callbackTime == 0.0f)
            {
                extern float __float_min[];
                callbackTime = __float_min[0];
            }

            float currTime = pController->m_fTime;
            float prevTime = pController->m_fPrevTime;
            bool shouldTrigger;

            if (currTime < prevTime)
            {
                shouldTrigger = false;
                if (callbackTime <= currTime || callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }
            else
            {
                shouldTrigger = false;
                if (callbackTime <= currTime && callbackTime > prevTime)
                {
                    shouldTrigger = true;
                }
            }

            if (shouldTrigger)
            {
                pCallback->m_funcCallback(pCallback->m_nParam1);
            }

            pCallback = pCallback->next;
        }
    }

    pController = pController->m_pSynchronizedController;
    if (pController == nullptr)
    {
        return;
    }

    pController->m_fPrevTime = pController->m_fTime;
    pController->m_fTime = time;

    if (!pController->m_bIgnoreTriggers)
    {
        pCallback = pController->m_pSAnim->GetCallbackList();

        while (pCallback != nullptr)
        {
            if (pController->TestTrigger(pCallback->m_fTime))
            {
                pCallback->m_funcCallback(pCallback->m_nParam1);
            }

            pCallback = pCallback->next;
        }
    }

    cPN_SAnimController* pTailController = pController->m_pSynchronizedController;
    if (pTailController == nullptr)
    {
        return;
    }

    pTailController->SetTime(time);
    pTailController->ProcessCallbacks();

    if (pTailController->m_pSynchronizedController != nullptr)
    {
        pTailController->m_pSynchronizedController->UpdateSynchronized(time);
    }
}

/**
 * Offset/Address/Size: 0x760 | 0x801EADBC | size: 0x88
 */
void cPN_SAnimController::Evaluate(float weight, cPoseAccumulator* pAccumulator) const
{
    m_fWeight = weight;

    for (int i = 0; i < pAccumulator->GetNumNodes(); ++i)
    {
        Evaluate(i, weight, pAccumulator);
    }
}

/**
 * Offset/Address/Size: 0x5DC | 0x801EAC38 | size: 0x184
 */
void cPN_SAnimController::Evaluate(int nodeIndex, float weight, cPoseAccumulator* pAccumulator) const
{
    int remappedNodeIndex;

    if (nodeIndex == 0)
    {
        for (int i = 0; i < m_pSAnim->m_nNumMorphChannels; ++i)
        {
            pAccumulator->m_MorphWeights.mData[i] += weight * m_pSAnim->GetMorphWeight(i, m_fTime);
        }
    }

    int actualNodeIndex = nodeIndex;
    if (m_bMirror != 0)
    {
        actualNodeIndex = pAccumulator->m_BaseSHierarchy->GetMirroredNode(nodeIndex);
    }

    if (m_pAnimRetarget != nullptr)
    {
        remappedNodeIndex = m_pAnimRetarget->m_pMap[actualNodeIndex];
    }
    else
    {
        remappedNodeIndex = actualNodeIndex;
    }

    if (remappedNodeIndex != -1)
    {
        m_pSAnim->BlendRot(nodeIndex, remappedNodeIndex, m_fTime, weight, pAccumulator, get_bMirror());
        m_pSAnim->BlendScale(nodeIndex, remappedNodeIndex, m_fTime, weight, pAccumulator, get_bMirror());
        m_pSAnim->BlendTrans(nodeIndex, remappedNodeIndex, m_fTime, weight, pAccumulator, get_bMirror());
        return;
    }

    nlVector3* pTranslationOffset = pAccumulator->m_BaseSHierarchy->GetTranslationOffset(nodeIndex);
    pAccumulator->BlendRotIdentity(nodeIndex, weight);
    pAccumulator->BlendScaleIdentity(nodeIndex, weight);
    pAccumulator->m_trans.mData[nodeIndex].t = *pTranslationOffset;
}

/**
 * Offset/Address/Size: 0x35C | 0x801EA9B8 | size: 0x280
 * TODO: 89.98% match - stack/local slot packing differs (0x44..0x4C vs 0x50..0x58),
 * causing a smaller frame and shifted prologue/epilogue offsets.
 */
static inline void GetRootTransDelta(cPN_SAnimController* pController, nlVector3* pRootTrans, float fStartTime, float fEndTime)
{
    nlVector3 v3LastFrame;

    pController->m_pSAnim->GetRootTrans(fEndTime, pRootTrans);
    pController->m_pSAnim->GetRootTrans(fStartTime, &v3LastFrame);

    pRootTrans->f.x = pRootTrans->f.x - v3LastFrame.f.x;
    pRootTrans->f.y = pRootTrans->f.y - v3LastFrame.f.y;
    pRootTrans->f.z = pRootTrans->f.z - v3LastFrame.f.z;
}

/**
 * Offset/Address/Size: 0x3DC | 0x801EA9B8 | size: 0x280
 */
void cPN_SAnimController::BlendRootTrans(nlVector3* pRootTrans, float fNodeWeight, float* fAccumulatedWeight)
{
    unsigned short aLastFrameFacing;
    nlVector3 v3RootTrans;
    nlVector3 v3RootTransLocal;
    nlVector3* pV3RootTransLocal;
    unsigned short aMirrorAdjust;
    nlVector3 v3Extra;
    float fBlendPercent;

    pV3RootTransLocal = &v3RootTransLocal;
    aMirrorAdjust = 0;

    if (m_fTime < m_fPrevTime)
    {
        GetRootTransDelta(this, &v3RootTrans, m_fPrevTime, 1.0f);
        GetRootTransDelta(this, &v3Extra, 0.0f, m_fTime);

        v3RootTrans.f.x = v3RootTrans.f.x + v3Extra.f.x;
        v3RootTrans.f.y = v3RootTrans.f.y + v3Extra.f.y;
        v3RootTrans.f.z = v3RootTrans.f.z + v3Extra.f.z;
    }
    else
    {
        GetRootTransDelta(this, &v3RootTrans, m_fPrevTime, m_fTime);
    }

    m_pSAnim->GetRootRot(m_fPrevTime, &aLastFrameFacing);

    if (m_bMirror != 0)
    {
        aMirrorAdjust = (unsigned short)((aLastFrameFacing - (unsigned short)(int)(10430.378f * nlATan2f(v3RootTrans.f.y, v3RootTrans.f.x))) << 1);
    }

    {
        float fCos;
        float fSin;

        nlSinCos(&fSin, &fCos, aMirrorAdjust - aLastFrameFacing);

        pV3RootTransLocal->f.x = v3RootTrans.f.x * fCos - v3RootTrans.f.y * fSin;
        pV3RootTransLocal->f.y = v3RootTrans.f.y * fCos + v3RootTrans.f.x * fSin;
        pV3RootTransLocal->f.z = v3RootTrans.f.z;
        *fAccumulatedWeight += fNodeWeight;
    }

    if (*fAccumulatedWeight != 0.0f)
    {
        fBlendPercent = fNodeWeight / *fAccumulatedWeight;
        pRootTrans->f.x = (1.0f - fBlendPercent) * pRootTrans->f.x + fBlendPercent * pV3RootTransLocal->f.x;
        pRootTrans->f.y = (1.0f - fBlendPercent) * pRootTrans->f.y + fBlendPercent * pV3RootTransLocal->f.y;
        pRootTrans->f.z = (1.0f - fBlendPercent) * pRootTrans->f.z + fBlendPercent * pV3RootTransLocal->f.z;
    }
}

/**
 * Offset/Address/Size: 0x1F4 | 0x801EA850 | size: 0x168
 */
void cPN_SAnimController::BlendRootRot(unsigned short* pOutRot, float weight, float* pAccumWeight)
{
    unsigned short deltaRot;

    float currTime = m_fTime;
    float prevTime = m_fPrevTime;

    if (currTime < prevTime)
    {
        unsigned short rot1, rot2, rot3, rot4;
        m_pSAnim->GetRootRot(prevTime, &rot1);
        m_pSAnim->GetRootRot(1.0f, &rot2);
        m_pSAnim->GetRootRot(0.0f, &rot3);
        m_pSAnim->GetRootRot(m_fTime, &rot4);
        deltaRot = (rot2 - rot1) + (rot4 - rot3);
    }
    else
    {
        unsigned short rot1, rot2;
        m_pSAnim->GetRootRot(currTime, &rot2);
        m_pSAnim->GetRootRot(m_fPrevTime, &rot1);
        deltaRot = rot2 - rot1;
    }

    if (m_bMirror != 0)
    {
        deltaRot *= -1;
    }

    *pAccumWeight += weight;
    float w = *pAccumWeight;

    if (w != 0.0f)
    {
        float blendFactor = weight / *pAccumWeight;
        short signedDelta = deltaRot - *pOutRot;
        float blendedDelta = blendFactor * signedDelta;

        *pOutRot = *pOutRot + (int)blendedDelta;
    }
}

/**
 * Offset/Address/Size: 0x114 | 0x801EA770 | size: 0xE0
 */
void cPN_SAnimController::ProcessCallbacks()
{
    bool shouldTrigger;

    if (m_bIgnoreTriggers)
    {
        return;
    }

    cSAnimCallback* pCallback = m_pSAnim->GetCallbackList();

    while (pCallback != nullptr)
    {
        float callbackTime = pCallback->m_fTime;

        if (callbackTime == 0.0f)
        {
            extern float __float_min[];
            callbackTime = __float_min[0];
        }

        float currTime = m_fTime;
        float prevTime = m_fPrevTime;

        if (currTime < prevTime)
        {
            shouldTrigger = false;
            if (callbackTime <= currTime || callbackTime > prevTime)
            {
                shouldTrigger = true;
            }
        }
        else
        {
            shouldTrigger = false;
            if (callbackTime <= currTime && callbackTime > prevTime)
            {
                shouldTrigger = true;
            }
        }

        if (shouldTrigger)
        {
            pCallback->m_funcCallback(pCallback->m_nParam1);
        }

        pCallback = pCallback->next;
    }
}

/**
 * Offset/Address/Size: 0xB0 | 0x801EA70C | size: 0x64
 */
bool cPN_SAnimController::TestTrigger(float normalizedTime) const
{
    bool isTriggering;

    if (normalizedTime == 0.0f)
    {
        extern float __float_min[];
        normalizedTime = __float_min[0];
    }

    float currTime = m_fTime;
    float prevTime = m_fPrevTime;

    if (currTime < prevTime)
    {
        isTriggering = 0;
        if ((normalizedTime <= currTime) || (normalizedTime > prevTime))
        {
            isTriggering = 1;
        }
    }
    else
    {
        isTriggering = 0;
        if (normalizedTime <= currTime && normalizedTime > prevTime)
        {
            isTriggering = 1;
        }
    }
    return isTriggering;
}

/**
 * Offset/Address/Size: 0x20 | 0x801EA67C | size: 0x90
 */
bool cPN_SAnimController::TestFrameTrigger(float frame)
{
    bool isTriggering;
    float normalizedTime = frame / (float)m_pSAnim->m_nNumKeys;

    if (normalizedTime == 0.0f)
    {
        extern float __float_min[];
        normalizedTime = __float_min[0];
    }

    float currTime = m_fTime;
    float prevTime = m_fPrevTime;

    if (currTime < prevTime)
    {
        isTriggering = 0;
        if (normalizedTime <= currTime || normalizedTime > prevTime)
        {
            isTriggering = 1;
        }
    }
    else
    {
        isTriggering = 0;
        if ((normalizedTime <= currTime) && (normalizedTime > prevTime))
        {
            isTriggering = 1;
        }
    }
    return isTriggering;
}

/**
 * Offset/Address/Size: 0x0 | 0x801EA65C | size: 0x20
 */
int cPN_SAnimController::RemapNode(int nodeIndex) const
{
    int remappedIndex = nodeIndex;
    if (m_pAnimRetarget != NULL)
    {
        remappedIndex = m_pAnimRetarget->m_pMap[remappedIndex];
    }
    return remappedIndex;
}
