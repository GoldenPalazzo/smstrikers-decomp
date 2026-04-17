#ifndef _PLATAUDIO_H_
#define _PLATAUDIO_H_

#include "types.h"
#include "NL/nlMath.h"
#include "musyx/musyx.h"

// Forward declarations
class PhysicsObject;
class cGameSFX;

// PosUpdateMethod enum - defined here to avoid circular dependency with audio.h
enum PosUpdateMethod
{
    NONE = 0,
    PHYSOBJ = 1,
    VECTORS = 2,
    PTRS_TO_VECTORS = 3,
};

enum MusyXEffectType
{
    MUSYX_EFFECT_NONE = 0,
    MUSYX_EFFECT_REVERB = 1,
    MUSYX_EFFECT_REVERB_HI = 2,
    MUSYX_EFFECT_CHORUS = 3,
    MUSYX_EFFECT_DELAY = 4,
};

enum MusyXOutputType
{
    MusyXOutputType_MONO = 0,
    MusyXOutputType_STEREO,
    MusyXOutputType_SURROUND
};

bool AddAuxEffect(MusyXEffectType type, void*, bool, unsigned char);
void PrintSoundStackInfo();
// void PrintAvailableARAMMemory();
void musyXFree(void*);
void* musyXAlloc(u32);
// void 0x8028D524..0x8028D528 | size: 0x4;

class AudioFileData;
class nlFile;

// class MusyXEffectType;
// class MusyXOutputType;
class SFXStartInfo
{
public:
    /* 0x00 */ unsigned long uSFXID;
    /* 0x04 */ float fVolume;
    /* 0x08 */ float fPan;
    /* 0x0C */ float fVolReverb;
    /* 0x10 */ unsigned char uSurroundPan;
    /* 0x11 */ unsigned char _pad1;
    /* 0x12 */ unsigned short uPitchBend;
    /* 0x14 */ unsigned short uModulation;
    /* 0x16 */ unsigned short uDoppler;
    /* 0x18 */ unsigned char bActivateFilter;
    /* 0x19 */ unsigned char _pad2;
    /* 0x1A */ unsigned short filterFreq;
}; // total size: 0x1C

// Forward declarations for types used in SoundStrToIDNode
class SoundPropAccessor;

struct SoundProperties
{
    /* 0x00 */ const char* typeStr;
    /* 0x04 */ const char* musyxStr;
    /* 0x08 */ float fVolume;
    /* 0x0C */ float fDelay;
    /* 0x10 */ float fVolReverb;
    /* 0x14 */ int volumeGroup;
    /* 0x18 */ int priority;
}; // total size: 0x1C

// struct SFXEmitter__
// {
//     /* 0x00 */ SND_EMITTER* m_sndEmitter; // Possibly a pointer to a sound instance or data block
//     /* 0x04 */ s32 m_unk_0x04;            // Sound ID or index (-1 = unused)
//     /* 0x08 */ u32 m_unk_0x08;
//     /* 0x0C */ f32 m_unk_0x0C;
//     /* 0x10 */ f32 m_unk_0x10;
//     /* 0x14 */ f32 m_unk_0x14;
//     /* 0x18 */ f32 m_unk_0x18;
//     /* 0x1C */ f32 m_unk_0x1C;
//     /* 0x20 */ f32 m_unk_0x20;
//     /* 0x24 */ f32 m_unk_0x24;
//     /* 0x28 */ u32 m_unk_0x28; // Frequently checked/cleared
//     /* 0x2C */ u32 m_unk_0x2C;
//     /* 0x30 */ u32 m_unk_0x30;
//     /* 0x34 */ u32 m_unk_0x34;
//     /* 0x38 */ u32 m_unk_0x38;
//     /* 0x3C */ u32 m_unk_0x3C;
//     /* 0x40 */ u32 m_unk_0x40;
//     /* 0x44 */ u32 m_unk_0x44;
//     /* 0x48 */ u32 m_unk_0x48;
//     /* 0x4C */ u32 m_unk_0x4C;
//     /* 0x50 */ u32 m_unk_0x50;
//     /* 0x54 */ u8 m_padding[0x34];
// };

