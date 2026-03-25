#include "Game/NisPlayer.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Character.h"
#include "Game/CharacterTemplate.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Game.h"
#include "Game/Goalie.h"
#include "Game/ReplayManager.h"
#include "NL/nlFileGC.h"
#include "NL/nlTask.h"
#include "NL/nlString.h"
#include "string.h"

bool g_ForceDoubleBallTransition;

struct TempStringData
{
    char* data;
    int size;
    int capacity;
    int refCount;
};

extern void* nlMalloc(unsigned long, unsigned int, bool);
extern void nlFree(void*);
extern void nlReadAsync(nlFile*, void*, unsigned int, void (*)(nlFile*, void*, unsigned int, unsigned long), unsigned long);
extern void nlAsyncLoadFileToVirtualMemory(nlFile*, int, void*, void (*)(nlFile*, void*, unsigned int, unsigned long), unsigned long);
extern void* nlLoadEntireFileToVirtualMemory(const char*, int*, unsigned int, void*, eAllocType);
extern void nlBreak();

namespace Detail
{
class TempStringAllocator
{
};
} // namespace Detail

template <typename CharT, typename Allocator>
class BasicString
{
public:
    TempStringData* m_data;

    BasicString(const CharT* str)
    {
        TempStringData* data = (TempStringData*)nlMalloc(sizeof(TempStringData), 8, true);
        if (data != 0)
        {
            data->data = 0;
            data->size = 0;
            data->capacity = 0;

            const CharT* s = str;
            while ((signed char)*s++ != 0)
            {
                data->size++;
            }

            data->size++;
            data->data = (char*)nlMalloc(data->size + 1, 8, true);
            data->capacity = data->size;

            for (int i = 0; i < data->size; i++)
            {
                data->data[i] = *str++;
            }

            data->refCount = 1;
        }

        m_data = data;
    }

    ~BasicString()
    {
        if (m_data)
        {
            TempStringData* data = m_data;
            if (--data->refCount == 0)
            {
                if (data)
                {
                    if (data)
                    {
                        delete[] data->data;
                    }
                    if (data)
                    {
                        nlFree(data);
                    }
                }
            }
        }
    }

    void AppendInPlace(const CharT* str);

    const CharT* c_str() const
    {
        static CharT emptyString = '\0';
        return m_data ? (CharT*)m_data->data : &emptyString;
    }
};

static unsigned char useAsyncLoading;

// /**
//  * Offset/Address/Size: 0x74 | 0x80118E84 | size: 0x2C
//  */
// void nlStrLen<char>(const char*)
// {
// }

// /**
//  * Offset/Address/Size: 0x54 | 0x80118E64 | size: 0x20
//  */
// void nlToLower<char>(char)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80118E10 | size: 0x54
//  */
// void nlToLower<char>(char*)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801186C4 | size: 0x74C
 */
