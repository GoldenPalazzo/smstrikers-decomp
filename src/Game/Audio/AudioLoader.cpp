#include "Game/Audio/AudioLoader.h"
#include "Game/Audio/SebringSoundDefines.h"
#include "Game/Audio/SoundEventScript.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Game.h"
#include "Game/Sys/PlatStream.h"
#include "Game/Sys/debug.h"
#include "dolphin/arq.h"

int nlSNPrintf(char*, unsigned long, const char*, ...);

/**
 * Helper struct for inlining FindGet with bool return to match target assembly.
 * The target uses a bool found flag pattern (li r0,1 / li r0,0 / clrlwi.)
 * which the native AVLTreeBase::FindGet (returning ValueType*) does not produce.
 */
struct SoundDefineMapType
{
    char pad[0x1C];
    AVLTreeEntry<int, SoundStrToIDNode*>* m_Root;

    inline bool FindGet(int key, SoundStrToIDNode*** foundValue) const
    {
        AVLTreeEntry<int, SoundStrToIDNode*>* node = m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;
            if (cmpResult == 0)
            {
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<int, SoundStrToIDNode*>*)node->node.left;
                else
                    node = (AVLTreeEntry<int, SoundStrToIDNode*>*)node->node.right;
            }
        }
        return false;
    }
};

static int gLoadedStadiumGroup;
static int gLoadedHomeCaptainGroup;
static int gLoadedAwayCaptainGroup;
static int gLoadedHomeSidekickGroup;
static int gLoadedAwaySidekickGroup;
static int gLoadedSurfaceGroup;

typedef DLListEntry<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL> FadeDLListEntry;
typedef DLListContainerBase<AudioStreamTrack::TrackManagerBase::FadeManager::STREAM_FADE_CTRL, BasicSlotPool<FadeDLListEntry> > FadeDLListContainer;
typedef DLListEntry<GCAudioStreaming::StereoAudioStream*> StreamDLListEntry;

