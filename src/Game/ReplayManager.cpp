#include "Game/ReplayManager.h"
#include "Game/Camera/CameraMan.h"
#include "Game/FixedUpdateTask.h"
#include "NL/nlTask.h"
#include "NL/nlMemory.h"
#include "NL/globalpad.h"
#include "NL/nlConfig.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"
#include "PowerPC_EABI_Support/Runtime/global_destructor_chain.h"

extern float g_fSimulationTick;
extern float g_fFixedUpdateTick;
extern bool g_bEnableGamecubePadMonkey;
extern bool g_bTweaking;
extern bool g_bProfiling;

// Forward declarations for constructor/destructor functions
extern "C"
{
    void __ct__14RenderSnapshotFv(void*, int);
    void __dt__14RenderSnapshotFv(void*, int);
    void __dt__13ReplayManagerFv(void*, int);
}

namespace
{
static bool sInitialized = false;
static char sInstanceStorage[sizeof(ReplayManager)];
static ReplayManager* sInstance = reinterpret_cast<ReplayManager*>(sInstanceStorage);
static DestructorChain sDestructorChain;
} // namespace

// /**
//  * Offset/Address/Size: 0x3B4 | 0x80112CC4 | size: 0x10
//  */
// void 0x8028D2B8..0x8028D2BC | size: 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x388 | 0x80112C98 | size: 0x2C
//  */
// void Replayable<0, LoadFrame, RenderSnapshot>(LoadFrame&, RenderSnapshot&)
// {
// }

// /**
//  * Offset/Address/Size: 0x35C | 0x80112C6C | size: 0x2C
//  */
// void Replayable<0, SaveFrame, RenderSnapshot>(SaveFrame&, RenderSnapshot&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1A4 | 0x80112AB4 | size: 0x1B8
//  */
// void Replay::Play<RenderSnapshot>(float, RenderSnapshot&, RenderSnapshot&, float*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x8011294C | size: 0x168
//  */
// void Replay::Record<RenderSnapshot>(float, RenderSnapshot&, unsigned int)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80112910 | size: 0x3C
//  */
// void Blend<RenderSnapshot>(const float*, const RenderSnapshot&, const RenderSnapshot&, RenderSnapshot&)
// {
// }

// /**
//  * Offset/Address/Size: 0x50 | 0x8011290C | size: 0x4
//  */
// void cBaseCamera::Reactivate()
// {
// }

// /**
//  * Offset/Address/Size: 0x48 | 0x80112904 | size: 0x8
//  */
// void cBaseCamera::GetFOV() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801128BC | size: 0x48
//  */
// cBaseCamera::~cBaseCamera()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80112838 | size: 0x84
//  */
// RenderSnapshot::RenderSnapshot()
// {
// }

// /**
//  * Offset/Address/Size: 0x5C | 0x801127BC | size: 0x7C
//  */
// RenderSnapshot::~RenderSnapshot()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80112760 | size: 0x5C
//  */
// cFollowCamera::~cFollowCamera()
// {
// }

/**
 * Offset/Address/Size: 0x964 | 0x801126D4 | size: 0x8C
 */
ReplayManager::~ReplayManager()
{
}

/**
 * Offset/Address/Size: 0x89C | 0x8011260C | size: 0xC8
 */
ReplayManager* ReplayManager::Instance()
{
    extern void __ct__13cFollowCameraFQ213cFollowCamera12FollowTarget(cFollowCamera*, cFollowCamera::FollowTarget);

    static s8 init;
    static char rm[sizeof(ReplayManager)];
    static DestructorChain chain;

    if (!init)
    {
        ReplayManager* instance = reinterpret_cast<ReplayManager*>(rm);

        __construct_array(instance, __ct__14RenderSnapshotFv, __dt__14RenderSnapshotFv, sizeof(RenderSnapshot), 3);
        instance->mCurrent = instance->mSnapshots;
        instance->mPrevious = instance->mSnapshots + 1;
        instance->mRender = 0;
        __ct__13cFollowCameraFQ213cFollowCamera12FollowTarget(&instance->mDebugCamera, cFollowCamera::FOLLOW_SELECTABLE);
        instance->mEvents = 0;
        instance->mSpeed = 1.0f;
        instance->mSpeedUp = 0.0f;
        instance->mDeltaTime = 0.0f;
        instance->mTime = 0.0f;
        instance->mReplay = 0;
        instance->mMemory = 0;
        __register_global_object(instance, __dt__13ReplayManagerFv, &chain);
        init = 1;
    }

    return reinterpret_cast<ReplayManager*>(rm);
}

