#ifndef _PHYSICSCHARACTER_H_
#define _PHYSICSCHARACTER_H_

#include "Game/Physics/PhysicsCharacterBase.h"

class PhysicsCharacter : public PhysicsCharacterBase
{
public:
    virtual ~PhysicsCharacter() { };
    PhysicsCharacter(float, float);

    virtual int GetObjectType() const { return 0x08; };
    virtual bool SetContactInfo(dContact* contact, PhysicsObject* other, bool first);

    virtual void PostUpdate();
    virtual void PreCollide();
    virtual ContactType Contact(PhysicsObject*, dContact*, int, PhysicsObject*);
    virtual PhysicsBoneID ResolvePhysicsBoneIDFromName(const char*);

    void SetCharacterVelocityXY(const nlVector3&);
    void GetCharacterVelocityXY(nlVector3*);
    void SetCharacterPositionXY(const nlVector3&);
    void GetCharacterPositionXY(nlVector3*);
    void GetRadius(float*);

    /* 0x80 */ unsigned int m_CanCollideWithWall : 1;
    /* 0x80 */ unsigned int m_CanCollideWithBall : 1;
    /* 0x80 */ unsigned int m_HasCollidedWithBall : 1;
    /* 0x80 */ unsigned int m_CanCollidedWithGoalLine : 1;

    /* 0x84 */ int m_nDKBallStuckHackCounter;
    /* 0x88 */ unsigned char m_IsOnTopOfBall;
    /* 0x89 */ unsigned char m_pad89[3];
    /* 0x8C */ class cCharacter* m_pAICharacter;
    /* 0x90 */ class PhysicsColumn* m_pPlayerPlayerColumn;
}; // total size: 0x94

#endif // _PHYSICSCHARACTER_H_