// /**
//  * Offset/Address/Size: 0x0 | 0x801490F4 | size: 0x60
//  */
//  void nlAVLTreeSlotPool<int, SoundStrToIDNode*, DefaultKeyCompare<int>>::~nlAVLTreeSlotPool()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x38 | 0x801490B0 | size: 0x44
//   */
//  void nlDLRingRemove<DLListEntry<GCAudioStreaming::StereoAudioStream*>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>**, DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x20 | 0x80149098 | size: 0x18
//   */
//  void nlDLRingGetStart<DLListEntry<GCAudioStreaming::StereoAudioStream*>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x0 | 0x80149078 | size: 0x20
//   */
//  void nlDLRingIsEnd<DLListEntry<GCAudioStreaming::StereoAudioStream*>>(DLListEntry<GCAudioStreaming::StereoAudioStream*>*, DLListEntry<GCAudioStreaming::StereoAudioStream*>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x0 | 0x80148C6C | size: 0x40C
//   */
//  void 0x80149078..0x801490F4 | size: 0x7C
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x900 | 0x80148BCC | size: 0xA0
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::AllocateEntry(void*, void*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x8D4 | 0x80148BA0 | size: 0x2C
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::CompareKey(void*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x8A8 | 0x80148B74 | size: 0x2C
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x82C | 0x80148AF8 | size: 0x7C
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::~AVLTreeBase()
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x81C | 0x80148AE8 | size: 0x10
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::DeleteEntry(AVLTreeEntry<int, SoundStrToIDNode*>*)
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x814 | 0x80148AE0 | size: 0x8
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::CastUp(AVLTreeNode*) const
//  {
//  }

//  /**
//   * Offset/Address/Size: 0xBC | 0x80148388 | size: 0x758
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::PostorderTraversal(AVLTreeEntry<int, SoundStrToIDNode*>*, void (AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SoundStrToIDNode*>*))
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x58 | 0x80148324 | size: 0x64
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::DestroyTree(void (AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::*)(AVLTreeEntry<int, SoundStrToIDNode*>*))
//  {
//  }

//  /**
//   * Offset/Address/Size: 0x0 | 0x801482CC | size: 0x58
//   */
//  void AVLTreeBase<int, SoundStrToIDNode*, BasicSlotPool<AVLTreeEntry<int, SoundStrToIDNode*>>, DefaultKeyCompare<int>>::Clear()
//  {
//  }

/**
 * Offset/Address/Size: 0x398 | 0x801481F8 | size: 0xD4
 * TODO: 96.23% match - ble- vs beq- branch for m_BufferCount > 0 check (MWCC
 * emits beq for wrapping-if structure, target has ble; functionally equivalent)
 */
void GCAudioStreaming::AudioStream::WarmReadDone(GCAudioStreaming::AudioStreamBuffer* pBuffer)
{
    if (m_Buffers[m_BufferCount - 1] != pBuffer)
    {
        return;
    }

    m_State = SS_Warm;

    if (!(m_Flags & (1 << SF_Play)))
    {
        return;
    }

    if (pBuffer != m_Buffers[m_BufferCount - 1])
    {
        return;
    }

    m_Flags &= ~(1 << SF_Play);

    volatile u32 i = 0;
    if (m_BufferCount > 0)
    {
        AudioStreamBuffer* buf = m_Buffers[0];
        while (buf != NULL)
        {
            sndStreamActivate(buf->m_StreamId);
            u32 ci = i + 1;
            i = ci;
            if (ci < m_BufferCount)
            {
                buf = m_Buffers[ci];
            }
            else
            {
                buf = NULL;
            }
        }
    }

    m_State = SS_Playing;
}

// /**
//  * Offset/Address/Size: 0x350 | 0x801481B0 | size: 0x48
//  */
// void GCAudioStreaming::AudioStream::~AudioStream()
// {
// }

// /**
//  * Offset/Address/Size: 0x344 | 0x801481A4 | size: 0xC
//  */
// void GCAudioStreaming::AudioStream::Purge()
// {
// }

// /**
//  * Offset/Address/Size: 0x70 | 0x80147ED0 | size: 0x2D4
//  */
// void GCAudioStreaming::AudioStream::Destructor()
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80147E60 | size: 0x70
 */
GCAudioStreaming::StereoAudioStream::~StereoAudioStream()
{
    Destructor();
}

/**
 * Offset/Address/Size: 0x3F48 | 0x80147D14 | size: 0x14C
 * TODO: 96% match, something with SebringSoundDefines ref is still wrong...
 */
void AudioLoader::SetupSoundDefinesAVLTree()
{
    u32 numDefines = GetNumSoundDefines();
    SoundDefinesTable* pDefines = &SebringSoundDefines[0];
    AVLTreeNode** const ppRoot = (AVLTreeNode**)((char*)&gMusyXSoundDefineMap.m_Root);

    for (u32 i = 0; i < numDefines; pDefines++, i++)
    {
        SoundStrToIDNode* p = (SoundStrToIDNode*)nlMalloc(sizeof(SoundStrToIDNode), 8, false);
        SoundStrToIDNode* newNode = p;
        p->typeID = -1;
        p->typeStr = NULL;
        p->musyxStr = NULL;
        p->musyxID = -1;
        p->fVolume = 100.0f;
        p->fDelay = -1.0f;
        p->fVolReverb = 100.0f;
        p->volGrp = -1;
        p->sfxPriority = 0;
        p->uHashVal = 0;
        p->pSoundPropAccessor = NULL;
        p->bSoundPropTableReloaded = 0;
        p->pSoundProp = NULL;
        p->pOwner = NULL;
        p->lastVoiceID = -1;
        p->pLastEmitter = NULL;
        p->m_unk_0x40 = false;

        newNode->musyxID = pDefines->musyxID;
        newNode->musyxStr = pDefines->musyxStr;
        newNode->uHashVal = nlStringLowerHash(newNode->musyxStr);

        unsigned int key = newNode->uHashVal;
        AVLTreeNode* existingNode;

        ((AVLTreeUntemplated*)&gMusyXSoundDefineMap)->AddAVLNode(ppRoot, &key, &newNode, &existingNode, *(unsigned int*)((char*)&gMusyXSoundDefineMap + 0x24));

        if (existingNode == NULL)
        {
            (*(unsigned int*)((char*)&gMusyXSoundDefineMap + 0x24))++;
        }
    }
}

namespace Audio
{
extern const char* gCharSoundTable[];
}

/**
 * Offset/Address/Size: 0x3DF4 | 0x80147BC0 | size: 0x154
 */
void AudioLoader::SetupCharSoundTypesAVLTree()
{
    const char* const* pSoundTable = Audio::gCharSoundTable;
    AVLTreeNode** ppRoot;
    int i;

    ppRoot = (AVLTreeNode**)((char*)&gCharSoundDefineMap + 0x1C);
    i = 0;

    do
    {
        SoundStrToIDNode* p = (SoundStrToIDNode*)nlMalloc(sizeof(SoundStrToIDNode), 8, false);
        SoundStrToIDNode* newNode = p;
        p->typeID = -1;
        p->typeStr = NULL;
        p->musyxStr = NULL;
        p->musyxID = -1;
        p->fVolume = 100.0f;
        p->fDelay = -1.0f;
        p->fVolReverb = 100.0f;
        p->volGrp = -1;
        p->sfxPriority = 0;
        p->uHashVal = 0;
        p->pSoundPropAccessor = NULL;
        p->bSoundPropTableReloaded = 0;
        p->pSoundProp = NULL;
        p->pOwner = NULL;
        p->lastVoiceID = -1;
        p->pLastEmitter = NULL;
        p->m_unk_0x40 = false;

        newNode->typeID = i;
        newNode->typeStr = pSoundTable[i];
        newNode->uHashVal = nlStringLowerHash(newNode->typeStr);

        unsigned int key = newNode->uHashVal;
        AVLTreeNode* existingNode;

        ((AVLTreeUntemplated*)&gCharSoundDefineMap)->AddAVLNode(ppRoot, &key, &newNode, &existingNode, *(unsigned int*)((char*)&gCharSoundDefineMap + 0x24));

        if (existingNode == NULL)
        {
            (*(unsigned int*)((char*)&gCharSoundDefineMap + 0x24))++;
        }

        i++;
    } while (i < 173);
}

/**
 * Offset/Address/Size: 0x3D40 | 0x80147B0C | size: 0xB4
 * TODO: 86% scratch match - prologue lis/addi scheduling differs due to -inline deferred context
 */
void AudioLoader::SetupWorldSoundTypesAVLTree()
{
    for (int i = 0; i < 211; i++)
    {
        SoundStrToIDNode* pNode = &Audio::gWorldSFXInfo[i];
        pNode->typeStr = Audio::gWorldSoundTable[i];
        pNode->uHashVal = nlStringLowerHash(pNode->typeStr);

        unsigned int key = pNode->uHashVal;
        AVLTreeNode* existingNode;

        gWorldSoundDefineMap.AddAVLNode(
            (AVLTreeNode**)&gWorldSoundDefineMap.m_Root,
            &key,
            &pNode,
            &existingNode,
            gWorldSoundDefineMap.m_NumElements);

        if (existingNode == NULL)
        {
            gWorldSoundDefineMap.m_NumElements++;
        }
    }
}

/**
 * Offset/Address/Size: 0x3C60 | 0x80147A2C | size: 0xE0
 */
unsigned long AudioLoader::GetSFXIDFromStr(const char* str, SoundStrToIDNode** ppNode)
{
    if (ppNode != NULL)
    {
        *ppNode = NULL;
    }

    SoundStrToIDNode** foundValue = NULL;
    unsigned long hash = nlStringLowerHash(str);
    bool found = ((SoundDefineMapType*)&AudioLoader::gMusyXSoundDefineMap)->FindGet(hash, &foundValue);

    if (found)
    {
        if (ppNode != NULL)
        {
            *ppNode = *foundValue;
        }
        return (*foundValue)->musyxID;
    }
    return -1;
}

/**
 * Offset/Address/Size: 0x3BAC | 0x80147978 | size: 0xB4
 */
unsigned long AudioLoader::GetCharSFXTypeFromStr(const char* str)
{
    SoundStrToIDNode** ppNode = NULL;
    unsigned long key = nlStringLowerHash(str);
    bool found = ((SoundDefineMapType*)&AudioLoader::gCharSoundDefineMap)->FindGet(key, &ppNode);
    if (found)
        return (*ppNode)->typeID;
    else
        return -1;
}

/**
 * Offset/Address/Size: 0x3AF8 | 0x801478C4 | size: 0xB4
 */
unsigned long AudioLoader::GetWorldSFXTypeFromStr(const char* str)
{
    SoundStrToIDNode** ppNode = NULL;
    unsigned long key = nlStringLowerHash(str);
    bool found = ((SoundDefineMapType*)&AudioLoader::gWorldSoundDefineMap)->FindGet(key, &ppNode);
    if (found)
        return (*ppNode)->typeID;
    else
        return -1;
}

/**
 * Offset/Address/Size: 0x39F4 | 0x801477C0 | size: 0x104
 * TODO: 96.15% match - callee-saved register shift (r29/r30/r31 rotated by 1)
 * is an unavoidable MWCC graph-coloring allocator decision
 */
void AudioLoader::SetupSoundGroups()
{
    SndGroupData* group;
    const char* firstName;

    group = sebringAudioGroups;
    firstName = sebringAudioGroups[0].szGroupName;
    int i = 0;

    while ((u32)nlStrLen(firstName) != 0)
    {
        SoundStrToIDNode** foundValue = NULL;
        int hash = (int)nlStringLowerHash(sebringAudioGroups[i].szGroupName);

        bool found = ((SoundDefineMapType*)&AudioLoader::gMusyXSoundDefineMap)->FindGet(hash, &foundValue);

        s32 musyxID;
        if (found)
            musyxID = (*foundValue)->musyxID;
        else
            musyxID = -1;

        if ((u32)(musyxID + 0x10000) == 0xFFFF)
            break;

        sebringAudioGroups[i].groupID = (u16)musyxID;
        i++;
    }
}

/**
 * Offset/Address/Size: 0x35EC | 0x801473B8 | size: 0x408
 */
void AudioLoader::ActivateDPL2(bool, bool)
{
}

/**
 * Offset/Address/Size: 0x356C | 0x80147338 | size: 0x80
 */
bool AudioLoader::StartLoad(LoadingManager*)
{
    if (gbDisableAudio)
    {
        return true;
    }

    if (gbDisableAudio)
    {
        return true;
    }

    bool isInited;
    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (isInited)
    {
        return true;
    }

    FEAudio::BuildAnimAudioEventLookup();
    g_FEStreamConfig.LoadFromFile("Audio/FEStreams.ssc");
    Audio::Initialize(false);
    return true;
}

/**
 * Offset/Address/Size: 0x34F8 | 0x801472C4 | size: 0x74
 */
bool AudioLoader::Initialize()
{
    if (gbDisableAudio)
    {
        return true;
    }

    bool isInited;
    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (isInited)
    {
        return true;
    }

    FEAudio::BuildAnimAudioEventLookup();
    g_FEStreamConfig.LoadFromFile("Audio/FEStreams.ssc");
    Audio::Initialize(false);
    return true;
}

/**
 * Offset/Address/Size: 0x34C4 | 0x80147290 | size: 0x34
 */
bool AudioLoader::IsInited()
{
    if (gbDisableAudio)
    {
        return false;
    }
    return Audio::IsInited();
}

/**
 * Offset/Address/Size: 0x31D4 | 0x80146FA0 | size: 0x2F0
 * TODO: 97.77% match - `gbStream` guard branch form and one temporary move
 * before hashing `pSoundName` still differ.
 */
void AudioLoader::StartFEStream(const char* pSoundName, bool bLoop, const char* pTrackName)
{
    if (gbDisableAudio)
    {
        return;
    }

    if (gbStream)
    {
        char var_68[64];
        nlSNPrintf(var_68, 64, "%s/%s", pSoundName, "Volume");
        float volume = GetConfigFloat(g_FEStreamConfig, var_68, 0.0f);
        volume /= 100.0f;

        nlSNPrintf(var_68, 64, "%s/%s", pSoundName, "FadeIn");
        unsigned long fadeIn = GetConfigInt(g_FEStreamConfig, var_68, 0);
        unsigned long interruptFadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);

        if (volume > 0.0f)
        {
            AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
            AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash(pTrackName));
            track->PlayStream(nlStringLowerHash(pSoundName), volume, bLoop, fadeIn, interruptFadeOut, "", Audio::MasterVolume::VG_Music);
        }
        else
        {
            AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
            AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash(pTrackName));
            track->Stop(interruptFadeOut);
        }
    }
}

