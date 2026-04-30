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
void Replayable<0, SaveFrame, bool>(SaveFrame& frame, bool& value)
{
    bool temp = value ? true : false;
    memcpy(frame.mStream.mStorage, &temp, sizeof(bool));
    frame.mStream.mStorage += sizeof(bool);
}

template <>
void Replayable<1, SaveFrame, bool>(SaveFrame& frame, bool& value)
{
    if (frame.mInterval == 1)
    {
        char temp = value ? 1 : 0;
        memcpy(frame.mStream.mStorage, &temp, 1);
        frame.mStream.mStorage += 1;
    }
}

template <>
void Replayable<3, SaveFrame, bool>(SaveFrame& frame, bool& value)
{
    if (frame.mInterval == 3)
    {
        bool temp = value ? true : false;
        memcpy(frame.mStream.mStorage, &temp, sizeof(bool));
        frame.mStream.mStorage += sizeof(bool);
    }
}

template <>
void Replayable<0, LoadFrame, bool>(LoadFrame& frame, bool& value)
{
    char temp = 0;
    memcpy(&temp, frame.mStream.mStorage, 1);
    frame.mStream.mStorage += 1;
    value = (temp != 0);
}

template <>
void Replayable<1, LoadFrame, bool>(LoadFrame& frame, bool& value)
{
    if (frame.mInterval == 1)
    {
        char temp = 0;
        memcpy(&temp, frame.mStream.mStorage, 1);
        frame.mStream.mStorage += 1;
        value = (temp != 0);
    }
}

template <>
void Replayable<3, LoadFrame, bool>(LoadFrame& frame, bool& value)
{
    if (frame.mInterval == 3)
    {
        char temp = 0;
        memcpy(&temp, frame.mStream.mStorage, 1);
        frame.mStream.mStorage += 1;
        value = (temp != 0);
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

template <>
void Replayable<1, SaveFrame, nlVector3>(SaveFrame& frame, nlVector3& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(nlVector3));
            frame.mStream.mStorage += sizeof(nlVector3);
        }
    }
}

template <>
void Replayable<1, LoadFrame, nlVector3>(LoadFrame& frame, nlVector3& value)
{
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(nlVector3));
            frame.mStream.mStorage += sizeof(nlVector3);
        }
    }
}

template <>
void Replayable<3, SaveFrame, nlVector3>(SaveFrame& frame, nlVector3& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(nlVector3));
            frame.mStream.mStorage += sizeof(nlVector3);
        }
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

template <>
void Replayable<3, SaveFrame, char>(SaveFrame& frame, char& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(char));
            frame.mStream.mStorage += sizeof(char);
        }
    }
}

template <>
void Replayable<3, LoadFrame, char>(LoadFrame& frame, char& value)
{
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(char));
            frame.mStream.mStorage += sizeof(char);
        }
    }
}

template <>
void Replayable<3, SaveFrame, unsigned short>(SaveFrame& frame, unsigned short& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(unsigned short));
            frame.mStream.mStorage += sizeof(unsigned short);
        }
    }
}

template <>
void Replayable<3, LoadFrame, unsigned short>(LoadFrame& frame, unsigned short& value)
{
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned short));
            frame.mStream.mStorage += sizeof(unsigned short);
        }
    }
}

template <>
void Replayable<3, SaveFrame, float>(SaveFrame& frame, float& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(float));
            frame.mStream.mStorage += sizeof(float);
        }
    }
}

template <>
void Replayable<3, LoadFrame, float>(LoadFrame& frame, float& value)
{
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(float));
            frame.mStream.mStorage += sizeof(float);
        }
    }
}

template <>
void Replayable<3, SaveFrame, unsigned long>(SaveFrame& frame, unsigned long& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(unsigned long));
            frame.mStream.mStorage += sizeof(unsigned long);
        }
    }
}

template <>
void Replayable<3, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value)
{
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned long));
            frame.mStream.mStorage += sizeof(unsigned long);
        }
    }
}

template <>
void Replayable<3, SaveFrame, nlQuaternion>(SaveFrame& frame, nlQuaternion& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(frame.mStream.mStorage, &value, sizeof(nlQuaternion));
            frame.mStream.mStorage += sizeof(nlQuaternion);
        }
    }
}

template <>
void Replayable<3, LoadFrame, nlQuaternion>(LoadFrame& frame, nlQuaternion& value)
{
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(nlQuaternion));
            frame.mStream.mStorage += sizeof(nlQuaternion);
        }
    }
}

template <>
void Replayable<1, LoadFrame, unsigned long>(LoadFrame& frame, unsigned long& value)
{
    if (frame.mInterval == 1)
    {
        if (frame.mInterval == 1)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(unsigned long));
            frame.mStream.mStorage += sizeof(unsigned long);
        }
    }
}

#endif // _REPLAYSPECIALIZATIONS_H_
