#include "Game/Camera/ReplayCamera.h"
#include "types.h"
#include "NL/nlConfig.h"
#include "NL/nlMath.h"
#include "NL/nlTask.h"
#include "Game/Field.h"
#include "Game/ReplayManager.h"
#include "Game/CharacterTemplate.h"
#include "Game/Render/depthoffield.h"

// /**
//  * Offset/Address/Size: 0x18 | 0x801ACB7C | size: 0x8
//  */
// void ReplayCamera::GetFOV() const
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x801ACB74 | size: 0x8
//  */
// void ReplayCamera::GetCameraPosition() const
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x801ACB6C | size: 0x8
//  */
// void ReplayCamera::GetTargetPosition() const
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801ACB64 | size: 0x8
//  */
// void ReplayCamera::GetType()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801AC980 | size: 0x1E4
//  */
// void BasicString<char, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0x1C78 | 0x801AC97C | size: 0x4
 */
void ReplayCamera::UpdateTweakMode()
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x1BC8 | 0x801AC8CC | size: 0xB0
 */
ReplayCamera::ReplayCamera()
{
    mDeltaFov = 0.0f;
    mFov = 50.0f;
    mSideOfInterest = 0;
    mNoDampenForOneUpdate = false;
    mFrozen = false;
    mFocus = 1;
    mCamPos = REPLAY_CAMERA_POSITION_SIDELINE;
    mViewMatrix.SetIdentity();
    nlVec3Set(mPosition, 0.0f, 0.0f, 2.0f);
    nlVec3Set(mLookAt, 0.0f, 0.0f, 1.0f);
}

// /**
//  * Offset/Address/Size: 0x1B88 | 0x801AC88C | size: 0x40
//  */
const nlMatrix4& ReplayCamera::GetViewMatrix() const
{
    glMatrixLookAt(*(nlMatrix4*)&mViewMatrix, mPosition, mLookAt, mUpVector);
    return mViewMatrix;
};

/**
 * Offset/Address/Size: 0x1B84 | 0x801AC888 | size: 0x4
 */
void ReplayCamera::Update(float)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x16E8 | 0x801AC3EC | size: 0x49C
 */
void ReplayCamera::ManualUpdate(float deltaT)
{
    if (!mFrozen)
    {
        nlVector3 lookAt = { 0.0f, 0.0f, 0.0f };
        int numFocusPoints = 0;

        if (ReplayManager::Instance()->mRender == NULL)
        {
            return;
        }

        RenderSnapshot* render = ReplayManager::Instance()->mRender;

        if (mFocus & 0x1)
        {
            numFocusPoints++;
            nlVec3Add(lookAt, lookAt, render->mBall.mPosition);
        }

        if (mFocus & 0x8)
        {
            DrawableCharacter* goalie = &render->mCharacters[GetGoalieIndex(mSideOfInterest)];
            nlVector3 bip01Pos = goalie->mPosition;
            bip01Pos.f.z += goalie->mHeight;
            nlVec3Add(lookAt, lookAt, bip01Pos);
        }

        if (mFocus & 0x2)
        {
            DrawableCharacter* player = render->mBall.IndexToPlayer(render->mBall.mOwnerIndex);
            if (player != NULL)
            {
                numFocusPoints++;
                nlVector3 bip01Pos = player->mPosition;
                bip01Pos.f.z += player->mHeight;
                nlVec3Add(lookAt, lookAt, bip01Pos);
            }
            else
            {
                player = render->mBall.IndexToPlayer(render->mBall.mPrevOwnerIndex);
                if (player != NULL)
                {
                    numFocusPoints++;
                    nlVector3 bip01Pos = player->mPosition;
                    bip01Pos.f.z += player->mHeight;
                    nlVec3Add(lookAt, lookAt, bip01Pos);
                }
            }
        }

        if (mFocus & 0x4)
        {
            nlVector3 netPos = { 0.0f, 0.0f, 0.0f };
            netPos.f.x = cField::GetGoalLineX(mSideOfInterest == 0 ? -1.0f : 1.0f);
            numFocusPoints++;
            nlVec3Add(lookAt, lookAt, netPos);
        }

        if (numFocusPoints != 0)
        {
            float invCount = 1.0f / (float)numFocusPoints;
            _nlVec3Scale(lookAt, invCount);
        }

        nlVector3 position = GetPosition(mCamPos, mSideOfInterest == 0 ? -1.0f : 1.0f);

        if (mNoDampenForOneUpdate)
        {
            mLookAt = lookAt;
            mPosition = position;
            mNoDampenForOneUpdate = false;
        }
        else
        {
            mLookAt.f.x = 0.85f * mLookAt.f.x + 0.15f * lookAt.f.x;
            mLookAt.f.y = 0.85f * mLookAt.f.y + 0.15f * lookAt.f.y;
            mLookAt.f.z = 0.85f * mLookAt.f.z + 0.15f * lookAt.f.z;

            mPosition.f.x = 0.9f * mPosition.f.x + 0.1f * position.f.x;
            mPosition.f.y = 0.9f * mPosition.f.y + 0.1f * position.f.y;
            mPosition.f.z = 0.9f * mPosition.f.z + 0.1f * position.f.z;
        }

        mFov -= deltaT * mDeltaFov;
        if (mFov < 10.0f)
        {
            mFov = 10.0f;
        }
        if (mFov > 120.0f)
        {
            mFov = 120.0f;
        }
    }

    if (nlTaskManager::m_pInstance->m_CurrState == 0x10)
    {
        nlVector3 dir;
        nlVec3Sub(dir, mPosition, mLookAt);
        DepthOfFieldManager::instance.m_fDistanceFromCamera = 4.0f + nlSqrt(dir.GetLengthSq3D(), true);
    }
}