/**
 * Offset/Address/Size: 0x2ED4 | 0x80146CA0 | size: 0x300
 */
void AudioLoader::PlayFETitleMusicWithFade()
{
    if (gbDisableAudio)
    {
        return;
    }

    if (!gbStream)
    {
        return;
    }

    char var_68[64];
    nlSNPrintf(var_68, 64, "%s/%s", "FE_Main_Title_With_Fade_In", "Volume");
    float volume = GetConfigFloat(g_FEStreamConfig, var_68, 0.0f);
    volume /= 100.0f;

    nlSNPrintf(var_68, 64, "%s/%s", "FE_Main_Title_With_Fade_In", "FadeIn");
    unsigned long fadeIn = GetConfigInt(g_FEStreamConfig, var_68, 0);
    unsigned long interruptFadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);

    if (volume > 0.0f)
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->PlayStream(nlStringLowerHash("FE_Main_Title_With_Fade_In"), volume, true, fadeIn, interruptFadeOut, "", Audio::MasterVolume::VG_Music);
    }
    else
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->Stop(interruptFadeOut);
    }
}

/**
 * Offset/Address/Size: 0x2BD4 | 0x801469A0 | size: 0x300
 */
void AudioLoader::PlayFEMenuMusic()
{
    if (gbDisableAudio)
    {
        return;
    }

    if (!gbStream)
    {
        return;
    }

    char var_68[64];
    nlSNPrintf(var_68, 64, "%s/%s", "FEMenuMusic", "Volume");
    float volume = GetConfigFloat(g_FEStreamConfig, var_68, 0.0f);
    volume /= 100.0f;

    nlSNPrintf(var_68, 64, "%s/%s", "FEMenuMusic", "FadeIn");
    unsigned long fadeIn = GetConfigInt(g_FEStreamConfig, var_68, 0);
    unsigned long interruptFadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);

    if (volume > 0.0f)
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->PlayStream(nlStringLowerHash("FEMenuMusic"), volume, true, fadeIn, interruptFadeOut, "", Audio::MasterVolume::VG_Music);
    }
    else
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->Stop(interruptFadeOut);
    }
}

/**
 * Offset/Address/Size: 0x27F0 | 0x801465BC | size: 0x3E4
 */
void AudioLoader::PlayLoadLoopMusic()
{
    if (gbDisableAudio || !gbStream)
    {
        return;
    }

    AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
    AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
    track->Stop(0);

    if (GetConfigBool(Config::Global(), "LoadLoopMusicDisable", false))
    {
        return;
    }

    if (gbDisableAudio)
    {
        return;
    }

    if (!gbStream)
    {
        return;
    }

    char var_68[64];
    nlSNPrintf(var_68, 64, "%s/%s", "FE_Load_Loop", "Volume");
    float volume = GetConfigFloat(g_FEStreamConfig, var_68, 0.0f);
    volume /= 100.0f;

    nlSNPrintf(var_68, 64, "%s/%s", "FE_Load_Loop", "FadeIn");
    unsigned long fadeIn = GetConfigInt(g_FEStreamConfig, var_68, 0);
    unsigned long interruptFadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);

    if (volume > 0.0f)
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->PlayStream(nlStringLowerHash("FE_Load_Loop"), volume, true, fadeIn, interruptFadeOut, "", Audio::MasterVolume::VG_Music);
    }
    else
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("FE"));
        track->Stop(interruptFadeOut);
    }
}

