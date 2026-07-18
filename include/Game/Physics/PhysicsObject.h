#ifndef _PHYSICSOBJECT_H_
#define _PHYSICSOBJECT_H_

#include "NL/nlMath.h"

#include "ode/objects.h"
#include "ode/collision.h"

#include "Game/Physics/PhysicsWorld.h"

enum ContactType
{
    NO_CONTACT = 0,
    ONE_WAY_CONTACT_THIS = 1,
    ONE_WAY_CONTACT_OTHER = 2,
    TWO_WAY_CONTACT = 3,
};

class PhysicsObject
{
public:
    static float DefaultGravity;

    enum CoordinateType
    {
        WORLD_COORDINATES = 0,
        RELATIVE_TO_PARENT = 1,
    };

    PhysicsObject(PhysicsWorld*);
    virtual ~PhysicsObject();

    void CloneObject(const PhysicsObject&);
    /* 0x0C */ virtual int GetObjectType() const = 0;
    /* 0x10 */ virtual bool SetContactInfo(dContact* contact, PhysicsObject* other, bool first);
    /* 0x14 */ virtual void PreUpdate();
    /* 0x18 */ virtual void PostUpdate();
    /* 0x1c */ virtual void PreCollide() { }
    /* 0x20 */ virtual ContactType Contact(PhysicsObject*, dContact*, int);
    /* 0x24 */ virtual ContactType Contact(PhysicsObject*, dContact*, int, PhysicsObject*);
    void MakeStatic();
    void SetMass(float);
    void Reconnect(dSpaceID);
    dSpaceID Disconnect();
    void EnableCollisions();
    void DisableCollisions();
    void SetWorldMatrix(const nlMatrix4&);
    void SetDefaultContactInfo(dContact*);
    void ZeroForceAccumulators();
    void AddForceAtCentreOfMass(const nlVector3&);
    void GetAngularVelocity(nlVector3* vel) const;
    void SetAngularVelocity(const nlVector3& vel);
    nlVector3& GetLinearVelocity();
    void GetLinearVelocity(nlVector3* vel) const;
    void SetLinearVelocity(const nlVector3& vel);
    void GetRotation(nlMatrix4*) const;
    void SetRotation(const nlMatrix4&);
    void SetRotation(const nlMatrix3&);
    nlVector3& GetPosition();
    void GetPosition(nlVector3*) const;
    void SetPosition(const nlVector3&, CoordinateType);
    void CheckForNaN();
    void SetCategory(unsigned int);
    void SetCollide(unsigned int);
    void SetDefaultCollideBits();

    inline bool IsObjectType(int type) const { return GetObjectType() == type; }

    /* 0x04 */ dBodyID m_bodyID;
    /* 0x08 */ dGeomID m_geomID;
    /* 0x0c */ PhysicsObject* m_parentObject;
    /* 0x10 */ float m_gravity;
    /* 0x14 */ nlVector3 m_position;
    /* 0x20 */ nlVector3 m_linearVelocity;
}; // total size: 0x2C

#endif // _PHYSICSOBJECT_H_
