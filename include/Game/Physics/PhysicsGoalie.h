#ifndef _PHYSICSGOALIE_H_
#define _PHYSICSGOALIE_H_

#include "NL/nlMath.h"
#include "Game/Physics/PhysicsCharacter.h"

class PhysicsGoalie : public PhysicsCharacter
{
public:
    PhysicsGoalie(float f1, float f2)
        : PhysicsCharacter(f1, f2)
    {
    }
    virtual ~PhysicsGoalie() { };
    virtual void PostUpdate();

    bool SweepTestForBallContact(const nlVector3&, const nlVector3&, const nlVector3&, float, nlVector3&, nlVector3&) const;
    bool SweepTestEveryBone(float, const nlVector3&, const nlVector3&, nlVector3&, nlVector3&) const;
    void CollideGoalieWithPost();
};

#endif // _PHYSICSGOALIE_H_
