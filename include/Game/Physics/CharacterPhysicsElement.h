#ifndef _CHARACTERPHYSICSELEMENT_H_
#define _CHARACTERPHYSICSELEMENT_H_

#include "NL/nlMath.h"

struct CharacterPhysicsElement
{
    /* 0x00 */ nlMatrix4 matLocalToParent;
    /* 0x40 */ s8 szName[32];
    /* 0x60 */ u32 uHashID;
    /* 0x64 */ s8 szParentName[32];
    /* 0x84 */ u32 uParentHashID;
    /* 0x88 */ u32 uPrimitiveType;
    /* 0x8C */ f32 fWidth;
    /* 0x90 */ f32 fLength;
    /* 0x94 */ f32 fHeight;
    /* 0x98 */ f32 fRadius;
    /* 0x9C */ u32 uReserved;
}; // total size: 0xA0

class CharacterPhysicsData
{
public:
#ifdef CHARACTERTEMPLATE_INLINE_PHYSICSDATA_DTOR
    // CharacterTemplate.cpp owns this class's out-of-line dtor in the target. Defining the
    // body in-class there emits __dt__ and __vt__20CharacterPhysicsData WEAK (the target's
    // scope) instead of GLOBAL at def-position. Guarded per-TU: every other includer takes
    // the #else branch and preprocesses to identical tokens.
    virtual ~CharacterPhysicsData() { delete[] pPhysicsElements; }
#else
    virtual ~CharacterPhysicsData();
#endif

    /* 0x04 */ u32 physicsElementCount;
    /* 0x08 */ CharacterPhysicsElement* pPhysicsElements;
}; // total size: 0xC

bool LoadCharacterPhysicsElements(const char*, CharacterPhysicsData*);

#endif // _CHARACTERPHYSICSELEMENT_H_
