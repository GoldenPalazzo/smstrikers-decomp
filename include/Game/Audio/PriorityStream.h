#ifndef _PRIORITYSTREAM_H_
#define _PRIORITYSTREAM_H_

#include "Game/Audio/StreamTrack.h"
#include "NL/nlBind.h"

union PriorityFadeBits
{
    unsigned long word;
    unsigned short half;
    struct
    {
        unsigned long fadeIn : 15;
        unsigned long fadeOut : 14;
        unsigned char looping : 1;
        unsigned char queue : 1;
        unsigned char active : 1;
    } b;
};

class PriorityStream
{
public:
    class PLAY_RECORD
    {
    public:
        PLAY_RECORD(AudioStreamTrack::StreamTrack& t)
            : m_StreamId(0)
            , m_Track(t)
        {
        }

        unsigned long GetNextStreamId(unsigned long);
        void Play(bool, bool);
        void Set(unsigned long, float, bool, unsigned long, unsigned long, const char*, Audio::MasterVolume::VOLUME_GROUP, bool, bool);

        /* 0x00 */ unsigned long m_StreamId;
        /* 0x04 */ unsigned long m_OrigStreamId;
        /* 0x08 */ float m_Volume;
        /* 0x0C */ PriorityFadeBits m_Fades;
        /* 0x10 */ char m_StreamParam[32];
        /* 0x30 */ AudioStreamTrack::StreamTrack& m_Track;
        /* 0x34 */ Audio::MasterVolume::VOLUME_GROUP m_VolGroup;

        static unsigned char s_BowserAttackNext;
        static unsigned char s_SuddenDeathNext;
    }; // total size: 0x38

    PriorityStream(AudioStreamTrack::StreamTrack&);
    void Reset();
    void PlayStream(unsigned long, float, bool, unsigned long, unsigned long, const char*);
    void Stop(unsigned long, unsigned long);
    void FakePause(unsigned long);
    void FakeResume(bool);
    void TrackIdleCB();
    bool GrabCrowdStream(unsigned long);

    /* 0x00 */ unsigned char m_InPause : 8;
    /* 0x04 */ AudioStreamTrack::StreamTrack& m_Track;
    /* 0x08 */ PLAY_RECORD m_PStream;
    /* 0x40 */ PLAY_RECORD m_CapChant;
}; // total size: 0x78

static inline void PriorityStreamSetIdleCallback(AudioStreamTrack::StreamTrack* track, const Function0<void>& f0)
{
    track->m_IdleCallback = Function<FnVoidVoid>(f0);
}

inline PriorityStream::PriorityStream(AudioStreamTrack::StreamTrack& track)
    : m_InPause(false)
    , m_Track(track)
    , m_PStream(m_Track)
    , m_CapChant(m_Track)
{
    Function0<void> f0(Bind<void>(MemFun<PriorityStream, void>(&PriorityStream::TrackIdleCB), this));
    AudioStreamTrack::StreamTrack& trackRef = m_Track;
    PriorityStreamSetIdleCallback(&trackRef, f0);
}

#endif // _PRIORITYSTREAM_H_
