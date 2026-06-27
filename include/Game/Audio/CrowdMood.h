#ifndef _CROWDMOOD_H_
#define _CROWDMOOD_H_

// #include "NL/nlStrChr.h"

// Forward declarations for audio streaming types
namespace GCAudioStreaming
{
class StereoAudioStream;
class MonoAudioStream;
} // namespace GCAudioStreaming

struct CROWD_AUDIO_INIT
{
    /* 0x00 */ unsigned long NeutralVoiceId;
    /* 0x04 */ unsigned long PositiveVoiceId;
    /* 0x08 */ unsigned long NegativeVoiceId;
    /* 0x0C */ GCAudioStreaming::StereoAudioStream* pChantStream;
    /* 0x10 */ GCAudioStreaming::MonoAudioStream* pHeckleStream;
    /* 0x14 */ unsigned long SaturationVoiceId;
    /* 0x18 */ unsigned long CurrentSaturationSFXId;
    /* 0x1C */ const char* CurrentSaturationSampleName;
}; // total size: 0x20

extern CROWD_AUDIO_INIT g_CrowdAudio;

// struct VOCALIZATION_STATE
// {
//     /* 0x00 */ float SinceLast;
//     /* 0x04 */ float NextAt;
//     /* 0x08 */ bool Ready;
// }; // total size: 0xC

struct VOLUME_FADE
{
    /* 0x00 */ float StartVol;
    /* 0x04 */ float EndVol;
    /* 0x08 */ float Time;
    /* 0x0C */ float Interp;
}; // total size: 0x10

struct CROWD_VOCAL_DEFINITION
{
    float Volume;      // offset 0x0, size 0x4
    float VolumeRange; // offset 0x4, size 0x4
    float Delay;       // offset 0x8, size 0x4
    float DelayRange;  // offset 0xC, size 0x4
}; // total size: 0x10

struct MOOD_DEFINITION
{
    float NeutralVol;       // offset 0x0, size 0x4
    float NegativeVol;      // offset 0x4, size 0x4
    float PositiveVol;      // offset 0x8, size 0x4
    float SaturationVolume; // offset 0xC, size 0x4
    union
    {                                  // inferred
        float SaturationStart;         // offset 0x10, size 0x4
        unsigned long SaturationSFXId; // offset 0x10, size 0x4
    };
    struct CROWD_VOCAL_DEFINITION Chant;  // offset 0x14, size 0x10
    struct CROWD_VOCAL_DEFINITION Heckle; // offset 0x24, size 0x10
}; // total size: 0x34

// struct CROWD_STATE
// {
//     char pad[0x53];
//     bool flag0 : 1;
//     bool flag1 : 1;
//     bool bFastTransition : 1;
// };

void PlayMoodDef(MOOD_DEFINITION&);
void ChangeCrowdVolume(float);
// void ___blank(const char*, ...);
// void NDimDistance<4>(float*, float*);

// void nlStrNCmp<char>(const char*, const char*, unsigned long);

namespace CrowdMood // I suspect this is a namespace, not a class
{

enum CROWD_MOOD
{
    CM_Positive = 0,
    CM_Negative = 1,
    CM_Bored = 2,
    CM_Frustrated = 3,
    CM_Neutral = 4,
    CM_END = 5,
};

void RestartLoops();
void EnableCrowdDecay(bool);
void UnlockStream();
GCAudioStreaming::StereoAudioStream* LockStream();
void SetLPF(unsigned short);
void ActivateLPF(bool);
void SetCrowdVolume(unsigned long, unsigned long);
void InitiateFastCrowdTransition();
void SetMood(CrowdMood::CROWD_MOOD, unsigned long);
void AdjustMood(CrowdMood::CROWD_MOOD, unsigned long);
void Update(float);
void Purge(bool);
void Init();
void ReadConfig();
unsigned char IsStreamLocked();

}; // namespace CrowdMood

struct CROWD_STATE
{
    struct VOCALIZATION_STATE
    {
        /* 0x00 */ float SinceLast;
        /* 0x04 */ float NextAt;
        /* 0x08 */ bool Ready;
    }; // total size: 0xC

    /* 0x00 */ float StartingMood[4];
    /* 0x10 */ float MidpointMood[4];
    /* 0x20 */ float DestinationMood[4];
    /* 0x30 */ float CurrentMoodBlend[4];
    /* 0x40 */ float Interpolant;
    /* 0x44 */ float InterpolantMidpoint;
    /* 0x48 */ float LastGameTime;
    /* 0x4C */ float SinceMoodDest;
    /* 0x50 */ unsigned char CurrentMood;
    /* 0x51 */ unsigned char DestMoodLevel;
    /* 0x52 */ unsigned char DestMood;
    /* 0x53 */ unsigned char HasChanged : 1;
    /* 0x53 */ unsigned char AtDestination : 1;
    /* 0x53 */ unsigned char BlendFast : 1;
    /* 0x53 */ unsigned char SkipBlend : 1;
    /* 0x53 */ unsigned char LPFOn : 1;
    /* 0x53 */ unsigned char _padBits : 3;
    /* 0x54 */ unsigned char StreamLocked; // SEPARATE BYTE at 0x54, not bitfield!
    /* 0x55 */ unsigned char _pad[3];      // padding for float alignment
    /* 0x58 */ float CrowdVolume;
    /* 0x5C */ unsigned long LPFFreq;
    /* 0x60 */ VOCALIZATION_STATE ChantState;
    /* 0x6C */ VOCALIZATION_STATE HeckleState;
    /* 0x78 */ float _unk78;
    /* 0x7C */ VOLUME_FADE VolumeFade;
}; // total size: 0x8C

extern CROWD_STATE g_CrowdState;

// class PlayVocal(const CROWD_VOCAL_DEFINITION&, CROWD_STATE
// {
// public:
//     void VOCALIZATION_STATE&, GCAudioStreaming::AudioStream*);
// };

// class WarmRandomStream<GCAudioStreaming
// {
// public:
//     void MonoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::MonoAudioStream*);
//     void StereoAudioStream>(const RANDOM_STREAMS&, GCAudioStreaming::StereoAudioStream*);
// };

// class Increment<CrowdMood
// {
// public:
//     void CROWD_MOOD>(CrowdMood::CROWD_MOOD&);
// };

// class GCAudioStreaming
// {
// public:
//     void MonoAudioStream::SafeToPurge();
//     void StereoAudioStream::SafeToPurge();
//     void MonoAudioStream::~MonoAudioStream();
//     void StereoAudioStream::Purge();
//     void MonoAudioStream::Purge();
// };

#endif // _CROWDMOOD_H_
