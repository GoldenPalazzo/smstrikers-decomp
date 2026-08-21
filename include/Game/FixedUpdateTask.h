#ifndef _FIXEDUPDATETASK_H_
#define _FIXEDUPDATETASK_H_

#include "NL/nlTask.h"

// void 0x8028D334..0x8028D338 | size: 0x4

class FixedUpdateTask : public nlTask
{
public:
    virtual void Run(float dt);
    virtual const char* GetName();

    static void DecrementFrameLock(float);
    static float GetPhysicsUpdateTick();

    static void CallFixedUpdateTasks();
    static void PostPhysicsAITask(float);
    static void PrePhysicsAITask(float);
    static void AIUpdateTask(float);

    FixedUpdateTask();

    static float mAccumulatedDeltaT;
    static float mSimulationTime;
    static float mfFrameLockTime;
    static float mTimeScale;
};

extern bool g_bRunSimAndRenderInLockStep;

#endif // _FIXEDUPDATETASK_H_
