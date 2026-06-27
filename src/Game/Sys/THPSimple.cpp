#include "Game/Sys/THPSimple.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxTexture.h"
#include "NL/nlFileGC.h"
#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/thp/THPAudio.h"
#include "dolphin/thp/THPInfo.h"
#include "dolphin/thp/THPPlayer.h"
#include "dolphin/thp/THPVideoDecode.h"

static s16 SoundBuffer[2][320];
static int Initialized;
static s32 SoundBufferIndex;
static void (*OldAIDCallback)();
static s16* LastAudioBuffer;
static s16* CurAudioBuffer;
static s32 AudioSystem;
static long WorkBuffer[16];

struct THPSimpleControlWork
{
    /* 0x00 */ nlFile* fileInfo;
    /* 0x04 */ char magic[4];
    /* 0x08 */ unsigned long version;
    /* 0x0C */ unsigned long bufSize;
    /* 0x10 */ unsigned long audioMaxSamples;
    /* 0x14 */ float frameRate;
    /* 0x18 */ unsigned long numFrames;
    /* 0x1C */ unsigned long firstFrameSize;
    /* 0x20 */ unsigned long movieDataSize;
    /* 0x24 */ unsigned long compInfoDataOffsets;
    /* 0x28 */ unsigned long offsetDataOffsets;
    /* 0x2C */ unsigned long movieDataOffsets;
    /* 0x30 */ unsigned long finalFrameDataOffsets;
    /* 0x34 */ THPFrameCompInfo compInfo;
    /* 0x48 */ THPVideoInfo videoInfo;
    /* 0x54 */ THPAudioInfo audioInfo;
    /* 0x64 */ void* thpWork;
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
extern "C" int strcmp(const char*, const char*);

/**
 * Offset/Address/Size: 0x0 | 0x801CBF64 | size: 0x10
 */
extern "C" s32 THPSimpleGetCurrentFrame()
{
    return ((THPSimpleControlWork*)&SimpleControl)->textureSet.mFrameNumber;
}

static long THPSimpleGetVolume()
{
    return (long)((THPSimpleControlWork*)&SimpleControl)->curVolume;
}

/**
 * Offset/Address/Size: 0x10 | 0x801CBF74 | size: 0x128
 */
extern "C" int THPSimpleSetVolume(long vol, long time)
{
    THPSimpleControlWork* ctrl = (THPSimpleControlWork*)&SimpleControl;

    if (ctrl->open && ctrl->audioExist)
    {
        u32 rate = AIGetDSPSampleRate();
        long samplePerMs = 0x30;
        if (!rate)
            samplePerMs = 0x20;

        if (vol > 127)
            vol = 127;
        if (vol < 0)
            vol = 0;
        if (time > 60000)
            time = 60000;
        if (time < 0)
            time = 0;

        int old = OSDisableInterrupts();
        ctrl = (THPSimpleControlWork*)&SimpleControl;

        ctrl->targetVolume = (float)vol;

        if (time != 0)
        {
            ctrl->rampCount = samplePerMs * time;
            ctrl->deltaVolume = (ctrl->targetVolume - ctrl->curVolume) / (float)ctrl->rampCount;
        }
        else
        {
            ctrl->curVolume = ctrl->targetVolume;
            ctrl->rampCount = 0;
        }

        OSRestoreInterrupts(old);
        return 1;
    }
    return 0;
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
 * Offset/Address/Size: 0xA14 | 0x801CC978 | size: 0x160
 */
extern "C" int THPSimpleLoadStop()
{
    long i;

    if (SimpleControl.open && SimpleControl.audioState == 0)
    {
        SimpleControl.playing = 0;

        if (SimpleControl.dvdBusy != 0)
        {
            nlCancelPendingAsyncReads(SimpleControl.file, __THPAsyncCancelCB);

            while (nlAsyncReadsPending(SimpleControl.file))
            {
                nlServiceFileSystem();
                OSYieldThread();
            }

            SimpleControl.dvdBusy = 0;
        }

        for (i = 0; i < 16; i++)
        {
            SimpleControl.readBuffers[i].mIsValid = 0;
        }

        SimpleControl.audioBuffer[0].mValidSample = 0;
        SimpleControl.audioBuffer[1].mValidSample = 0;
        SimpleControl.audioBuffer[2].mValidSample = 0;
        SimpleControl.audioBuffer[3].mValidSample = 0;
        SimpleControl.audioBuffer[4].mValidSample = 0;
        SimpleControl.audioBuffer[5].mValidSample = 0;
        SimpleControl.textureSet.mFrameNumber = -1;
        SimpleControl.totalRead = SimpleControl.movieDataOffsets;
        SimpleControl.nextSize = SimpleControl.firstFrameSize;
        SimpleControl.readIdx = 0;
        SimpleControl.frameCount = 0;
        SimpleControl.unk74 = 0;
        SimpleControl.nextDecodeIndex = 0;
        SimpleControl.audioDecodeIndex = 0;
        SimpleControl.audioOutputIndex = 0;
        SimpleControl.curVolume = SimpleControl.targetVolume;
        SimpleControl.rampCount = 0;

        return 1;
    }

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
 * Offset/Address/Size: 0xB9C | 0x801CCB00 | size: 0x170
 */
extern "C" int THPSimplePreLoad(long loop)
{
    unsigned long i;
    unsigned long readNum;

    if (SimpleControl.open && SimpleControl.playing == 0)
    {
        readNum = NumReadBuffers;
        if (loop == 0 && SimpleControl.numFrames < (unsigned long)NumReadBuffers)
        {
            readNum = SimpleControl.numFrames;
        }

        for (i = 0; i < readNum; i++)
        {
            nlSeek(SimpleControl.file, SimpleControl.totalRead, 0);
            nlRead(SimpleControl.file, SimpleControl.readBuffers[SimpleControl.readIdx].mPtr, SimpleControl.nextSize);

            long idx = SimpleControl.readIdx;
            SimpleControl.totalRead += SimpleControl.nextSize;
            SimpleControl.nextSize = *(long*)SimpleControl.readBuffers[idx].mPtr;
            SimpleControl.readBuffers[idx].mIsValid = 1;
            SimpleControl.readBuffers[SimpleControl.readIdx].mFrameNumber = SimpleControl.frameCount;

            SimpleControl.readIdx = (SimpleControl.readIdx + 1 >= NumReadBuffers) ? 0 : SimpleControl.readIdx + 1;
            SimpleControl.frameCount++;

            if ((unsigned long)SimpleControl.frameCount > SimpleControl.numFrames - 1)
            {
                if (SimpleControl.loop == 1)
                {
                    SimpleControl.frameCount = 0;
                    SimpleControl.totalRead = SimpleControl.movieDataOffsets;
                    SimpleControl.nextSize = SimpleControl.firstFrameSize;
                }
            }
        }

        SimpleControl.loop = loop;
        SimpleControl.playing = 1;
        return 1;
    }

    return 0;
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
    if (readIdx + 1 >= NumReadBuffers)
        readIdx = 0;
    else
        readIdx = readIdx + 1;
    SimpleControl.readIdx = readIdx;
}

/**
 * Offset/Address/Size: 0xD0C | 0x801CCC70 | size: 0x160
 * TODO: 99.89% match - callback pointer immediate uses symbol @ha/@l instead of section .text@ha/@l
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
 * TODO: 99.5% match - extra move after audio-buffer valid-sample load shifts branch targets
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
        if ((SimpleControl.open != 0) && (SimpleControl.audioState == 1) && (SimpleControl.audioExist != 0))
        {
            while (1)
            {
                requestSample = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample;
                if (requestSample == 0)
                {
                    break;
                }

                requestSample = (requestSample >= sample) ? sample : requestSample;

                thpsrc = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr;
                dst = destination;
                libsrc = source;

                for (i = 0; i < requestSample; i++)
                {
                    if (SimpleControl.rampCount != 0)
                    {
                        SimpleControl.rampCount--;
                        SimpleControl.curVolume += SimpleControl.deltaVolume;
                    }
                    else
                    {
                        SimpleControl.curVolume = SimpleControl.targetVolume;
                    }

                    vol = VolumeTable[(long)SimpleControl.curVolume];

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

                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample -= requestSample;
                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr = thpsrc;

                if ((SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample == 0) && (++SimpleControl.audioOutputIndex >= NumAudioBuffers))
                {
                    SimpleControl.audioOutputIndex = 0;
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
        if ((SimpleControl.open != 0) && (SimpleControl.audioState == 1) && (SimpleControl.audioExist != 0))
        {
            while (1)
            {
                requestSample = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample;
                if (requestSample == 0)
                {
                    break;
                }

                if (requestSample >= sample)
                {
                    requestSample = sample;
                }

                thpsrc = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr;
                dst = destination;

                for (i = 0; i < requestSample; i++)
                {
                    if (SimpleControl.rampCount != 0)
                    {
                        SimpleControl.rampCount--;
                        SimpleControl.curVolume += SimpleControl.deltaVolume;
                    }
                    else
                    {
                        SimpleControl.curVolume = SimpleControl.targetVolume;
                    }

                    vol = VolumeTable[(long)SimpleControl.curVolume];

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

                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample -= requestSample;
                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr = thpsrc;

                if ((SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample == 0) && (++SimpleControl.audioOutputIndex >= NumAudioBuffers))
                {
                    SimpleControl.audioOutputIndex = 0;
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

static int VideoDecode(unsigned char* videoFrame)
{
    long ret = THPVideoDecode(videoFrame,
        ((THPSimpleControlWork*)&SimpleControl)->textureSet.mYTexture,
        ((THPSimpleControlWork*)&SimpleControl)->textureSet.mUTexture,
        ((THPSimpleControlWork*)&SimpleControl)->textureSet.mVTexture,
        ((THPSimpleControlWork*)&SimpleControl)->thpWork);
    if (ret == 0)
    {
        ((THPSimpleControlWork*)&SimpleControl)->textureSet.mFrameNumber = ((THPSimpleControlWork*)&SimpleControl)->readBuffer[((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex].mFrameNumber;
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x684 | 0x801CC5E8 | size: 0x390
 */
extern "C" long THPSimpleDecode(long audioTrack)
{
    int* validBuffer;
    THPReadBuffer* readBuffer;
    int old;
    unsigned long i;
    unsigned char* ptr;
    unsigned long* compSizePtr;
    unsigned long sample;

    validBuffer = &((THPSimpleControlWork*)&SimpleControl)->readBuffer[0].mIsValid;

    if (validBuffer[((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex * 3] == 0)
    {
        goto ret2;
    }

    readBuffer = ((THPSimpleControlWork*)&SimpleControl)->readBuffer;
    compSizePtr = (unsigned long*)(readBuffer[((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex].mPtr + 8);
    ptr = readBuffer[((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex].mPtr + ((THPSimpleControlWork*)&SimpleControl)->compInfo.mNumComponents * 4 + 8;

    if (((THPSimpleControlWork*)&SimpleControl)->audioExist != 0 && AudioSystem != 1)
    {
        if (audioTrack < 0 || (unsigned long)audioTrack >= ((THPSimpleControlWork*)&SimpleControl)->audioInfo.mSndNumTracks)
        {
            return 4;
        }

        if (((THPSimpleControlWork*)&SimpleControl)->audioBuffer[((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex].mValidSample != 0)
        {
            goto ret3;
        }

        for (i = 0; i < ((THPSimpleControlWork*)&SimpleControl)->compInfo.mNumComponents; i++)
        {
            switch (((THPSimpleControlWork*)&SimpleControl)->compInfo.mFrameComp[i])
            {
            case 0:
                if (!VideoDecode(ptr))
                {
                    return 1;
                }
                break;
            case 1:
                sample = THPAudioDecode(
                    ((THPSimpleControlWork*)&SimpleControl)->audioBuffer[((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex].mBuffer,
                    ptr + *compSizePtr * audioTrack,
                    0);
                old = OSDisableInterrupts();
                ((THPSimpleControlWork*)&SimpleControl)->audioBuffer[((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex].mValidSample = sample;
                ((THPSimpleControlWork*)&SimpleControl)->audioBuffer[((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex].mCurPtr = ((THPSimpleControlWork*)&SimpleControl)->audioBuffer[((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex].mBuffer;
                OSRestoreInterrupts(old);
                if (++((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex >= NumAudioBuffers)
                {
                    ((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex = 0;
                }
                break;
            }
            ptr += *compSizePtr;
            compSizePtr++;
        }
        goto decoded;
    ret3:
        return 3;
    }

    for (i = 0; i < ((THPSimpleControlWork*)&SimpleControl)->compInfo.mNumComponents; i++)
    {
        switch (((THPSimpleControlWork*)&SimpleControl)->compInfo.mFrameComp[i])
        {
        case 0:
            if (!VideoDecode(ptr))
            {
                return 1;
            }
            break;
        }
        ptr += *compSizePtr;
        compSizePtr++;
    }

decoded:
    validBuffer[((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex * 3] = 0;
    ((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex = (((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex + 1 >= NumReadBuffers) ? 0 : ((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex + 1;

    old = OSDisableInterrupts();

    if (validBuffer[((THPSimpleControlWork*)&SimpleControl)->readIndex * 3] == 0 && ((THPSimpleControlWork*)&SimpleControl)->readProgress == 0 && ((THPSimpleControlWork*)&SimpleControl)->dvdError == 0 && ((THPSimpleControlWork*)&SimpleControl)->preFetchState == 1)
    {
        if ((unsigned long)((THPSimpleControlWork*)&SimpleControl)->totalReadFrame > ((THPSimpleControlWork*)&SimpleControl)->numFrames - 1)
        {
            if (((THPSimpleControlWork*)&SimpleControl)->loop != 1)
            {
                goto done;
            }
            ((THPSimpleControlWork*)&SimpleControl)->totalReadFrame = 0;
            ((THPSimpleControlWork*)&SimpleControl)->curOffset = ((THPSimpleControlWork*)&SimpleControl)->movieDataOffsets;
            ((THPSimpleControlWork*)&SimpleControl)->readSize = ((THPSimpleControlWork*)&SimpleControl)->firstFrameSize;
        }

        ((THPSimpleControlWork*)&SimpleControl)->readProgress = 1;
        nlSeek(((THPSimpleControlWork*)&SimpleControl)->fileInfo, ((THPSimpleControlWork*)&SimpleControl)->curOffset, 0);
        nlReadAsync(((THPSimpleControlWork*)&SimpleControl)->fileInfo,
            readBuffer[((THPSimpleControlWork*)&SimpleControl)->readIndex].mPtr,
            ((THPSimpleControlWork*)&SimpleControl)->readSize,
            __THPSimpleDVDCallback,
            0);
    }
done:
    OSRestoreInterrupts(old);
    return 0;
ret2:
    return 2;
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
 * Offset/Address/Size: 0x116C | 0x801CCDD0 | size: 0x3CC
 */
extern "C" int THPSimpleSetBuffer(unsigned char* buffer)
{
    unsigned long i;
    unsigned char* ptr;
    unsigned long numRead;
    unsigned long numAudio;
    PlatTexture* tex;

    if (SimpleControl.open && SimpleControl.playing == 0)
    {
        if (SimpleControl.audioState == 1)
        {
            return 0;
        }

        ptr = buffer;

        SimpleControl.textureSet.mYTexture = (u8*)glx_GetTex(glGetTexture("movie"), 1, 1)->m_SwizzledData;
        SimpleControl.textureSet.mUTexture = (u8*)glx_GetTex(glGetTexture("movie_u"), 1, 1)->m_SwizzledData;
        tex = glx_GetTex(glGetTexture("movie_v"), 1, 1);
        numRead = NumReadBuffers;
        SimpleControl.textureSet.mVTexture = (u8*)tex->m_SwizzledData;

        for (i = 0; i < (unsigned long)NumReadBuffers; i++)
        {
            SimpleControl.readBuffers[i].mPtr = ptr;
            ptr += (SimpleControl.bufSize + 31) & ~31;
            SimpleControl.readBuffers[i].mIsValid = 0;
        }

        if (SimpleControl.audioExist)
        {
            numAudio = NumAudioBuffers;
            for (i = 0; i < (unsigned long)NumAudioBuffers; i++)
            {
                SimpleControl.audioBuffer[i].mBuffer = (s16*)ptr;
                SimpleControl.audioBuffer[i].mCurPtr = (s16*)ptr;
                SimpleControl.audioBuffer[i].mValidSample = 0;
                ptr += (SimpleControl.audioMaxSamples * 4 + 31) & ~31;
            }
        }

        SimpleControl.thpWork = ptr;
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x1298 | 0x801CD1FC | size: 0xCC
 */
extern "C" int THPSimpleClose()
{
    THPSimpleControlWork* ctrl = (THPSimpleControlWork*)&SimpleControl;

    if (ctrl->open && ctrl->preFetchState == 0)
    {
        if (ctrl->audioExist)
        {
            if (ctrl->audioState == 1)
            {
                return 0;
            }
        }
        else
        {
            ctrl->audioState = 0;
        }

        THPSimpleControlWork* sc = (THPSimpleControlWork*)&SimpleControl;

        if (sc->readProgress == 0)
        {
            ctrl->open = 0;

            while (nlAsyncReadsPending(sc->fileInfo))
            {
                nlServiceFileSystem();
            }

            nlClose(((THPSimpleControlWork*)&SimpleControl)->fileInfo);

            ((THPSimpleControlWork*)&SimpleControl)->fileInfo = NULL;

            return 1;
        }
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x1364 | 0x801CD2C8 | size: 0x300
 * TODO: 97.4% match - 8 instruction scheduling diffs in init block setup (li/lwz/lfs ordering)
 */
extern "C" int THPSimpleOpen(const char* fileName)
{
    long offset;
    long i;

    if (!Initialized)
    {
        return 0;
    }

    if (((THPSimpleControlWork*)&SimpleControl)->open)
    {
        return 0;
    }

    memset(&((THPSimpleControlWork*)&SimpleControl)->videoInfo, 0, sizeof(THPVideoInfo));
    memset(&((THPSimpleControlWork*)&SimpleControl)->audioInfo, 0, sizeof(THPAudioInfo));

    SimpleControl.file = nlOpen(fileName);
    if (!SimpleControl.file)
    {
        return 0;
    }

    nlRead(SimpleControl.file, WorkBuffer, sizeof(WorkBuffer));
    memcpy(((THPSimpleControlWork*)&SimpleControl)->magic, WorkBuffer, 0x30);

    if (strcmp(((THPSimpleControlWork*)&SimpleControl)->magic, "THP") != 0)
    {
        nlClose(((THPSimpleControlWork*)&SimpleControl)->fileInfo);
        ((THPSimpleControlWork*)&SimpleControl)->fileInfo = NULL;
        return 0;
    }

    if (((THPSimpleControlWork*)&SimpleControl)->version != 0x00011000)
    {
        nlClose(((THPSimpleControlWork*)&SimpleControl)->fileInfo);
        ((THPSimpleControlWork*)&SimpleControl)->fileInfo = NULL;
        return 0;
    }

    offset = ((THPSimpleControlWork*)&SimpleControl)->compInfoDataOffsets;
    nlSeek(((THPSimpleControlWork*)&SimpleControl)->fileInfo, offset, 0);
    nlRead(((THPSimpleControlWork*)&SimpleControl)->fileInfo, WorkBuffer, 0x20);
    memcpy(&((THPSimpleControlWork*)&SimpleControl)->compInfo, WorkBuffer, sizeof(THPFrameCompInfo));

    offset += sizeof(THPFrameCompInfo);
    ((THPSimpleControlWork*)&SimpleControl)->audioExist = 0;

    for (i = 0; i < ((THPSimpleControlWork*)&SimpleControl)->compInfo.mNumComponents; i++)
    {
        switch (((THPSimpleControlWork*)&SimpleControl)->compInfo.mFrameComp[i])
        {
        case 0:
            nlSeek(((THPSimpleControlWork*)&SimpleControl)->fileInfo, offset, 0);
            nlRead(((THPSimpleControlWork*)&SimpleControl)->fileInfo, WorkBuffer, 0x20);
            memcpy(&((THPSimpleControlWork*)&SimpleControl)->videoInfo, WorkBuffer, sizeof(THPVideoInfo));
            offset += sizeof(THPVideoInfo);
            break;
        case 1:
            nlSeek(((THPSimpleControlWork*)&SimpleControl)->fileInfo, offset, 0);
            nlRead(((THPSimpleControlWork*)&SimpleControl)->fileInfo, WorkBuffer, 0x20);
            memcpy(&((THPSimpleControlWork*)&SimpleControl)->audioInfo, WorkBuffer, sizeof(THPAudioInfo));
            offset += sizeof(THPAudioInfo);
            ((THPSimpleControlWork*)&SimpleControl)->audioExist = 1;
            break;
        default:
            return 0;
        }
    }

    ((THPSimpleControlWork*)&SimpleControl)->curOffset = ((THPSimpleControlWork*)&SimpleControl)->movieDataOffsets;
    ((THPSimpleControlWork*)&SimpleControl)->readSize = ((THPSimpleControlWork*)&SimpleControl)->firstFrameSize;
    ((THPSimpleControlWork*)&SimpleControl)->readIndex = 0;
    ((THPSimpleControlWork*)&SimpleControl)->totalReadFrame = 0;
    ((THPSimpleControlWork*)&SimpleControl)->dvdError = 0;
    ((THPSimpleControlWork*)&SimpleControl)->textureSet.mFrameNumber = -1;
    ((THPSimpleControlWork*)&SimpleControl)->nextDecodeIndex = 0;
    ((THPSimpleControlWork*)&SimpleControl)->audioDecodeIndex = 0;
    ((THPSimpleControlWork*)&SimpleControl)->audioOutputIndex = 0;
    ((THPSimpleControlWork*)&SimpleControl)->preFetchState = 0;
    ((THPSimpleControlWork*)&SimpleControl)->audioState = 0;
    ((THPSimpleControlWork*)&SimpleControl)->loop = 0;
    ((THPSimpleControlWork*)&SimpleControl)->open = 1;
    ((THPSimpleControlWork*)&SimpleControl)->curVolume = 127.0f;
    ((THPSimpleControlWork*)&SimpleControl)->targetVolume = 127.0f;
    ((THPSimpleControlWork*)&SimpleControl)->rampCount = 0;

    return 1;
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