// struct SFXEmitter
// {
//     // /* 0x00 */ char pad0[0x50];
//     SND_EMITTER m_sndEmitter;
//     /* 0x50 */ s8 unk50;    /* inferred */
//                             // /* 0x51 */ char pad51[3];                       /* maybe part of unk50[4]? */
//     /* 0x54 */ s32 unk54;   /* inferred */
//     /* 0x58 */ f32 unk58;   /* inferred */
//     /* 0x5C */ u8 unk5C;    /* inferred */
//     /* 0x5D */ u8 unk5D;    /* inferred */
//     /* 0x5E */ s8 unk5E;    /* inferred */
//     /* 0x5F */ s8 unk5F;    /* inferred */
//     /* 0x60 */ s32 unk60;   /* inferred */
//     /* 0x64 */ s32 unk64;   /* inferred */
//     /* 0x68 */ f32 unk68;   /* inferred */
//     /* 0x6C */ f32 unk6C;   /* inferred */
//     /* 0x70 */ f32 unk70;   /* inferred */
//     /* 0x74 */ f32 unk74;   /* inferred */
//     /* 0x78 */ f32 unk78;   /* inferred */
//     /* 0x7C */ f32 unk7C;   /* inferred */
//     /* 0x80 */ s32 unk80;   /* inferred */
//     /* 0x84 */ void* unk84; /* inferred */
// };

struct SFXEmitter
{
    /* 0x00 */ SND_EMITTER emitter;
    /* 0x50 */ bool bKeepTrack;
    /* 0x54 */ unsigned long soundType;
    /* 0x58 */ float fTimeStamp;
    /* 0x5C */ bool bIsStopping;
    /* 0x5D */ bool bInUse;
    /* 0x5E */ bool bIsFilterOn;
    /* 0x5F */ bool m_unk_0x5F;
    /* 0x60 */ PhysicsObject* pPhysObj;
    /* 0x64 */ void* pOwner;
    union
    {
        const nlVector3* pvPos; // offset 0x0, size 0x4
        nlVector3 vPos;         // offset 0x0, size 0xC
    } pos;                      // offset 0x68, size 0xC
    union
    {
        const nlVector3* pvDir; // offset 0x0, size 0x4
        nlVector3 vDir;         // offset 0x0, size 0xC
    } dir;                      // offset 0x74, size 0xC
    /* 0x80 */ PosUpdateMethod posUpdateMethod;
    /* 0x84 */ SND_PARAMETER_INFO* pMIDIControllerInfo;

    inline void Init()
    {
        soundType = (unsigned long)-1;
        fTimeStamp = -1.0f;
        bIsStopping = 0;
        bInUse = 0;
        bIsFilterOn = 0;
        m_unk_0x5F = 0;
        pPhysObj = NULL;
        pOwner = NULL;
        pos.pvPos = NULL;
        dir.pvDir = NULL;
    }

}; // total size: 0x88

struct SoundStrToIDNode
{
    /*  0x00 */ unsigned long typeID;
    /*  0x04 */ const char* typeStr;
    /*  0x08 */ const char* musyxStr;
    /*  0x0C */ unsigned long musyxID;
    /*  0x10 */ float fVolume;
    /*  0x14 */ float fDelay;
    /*  0x18 */ float fVolReverb;
    /*  0x1C */ int volGrp;
    /*  0x20 */ int sfxPriority;
    /*  0x24 */ unsigned long uHashVal;
    /*  0x28 */ SoundPropAccessor* pSoundPropAccessor;
    /*  0x2C */ unsigned char bSoundPropTableReloaded;
    /*  0x30 */ const struct SoundProperties* pSoundProp;
    /*  0x34 */ cGameSFX* pOwner;
    /*  0x38 */ unsigned long lastVoiceID;
    /*  0x3C */ SFXEmitter* pLastEmitter;
    /*  0x40 */ bool m_unk_0x40;
}; // total size: 0x44

struct EmitterStartInfo
{