/**
 * Offset/Address/Size: 0x24E8 | 0x801462B4 | size: 0x308
 */
void AudioLoader::PlayPauseMenuMusic()
{
    if (gbDisableAudio)
    {
        return;
    }

    if (!gbStream)
    {
        return;
    }

    char var_68[64];
    nlSNPrintf(var_68, 64, "%s/%s", "PauseMenuMusic", "Volume");
    float volume = GetConfigFloat(g_FEStreamConfig, var_68, 0.0f);
    volume /= 100.0f;

    nlSNPrintf(var_68, 64, "%s/%s", "PauseMenuMusic", "FadeIn");
    unsigned long fadeIn = GetConfigInt(g_FEStreamConfig, var_68, 0);
    unsigned long interruptFadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);

    if (volume > 0.0f)
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("Announcer"));
        track->PlayStream(nlStringLowerHash("PauseMenuMusic"), volume, true, fadeIn, interruptFadeOut, "", Audio::MasterVolume::VG_Music);
    }
    else
    {
        AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
        AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("Announcer"));
        track->Stop(interruptFadeOut);
    }
}

/**
 * Offset/Address/Size: 0x23D4 | 0x801461A0 | size: 0x114
 */
void AudioLoader::StopPauseMenuMusic()
{
    if (gbDisableAudio || !gbStream)
    {
        return;
    }

    AudioStreamTrack::TrackManagerBase* pTrackMgr = g_pTrackManager;
    AudioStreamTrack::StreamTrack* track = pTrackMgr->GetTrack(nlStringLowerHash("Announcer"));
    s32 fadeOut = GetConfigInt(g_FEStreamConfig, "InterruptFadeOut", 0);
    track->Stop(fadeOut);
}

/**
 * Offset/Address/Size: 0x23B4 | 0x80146180 | size: 0x20
 */
void AudioLoader::StopStreaming()
{
    Audio::StopStreaming();
}

/**
 * Offset/Address/Size: 0x237C | 0x80146148 | size: 0x38
 */
void AudioLoader::SetupSoundBuffers()
{
    if (gbDisableAudio)
    {
        return;
    }

    PlatAudio::SetupSoundBuffers(sebringAudioFileData, true);
}

/**
 * Offset/Address/Size: 0x20E4 | 0x80145EB0 | size: 0x298
 * TODO: 99.94% match - MWCC shrink-wraps the stack frame and emits local
 * FE/FE2 labels (@856/@857) instead of target @3134/@3135.
 */
void AudioLoader::LoadFE(bool bLoadSampleFile)
{
    if (gbDisableAudio)
    {
        return;
    }

    if (bLoadSampleFile)
    {
        gbAsyncLoadEntireSampleFileIntoMemRequestMade = true;
        PlatAudio::ReadEntireSampleFileIntoMemSync(sebringAudioFileData.szSampleFile);

        if (!gbDisableAudio)
        {
            bool bAlreadyLoaded = false;
            if (sebringAudioGroups[3].uLoadOrder > -1 && sebringAudioGroups[3].stackEnum > -1)
            {
                bAlreadyLoaded = true;
            }

            if (!bAlreadyLoaded)
            {
                if (!gbDisableAudio)
                {
                    bool isInited;
                    if (gbDisableAudio)
                    {
                        isInited = false;
                    }
                    else
                    {
                        isInited = Audio::IsInited();
                    }

                    if (isInited)
                    {
                        PlatAudio::LoadSoundGroup(sebringAudioFileData, 3, 1, false);
                    }
                }
            }
        }

        if (!gbDisableAudio)
        {
            bool bAlreadyLoaded = false;
            if (sebringAudioGroups[4].uLoadOrder > -1 && sebringAudioGroups[4].stackEnum > -1)
            {
                bAlreadyLoaded = true;
            }

            if (!bAlreadyLoaded)
            {
                if (!gbDisableAudio)
                {
                    bool isInited;
                    if (gbDisableAudio)
                    {
                        isInited = false;
                    }
                    else
                    {
                        isInited = Audio::IsInited();
                    }

                    if (isInited)
                    {
                        PlatAudio::LoadSoundGroup(sebringAudioFileData, 4, 1, false);
                    }
                }
            }
        }

        bool bAlreadyLoaded = false;
        if (sebringAudioGroups[0].uLoadOrder > -1 && sebringAudioGroups[0].stackEnum > -1)
        {
            bAlreadyLoaded = true;
        }

        if (!bAlreadyLoaded)
        {
            if (!gbDisableAudio)
            {
                bool isInited;
                if (gbDisableAudio)
                {
                    isInited = false;
                }
                else
                {
                    isInited = Audio::IsInited();
                }

                if (isInited)
                {
                    PlatAudio::LoadSoundGroup(sebringAudioFileData, 0, 1, false);
                }
            }
        }

        bAlreadyLoaded = false;
        if (sebringAudioGroups[1].uLoadOrder > -1 && sebringAudioGroups[1].stackEnum > -1)
        {
            bAlreadyLoaded = true;
        }

        if (!bAlreadyLoaded)
        {
            if (!gbDisableAudio)
            {
                bool isInited;
                if (gbDisableAudio)
                {
                    isInited = false;
                }
                else
                {
                    isInited = Audio::IsInited();
                }

                if (isInited)
                {
                    PlatAudio::LoadSoundGroup(sebringAudioFileData, 1, 1, false);
                }
            }
        }

        ARQSetChunkSize(0x1000);
        PlatAudio::PurgeSampleFileBuffer();
        gbAsyncLoadEntireSampleFileIntoMemRequestMade = false;
    }

    Audio::LoadWorldSFX();
    CrowdMood::SetCrowdVolume(0, 0);
    g_pTrackManager->StopAllTracks(0);
    PlatAudio::ConfigureStreamBuffers(4);
    g_pTrackManager->DestroyAllTracks();
    g_pTrackManager->CreateTrack("FE", Audio::MasterVolume::VG_Music);
    g_pTrackManager->CreateTrack("FE2", Audio::MasterVolume::VG_Music);
}

/**
 * Offset/Address/Size: 0x205C | 0x80145E28 | size: 0x88
 */
void AudioLoader::UnloadFE()
{
    if (gbDisableAudio)
    {
        return;
    }

    Audio::ClearFadeData();
    Audio::gWorldSFX.ShutdownPlaySet();
    Audio::gPowerupSFX.ShutdownPlaySet();
    Audio::gStadGenSFX.ShutdownPlaySet();
    Audio::gCrowdSFX.ShutdownPlaySet();
    SlotPoolBase::BaseFreeBlocks(&SFXPlaySet::m_TrackedSFXSlotPool, 0x24);
    PlatAudio::UnloadAllSoundGroupsOnStack(sebringAudioFileData, 1);
    Audio::gWorldSFX.StopPlayingAllTrackedSFX();
}

