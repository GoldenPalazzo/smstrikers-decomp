#ifndef _RESETTASK_H_
#define _RESETTASK_H_

#include "NL/nlTask.h"

enum RESET_STATE
{
    RS_RUNNING = 0,
    RS_STARTRESET = 1,
    RS_DOIT = 2,
};

void HandleSoftReset();

class ResetTask : public nlTask
{
public:
    virtual void Run(float);
    virtual const char* GetName() { return "Reset"; };

    static s32 s_ResetMode;
    static RESET_STATE s_ResetState;
    static bool s_AudioInInit;
    static bool s_ResetPressed;
    static bool s_resetPaused;
    static bool s_checkCardRemoved;

    void FSCheckForReset();
};

#endif // _RESETTASK_H_
