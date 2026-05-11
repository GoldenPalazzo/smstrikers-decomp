#include "Game/Render/Nis.h"
#include "Game/ReplayManager.h"
#include "Game/NisPlayer.h"
#include "Game/Sys/audio.h"
#include "Game/Audio/AudioStream.h"
#include "Game/CharacterAudio.h"
#include "Game/CharacterTriggers.h"
#include "Game/WorldManager.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/Sys/eventman.h"
#include "Game/Game.h"
#include "NL/nlFunction.h"
#include "NL/nlList.h"
#include "NL/nlString.h"

#include "types.h"

// Local mirror of NISData from Game/Render/Presentation.h — including that
// header drags in nlBasicString.h / nlTask.h, which clash with the in-file
// nlTaskManager / BasicString / TempStringAllocator declarations relied on
// by the already-matched ~Nis() / SelectCamera / Render in this TU.
struct NISData : public EventData
{
    virtual u32 GetID();

    /* 0x04 */ const char* Type;
    /* 0x08 */ const char* Param;
}; // total size: 0xC

class nlTaskManager
{
public:
    static void SetTimeDilation(float);
};

class EmissionController;

typedef void (*NisEmissionFn)(EmissionController&, int);
typedef BindExp2<void, NisEmissionFn, Placeholder<0>, int> (*NisBindPtr)(NisEmissionFn, const Placeholder<0>&, const int&);

NisBindPtr gNisBindPtr = &Bind<void, NisEmissionFn, Placeholder<0>, int>;

// /**
//  * Offset/Address/Size: 0xEA0 | 0x8012E074 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<const char*>(const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x12C | 0x8012D300 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<int>(const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8012D1D4 | size: 0x12C
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, char[64], int>(const BasicString<char, Detail::TempStringAllocator>&, const char(&)[64], const int&)
// {
// }

/**
 * Offset/Address/Size: 0xF0 | 0x8012D194 | size: 0x28
 */
template <>
void nlListAddStart<Nis::NisAudioData>(Nis::NisAudioData** head, Nis::NisAudioData* entry, Nis::NisAudioData** tail)
{
    if (tail != 0)
    {
        if (*head == 0)
        {
            *tail = entry;
        }
    }

    entry->next = *head;
    *head = entry;
}

// /**
//  * Offset/Address/Size: 0x54 | 0x8012D0F8 | size: 0x9C
//  */
// void nlListRemoveElement<Nis::NisAudioData>(Nis::NisAudioData**, Nis::NisAudioData*, Nis::NisAudioData**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8012D0A4 | size: 0x54
//  */
// void nlDeleteList<Nis::NisAudioData>(Nis::NisAudioData**)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8012D048 | size: 0x5C
//  */
// void Function1<void, EmissionController&>::FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::~FunctorImpl()
// {
// }

// /**
//  * Offset/Address/Size: 0x70 | 0x8012D014 | size: 0x34
//  */
// void Function1<void, EmissionController&>::FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::operator()(EmissionController&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x8012CFA4 | size: 0x70
//  */
// void Function1<void, EmissionController&>::FunctorImpl<BindExp2<void, void (*)(EmissionController&, int), Placeholder<0>, int>>::Clone() const
// {
// }

/**
 * Offset/Address/Size: 0x1650 | 0x8012CA60 | size: 0x8
 */
char* Nis::Name() const
{
    return mHeader->name;
}

struct BasicStringInternal
{
    char* mData;
    int mSize;
    int mCapacity;
    int mRefCount;
};

namespace Detail
{
class TempStringAllocator
{
public:
    static void* allocate(unsigned long size)
    {
        return nlMalloc(size, 8, true);
    }

    static void deallocate(void* ptr)
    {
        nlFree(ptr);
    }
};
} // namespace Detail

template <typename CharT, typename Allocator>
class BasicString
{
public:
    BasicStringInternal* m_data;

    BasicString()
        : m_data(0)
    {
    }