void NisPlayer::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
    {
        m_SP--;

        unsigned long p1 = *m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;
        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_EFFECT, frame, name, target, &params);
        break;
    }
    case 1:
    {
        m_SP--;
        int useStopAtNisEnd = (int)*m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;
        if (useStopAtNisEnd == 1)
        {
            params.param2 = 1;
        }

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_SOUND, frame, name, target, &params);
        break;
    }
    case 2:
    {
        m_SP--;
        int useStopAtNisEnd = (int)*m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        float volume = *(float*)m_SP;
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.float1 = volume;
        params.param1 = p1;
        if (useStopAtNisEnd == 1)
        {
            params.param2 = 1;
        }

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_SOUND, frame, name, target, &params);
        break;
    }
    case 3:
    {
        m_SP--;
        int useNameAsTarget = (int)*m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        unsigned long p2 = *m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;
        params.param2 = p2;
        if (useNameAsTarget == 1)
        {
            params.param3 = 1;
        }

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_RANDOM_DIALOGUE, frame, "", target, &params);
        break;
    }
    case 4:
    {
        m_SP--;
        unsigned long p4 = *m_SP;
        m_SP--;
        unsigned long p3 = *m_SP;
        m_SP--;
        unsigned long p2 = *m_SP;
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;
        params.param2 = p2;
        params.param3 = p3;
        params.param4 = p4;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_STREAM, frame, "", "", &params);
        break;
    }
    case 5:
    {
        m_SP--;
        int useStopAtNisEnd = (int)*m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        if (useStopAtNisEnd == 1)
        {
            params.param2 = 1;
        }

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_SOUND, frame, name, target, &params);
        break;
    }
    case 6:
    {
        m_SP--;
        int useStopAtNisEnd = (int)*m_SP;
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        float volume = *(float*)m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.float1 = volume;
        if (useStopAtNisEnd == 1)
        {
            params.param2 = 1;
        }

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_PLAY_SOUND, frame, name, target, &params);
        break;
    }
    case 7:
    {
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_RAISE_EVENT, frame, name, target, NULL);
        break;
    }
    case 8:
    {
        m_SP--;
        float frame = *(float*)m_SP;
        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_REGISTER_GOAL_AUDIO, frame, "", "", NULL);
        break;
    }
    case 9:
    {
        m_SP--;
        unsigned long loopActive = *m_SP;
        m_SP--;
        unsigned long stopAtNisEnd = *m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;

        if (loopActive != 0)
        {
            params.param1 = 1;
        }
        else
        {
            params.param1 = 0;
        }

        params.param2 = stopAtNisEnd;
        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_SET_ACTIVE_STREAM_LOOPING, frame, "", "", &params);
        break;
    }
    case 10:
    {
        m_SP--;
        float frame = *(float*)m_SP;
        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_STOP_ALL_STREAMS, frame, "", "", NULL);
        break;
    }
    case 11:
    {
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_STOP_SOUND, frame, name, target, &params);
        break;
    }
    case 12:
    {
        m_SP--;
        unsigned long p1 = *m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.param1 = p1;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_STOP_STREAM, frame, "", "", &params);
        break;
    }
    case 13:
    {
        m_SP--;
        const char* target = (const char*)*m_SP;
        m_SP--;
        const char* name = (const char*)*m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_STOP_SOUND, frame, name, target, NULL);
        break;
    }
    case 14:
    {
        m_SP--;
        float delta = *(float*)m_SP;
        m_SP--;
        float frame = *(float*)m_SP;

        Nis::TriggerParams params;
        params.float1 = -1.0f;
        params.param1 = -1;
        params.param2 = -1;
        params.param3 = -1;
        params.param4 = -1;
        params.float1 = delta;

        mNisForTriggerLoading->AddTrigger(NIS_TRIGGER_TYPE_TIME_DILATION, frame, "", "", &params);
        break;
    }
    default:
        nlBreak();
        break;
    }
}

/**
 * Offset/Address/Size: 0x3990 | 0x8011866C | size: 0x58
 */
NisPlayer* NisPlayer::Instance()
{
    static NisPlayer instance;
    return &instance;
}

/**
 * Offset/Address/Size: 0x3408 | 0x801180E4 | size: 0x588
 */
NisPlayer::NisPlayer()
    : InterpreterCore(0x1000)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x3358 | 0x80118034 | size: 0xB0
 */
float NisPlayer::TimeLeft() const
{
    if (mPlaying[0] != NULL)
    {
        if (strstr(mPlaying[0]->Name(), "trophy") != NULL)
        {
            return 1.0f;
        }
    }

    cCameraData* pCameraData = mCamera.m_pActiveCameraData;
    if (pCameraData != NULL)
    {
        float animTime = mCamera.m_fAnimationTime;
        float duration;
        if (pCameraData != NULL)
        {
            duration = (float)(pCameraData->m_uKeyCount - 1) / 30.0f;
        }
        else
        {
            duration = 0.0f;
        }

        return (1.0f - animTime) * duration;
    }

    return 0.0f;
}

/**
 * Offset/Address/Size: 0x327C | 0x80117F58 | size: 0xDC
 */
bool NisPlayer::WorldIsFrozen() const
{
    bool stateOK = (nlTaskManager::m_pInstance->m_CurrState == 0x100);
    if (stateOK)
    {
        return TimeLeft() == 0.0f;
    }
    return stateOK;
}

/**
 * Offset/Address/Size: 0x2F64 | 0x80117C40 | size: 0x318
 * TODO: 95.38% match - register allocation diffs from -inline deferred string literal hoisting in BasicString ctor
 */