/**
 * Offset/Address/Size: 0x1F54 | 0x80145D20 | size: 0x108
 */
void AudioLoader::LoadFEAudioData(bool bAsync)
{
    bool bAlreadyLoaded = false;
    if (sebringAudioGroups[0].uLoadOrder > -1 && sebringAudioGroups[0].stackEnum > -1)
    {
        bAlreadyLoaded = true;
    }

    if (!bAlreadyLoaded)
    {
        if (!gbDisableAudio)
        {
            bool isInited;
            if (gbDisableAudio)
            {
                isInited = false;
            }
            else
            {
                isInited = Audio::IsInited();
            }

            if (isInited)
            {
                PlatAudio::LoadSoundGroup(sebringAudioFileData, 0, 1, bAsync);
            }
        }
    }

    bAlreadyLoaded = false;
    if (sebringAudioGroups[1].uLoadOrder > -1 && sebringAudioGroups[1].stackEnum > -1)
    {
        bAlreadyLoaded = true;
    }

    if (!bAlreadyLoaded)
    {
        if (!gbDisableAudio)
        {
            bool isInited;
            if (gbDisableAudio)
            {
                isInited = false;
            }
            else
            {
                isInited = Audio::IsInited();
            }

            if (isInited)
            {
                PlatAudio::LoadSoundGroup(sebringAudioFileData, 1, 1, bAsync);
            }
        }
    }

    ARQSetChunkSize(0x1000);
}

/**
 * Offset/Address/Size: 0x1EB8 | 0x80145C84 | size: 0x9C
 */
void AudioLoader::LoadNintendoDialogueGroup(bool bAsync)
{
    if (gbDisableAudio)
        return;

    bool bAlreadyLoaded = false;
    if (sebringAudioGroups[3].uLoadOrder > -1 && sebringAudioGroups[3].stackEnum > -1)
    {
        bAlreadyLoaded = true;
    }

    if (bAlreadyLoaded)
        return;

    if (gbDisableAudio)
        return;

    bool isInited;
    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (!isInited)
        return;

    PlatAudio::LoadSoundGroup(sebringAudioFileData, 3, 1, bAsync);
}

/**
 * Offset/Address/Size: 0x1E1C | 0x80145BE8 | size: 0x9C
 * TODO: 99.87% match - r3/r4 register swap (this vs bAsync) at offset 0x10
 */
void AudioLoader::LoadNLGDialogueGroup(bool bAsync)
{
    if (gbDisableAudio)
        return;

    bool bAlreadyLoaded = false;
    if (sebringAudioGroups[4].uLoadOrder > -1 && sebringAudioGroups[4].stackEnum > -1)
    {
        bAlreadyLoaded = true;
    }

    if (bAlreadyLoaded)
        return;

    if (gbDisableAudio)
        return;

    if (!AudioLoader::IsInited())
        return;

    PlatAudio::LoadSoundGroup(sebringAudioFileData, 4, 1, bAsync);
}

/**
 * Offset/Address/Size: 0x1D8C | 0x80145B58 | size: 0x90
 */
void AudioLoader::LoadFEButtonSoundGroup()
{
    if (gbDisableAudio)
    {
        return;
    }

    bool alreadyLoaded = (sebringAudioGroups[2].uLoadOrder > -1) && (sebringAudioGroups[2].stackEnum > -1);

    if (alreadyLoaded)
    {
        return;
    }

    if (gbDisableAudio)
    {
        return;
    }

    bool isInited;
    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (!isInited)
    {
        return;
    }

    PlatAudio::LoadSoundGroup(sebringAudioFileData, 2, 0, true);
}

/**
 * Offset/Address/Size: 0x1630 | 0x801453FC | size: 0x75C
 */
void AudioLoader::LoadPermanentSoundGroups(bool)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x14F4 | 0x801452C0 | size: 0x13C
 */
void AudioLoader::LoadInGame()
{
    if (gbDisableAudio)
    {
        return;
    }

    bool bLoaded = false;
    if (sebringAudioGroups[1].uLoadOrder > -1 && sebringAudioGroups[1].stackEnum > -1)
    {
        bLoaded = true;
    }

    if (bLoaded)
    {
        if (!PlatAudio::UnloadSoundGroup(sebringAudioFileData, 1))
        {
            tDebugPrintManager::Print(DC_SOUND, "LoadInGame: Failed to unload sound group 1\n");
        }
    }

    bLoaded = false;
    if (sebringAudioGroups[0].uLoadOrder > -1 && sebringAudioGroups[0].stackEnum > -1)
    {
        bLoaded = true;
    }

    if (bLoaded)
    {
        if (!PlatAudio::UnloadSoundGroup(sebringAudioFileData, 0))
        {
            tDebugPrintManager::Print(DC_SOUND, "LoadInGame: Failed to unload sound group 0\n");
        }
    }

    LoadPermanentSoundGroups(false);
    Audio::LoadInGameSFX();
    Audio::gWorldSFX.Init();
    Audio::gPowerupSFX.Init();
    Audio::gStadGenSFX.Init();

    if (!gbDisableCrowd)
    {
        Audio::gCrowdSFX.Init();
    }

    LoadInGameAudioData();
    Audio::LoadWorldSFX();
    ARQSetChunkSize(0x1000);
    SoundEventScript::CreateInstance();
}

/**
 * Offset/Address/Size: 0x130C | 0x801450D8 | size: 0x1E8
 */
