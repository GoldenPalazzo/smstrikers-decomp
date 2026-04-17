#include "Game/Sys/THPSimple.h"
#include "NL/nlFileGC.h"
#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/thp/THPInfo.h"
#include "dolphin/thp/THPPlayer.h"

static s16 SoundBuffer[2][320];
static int Initialized;
static s32 SoundBufferIndex;
static void (*OldAIDCallback)();
static s16* LastAudioBuffer;
static s16* CurAudioBuffer;
static s32 AudioSystem;

struct THPSimpleControlWork
{
    /* 0x00 */ nlFile* fileInfo;
    /* 0x04 */ unsigned char _pad04[0x08];
    /* 0x0C */ unsigned long bufSize;
    /* 0x10 */ unsigned long audioMaxSamples;
    /* 0x14 */ unsigned char _pad14[0x34];
    /* 0x48 */ THPVideoInfo videoInfo;
    /* 0x54 */ unsigned char _pad54[0x14];
    /* 0x68 */ int open;
    /* 0x6C */ unsigned char preFetchState;
    /* 0x6D */ unsigned char audioState;
    /* 0x6E */ unsigned char loop;
    /* 0x6F */ unsigned char audioExist;
    /* 0x70 */ long curOffset;
    /* 0x74 */ int dvdError;
    /* 0x78 */ unsigned long readProgress;
    /* 0x7C */ long nextDecodeIndex;
    /* 0x80 */ long readIndex;
    /* 0x84 */ long readSize;
    /* 0x88 */ long totalReadFrame;
    /* 0x8C */ float curVolume;
    /* 0x90 */ float targetVolume;
    /* 0x94 */ float deltaVolume;
    /* 0x98 */ long rampCount;
    /* 0x9C */ THPReadBuffer readBuffer[16];
    /* 0x15C */ THPTextureSet textureSet;
    /* 0x16C */ THPAudioBuffer audioBuffer[6];
    /* 0x1B4 */ long audioDecodeIndex;
    /* 0x1B8 */ long audioOutputIndex;
};

extern int NumAudioBuffers;
extern unsigned short VolumeTable[128];
extern "C" void* memcpy(void*, const void*, unsigned long);
extern "C" void* memset(void*, int, unsigned long);

/**
 * Offset/Address/Size: 0x0 | 0x801CBF64 | size: 0x10
 */
extern "C" s32 THPSimpleGetCurrentFrame()
{
    return ((THPSimpleControlWork*)&SimpleControl)->textureSet.mFrameNumber;
}

/**
 * Offset/Address/Size: 0x2B0 | 0x801CC214 | size: 0x24
 */
extern "C" s32 THPSimpleGetTotalFrame()
{
    if (((THPSimpleControlWork*)&SimpleControl)->open)
        return SimpleControl.numFrames;
    return 0;
}

/**
 * Offset/Address/Size: 0xB74 | 0x801CCAD8 | size: 0x14
 */
extern "C" void THPSimpleAudioStop()
{
    ((THPSimpleControlWork*)&SimpleControl)->audioState = 0;
}

/**
 * Offset/Address/Size: 0xB88 | 0x801CCAEC | size: 0x14
 */
extern "C" void THPSimpleAudioStart()
{
    ((THPSimpleControlWork*)&SimpleControl)->audioState = 1;
}

/**
 * Offset/Address/Size: 0x17E4 | 0x801CD748 | size: 0x4
 */
void __THPAsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long))
{
}

static inline void update_read_idx()
{
    register s32 readIdx = SimpleControl.readIdx;
    readIdx = (readIdx + 1 >= NumReadBuffers) ? 0 : readIdx + 1;
    SimpleControl.readIdx = readIdx;
}

/**
 * Offset/Address/Size: 0xD0C | 0x801CCC70 | size: 0x160
 * TODO: 99.49% match - remaining register swap in readIdx/nextSize update (lwzx/andc temp assignment)
 */
void __THPSimpleDVDCallback(nlFile* file, void* buffer, unsigned int bytesRead, unsigned long offset)
{
    SimpleControl.dvdBusy = 0;

    SimpleControl.readBuffers[SimpleControl.readIdx].mFrameNumber = SimpleControl.frameCount;
    SimpleControl.frameCount++;
    SimpleControl.readBuffers[SimpleControl.readIdx].mIsValid = TRUE;

    SimpleControl.totalRead += SimpleControl.nextSize;
    SimpleControl.nextSize = *(u32*)SimpleControl.readBuffers[SimpleControl.readIdx].mPtr;

    update_read_idx();

    if (SimpleControl.readBuffers[SimpleControl.readIdx].mIsValid != 0)
    {
        return;
    }

    if (SimpleControl.unk74 != 0)
    {
        return;
    }

    if (SimpleControl.playing != 1)
    {
        return;
    }

    if (SimpleControl.frameCount > SimpleControl.numFrames - 1)
    {
        if (SimpleControl.loop != 1)
        {
            return;
        }
        SimpleControl.frameCount = 0;
        SimpleControl.totalRead = SimpleControl.movieDataOffsets;
        SimpleControl.nextSize = SimpleControl.firstFrameSize;
    }

    SimpleControl.dvdBusy = 1;
    nlSeek(SimpleControl.file, SimpleControl.totalRead, 0);
    nlReadAsync(SimpleControl.file, SimpleControl.readBuffers[SimpleControl.readIdx].mPtr, SimpleControl.nextSize, __THPSimpleDVDCallback, 0);
}

