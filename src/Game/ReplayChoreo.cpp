#include "Game/ReplayChoreo.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Game.h"
#include "Game/Sys/audio.h"
#include "Game/Team.h"
#include "NL/nlDebug.h"
#include "NL/nlTask.h"

struct GoalScoredDataExt
{
    GoalScoredData data;
    int sideOfInterest;
};

namespace Detail
{
class TempStringAllocator;
}

struct BasicStringDataHack
{
    char* mData;
    int mSize;
    int mCapacity;
    int mRefCount;
};

template <typename CharT, typename Allocator>
class BasicString
{
public:
    BasicStringDataHack* m_data;

    ~BasicString()
    {
        if (m_data)
        {
            BasicStringDataHack* data = m_data;
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
        return m_data ? (const CharT*)m_data->mData : &emptyString;
    }
};

class NetMesh
{
public:
    static NetMesh* spPositiveXNetMesh;
    static NetMesh* spNegativeXNetMesh;

    void UpdateUntilRelaxed();
};

template <typename StringType, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
void Format(StringType&, const StringType&, const Arg0&, const Arg1&, const Arg2&, const Arg3&);

unsigned int nlRandom(unsigned int range, unsigned int* seed);
extern unsigned int nlDefaultSeed;
double fabs(double);

namespace
{
char* replayTypeNames[9];
char* zoneDepthNames[3];
char* zoneInWidthNames[3];
char scriptName[128];
int cameraPick;
} // namespace

extern "C" void CalcAutoReplayScriptName__12ReplayChoreoCF10ReplayType(BasicString<char, Detail::TempStringAllocator>*, const ReplayChoreo*, ReplayType);

// /**
//  * Offset/Address/Size: 0xEC4 | 0x80129A38 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<const char*>(const char* const&)
// {
// }

// /**
//  * Offset/Address/Size: 0x150 | 0x80128CC4 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<int>(const int&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80128B74 | size: 0x150
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, int>(const BasicString<char, Detail::TempStringAllocator>&, const char* const&, const char* const&, const char* const&, const int&)
// {
// }

ReplayChoreo& ReplayChoreo::Instance()
{
    static ReplayChoreo instance;
    return instance;
}

/**
 * Offset/Address/Size: 0x0 | 0x801287C0 | size: 0x3B4
 * TODO: 99.96% match - single OR operand order swap in case 0 (or r0,r0,r3 vs or r0,r3,r0)
 */
void ReplayChoreo::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
    {
        m_SP--;
        mCamera.mFocus = *m_SP | mCamera.mFocus;
        break;
    }
    case 1:
    {
        m_SP--;
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        f32 duration = *(f32*)m_SP;
        Audio::FadeFilter(tweaks->unk214, tweaks->unk210, duration, 0.0f);
        break;
    }
    case 2:
    {
        m_SP--;
        GameTweaks* tweaks = g_pGame->m_pGameTweaks;
        f32 duration = *(f32*)m_SP;
        Audio::FadeFilter(tweaks->unk210, tweaks->unk214, duration, 0.0f);
        break;
    }
    case 3:
    {
        mCamera.mFrozen = true;
        break;
    }
    case 4:
    {
        m_SP--;
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::PlayWorldSFXbyStr(sfxName, 1.0f, 0.5f, false, true, NULL, NULL, NULL);
        break;
    }
    case 5:
    {
        m_SP--;
        m_SP--;
        f32 vol = *(f32*)m_SP;
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::PlayWorldSFXbyStr(sfxName, vol, 0.5f, false, true, NULL, NULL, NULL);
        break;
    }
    case 6:
    {
        m_SP--;
        f32 timeOffset = *(f32*)m_SP;
        m_SP--;
        mReplayManager->SetCurrentTime(timeOffset + mReplay->TimeOfLastOccurence(*m_SP));
        f32 endTime = mReplayManager->mReplay->EndTime();
        if (endTime - mReplayManager->mTime > 0.033333f)
        {
            mReplayManager->SetCurrentTime(mReplayManager->mReplay->EndTime() - 0.033333f);
        }
        break;
    }
    case 7:
    {
        m_SP--;
        f32 speed = *(f32*)m_SP;
        if (mRunningFor)
        {
            if (0.0f >= mRunForTimeLeft)
            {
                mRunForTimeLeft = 0.0f;
                mRunningFor = false;
                break;
            }
        }
        if (!mRunningFor)
        {
            mRunForTimeLeft = speed;
            mRunningFor = true;
        }
        StopWithUndo();
        break;
    }
    case 8:
    {
        m_SP--;
        f32 timeOffset = *(f32*)m_SP;
        m_SP--;
        u32 eventId = *m_SP;
        if (!IsFinished())
        {
            f32 currentTime = mReplayManager->mTime;
            f32 lastOccurence = mReplay->TimeOfLastOccurence(eventId);
            if (currentTime < timeOffset + lastOccurence)
            {
                StopWithUndo();
            }
        }
        break;
    }
    case 9:
    {
        m_SP--;
        mCamera.CutTo((ReplayCameraPosition)*m_SP);
        break;
    }
    case 10:
    {
        m_SP--;
        mCamera.mFocus = *m_SP;
        break;
    }
    case 11:
    {
        m_SP--;
        f32 fov = *(f32*)m_SP;
        mCamera.mFov = fov;
        mCamera.mDeltaFov = 0.0f;
        break;
    }
    case 12:
    {
        m_SP--;
        f32 speed = *(f32*)m_SP;
        mReplayManager->mSpeed = speed;
        mReplayManager->mSpeedUp = 0.0f;
        break;
    }
    case 13:
    {
        m_SP--;
        f32 deltaFov = *(f32*)m_SP;
        mCamera.mDeltaFov = deltaFov;
        break;
    }
    case 14:
    {
        m_SP--;
        f32 speedUp = *(f32*)m_SP;
        mReplayManager->mSpeedUp = speedUp;
        break;
    }
    case 15:
    {
        m_SP--;
        const char* sfxName = (const char*)*m_SP;
        Audio::StopWorldSFXbyStr(sfxName);
        break;
    }
    default:
        nlBreak();
        break;
    }
}

