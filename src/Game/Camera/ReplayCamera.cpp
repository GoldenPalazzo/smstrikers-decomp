#include "Game/Camera/ReplayCamera.h"
#include "types.h"
#include "NL/nlConfig.h"

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
void ReplayCamera::ManualUpdate(float)
{
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
 */
nlVector3 ReplayCamera::GetPosition(ReplayCameraPosition, float) const
{
    FORCE_DONT_INLINE;
    return mPosition;
}
