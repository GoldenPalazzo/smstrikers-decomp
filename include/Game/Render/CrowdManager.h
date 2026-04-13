#ifndef _CROWDMANAGER_H_
#define _CROWDMANAGER_H_

#include "types.h"

#include "Game/Replay.h"
#include "Game/Sys/eventman.h"

enum eCrowdState
{
    Crowd_Idle = 0,
    Crowd_Happy = 1,
    Crowd_Excited = 2,
    Crowd_NumStates = 3,
};

// void Replayable<1, LoadFrame, int>(LoadFrame&, int&);
// void Replayable<1, SaveFrame, int>(SaveFrame&, int&);

class CrowdManager
{
public:
    void Initialize();
    void Uninitialize();
    void SetStadium(const char*);
    u32 GetTextureHandle(unsigned long) const;
    void Replay(LoadFrame&);
    void Replay(SaveFrame&);
    void SetState(eCrowdState, bool);
    void Update(float);
    void EventHandler(Event*);
    static void EventHandler(Event* event, void* userData) { ((CrowdManager*)userData)->EventHandler(event); }
    inline void SetFrameConstant();

    /* 0x00 */ eCrowdState m_State;
    /* 0x04 */ float m_fTime;
    /* 0x08 */ float m_fAnimScale;
    /* 0x0C */ s32 m_nCurrentFrame;
    /* 0x10 */ u32 m_CurrentTexture;
    /* 0x14 */ int m_nNumFrames;
    /* 0x18 */ u32 m_TextureHandle;
    /* 0x1C */ char m_szTexture[64];
    /* 0x5C */ char m_szStadium[64];
    /* 0x9C */ u32 m_BundleLoadBase;

    static CrowdManager instance;
}; // total size: 0xA0

#include "Game/ReplaySpecializations.h"

#endif // _CROWDMANAGER_H_
