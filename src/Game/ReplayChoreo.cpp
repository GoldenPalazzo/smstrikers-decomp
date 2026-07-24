#include "Game/ReplayChoreo.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Game.h"
#include "Game/Render/NetMesh.h"
#include "Game/Sys/audio.h"
#include "Game/Team.h"
#include "NL/nlDebug.h"
#include "NL/nlFormat.h"
#include "NL/nlTask.h"

struct GoalScoredDataExt
{
    GoalScoredData data;
    int sideOfInterest;
};

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

ReplayChoreo& ReplayChoreo::Instance()
{
    static ReplayChoreo instance;
    return instance;
}

/**
 * Offset/Address/Size: 0x0 | 0x801287C0 | size: 0x3B4
 */
void ReplayChoreo::DoFunctionCall(unsigned int func)
{
    switch (func)
    {
    case 0:
    {
        m_SP--;
        ReplayCameraFocus arg0 = (ReplayCameraFocus)*m_SP;
        AddCameraFocus(arg0);
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

template <typename StringType, typename T1, typename T2, typename T3, typename T4>
void Format(StringType& result, const StringType& format, const T1& value1, const T2& value2, const T3& value3, const T4& value4);

/**
 * Offset/Address/Size: 0xCC8 | 0x80128334 | size: 0x314
 * TODO: 98.10% match - remaining register permutation in this/loop cursor allocation
 */
void ReplayChoreo::LoadScript()
{
    if (mByteCode != 0)
    {
        nlFree(mByteCode);
    }

    unsigned long fileSize = 0;
    mByteCode = nlLoadEntireFile("art/presentation/replay_choreo.byte_code", &fileSize, 0x20, (eAllocType)0);
    LoadByteCode(mByteCode);

    int j;
    int w;
    int t;
    int d;

    for (d = 0; d < 3; d++)
    {
        for (w = 0; w < 3; w++)
        {
            for (t = 0; t < 9; t++)
            {
                mNumScripts[d][w][t] = 0;

                for (j = 0; j < 8; j++)
                {
                    BasicString<char, Detail::TempStringAllocator> name = Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, int>(
                        BasicString<char, Detail::TempStringAllocator>("{0}_{1}_{2}_{3}"),
                        zoneDepthNames[d],
                        zoneInWidthNames[w],
                        replayTypeNames[t],
                        j);

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
 * TODO: 95.47% match - register allocation: stmw r26 vs r27 (data/replayType share r30 in target).
 */
BasicString<char, Detail::TempStringAllocator> ReplayChoreo::CalcAutoReplayScriptName(ReplayType) const
{
    BasicString<char, Detail::TempStringAllocator> format("{0}_{1}_{2}_{3}");

    int zoneDepth = 0;
    int zoneInWidth = 0;
    int goalType = mGoalScoredData.uGoalType;

    f32 fieldDepth = 2.0f * cField::mv3FieldPosition.f.y;
    f32 y = 0.5f * fieldDepth + mGoalScoredData.v3ShotPosition.f.y;

    if (y > 0.66f * fieldDepth)
    {
        zoneInWidth = 2;
    }
    else if (y < 0.33f * fieldDepth)
    {
        zoneInWidth = 1;
    }

    f32 negGoalLineX = -cField::GetGoalLineX((unsigned int)mGoalScoredData.uTeamIndex);

    if ((f32)fabs(negGoalLineX - mGoalScoredData.v3ShotPosition.f.x) < 0.33f * (f32)fabs(negGoalLineX))
    {
        zoneDepth = 1;
    }
    if ((f32)fabs(negGoalLineX - mGoalScoredData.v3ShotPosition.f.x) > 0.66f * (f32)fabs(negGoalLineX))
    {
        zoneDepth = 2;
    }

    while (true)
    {
        if (mNumScripts[zoneDepth][zoneInWidth][goalType] > 0)
            break;

        if (mNumScripts[zoneDepth][0][goalType] > 0)
        {
            zoneInWidth = 0;
            break;
        }
        if (mNumScripts[0][zoneInWidth][goalType] > 0)
        {
            zoneDepth = 0;
            break;
        }
        if (mNumScripts[0][0][goalType] > 0)
        {
            zoneInWidth = 0;
            zoneDepth = 0;
            break;
        }
        goalType = 0;
    }

    if (!mReplay->DidOccurInLastNumSeconds(2, 6.0f))
    {
        return BasicString<char, Detail::TempStringAllocator>("MID_CENTER_OWN_GOAL_0");
    }

    int pick = nlRandom(mNumScripts[zoneDepth][zoneInWidth][goalType], &nlDefaultSeed);
    if (cameraPick > -1)
    {
        pick = cameraPick % mNumScripts[zoneDepth][zoneInWidth][goalType];
    }

    return Format<BasicString<char, Detail::TempStringAllocator>, const char*, const char*, const char*, int>(
        format, zoneDepthNames[zoneDepth], zoneInWidthNames[zoneInWidth], replayTypeNames[goalType], pick);
}

/**
 * Offset/Address/Size: 0x428 | 0x80127A94 | size: 0x270
 * TODO: 99.36% match - remaining diffs are reel counter initialization and temporary string cleanup.
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
        int i;
        int validReels = 0;
        i = validReels;
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
        mCamera.SetSideOfInterest(mHighlights[mHighlightIndex].mReplayPad);

        GoalScoredDataExt* goal = (GoalScoredDataExt*)&mHighlights[mHighlightIndex].mGoalScoredData;
        mGoalScoredData = goal->data;
        mReplayPad = goal->sideOfInterest;

        rt = REPLAY_TYPE_GOAL;
    }
    else
    {
        mReplay->mReelIdx = 0;
    }

    nlStrNCpy(scriptName, CalcAutoReplayScriptName(rt).c_str(), 0x80);
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
 */
void ReplayChoreo::SaveHighlight(ReplayChoreo::HighlightQuality quality)
{
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
        if (mReplay->LockReel(0.0f, idx + 1, quality))
        {
            char* highlight = (char*)this + idx * sizeof(Highlight);

            *(int*)(highlight + 0x230) = quality;
            *(float*)(highlight + 0x234) = mReplayManager->mTime;
            *(GoalScoredData*)(highlight + 0x23C) = mGoalScoredData;
            *(int*)(highlight + 0x260) = mReplayPad;
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
    ReplayManager* volatile* pMgr = &self->mReplayManager;
    self->mReplay = (*pMgr)->mReplay;
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
