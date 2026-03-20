#include "Game/Replay.h"
#include "NL/nlConfig.h"
#include "NL/nlLexicalCast.h"

SlotPool<Replay::Frame> Replay::Frame::mSlotPool;

namespace
{
bool renderMemoryLayout = false;
}

struct ReplayDestructorChain
{
    ReplayDestructorChain* next;
    void* destructor;
    void* object;
};

extern "C"
{
    void __ct__12SlotPoolBaseFv(void*);
    void* __register_global_object(void* object, void* destructor, void* registration);
    void ReplayFrameSlotPoolDtor(void* obj, int)
    {
        ((SlotPool<Replay::Frame>*)obj)->~SlotPool<Replay::Frame>();
    }
}

/**
 * Offset/Address/Size: 0x964 | 0x80214210 | size: 0x68
 * TODO: 87.69% match - extra addic./beq null-check before SlotPoolBase ctor
 * and unresolved dtor/registration symbols (@228 and
 * __dt__25SlotPool<Q26Replay5Frame>Fv).
 */
extern "C" void __sinit_Replay_cpp()
{
    static ReplayDestructorChain chain;
    SlotPool<Replay::Frame>* pool = &Replay::Frame::mSlotPool;

    if (pool)
    {
        __ct__12SlotPoolBaseFv(pool);
    }

    pool->m_Initial = 0x400;
    SlotPoolBase::BaseAddNewBlock(pool, 0x1C);
    pool->m_Delta = 0x80;
    __register_global_object(pool, (void*)ReplayFrameSlotPoolDtor, &chain);
}

static inline Replay::Frame* ReplayAllocFrame(char* memory, int memorySize)
{
    Replay::Frame* frame = nullptr;
    SlotPool<Replay::Frame>* pool = &Replay::Frame::mSlotPool;

    if (pool->m_FreeList == nullptr)
    {
        SlotPoolBase::BaseAddNewBlock(pool, sizeof(Replay::Frame));
    }

    if (pool->m_FreeList != nullptr)
    {
        frame = (Replay::Frame*)pool->m_FreeList;
        pool->m_FreeList = ((SlotPoolEntry*)frame)->m_next;
    }

    if (frame != nullptr)
    {
        frame->mTime = 0.0f;
        frame->mBegin = memory;
        frame->mSize = memorySize;
        frame->mInterval = 0;
        frame->mEvents = 0;
        frame->mReelIdx = -1;
        frame->mNext = nullptr;
    }

    return frame;
}

/**
 * Offset/Address/Size: 0x7C8 | 0x80214074 | size: 0x188
 */
Replay::Replay(char* memory, int memorySize, int maxFrameSize)
    : mFree(ReplayAllocFrame(memory, memorySize))
{
    mReelIdx = 0;
    mTick = 0;
    mMemorySize = memorySize;
    mMaxFrameSize = maxFrameSize;
    mActualMaxFrameSize = 0;

    mFree->mNext = mFree;
    Frame* freeFrame = mFree;
    mReels[0].mLast = freeFrame;
    mReels[0].mBegin = freeFrame;

    register Config& config = Config::Global();
    renderMemoryLayout = GetConfigBool(config, "draw_replay_bar", false);
}

/**
 * Offset/Address/Size: 0x740 | 0x80213FEC | size: 0x88
 */
Replay::~Replay()
{
    Frame* frame = mFree;

    do
    {
        Frame* next = frame->mNext;
        ((SlotPoolEntry*)frame)->m_next = Frame::mSlotPool.m_FreeList;
        Frame::mSlotPool.m_FreeList = (SlotPoolEntry*)frame;
        frame = next;
    } while (frame != mFree);

    SlotPoolBase::BaseFreeBlocks(&Frame::mSlotPool, sizeof(Frame));
}

/**
 * Offset/Address/Size: 0x714 | 0x80213FC0 | size: 0x2C
 */
Replay::Frame::Frame(char* begin, int size, Frame* next)
{
    mTime = 0.0f;
    mBegin = begin;
    mSize = size;
    mInterval = 0;
    mEvents = 0;
    mReelIdx = -1;
    mNext = next;
}

/**
 * Offset/Address/Size: 0x6AC | 0x80213F58 | size: 0x68
 */
Replay::Frame* Replay::Next(Replay::Frame* frame, int reelIdx) const
{
    Frame* current = frame->mNext;

    while (current != mReels[reelIdx].mBegin)
    {
        if (current->mReelIdx == reelIdx)
        {
            return current;
        }

        if (reelIdx == 0 && current->mReelIdx > 0)
        {
            if (frame->mTime <= current->mTime)
            {
                return current;
            }
        }

        current = current->mNext;
    }

    return nullptr;
}

inline Replay::Frame* GetFrame(const Replay::Reel* mReels, int reelIdx)
{
    return mReels[reelIdx].mBegin;
}

/**
 * Offset/Address/Size: 0x618 | 0x80213EC4 | size: 0x94
 */
float Replay::TimeOfLastOccurence(unsigned int events) const
{
    Frame* frame = GetFrame(mReels, mReelIdx);
    float time = frame->mTime;

    while (frame != nullptr)
    {
        if ((frame->mEvents & events) != 0)
        {
            time = frame->mTime;
        }
        frame = Next(frame, mReelIdx);
    }

    return time;
}

/**
 * Offset/Address/Size: 0x49C | 0x80213D48 | size: 0x17C
 * TODO: 98.63% match - r4/r5 register swap for mFree->mBegin/mSize loads
 * in merge section. Likely -inline deferred vs -inline auto register
 * allocation difference.
 */