/**
 * Offset/Address/Size: 0x828 | 0x80112598 | size: 0x74
 */
void ReplayManager::Initialize()
{
    mMemory = (u8*)nlVirtualAlloc(0x1C0000, false);
    mReplay = new (nlMalloc(0x48, 8, false)) Replay((char*)mMemory, 0x1C0000, 0x8000);
    mTime = 0.0f;
}

/**
 * Offset/Address/Size: 0x7DC | 0x8011254C | size: 0x4C
 */
void ReplayManager::InitializeSnapshots()
{
    for (int i = 0; i < 3; i++)
    {
        mSnapshots[i].Initialize();
    }
}

/**
 * Offset/Address/Size: 0x760 | 0x801124D0 | size: 0x7C
 */
void ReplayManager::Uninitialize()
{
    nlVirtualFree(mMemory);
    mMemory = nullptr;

    for (int i = 0; i < 3; i++)
    {
        mSnapshots[i].Free();
    }

    delete mReplay;
    mReplay = nullptr;
}

/**
 * Offset/Address/Size: 0x6E0 | 0x80112450 | size: 0x80
 */
void ReplayManager::GrabSnapshot()
{
    RenderSnapshot* temp = mCurrent;
    mCurrent = mPrevious;
    mPrevious = temp;

    mCurrent->Grab();

    if (nlTaskManager::m_pInstance->m_CurrState == 2)
    {
        mTime = mReplay->EndTime() + g_fSimulationTick;
        mReplay->Record<RenderSnapshot>(mTime, *mCurrent, mEvents);
        mEvents = 0;
    }
}

/**
 * Offset/Address/Size: 0x6B4 | 0x80112424 | size: 0x2C
 */
RenderSnapshot& ReplayManager::GetMutableRenderSnapshot()
{
    mRender = mCurrent;
    return mRender->GetMutable();
}

/**
 * Offset/Address/Size: 0x5C4 | 0x80112334 | size: 0xF0
 */
void ReplayManager::Flush()
{
    delete mReplay;
    mReplay = new (nlMalloc(0x48, 8, false)) Replay((char*)mMemory, 0x1C0000, 0x8000);

    int i;
    RenderSnapshot* p = mSnapshots;
    for (i = 0; i < 3; i++, p++)
    {
        p->Invalidate();
    }

    RenderSnapshot* temp = mCurrent;
    mCurrent = mPrevious;
    mPrevious = temp;

    mCurrent->Grab();

    if (nlTaskManager::m_pInstance->m_CurrState == 2)
    {
        mTime = mReplay->EndTime() + g_fSimulationTick;
        mReplay->Record<RenderSnapshot>(mTime, *mCurrent, mEvents);
        mEvents = 0;
    }
}

/**
 * Offset/Address/Size: 0x514 | 0x80112284 | size: 0xB0
 */
void ReplayManager::ResetSnapshots()
{
    for (int i = 0; i < 3; i++)
    {
        mSnapshots[i].Invalidate();
    }

    RenderSnapshot* temp = mCurrent;
    mCurrent = mPrevious;
    mPrevious = temp;

    mCurrent->Grab();

    if (nlTaskManager::m_pInstance->m_CurrState == 2)
    {
        mTime = mReplay->EndTime() + g_fSimulationTick;
        mReplay->Record<RenderSnapshot>(mTime, *mCurrent, mEvents);
        mEvents = 0;
    }
}

/**
 * Offset/Address/Size: 0x4C0 | 0x80112230 | size: 0x54
 */
void ReplayManager::PrepareForRecording()
{
    cCameraManager::Remove(mDebugCamera);
    mTime = mReplay->EndTime();
    mPrevious->Invalidate();
    mCurrent->Invalidate();
    mRender = nullptr;
}

/**
 * Offset/Address/Size: 0x454 | 0x801121C4 | size: 0x6C
 */