void AudioLoader::UnloadInGame()
{
    if (gbDisableAudio)
    {
        return;
    }

    Audio::ClearFadeData();
    CrowdMood::SetCrowdVolume(0, 0);
    Audio::UnloadWorldSFX();
    Audio::UnloadInGameSFX();
    g_pTrackManager->StopAllTracks(0);
    CrowdMood::Purge(false);
    Audio::DestroyPriorityStreams();
    g_pTrackManager->DestroyAllTracks();

    AudioStreamTrack::TrackManagerBase* pTM = g_pTrackManager;
    pTM->StopAllTracks(0);

    // Declare loop variables in order for register allocation
    GCAudioStreaming::StereoAudioStream* pStream;
    StreamDLListEntry* pFree;
    StreamDLListEntry* pRemove;
    StreamDLListEntry* pHead;
    StreamDLListEntry* pCur;
    StreamDLListEntry** ppHead;

    // nlWalkDLRing on FadeManager
    nlWalkDLRing(
        *(FadeDLListEntry**)(pTM->_pad_0x18 + 0x18),
        (FadeDLListContainer*)pTM->_pad_0x18,
        &FadeDLListContainer::DeleteEntry);
    *(FadeDLListEntry**)(pTM->_pad_0x18 + 0x18) = NULL;
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)pTM->_pad_0x18, sizeof(FadeDLListEntry));

    // StereoAudioStream DLRing loop - use _pad_0x50 + 0x18 for offset 0x68
    StreamDLListEntry* tmp = nlDLRingGetStart(*(StreamDLListEntry**)(pTM->_pad_0x50 + 0x18));
    pHead = *(StreamDLListEntry**)(pTM->_pad_0x50 + 0x18);
    ppHead = (StreamDLListEntry**)(pTM->_pad_0x50 + 0x18);
    pCur = tmp;

    while (pCur != NULL)
    {
        pStream = pCur->m_data;
        pStream->~StereoAudioStream();

        // Link freed stream memory into StreamPool free list (offset 0x44)
        *(unsigned long*)pStream = *(unsigned long*)(pTM->_pad_0x38 + 0x0C);
        *(unsigned long*)(pTM->_pad_0x38 + 0x0C) = (unsigned long)pStream;

        pRemove = pCur;
        pFree = pCur;

        if (nlDLRingIsEnd(pHead, pCur) || pCur == NULL)
        {
            pCur = NULL;
        }
        else
        {
            pCur = pCur->m_next;
        }

        nlDLRingRemove(ppHead, pRemove);

        // Link removed entry into StreamDeleteList free list (offset 0x5C)
        *(unsigned long*)pFree = *(unsigned long*)(pTM->_pad_0x50 + 0x0C);
        *(unsigned long*)(pTM->_pad_0x50 + 0x0C) = (unsigned long)pFree;
    }

    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)pTM->_pad_0x38, 0x40);
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)pTM->_pad_0x50, 0x0C);

    PlatAudio::UnloadAllSoundGroupsOnStack(sebringAudioFileData, 1);

    gLoadedAwaySidekickGroup = -1;
    gLoadedHomeSidekickGroup = -1;
    gLoadedAwayCaptainGroup = -1;
    gLoadedHomeCaptainGroup = -1;
    gLoadedSurfaceGroup = -1;
    gLoadedStadiumGroup = -1;

    if (!gbDisableReverb)
    {
        if (!Audio::ShutdownReverb())
        {
            nlPrintf("AudioLoader::UnloadInGame - ShutdownReverb failed\n");
        }
    }

    SoundEventScript::DestroyInstance();
    FEAudio::ResetRandomVoiceToggleSFX();
    Audio::Silence();
}

/**
 * Offset/Address/Size: 0x1170 | 0x80144F3C | size: 0x19C
 * TODO: 99.90% match - symbol name diffs only (static const kUpVec generates named label vs anonymous @NNNN in target). Actual instructions identical.
 */
void AudioLoader::SetupPostPhysicsCameraLoad()
{
    if (AudioLoader::gbDisableAudio)
    {
        return;
    }

    if (AudioLoader::gbDisableAudio || Audio::IsListenerActive())
    {
        goto setupCharStadiumSoundTable;
    }

    nlVector3 up;
    nlVector3 heading;
    nlVector3 cameraPos;
    nlVector3 targetPos;
    nlVector3 vel = { { 0.0f, 0.0f, 0.0f } };

    cBaseCamera* camera = nlDLRingGetStart(cCameraManager::m_cameraStack);
    targetPos = camera->GetTargetPosition();

    camera = nlDLRingGetStart(cCameraManager::m_cameraStack);
    cameraPos = camera->GetCameraPosition();

    float dx, dy, dz;
    dy = targetPos.f.y - cameraPos.f.y;
    dx = targetPos.f.x - cameraPos.f.x;
    dz = targetPos.f.z - cameraPos.f.z;

    float recipLen = nlRecipSqrt(dx * dx + dy * dy + dz * dz, true);

    heading.f.x = recipLen * dx;
    heading.f.y = recipLen * dy;
    heading.f.z = recipLen * dz;

    static const nlVector3 kUpVec = { { 0.0f, 0.0f, 1.0f } };
    up = kUpVec;

    GameTweaks* tweaks = g_pGame->m_pGameTweaks;

    PlatAudio::Add3DSFXListener(
        &Audio::gListener,
        cameraPos,
        up,
        heading,
        vel,
        tweaks->unk1F0,
        tweaks->unk1F4,
        0.0f,
        tweaks->unk1FC,
        false,
        tweaks->unk1F8);

    Audio::SetListenerActive(true);

setupCharStadiumSoundTable:
    ((void (*)())AudioLoader::SetupCharStadiumSoundTable)();
}

extern SoundPropAccessor* surfaceSoundPropTables[14][5];
extern SoundPropAccessor* gpCRITTERWOODSoundPropAccessor;
extern SoundPropAccessor* gpBOWSERWOODSoundPropAccessor;
extern SoundPropAccessor* gpCRITTERCONCRETESoundPropAccessor;
extern SoundPropAccessor* gpBOWSERCONCRETESoundPropAccessor;
extern SoundPropAccessor* gpCRITTERGRASSSoundPropAccessor;
extern SoundPropAccessor* gpBOWSERGRASSSoundPropAccessor;
extern SoundPropAccessor* gpCRITTERMETALSoundPropAccessor;
extern SoundPropAccessor* gpBOWSERMETALSoundPropAccessor;
extern SoundPropAccessor* gpCRITTERRUBBERSoundPropAccessor;
extern SoundPropAccessor* gpBOWSERRUBBERSoundPropAccessor;
extern SoundPropAccessor* gpCRITTERROBOTSoundPropAccessor;
extern SoundPropAccessor* gpSTADWOODSoundPropAccessor;
extern SoundPropAccessor* gpSTADKONGASoundPropAccessor;
extern SoundPropAccessor* gpSTADCONCRETESoundPropAccessor;
extern SoundPropAccessor* gpSTADPIPESoundPropAccessor;
extern SoundPropAccessor* gpSTADGRASSSoundPropAccessor;
extern SoundPropAccessor* gpSTADPALACESoundPropAccessor;
extern SoundPropAccessor* gpSTADMETALSoundPropAccessor;
extern SoundPropAccessor* gpSTADUNDERSoundPropAccessor;
extern SoundPropAccessor* gpSTADCRATERSoundPropAccessor;
extern SoundPropAccessor* gpSTADRUBBERSoundPropAccessor;
extern SoundPropAccessor* gpSTADBOWSERSoundPropAccessor;
extern SoundPropAccessor* gpSTADBATTLESoundPropAccessor;
extern eCharacterClass ConvertToCharacterClass(eTeamID);
extern eCharacterClass ConvertToCharacterClass(eSidekickID);

