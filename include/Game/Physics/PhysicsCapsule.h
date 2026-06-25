#ifndef _PHYSICSCAPSULE_H_
#define _PHYSICSCAPSULE_H_

#include "Game/Physics/PhysicsObject.h"

#include "Game/Physics/CollisionSpace.h"

class PhysicsCapsule : public PhysicsObject
{
public:
    PhysicsCapsule(CollisionSpace*, PhysicsWorld*, float, float);
    virtual int GetObjectType() const { return 0x2; };
};

#endif // _PHYSICSCAPSULE_H_
