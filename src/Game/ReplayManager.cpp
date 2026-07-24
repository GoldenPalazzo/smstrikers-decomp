#include "Game/ReplayManager.h"
#include "Game/Camera/CameraMan.h"
#include "Game/FixedUpdateTask.h"
#include "NL/nlTask.h"
#include "NL/nlMemory.h"
#include "NL/globalpad.h"
#include "NL/nlConfig.h"
#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"

extern float g_fSimulationTick;
extern float g_fFixedUpdateTick;
extern bool g_bEnableGamecubePadMonkey;
extern bool g_bTweaking;
extern bool g_bProfiling;

static f32 CANT_COLLIDE = *(f32*)__float_max;

#pragma inline_max_size(0)
#pragma inline_max_total_size(0)

/**
 * Offset/Address/Size: 0x1A4 | 0x80112AB4 | size: 0x1B8
 */
template <typename T>
void Replay::Play(float time, T& previous, T& current, float* blend) const
{
    if (time < BeginTime())
    {
        time = BeginTime();
    }
    if (time > EndTime())
    {
        time = EndTime();
    }

    int interval;
    Frame* rhs;
    Frame* lhs;
    Frame* tryLhs;
    LoadFrame previousLoadFrame;
    LoadFrame currentLoadFrame;

    for (interval = 1; interval <= 3; interval++)
    {
        rhs = mReels[mReelIdx].mBegin;
        while (rhs != NULL)
        {
            if (rhs->mInterval == interval && rhs->mTime > time)
            {
                lhs = NULL;
                tryLhs = mReels[mReelIdx].mBegin;
                while (tryLhs != NULL)
                {
                    if (tryLhs->mInterval == interval && tryLhs->mTime <= time)
                    {
                        lhs = tryLhs;
                    }
                    tryLhs = Next(tryLhs, mReelIdx);
                }

                if (lhs != NULL && rhs != NULL)
                {
                    blend[interval - 1] = (time - lhs->mTime) / (rhs->mTime - lhs->mTime);

                    float aheadOfFrame = time - lhs->mTime;
                    char* lhsBegin = lhs->mBegin;
                    previousLoadFrame.mInterval = interval;
                    previousLoadFrame.mStream.mCount = 0;
                    previousLoadFrame.mStream.mStorage = lhsBegin;
                    previousLoadFrame.mReplayNonBlendables = REPLAY_NON_BLENDABLES;
                    previousLoadFrame.mNonBlendableAheadOfFrame = aheadOfFrame;
                    Replayable<0>(previousLoadFrame, previous);

                    char* rhsBegin = rhs->mBegin;
                    currentLoadFrame.mInterval = interval;
                    currentLoadFrame.mStream.mCount = 0;
                    currentLoadFrame.mStream.mStorage = rhsBegin;
                    currentLoadFrame.mReplayNonBlendables = DO_NOT_REPLAY_NON_BLENDABLES;
                    currentLoadFrame.mNonBlendableAheadOfFrame = 0.0f;
                    Replayable<0>(currentLoadFrame, current);

                    break;
                }
            }
            rhs = Next(rhs, mReelIdx);
        }
    }
}

#pragma inline_max_size(256)
#pragma inline_max_total_size(10000)

/**
 * Offset/Address/Size: 0x3C | 0x8011294C | size: 0x168
 */
template <typename T>
void Replay::Record(float time, T& snapshot, unsigned int events)
{
    for (int interval = 1; interval <= 3; interval++)
    {
        if (mTick % interval == 0)
        {
            NewFrame();

            SaveFrame frame;
            char* storage = mFree->mBegin;
            frame.mInterval = interval;
            frame.mStream.mCount = 0;
            frame.mStream.mStorage = storage;
            Replayable<0>(frame, snapshot);

            int frameSize = frame.mStream.mStorage - mFree->mBegin;
            if (mActualMaxFrameSize < frameSize)
            {
                mActualMaxFrameSize = frameSize;
            }

            mReels[0].mLast = mFree;
            mFree->mReelIdx = 0;
            mFree->mTime = time;
            mFree->mInterval = interval;
            mFree->mEvents = events;

            Frame* newFrame = Frame::mSlotPool.Allocate();
            newFrame = new (newFrame) Frame(mFree->mBegin + frameSize, mFree->mSize - frameSize, mFree->mNext);

            mFree->mNext = newFrame;
            mFree->mSize = frameSize;
            mFree = mFree->mNext;
        }
    }

    mTick++;
}

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

ReplayManager::ReplayManager()
    : mCurrent(mSnapshots)
    , mPrevious(mSnapshots + 1)
    , mRender(NULL)
    , mDebugCamera(cFollowCamera::FOLLOW_SELECTABLE)
    , mEvents(0)
    , mSpeed(1.0f)
    , mSpeedUp(0.0f)
    , mDeltaTime(0.0f)
    , mTime(0.0f)
    , mReplay(NULL)
    , mMemory(NULL)
{
}

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
    static ReplayManager rm;
    return &rm;
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
            mDeltaTime -= 0.02f * cPadManager::GetPad(0)->GetPressure(5, true);
            mDeltaTime += 0.02f * cPadManager::GetPad(0)->GetPressure(6, true);

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
        if (mSpeed < 0.1f)
        {
            mSpeed = 0.1f;
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