    BasicString(const CharT* str)
    {
        BasicStringInternal* data = (BasicStringInternal*)Allocator::allocate(sizeof(BasicStringInternal));
        if (data != 0)
        {
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;

            const CharT* s = str;
            while ((signed char)*s++ != 0)
            {
                data->mSize++;
            }
            data->mSize++;

            data->mData = (char*)Allocator::allocate(data->mSize + 1);
            data->mCapacity = data->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = *str++;
            }

            data->mRefCount = 1;
        }
        m_data = data;
    }

    BasicString(const BasicString& other)
    {
        BasicStringInternal* data;
        if (other.m_data)
        {
            other.m_data->mRefCount++;
            data = other.m_data;
        }
        else
        {
            data = 0;
        }
        m_data = data;
    }

    ~BasicString()
    {
        if (m_data)
        {
            BasicStringInternal* data = m_data;
            if (--data->mRefCount == 0)
            {
                if (data)
                {
                    if (data)
                    {
                        delete[] data->mData;
                    }
                    if (data)
                    {
                        nlFree(data);
                    }
                }
            }
        }
    }

    const CharT* c_str() const
    {
        static CharT emptyString = '\0';
        return m_data ? m_data->mData : &emptyString;
    }
};

template <typename StringType, typename Arg0, typename Arg1>
StringType Format(const StringType&, const Arg0&, const Arg1&);

/**
 * Offset/Address/Size: 0x1658 | 0x8012CA68 | size: 0x53C
 * TODO: 94.63% scratch match (uVwFo). Remaining diffs are register-allocation
 * permutation (target uses r25/r28/r30/r31 for header/this/end/chunk where
 * MWCC here picks r26/r25/r27/r28) plus an extra `li r7,0` at the
 * LoadCameraAnimation call site (target omits r7 even though signature is 4-arg).
 */
Nis::Nis(NisHeader& header, char* data, int size)
{
    mHeader = &header;
    mTarget = header.target;
    mWinnerType = header.winnerType;
    mData = data;
    mSize = size;
    mMirrored = NisPlayer::Instance()->IsMirrored(header.target, header.name, header.winnerType);
    mCamera = NULL;
    mNumCameras = 0;
    mNumTriggers = 0;
    mMainCharacterIndex = -1;
    mUnk_0x738 = -1;
    mNisAudioDataList = NULL;
    for (int i = 0; i < 10; i++)
    {
        mCharacterControllers[i] = NULL;
        mBallId[i] = -1;
    }
    nlChunk* chunk = (nlChunk*)data;
    nlChunk* end = (nlChunk*)(data + size);
    int numAnimations = 0;
    while (chunk != end)
    {
        if ((chunk->m_ID & 0x80FFFFFF) == 0x80017000)
        {
            cSAnim* anim = cSAnim::Initialize(chunk);
            int i = NisPlayer::Instance()->TargetToIndex(mTarget, numAnimations, mWinnerType);
            if (NisPlayer::Instance()->mGoalScorerCharIndex >= 0 && mTarget == NIS_TARGET_WINNER_SIDEKICK)
            {
                int goalScorer = NisPlayer::Instance()->mGoalScorerCharIndex;
                mMainCharacterIndex = goalScorer;
                i = goalScorer;
            }
            NisPlayer::Instance()->mGoalScorerCharIndex = -1;
            if (mCharacterControllers[i] != NULL)
            {
                i = NisPlayer::Instance()->TargetToIndex(NIS_TARGET_HOME_CAPTAIN, numAnimations, mWinnerType);
            }
            if (mCharacterControllers[i] != NULL)
            {
                i = NisPlayer::Instance()->TargetToIndex(NIS_TARGET_AWAY_CAPTAIN, numAnimations, mWinnerType);
            }
            if (mCharacterControllers[i] != NULL)
            {
                for (i = 0; i < 10; i++)
                {
                    if (mCharacterControllers[i] == NULL)
                        break;
                }
            }
            if (i < 10)
            {
                mBallId[i] = numAnimations;
                cPN_SAnimController* controller = new (AllocateSAnimController()) cPN_SAnimController(anim, NULL, PM_HOLD, NULL, 0, false);
                mCharacterControllers[i] = controller;
                if (mUnk_0x738 < 0)
                {
                    mUnk_0x738 = i;
                }
            }
            numAnimations++;
        }
        if ((chunk->m_ID & 0x80FFFFFF) == 0x80015501)
        {
            BasicString<char, Detail::TempStringAllocator> name = Format(BasicString<char, Detail::TempStringAllocator>("{0}_{1}"), mHeader->name, mNumCameras);
            cAnimCamera* cam = (cAnimCamera*)((char*)chunk + 8);
            cam->LoadCameraAnimation((nlChunk*)((char*)chunk + chunk->m_Size + 8), (nlChunk*)name.c_str(), (const char*)0, false);
            mNumCameras++;
        }
        chunk = (nlChunk*)((char*)chunk + chunk->m_Size + 8);
    }
}

