#ifndef _PHYSICSBALL_H_
#define _PHYSICSBALL_H_

#include "NL/nlMath.h"
#include "Game/Physics/PhysicsSphere.h"

class PhysicsBall : public PhysicsSphere
{
public:
    PhysicsBall(CollisionSpace*, PhysicsWorld*, float);
    virtual ~PhysicsBall() { };
    virtual bool SetContactInfo(dContact* contact, PhysicsObject* other, bool first);
    virtual void PreUpdate();
    virtual void PreCollide();
    virtual void PostUpdate();
    virtual ContactType Contact(PhysicsObject*, dContact*, int);

    void CalcAngularFromLinearVelocity(nlVector3&);
    void CalcSurfaceVelocity(nlVector3&);
    void SetUseAngularVelocity(bool);
    void ScaleAngularVelocity(float);
    void AddResistanceForces();
    void CloneBall(const PhysicsBall&);

    static float GetBallMaxVelocity();

    /* 0x2c */ nlVector3 m_v3TiltForce;
    /* 0x38 */ bool m_bUseTiltForce;
    /* 0x39 */ bool m_bIsSupportedByGround;
    /* 0x3a */ bool m_bUseAngularVel;
    /* 0x3b */ bool m_bUseMagnusEffect;
    /* 0x3c */ float m_fSpinTimer;
}; // total size: 0x40

#endif // _PHYSICSBALL_H_
