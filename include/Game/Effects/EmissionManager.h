#ifndef _EMISSIONMANAGER_H_
#define _EMISSIONMANAGER_H_

#include "NL/gl/gl.h"
#include "Game/Effects/EmissionController.h"

void fxSetTerrain(unsigned long);
u32 fxGetTerrain();

class LoadFrame;
class SaveFrame;

#include "Game/Effects/EffectsGroup.h"

class EffectsLight
{
public:
    /* 0x00 */ nlVector3 m_v3Position;
    /* 0x0C */ f32 m_fRadius;
    /* 0x10 */ nlColour m_Colour;
}; // size: 0x14

struct LingerMessage : public efNode
{
    /* 0x008 */ char szMessage[256];
    /* 0x108 */ int nLingers;
    /* 0x10C */ int nParticles;
}; // size: 0x110

class EmissionManager
{
protected:
    EmissionManager(bool recording = false)
        : m_bRecording(recording)
    {
    }

public:
    static void KillOldest(int, bool);
    void Replay(SaveFrame&);
    void Replay(LoadFrame&);
    static void AddError(const char*, ...);
    static void ResetLingerers();
    static void Destroy(unsigned long, const EffectsGroup*);
    static void DestroyAll(bool);
    static bool IsPlaying(unsigned long, const EffectsGroup*);
    static void Kill(unsigned long, const EffectsGroup*);
    static bool IsStillAlive(EmissionController*);
    static EmissionController* Create(EffectsGroup* pEffectsGroup, unsigned short id);
    static efList* GetContainer();
    static void Render(); // not sure it's static
    static void AddEffectsLight(const EffectsLight&);
    static EffectsLight* GetLight(int);
    static s32 GetNumLights();
    static void Update(float); // not sure it's static
    static bool Shutdown();
    static bool Startup(eGLView);

    static EmissionManager& InstanceForReplayOnly();

    /* 0x00 */ bool m_bRecording;
};

#endif // _EMISSIONMANAGER_H_
