#ifndef _BEGINFRAMETASK_H_
#define _BEGINFRAMETASK_H_

#include "NL/nlTask.h"
#include "NL/gl/glModel.h"

void DrawGrid(int);
void DrawSafeFrame();
void SetupRenderInfo();
void SetupMatrices();
glModel* cb_ParticleLighting(glModel* pModel);

enum eModelSkinMethod
{
    eModelSkin_Rigid = 0,
    eModelSkin_Blend = 1,
    eModelSkin_Both = 2,
    eModelSkin_Num = 3,
};

class BeginFrameTask : public nlTask
{
public:
    virtual const char* GetName() { return "Begin Frame"; };
    virtual void Run(float dt);

    static u8 s_FramerateLocked;
    static eModelSkinMethod s_GameplaySkin;
    static eModelSkinMethod s_ReplaySkin;
};

// class Config
// {
// public:
//     void TagValuePair::Get<BasicString<char, Detail::TempStringAllocator> >() const;
//     void Get<BasicString<char, Detail::TempStringAllocator> >(const char*, BasicString<char, Detail::TempStringAllocator>);
// };

#endif // _BEGINFRAMETASK_H_
