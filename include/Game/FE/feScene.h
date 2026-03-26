#ifndef _FESCENE_H_
#define _FESCENE_H_

#include "types.h"

#include "NL/gl/gl.h"
#include "Game/FE/fePackage.h"
#include "Game/FE/feResourceManager.h"
#include "Game/FE/feSceneResource.h"

class FEScene
{
public:
    FEScene();
    ~FEScene();

    void AllResourcesLoadedCallback();
    bool LoadPackage(const char*);
    void UnloadPackage();
    void Update(float);
    const nlMatrix4& GetCameraMatrix() const;
    eGLView GetRenderView() const;

    /* 0x00 */ FEPackage* m_pFEPackage;
    /* 0x04 */ unsigned long m_uHashID;
    /* 0x08 */ bool m_bValid;
    /* 0x0C */ nlMatrix4 m_matView;
    /* 0x4C */ unsigned long m_uRenderView;
    /* 0x50 */ FESceneResource m_feSceneResourceHandle; // size 0x20
}; // total size: 0x70

class QueueResourceLoadCallback
{
public:
    void Callback(FEResourceHandle*);
    FEResourceManager* m_resourceManager;
};

class UnloadResourceCallback
{
public:
    void Callback(FEResourceHandle*);
    FEResourceManager* m_resourceManager;
};

// class nlWalkRing<FEResourceHandle, QueueResourceLoadCallback>(FEResourceHandle*, QueueResourceLoadCallback*, void
// (QueueResourceLoadCallback
// {
// public:
//     void *)(FEResourceHandle*));
// };

// class nlWalkRing<FEResourceHandle, UnloadResourceCallback>(FEResourceHandle*, UnloadResourceCallback*, void (UnloadResourceCallback
// {
// public:
//     void *)(FEResourceHandle*));
// };

#endif // _FESCENE_H_
