#ifndef _TRANSITIONTASK_H_
#define _TRANSITIONTASK_H_

#include "NL/nlTask.h"
#include "NL/nlMemory.h"
#include "Game/Loader.h"
#include "Game/FE/feManager.h"

void LoadFonts();
// void 0x8028D338..0x8028D33C | size: 0x4;

enum TRANSITION_STATE
{
    eTS_Unknown = 0,
    eTS_InState = 1,
    eTS_Initializing = 2,
    eTS_Destroying = 3,
};

class LoadingManager;
class EventHandler;

class TransitionTask : public nlTask
{
public:
    TransitionTask();

    virtual const char* GetName() { return "Transition Manager"; };
    virtual void Run(float) { };

    void Initialize(LoadingManager&);
    void StateTransition(unsigned int, unsigned int);
    void InitializeGameState();
    void DestroyGameState();
    void InitializeFEState();

    /* 0x18 */ unsigned long long m_GameResourceMark;
    /* 0x20 */ EventHandler* m_pAIHandler;
    /* 0x24 */ EventHandler* m_pGoalieHandler;
    /* 0x28 */ LoadingManager* m_pLoadingManager;
    /* 0x2C */ TRANSITION_STATE m_TransitionState;

    static TransitionTask* sm_pGlobalTask;
}; // total size: 0x30

namespace Detail
{
class SwitchToStartScreenLoader : public Loader
{
public:
    virtual bool StartLoad(LoadingManager*)
    {
        FrontEnd::EnterStartScreen(false);
        return true;
    }
    virtual bool Update() { return false; }
    virtual const char* GetName() { return "SwitchToStartScreenLoader"; }
};
} // namespace Detail

#endif // _TRANSITIONTASK_H_
