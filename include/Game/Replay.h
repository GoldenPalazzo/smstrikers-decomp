#ifndef _REPLAY_H_
#define _REPLAY_H_

#include <string.h>
#include "NL/nlMath.h"
#include "NL/nlSlotPool.h"
#include "Game/Effects/EmissionController.h"

class DrawableCharacter;
class DrawableBall;
class DrawableExplosionFragment;
class DrawablePowerup;
class CrowdManager;
class EmissionManager;
class DrawableNetMesh;
class RenderSnapshot;
class cPoseNode;
class LoadFrame;

void nlBreak();

class WriteByteStream
{
public:
    /* 0x0 */ char mCount;
    /* 0x4 */ char* mStorage;
}; // total size: 0x8

class ReadByteStream
{
public:
    /* 0x0 */ char mCount;
    /* 0x4 */ const char* mStorage;
}; // total size: 0x8

enum ReplayNonBlendables
{
    REPLAY_NON_BLENDABLES = 0,
    DO_NOT_REPLAY_NON_BLENDABLES = 1,
};

struct ReplayablePod
{
}; // total size: 0x1

struct NotReplayablePod
{
}; // total size: 0x1

template <typename T>
struct ReplayableCategory
{
    typedef NotReplayablePod Type;
};

template <>
struct ReplayableCategory<bool>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<char>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<unsigned char>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<unsigned short>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<int>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<unsigned int>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<unsigned long>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<float>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<nlVector3>
{
    typedef ReplayablePod Type;
};

template <>
struct ReplayableCategory<nlQuaternion>
{
    typedef ReplayablePod Type;
};

class SaveFrame
{
public:
    template <int N, typename T>
    void Replayable(T& current);

    template <int N, typename T>
    void Replayable(T& current, ReplayablePod);

    template <int N>
    void Replayable(bool& current, ReplayablePod);

    template <int N, typename T>
    void Replayable(T& current, NotReplayablePod);

    template <int N, typename T>
    void ReplayablePolymorphicPtr(T*& ptr);

    /* 0x0 */ int mInterval;
    /* 0x4 */ WriteByteStream mStream;
}; // total size: 0xC

template <int N, typename T>
inline void SaveFrame::Replayable(T& current)
{
    typename ReplayableCategory<T>::Type category;
    Replayable<N>(current, category);
}

template <int N, typename T>
inline void SaveFrame::Replayable(T& current, ReplayablePod)
{
    if (N == 0 || mInterval == N)
    {
        memcpy(mStream.mStorage, &current, sizeof(T));
        mStream.mStorage += sizeof(T);
    }
}

template <int N>
inline void SaveFrame::Replayable(bool& current, ReplayablePod)
{
    char value = current ? 1 : 0;
    memcpy(mStream.mStorage, &value, 1);
    mStream.mStorage += 1;
}

template <int N, typename T>
inline void SaveFrame::Replayable(T& current, NotReplayablePod)
{
    if (N == 0 || mInterval == N)
    {
        current.Replay(*this);
    }
}

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& current);

template <int N>
void Replayable(SaveFrame& frame, char typeId, cPoseNode*& poseNode);

template <typename FrameType>
static inline void ReplayControllerFloats(FrameType& frame, EmissionController& controller);

static inline void ReplayControllerState(LoadFrame& frame, EmissionController& controller);

template <typename T>
static inline void ReplayLoadObject(LoadFrame& frame, T& current);

static inline void ReplayLoadObject(LoadFrame& frame, EmissionController& controller);

#include "Game/LoadFrame.h"

template <int N, typename T>
inline void SaveFrame::ReplayablePolymorphicPtr(T*& ptr)
{
    T* current = ptr;
    if (N == 0 || mInterval == N)
    {
        unsigned char notNull = (current != 0);
        memcpy(mStream.mStorage, &notNull, 1);
        mStream.mStorage++;

        if (notNull)
        {
            char typeId = (char)current->GetType();
            if (typeId < 0 || typeId > 4)
                nlBreak();

            memcpy(mStream.mStorage, &typeId, 1);
            mStream.mStorage++;

            ::Replayable<N>(*this, typeId, current);
        }
    }
}

#include "Game/Compressor.h"

