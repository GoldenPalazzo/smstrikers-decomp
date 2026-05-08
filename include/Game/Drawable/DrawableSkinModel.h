#ifndef _DRAWABLESKINMODEL_H_
#define _DRAWABLESKINMODEL_H_

#include "Game/Drawable/DrawableObj.h"

class WorldAnimController;
class GLShadowVolume;

class DrawableSkinModel : public DrawableObject
{
public:
    DrawableSkinModel()
        : m_pAnimController(NULL)
    {
    }

    /* 0x08 */ virtual ~DrawableSkinModel();
    /* 0x10 */ virtual void Draw();
    /* 0x14 */ virtual void SetAnimation(const char*, unsigned long);
    /* 0x18 */ virtual void SetAnimationSpeed(float);
    /* 0x1C */ virtual float GetAnimationTime();
    /* 0x20 */ virtual void SetAnimationTime(float);
    /* 0x24 */ virtual float GetAnimationDuration();

    /* 0x9C */ glModel* m_pModel;
    /* 0xA0 */ GLShadowVolume* m_pShadowVolume;
    /* 0xA4 */ WorldAnimController* m_pAnimController;
};

#endif // _DRAWABLESKINMODEL_H_