void ReplayManager::SetCurrentTime(float time)
{
    mTime = time;

    if (mTime < mReplay->BeginTime())
    {
        mTime = mReplay->BeginTime();
    }

    if (mTime > mReplay->EndTime())
    {
        mTime = mReplay->EndTime();
    }
}

/**
 * Offset/Address/Size: 0x3C0 | 0x80112130 | size: 0x94
 */
void ReplayManager::EventHandler(Event* event)
{
    if (event->m_uEventID == 0xD)
    {
        mEvents |= 4;
    }
    if (event->m_uEventID == 0x14)
    {
        mEvents |= 2;
    }
    if (event->m_uEventID == 0xE)
    {
        mEvents |= 8;
    }
    if (event->m_uEventID == 5)
    {
        mEvents |= 1;
    }
    if (event->m_uEventID == 0xF)
    {
        mEvents |= 0x16;
    }
    if (event->m_uEventID == 0xF)
    {
        mEvents |= 1;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x80111D70 | size: 0x3C0
 * TODO: 99.92% match - remaining diffs are `i` (SDA offset) on static local labels
 */
void ReplayManager::RenderSnapshotAt(float deltaTime)
{
    mBlend[0] = FixedUpdateTask::mAccumulatedDeltaT / g_fFixedUpdateTick;
    mBlend[1] = FixedUpdateTask::mAccumulatedDeltaT / g_fFixedUpdateTick;
    mBlend[2] = FixedUpdateTask::mAccumulatedDeltaT / g_fFixedUpdateTick;
    mDeltaTime = FixedUpdateTask::mAccumulatedDeltaT;

    if (!g_bEnableGamecubePadMonkey)
    {
        static bool debugReplay = GetConfigBool(Config::Global(), "debugReplay", false);

        if (debugReplay && !g_bTweaking && !g_bProfiling)
        {
            if (cPadManager::GetPad(0)->JustPressed(4, true))
            {
                if (nlTaskManager::m_pInstance->m_CurrState == 0x20000)
                {
                    nlTaskManager::SetNextState(2);
                }
                else if (nlTaskManager::m_pInstance->m_CurrState == 2)
                {
                    mTime = mReplay->EndTime();
                    nlTaskManager::SetNextState(0x20000);
                }
            }
        }

        if (nlTaskManager::m_pInstance->m_CurrState == 0x20000)
        {
            if (!cCameraManager::HasCamera(&mDebugCamera))
            {
                cCameraManager::PushCamera(&mDebugCamera);
            }

            mDeltaTime = 0.0f;
            mDeltaTime -= 0.5f * cPadManager::GetPad(0)->GetPressure(5, true);
            mDeltaTime += 0.5f * cPadManager::GetPad(0)->GetPressure(6, true);

            f32 newTime = mTime + mDeltaTime;
            if (newTime < mReplay->BeginTime())
            {
                newTime = mReplay->BeginTime();
            }
            if (newTime > mReplay->EndTime())
            {
                newTime = mReplay->EndTime();
            }

            mDeltaTime = newTime - mTime;
            mTime = newTime;
            mReplay->Play<RenderSnapshot>(mTime, *mPrevious, *mCurrent, mBlend);
        }
    }

    if (nlTaskManager::m_pInstance->m_CurrState == 0x10)
    {
        mSpeed = mSpeedUp * deltaTime + mSpeed;
        if (mSpeed < 0.0f)
        {
            mSpeed = 0.0f;
        }
        mDeltaTime = mSpeed * deltaTime;
        mTime = mTime + mDeltaTime;
        mReplay->Play<RenderSnapshot>(mTime, *mPrevious, *mCurrent, mBlend);
    }

    mRender = mCurrent;

    bool transitioning = (nlTaskManager::m_pInstance->m_CurrState == 0x100) || (nlTaskManager::m_pInstance->m_PrevState == 0x100 && nlTaskManager::m_pInstance->m_CurrState == 1);

    if (!transitioning && mPrevious->mValid)
    {
        Blend<RenderSnapshot>(mBlend, *mPrevious, *mCurrent, mSnapshots[2]);
        mRender = &mSnapshots[2];
    }

    mRender->Render(deltaTime);

    if (nlTaskManager::m_pInstance->m_CurrState == 0x20000)
    {
        mSnapshots[2].RenderDebugInfo(*mPrevious, *mCurrent, mBlend[0]);
    }
}