// Proxy generic for FloatCompressor-style types (Read/Transfer/Apply).
template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, const T& proxy)
{
    if (N == 0 || frame.mInterval == N)
    {
        unsigned int value = proxy.Read(frame);
        if (N == 0)
        {
            proxy.Transfer(frame, value);
        }
        else if (frame.mInterval == N)
        {
            proxy.TransferOR(frame, value);
        }
        proxy.Apply(frame, value);
    }
}

template <typename FrameType>
static inline void ReplayControllerFloats(FrameType& frame, EmissionController& controller)
{
    const FloatCompressor<-255, 255, 6> positionX(controller.m_vPosition.f.x);
    ::Replayable<0>(frame, positionX);
    const FloatCompressor<-255, 255, 6> positionY(controller.m_vPosition.f.y);
    ::Replayable<0>(frame, positionY);
    const FloatCompressor<-255, 255, 6> positionZ(controller.m_vPosition.f.z);
    ::Replayable<0>(frame, positionZ);
    const FloatCompressor<-255, 255, 6> directionX(controller.m_vDirection.f.x);
    ::Replayable<0>(frame, directionX);
    const FloatCompressor<-255, 255, 6> directionY(controller.m_vDirection.f.y);
    ::Replayable<0>(frame, directionY);
    const FloatCompressor<-255, 255, 6> directionZ(controller.m_vDirection.f.z);
    ::Replayable<0>(frame, directionZ);
    const FloatCompressor<-255, 255, 6> velocityX(controller.m_vVelocity.f.x);
    ::Replayable<0>(frame, velocityX);
    const FloatCompressor<-255, 255, 6> velocityY(controller.m_vVelocity.f.y);
    ::Replayable<0>(frame, velocityY);
    const FloatCompressor<-255, 255, 6> velocityZ(controller.m_vVelocity.f.z);
    ::Replayable<0>(frame, velocityZ);
}

static inline void ReplayControllerState(LoadFrame& frame, EmissionController& controller)
{
    float age = 0.0f;
    ::Replayable<0>(frame, age);
    age += frame.mNonBlendableAheadOfFrame;
    controller.m_ReplayDeltaTime = age - controller.m_Age;
    controller.m_Age = age;

    unsigned int updateCb = 0;
    ::Replayable<0>(frame, updateCb);
    unsigned int callback = updateCb;
    if (callback != 0)
    {
        controller.mUpdateCallback = (void (*)(EmissionController&))callback;
    }

    unsigned int finishedCb = 0;
    ::Replayable<0>(frame, finishedCb);
    callback = finishedCb;
    if (callback != 0)
    {
        controller.mFinishedCallback = (void (*)(EmissionController&))callback;
    }
}

static inline void ReplayControllerCallbacks(SaveFrame& frame, EmissionController& controller)
{
    unsigned int updateCb = (unsigned int)controller.mUpdateCallback.GetFreeFunction();
    Replayable<0>(frame, updateCb);

    unsigned int finishedCb = (unsigned int)controller.mFinishedCallback.GetFreeFunction();
    Replayable<0>(frame, finishedCb);
}

template <typename T>
static inline void ReplayLoadObject(LoadFrame& frame, T& current)
{
    current.Replay(frame);
}

static inline void ReplayLoadObject(LoadFrame& frame, EmissionController& controller)
{
    ::Replayable<0>(frame, (unsigned int&)controller.m_pPose);
    ::Replayable<0>(frame, (unsigned int&)controller.m_pAnimController);
    memcpy(&controller.m_uUserData, frame.mStream.mStorage, sizeof(unsigned long));
    frame.mStream.mStorage += sizeof(unsigned long);
    ::Replayable<0>(frame, controller.m_fGround);
    memcpy(&controller.m_aFacing, frame.mStream.mStorage, sizeof(unsigned short));
    frame.mStream.mStorage += sizeof(unsigned short);
    ::Replayable<0>(frame, (char&)controller.m_GlView);
    ReplayControllerFloats(frame, controller);
    controller.m_Replaying = true;
    ReplayControllerState(frame, controller);
}

template <int N, typename FrameType, typename T>
static inline void ReplayFrameValue(FrameType& frame, T& current, ReplayablePod category)
{
    frame.Replayable<N>(current, category);
}

template <int N, typename FrameType, typename T>
static inline void ReplayFrameValue(FrameType& frame, T& current, NotReplayablePod)
{
    if (N == 0 || frame.mInterval == N)
    {
        current.Replay(frame);
    }
}

