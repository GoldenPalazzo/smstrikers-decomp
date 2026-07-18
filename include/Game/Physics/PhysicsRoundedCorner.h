#ifndef _PHYSICSROUNDEDCORNER_H_
#define _PHYSICSROUNDEDCORNER_H_

#include "NL/nlMath.h"

#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/CollisionSpace.h"

class PhysicsRoundedCorner : public PhysicsObject
{
public:
    PhysicsRoundedCorner(CollisionSpace*, const nlVector2&, float, bool, bool);
    virtual int GetObjectType() const
    {
        return 0x05;
    };
};

#endif // _PHYSICSROUNDEDCORNER_H_
