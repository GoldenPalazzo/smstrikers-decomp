#include "NL/plat/plataudio.h"
#include "NL/nlMemory.h"
#include "NL/nlFile.h"
#include "NL/nlString.h"
#include "NL/nlTicker.h"
#include "NL/nlFileGC.h"
#include "NL/gl/glMemory.h"
#include "Game/Sys/debug.h"
#include "Game/Audio/AudioLoader.h"
#include "types.h"

extern u32 sndStackGetAvailableSampleMemory(unsigned long id);
extern "C" u32 sndStackSetCurrent(u32 id);
extern "C" u32 sndStackGetSize(void);
extern "C" unsigned long sndStackAdd(void* buffer, u32 aramAddr, u32 size);
extern "C" u32 sndStackGetARAMAddressRange(u32 id, u32* start, u32* end);

static void* gpEntireSampleFileBufferFirstHalf;
static void* gpEntireSampleFileBufferSecondHalf;
static unsigned long gEntireSampleFileFirstHalfAllocSize;
static unsigned long gEntireSampleFileSecondHalfAllocSize;
static void* gpEntireSampleFileMRAMXferBuffer;
static u64 gEntireSampleMarker;
static u8 gAllowSyncReadsPastLoadedData;

// ARAMTransferHelperLoadEntireFile static members
// static u32 m_uFileSize__32ARAMTransferHelperLoadEntireFile;
// static nlFile* s_pFile__32ARAMTransferHelperLoadEntireFile;
const char* m_szFileName__32ARAMTransferHelperLoadEntireFile;
ARAMTransferHelperLoadEntireFile* ARAMTransferHelperLoadEntireFile::m_pARAMHelper;

// ARAMTransferHelper static members
ARAMTransferHelper* ARAMTransferHelper::m_pARAMHelper;
unsigned char ARAMTransferHelper::m_bFileOpened;
nlFile* ARAMTransferHelper::m_pFile;
const char* ARAMTransferHelper::m_szFileName;

#include <dolphin/ai.h>
#include <dolphin/arq.h>

SFXEmitter gEmitters[16];

struct _struct_stack_list_0x10
{
    /* 0x00 */ u32* unk0;        /* inferred */
    /* 0x04 */ unsigned long id; /* stack id for sndStack functions */
    /* 0x08 */ s32 unk8;         /* inferred */
    /* 0x0C */ u32 unkC;         /* inferred */
};

static struct _struct_stack_list_0x10 stack_list[2] = {
    { NULL, 0xFFFFFFFEU, 0, 0U },
    { NULL, 0xFFFFFFFFU, 0x2B4000, 0U },
};

struct EffectSettings
{
    // todo: implement
};

static s32 gDPL2AuxAEffect;
static s32 gDPL2AuxBEffect;
static s32 gDPL2AuxAEffectSettings;
static s32 gDPL2AuxBEffectSettings;
static s32 gAuxAEffect;
static s32 gAuxBEffect;
static s32 gAuxAEffectSettings;
static s32 gAuxBEffectSettings;

static u32 aramMemArray[2];
static void* (*const sndHookMalloc)(size_t) = musyXAlloc;
static void (*const sndHookFree)(void*) = musyXFree;

static bool gAreSoundBuffersSetup;

