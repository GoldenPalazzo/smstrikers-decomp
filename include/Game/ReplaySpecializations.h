#ifndef _REPLAYSPECIALIZATIONS_H_
#define _REPLAYSPECIALIZATIONS_H_

#include "Game/Replay.h"

#include "Game/Drawable/DrawableCharacter.h"
#include "Game/Drawable/DrawableBall.h"
#include "Game/Drawable/DrawableExplosionFragment.h"
#include "Game/Drawable/DrawablePowerup.h"
#include "Game/Render/CrowdManager.h"
#include "Game/Effects/EmissionManager.h"
#include "Game/Drawable/DrawableNetMesh.h"

template <int N>
void Replayable(LoadFrame& frame, bool& value)
{
    if (frame.mInterval == N)
    {
        char temp = 0;
        memcpy(&temp, frame.mStream.mStorage, 1);
        value = (temp != 0);
        char* storage = const_cast<char*>(frame.mStream.mStorage);
        storage += 1;
        frame.mStream.mStorage = storage;
    }
}

template <int N>
void Replayable(SaveFrame& frame, bool& value)
{
    if (frame.mInterval == N)
    {
        char temp = value ? 1 : 0;
        memcpy(frame.mStream.mStorage, &temp, 1);
        frame.mStream.mStorage += 1;
    }
}

template <>
void Replayable<1, SaveFrame, int>(SaveFrame& frame, int& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(int));
            frame.mStream.mStorage += sizeof(int);
        }
    }
}

template <>
void Replayable<1, LoadFrame, int>(LoadFrame& frame, int& value)
{
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(int));
            frame.mStream.mStorage += sizeof(int);
        }
    }
}

// Overloads for nlVector3 - must be declared before generic template
template <int N>
void Replayable(LoadFrame& frame, nlVector3& value)
{
    if (frame.mInterval == N)
    {
        memcpy(&value, frame.mStream.mStorage, sizeof(nlVector3));
        char* storage = const_cast<char*>(frame.mStream.mStorage);
        storage += sizeof(nlVector3);
        frame.mStream.mStorage = storage;
    }
}

template <int N>
void Replayable(SaveFrame& frame, nlVector3& value)
{
    if (frame.mInterval == N)
    {
        memcpy(frame.mStream.mStorage, &value, sizeof(nlVector3));
        frame.mStream.mStorage += sizeof(nlVector3);
    }
}

template <int N, typename FrameType, typename T>
void ReplayableImpl(FrameType& frame, T& manager)
{
    if (frame.mInterval == N)
    {
        if (frame.mInterval == N)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<1, LoadFrame, CrowdManager>(LoadFrame& frame, CrowdManager& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<1, SaveFrame, CrowdManager>(SaveFrame& frame, CrowdManager& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<3, LoadFrame, EmissionManager>(LoadFrame& frame, EmissionManager& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<3, SaveFrame, EmissionManager>(SaveFrame& frame, EmissionManager& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<1, LoadFrame, DrawableNetMesh>(LoadFrame& frame, DrawableNetMesh& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            manager.Replay(frame);
        }
    }
}

template <>
void Replayable<1, SaveFrame, DrawableNetMesh>(SaveFrame& frame, DrawableNetMesh& manager)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            manager.Replay(frame);
        }
    }
}

#endif // _REPLAYSPECIALIZATIONS_H_
