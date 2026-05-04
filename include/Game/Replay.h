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
class cPoseNode;

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

template <int N>
void Replayable(LoadFrame& frame, char typeId, cPoseNode*& poseNode);

template <int N, typename T>
void LoadFrame::ReplayablePolymorphicPtr(T*& current)
{
    unsigned char notNull = 1;
    memcpy(&notNull, mStream.mStorage, 1);
    mStream.mStorage++;

    if (notNull)
    {
        char typeId = 0;
        memcpy(&typeId, mStream.mStorage, 1);
        mStream.mStorage++;

        if (typeId < 0 || typeId > 4)
            nlBreak();

        ::Replayable<N>(*this, typeId, current);
    }
    else
    {
        current = 0;
    }
}

template <int MIN, int MAX, int BITS>
class FloatCompressor
{
public:
    FloatCompressor(float& f);
    inline unsigned int Read(LoadFrame& frame) const;
    inline unsigned int Read(SaveFrame& frame) const;
    inline void Transfer(LoadFrame& frame, unsigned int& value) const;
    inline void Transfer(SaveFrame& frame, unsigned int& value) const;
    inline void Apply(LoadFrame& frame, unsigned int value) const;
    inline void Apply(SaveFrame& frame, unsigned int value) const;
    inline void Replay(LoadFrame& frame) const;
    inline void Replay(SaveFrame& frame) const;

    /* 0x0 */ float& mF;
}; // total size: 0x4

template <>
FloatCompressor<0, 1, 15>::FloatCompressor(float& f);

template <int MIN, int MAX, int BITS>
inline FloatCompressor<MIN, MAX, BITS>::FloatCompressor(float& f)
    : mF(f)
{
}

template <int MIN, int MAX, int BITS>
inline unsigned int FloatCompressor<MIN, MAX, BITS>::Read(LoadFrame& frame) const
{
    return 0;
}

template <int MIN, int MAX, int BITS>
inline unsigned int FloatCompressor<MIN, MAX, BITS>::Read(SaveFrame& frame) const
{
    float f = mF;
    if (f > (float)MAX)
        f = (float)MAX;
    if (f < (float)MIN)
        f = (float)MIN;
    f -= (float)MIN;
    f *= (float)(1 << BITS);
    return (unsigned int)f;
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Transfer(LoadFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        const char* cursor = frame.mStream.mStorage;
        value = (unsigned int)(unsigned char)*cursor++;
        frame.mStream.mStorage = cursor;
    }
    else if ((MAX - MIN) * (1 << BITS) <= 65535)
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char hi = (unsigned char)cursor[1];
        unsigned char lo = (unsigned char)cursor[0];
        cursor += 2;
        value = (unsigned int)hi << 8;
        frame.mStream.mStorage = cursor;
        value |= (unsigned int)lo;
    }
    else
    {
        const char* cursor = frame.mStream.mStorage;
        unsigned char mid = (unsigned char)cursor[1];
        unsigned char hi = (unsigned char)cursor[2];
        unsigned char lo = (unsigned char)cursor[0];
        cursor += 3;
        frame.mStream.mStorage = cursor;
        value = ((unsigned int)hi << 16) | ((unsigned int)mid << 8) | (unsigned int)lo;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Transfer(SaveFrame& frame, unsigned int& value) const
{
    if ((MAX - MIN) * (1 << BITS) <= 255)
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)value;
        frame.mStream.mStorage = p;
    }
    else
    {
        char* p = frame.mStream.mStorage;
        *p++ = (char)(value & 0xFF);
        *p++ = (char)((value >> 8) & 0xFF);
        frame.mStream.mStorage = p;
    }
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Apply(LoadFrame& frame, unsigned int value) const
{
    mF = (float)value / (float)(1 << BITS);
    mF += (float)MIN;
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Apply(SaveFrame& frame, unsigned int value) const
{
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Replay(LoadFrame& frame) const
{
    unsigned int value = Read(frame);
    Transfer(frame, value);
    Apply(frame, value);
}

template <int MIN, int MAX, int BITS>
inline void FloatCompressor<MIN, MAX, BITS>::Replay(SaveFrame& frame) const
{
    unsigned int value = Read(frame);
    Transfer(frame, value);
    Apply(frame, value);
}

// Forward declaration of generic template (needed before specializations)
template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, T& manager);

template <int N, typename FrameType, typename T>
void Replayable(FrameType& frame, const T& proxy)
{
    FORCE_DONT_INLINE;
    if (N == 0 || frame.mInterval == N)
    {
        unsigned int value = proxy.Read(frame);
        if (N == 0 || frame.mInterval == N)
        {
            proxy.Transfer(frame, value);
        }
        proxy.Apply(frame, value);
    }
}

template <>
inline void Replayable<1, LoadFrame, FloatCompressor<-512, 512, 8> >(LoadFrame& frame, const FloatCompressor<-512, 512, 8>& proxy)
{
    if (frame.mInterval == 1)
    {
        unsigned int value = 0;
        if (frame.mInterval == 1)
        {
            const char* cursor = frame.mStream.mStorage;
            unsigned char mid = (unsigned char)cursor[1];
            unsigned char hi = (unsigned char)cursor[2];
            unsigned char lo = (unsigned char)cursor[0];
            unsigned int hiShift = (unsigned int)hi << 16;
            value = (unsigned int)mid << 8;
            value |= (unsigned int)lo;
            frame.mStream.mStorage = cursor + 3;
            value |= hiShift;
        }
        proxy.mF = (float)value / (float)(1 << 8);
        proxy.mF += (float)-512;
    }
}

template <>
inline void Replayable<1, LoadFrame, FloatCompressor<-128, 128, 8> >(LoadFrame& frame, const FloatCompressor<-128, 128, 8>& proxy)
{
    if (frame.mInterval == 1)
    {
        unsigned int value = 0;
        if (frame.mInterval == 1)
        {
            const char* cursor = frame.mStream.mStorage;
            unsigned char mid = (unsigned char)cursor[1];
            unsigned char hi = (unsigned char)cursor[2];
            unsigned char lo = (unsigned char)cursor[0];
            unsigned int hiShift = (unsigned int)hi << 16;
            value = (unsigned int)mid << 8;
            value |= (unsigned int)lo;
            frame.mStream.mStorage = cursor + 3;
            value |= hiShift;
        }
        proxy.mF = (float)value / (float)(1 << 8);
        proxy.mF += (float)-128;
    }
}

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
void Replayable<0, LoadFrame, char>(LoadFrame& frame, char& value);
template <>
void Replayable<0, SaveFrame, char>(SaveFrame& frame, char& value);
template <>
void Replayable<0, SaveFrame, int>(SaveFrame& frame, int& value);
template <>
void Replayable<0, LoadFrame, int>(LoadFrame& frame, int& value);

template <>
void Replayable<0, SaveFrame, unsigned int>(SaveFrame& frame, unsigned int& value);
template <>
void Replayable<0, LoadFrame, unsigned int>(LoadFrame& frame, unsigned int& value);
template <>
void Replayable<0, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value);
template <>
void Replayable<0, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value);
template <>
void Replayable<0, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value);
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