/**
 * Offset/Address/Size: 0x13C0 | 0x8012C7D0 | size: 0x290
 * TODO: 95.62% match - r28/r29/r30 register assignment swap (this/dealloc/data)
 * and compiler hoists format string load outside loop body
 */
Nis::~Nis()
{
    for (int i = 0; i < mNumCameras; i++)
    {
        BasicString<char, Detail::TempStringAllocator> cameraName = Format(BasicString<char, Detail::TempStringAllocator>("{0}_{1}"), mHeader->name, i);
        cAnimCamera::FreeCameraAnimation(cameraName.c_str());
    }

    if (mCamera)
    {
        mCamera->UnselectCameraAnimation();
    }

    StopAllOutstandingNisAudio();
    NisPlayer::Instance()->ResetEffects();
    nlTaskManager::SetTimeDilation(1.0f);
}

/**
 * Offset/Address/Size: 0x1350 | 0x8012C760 | size: 0x70
 */
void Nis::Update(float dt)
{
    for (int i = 0; i < 10; ++i)
    {
        cPN_SAnimController* pController = mCharacterControllers[i];
        if (pController != nullptr)
        {
            pController->Update(dt);
        }
    }
}

/**
 * Offset/Address/Size: 0x1270 | 0x8012C680 | size: 0xE0
 */