namespace PlatAudio
{

static u32 gPrimaryStackSize;
bool gUsingDolbyProLogic2 = false;

/**
 * Offset/Address/Size: 0x0 | 0x801C47FC | size: 0x8
 */
u32 GetSndIDError()
{
    return -1;
}

/**
 * Offset/Address/Size: 0x8 | 0x801C4804 | size: 0x30
 */
bool IsSFXPlaying(unsigned long id)
{
    SND_VOICEID result = sndFXCheck(id);
    return result != -1;
}

/**
 * Offset/Address/Size: 0x38 | 0x801C4834 | size: 0xC0
 * TODO: 94.17% match - r0/r5 register swap for true/gEmitters@l and lfs f0 scheduling
 */
void PlatAudio::InitEmitter(unsigned long idx)
{
    SFXEmitter* emitter = &gEmitters[idx];
    SND_PARAMETER_INFO** ppInfo = &emitter->pMIDIControllerInfo;

    gEmitters[idx].bKeepTrack = true;
    emitter->Init();

    emitter->pos.vPos.f.x = 0.0f;
    emitter->pos.vPos.f.y = 0.0f;
    emitter->pos.vPos.f.z = 0.0f;
    emitter->dir.vDir.f.x = 0.0f;
    emitter->dir.vDir.f.y = 0.0f;
    emitter->dir.vDir.f.z = 0.0f;
    emitter->posUpdateMethod = NONE;

    SND_PARAMETER_INFO* pInfo = *ppInfo;
    if (pInfo != NULL)
    {
        if (pInfo->paraArray != NULL)
        {
            delete[] (char*)pInfo->paraArray;
        }
        delete *ppInfo;
    }
    *ppInfo = NULL;
}

/**
 * Offset/Address/Size: 0xF8 | 0x801C48F4 | size: 0x20
 */
bool RemoveEmitter(SFXEmitter* emitter)
{
    return sndRemoveEmitter((SND_EMITTER*)emitter);
}

/**
 * Offset/Address/Size: 0x118 | 0x801C4914 | size: 0x30
 */
bool RemoveEmitter(unsigned long idx)
{
    return sndRemoveEmitter((SND_EMITTER*)&gEmitters[idx]);
}

/**
 * Offset/Address/Size: 0x148 | 0x801C4944 | size: 0x14
 */
SFXEmitter* GetSFXEmitter(unsigned long id)
{
    return &gEmitters[id];
}

/**
 * Offset/Address/Size: 0x15C | 0x801C4958 | size: 0x35C
 * TODO: 84.02% match - stmw/5-reg allocation (r27-r31) vs 3-reg (r29-r31),
 * r4/r5 volatile register swap in reset stores, missing stfsx indexed store
 * in second section (r28=gEmitters+0x58 base not hoisted by compiler)
 */
SFXEmitter* GetFreeEmitter(unsigned long& index)
{
    int i;
    SFXEmitter* emitter = gEmitters;

    index = 0;
    for (i = 0; i < 64; i++)
    {
        if (!sndCheckEmitter((SND_EMITTER*)emitter) && sndFXCheck(sndEmitterVoiceID((SND_EMITTER*)emitter)) == -1
            && !emitter->bIsStopping && !emitter->bInUse)
        {
            sndRemoveEmitter((SND_EMITTER*)&gEmitters[i]);
            gEmitters[i].bInUse = true;
            index = i;
            gEmitters[i].bKeepTrack = true;
            gEmitters[i].soundType = (unsigned long)-1;
            gEmitters[i].fTimeStamp = -1.0f;
            gEmitters[i].bIsStopping = false;
            gEmitters[i].bInUse = false;
            gEmitters[i].bIsFilterOn = false;
            gEmitters[i].m_unk_0x5F = false;
            gEmitters[i].pPhysObj = NULL;
            gEmitters[i].pOwner = NULL;
            gEmitters[i].pos.pvPos = NULL;
            gEmitters[i].dir.pvDir = NULL;
            gEmitters[i].pos.vPos.f.x = 0.0f;
            gEmitters[i].pos.vPos.f.y = 0.0f;
            gEmitters[i].pos.vPos.f.z = 0.0f;
            gEmitters[i].dir.vDir.f.x = 0.0f;
            gEmitters[i].dir.vDir.f.y = 0.0f;
            gEmitters[i].dir.vDir.f.z = 0.0f;
            gEmitters[i].posUpdateMethod = NONE;
            if (gEmitters[i].pMIDIControllerInfo != NULL)
            {
                if (gEmitters[i].pMIDIControllerInfo->paraArray != NULL)
                    delete[] (char*)gEmitters[i].pMIDIControllerInfo->paraArray;
                delete gEmitters[i].pMIDIControllerInfo;
            }
            gEmitters[i].pMIDIControllerInfo = NULL;
            break;
        }
        emitter += 1;
    }

    if (i == 64)
    {
        float min = gEmitters[0].fTimeStamp;
        int minIndex = 0;
        for (i = 1; i < 64; i++)
        {
            if (gEmitters[i].fTimeStamp < min)
            {
                min = gEmitters[i].fTimeStamp;
                minIndex = i;
            }
        }
        sndRemoveEmitter((SND_EMITTER*)&gEmitters[minIndex]);
        gEmitters[minIndex].bInUse = true;
        index = minIndex;
        gEmitters[minIndex].bKeepTrack = true;
        gEmitters[minIndex].soundType = (unsigned long)-1;
        gEmitters[minIndex].fTimeStamp = -1.0f;
        gEmitters[minIndex].bIsStopping = false;
        gEmitters[minIndex].bInUse = false;
        gEmitters[minIndex].bIsFilterOn = false;
        gEmitters[minIndex].m_unk_0x5F = false;
        gEmitters[minIndex].pPhysObj = NULL;
        gEmitters[minIndex].pOwner = NULL;
        gEmitters[minIndex].pos.pvPos = NULL;
        gEmitters[minIndex].dir.pvDir = NULL;
        gEmitters[minIndex].pos.vPos.f.x = 0.0f;
        gEmitters[minIndex].pos.vPos.f.y = 0.0f;
        gEmitters[minIndex].pos.vPos.f.z = 0.0f;
        gEmitters[minIndex].dir.vDir.f.x = 0.0f;
        gEmitters[minIndex].dir.vDir.f.y = 0.0f;
        gEmitters[minIndex].dir.vDir.f.z = 0.0f;
        gEmitters[minIndex].posUpdateMethod = NONE;
        if (gEmitters[minIndex].pMIDIControllerInfo != NULL)
        {
            if (gEmitters[minIndex].pMIDIControllerInfo->paraArray != NULL)
                delete[] (char*)gEmitters[minIndex].pMIDIControllerInfo->paraArray;
            delete gEmitters[minIndex].pMIDIControllerInfo;
        }
        gEmitters[minIndex].pMIDIControllerInfo = NULL;
        tDebugPrintManager::Print(DC_SOUND, "GetFreeEmitter: No free SFX emitters available!\n");
    }

    return &gEmitters[index];
}

/**
 * Offset/Address/Size: 0x4B8 | 0x801C4CB4 | size: 0x20
 */
SND_VOICEID GetEmitterVoiceID(SFXEmitter* emitter)
{
    return sndEmitterVoiceID((SND_EMITTER*)emitter);
}

/**
 * Offset/Address/Size: 0x4D8 | 0x801C4CD4 | size: 0x20
 */
bool IsEmitterActive(SFXEmitter* emitter)
{
    return sndCheckEmitter((SND_EMITTER*)emitter);
}

/**
 * Offset/Address/Size: 0x4F8 | 0x801C4CF4 | size: 0x90
 */
void Update3DSFXEmitter(SFXEmitter* pSFXEmitter, const nlVector3& position, const nlVector3& direction, float maxVol)
{
    float var_f0;
    float temp_f6 = 127.0f * maxVol;

    SND_FVECTOR svPos;
    svPos.x = position.f.x;
    svPos.y = position.f.y;
    svPos.z = position.f.z;

    SND_FVECTOR svDir;
    svDir.x = direction.f.x;
    svDir.y = direction.f.y;
    svDir.z = direction.f.z;

    if (temp_f6 < 0.0f)
    {
        var_f0 = -0.5f;
    }
    else
    {
        var_f0 = 0.5f;
    }
    temp_f6 += var_f0;
    sndUpdateEmitter(&pSFXEmitter->emitter, &svPos, &svDir, (u8)(s32)temp_f6, NULL);
}

/**
 * Offset/Address/Size: 0x588 | 0x801C4D84 | size: 0x2E4
 * TODO: 98.49% match - remaining MWCC f-register scheduling in reverb conversion
 * and pitch/debug-print register allocation (r5 vs r0 and lis/addi register choice).
 */
void Add3DSFXEmitter(const EmitterStartInfo& info)
{
    SND_FVECTOR svPos;
    SND_FVECTOR svDir;
    SFXEmitter* pSFXEmitter;
    unsigned long flags;
    unsigned long numPara;
    int currParaIndex;
    SND_PARAMETER_INFO* pParaInfo;
    SND_PARAMETER* pParaArray;
    SND_PARAMETER_INFO tempParaInfo;
    SND_PARAMETER tempParaArray[4];

    svPos.x = info.position.f.x;
    svPos.y = info.position.f.y;
    svPos.z = info.position.f.z;

    svDir.x = info.direction.f.x;
    svDir.y = info.direction.f.y;
    svDir.z = info.direction.f.z;

    pSFXEmitter = info.pSFXEmitter;
    flags = 0;

    if (pSFXEmitter == NULL)
    {
        flags = 0;
    }
    else
    {
        if (info.bContinuous)
        {
            flags |= SND_EMITTER_CONTINUOUS;
        }

        if (info.bRestartable)
        {
            flags |= SND_EMITTER_RESTARTABLE;
        }

        if (info.bPausable)
        {
            flags |= SND_EMITTER_PAUSABLE;
        }

        if (info.bUseDoppler)
        {
            flags |= SND_EMITTER_DOPPLERFX;
        }

        if (info.bHardStart)
        {
            flags |= SND_EMITTER_HARDSTART;
        }
    }

    numPara = 0;
    currParaIndex = 0;
    pParaInfo = NULL;
    pParaArray = NULL;

    if (info.fVolReverb != 0.0f)
    {
        numPara = 1;
    }

    if (info.pitch != 0x2000)
    {
        numPara += 1;
    }

    if (info.bActivateFilter)
    {
        numPara += 2;
    }

    if (numPara != 0)
    {
        if (pSFXEmitter == NULL)
        {
            pParaInfo = &tempParaInfo;
            pParaArray = tempParaArray;
        }
        else
        {
            void* alloc = nlMalloc(8, 8, false);
            pParaInfo = (SND_PARAMETER_INFO*)alloc;
            pParaArray = (SND_PARAMETER*)nlMalloc(numPara * sizeof(SND_PARAMETER), 8, false);
        }

        pParaInfo->numPara = numPara;
        pParaInfo->paraArray = pParaArray;
    }

    if (info.fVolReverb != 0.0f)
    {
        float var_f1 = info.fVolReverb;
        float var_f2 = 127.0f;
        float var_f0 = 0.0f;
        pParaArray[0].ctrl = 0x5B;
        var_f1 = var_f2 * var_f1;
        if (var_f1 < var_f0)
        {
            var_f0 = -0.5f;
        }
        else
        {
            var_f0 = 0.5f;
        }
        var_f0 = var_f1 + var_f0;

        currParaIndex = 1;
        pParaArray[0].paraData.value7 = (u8)(s32)var_f0;
    }

    if (info.pitch != 0x2000)
    {
        if (info.pitch != 0x2000)
        {
            const char* msg = "pitch bend should be non-default\n";
            tDebugPrintManager::Print(DC_SOUND, msg);
        }

        pParaArray[currParaIndex].ctrl = 0x80;
        pParaArray[currParaIndex].paraData.value14 = info.pitch;
        currParaIndex += 1;
    }

    if (info.bActivateFilter)
    {
        unsigned long idx = currParaIndex;
        unsigned long freq = info.filterFreq;

        pParaArray[idx].ctrl = 0x4F;
        pParaArray[idx].paraData.value14 = 0x2000;

        if (freq > 0x3FFF)
        {
            freq = 0x3FFF;
        }

        idx = currParaIndex + 1;
        pParaArray[idx].ctrl = 1;
        pParaArray[idx].paraData.value14 = (u16)freq;

        if (pSFXEmitter != NULL)
        {
            pSFXEmitter->bIsFilterOn = true;
        }
    }

    if (pSFXEmitter != NULL)
    {
        pSFXEmitter->pMIDIControllerInfo = pParaInfo;
    }

    {
        float var_f0;
        float temp_f6 = 127.0f * info.minVol;
        if (temp_f6 < 0.0f)
        {
            var_f0 = -0.5f;
        }
        else
        {
            var_f0 = 0.5f;
        }
        temp_f6 += var_f0;

        u8 minVol = (u8)(s32)temp_f6;

        temp_f6 = 127.0f * info.maxVol;
        if (temp_f6 < 0.0f)
        {
            var_f0 = -0.5f;
        }
        else
        {
            var_f0 = 0.5f;
        }
        temp_f6 += var_f0;

        sndAddEmitter2StudioPara((SND_EMITTER*)pSFXEmitter, &svPos, &svDir, info.maxDist, info.comp, flags, (u16)info.uSFXID, (u8)(s32)temp_f6, minVol, 0, pParaInfo);
    }
}

/**
 * Offset/Address/Size: 0x86C | 0x801C5068 | size: 0x20
 */
void Remove3DSFXListener(SND_LISTENER* listener)
{
    sndRemoveListener(listener);
}

/**
 * Offset/Address/Size: 0x88C | 0x801C5088 | size: 0xC8
 */
void Update3DSFXListener(SND_LISTENER* pListener, const nlVector3& position, const nlVector3& direction, const nlVector3& heading, const nlVector3& up, float overallEmitterVol)
{
    SND_FVECTOR sndPos;
    sndPos.x = position.f.x;
    sndPos.y = position.f.y;
    sndPos.z = position.f.z;

    SND_FVECTOR sndDir;
    sndDir.x = direction.f.x;
    sndDir.y = direction.f.y;
    sndDir.z = direction.f.z;

    SND_FVECTOR sndHeading;
    sndHeading.x = heading.f.x;
    sndHeading.y = heading.f.y;
    sndHeading.z = heading.f.z;

    SND_FVECTOR sndUp;
    sndUp.x = up.f.x;
    sndUp.y = up.f.y;
    sndUp.z = up.f.z;

    f32 rounded = 127.0f * overallEmitterVol;
    f32 adj;
    if (rounded < 0.0f)
    {
        adj = -0.5f;
    }
    else
    {
        adj = 0.5f;
    }
    rounded += adj;

    sndUpdateListener(pListener, &sndPos, &sndDir, &sndHeading, &sndUp, (u8)(s32)rounded, NULL);
}

/**
 * Offset/Address/Size: 0x954 | 0x801C5150 | size: 0x10C
 */
void Add3DSFXListener(SND_LISTENER* li, const nlVector3& pos, const nlVector3& dir, const nlVector3& heading, const nlVector3& up, float front_sur, float back_sur, float vol, float volPosOffset, bool doppler, float soundSpeed)
{
    SND_FVECTOR sndPos;
    sndPos.x = pos.f.x;
    sndPos.y = pos.f.y;
    sndPos.z = pos.f.z;

    SND_FVECTOR sndDir;
    sndDir.x = dir.f.x;
    sndDir.y = dir.f.y;
    sndDir.z = dir.f.z;

    SND_FVECTOR sndHeading;
    sndHeading.x = heading.f.x;
    sndHeading.y = heading.f.y;
    sndHeading.z = heading.f.z;

    SND_FVECTOR sndUp;
    sndUp.x = up.f.x;
    sndUp.y = up.f.y;
    sndUp.z = up.f.z;

    f32 rounded = 127.0f * vol;
    f32 adj;
    if (rounded < 0.0f)
    {
        adj = -0.5f;
    }
    else
    {
        adj = 0.5f;
    }
    rounded += adj;

    u32 flags = doppler ? 1 : 0;

    sndAddListenerEx(li, &sndPos, &sndDir, &sndHeading, &sndUp, front_sur, back_sur, soundSpeed, volPosOffset, flags, (u8)(s32)rounded, NULL);
}

/**
 * Offset/Address/Size: 0xA60 | 0x801C525C | size: 0x68
 */
bool SetPitchBendOnSFX(SND_VOICEID vid, u16 value)
{
    if (vid == -1 || sndFXCheck(vid) == -1)
    {
        return true;
    }
    return sndFXCtrl14(vid, 0x80, value);
}

/**
 * Offset/Address/Size: 0xAC8 | 0x801C52C4 | size: 0x7C
 */
bool SetFilterFreqOnSFX(SND_VOICEID vid, u16 value)
{
    u16 v = value;
    if (value > 0x3FFFU)
    {
        v = 0x3FFF;
    }

    if (vid == -1 || sndFXCheck(vid) == -1)
    {
        return true;
    }
    return sndFXCtrl14(vid, 0x1, v);
}

/**
 * Offset/Address/Size: 0xB44 | 0x801C5340 | size: 0x20
 */
// void SetMIDIControllerVal14Bit(unsigned long, unsigned char, unsigned short)
bool SetMIDIControllerVal14Bit(SND_VOICEID vid, u8 ctrl, u16 value)
{
    return sndFXCtrl14(vid, ctrl, value);
}

/**
 * Offset/Address/Size: 0xB64 | 0x801C5360 | size: 0x58
 */
void SetVolGroupVolume(u8 volGroup, float volume, u16 time)
{
    f32 temp_f1;
    f32 var_f0;

    temp_f1 = 127.0f * volume;
    if (temp_f1 < 0.0f)
    {
        var_f0 = -0.5f;
    }
    else
    {
        var_f0 = 0.5f;
    }
    sndVolume((s8)(temp_f1 + var_f0), time, volGroup);
}

/**
 * Offset/Address/Size: 0xBBC | 0x801C53B8 | size: 0x24
 */
bool SetSFXVolumeGroup(u32 fid, u8 vGroup)
{
    return sndFXAssignVolGroup2FXId((SND_FXID)fid, vGroup);
}

/**
 * Offset/Address/Size: 0xBE0 | 0x801C53DC | size: 0x68
 */
bool SetSFXReverbVol(SND_VOICEID vid, float value)
{
    int v;

    // u8 ctrl = 0x5B;
    if (value == 1.0f)
    {
        value = 0.0f; // Fallback
    }

    float value2 = 255.0f * value;
    value2 += (value2 < 0.0f ? -0.5f : 0.5f);
    v = value2;
    return sndFXCtrl(vid, 0x5B, v);
}

/**
 * Offset/Address/Size: 0xC48 | 0x801C5444 | size: 0x88
 */
void SetSFXVolume(unsigned long voiceID, float volume)
{
    volume = (volume >= 0.0f) ? volume : 0.0f;
    volume = (volume <= 1.0f) ? volume : 1.0f;

    float v = 127.0f * volume;
    float r = (v < 0.0f) ? -0.5f : 0.5f;
    v = v + r;
    sndFXCtrl(voiceID, 7, (u8)(s32)v);
}

/**
 * Offset/Address/Size: 0xCD0 | 0x801C54CC | size: 0x48
 */
bool StopSFX(unsigned long handle)
{
    if (handle == 0xFFFFFFFF)
    {
        return false;
    }
    bool result = sndFXKeyOff(handle);
    if (result)
    {
        return true;
    }
    return result;
}

/**
 * Offset/Address/Size: 0xD18 | 0x801C5514 | size: 0x244
 * TODO: 97.72% match - remaining differences are concentrated in the pitch/filter
 * parameter block: r5/r0 pitch bend compare, Print call lis/li ordering, and
 * r3/r0 plus r4/r5 register swaps during ctrl/freq writes.
 */
#pragma opt_common_subs off
unsigned long PlaySFX(const SFXStartInfo& info)
{
    const SFXStartInfo* pInfo = &info;
    u8 uVolume;
    u8 uPan;
    SND_PARAMETER tempParaArray[4];
    int currParaIndex;
    SND_PARAMETER* pParaArray;
    SND_PARAMETER_INFO tempParaInfo;
    unsigned long numPara;

    float f2 = pInfo->fVolume;
    if (0.0f == f2)
    {
        uVolume = 0xFF;
    }
    else if (f2 > 1.0f)
    {
        uVolume = 0x7F;
    }
    else if (f2 < -0.0f)
    {
        uVolume = 0;
    }
    else
    {
        float f0;
        f2 = 127.0f * f2;
        if (f2 < -0.0f)
        {
            f0 = -0.5f;
        }
        else
        {
            f0 = 0.5f;
        }
        f0 = f2 + f0;
        uVolume = (u8)(s32)f0;
    }

    if (0.0f == pInfo->fPan)
    {
        uPan = 0xFF;
    }
    else
    {
        float f3 = 0.5f;
        float f1 = 127.0f * (f3 * (1.0f + pInfo->fPan));
        if (f1 < -0.0f)
        {
            f3 = -0.5f;
        }
        float f0 = f1 + f3;
        uPan = (u8)(s32)f0;
    }

    numPara = 0;
    currParaIndex = 0;
    tempParaInfo.numPara = 0;
    tempParaInfo.paraArray = NULL;
    pParaArray = NULL;

    if (0.0f != pInfo->fVolReverb)
    {
        numPara = 1;
    }

    if (pInfo->uPitchBend != 0x2000)
    {
        numPara += 1;
    }

    if (pInfo->bActivateFilter)
    {
        numPara += 2;
    }

    if (numPara != 0)
    {
        pParaArray = tempParaArray;
        tempParaInfo.numPara = numPara;
        tempParaInfo.paraArray = pParaArray;
    }

    f2 = pInfo->fVolReverb;
    if (0.0f != f2)
    {
        float f0;
        float f1v = 127.0f * f2;
        pParaArray[0].ctrl = 0x5B;
        if (f1v < -0.0f)
        {
            f0 = -0.5f;
        }
        else
        {
            f0 = 0.5f;
        }
        f0 = f1v + f0;
        currParaIndex = 1;
        pParaArray[0].paraData.value7 = (u8)(s32)f0;
    }

    if (pInfo->uPitchBend != 0x2000)
    {
        if (pInfo->uPitchBend != 0x2000)
        {
            tDebugPrintManager::Print(DC_SOUND, "pitch bend should be non-default\n");
        }

        pParaArray[currParaIndex].ctrl = 0x80;
        pParaArray[currParaIndex].paraData.value14 = pInfo->uPitchBend;
        currParaIndex += 1;
    }

    if (pInfo->bActivateFilter)
    {
        unsigned long idx = currParaIndex;
        u16 freq = pInfo->filterFreq;

        pParaArray[idx].ctrl = 0x4F;
        pParaArray[idx].paraData.value14 = 0x2000;

        if (freq > 0x3FFF)
        {
            freq = 0x3FFF;
        }

        idx = currParaIndex + 1;
        pParaArray[idx].ctrl = 1;
        pParaArray[idx].paraData.value14 = freq;
    }

    return sndFXStartParaInfo((u16)pInfo->uSFXID, uVolume, uPan, 0, &tempParaInfo);
}
#pragma opt_common_subs on

/**
 * Offset/Address/Size: 0xF5C | 0x801C5758 | size: 0x120
 * TODO: 99.38% match - r6/r7 register swap for loop counter vs grp pointer
 */
bool UnloadAllSoundGroupsOnStack(AudioFileData& fileData, unsigned long stackEnum)
{
    int i;

    if (!(unsigned char)sndStackSetCurrent(stack_list[stackEnum].id))
    {
        tDebugPrintManager::Print(DC_SOUND, "sndStackSetCurrent failed for stack %d\n", stack_list[stackEnum].id);
        return 0;
    }

    for (i = 0; (unsigned long)i < stack_list[stackEnum].unkC; i++)
    {
        if (!sndPopGroup())
        {
            tDebugPrintManager::Print(DC_SOUND, "sndPopGroup failed on stack %d\n", stackEnum);
            return 0;
        }
        PrintSoundStackInfo();
    }

    stack_list[stackEnum].unkC = 0;

    {
        SndGroupData* grp;
        i = 0;
        while (i < fileData.numSoundGroups)
        {
            grp = &fileData.soundGroups[i];
            if (stackEnum == (unsigned long)grp->stackEnum)
            {
                grp->stackEnum = -1;
                grp->uLoadOrder = -1;
                grp->loadType = (LoadType)0;
            }
            i++;
        }
    }

    return true;
}

static inline SndGroupData* GetSoundGroupAt(SndGroupData* data, int idx)
{
    return &data[idx];
}

/**
 * Offset/Address/Size: 0x107C | 0x801C5878 | size: 0x150
 * TODO: 98.93% match - remaining MWCC register allocator diffs: r29/r30 swap
 * (fileData/offset) and r5/r6 swap in the first soundGroups loop
 */
bool UnloadAllSoundGroups(AudioFileData& fileData)
{
    int i;
    unsigned long* idPtr;
    _struct_stack_list_0x10* entry;

    sndSilence();

    char* base = (char*)stack_list;
    int stackEnum = 1;
    int offset = 0x10;

    do
    {
        entry = (_struct_stack_list_0x10*)(base + offset);
        unsigned long id = entry->id;
        idPtr = &entry->id;
        if (!(unsigned char)sndStackSetCurrent(id))
        {
            tDebugPrintManager::Print(DC_SOUND, "sndStackSetCurrent failed for stack %d\n", *idPtr);
            goto next;
        }

        {
            u32* numPtr = &entry->unkC;
            i = 0;
            while ((unsigned long)i < *numPtr)
            {
                if (!sndPopGroup())
                {
                    tDebugPrintManager::Print(DC_SOUND, "sndPopGroup failed on stack %d\n", stackEnum);
                    goto next;
                }
                PrintSoundStackInfo();
                i++;
            }

            *numPtr = 0;
        }

        {
            SndGroupData* grp;
            i = 0;
            while (i < fileData.numSoundGroups)
            {
                grp = GetSoundGroupAt(fileData.soundGroups, i);
                if ((unsigned long)stackEnum == (unsigned long)grp->stackEnum)
                {
                    grp->stackEnum = -1;
                    grp->uLoadOrder = -1;
                    grp->loadType = (LoadType)0;
                }
                i++;
            }
        }

    next:
        stackEnum--;
        offset -= 0x10;
    } while (stackEnum >= 0);

    {
        SndGroupData* grp;
        i = 0;
        while (i < fileData.numSoundGroups)
        {
            grp = GetSoundGroupAt(fileData.soundGroups, i);
            grp->stackEnum = -1;
            grp->uLoadOrder = -1;
            grp->loadType = (LoadType)0;
            i++;
        }
    }

    return true;
}

/**
 * Offset/Address/Size: 0x11CC | 0x801C59C8 | size: 0x184
 */
bool UnloadSoundGroup(AudioFileData& fileData, unsigned long groupEnum)
{
    u32 uTickStart = nlGetTicker();

    if ((unsigned long)fileData.soundGroups[groupEnum].uLoadOrder == stack_list[fileData.soundGroups[groupEnum].stackEnum].unkC - 1)
    {
        if (!(unsigned char)sndStackSetCurrent(stack_list[fileData.soundGroups[groupEnum].stackEnum].id))
        {
            tDebugPrintManager::Print(DC_SOUND, "sndStackSetCurrent failed for stack %d\n", stack_list[fileData.soundGroups[groupEnum].stackEnum].id);
            return false;
        }

        if (!(unsigned char)sndPopGroup())
        {
            tDebugPrintManager::Print(DC_SOUND, "sndPopGroup failed for %s\n", fileData.soundGroups[groupEnum].szGroupName);
            return false;
        }

        stack_list[fileData.soundGroups[groupEnum].stackEnum].unkC--;

        u32 uTickEnd = nlGetTicker();
        f32 fTime = nlGetTickerDifference(uTickStart, uTickEnd) / 1000.0f;

        tDebugPrintManager::Print(DC_SOUND, "Unloaded %s from stack %d in %f seconds\n", fileData.soundGroups[groupEnum].szGroupName, stack_list[fileData.soundGroups[groupEnum].stackEnum].id, fTime);
        PrintSoundStackInfo();

        SndGroupData* grp = &fileData.soundGroups[groupEnum];
        grp->stackEnum = -1;
        grp->uLoadOrder = -1;
        grp->loadType = (LoadType)0;

        return true;
    }

    tDebugPrintManager::Print(DC_SOUND, "Can't unload %s\n", fileData.soundGroups[groupEnum].szGroupName);
    return false;
}

/**
 * Offset/Address/Size: 0x1350 | 0x801C5B4C | size: 0x328
 * 99.85% match - i diffs only (string pool index differences)
 */
bool LoadSoundGroup(AudioFileData& fileData, unsigned long groupEnum, unsigned long stackEnum, bool bUseARAMStreamCallback)
{
    unsigned long uTickStart;
    unsigned short groupID = fileData.soundGroups[groupEnum].groupID;
    uTickStart = nlGetTicker();

    if (!(unsigned char)sndStackSetCurrent(stack_list[stackEnum].id))
    {
        tDebugPrintManager::Print(DC_SOUND, "sndStackSetCurrent failed for stack %d\n", stack_list[stackEnum].id);
        return false;
    }

    ARAMTransferHelper* pTransferHelperLoadFromDisc = NULL;
    ARAMTransferHelperLoadEntireFile* pTransferHelperLoadEntireFile = NULL;

    if (bUseARAMStreamCallback)
    {
        pTransferHelperLoadFromDisc = (ARAMTransferHelper*)nlMalloc(0x10, 0x20, true);
        if (pTransferHelperLoadFromDisc != NULL)
        {
            const char* szFile = fileData.szSampleFile;
            pTransferHelperLoadFromDisc->m_pARAMXferBlockBaseAddress = NULL;
            pTransferHelperLoadFromDisc->m_uCachedDataOffset = -1;
            pTransferHelperLoadFromDisc->m_pDiskCacheBaseAddress = NULL;
            ARAMTransferHelper::m_szFileName = szFile;

            pTransferHelperLoadFromDisc->m_pDiskCacheBaseAddress = (unsigned char*)nlMalloc(0x20000, 0x20, true);
            pTransferHelperLoadFromDisc->m_pARAMXferBlockBaseAddress = (unsigned char*)nlMalloc(0x20000, 0x20, true);

            ARAMTransferHelper::m_pFile = nlOpen(ARAMTransferHelper::m_szFileName);
            {
                unsigned int allocSize;
                pTransferHelperLoadFromDisc->m_uFileSize = nlFileSize(ARAMTransferHelper::m_pFile, &allocSize);
            }
            nlClose(ARAMTransferHelper::m_pFile);
            ARAMTransferHelper::m_pFile = NULL;

            tDebugPrintManager::Print(DC_SOUND, "sound group file size %d\n", pTransferHelperLoadFromDisc->m_uFileSize);

            ARAMTransferHelper::m_pARAMHelper = pTransferHelperLoadFromDisc;
        }

        ARQSetChunkSize(0x20000);
        sndSetSampleDataUploadCallback(ARAMTransferHelper::sndPushGroupCallback, 0x20000);
    }
    else
    {
        pTransferHelperLoadEntireFile = (ARAMTransferHelperLoadEntireFile*)nlMalloc(4, 0x20, true);
        if (pTransferHelperLoadEntireFile != NULL)
        {
            const char* szFile = fileData.szSampleFile;
            pTransferHelperLoadEntireFile->m_pARAMXferBlockBaseAddress = NULL;
            m_szFileName__32ARAMTransferHelperLoadEntireFile = szFile;
            pTransferHelperLoadEntireFile->m_pARAMXferBlockBaseAddress = (unsigned char*)nlMalloc(0x20000, 0x20, true);
            ARAMTransferHelperLoadEntireFile::m_pARAMHelper = pTransferHelperLoadEntireFile;
        }

        ARQSetChunkSize(0x20000);
        sndSetSampleDataUploadCallback(ARAMTransferHelperLoadEntireFile::sndPushGroupCallback, 0x20000);
    }

    if (!(unsigned char)sndPushGroup(fileData.proj_buffer, groupID, 0, fileData.sdir_buffer, fileData.pool_buffer))
    {
        tDebugPrintManager::Print(DC_SOUND, "sndPushGroup failed for group %d\n", groupID);
        return false;
    }

    fileData.soundGroups[groupEnum].stackEnum = stackEnum;

    unsigned long uLoadOrder = stack_list[stackEnum].unkC;
    stack_list[stackEnum].unkC = uLoadOrder + 1;
    fileData.soundGroups[groupEnum].uLoadOrder = uLoadOrder;

    int loadType = 1;
    if (pTransferHelperLoadFromDisc != NULL)
    {
        loadType = 2;
    }
    fileData.soundGroups[groupEnum].loadType = (LoadType)loadType;

    if (pTransferHelperLoadFromDisc != NULL)
    {
        if (pTransferHelperLoadFromDisc != NULL)
        {
            if (ARAMTransferHelper::m_bFileOpened)
            {
                nlClose(ARAMTransferHelper::m_pFile);
                ARAMTransferHelper::m_pFile = NULL;
                ARAMTransferHelper::m_bFileOpened = 0;
            }

            if (pTransferHelperLoadFromDisc->m_pARAMXferBlockBaseAddress != NULL)
            {
                delete[] pTransferHelperLoadFromDisc->m_pARAMXferBlockBaseAddress;
                pTransferHelperLoadFromDisc->m_pARAMXferBlockBaseAddress = NULL;
            }

            if (pTransferHelperLoadFromDisc->m_pDiskCacheBaseAddress != NULL)
            {
                delete[] pTransferHelperLoadFromDisc->m_pDiskCacheBaseAddress;
                pTransferHelperLoadFromDisc->m_pDiskCacheBaseAddress = NULL;
            }

            ARAMTransferHelper::m_pARAMHelper = NULL;
            delete pTransferHelperLoadFromDisc;
        }
    }

    if (pTransferHelperLoadEntireFile != NULL)
    {
        if (pTransferHelperLoadEntireFile != NULL)
        {
            if (pTransferHelperLoadEntireFile->m_pARAMXferBlockBaseAddress != NULL)
            {
                delete[] pTransferHelperLoadEntireFile->m_pARAMXferBlockBaseAddress;
                pTransferHelperLoadEntireFile->m_pARAMXferBlockBaseAddress = NULL;
            }

            if (ARAMTransferHelperLoadEntireFile::s_pFile != NULL)
            {
                nlClose(ARAMTransferHelperLoadEntireFile::s_pFile);
            }

            ARAMTransferHelperLoadEntireFile::m_pARAMHelper = NULL;
            delete pTransferHelperLoadEntireFile;
        }
    }

    {
        unsigned long uTickEnd = nlGetTicker();
        float fTime = nlGetTickerDifference(uTickStart, uTickEnd) / 1000.0f;
        nlPrintf("Loaded %s from stack %d in %f seconds\n", fileData.soundGroups[groupEnum].szGroupName, stack_list[stackEnum].id, fTime);
    }

    PrintSoundStackInfo();
    return true;
}

/**
 * Offset/Address/Size: 0x1678 | 0x801C5E74 | size: 0x16C
 * 99.95% match - i diffs only (string pool index differences)
 */
void SetupSoundBuffers(AudioFileData& fileData, bool bStream)
{
    unsigned long fileSize1;
    unsigned long fileSize2;
    unsigned long fileSize3;
    char* fn;
    unsigned char* buf;

    u32 uTickStart = nlGetTicker();

    if (!fileData.pool_buffer)
    {
        fn = fileData.szPoolFile;
        buf = (unsigned char*)nlLoadEntireFile(fn, &fileSize1, 0x20, AllocateStart);
        if (!buf)
            tDebugPrintManager::Print(DC_SOUND, "Failed to open file %s\n", fn);
        fileData.pool_buffer = buf;
    }

    if (!fileData.proj_buffer)
    {
        fn = fileData.szProjectFile;
        buf = (unsigned char*)nlLoadEntireFile(fn, &fileSize2, 0x20, AllocateStart);
        if (!buf)
            tDebugPrintManager::Print(DC_SOUND, "Failed to open file %s\n", fn);
        fileData.proj_buffer = buf;
    }

    if (!fileData.sdir_buffer)
    {
        fn = fileData.szDirFile;
        buf = (unsigned char*)nlLoadEntireFile(fn, &fileSize3, 0x20, AllocateStart);
        if (!buf)
            tDebugPrintManager::Print(DC_SOUND, "Failed to open file %s\n", fn);
        fileData.sdir_buffer = buf;
    }

    gAreSoundBuffersSetup = 1;

    u32 uTickEnd = nlGetTicker();
    f32 fTime = nlGetTickerDifference(uTickStart, uTickEnd);

    if (!bStream)
    {
        tDebugPrintManager::Print(DC_SOUND, "Immediate Audio load: %0.3f seconds\n", fTime / 1000.0f);
    }
    else
    {
        tDebugPrintManager::Print(DC_SOUND, "Setting up MusyX buffers and stream load callback: %0.3f seconds\n", fTime / 1000.0f);
    }
}

/**
 * Offset/Address/Size: 0x17E4 | 0x801C5FE0 | size: 0x20
 */
void StopAllSound()
{
    sndSilence();
}

/**
 * Offset/Address/Size: 0x1804 | 0x801C6000 | size: 0x78
 */
void Shutdown()
{
    sndQuit();

    u32 length;
    for (int i = 0; i < 2; i++)
    {
        ARFree(&length);
        delete stack_list[i].unk0;
        stack_list[i].unk0 = NULL;
    }

    AIReset();
    ARQReset();
    ARReset();
}

/**
 * Offset/Address/Size: 0x187C | 0x801C6078 | size: 0x11C
 * 99.86% match - i diffs only (sndHookMalloc/sndHookFree symbol names vs @495 anonymous label)
 */
bool Initialize(bool bUseDSP)
{
    SND_HOOKS hooks;

    tDebugPrintManager::Print(DC_SOUND, "Initializing PlatAudio\n");

    hooks.malloc = sndHookMalloc;
    hooks.free = sndHookFree;

    ARInit(aramMemArray, 2);
    ARQInit();
    AIInit(NULL);

    ARAlloc(gPrimaryStackSize);

    sndSetHooks(&hooks);

    u32 flags = 0;
    flags |= 0x2;
    if (bUseDSP)
    {
        flags |= 0x1;
    }

    if (!sndIsInstalled())
    {
        sndInit(0x40, 0, 0x40, 1, flags, gPrimaryStackSize);
    }

    _struct_stack_list_0x10* pStack = &stack_list[1];
    u32 aramAddr = ARAlloc(pStack->unk8);
    u32 stackSize = sndStackGetSize();
    u32* buffer = (u32*)nlMalloc(stackSize, 8, false);
    pStack->id = sndStackAdd(buffer, aramAddr, pStack->unk8);
    pStack->unk0 = buffer;

    sndVolume(0x7F, 0, 0xFF);
    sndOutputMode(SND_OUTPUTMODE_STEREO);
    PrintSoundStackInfo();

    return true;
}

/**
 * Offset/Address/Size: 0x1998 | 0x801C6194 | size: 0x74
 */
void PurgeSampleFileBuffer()
{
    if (gpEntireSampleFileBufferFirstHalf != NULL)
    {
        glResourceRelease(gEntireSampleMarker);
        gEntireSampleMarker = 0;
        gpEntireSampleFileBufferFirstHalf = NULL;
    }

    if (gpEntireSampleFileBufferSecondHalf != NULL)
    {
        nlVirtualFree(gpEntireSampleFileBufferSecondHalf);
        gpEntireSampleFileBufferSecondHalf = NULL;
        nlFree(gpEntireSampleFileMRAMXferBuffer);
        gpEntireSampleFileMRAMXferBuffer = NULL;
    }

    gAllowSyncReadsPastLoadedData = 0;
}

/**
 * Offset/Address/Size: 0x1A0C | 0x801C6208 | size: 0x14
 */
bool IsEntireSampleFileInMem()
{
    return gpEntireSampleFileBufferSecondHalf != NULL;
}

/**
 * Offset/Address/Size: 0x1A20 | 0x801C621C | size: 0x124
 */
unsigned char ReadEntireSampleFileIntoMemSync(const char* sampleFile)
{
    ARAMTransferHelperLoadEntireFile::s_pFile = nlOpen(sampleFile);
    if (ARAMTransferHelperLoadEntireFile::s_pFile == NULL)
    {
        tDebugPrintManager::Print(DC_SOUND, "ReadEntireSampleFileIntoMem: Could not open file.\n");
        return 0;
    }

    gAllowSyncReadsPastLoadedData = 1;
    unsigned int allocSize;
    unsigned int fileSize = nlFileSize(ARAMTransferHelperLoadEntireFile::s_pFile, &allocSize);
    if (fileSize != 0)
    {
        gEntireSampleFileFirstHalfAllocSize = glx_GetFreeMemory() - 0x400;
        unsigned long firstHalfSize = (gEntireSampleFileFirstHalfAllocSize <= fileSize) ? gEntireSampleFileFirstHalfAllocSize : fileSize;
        gEntireSampleFileFirstHalfAllocSize = firstHalfSize;

        gEntireSampleMarker = glResourceMark();
        gpEntireSampleFileBufferFirstHalf = glResourceAlloc(gEntireSampleFileFirstHalfAllocSize, GLM_TextureData);
        *(u32*)gpEntireSampleFileBufferFirstHalf = 0;
        nlRead(ARAMTransferHelperLoadEntireFile::s_pFile, gpEntireSampleFileBufferFirstHalf, gEntireSampleFileFirstHalfAllocSize);

        fileSize -= gEntireSampleFileFirstHalfAllocSize;
        unsigned int virtualFree = nlVirtualLargestBlock() - 0x400;
        if (virtualFree <= fileSize)
        {
            fileSize = virtualFree;
        }
        gEntireSampleFileSecondHalfAllocSize = fileSize;
        if (fileSize != 0)
        {
            gpEntireSampleFileBufferSecondHalf = nlVirtualAlloc(fileSize, true);
            *(u32*)gpEntireSampleFileBufferSecondHalf = 0;
            nlReadToVirtualMemory(ARAMTransferHelperLoadEntireFile::s_pFile, gpEntireSampleFileBufferSecondHalf, gEntireSampleFileSecondHalfAllocSize, 0x80000);
        }
    }

    nlClose(ARAMTransferHelperLoadEntireFile::s_pFile);
    ARAMTransferHelperLoadEntireFile::s_pFile = NULL;
    return 1;
}

/**
 * Offset/Address/Size: 0x1B44 | 0x801C6340 | size: 0x100
 */
unsigned char ReadEntireSampleFileIntoMem(const char* sampleFile)
{
    unsigned int allocSize;

    ARAMTransferHelperLoadEntireFile::s_pFile = nlOpen(sampleFile);
    if (ARAMTransferHelperLoadEntireFile::s_pFile == NULL)
    {
        tDebugPrintManager::Print(DC_SOUND, "ReadEntireSampleFileIntoMem: Could not open file.\n");
        return 0;
    }

    unsigned long SecondHalfSize = nlFileSize(ARAMTransferHelperLoadEntireFile::s_pFile, &allocSize);
    if (SecondHalfSize != 0)
    {
        gEntireSampleFileFirstHalfAllocSize = glx_GetFreeMemory() - 0x400;
        gEntireSampleMarker = glResourceMark();

        void* buffer = glResourceAlloc(gEntireSampleFileFirstHalfAllocSize, GLM_TextureData);
        *(u32*)buffer = 0;
        nlReadAsync(ARAMTransferHelperLoadEntireFile::s_pFile, buffer, gEntireSampleFileFirstHalfAllocSize, ARAMTransferHelperLoadEntireFile::LoadEntireFileCallback, 0);

        gpEntireSampleFileMRAMXferBuffer = nlMalloc(0x80000, 0x20, true);
        gEntireSampleFileSecondHalfAllocSize = SecondHalfSize - gEntireSampleFileFirstHalfAllocSize;
        void* vBuffer = nlVirtualAlloc(gEntireSampleFileSecondHalfAllocSize, true);
        *(u32*)vBuffer = 0;
        nlReadAsyncToVirtualMemory(ARAMTransferHelperLoadEntireFile::s_pFile, vBuffer, gEntireSampleFileSecondHalfAllocSize, ARAMTransferHelperLoadEntireFile::LoadEntireFileCallback, 1, 0x80000, gpEntireSampleFileMRAMXferBuffer);
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x1F1C | 0x801C6718 | size: 0x10C
 */
bool UpdateAuxEffectA(MusyXEffectType type, void* auxEffectSettings)
{
    bool result;

    switch (type)
    {
    case MUSYX_EFFECT_NONE:
        return true;
    case MUSYX_EFFECT_REVERB:
        result = sndAuxCallbackUpdateSettingsReverbSTD((SND_AUX_REVERBSTD*)auxEffectSettings);
        if (!result)
        {
            nlPrintf("UpdateAuxEffectA: sndAuxCallbackUpdateSettingsReverbSTD failed.\n");
            return false;
        }
        break;
    case MUSYX_EFFECT_REVERB_HI:
        result = sndAuxCallbackUpdateSettingsReverbHI((SND_AUX_REVERBHI*)auxEffectSettings);
        if (!result)
        {
            nlPrintf("UpdateAuxEffectA: sndAuxCallbackUpdateSettingsReverbHI failed.\n");
            return false;
        }
        break;
    case MUSYX_EFFECT_CHORUS:
        result = sndAuxCallbackUpdateSettingsChorus((SND_AUX_CHORUS*)auxEffectSettings);
        if (!result)
        {
            nlPrintf("UpdateAuxEffectA: sndAuxCallbackUpdateSettingsChorus failed.\n");
            return false;
        }
        break;
    case MUSYX_EFFECT_DELAY:
        result = sndAuxCallbackUpdateSettingsDelay((SND_AUX_DELAY*)auxEffectSettings);
        if (!result)
        {
            nlPrintf("UpdateAuxEffectA: sndAuxCallbackUpdateSettingsDelay failed.\n");
            return false;
        }
        break;
    default:
        nlPrintf("UpdateAuxEffectA: Unrecognized effect type.\n");
        return false;
    }

    return true;
}

} // namespace PlatAudio

/**
 * Offset/Address/Size: 0x2028 | 0x801C6824 | size: 0x244
 * TODO: 96.17% match - register allocation mismatch for type/data/arg2/arg3 and callback (r25-r29 swap), causing extra moves in callback setup paths.
 */
bool AddAuxEffect(MusyXEffectType type, void* data, bool arg2, unsigned char arg3)
{
    if ((arg2 == 0) && (PlatAudio::gUsingDolbyProLogic2) && (type != 0))
    {
        return false;
    }

    void* pAuxEffectSettings;
    MusyXEffectType* pAuxEffect;
    void (*callback)(u8 reason, SND_AUX_INFO* info, void* user);

    if (PlatAudio::gUsingDolbyProLogic2)
    {
        if (arg2)
        {
            pAuxEffectSettings = &gDPL2AuxAEffectSettings;
            pAuxEffect = (MusyXEffectType*)&gDPL2AuxAEffect;
        }
        else
        {
            pAuxEffectSettings = &gDPL2AuxBEffectSettings;
            pAuxEffect = (MusyXEffectType*)&gDPL2AuxBEffect;
        }
    }
    else if (arg2)
    {
        pAuxEffectSettings = &gAuxAEffectSettings;
        pAuxEffect = (MusyXEffectType*)&gAuxAEffect;
    }
    else
    {
        pAuxEffectSettings = &gAuxBEffectSettings;
        pAuxEffect = (MusyXEffectType*)&gAuxBEffect;
    }

    switch (type)
    {
    case MUSYX_EFFECT_NONE:
        callback = NULL;
        nlPrintf("InitAuxEffect: MUSYX_EFFECT_NONE passed in, callback return is NULL.\n");
        break;
    case MUSYX_EFFECT_REVERB:
        callback = sndAuxCallbackReverbSTD;
        if (!sndAuxCallbackPrepareReverbSTD((SND_AUX_REVERBSTD*)data))
        {
            nlPrintf("InitAuxEffect: MUSYX_EFFECT_REVERB passed in, callback return is NULL.\n");
            callback = NULL;
        }
        break;
    case MUSYX_EFFECT_REVERB_HI:
        callback = sndAuxCallbackReverbHI;
        if (!sndAuxCallbackPrepareReverbHI((SND_AUX_REVERBHI*)data))
        {
            nlPrintf("InitAuxEffect: MUSYX_EFFECT_REVERB_HI passed in, callback return is NULL.\n");
            callback = NULL;
        }
        break;
    case MUSYX_EFFECT_CHORUS:
        callback = sndAuxCallbackChorus;
        if (!sndAuxCallbackPrepareChorus((SND_AUX_CHORUS*)data))
        {
            nlPrintf("InitAuxEffect: MUSYX_EFFECT_CHORUS passed in, callback return is NULL.\n");
            callback = NULL;
        }
        break;
    case MUSYX_EFFECT_DELAY:
        callback = sndAuxCallbackDelay;
        if (!sndAuxCallbackPrepareDelay((SND_AUX_DELAY*)data))
        {
            nlPrintf("InitAuxEffect: MUSYX_EFFECT_DELAY passed in, callback return is NULL.\n");
            callback = NULL;
        }
        break;
    default:
        nlPrintf("InitAuxEffect: Unaccounted-for case.\n");
        break;
    }

    if (callback == NULL)
    {
        nlPrintf("PlatAudio::SetAuxEffects(), callback is NULL. Should not happen unless values are invalid.\n");
        return false;
    }

    *pAuxEffect = type;
    *(void**)pAuxEffectSettings = data;

    if (arg2)
    {
        sndSetAuxProcessingCallbacks(arg3, callback, *(void**)pAuxEffectSettings, 0xFF, 0, NULL, NULL, 0xFF, 0);
    }
    else
    {
        sndSetAuxProcessingCallbacks(arg3, NULL, NULL, 0xFF, 0, callback, *(void**)pAuxEffectSettings, 0xFF, 0);
    }

    return true;
}

namespace PlatAudio
{

/**
 * Offset/Address/Size: 0x226C | 0x801C6A68 | size: 0x28
 */
bool AddAuxEffectA(MusyXEffectType type, void* data, unsigned char arg)
{
    return AddAuxEffect(type, data, true, arg);
}

/**
 * Offset/Address/Size: 0x2294 | 0x801C6A90 | size: 0x21C
 */
bool ShutdownAuxEffectA()
{
    if (gUsingDolbyProLogic2)
    {
        s32 effect = gDPL2AuxAEffect;
        s32 settings = gDPL2AuxAEffectSettings;

        if (effect == 0)
        {
            nlPrintf("ShutdownAuxEffectA: No effect to shut down\n");
            return true;
        }

        sndSetAuxProcessingCallbacks(0, NULL, NULL, 0xFF, 0, NULL, NULL, 0xFF, 0);

        switch (effect)
        {
        case MUSYX_EFFECT_REVERB:
            if (!sndAuxCallbackShutdownReverbSTD((SND_AUX_REVERBSTD*)settings))
                return false;
            break;
        case MUSYX_EFFECT_REVERB_HI:
            if (!sndAuxCallbackShutdownReverbHI((SND_AUX_REVERBHI*)settings))
            {
                nlPrintf("ShutdownAuxEffectA: Reverb HI shut down\n");
                return false;
            }
            break;
        case MUSYX_EFFECT_CHORUS:
            if (!sndAuxCallbackShutdownChorus((SND_AUX_CHORUS*)settings))
                return false;
            break;
        case MUSYX_EFFECT_DELAY:
            if (!sndAuxCallbackShutdownDelay((SND_AUX_DELAY*)settings))
                return false;
            break;
        }

        return true;
    }
    else
    {
        s32 effect = gAuxAEffect;
        s32 settings = gAuxAEffectSettings;

        if (effect == 0)
        {
            nlPrintf("ShutdownAuxEffectA: No effect to shut down\n");
            return true;
        }

        sndSetAuxProcessingCallbacks(0, NULL, NULL, 0xFF, 0, NULL, NULL, 0xFF, 0);

        switch (effect)
        {
        case MUSYX_EFFECT_REVERB:
            if (!sndAuxCallbackShutdownReverbSTD((SND_AUX_REVERBSTD*)settings))
                return false;
            break;
        case MUSYX_EFFECT_REVERB_HI:
            if (!sndAuxCallbackShutdownReverbHI((SND_AUX_REVERBHI*)settings))
            {
                nlPrintf("ShutdownAuxEffectA: Reverb HI shut down\n");
                return false;
            }
            break;
        case MUSYX_EFFECT_CHORUS:
            if (!sndAuxCallbackShutdownChorus((SND_AUX_CHORUS*)settings))
                return false;
            break;
        case MUSYX_EFFECT_DELAY:
            if (!sndAuxCallbackShutdownDelay((SND_AUX_DELAY*)settings))
                return false;
            break;
        }

        return true;
    }
}

/**
 * Offset/Address/Size: 0x24B0 | 0x801C6CAC | size: 0x24
 */
bool DeactivateDPL2()
{
    if (gUsingDolbyProLogic2 == false)
    {
        return true;
    }
    gUsingDolbyProLogic2 = false;
    return true;
}

/**
 * Offset/Address/Size: 0x24D4 | 0x801C6CD0 | size: 0x24
 */
bool ActivateDPL2()
{
    if (gUsingDolbyProLogic2)
    {
        return true;
    }
    gUsingDolbyProLogic2 = true;
    return true;
}

/**
 * Offset/Address/Size: 0x24F8 | 0x801C6CF4 | size: 0x60
 */
void SetOutputMode(MusyXOutputType outputType)
{
    switch (outputType)
    {
    case MusyXOutputType_MONO:
        sndOutputMode(SND_OUTPUTMODE_MONO);
        return;
    case MusyXOutputType_STEREO:
        sndOutputMode(SND_OUTPUTMODE_STEREO);
        return;
    case MusyXOutputType_SURROUND:
        sndOutputMode(SND_OUTPUTMODE_SURROUND);
        return;
    }
}

} // namespace PlatAudio

/**
 * Offset/Address/Size: 0x1C44 | 0x801C6440 | size: 0x54
 */
void ARAMTransferHelperLoadEntireFile::LoadEntireFileCallback(nlFile* pFile, void* pBuffer, unsigned int size, unsigned long halfIndex)
{
    unsigned int fileSize;
    if (halfIndex == 0)
    {
        gpEntireSampleFileBufferFirstHalf = (void*)((char*)pBuffer - size);
    }
    else
    {
        gpEntireSampleFileBufferSecondHalf = (void*)((char*)pBuffer - size);
        ARAMTransferHelperLoadEntireFile::m_uFileSize = nlFileSize(pFile, &fileSize);
        nlClose(ARAMTransferHelperLoadEntireFile::s_pFile);
        ARAMTransferHelperLoadEntireFile::s_pFile = NULL;
    }
}

/**
 * Offset/Address/Size: 0x1C98 | 0x801C6494 | size: 0x13C
 */
void* ARAMTransferHelperLoadEntireFile::sndPushGroupCallback(unsigned long uOffset, unsigned long uSize)
{
    unsigned long uRemSize = uSize;
    unsigned char* pARAMBlock = ARAMTransferHelperLoadEntireFile::m_pARAMHelper->m_pARAMXferBlockBaseAddress;

    while (uRemSize != 0)
    {
        unsigned long uCopySize = uRemSize < 0x20000 ? uRemSize : (unsigned long)0x20000;

        if (uOffset > gEntireSampleFileFirstHalfAllocSize + gEntireSampleFileSecondHalfAllocSize)
        {
            nlRead(ARAMTransferHelperLoadEntireFile::s_pFile, pARAMBlock, uCopySize);
        }
        else
        {
            unsigned long totalBufSize = gEntireSampleFileFirstHalfAllocSize + gEntireSampleFileSecondHalfAllocSize;
            if (uOffset + uCopySize > totalBufSize)
            {
                unsigned long firstCopySize = totalBufSize - uOffset;
                unsigned char* pSrc = (unsigned char*)gpEntireSampleFileBufferSecondHalf + (uOffset - gEntireSampleFileFirstHalfAllocSize);
                memcpy(pARAMBlock, pSrc, firstCopySize);
                nlRead(ARAMTransferHelperLoadEntireFile::s_pFile, pARAMBlock + firstCopySize, uCopySize - firstCopySize);
            }
        }

        {
            unsigned long firstHalfSize = gEntireSampleFileFirstHalfAllocSize;
            if (uOffset > firstHalfSize)
            {
                unsigned char* pSrc2 = (unsigned char*)gpEntireSampleFileBufferSecondHalf + (uOffset - firstHalfSize);
                memcpy(pARAMBlock, pSrc2, uCopySize);
            }
            else if (uOffset + uCopySize > firstHalfSize)
            {
                unsigned long firstCopySize2 = firstHalfSize - uOffset;
                memcpy(pARAMBlock, (unsigned char*)gpEntireSampleFileBufferFirstHalf + uOffset, firstCopySize2);
                memcpy(pARAMBlock + firstCopySize2, gpEntireSampleFileBufferSecondHalf, uCopySize - firstCopySize2);
            }
            else
            {
                memcpy(pARAMBlock, (unsigned char*)gpEntireSampleFileBufferFirstHalf + uOffset, uCopySize);
            }
        }

        uRemSize -= uCopySize;
        uOffset += uCopySize;
        pARAMBlock += uCopySize;
    }

    return ARAMTransferHelperLoadEntireFile::m_pARAMHelper->m_pARAMXferBlockBaseAddress;
}

/**
 * Offset/Address/Size: 0x1DD4 | 0x801C65D0 | size: 0x148
 */
void* ARAMTransferHelper::sndPushGroupCallback(unsigned long arg0, unsigned long arg1)
{
    unsigned long uSize = arg1;
    unsigned char* pARAMBlock = ARAMTransferHelper::m_pARAMHelper->m_pARAMXferBlockBaseAddress;
    unsigned long uOffset = arg0;

    while (uSize != 0)
    {
        if (uOffset >= ARAMTransferHelper::m_pARAMHelper->m_uCachedDataOffset && uOffset < ARAMTransferHelper::m_pARAMHelper->m_uCachedDataOffset + 0x20000)
        {
            if (!ARAMTransferHelper::m_bFileOpened)
            {
                ARAMTransferHelper::m_pFile = nlOpen(ARAMTransferHelper::m_szFileName);
                ARAMTransferHelper::m_bFileOpened = 1;
            }

            unsigned long uOffsetInBlock = uOffset - ARAMTransferHelper::m_pARAMHelper->m_uCachedDataOffset;
            unsigned long uRemainingInCache = 0x20000 - uOffsetInBlock;
            unsigned long uCopySize = uSize;
            if (uRemainingInCache <= uSize)
                uCopySize = uRemainingInCache;

            memcpy(pARAMBlock, ARAMTransferHelper::m_pARAMHelper->m_pDiskCacheBaseAddress + uOffsetInBlock, uCopySize);
            uSize -= uCopySize;
            uOffset += uCopySize;
            pARAMBlock += uCopySize;
        }
        else
        {
            if (!ARAMTransferHelper::m_bFileOpened)
            {
                ARAMTransferHelper::m_pFile = nlOpen(ARAMTransferHelper::m_szFileName);
                ARAMTransferHelper::m_bFileOpened = 1;
            }

            unsigned long uSeekPosition = uOffset & ~0x1FFFF;
            nlSeek(ARAMTransferHelper::m_pFile, uSeekPosition, 0);

            nlFile* pFile = ARAMTransferHelper::m_pFile;
            unsigned long uFileRemaining = ARAMTransferHelper::m_pARAMHelper->m_uFileSize - uSeekPosition;
            unsigned char* pDiskCache = ARAMTransferHelper::m_pARAMHelper->m_pDiskCacheBaseAddress;
            unsigned long uReadSize = uFileRemaining < 0x20000 ? uFileRemaining : (unsigned long)0x20000;
            nlRead(pFile, pDiskCache, uReadSize);
            ARAMTransferHelper::m_pARAMHelper->m_uCachedDataOffset = uSeekPosition;
        }
    }

    return ARAMTransferHelper::m_pARAMHelper->m_pARAMXferBlockBaseAddress;
}

/**
 * Offset/Address/Size: 0x2558 | 0x801C6D54 | size: 0x278
 * TODO: 99.37% match - remaining i-diffs are local static/string pool symbol IDs in this partially decompiled TU.
 */
void PrintSoundStackInfo()
{
    static u32 prevAvailPrimaryStackSampleMem = PlatAudio::gPrimaryStackSize - 0x500;
    static u32 prevAvailSecondaryStackSampleMem = 0x2B4000;
    static bool bRunOnce = true;

    for (int i = 0; i < 2; i++)
    {
        u32 available = sndStackGetAvailableSampleMemory(stack_list[i].id);
        nlPrintf("Stack %d available sample memory %d\n", stack_list[i].id, available);

        if (bRunOnce)
        {
            u32 start;
            u32 end;
            if ((unsigned char)sndStackGetARAMAddressRange(stack_list[i].id, &start, &end))
            {
                tDebugPrintManager::Print(DC_SOUND, "Stack %d ARAM range start 0x%x end 0x%x\n", stack_list[i].id, start, end);
            }
            else
            {
                tDebugPrintManager::Print(DC_SOUND, "Stack %d ARAM range not available\n", stack_list[i].id);
            }
        }

        if (stack_list[i].id == 0xFFFFFFFE)
        {
            if (available < prevAvailPrimaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Primary stack dropped by %d\n", prevAvailPrimaryStackSampleMem - available);
                prevAvailPrimaryStackSampleMem = available;
            }
            else if (available > prevAvailPrimaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Primary stack increased by %d\n", available - prevAvailPrimaryStackSampleMem);
                prevAvailPrimaryStackSampleMem = available;
            }
            else if (available == prevAvailPrimaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Primary stack unchanged\n");
            }

            if (available == PlatAudio::gPrimaryStackSize - 0x500)
            {
                prevAvailPrimaryStackSampleMem = PlatAudio::gPrimaryStackSize - 0x500;
                tDebugPrintManager::Print(DC_SOUND, "Primary stack is fully available\n");
            }
        }
        else
        {
            if (available < prevAvailSecondaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Secondary stack dropped by %d\n", prevAvailSecondaryStackSampleMem - available);
                prevAvailSecondaryStackSampleMem = available;
            }
            else if (available > prevAvailSecondaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Secondary stack increased by %d\n", available - prevAvailSecondaryStackSampleMem);
                prevAvailSecondaryStackSampleMem = available;
            }
            else if (available == prevAvailSecondaryStackSampleMem)
            {
                tDebugPrintManager::Print(DC_SOUND, "Secondary stack unchanged\n");
            }

            if (available == 0x2B4000)
            {
                prevAvailSecondaryStackSampleMem = 0x2B4000;
                tDebugPrintManager::Print(DC_SOUND, "Secondary stack is fully available\n");
            }
        }
    }

    bRunOnce = false;
}

/**
 * Offset/Address/Size: 0x27D0 | 0x801C6FCC | size: 0x74
 */
void PrintAvailableARAMMemory()
{
    for (int i = 0; i < 2; i++)
    {
        u32 available = sndStackGetAvailableSampleMemory(stack_list[i].id);
        tDebugPrintManager::Print(DC_MEMORY, "Available ARAM: %d\n", available);
    }
}

/**
 * Offset/Address/Size: 0x2844 | 0x801C7040 | size: 0x20
 */
void musyXFree(void* ptr)
{
    nlFree(ptr);
}

/**
 * Offset/Address/Size: 0x2864 | 0x801C7060 | size: 0x28
 */
void* musyXAlloc(u32 size)
{
    return nlMalloc(size, 0x20, false);
}

// /**
//  * Offset/Address/Size: 0x288C | 0x801C7088 | size: 0x3C
//  */
// void 0x8028D524..0x8028D528 | size: 0x4
// {
// }
