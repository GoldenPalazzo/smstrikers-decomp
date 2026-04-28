#ifndef _REPLAY_H_
#define _REPLAY_H_

#include <string.h>
#include "NL/nlMath.h"
#include "NL/nlSlotPool.h"

// Forward declarations
class DrawableCharacter;
class DrawableBall;
class DrawableExplosionFragment;
class DrawablePowerup;
class CrowdManager;
class EmissionManager;
class DrawableNetMesh;
class EmissionController;
class RenderSnapshot;

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

class SaveFrame
{
public:
    /* 0x0 */ int mInterval;
    /* 0x4 */ WriteByteStream mStream;
}; // total size: 0xC

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

class LoadFrame
{
public:
    template <int N, typename T>
    void Replayable(T& current);

    template <int N, typename T>
    void Replayable(T& current, NotReplayablePod);

    template <int N, typename T>
    void ReplayablePolymorphicPtr(T*& ptr);

    /* 0x0 */ int mInterval;
    /* 0x4 */ ReadByteStream mStream;
    /* 0xC */ ReplayNonBlendables mReplayNonBlendables;
    /* 0x10 */ float mNonBlendableAheadOfFrame;
}; // total size: 0x14

template <int N, typename T>
void LoadFrame::Replayable(T& current)
{
    NotReplayablePod pod;
    Replayable<N>(current, pod);
}

template <int MIN, int MAX, int BITS>
class FloatCompressor
{
public:
    FloatCompressor(float& f);

    /* 0x0 */ float& mF;
}; // total size: 0x4

template <int MIN, int MAX, int BITS>
inline FloatCompressor<MIN, MAX, BITS>::FloatCompressor(float& f)
    : mF(f)
{
}

// Forward declaration of generic template (needed before specializations)
template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& manager);

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, const T& manager);

template <>
void Replayable<1, LoadFrame, CrowdManager>(LoadFrame& frame, CrowdManager& manager);

template <>
void Replayable<1, SaveFrame, CrowdManager>(SaveFrame& frame, CrowdManager& manager);

template <>
void Replayable<3, LoadFrame, EmissionManager>(LoadFrame& frame, EmissionManager& manager);

template <>
void Replayable<3, SaveFrame, EmissionManager>(SaveFrame& frame, EmissionManager& manager);

template <>
void Replayable<0, LoadFrame, bool>(LoadFrame& frame, bool& value);
template <>
void Replayable<1, LoadFrame, bool>(LoadFrame& frame, bool& value);
template <>
void Replayable<0, SaveFrame, bool>(SaveFrame& frame, bool& value);
template <>
void Replayable<1, SaveFrame, bool>(SaveFrame& frame, bool& value);
template <>
void Replayable<1, LoadFrame, char>(LoadFrame& frame, char& value);
template <>
void Replayable<1, SaveFrame, char>(SaveFrame& frame, char& value);
template <>
void Replayable<1, LoadFrame, nlVector3>(LoadFrame& frame, nlVector3& value);
template <>
void Replayable<1, SaveFrame, nlVector3>(SaveFrame& frame, nlVector3& value);

template <>
void Replayable<1, LoadFrame, int>(LoadFrame& frame, int& value);
template <>
void Replayable<1, SaveFrame, int>(SaveFrame& frame, int& value);

template <>
void Replayable<0, SaveFrame, float>(SaveFrame& frame, float& value);
template <>
void Replayable<0, LoadFrame, float>(LoadFrame& frame, float& value);

template <>
void Replayable<0, SaveFrame, int>(SaveFrame& frame, int& value);
template <>
void Replayable<0, LoadFrame, int>(LoadFrame& frame, int& value);

template <>
void Replayable<0, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value);
template <>
void Replayable<0, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value);
template <>
void Replayable<0, SaveFrame, EmissionController>(SaveFrame& frame, EmissionController& controller);
template <>
void Replayable<0, LoadFrame, EmissionController>(LoadFrame& frame, EmissionController& controller);

template <>
void Replayable<3, LoadFrame, bool>(LoadFrame& frame, bool& value);
template <>
void Replayable<3, SaveFrame, bool>(SaveFrame& frame, bool& value);
template <>
void Replayable<3, LoadFrame, char>(LoadFrame& frame, char& value);
template <>
void Replayable<3, SaveFrame, char>(SaveFrame& frame, char& value);
template <>
void Replayable<3, LoadFrame, float>(LoadFrame& frame, float& value);
template <>
void Replayable<3, SaveFrame, float>(SaveFrame& frame, float& value);
template <>
void Replayable<3, LoadFrame, unsigned short>(LoadFrame& frame, unsigned short& value);
template <>
void Replayable<3, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value);
template <>
void Replayable<3, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value);
template <>
void Replayable<3, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value);
template <>
void Replayable<3, LoadFrame, nlVector3>(LoadFrame& frame, nlVector3& value);
template <>
void Replayable<3, SaveFrame, nlVector3>(SaveFrame& frame, nlVector3& value);
template <>
void Replayable<3, LoadFrame, nlQuaternion>(LoadFrame& frame, nlQuaternion& value);
template <>
void Replayable<3, SaveFrame, nlQuaternion>(SaveFrame& frame, nlQuaternion& value);

template <>
void Replayable<1, SaveFrame, unsigned char>(SaveFrame& frame, unsigned char& value);
template <>
void Replayable<1, LoadFrame, unsigned char>(LoadFrame& frame, unsigned char& value);
template <>
void Replayable<1, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value);
template <>
void Replayable<1, LoadFrame, unsigned short>(LoadFrame& frame, unsigned short& value);
template <>
void Replayable<1, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value);
template <>
void Replayable<1, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value);

template <int N, typename FrameType, typename T>
void ReplayablePolymorphic(FrameType& frame, T*& ptr);

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& drawable)
{
    drawable.Replay(frame);
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
            // mQuality = 0;
            mBegin = nullptr;
            mLast = nullptr;
            mAge = 0;
        };

        /* 0x0 */ Frame* mBegin;
        /* 0x4 */ Frame* mLast;
        /* 0x8 */ int mAge; // or float?
    }; // total size: 0xC (corrected from 0x10)

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

    /* 0x00 */ Frame* mFree;            // offset 0x0, size 0x4
    /* 0x04 */ Reel mReels[4];          // offset 0x4, size 0x40
    /* 0x34 */ int mReelIdx;            // offset 0x44, size 0x4
    /* 0x38 */ int mTick;               // offset 0x48, size 0x4
    /* 0x3C */ int mMemorySize;         // offset 0x4C, size 0x4
    /* 0x40 */ int mMaxFrameSize;       // offset 0x50, size 0x4
    /* 0x44 */ int mActualMaxFrameSize; // offset 0x54, size 0x4
    /* 0x48 */ Reel* mHighlights[3];    // offset 0x58, size 0xC

}; // total size: 0x64

#endif // _REPLAY_H_
