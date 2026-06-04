#ifndef _GAMERENDERTASK_H_
#define _GAMERENDERTASK_H_

#include "NL/nlTask.h"

class GameRenderTask : public nlTask
{
public:
    virtual void Run(float);
    virtual const char* GetName() { return "Game Render"; }
};

extern bool g_bRenderWorld;

#endif // _GAMERENDERTASK_H_
