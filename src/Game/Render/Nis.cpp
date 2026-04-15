#include "Game/Render/Nis.h"
#include "Game/ReplayManager.h"
#include "Game/NisPlayer.h"
#include "Game/Sys/audio.h"
#include "NL/nlFunction.h"
#include "NL/nlList.h"

#include "types.h"

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

// /**
//  * Offset/Address/Size: 0xF0 | 0x8012D194 | size: 0x28
//  */
// void nlListAddStart<Nis::NisAudioData>(Nis::NisAudioData**, Nis::NisAudioData*, Nis::NisAudioData**)
// {
// }

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
 * Offset/Address/Size: 0x1658 | 0x8012CA68 | size: 0x53C
 */
Nis::Nis(NisHeader&, char*, int)
{
}

/**
 * Offset/Address/Size: 0x1650 | 0x8012CA60 | size: 0x8
 */
char* Nis::Name() const
{
    return mHeader->name;
}

/**
 * Offset/Address/Size: 0x13C0 | 0x8012C7D0 | size: 0x290
 */
Nis::~Nis()
{
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
 */
void Nis::Trigger::FireEffect(const Nis&) const
{
}

/**
 * Offset/Address/Size: 0x2D0 | 0x8012B6E0 | size: 0x564
 */
void Nis::Trigger::Fire(Nis&) const
{
    FORCE_DONT_INLINE;
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