void Replay::NewFrame()
{
    while (mFree->mSize < mMaxFrameSize)
    {
        Frame* next = mFree->mNext;

        if (next->mReelIdx > 0)
        {
            do
            {
                if (mReels[0].mBegin == mFree->mNext)
                {
                    mReels[0].mBegin = Next(mReels[0].mBegin, 0);
                }
                mFree = mFree->mNext;
            } while (mFree->mReelIdx > 0);
            mFree->mReelIdx = -1;
        }
        else
        {
            if (mReels[0].mBegin == next)
            {
                mReels[0].mBegin = Next(mReels[0].mBegin, 0);
            }

            if (mFree->mBegin + mFree->mSize == mFree->mNext->mBegin)
            {
                Frame* nextFrame = mFree->mNext;
                mFree->mSize += nextFrame->mSize;
                mFree->mNext = nextFrame->mNext;
                mFree->mReelIdx = -1;
                ((SlotPoolEntry*)nextFrame)->m_next = Frame::mSlotPool.m_FreeList;
                Frame::mSlotPool.m_FreeList = (SlotPoolEntry*)nextFrame;
            }
            else
            {
                mFree = mFree->mNext;
                mFree->mReelIdx = -1;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x480 | 0x80213D2C | size: 0x1C
 */
bool Replay::IsReelValid(int reelIdx) const
{
    return mReels[reelIdx].mBegin != nullptr;
}

/**
 * Offset/Address/Size: 0x3D4 | 0x80213C80 | size: 0xAC
 */
bool Replay::DidOccurInLastNumSeconds(unsigned int events, float seconds) const
{
    float timeThreshold = mReels[mReelIdx].mLast->mTime - seconds;
    Frame* frame = GetFrame(mReels, mReelIdx);

    while (frame != nullptr)
    {
        if (frame->mTime >= timeThreshold)
        {
            if ((frame->mEvents & events) != 0)
            {
                return true;
            }
        }

        frame = Next(frame, mReelIdx);
    }

    return false;
}

/**
 * Offset/Address/Size: 0x38 | 0x802138E4 | size: 0x39C
 * TODO: 96.81% match (repo). Register allocation diffs from -inline deferred:
 * r9/r10 swap (first do-while), r9/r12 swap (inner Next loops), r8/r10
 * (post-valid frame loops). Also bgt- vs ble-+inline-return at size check.
 */
bool Replay::LockReel(float numSeconds, int idx, int quality)
{
    if (mReels[idx].mAge > quality)
    {
        return false;
    }

    Frame* frame = mFree->mNext;
    float beginTime = mReels[mReelIdx].mLast->mTime - numSeconds;
    bool valid;

    Frame* iter = frame;
    do
    {
        if (iter->mTime >= beginTime)
        {
            if (iter->mReelIdx > 0)
            {
                if (mReels[iter->mReelIdx].mAge > quality)
                {
                    valid = false;
                    goto afterLoops;
                }
            }
        }

        iter = iter->mNext;
    } while (mFree != iter);

    do
    {
        if (frame->mTime >= beginTime)
        {
            int reelIdx = frame->mReelIdx;
            if (reelIdx > 0)
            {
                Frame* r = mReels[0].mBegin;

                while (r != nullptr)
                {
                    if (r->mReelIdx == reelIdx)
                    {
                        r->mReelIdx = 0;
                    }

                    r = Next(r, 0);
                }

                r = mFree->mNext;
                while (r != mFree)
                {
                    if (r->mReelIdx == reelIdx)
                    {
                        r->mReelIdx = -1;
                    }

                    r = r->mNext;
                }

                mReels[reelIdx].mBegin = nullptr;
                mReels[reelIdx].mLast = nullptr;
            }
        }

        frame = frame->mNext;
    } while (frame != mFree);

    valid = true;

afterLoops:
    if (!valid)
    {
        return false;
    }

    frame = mReels[0].mBegin;
    while (frame != nullptr)
    {
        if (frame->mReelIdx == idx)
        {
            frame->mReelIdx = 0;
        }

        frame = Next(frame, 0);
    }

    frame = mFree->mNext;
    while (frame != mFree)
    {
        if (frame->mReelIdx == idx)
        {
            frame->mReelIdx = -1;
        }

        frame = frame->mNext;
    }

    mReels[idx].mBegin = nullptr;
    mReels[idx].mLast = nullptr;

    int size = 0;
    beginTime = mReels[mReelIdx].mLast->mTime - numSeconds;
    frame = mReels[0].mBegin;
    while (frame != nullptr)
    {
        if (frame->mTime >= beginTime)
        {
            size += frame->mSize;
        }

        frame = Next(frame, 0);
    }

    if (size > (int)(0.75f * (float)(mMemorySize / 4)))
    {
        return false;
    }

    mReels[idx].mAge = quality;
    frame = mReels[0].mBegin;
    while (frame != nullptr)
    {
        if (frame->mTime >= beginTime)
        {
            frame->mReelIdx = idx;
            if (mReels[idx].mBegin == nullptr)
            {
                mReels[idx].mBegin = frame;
            }
            mReels[idx].mLast = frame;
        }

        frame = Next(frame, 0);
    }

    return true;
}

/**
 * Offset/Address/Size: 0x20 | 0x802138CC | size: 0x18
 */
float Replay::BeginTime() const
{
    return mReels[mReelIdx].mBegin->mTime;
}

/**
 * Offset/Address/Size: 0x8 | 0x802138B4 | size: 0x18
 */
float Replay::EndTime() const
{
    return mReels[mReelIdx].mLast->mTime;
}

/**
 * Offset/Address/Size: 0x0 | 0x802138AC | size: 0x8
 */
void Replay::PlayReel(int reelIdx)
{
    mReelIdx = reelIdx;
}