void NisPlayer::HandleAsyncs()
{
    for (int i = 0; i < 4; i++)
    {
        if (mLoadQueue[i] != 0)
        {
            if (!mAsyncStarted[i])
            {
                mAsyncStarted[i] = 1;

                if (mLoadingFromBack)
                {
                    mUsedFromBack -= mLoadQueue[i]->size;
                    mUsedFromBack -= 0x20;
                }

                int used;
                if (mLoadingFromBack)
                {
                    used = mUsedFromBack;
                }
                else
                {
                    used = mUsedFromFront;
                }

                char* loadAt = mMemory + used;
                loadAt = loadAt + (0x20 - ((unsigned int)loadAt & 0x1F));

                if (!mLoadingFromBack)
                {
                    mUsedFromFront += mLoadQueue[i]->size;
                    mUsedFromFront += 0x20;
                }

                if (mUsedFromFront >= mUsedFromBack)
                {
                    nlBreak();
                }

                BasicString<char, Detail::TempStringAllocator> fileName("art/nis/");
                fileName.AppendInPlace(mLoadQueue[i]->name);

                if (useAsyncLoading)
                {
                    nlFile* file = nlOpen(fileName.c_str());
                    if ((OSGetConsoleType() & 0x20000000) != 0)
                    {
                        nlReadAsync(file, loadAt, mLoadQueue[i]->size, AsyncLoad, (unsigned long)mLoadQueue[i]);
                    }
                    else
                    {
                        nlAsyncLoadFileToVirtualMemory(file, mLoadQueue[i]->size, loadAt, AsyncLoad, (unsigned long)mLoadQueue[i]);
                    }
                }
                else
                {
                    int size = 0;
                    nlLoadEntireFileToVirtualMemory(fileName.c_str(), &size, 0x2000, loadAt, AllocateStart);
                    AsyncLoad(0, loadAt + mLoadQueue[i]->size, mLoadQueue[i]->size, (unsigned long)mLoadQueue[i]);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x2DA4 | 0x80117A80 | size: 0x1C0
 */
void NisPlayer::Update(float deltaT)
{
    HandleAsyncs();

    for (int i = 0; i < 4; i++)
    {
        if (mLoadQueue[i] != NULL)
        {
            mLoadQueue[i]->mTime += deltaT;
        }
    }

    if (nlTaskManager::m_pInstance->m_CurrState == 0x100)
    {
        float animTime = mCamera.m_fAnimationTime;
        if (mCamera.m_pActiveCameraData != NULL)
        {
            mCamera.ManualUpdate(deltaT);
        }

        for (int i = 0; i < 4; i++)
        {
            if (mPlaying[i] == NULL)
                continue;

            mPlaying[i]->Update(deltaT);

            float duration;
            if (mCamera.m_pActiveCameraData != NULL)
            {
                duration = (float)(mCamera.m_pActiveCameraData->m_uKeyCount - 1) / 30.0f;
            }
            else
            {
                duration = 0.0f;
            }

            mPlaying[i]->UpdateTriggers(animTime, mCamera.m_fAnimationTime, duration);

            mCamera.m_OffsetPos = mPlaying[i]->Offset();
        }
    }
}

/**
 * Offset/Address/Size: 0x2CCC | 0x801179A8 | size: 0xD8
 */
void NisPlayer::Reset()
{
    if (!mActive)
    {
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        delete mPlaying[i];
        delete mLoaded[i];
        mPlaying[i] = NULL;
        mLoaded[i] = NULL;
        mLoadQueue[i] = NULL;
        mAsyncStarted[i] = false;
    }

    mActive = false;
    mLoadingFromBack = false;
    mUsedFromFront = 0;
    mUsedFromBack = 0x70800;
    mCamera.UnselectCameraAnimation();
    cCameraManager::Remove(mCamera);
}

/**
 * Offset/Address/Size: 0x2AD4 | 0x801177B0 | size: 0x1F8
 */
bool NisPlayer::Play()
{
    int i;
    mActive = true;

    for (i = 0; i < 4; i++)
    {
        if (mLoadQueue[i] != NULL)
        {
            if (mLoadQueue[i]->mTime > 0.0f)
            {
                mLoadQueue[i] = NULL;
            }
            else
            {
                return false;
            }
        }
    }

    for (int j = 0; j < 4; j++)
    {
        delete mPlaying[j];
        mPlaying[j] = mLoaded[j];
        mLoaded[j] = NULL;
    }

    EmissionManager::Destroy(reinterpret_cast<unsigned long>(this), NULL);

    if (nlDLRingGetStart<cBaseCamera>(cCameraManager::m_cameraStack) != (cBaseCamera*)&mCamera)
    {
        cCameraManager::Remove(mCamera);
        cCameraManager::PushCamera(&mCamera);
    }

    for (i = 0; i < 4; i++)
    {
        if (mPlaying[i] != NULL)
        {
            if (mPlaying[i]->SelectRandomCamera(mCamera))
            {
                break;
            }
        }
    }

    if (mLoadingFromBack)
    {
        mLoadingFromBack = false;
        mUsedFromFront = 0;
    }
    else
    {
        mLoadingFromBack = true;
        mUsedFromBack = 0x70800;
    }

    return true;
}

static inline void HideAllActors()
{
    RenderSnapshot& snapshot = ReplayManager::Instance()->GetMutableRenderSnapshot();

    for (int i = 0; i < 150; i++)
    {
        snapshot.mPowerups[i].mVisible = false;
    }

    for (int i = 0; i < 10; i++)
    {
        snapshot.mCharacters[i].mVisible = false;
    }
    snapshot.mBall.mVisible = false;
}

/**
 * Offset/Address/Size: 0x28F4 | 0x801175D0 | size: 0x1E0
 */
void NisPlayer::Render() const
{
    int i;
    nlTaskManager* taskManager = nlTaskManager::m_pInstance;
    unsigned long currState = taskManager->m_CurrState;

    if (currState != 0x100 || ((taskManager->m_PrevState == 0x100) && (currState != 1)))
    {
        return;
    }

    HideAllActors();

    for (i = 0; i < 4; i++)
    {
        if (mPlaying[i] != NULL)
        {
            mPlaying[i]->Render();
        }
    }
}

/**
 * Offset/Address/Size: 0x27E8 | 0x801174C4 | size: 0x10C
 */
void NisPlayer::Load(char* buffer, unsigned int size, NisHeader& nisHeader)
{
    if (!mActive)
        return;

    for (int i = 0; i < 4; i++)
    {
        if (mLoaded[i] != NULL)
            continue;

        int j;
        for (j = 0; j < 4; j++)
        {
            if (&nisHeader == mLoadQueue[j])
            {
                mLoadQueue[j] = NULL;
                mAsyncStarted[j] = false;
                break;
            }
        }

        if (j >= 4)
            continue;

        Nis* nis = new (nlMalloc(0x740, 8, false)) Nis(nisHeader, buffer, size);
        mLoaded[i] = nis;
        LoadTriggers(*mLoaded[i]);
        return;
    }
}

/**
 * Offset/Address/Size: 0x1AE0 | 0x801167BC | size: 0xD08
 */
void NisPlayer::LoadTriggers(Nis&)
{
    FORCE_DONT_INLINE;
}

#pragma dont_inline on
/**
 * Offset/Address/Size: 0x1A48 | 0x80116724 | size: 0x98
 * TODO: 99.21% match - remaining `i` diffs are the local-static `instance` / `init$` relocation immediates.
 */
void NisPlayer::AsyncLoad(nlFile* file, void* buffer, unsigned int size, unsigned long param)
{
    if (file != NULL)
    {
        nlClose(file);
    }

    static NisPlayer instance;
    instance.Load((char*)buffer - size, size, *(NisHeader*)param);
}
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0xFAC | 0x80115C88 | size: 0xA9C
 */
void NisPlayer::GetTargetFilter(NisTarget, NisWinnerType) const
{
}

/**
 * Offset/Address/Size: 0x610 | 0x801152EC | size: 0x99C
 */
void NisPlayer::Load(const char*, NisTarget, NisUseStadiumOffset, NisUseFilter, NisWinnerType)
{
}

/**
 * Offset/Address/Size: 0x548 | 0x80115224 | size: 0xC8
 */
void NisPlayer::PlayCharacterDirection()
{
    Event* event = g_pEventManager->CreateValidEvent(7, 0x20);
    CharacterDirectionData* pData = new (&event->m_data) CharacterDirectionData();
    pData->home = &mBeginPositions[0];
    pData->away = &mBeginPositions[4];
    for (int i = 0; i < 10; i++)
    {
        mBeginPositions[i].f.x = nlRandomf(-20.0f, 20.0f, &nlDefaultSeed);
        mBeginPositions[i].f.y = nlRandomf(-50.0f, 10.0f, &nlDefaultSeed);
        mBeginPositions[i].f.z = 0.0f;
    }
}

/**
 * Offset/Address/Size: 0x370 | 0x8011504C | size: 0x1D8
 */
void NisPlayer::EventHandler(Event* event)
{
    if (g_pGame == NULL)
    {
        return;
    }
    if (g_pGame->m_eGameState == 3)
    {
        return;
    }

    if (event->m_uEventID == 5)
    {
        GoalScoredData* gsd;
        if ((s32)event->m_data.GetID() == -1)
        {
            nlPrintf("NisPlayer EventHandler invalid data\n");
            gsd = NULL;
        }
        else if ((s32)event->m_data.GetID() != 0x18A)
        {
            nlPrintf("NisPlayer EventHandler unexpected data\n");
            gsd = NULL;
        }
        else
        {
            gsd = (GoalScoredData*)&event->m_data;
        }

        if (gsd != NULL)
        {
            if (gsd->uGoalType == 6)
            {
                g_ForceDoubleBallTransition = 1;
            }

            if (!gsd->pLastTouch[gsd->uTeamIndex]->IsCaptain())
            {
                mGoalScorerCharIndex = GetCharacterIndex(gsd->pLastTouch[gsd->uTeamIndex]);
            }
        }
    }

    if (event->m_uEventID == 0xF)
    {
        GoalieSaveData* gsd;
        if ((s32)event->m_data.GetID() == -1)
        {
            nlPrintf("NisPlayer EventHandler invalid data\n");
            gsd = NULL;
        }
        else if ((s32)event->m_data.GetID() != 0x13C)
        {
            nlPrintf("NisPlayer EventHandler unexpected data\n");
            gsd = NULL;
        }
        else
        {
            gsd = (GoalieSaveData*)&event->m_data;
        }

        if (gsd != NULL)
        {
            if (gsd->pGoalie == g_pCharacters[8])
            {
                mWinnerSide[1] = 0;
            }
            else
            {
                mWinnerSide[1] = 1;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x1F4 | 0x80114ED0 | size: 0x17C
 */
int NisPlayer::TargetToIndex(NisTarget target, int idx, NisWinnerType wt) const
{
    if (target == NIS_TARGET_HOME_CAPTAIN)
    {
        return idx;
    }
    if (target == NIS_TARGET_AWAY_CAPTAIN)
    {
        return idx + 4;
    }
    if (target == NIS_TARGET_HOME_SIDEKICK)
    {
        return idx + 1;
    }
    if (target == NIS_TARGET_AWAY_SIDEKICK)
    {
        return idx + 5;
    }
    if (target == NIS_TARGET_HOME_GOALIE)
    {
        return 8;
    }
    if (target == NIS_TARGET_AWAY_GOALIE)
    {
        return 9;
    }
    if (target == NIS_TARGET_LOSER_SIDEKICK)
    {
        if (mWinnerSide[wt] == 0)
        {
            return idx + 5;
        }
        return idx + 1;
    }
    if (target == NIS_TARGET_WINNER_SIDEKICK)
    {
        if (mWinnerSide[wt] == 0)
        {
            return idx + 1;
        }
        return idx + 5;
    }
    if (target == NIS_TARGET_LOSER_GOALIE)
    {
        return (mWinnerSide[wt] == 0) ? 9 : 8;
    }
    if (target == NIS_TARGET_WINNER_GOALIE)
    {
        return (mWinnerSide[wt] == 0) ? 9 : 8;
    }
    if (target == NIS_TARGET_WINNER_CAPTAIN)
    {
        return (mWinnerSide[wt] == 0) ? 0 : 4;
    }
    if (target == NIS_TARGET_LOSER_CAPTAIN)
    {
        return (mWinnerSide[wt] == 0) ? 4 : 0;
    }
    return (target == NIS_TARGET_NONE) ? idx : 0;
}

/**
 * Offset/Address/Size: 0xC4 | 0x80114DA0 | size: 0x130
 */
bool NisPlayer::IsMirrored(NisTarget target, const char* name, NisWinnerType winnerType) const
{
    if (target == NIS_TARGET_LOSER_CAPTAIN || target == NIS_TARGET_WINNER_CAPTAIN || target == NIS_TARGET_WINNER_SIDEKICK || target == NIS_TARGET_LOSER_GOALIE || target == NIS_TARGET_WINNER_GOALIE || target == NIS_TARGET_LOSER_SIDEKICK)
    {
        bool mirrored = true;
        if (strstr(name, "_goal_") == NULL && strstr(name, "goalie_loser") == NULL)
        {
            mirrored = false;
        }

        if (mWinnerSide[winnerType] == 0)
        {
            return mirrored;
        }
        else
        {
            return !mirrored;
        }
    }
    else
    {
        if (strstr(name, "home") != NULL || strstr(name, "run_to_center") != NULL)
        {
            if (target == NIS_TARGET_AWAY_CAPTAIN)
            {
                return true;
            }
            if (target == NIS_TARGET_AWAY_SIDEKICK)
            {
                return true;
            }
            if (target == NIS_TARGET_NONE)
            {
                return true;
            }
        }
        return false;
    }
}

/**
 * Offset/Address/Size: 0xA0 | 0x80114D7C | size: 0x24
 */
void NisPlayer::ResetEffects()
{
    EmissionManager::Destroy(reinterpret_cast<unsigned long>(this), nullptr);
}

/**
 * Offset/Address/Size: 0x74 | 0x80114D50 | size: 0x2C
 */
void NisPlayer::SetExtraNameFilter(const char* filter)
{
    nlStrNCpy(mExtraNameFilter, filter, 128);
}

/**
 * Offset/Address/Size: 0x0 | 0x80114CDC | size: 0x74
 */
NisPlayer::~NisPlayer()
{
}
