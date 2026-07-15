#ifndef _PHYSICSROUNDEDCORNER_H_
#define _PHYSICSROUNDEDCORNER_H_

#include "NL/nlMath.h"

#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/CollisionSpace.h"

class PhysicsRoundedCorner : public PhysicsObject
{
public:
    PhysicsRoundedCorner(CollisionSpace*, const nlVector2&, float, bool, bool);
#ifdef PHYSICSROUNDEDCORNER_DTOR_INLINE
#ifdef PHYSICSROUNDEDCORNER_DTOR_MAIN_TEXT
    DECL_SECT(".text")
#endif
    ~PhysicsRoundedCorner() { }
#else
#ifdef PHYSICSROUNDEDCORNER_DTOR_INLINE_DECL
    inline
#endif
#ifdef PHYSICSROUNDEDCORNER_DTOR_WEAK
        WEAKFUNC
#endif
        ~PhysicsRoundedCorner();
#endif

#ifdef PHYSICSROUNDEDCORNER_GETOBJECTTYPE_OUT_OF_CLASS
    virtual int GetObjectType() const;
#else
    virtual int GetObjectType() const
    {
        return 0x05;
    };
#endif
};

#endif // _PHYSICSROUNDEDCORNER_H_
