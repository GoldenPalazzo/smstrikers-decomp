#ifndef _DRAWABLEPOWERUP_H_
#define _DRAWABLEPOWERUP_H_

#include "Game/Replay.h"
#include "Game/Drawable/DrawableObj.h"

class DrawablePowerup
{
public:
    template <typename T>
    void Replay(T&);

    void Blend(const float*, const DrawablePowerup&, const DrawablePowerup&);
    void Render(int) const;
    void Grab(int);

    /* 0x00 */ s8 mType;
    /* 0x04 */ float mScale;
    /* 0x08 */ float mRadius;
    /* 0x0C */ bool mVisible;
    /* 0x10 */ nlVector3 mPosition;
    /* 0x1C */ u16 mOrientation;
}; // total size: 0x20

template <>
void DrawablePowerup::Replay<SaveFrame>(SaveFrame&);

template <>
void DrawablePowerup::Replay<LoadFrame>(LoadFrame&);

#endif // _DRAWABLEPOWERUP_H_