/**
 * Offset/Address/Size: 0x16E0 | 0x801AC3E4 | size: 0x8
 */
void ReplayCamera::SetSideOfInterest(int sideOfInterest)
{
    mSideOfInterest = sideOfInterest;
}

/**
 * Offset/Address/Size: 0x1668 | 0x801AC36C | size: 0x78
 */
void ReplayCamera::CutTo(ReplayCameraPosition camPos)
{
    mFrozen = false;
    mNoDampenForOneUpdate = true;
    mCamPos = camPos;
    mPosition = GetPosition(mCamPos, -1.0f);
    mFov = GetFov(mCamPos);
}

/**
 * Offset/Address/Size: 0x11AC | 0x801ABEB0 | size: 0x4BC
 * TODO: 95.69% match - r29/r31 register swap for position parameter, compiler pass ordering difference
 */
float ReplayCamera::GetFov(ReplayCameraPosition position) const
{
    switch (position)
    {
    case REPLAY_CAMERA_POSITION_INSIDE_NET:
    {
        Config& g = Config::Global();
        return GetConfigFloat(g, "replay/camera_inside_net_fov", 50.0f);
    }
    case REPLAY_CAMERA_POSITION_HIGH_UP:
    {
        Config& g = Config::Global();
        return GetConfigFloat(g, "replay/camera_high_up_fov", 50.0f);
    }
    default:
        if (position >= REPLAY_CAMERA_POSITION_GENERIC_0 && position <= REPLAY_CAMERA_POSITION_GENERIC_LAST)
        {
            BasicString<char, Detail::TempStringAllocator> prefix("replay/camera_");
            {
                BasicString<char, Detail::TempStringAllocator> formatStr("generic_{0}_fov");
                int idx = position - REPLAY_CAMERA_POSITION_GENERIC_0;
                prefix.AppendInPlace(Format(formatStr, idx));
            }
            float fov = GetConfigFloat(Config::Global(), prefix.c_str(), 50.0f);
            return fov;
        }
        return 27.0f;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801AAD04 | size: 0x11AC
 * TODO: 94.07% match - f23 callee-saved register not allocated, causing all float register numbers to shift by 2 and stack offsets by 0x10. Zero structural differences.
 */
nlVector3 ReplayCamera::GetPosition(ReplayCameraPosition position, float direction) const
{
    nlVector3 result = { 0.0f, 0.0f, 0.0f };
    float goalLineX = cField::GetGoalLineX(direction);
    float sidelineY = cField::GetSidelineY(1);

    switch (position)
    {
    case REPLAY_CAMERA_POSITION_INSIDE_NET:
    {
        float x = GetConfigFloat(Config::Global(), "replay/camera_inside_net_x", 7.0f);
        float y = GetConfigFloat(Config::Global(), "replay/camera_inside_net_y", 8.0f);
        float z = GetConfigFloat(Config::Global(), "replay/camera_inside_net_z", 2.0f);
        result.f.x = cField::GetGoalLineX(direction) + direction * x;
        result.f.y = y;
        result.f.z = z;
        break;
    }
    case REPLAY_CAMERA_POSITION_SIDELINE:
    {
        RenderSnapshot* render = ReplayManager::Instance()->mRender;
        result = render->mBall.mPosition;
        result.f.x *= 0.8f;
        result.f.y = cField::GetSidelineY(0) + (-5.0f);
        result.f.z = 2.0f;
        break;
    }
    case REPLAY_CAMERA_POSITION_BALL_TO_GOAL:
    {
        RenderSnapshot* render = ReplayManager::Instance()->mRender;
        nlVector3 goalPos = { 0.0f, 0.0f, 0.0f };
        float goalX = 30.0f * direction + goalLineX;
        nlVector3 ballPos = render->mBall.mPosition;

        float dy = goalPos.f.y - ballPos.f.y;
        float dx = goalX - ballPos.f.x;
        float dz = goalPos.f.z - ballPos.f.z;
        goalPos.f.x = goalX;

        float normDx, normDy, normDz;
        {
            float invLen = nlRecipSqrt(dx * dx + dy * dy + dz * dz, false);
            normDz = invLen * dz;
            normDy = invLen * dy;
            normDx = invLen * dx;
        }

        float behindDist = GetConfigFloat(Config::Global(), "replay/camera_ball_to_goal_behind_dist", 16.0f);
        result.f.x = ballPos.f.x + (-behindDist) * normDx;
        result.f.y = ballPos.f.y + (-behindDist) * normDy;
        result.f.z = ballPos.f.z + (-behindDist) * normDz;

        float minHeight = GetConfigFloat(Config::Global(), "replay/camera_ball_to_goal_min_height", 3.0f);
        if (result.f.z < minHeight)
        {
            result.f.z = minHeight;
        }

        float minDistToGoal = GetConfigFloat(Config::Global(), "replay/camera_ball_to_goal_min_dist_to_goal", 8.0f);
        if ((float)fabs(goalPos.f.x - result.f.x) < minDistToGoal)
        {
            result.f.x = goalPos.f.x - direction * minDistToGoal;
        }
        break;
    }
    case REPLAY_CAMERA_POSITION_HIGH_UP:
    {
        float highX = GetConfigFloat(Config::Global(), "replay/camera_high_up_x", -6.0f);
        float highY = GetConfigFloat(Config::Global(), "replay/camera_high_up_y", 0.0f);
        float highZ = GetConfigFloat(Config::Global(), "replay/camera_high_up_z", 8.0f);
        float minDistBehind = GetConfigFloat(Config::Global(), "replay/camera_high_up_min_dist_behind", 8.0f);

        float side = (mSideOfInterest == 0) ? -1.0f : 1.0f;
        result.f.x = highX * side;
        result.f.y = highY;
        result.f.z = highZ;

        if ((float)fabs(result.f.x - mLookAt.f.x) < minDistBehind)
        {
            float side2 = (mSideOfInterest == 0) ? -1.0f : 1.0f;
            result.f.x = mLookAt.f.x - minDistBehind * side2;
        }
        break;
    }
    default:
    {
        if (position >= REPLAY_CAMERA_POSITION_GENERIC_0 && position <= REPLAY_CAMERA_POSITION_GENERIC_LAST)
        {
            BasicString<char, Detail::TempStringAllocator> prefix("replay/camera_");
            {
                BasicString<char, Detail::TempStringAllocator> formatStr("generic_{0}_");
                int idx = position - REPLAY_CAMERA_POSITION_GENERIC_0;
                prefix.AppendInPlace(Format(formatStr, idx));
            }

            float side = (mSideOfInterest == 0) ? -1.0f : 1.0f;
            float xVal = GetConfigFloat(Config::Global(), prefix.Append("x").c_str(), 0.0f) * side;
            float yVal = GetConfigFloat(Config::Global(), prefix.Append("y").c_str(), 0.0f);
            float zVal = GetConfigFloat(Config::Global(), prefix.Append("z").c_str(), 0.0f);
            result.f.x = xVal;
            result.f.y = yVal;
            result.f.z = zVal;
        }
        break;
    }
    }

    nlVector3 limits = { 0.0f, 0.0f, 0.0f };
    limits.f.x = GetConfigFloat(Config::Global(), "replay/camera_max_behind_goal_line", 2.0f);
    limits.f.y = GetConfigFloat(Config::Global(), "replay/camera_max_beyond_side_line", 2.0f);
    limits.f.z = GetConfigFloat(Config::Global(), "replay/camera_max_height", 20.0f);

    float minZ = GetConfigFloat(Config::Global(), "replay/camera_min_height", 0.5f);

    if (result.f.z > limits.f.z)
        result.f.z = limits.f.z;
    if (result.f.z < minZ)
        result.f.z = minZ;

    if (result.f.x < -((float)fabs(goalLineX)) - limits.f.x)
        result.f.x = -((float)fabs(goalLineX)) - limits.f.x;
    if (result.f.x > limits.f.x + (float)fabs(goalLineX))
        result.f.x = limits.f.x + (float)fabs(goalLineX);

    if (result.f.y < -sidelineY - limits.f.y)
        result.f.y = -sidelineY - limits.f.y;
    if (result.f.y > sidelineY + limits.f.y)
        result.f.y = sidelineY + limits.f.y;

    return result;
}