/**
 * Offset/Address/Size: 0x2D4 | 0x801CC238 | size: 0x48
 */
extern "C" int THPSimpleGetVideoInfo(THPVideoInfo* videoInfo)
{
    if (((THPSimpleControlWork*)&SimpleControl)->open)
    {
        memcpy(videoInfo, &((THPSimpleControlWork*)&SimpleControl)->videoInfo, sizeof(THPVideoInfo));
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x31C | 0x801CC280 | size: 0x368
 * TODO: 89.5% match - MWCC register allocation puts requestSample in r0 vs r11, cascading to
 * inner loop register diffs and post-loop addressing mode differences (addi+lwzx vs add+lwz)
 */
void MixAudio(short* destination, short* source, unsigned long sample)
{
    unsigned long requestSample;
    unsigned long i;
    unsigned short vol;
    long mix;
    short* dst;
    short* libsrc;
    short* thpsrc;

    if (AudioSystem == 1)
    {
        return;
    }

    if (source != NULL)
    {
        THPSimpleControlWork* simple = (THPSimpleControlWork*)&SimpleControl;

        if ((simple->open != 0) && (simple->audioState == 1) && (simple->audioExist != 0))
        {
            while (1)
            {
                requestSample = simple->audioBuffer[simple->audioOutputIndex].mValidSample;
                if (requestSample == 0)
                {
                    break;
                }

                if (requestSample >= sample)
                {
                    requestSample = sample;
                }

                thpsrc = simple->audioBuffer[simple->audioOutputIndex].mCurPtr;
                dst = destination;
                libsrc = source;

                for (i = 0; i < requestSample; i++)
                {
                    if (simple->rampCount != 0)
                    {
                        simple->rampCount--;
                        simple->curVolume += simple->deltaVolume;
                    }
                    else
                    {
                        simple->curVolume = simple->targetVolume;
                    }

                    vol = VolumeTable[(long)simple->curVolume];

                    mix = libsrc[0] + ((vol * thpsrc[0]) >> 15);
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[0] = mix;

                    mix = libsrc[1] + ((vol * thpsrc[1]) >> 15);
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[1] = mix;

                    dst += 2;
                    libsrc += 2;
                    thpsrc += 2;
                }

                sample -= requestSample;

                THPAudioBuffer* audio = &simple->audioBuffer[simple->audioOutputIndex];
                audio->mValidSample -= requestSample;

                audio = &simple->audioBuffer[simple->audioOutputIndex];
                audio->mCurPtr = thpsrc;

                audio = &simple->audioBuffer[simple->audioOutputIndex];
                if (audio->mValidSample == 0)
                {
                    i = simple->audioOutputIndex + 1;
                    simple->audioOutputIndex = i;
                    if ((int)i >= NumAudioBuffers)
                    {
                        simple->audioOutputIndex = 0;
                    }
                }

                if (sample == 0)
                {
                    return;
                }

                destination = dst;
                source = libsrc;
            }

            memcpy(destination, source, sample << 2);
        }
        else
        {
            memcpy(destination, source, sample << 2);
        }
    }
    else
    {
        THPSimpleControlWork* simple = (THPSimpleControlWork*)&SimpleControl;

        if ((simple->open != 0) && (simple->audioState == 1) && (simple->audioExist != 0))
        {
            while (1)
            {
                requestSample = simple->audioBuffer[simple->audioOutputIndex].mValidSample;
                if (requestSample == 0)
                {
                    break;
                }

                if (requestSample >= sample)
                {
                    requestSample = sample;
                }

                thpsrc = simple->audioBuffer[simple->audioOutputIndex].mCurPtr;
                dst = destination;

                for (i = 0; i < requestSample; i++)
                {
                    if (simple->rampCount != 0)
                    {
                        simple->rampCount--;
                        simple->curVolume += simple->deltaVolume;
                    }
                    else
                    {
                        simple->curVolume = simple->targetVolume;
                    }

                    vol = VolumeTable[(long)simple->curVolume];

                    mix = (vol * thpsrc[0]) >> 15;
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[0] = mix;

                    mix = (vol * thpsrc[1]) >> 15;
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[1] = mix;

                    dst += 2;
                    thpsrc += 2;
                }

                sample -= requestSample;

                THPAudioBuffer* audio = &simple->audioBuffer[simple->audioOutputIndex];
                audio->mValidSample -= requestSample;

                audio = &simple->audioBuffer[simple->audioOutputIndex];
                audio->mCurPtr = thpsrc;

                audio = &simple->audioBuffer[simple->audioOutputIndex];
                if (audio->mValidSample == 0)
                {
                    i = simple->audioOutputIndex + 1;
                    simple->audioOutputIndex = i;
                    if ((int)i >= NumAudioBuffers)
                    {
                        simple->audioOutputIndex = 0;
                    }
                }

                if (sample == 0)
                {
                    return;
                }

                destination = dst;
            }

            memset(destination, 0, sample << 2);
        }
        else
        {
            memset(destination, 0, sample << 2);
        }
    }
}

/**
 * Offset/Address/Size: 0x138 | 0x801CC09C | size: 0x178
 */
void THPAudioMixCallback()
{
    if (AudioSystem == 0)
    {
        SoundBufferIndex ^= 1;
        AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
        BOOL old = OSEnableInterrupts();
        MixAudio(SoundBuffer[SoundBufferIndex], NULL, 0xA0);
        DCFlushRange(SoundBuffer[SoundBufferIndex], 0x280);
        OSRestoreInterrupts(old);
    }
    else
    {
        if (AudioSystem == 2)
        {
            if (LastAudioBuffer != NULL)
            {
                CurAudioBuffer = LastAudioBuffer;
            }
            OldAIDCallback();
            LastAudioBuffer = (s16*)((u32)AIGetDMAStartAddr() + 0x80000000);
        }
        else
        {
            OldAIDCallback();
            CurAudioBuffer = (s16*)((u32)AIGetDMAStartAddr() + 0x80000000);
        }

        SoundBufferIndex ^= 1;
        AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
        BOOL old = OSEnableInterrupts();

        if (CurAudioBuffer != NULL)
        {
            DCInvalidateRange(CurAudioBuffer, 0x280);
        }

        MixAudio(SoundBuffer[SoundBufferIndex], CurAudioBuffer, 0xA0);
        DCFlushRange(SoundBuffer[SoundBufferIndex], 0x280);
        OSRestoreInterrupts(old);
    }
}

/**
 * Offset/Address/Size: 0x1238 | 0x801CD19C | size: 0x60
 */
extern "C" unsigned long THPSimpleCalcNeedMemory(int numReadBuffers, int numAudioBuffers)
{
    unsigned long size;

    NumReadBuffers = numReadBuffers;

    THPSimpleControlWork* ctrl = (THPSimpleControlWork*)&SimpleControl;

    NumAudioBuffers = numAudioBuffers;

    if (ctrl->open)
    {
        size = ((ctrl->bufSize + 31) & ~31) * numReadBuffers;

        if (ctrl->audioExist)
        {
            size += numAudioBuffers * ((ctrl->audioMaxSamples * 4 + 31) & ~31);
        }

        return size + 0x1000;
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x1664 | 0x801CD5C8 | size: 0x64
 */
extern "C" void THPSimpleQuit()
{
    LCDisable();
    if (AudioSystem != 1 && OldAIDCallback != NULL)
    {
        int old = OSDisableInterrupts();
        AIRegisterDMACallback(OldAIDCallback);
        OSRestoreInterrupts(old);
    }
    Initialized = 0;
}

/**
 * Offset/Address/Size: 0x16C8 | 0x801CD62C | size: 0x11C
 */
extern "C" int THPSimpleInit(long audioSystem)
{
    memset(&SimpleControl, 0, 0x1BC);
    LCEnable();

    if (!THPInit())
    {
        return 0;
    }

    AudioSystem = audioSystem;
    SoundBufferIndex = 0;
    LastAudioBuffer = NULL;
    CurAudioBuffer = NULL;

    if (audioSystem != 1)
    {
        int old = OSDisableInterrupts();
        OldAIDCallback = AIRegisterDMACallback(THPAudioMixCallback);

        if (OldAIDCallback == NULL && AudioSystem != 0)
        {
            AIRegisterDMACallback(NULL);
            OSRestoreInterrupts(old);
            return 0;
        }

        OSRestoreInterrupts(old);

        if (AudioSystem == 0)
        {
            memset(SoundBuffer, 0, sizeof(SoundBuffer));
            DCFlushRange(SoundBuffer, sizeof(SoundBuffer));
            AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
            AIStartDMA();
        }
    }

    Initialized = 1;
    return 1;
}