/**
 * Offset/Address/Size: 0xF90 | 0x80144D5C | size: 0x1E0
 */
#pragma push
#pragma dont_inline on
SoundPropAccessor* GetSoundPropTableFromPlayerStadium(eStadiumID stadiumId, eCharacterClass charClass)
{
    switch (stadiumId)
    {
    case STAD_MARIO_STADIUM:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERWOODSoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERWOODSoundPropAccessor;
        return surfaceSoundPropTables[charClass][4];
    case STAD_PEACH_TOAD_STADIUM:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERCONCRETESoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERCONCRETESoundPropAccessor;
        return surfaceSoundPropTables[charClass][2];
    case STAD_DK_DAISY:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERGRASSSoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERGRASSSoundPropAccessor;
        return surfaceSoundPropTables[charClass][0];
    case STAD_WARIO_STADIUM:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERMETALSoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERMETALSoundPropAccessor;
        return surfaceSoundPropTables[charClass][1];
    case STAD_YOSHI_STADIUM:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERGRASSSoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERGRASSSoundPropAccessor;
        return surfaceSoundPropTables[charClass][0];
    case STAD_SUPER_STADIUM:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERRUBBERSoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERRUBBERSoundPropAccessor;
        return surfaceSoundPropTables[charClass][3];
    case STAD_FORBIDDEN_DOME:
        if (charClass >= NUM_FIELDER_CLASSES && charClass < NUM_CHARACTER_CLASSES)
            return gpCRITTERCONCRETESoundPropAccessor;
        if (charClass == NUM_CHARACTER_CLASSES)
            return gpBOWSERCONCRETESoundPropAccessor;
        return surfaceSoundPropTables[charClass][2];
    default:
        return NULL;
    }
}
#pragma pop

/**
 * Offset/Address/Size: 0xC88 | 0x80144A54 | size: 0x308
 */