void Nis::UpdateTriggers(float oldTime, float newTime, float duration)
{
    if (duration != 0.0f)
    {
        for (int i = 0; i < mNumTriggers; ++i)
        {
            float triggerFrame = (mTriggers[i].frameNumber / 30.0f) / duration;
            if ((oldTime <= triggerFrame) && (newTime > triggerFrame))
            {
                mTriggers[i].Fire(*this);
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xF80 | 0x8012C390 | size: 0x2F0
 * TODO: 96.01% match - remaining diffs are MWCC register allocation and literal
 * symbol selection (`this`/camera/temp register rotation and local constant labels).
 */
void Nis::SelectCamera(cAnimCamera& camera, int cameraIndex)
{
    if (mNumCameras == 0)
    {
        return;
    }

    int index = cameraIndex % mNumCameras;
    BasicString<char, Detail::TempStringAllocator> cameraName = Format(BasicString<char, Detail::TempStringAllocator>("{0}_{1}"), mHeader->name, index);

    camera.SelectCameraAnimation(cameraName.c_str());

    if (mMirrored)
    {
        camera.m_Mirror = (nlVector3) { -1.0f, 1.0f, 1.0f };
    }
    else
    {
        camera.m_Mirror = (nlVector3) { 1.0f, 1.0f, 1.0f };
    }

    camera.m_fAnimationTime = 0.0f;
    camera.BuildAnimViewMatrix(camera.m_matView);

    if (strstr(mHeader->name, "cup") != NULL)
    {
        camera.m_bCyclic = true;
    }
    else
    {
        camera.m_bCyclic = false;
    }

    mCamera = &camera;
}

/**
 * Offset/Address/Size: 0xF18 | 0x8012C328 | size: 0x68
 */
bool Nis::SelectRandomCamera(cAnimCamera& camera)
{
    if (mNumCameras == 0)
    {
        return false;
    }

    int randomIndex = nlRandom(mNumCameras, &nlDefaultSeed);
    SelectCamera(camera, randomIndex);
    return true;
}

/**
 * Offset/Address/Size: 0xD18 | 0x8012C128 | size: 0x200
 * TODO: 96.91% match - remaining diffs are MWCC register assignment/scheduling:
 * r29/r30/r31 role rotation (this/character base/current dc), GetRootRot load order,
 * and FP temp register allocation around stadium/offset accumulation.
 */
void Nis::Render()
{
    DrawableCharacter* pDC;
    RenderSnapshot& snapshot = ReplayManager::Instance()->GetMutableRenderSnapshot();
    nlVector3 offset = { 0.0f, 0.0f, 0.0f };
    int numBalls = 0;

    for (int i = 0; i < 10; i++)
    {
        pDC = &snapshot.mCharacters[i];
        if (mCharacterControllers[i] == NULL)
            continue;
        pDC->mVisible = true;

        nlVector3 rootTrans = { 0.0f, 0.0f, 0.0f };
        u16 angle = 0;
        float fTime = mCharacterControllers[i]->m_fTime;
        mCharacterControllers[i]->m_pSAnim->GetRootTrans(fTime, &rootTrans);
        fTime = mCharacterControllers[i]->m_fTime;
        mCharacterControllers[i]->m_pSAnim->GetRootRot(fTime, &angle);
        if (mMirrored)
        {
            mCharacterControllers[i]->m_bMirror = true;
            rootTrans.f.x *= -1.0f;
            angle = angle + (0x4000 - angle) * 2;
        }

        float z = rootTrans.f.z + mHeader->stadiumOffset.f.z;
        float y = rootTrans.f.y + mHeader->stadiumOffset.f.y;
        float x = rootTrans.f.x + mHeader->stadiumOffset.f.x;
        rootTrans.f.z = z;
        rootTrans.f.y = y;
        rootTrans.f.x = x;
        rootTrans.f.x = x + offset.f.x;
        rootTrans.f.y = y + offset.f.z;
        rootTrans.f.z = z + offset.f.y;

        pDC->EvaluateFrom(*mCharacterControllers[i], rootTrans, angle);
        pDC->BuildNodeMatrices();
        if (mBallId[i] >= 0 && numBalls < mHeader->numBalls
            && numBalls < NisPlayer::Instance()->mMaxNumBallsVisible)
        {
            if (mBallId[i] == 0)
            {
                snapshot.mBall.mVisible = true;
                snapshot.mBall.EvaluateFrom(*pDC);
            }
            numBalls++;
        }
    }
}

/**
 * Offset/Address/Size: 0xCF8 | 0x8012C108 | size: 0x20
 */
nlVector3 Nis::Offset() const
{
    return mHeader->stadiumOffset;
}

/**
 * Offset/Address/Size: 0xC10 | 0x8012C020 | size: 0xE8
 */
void Nis::AddTrigger(NisTriggerType triggerType, float frameNumber, const char* name, const char* target, Nis::TriggerParams* trigParams)
{
    mTriggers[mNumTriggers].type = triggerType;
    mTriggers[mNumTriggers].frameNumber = frameNumber;
    mTriggers[mNumTriggers].name = name;
    mTriggers[mNumTriggers].target = target;

    TriggerParams* pParams = &(mTriggers[mNumTriggers].params);
    pParams->float1 = -1.0f;
    pParams->param1 = -1;
    pParams->param2 = -1;
    pParams->param3 = -1;
    pParams->param4 = -1;

    if (trigParams != NULL)
    {
        mTriggers[mNumTriggers].params.float1 = trigParams->float1;
        mTriggers[mNumTriggers].params.param1 = trigParams->param1;
        mTriggers[mNumTriggers].params.param2 = trigParams->param2;
        mTriggers[mNumTriggers].params.param3 = trigParams->param3;
        mTriggers[mNumTriggers].params.param4 = trigParams->param4;
    }

    mNumTriggers++;
}

/**
 * Offset/Address/Size: 0x834 | 0x8012BC44 | size: 0x3DC
 * TODO: ~97% scratch match (9984U). Transferred from decomp.me scratch on
 * 2026-05-11 to preserve progress baseline for collaborator.
 */
void Nis::Trigger::FireEffect(const Nis& nis) const
{
    NisPlayer* player = NULL;
    if (params.param1 == 0)
    {
        player = NisPlayer::Instance();
    }

    if (strstr(target, "ball") != NULL)
    {
        EffectsGroup* group = fxGetGroup(name);
        if (group == NULL)
            return;
        EmissionController* ctrl = EmissionManager::Create(group, 0);
        if (ctrl == NULL)
            return;
        ctrl->m_uUserData = (u32)player;
        {
            Function<EmissionController&> update;
            update.mTag = FREE_FUNCTION;
            update.mFreeFunction = UpdateEmitterFromBall;
            ctrl->SetUpdateCallback(update);
        }
    }
    else if (strstr(target, "bip0") != NULL)
    {
        s32 idx = (s32)(s8)target[4] - '1';
        if (idx < 0)
            idx = 0;

        s32 index = nis.mMainCharacterIndex;
        if (index < 0)
        {
            index = NisPlayer::Instance()->TargetToIndex(nis.mTarget, idx, nis.mWinnerType);
        }

        s32 charIdx = index;
        if (charIdx >= 10)
            return;

        EffectsGroup* group = fxGetGroup(name);
        if (group == NULL)
            return;
        EmissionController* ctrl = EmissionManager::Create(group, 0);
        if (ctrl == NULL)
            return;
        ctrl->SetAnimController(*nis.mCharacterControllers[charIdx]);
        ctrl->m_uUserData = (u32)player;
        if (!nis.mMirrored)
        {
            nlVector3 mirror = { -1.0f, 1.0f, 1.0f };
            ctrl->m_Mirror = mirror;
        }
        u8 hasOffset;
        EffectsSpec* specs = group->m_specs;
        if (specs == NULL)
        {
            hasOffset = 0;
        }
        else
        {
            hasOffset = 0;
            for (int i = 0; i < group->m_numSpecs; i++)
            {
                if (specs[i].m_vLocalOffset.f.x != 0.0f || specs[i].m_vLocalOffset.f.y != 0.0f || specs[i].m_vLocalOffset.f.z != 0.0f)
                {
                    hasOffset = 1;
                    break;
                }
            }
        }
        if (hasOffset)
        {
            Function<EmissionController&> callback(
                Bind<void>(UpdateEmitterFromCharacterIdxWithCoordSys, placeholder0, charIdx));
            ctrl->SetUpdateCallback(callback);
        }
        else
        {
            Function<EmissionController&> callback(
                Bind<void>(UpdateEmitterFromCharacterIdxWithoutAnimController, placeholder0, charIdx));
            ctrl->SetUpdateCallback(callback);
        }
    }
    else
    {
        World* world = WorldManager::s_World;
        HelperObject* helperObj = world->FindHelperObject(world->GetHashIdForGenericName(target));
        if (helperObj == NULL)
            return;
        nlVector3 velocity = { 0.0f, 0.0f, 1.0f };
        EmissionController* ctrl = EmissionManager::Create(fxGetGroup(name), 0);
        ctrl->m_uUserData = (u32)player;
        ctrl->SetVelocity(velocity);
        ctrl->SetPosition(helperObj->m_worldMatrix.GetTranslation());
        ctrl->m_fGround = 0.02f;
    }
}

/**
 * Offset/Address/Size: 0x2D0 | 0x8012B6E0 | size: 0x564
 * TODO: ~90% scratch match (xqwZV). Transferred from decomp.me scratch on
 * 2026-05-11 to preserve progress baseline for collaborator.
 */
void Nis::Trigger::Fire(Nis& nis) const
{
    struct NisAudioDataExt
    {
        NisAudioType audioType;
        union
        {
            SFXEmitter* pEmitter;
            unsigned long index;
        } identifier;
        unsigned long soundType;
        char str[128];
        unsigned char isEmitter;
        unsigned char stopAtNisEnd;
        unsigned char pad[2];
        NisAudioDataExt* next;
    };

    bool bIsEmitter;
    bool bStopAtNisEnd;
    unsigned long sfxHandle;

    switch (type)
    {
    case NIS_TRIGGER_TYPE_PLAY_SOUND:
    {
        float volume = params.float1;
        unsigned long trackingId = (unsigned long)-1;
        bIsEmitter = false;
        bStopAtNisEnd = true;

        if (volume != -1.0f)
            volume = 100.0f;

        if (params.param1 == (unsigned long)-1)
        {
            if (strlen(target) > 0)
            {
                World* pWorld = WorldManager::s_World;
                unsigned long hashId = pWorld->GetHashIdForGenericName(target);
                HelperObject* helper = pWorld->FindHelperObject(hashId);
                if (helper == NULL)
                    return;
                static nlVector3 zeroDirection = { 0.0f, 0.0f, 0.0f };
                sfxHandle = Audio::PlayWorldSFXbyStr(name, volume, -1.0f, true, false, (const nlVector3*)&helper->m_worldMatrix.m[3][0], &zeroDirection, &trackingId);
                bIsEmitter = true;
            }
            else
            {
                sfxHandle = Audio::PlayWorldSFXbyStr(name, 100.0f, -1.0f, false, true, NULL, NULL, NULL);
            }
        }
        else
        {
            int charIdx = nis.mUnk_0x738;
            RenderSnapshot& snap = ReplayManager::Instance()->GetMutableRenderSnapshot();
            int charIdx2 = nis.mUnk_0x738;
            RenderSnapshot& snap2 = ReplayManager::Instance()->GetMutableRenderSnapshot();
            sfxHandle = (unsigned long)Audio::PlayCharSFXbyStr(name, (NisCharacterClass)params.param1, volume, -1.0f, true, false, &snap2.mCharacters[charIdx2].mBip01Position, &snap.mCharacters[charIdx].mVelocity, &trackingId);
            bIsEmitter = true;
        }

        if (params.param2 != (unsigned long)-1)
            bStopAtNisEnd = false;
        if (sfxHandle == (unsigned long)-1)
            break;

        const char* sfxName = name;
        unsigned long trackingVal = trackingId;
        NisAudioDataExt* pAudioData = (NisAudioDataExt*)nlMalloc(sizeof(NisAudioDataExt), 8, false);
        pAudioData->audioType = NIS_AUDIO_TYPE_NONE;
        pAudioData->identifier.index = (unsigned long)-1;
        memset(pAudioData->str, 0, 0x80);
        pAudioData->soundType = (unsigned long)-1;
        pAudioData->stopAtNisEnd = 1;
        pAudioData->isEmitter = 0;
        pAudioData->audioType = NIS_AUDIO_TYPE_SFX;
        if (bIsEmitter)
            pAudioData->identifier.pEmitter = (SFXEmitter*)sfxHandle;
        else
            pAudioData->identifier.index = sfxHandle;
        nlStrNCpy(pAudioData->str, sfxName, (unsigned long)0x80);
        pAudioData->soundType = trackingVal;
        pAudioData->isEmitter = bIsEmitter;
        pAudioData->stopAtNisEnd = bStopAtNisEnd;
        pAudioData->next = NULL;
        nlListAddStart((NisAudioData**)&nis.mNisAudioDataList, (NisAudioData*)pAudioData, (NisAudioData**)NULL);
        break;
    }

    case NIS_TRIGGER_TYPE_PLAY_RANDOM_DIALOGUE:
    {
        unsigned long trackingId = (unsigned long)-1;
        int charIdx = nis.mUnk_0x738;
        RenderSnapshot& snap = ReplayManager::Instance()->GetMutableRenderSnapshot();
        int charIdx2 = nis.mUnk_0x738;
        RenderSnapshot& snap2 = ReplayManager::Instance()->GetMutableRenderSnapshot();
        sfxHandle = Audio::cCharacterSFX::PlayNISRandomCharDialogue((CharDialogueType)params.param2, (NisCharacterClass)params.param1, 100.0f, -1.0f, true, &snap2.mCharacters[charIdx2].mBip01Position, &snap.mCharacters[charIdx].mVelocity, &trackingId);
        bStopAtNisEnd = true;
        if (params.param3 != (unsigned long)-1)
            bStopAtNisEnd = false;
        if (sfxHandle == (unsigned long)-1)
            break;

        unsigned long trackingVal = trackingId;
        const char* sfxName = name;
        NisAudioDataExt* pAudioData = (NisAudioDataExt*)nlMalloc(sizeof(NisAudioDataExt), 8, false);
        pAudioData->audioType = NIS_AUDIO_TYPE_NONE;
        pAudioData->identifier.index = (unsigned long)-1;
        memset(pAudioData->str, 0, 0x80);
        pAudioData->soundType = (unsigned long)-1;
        pAudioData->stopAtNisEnd = 1;
        pAudioData->isEmitter = 0;
        pAudioData->audioType = NIS_AUDIO_TYPE_SFX;
        pAudioData->identifier.index = sfxHandle;
        nlStrNCpy(pAudioData->str, sfxName, (unsigned long)0x80);
        pAudioData->soundType = trackingVal;
        pAudioData->isEmitter = true;
        pAudioData->stopAtNisEnd = bStopAtNisEnd;
        pAudioData->next = NULL;
        nlListAddStart((NisAudioData**)&nis.mNisAudioDataList, (NisAudioData*)pAudioData, (NisAudioData**)NULL);
        break;
    }

    case NIS_TRIGGER_TYPE_STOP_SOUND:
    {
        const char* targetName = name;
        NisAudioDataExt* pNode = (NisAudioDataExt*)nis.mNisAudioDataList;
        while (pNode != NULL)
        {
            if (nlStrICmp(pNode->str, targetName) != 0)
            {
                pNode = pNode->next;
                continue;
            }
            if (pNode->isEmitter)
            {
                SFXEmitter* pEmitter = pNode->identifier.pEmitter;
                if (pNode->soundType == pEmitter->soundType)
                {
                    if (Audio::Remove3DSFXEmitter(pEmitter))
                    {
                        if (!Audio::IsEmitterActive(pEmitter))
                        {
                            pEmitter->bKeepTrack = true;
                            pEmitter->soundType = (unsigned long)-1;
                            pEmitter->fTimeStamp = -1.0f;
                            pEmitter->bIsStopping = false;
                            pEmitter->bInUse = false;
                            pEmitter->bIsFilterOn = false;
                            pEmitter->m_unk_0x5F = false;
                            pEmitter->pPhysObj = NULL;
                            pEmitter->pOwner = NULL;
                            pEmitter->pos.pvPos = NULL;
                            pEmitter->dir.pvDir = NULL;
                            pEmitter->pos.vPos.f.x = 0.0f;
                            pEmitter->pos.vPos.f.y = 0.0f;
                            pEmitter->pos.vPos.f.z = 0.0f;
                            pEmitter->dir.vDir.f.x = 0.0f;
                            pEmitter->dir.vDir.f.y = 0.0f;
                            pEmitter->dir.vDir.f.z = 0.0f;
                            pEmitter->posUpdateMethod = NONE;
                            if (pEmitter->pMIDIControllerInfo != NULL)
                            {
                                if (pEmitter->pMIDIControllerInfo->paraArray != NULL)
                                    delete[] pEmitter->pMIDIControllerInfo->paraArray;
                                delete pEmitter->pMIDIControllerInfo;
                            }
                            pEmitter->pMIDIControllerInfo = NULL;
                            pNode->identifier.pEmitter = NULL;
                        }
                    }
                }
            }
            else
            {
                if (Audio::IsSFXPlaying(pNode->identifier.index))
                {
                    Audio::StopSFX(pNode->identifier.index);
                    pNode->identifier.index = (unsigned long)-1;
                }
            }
            nlListRemoveElement((NisAudioData**)&nis.mNisAudioDataList, (NisAudioData*)pNode, (NisAudioData**)NULL);
            NisAudioDataExt* pNext = pNode->next;
            pNode->audioType = NIS_AUDIO_TYPE_NONE;
            pNode->identifier.index = (unsigned long)-1;
            memset(pNode->str, 0, 0x80);
            pNode->soundType = (unsigned long)-1;
            pNode->stopAtNisEnd = 1;
            pNode->isEmitter = 0;
            delete pNode;
            pNode = pNext;
        }
        break;
    }

    case NIS_TRIGGER_TYPE_PLAY_STREAM:
    case NIS_TRIGGER_TYPE_STOP_STREAM:
    case NIS_TRIGGER_TYPE_SET_ACTIVE_STREAM_LOOPING:
        break;

    case NIS_TRIGGER_TYPE_STOP_ALL_STREAMS:
        Audio::StopStreaming();
        break;

    case NIS_TRIGGER_TYPE_REGISTER_GOAL_AUDIO:
        g_pGame->m_nLastTeamToScore = NisPlayer::Instance()->mWinnerSide[1];
        break;

    case NIS_TRIGGER_TYPE_TIME_DILATION:
        nlTaskManager::SetTimeDilation(params.float1);
        break;

    case NIS_TRIGGER_TYPE_EFFECT:
        FireEffect(nis);
        break;

    case NIS_TRIGGER_TYPE_RAISE_EVENT:
    {
        Event* event = g_pEventManager->CreateValidEvent(0x56, 0x20);
        NISData* pData = new (&event->m_data) NISData();
        pData->Type = name;
        pData->Param = target;
        break;
    }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8012B410 | size: 0x2D0
 */
void Nis::StopAllOutstandingNisAudio()
{
    struct NisAudioDataExt
    {
        NisAudioType audioType;
        union
        {
            SFXEmitter* pEmitter;
            unsigned long index;
        } identifier;
        unsigned long soundType;
        char str[128];
        unsigned char isEmitter;
        unsigned char stopAtNisEnd;
        unsigned char pad[2];
        NisAudioDataExt* next;
    };

    NisAudioDataExt* pNisAudioData = (NisAudioDataExt*)mNisAudioDataList;
    while (pNisAudioData != NULL)
    {
        switch (pNisAudioData->audioType)
        {
        case NIS_AUDIO_TYPE_SFX:
        {
            SFXEmitter* pSFXEmitter;
            unsigned char bNisEndedNormally = 0;
            cPN_SAnimController* pController;
            int i;
            for (i = 0; i < 10; i++)
            {
                pController = mCharacterControllers[i];
                if (pController != NULL)
                {
                    float remainingTime = 1.0f - pController->m_fTime;
                    if (remainingTime < 0.1f)
                    {
                        bNisEndedNormally = 1;
                        break;
                    }
                }
            }

            if (pNisAudioData->isEmitter)
            {
                pSFXEmitter = pNisAudioData->identifier.pEmitter;
                if (pNisAudioData->soundType == pSFXEmitter->soundType)
                {
                    if ((!bNisEndedNormally) || (bNisEndedNormally && pNisAudioData->stopAtNisEnd))
                    {
                        if (Audio::Remove3DSFXEmitter(pSFXEmitter))
                        {
                            if (!Audio::IsEmitterActive(pSFXEmitter))
                            {
                                pSFXEmitter->bKeepTrack = true;
                                pSFXEmitter->soundType = (unsigned long)-1;
                                pSFXEmitter->fTimeStamp = -1.0f;
                                pSFXEmitter->bIsStopping = false;
                                pSFXEmitter->bInUse = false;
                                pSFXEmitter->bIsFilterOn = false;
                                pSFXEmitter->m_unk_0x5F = false;
                                pSFXEmitter->pPhysObj = NULL;
                                pSFXEmitter->pOwner = NULL;
                                pSFXEmitter->pos.pvPos = NULL;
                                pSFXEmitter->dir.pvDir = NULL;
                                pSFXEmitter->pos.vPos.f.x = 0.0f;
                                pSFXEmitter->pos.vPos.f.y = 0.0f;
                                pSFXEmitter->pos.vPos.f.z = 0.0f;
                                pSFXEmitter->dir.vDir.f.x = 0.0f;
                                pSFXEmitter->dir.vDir.f.y = 0.0f;
                                pSFXEmitter->dir.vDir.f.z = 0.0f;
                                pSFXEmitter->posUpdateMethod = NONE;

                                if (pSFXEmitter->pMIDIControllerInfo != NULL)
                                {
                                    if (pSFXEmitter->pMIDIControllerInfo->paraArray != NULL)
                                    {
                                        delete[] pSFXEmitter->pMIDIControllerInfo->paraArray;
                                    }
                                    delete pSFXEmitter->pMIDIControllerInfo;
                                }
                                pSFXEmitter->pMIDIControllerInfo = NULL;
                                pNisAudioData->identifier.pEmitter = NULL;
                            }
                        }
                    }
                }
            }
            else
            {
                if (Audio::IsSFXPlaying(pNisAudioData->identifier.index))
                {
                    if ((!bNisEndedNormally) || (bNisEndedNormally && pNisAudioData->stopAtNisEnd))
                    {
                        Audio::StopSFX(pNisAudioData->identifier.index);
                        pNisAudioData->identifier.index = (unsigned long)-1;
                    }
                }
            }

            nlListRemoveElement(&mNisAudioDataList, (NisAudioData*)pNisAudioData, (NisAudioData**)NULL);
            NisAudioDataExt* pNextNisAudioData = pNisAudioData->next;

            pNisAudioData->audioType = NIS_AUDIO_TYPE_NONE;
            pNisAudioData->identifier.index = (unsigned long)-1;
            memset(pNisAudioData->str, 0, 0x80);
            pNisAudioData->soundType = (unsigned long)-1;
            pNisAudioData->stopAtNisEnd = 1;
            pNisAudioData->isEmitter = 0;

            delete pNisAudioData;
            pNisAudioData = pNextNisAudioData;
            break;
        }
        case NIS_AUDIO_TYPE_NONE:
        case NIS_AUDIO_TYPE_STREAM:
        default:
            pNisAudioData = pNisAudioData->next;
            break;
        }
    }

    nlDeleteList(&mNisAudioDataList);
    mNisAudioDataList = NULL;
}