template <>
inline void ReplayFrameValue<0, LoadFrame, EmissionController>(LoadFrame& frame, EmissionController& controller, NotReplayablePod)
{
    frame.Replayable<0>(controller);
}

template <>
inline void ReplayFrameValue<0, SaveFrame, EmissionController>(SaveFrame& frame, EmissionController& controller, NotReplayablePod)
{
    ::Replayable<0>(frame, (unsigned int&)controller.m_pPose);
    ::Replayable<0>(frame, (unsigned int&)controller.m_pAnimController);
    memcpy(frame.mStream.mStorage, &controller.m_uUserData, sizeof(unsigned long));
    frame.mStream.mStorage += sizeof(unsigned long);
    ::Replayable<0>(frame, controller.m_fGround);
    memcpy(frame.mStream.mStorage, &controller.m_aFacing, sizeof(unsigned short));
    frame.mStream.mStorage += sizeof(unsigned short);
    ::Replayable<0>(frame, (char&)controller.m_GlView);
    ReplayControllerFloats(frame, controller);
    controller.m_Replaying = false;
    controller.m_ReplayDeltaTime = 0.0f;
    ::Replayable<0>(frame, controller.m_Age);
    ReplayControllerCallbacks(frame, controller);
}

template <int N, typename FrameType, typename T>
void ReplayablePolymorphic(FrameType& frame, T*& ptr)
{
    frame.template ReplayablePolymorphicPtr<N>(ptr);
}

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& drawable)
{
    if (N == 0 || frame.mInterval == N)
    {
        typename ReplayableCategory<T>::Type category;
        ReplayFrameValue<N, FrameType, T>(frame, drawable, category);
    }
}

enum ReplayType
{
    REPLAY_TYPE_GOAL = 6,
    REPLAY_TYPE_HIGHLIGHT = 7,
    REPLAY_TYPE_HYPER_STRIKE = 8,
    NUM_REPLAY_TYPES = 9,
};

class Replay
{
public:
    struct Frame
    {
        Frame(char* begin, int size, Frame* next);

        char* End() const { return mBegin + mSize; }

        /* 0x00 */ float mTime;
        /* 0x04 */ char* mBegin;
        /* 0x08 */ int mSize;
        /* 0x0C */ int mInterval;
        /* 0x10 */ unsigned int mEvents;
        /* 0x14 */ int mReelIdx;
        /* 0x18 */ Frame* mNext;
        static SlotPool<Frame> mSlotPool;
    }; // total size: 0x1C

    struct Reel
    {
        Reel()
        {
            mBegin = nullptr;
            mLast = nullptr;
            mAge = 0;
        };

        /* 0x0 */ Frame* mBegin;
        /* 0x4 */ Frame* mLast;
        /* 0x8 */ int mAge;
        // Do not "fix" this layout from dwarf.txt. The debug ELF's ReplayManager.cpp
        // compile unit describes a 0x10 Reel (float mAge, int mQuality, mBegin, mLast),
        // but the retail build indexes mReels with `mulli rX,rY,12`, so the shipped
        // stride is 0xC and this ordering is what reproduces LockReel exactly.
    }; // total size: 0xC

    Replay(char*, int, int);
    ~Replay();

    Frame* Next(Frame*, int) const;
    float TimeOfLastOccurence(unsigned int) const;
    void NewFrame();
    bool IsReelValid(int) const;
    bool DidOccurInLastNumSeconds(unsigned int, float) const;
    bool LockReel(float, int, int);
    float BeginTime() const;
    float EndTime() const;
    void PlayReel(int);

    template <typename T>
    void Record(float time, T& snapshot, unsigned int events);

    template <typename T>
    void Play(float time, T& previous, T& current, float* blend) const;

    /* 0x00 */ Frame* mFree;
    /* 0x04 */ Reel mReels[4];
    /* 0x34 */ int mReelIdx;
    /* 0x38 */ int mTick;
    /* 0x3C */ int mMemorySize;
    /* 0x40 */ int mMaxFrameSize;
    /* 0x44 */ int mActualMaxFrameSize;
    /* 0x48 */ Reel* mHighlights[3];

}; // total size: 0x54

#endif // _REPLAY_H_