void AudioLoader::SetupCharStadiumSoundTable()
{
    if (gbDisableAudio)
    {
        return;
    }

    eStadiumID stadium;
    stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();

    eCharacterClass homeCaptainClass = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(0));
    eCharacterClass awayCaptainClass = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetTeam(1));
    eCharacterClass homeSidekickClass = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(0));
    eCharacterClass awaySidekickClass = ConvertToCharacterClass(nlSingleton<GameInfoManager>::s_pInstance->GetSidekick(1));

    SoundPropAccessor* homeCaptainPropTable = GetSoundPropTableFromPlayerStadium(stadium, homeCaptainClass);
    SoundPropAccessor* awayCaptainPropTable = GetSoundPropTableFromPlayerStadium(stadium, awayCaptainClass);
    SoundPropAccessor* homeSidekickPropTable = GetSoundPropTableFromPlayerStadium(stadium, homeSidekickClass);
    SoundPropAccessor* awaySidekickPropTable = GetSoundPropTableFromPlayerStadium(stadium, awaySidekickClass);

    for (int team = 0; team < 2; team++)
    {
        cTeam* pTeam = g_pTeams[team];
        for (int player = 0; player < 5; player++)
        {
            cPlayer* pPlayer = pTeam->GetPlayer(player);
            if (pPlayer->m_eClassType == GOALIE)
            {
                SoundPropAccessor* goaliePropTable = GetSoundPropTableFromPlayerStadium(stadium, g_pTeams[team]->GetGoalie()->m_eCharacterClass);
                g_pTeams[team]->GetGoalie()->m_pCharacterSFX->SetSFX(goaliePropTable);
                if (((cCharacter*)g_pTeams[team]->GetCaptain())->m_eCharacterClass == MYSTERY)
                {
                    g_pTeams[team]->GetGoalie()->m_pCharacterSFX->SetSFX(gpCRITTERROBOTSoundPropAccessor);
                }
            }
            else if (pPlayer->IsCaptain())
            {
                if (team == 0)
                {
                    pPlayer->m_pCharacterSFX->SetSFX(homeCaptainPropTable);
                }
                else
                {
                    pPlayer->m_pCharacterSFX->SetSFX(awayCaptainPropTable);
                }
            }
            else
            {
                if (team == 0)
                {
                    pPlayer->m_pCharacterSFX->SetSFX(homeSidekickPropTable);
                }
                else
                {
                    pPlayer->m_pCharacterSFX->SetSFX(awaySidekickPropTable);
                }
            }
        }
    }

    if (((cCharacter*)g_pTeams[0]->GetCaptain())->m_eCharacterClass == MYSTERY || ((cCharacter*)g_pTeams[1]->GetCaptain())->m_eCharacterClass == MYSTERY)
    {
        bool bAlreadyLoaded = false;
        if (sebringAudioGroups[13].uLoadOrder > -1 && sebringAudioGroups[13].stackEnum > -1)
        {
            bAlreadyLoaded = true;
        }

        if (!bAlreadyLoaded)
        {
            bool loaded;
            if (gbDisableAudio)
            {
                loaded = true;
            }
            else
            {
                if (!AudioLoader::IsInited())
                {
                    loaded = false;
                }
                else
                {
                    loaded = ((bool (*)(AudioFileData&, unsigned long, unsigned long, bool))PlatAudio::LoadSoundGroup)(sebringAudioFileData, 13, 1, true);
                }
            }

            if (!loaded)
            {
                tDebugPrintManager::Print(DC_SOUND, "Could not load mystery stadium sound group onto secondary sound stack.\n");
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xC34 | 0x80144A00 | size: 0x54
 */
void AudioLoader::SetupBowserStadiumSoundTable(Bowser* bowser)
{
    eStadiumID stadium = nlSingleton<GameInfoManager>::s_pInstance->GetStadium();
    SoundPropAccessor* propTable = GetSoundPropTableFromPlayerStadium(stadium, (eCharacterClass)0x16);
    bowser->m_pCharacterSFX->SetSFX(propTable);
}

/**
 * Offset/Address/Size: 0x8E4 | 0x801446B0 | size: 0x350
 * TODO: 99.72% match - r4/r5 register swap for bAlreadyLoaded in loaded-group checks
 */
unsigned char AudioLoader::LoadStadiumSpecificSoundGroups(eStadiumID stadiumID)
{
    switch (stadiumID)
    {
    case STAD_MARIO_STADIUM:
        gLoadedSurfaceGroup = 18;
        gLoadedStadiumGroup = 10;
        Audio::gStadGenSFX.SetSFX(gpSTADWOODSoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADKONGASoundPropAccessor);
        break;
    case STAD_PEACH_TOAD_STADIUM:
        gLoadedSurfaceGroup = 16;
        gLoadedStadiumGroup = 8;
        Audio::gStadGenSFX.SetSFX(gpSTADCONCRETESoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADPIPESoundPropAccessor);
        break;
    case STAD_DK_DAISY:
        gLoadedSurfaceGroup = 14;
        gLoadedStadiumGroup = 7;
        Audio::gStadGenSFX.SetSFX(gpSTADGRASSSoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADPALACESoundPropAccessor);
        break;
    case STAD_WARIO_STADIUM:
        gLoadedSurfaceGroup = 15;
        gLoadedStadiumGroup = 9;
        Audio::gStadGenSFX.SetSFX(gpSTADMETALSoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADUNDERSoundPropAccessor);
        break;
    case STAD_YOSHI_STADIUM:
        gLoadedSurfaceGroup = 14;
        gLoadedStadiumGroup = 11;
        Audio::gStadGenSFX.SetSFX(gpSTADGRASSSoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADCRATERSoundPropAccessor);
        break;
    case STAD_SUPER_STADIUM:
        gLoadedSurfaceGroup = 17;
        gLoadedStadiumGroup = 12;
        Audio::gStadGenSFX.SetSFX(gpSTADRUBBERSoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADBOWSERSoundPropAccessor);
        break;
    case STAD_FORBIDDEN_DOME:
        gLoadedSurfaceGroup = 16;
        gLoadedStadiumGroup = 9;
        Audio::gStadGenSFX.SetSFX(gpSTADCONCRETESoundPropAccessor);
        Audio::gStadGenSFX.SetSFX(gpSTADBATTLESoundPropAccessor);
        break;
    default:
        tDebugPrintManager::Print(DC_SOUND, "A new stadium needs to be added to AudioLoader::LoadInGameAudioData()\n");
        return false;
    }

    s32 loadedGroup = gLoadedSurfaceGroup;
    bool bAlreadyLoaded;

    if (loadedGroup < 0)
    {
        bAlreadyLoaded = false;
    }
    else
    {
        bAlreadyLoaded = false;
        if (sebringAudioGroups[loadedGroup].uLoadOrder > -1 && sebringAudioGroups[loadedGroup].stackEnum > -1)
        {
            bAlreadyLoaded = true;
        }
    }

    if (bAlreadyLoaded == false)
    {
        bool loaded;
        if (gbDisableAudio)
        {
            loaded = true;
        }
        else
        {
            if (AudioLoader::IsInited() == false)
            {
                loaded = false;
            }
            else
            {
                loaded = PlatAudio::LoadSoundGroup(sebringAudioFileData, loadedGroup, 1, true);
            }
        }

        if (loaded == false)
        {
            tDebugPrintManager::Print(DC_SOUND, "Could not load surface sound group %d onto secondary sound stack.\n", gLoadedSurfaceGroup);
            return false;
        }
    }

    loadedGroup = gLoadedStadiumGroup;
    if (loadedGroup < 0)
    {
        bAlreadyLoaded = false;
    }
    else
    {
        bAlreadyLoaded = false;
        if (sebringAudioGroups[loadedGroup].uLoadOrder > -1 && sebringAudioGroups[loadedGroup].stackEnum > -1)
        {
            bAlreadyLoaded = true;
        }
    }

    if (bAlreadyLoaded == false)
    {
        bool loaded;
        if (gbDisableAudio)
        {
            loaded = true;
        }
        else
        {
            if (AudioLoader::IsInited() == false)
            {
                loaded = false;
            }
            else
            {
                loaded = PlatAudio::LoadSoundGroup(sebringAudioFileData, loadedGroup, 1, true);
            }
        }

        if (loaded == false)
        {
            tDebugPrintManager::Print(DC_SOUND, "Could not load stadium sound group %d onto secondary sound stack.\n", gLoadedStadiumGroup);
            return false;
        }
    }

    return true;
}

/**
 * Offset/Address/Size: 0x1F8 | 0x80143FC4 | size: 0x6EC
 */
void AudioLoader::LoadInGameAudioData()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x1A8 | 0x80143F74 | size: 0x50
 * TODO: 96.5% match - register scheduling difference (r3 vs r4)
 */
void AudioLoader::ReadEntireSampleFileIntoMem(bool sync)
{
    gbAsyncLoadEntireSampleFileIntoMemRequestMade = true;

    if (sync)
    {
        PlatAudio::ReadEntireSampleFileIntoMemSync(sebringAudioFileData.szSampleFile);
    }
    else
    {
        PlatAudio::ReadEntireSampleFileIntoMem(sebringAudioFileData.szSampleFile);
    }
}

/**
 * Offset/Address/Size: 0x144 | 0x80143F10 | size: 0x64
 */
void AudioLoader::ResetForNewGame()
{
    bool isInited;

    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (!isInited)
    {
        return;
    }

    if (!gbDisableCrowd)
    {
        CrowdMood::Init();
        CrowdMood::SetMood(CrowdMood::CM_Neutral, 0);
    }

    Audio::GetPriorityStream()->Reset();
    Audio::ResetForNewGame();
}

/**
 * Offset/Address/Size: 0x68 | 0x80143E34 | size: 0xDC
 */
void AudioLoader::ResetForRematch()
{
    bool isInited;

    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (!isInited)
    {
        return;
    }

    CrowdMood::Purge(false);
    Audio::Silence();

    if (!gbDisableCrowd)
    {
        g_pTrackManager->StopAllTracks(false);
        Audio::GetPriorityStream()->m_Track.Stop(0);
        Audio::GetPriorityStream()->Reset();
        CrowdMood::Init();
        CrowdMood::SetMood(CrowdMood::CM_Neutral, 0);
    }

    if (gbDisableAudio)
    {
        return;
    }

    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (!isInited)
    {
        return;
    }

    if (!Audio::IsWorldSFXLoaded())
    {
        return;
    }

    Audio::ResetPauseStatus();
    nlSingleton<GameInfoManager>::s_pInstance->GetAudioOptions();
    CrowdMood::SetCrowdVolume(0x7f, 0);
}

/**
 * Offset/Address/Size: 0x0 | 0x80143DCC | size: 0x68
 * TODO: 88.5% match - bne+b vs beq branch optimization difference
 */
void AudioLoader::InitCrowdFromStateTransition()
{
    bool isInited;

    if (gbDisableAudio)
    {
        return;
    }

    if (gbDisableAudio)
    {
        isInited = false;
    }
    else
    {
        isInited = Audio::IsInited();
    }

    if (isInited == 0)
    {
        return;
    }

    if (Audio::IsWorldSFXLoaded() == false)
    {
        return;
    }

    Audio::ResetPauseStatus();
    nlSingleton<GameInfoManager>::s_pInstance->GetAudioOptions();
    CrowdMood::SetCrowdVolume(0x7f, 0);
}