// /**
//  * Offset/Address/Size: 0x34 | 0x80128764 | size: 0x5C
//  */
// ReplayCamera::~ReplayCamera()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80128730 | size: 0x34
//  */
// void ReplayChoreo::Highlight::Highlight()
// {
// }

/**
 * Offset/Address/Size: 0xCC8 | 0x80128334 | size: 0x314
 */
void ReplayChoreo::LoadScript()
{
    if (mByteCode != 0)
    {
        nlFree(mByteCode);
    }

    unsigned long fileSize = 0;
    mByteCode = nlLoadEntireFile("replay/replay_choreo.byte_code", &fileSize, 0x20, (eAllocType)0);
    LoadByteCode(mByteCode);

    for (int d = 0; d < 3; d++)
    {
        for (int w = 0; w < 3; w++)
        {
            for (int t = 0; t < 9; t++)
            {
                mNumScripts[d][w][t] = 0;

                for (int j = 0; j < 8; j++)
                {
                    BasicString<char, Detail::TempStringAllocator> name;
                    {
                        BasicString<char, Detail::TempStringAllocator> format;
                        BasicStringDataHack* data = (BasicStringDataHack*)nlMalloc(0x10, 8, true);
                        if (data != 0)
                        {
                            const char* src = "{0}_{1}_{2}_{3}";
                            data->mData = 0;
                            data->mSize = 0;
                            data->mCapacity = 0;

                            const char* p = src;
                            while ((signed char)*p++ != 0)
                            {
                                data->mSize++;
                            }

                            data->mSize++;
                            data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
                            data->mCapacity = data->mSize;

                            for (int i = 0; i < data->mSize; i++)
                            {
                                data->mData[i] = *src++;
                            }

                            data->mRefCount = 1;
                        }
                        format.m_data = data;

                        BasicString<char, Detail::TempStringAllocator> temp;
                        void* nameData;
                        Format(temp,
                            format,
                            zoneDepthNames[d],
                            zoneInWidthNames[w],
                            replayTypeNames[t],
                            j);

                        if (temp.m_data != 0)
                        {
                            temp.m_data->mRefCount = temp.m_data->mRefCount + 1;
                            nameData = temp.m_data;
                        }
                        else
                        {
                            nameData = 0;
                        }

                        name.m_data = (BasicStringDataHack*)nameData;
                    }

                    if (FunctionExists(nlStringHash(name.c_str())))
                    {
                        mNumScripts[d][w][t] = mNumScripts[d][w][t] + 1;
                    }
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0xA9C | 0x80128108 | size: 0x22C
 */
void ReplayChoreo::EventHandler(Event* event)
{
    if (!g_pGame)
        return;

    if (event->m_uEventID == 5)
    {
        GoalScoredDataExt* gsd;
        s32 id = event->m_data.GetID();
        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            gsd = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x18A)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                gsd = 0;
            }
            else
            {
                gsd = (GoalScoredDataExt*)&event->m_data;
            }
        }

        if (gsd != 0)
        {
            mGoalScoredData = gsd->data;
            mReplayPad = gsd->sideOfInterest;

            if ((mGoalScoredData.uGoalType == 6 || mGoalScoredData.uGoalType == 2) && mGoalScoredData.uIsHyper)
            {
                mGoalScoredData.uGoalType = 8;
            }

            mCamera.SetSideOfInterest((gsd->data.uTeamIndex + 1) % 2);
        }
    }

    if (event->m_uEventID == 0xF)
    {
        GoalieSaveData* gsd;
        s32 id = event->m_data.GetID();
        if (id == -1)
        {
            nlPrintf("Error: Trying to get event data on event with none!\n");
            gsd = 0;
        }
        else
        {
            id = event->m_data.GetID();
            if (id != 0x13C)
            {
                nlPrintf("Error: GetData() failed! Data types do not match!\n");
                gsd = 0;
            }
            else
            {
                gsd = (GoalieSaveData*)&event->m_data;
            }
        }

        if (gsd != 0)
        {
            if (gsd->isSTS)
                return;

            int teamSide = gsd->pGoalie->m_pTeam->m_nSide;
            mGoalScoredData.uTeamIndex = (teamSide + 1) % 2;
            mGoalScoredData.uGoalType = gsd->saveType;
            mGoalScoredData.pScorer = gsd->pShooter;
            mCamera.SetSideOfInterest(teamSide);
        }
    }
}

/**
 * Offset/Address/Size: 0xA78 | 0x801280E4 | size: 0x24
 */
void ReplayChoreo::Reset()
{
    cCameraManager::Remove(mCamera);
}

/**
 * Offset/Address/Size: 0x698 | 0x80127D04 | size: 0x3E0
 * TODO: 95.2% match - register allocation: stmw r26 vs r27.
 * With -inline deferred, data/goalType share r30 and src/zoneInWidth share r31,
 * but decomp.me uses -inline auto which prevents this register reuse.
 */
extern "C" void CalcAutoReplayScriptName__12ReplayChoreoCF10ReplayType(
    BasicString<char, Detail::TempStringAllocator>* __return, const ReplayChoreo* __this, ReplayType)
{
    BasicStringDataHack* data = (BasicStringDataHack*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        const char* src = "{0}_{1}_{2}_{3}";
        data->mData = 0;
        const char* p = src;
        data->mSize = 0;
        data->mCapacity = 0;

        while ((signed char)*p++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
        data->mCapacity = data->mSize;

        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *src++;
        }

        data->mRefCount = 1;
    }

    BasicString<char, Detail::TempStringAllocator> format;
    format.m_data = data;

    int zoneInWidth = 0;
    int zoneDepth = 0;
    int goalType = __this->mGoalScoredData.uGoalType;

    f32 halfField = 0.5f * cField::mv3FieldPosition.f.y;
    f32 adjustedY = -1.0f * halfField + __this->mGoalScoredData.v3ShotPosition.f.y;

    if (adjustedY > 0.1f * halfField)
    {
        zoneInWidth = 2;
    }
    else if (adjustedY < -0.33f * halfField)
    {
        zoneInWidth = 1;
    }

    f32 negGoalLineX = -cField::GetGoalLineX((unsigned int)__this->mGoalScoredData.uTeamIndex);

    if ((f32)fabs(negGoalLineX - __this->mGoalScoredData.v3ShotPosition.f.x) < -0.33f * (f32)fabs(negGoalLineX))
    {
        zoneDepth = 1;
    }
    if ((f32)fabs(negGoalLineX - __this->mGoalScoredData.v3ShotPosition.f.x) > 0.1f * (f32)fabs(negGoalLineX))
    {
        zoneDepth = 2;
    }

    while (true)
    {
        if (__this->mNumScripts[zoneDepth][zoneInWidth][goalType] > 0)
            break;

        if (__this->mNumScripts[zoneDepth][0][goalType] > 0)
        {
            zoneInWidth = 0;
            break;
        }
        if (__this->mNumScripts[0][zoneInWidth][goalType] > 0)
        {
            zoneDepth = 0;
            break;
        }
        if (__this->mNumScripts[0][0][goalType] > 0)
        {
            zoneDepth = 0;
            zoneInWidth = 0;
            break;
        }
        goalType = 0;
    }

    if (!__this->mReplay->DidOccurInLastNumSeconds(2, 3.0f))
    {
        BasicStringDataHack* d2 = (BasicStringDataHack*)nlMalloc(0x10, 8, true);
        if (d2 != 0)
        {
            const char* src2 = "default_replay";
            d2->mData = 0;
            const char* p2 = src2;
            d2->mSize = 0;
            d2->mCapacity = 0;

            while ((signed char)*p2++ != 0)
            {
                d2->mSize++;
            }

            d2->mSize++;
            d2->mData = (char*)nlMalloc(d2->mSize + 1, 8, true);
            d2->mCapacity = d2->mSize;

            for (int i = 0; i < d2->mSize; i++)
            {
                d2->mData[i] = *src2++;
            }

            d2->mRefCount = 1;
        }
        __return->m_data = d2;
        return;
    }

    int pick = nlRandom(__this->mNumScripts[zoneDepth][zoneInWidth][goalType], &nlDefaultSeed);
    if (cameraPick > -1)
    {
        pick = cameraPick % __this->mNumScripts[zoneDepth][zoneInWidth][goalType];
    }
    Format(*__return, format, zoneDepthNames[zoneDepth], zoneInWidthNames[zoneInWidth], replayTypeNames[goalType], pick);
}

/**
 * Offset/Address/Size: 0x428 | 0x80127A94 | size: 0x270
 * TODO: 95.00% match - remaining diffs are register allocation in highlight init/copy block and scriptName symbol placement.
 */
void ReplayChoreo::StartAutoReplay(ReplayType rt)
{
    mReplayManager = ReplayManager::Instance();
    mReplay = mReplayManager->mReplay;

    NetMesh::spNegativeXNetMesh->UpdateUntilRelaxed();
    NetMesh::spPositiveXNetMesh->UpdateUntilRelaxed();

    if (!cCameraManager::HasCamera(&mCamera))
    {
        mCamera.mNoDampenForOneUpdate = true;
        cCameraManager::PushCamera(&mCamera);
    }

    if (rt == REPLAY_TYPE_HIGHLIGHT)
    {
        mReplayManager = ReplayManager::Instance();
        int i = 0;
        int validReels = i;
        mReplay = mReplayManager->mReplay;

        while (i < 3)
        {
            if (mReplay->IsReelValid(i + 1))
            {
                validReels++;
            }
            i++;
        }

        mNumHighlights = mNumHighlights - 1;
        if (mNumHighlights < 0)
        {
            mNumHighlights = validReels - 1;
        }

        do
        {
            mHighlightIndex = mHighlightIndex + 1;
            mHighlightIndex = mHighlightIndex % 3;
        } while (!mReplay->IsReelValid(mHighlightIndex + 1));

        mReplay->PlayReel(mHighlightIndex + 1);
        int highlightOffset = mHighlightIndex * 0x34;
        int* highlight = (int*)((char*)this + highlightOffset);
        mCamera.SetSideOfInterest(*(int*)((char*)highlight + 0x238));

        int* pHighlight = (int*)((char*)this + mHighlightIndex * 0x34 + 0x23C);
        int* pGoal = (int*)((char*)&mGoalScoredData + 0x4);
        pGoal[0] = pHighlight[1];
        int p8 = pHighlight[2];
        int pC = pHighlight[3];
        pGoal[1] = p8;
        pGoal[2] = pC;
        pGoal[3] = pHighlight[4];
        pGoal[4] = pHighlight[5];
        pGoal[5] = pHighlight[6];
        int p1C = pHighlight[7];
        int p20 = pHighlight[8];
        pGoal[6] = p1C;
        pGoal[7] = p20;
        pGoal[8] = pHighlight[9];

        rt = REPLAY_TYPE_GOAL;
    }
    else
    {
        mReplay->mReelIdx = 0;
    }

    {
        BasicString<char, Detail::TempStringAllocator> name;
        CalcAutoReplayScriptName__12ReplayChoreoCF10ReplayType(&name, this, rt);
        nlStrNCpy(scriptName, name.c_str(), 0x80);
    }

    CallFunction(nlStringHash(scriptName));
}

/**
 * Offset/Address/Size: 0x408 | 0x80127A74 | size: 0x20
 */
void ReplayChoreo::FlushHighlights()
{
    mHighlightIndex = -1;
    mNumHighlights = 0;
    mHighlights[0].mSideOfInterest = 0;
    mHighlights[1].mSideOfInterest = 0;
    mHighlights[2].mSideOfInterest = 0;
}

/**
 * Offset/Address/Size: 0x390 | 0x801279FC | size: 0x78
 */
void ReplayChoreo::Update(float dt)
{
    if (nlTaskManager::m_pInstance->m_CurrState == 0x10)
    {
        if (mRunningFor)
        {
            mRunForTimeLeft -= dt;
        }
        Run();
        mCamera.ManualUpdate(dt);
    }
}

/**
 * Offset/Address/Size: 0x344 | 0x801279B0 | size: 0x4C
 */
bool ReplayChoreo::Done() const
{
    bool result = false;
    if (IsFinished())
    {
        if (nlTaskManager::m_pInstance->m_CurrState == 0x10)
        {
            result = true;
        }
    }
    return result;
}

/**
 * Offset/Address/Size: 0x108 | 0x80127774 | size: 0x23C
 * TODO: 99.86% match - remaining diffs are register swaps when storing GoalScoredData
 * word pairs (+0x8/+0xC and +0x1C/+0x20) into the highlight copy.
 */
void ReplayChoreo::SaveHighlight(ReplayChoreo::HighlightQuality quality)
{
#pragma cplusplus off
    extern unsigned char LockReel__6ReplayFfii(void*, float, int, int);
#pragma cplusplus reset
    extern u8 g_e3_Build;

    mReplayManager = ReplayManager::Instance();
    mReplay = mReplayManager->mReplay;

    if (g_e3_Build != 0)
    {
        return;
    }

    if (nlTaskManager::m_pInstance->m_CurrState != 2)
    {
        return;
    }

    int idx = -1;

    if (quality == HIGHLIGHT_QUALITY_GOAL_EQUALIZER)
    {
        idx = 1;
    }
    else if (quality == HIGHLIGHT_QUALITY_GOAL_INCREASE_DIFF)
    {
        idx = 2;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            if (!mReplay->IsReelValid(i + 1) || mHighlights[i].mSideOfInterest == 0)
            {
                idx = i;
                break;
            }
        }

        if (idx < 0)
        {
            if (mHighlights[0].mSideOfInterest < quality)
            {
                idx = 0;
            }
            else if (mHighlights[1].mSideOfInterest < quality)
            {
                idx = 1;
            }
            else if (mHighlights[2].mSideOfInterest < quality)
            {
                idx = 2;
            }
        }

        if (idx < 0)
        {
            int age = 0;
            for (int i = 0; i < 3; i++)
            {
                int dt = (int)(mReplayManager->mTime - mHighlights[i].mTime);
                if (mHighlights[i].mSideOfInterest == quality && dt > age)
                {
                    idx = i;
                    age = dt;
                }
            }
        }
    }

    if (idx >= 0)
    {
        if (LockReel__6ReplayFfii(mReplay, 0.0f, idx + 1, quality))
        {
            char* highlight = (char*)this + idx * sizeof(Highlight);

            *(int*)(highlight + 0x230) = quality;
            *(float*)(highlight + 0x234) = mReplayManager->mTime;
            *(int*)(highlight + 0x240) = *(int*)((char*)&mGoalScoredData + 4);
            int x214 = *(int*)((char*)&mGoalScoredData + 8);
            int x210 = *(int*)((char*)&mGoalScoredData + 0xC);
            *(int*)(highlight + 0x244) = x210;
            *(int*)(highlight + 0x248) = x214;
            *(int*)(highlight + 0x24C) = *(int*)((char*)&mGoalScoredData + 0x10);
            *(int*)(highlight + 0x250) = *(int*)((char*)&mGoalScoredData + 0x14);
            *(int*)(highlight + 0x254) = *(int*)((char*)&mGoalScoredData + 0x18);
            int x228 = *(int*)((char*)&mGoalScoredData + 0x1C);
            int x224 = *(int*)((char*)&mGoalScoredData + 0x20);
            *(int*)(highlight + 0x258) = x224;
            *(int*)(highlight + 0x25C) = x228;
            *(int*)(highlight + 0x260) = *(int*)((char*)&mGoalScoredData + 0x24);
            *(int*)(highlight + 0x238) = mCamera.mSideOfInterest;
        }
    }
}

/**
 * Offset/Address/Size: 0x8C | 0x801276F8 | size: 0x7C
 */
int ReplayChoreo::NumHighlights() const
{
    ReplayChoreo* self = const_cast<ReplayChoreo*>(this);
    self->mReplayManager = ReplayManager::Instance();
    int count = 0;
    self->mReplay = mReplayManager->mReplay;
    for (int i = 0; i < 3; i++)
    {
        if (mReplay->IsReelValid(i + 1))
        {
            count++;
        }
    }
    return count;
}

/**
 * Offset/Address/Size: 0x0 | 0x8012766C | size: 0x8C
 */
ReplayChoreo::~ReplayChoreo()
{
}