    /* 0x00 */ SFXEmitter* pSFXEmitter;
    /* 0x04 */ unsigned long uSFXID;
    /* 0x08 */ unsigned long groupID;
    /* 0x0C */ nlVector3 position;
    /* 0x18 */ nlVector3 direction;
    /* 0x24 */ float maxDist;
    /* 0x28 */ float comp;
    /* 0x2C */ float minVol;
    /* 0x30 */ float maxVol;
    /* 0x34 */ float fVolReverb;
    /* 0x38 */ unsigned char bContinuous;
    /* 0x39 */ unsigned char bRestartable;
    /* 0x3A */ unsigned char bPausable;
    /* 0x3B */ unsigned char bUseDoppler;
    /* 0x3C */ unsigned char bHardStart;
    /* 0x3D */ unsigned char bActivateFilter;
    /* 0x3E */ unsigned short filterFreq;
    /* 0x40 */ unsigned short pitch;
}; // total size: 0x44

namespace PlatAudio
{

extern bool gUsingDolbyProLogic2;

// public:
u32 GetSndIDError();
bool IsSFXPlaying(unsigned long);
static void InitEmitter(unsigned long);
bool RemoveEmitter(SFXEmitter*);
bool RemoveEmitter(unsigned long);
SFXEmitter* GetSFXEmitter(unsigned long);
SFXEmitter* GetFreeEmitter(unsigned long&);
SND_VOICEID GetEmitterVoiceID(SFXEmitter*);
bool IsEmitterActive(SFXEmitter*);
void Update3DSFXEmitter(SFXEmitter*, const nlVector3&, const nlVector3&, float);
void Add3DSFXEmitter(const EmitterStartInfo&);
void Remove3DSFXListener(SND_LISTENER*);
void Update3DSFXListener(SND_LISTENER*, const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&, float);
void Add3DSFXListener(SND_LISTENER*, const nlVector3&, const nlVector3&, const nlVector3&, const nlVector3&, float, float, float, float,
    bool, float);
bool SetPitchBendOnSFX(SND_VOICEID vid, u16 value);
bool SetFilterFreqOnSFX(SND_VOICEID vid, u16 value);
bool SetMIDIControllerVal14Bit(SND_VOICEID vid, u8 ctrl, u16 value);
void SetVolGroupVolume(u8 volGroup, float volume, u16 time);
bool SetSFXVolumeGroup(u32 fid, u8 vGroup);
bool SetSFXReverbVol(unsigned long, float);
void SetSFXVolume(unsigned long, float);
bool StopSFX(unsigned long);
unsigned long PlaySFX(const SFXStartInfo&);
bool UnloadAllSoundGroupsOnStack(AudioFileData&, unsigned long);
bool UnloadAllSoundGroups(AudioFileData&);
bool UnloadSoundGroup(AudioFileData&, unsigned long);
bool LoadSoundGroup(AudioFileData&, unsigned long, unsigned long, bool);
void SetupSoundBuffers(AudioFileData&, bool);
void StopAllSound();
void Shutdown();
bool Initialize(bool);
void PurgeSampleFileBuffer();
bool IsEntireSampleFileInMem();
unsigned char ReadEntireSampleFileIntoMemSync(const char*);
unsigned char ReadEntireSampleFileIntoMem(const char*);
bool UpdateAuxEffectA(MusyXEffectType, void*);
bool AddAuxEffectA(MusyXEffectType, void*, unsigned char);
bool ShutdownAuxEffectA();
bool DeactivateDPL2();
bool ActivateDPL2();
void SetOutputMode(MusyXOutputType);
} // namespace PlatAudio

class ARAMTransferHelperLoadEntireFile
{
public:
    static void LoadEntireFileCallback(nlFile*, void*, unsigned int, unsigned long);
    static void* sndPushGroupCallback(unsigned long, unsigned long);

    unsigned char* m_pARAMXferBlockBaseAddress; // offset 0x0

    static u32 m_uFileSize;
    static nlFile* s_pFile;
    static ARAMTransferHelperLoadEntireFile* m_pARAMHelper;
};

class ARAMTransferHelper
{
public:
    static void* sndPushGroupCallback(unsigned long, unsigned long);

    unsigned char* m_pARAMXferBlockBaseAddress; // offset 0x0
    unsigned long m_uCachedDataOffset;          // offset 0x4
    unsigned char* m_pDiskCacheBaseAddress;     // offset 0x8
    unsigned long m_uFileSize;                  // offset 0xC

    static ARAMTransferHelper* m_pARAMHelper;
    static unsigned char m_bFileOpened;
    static nlFile* m_pFile;
    static const char* m_szFileName;
};

#endif // _PLATAUDIO_H_
