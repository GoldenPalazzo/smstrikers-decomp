#ifndef _PHYSICSAIBALL_H_
#define _PHYSICSAIBALL_H_

#include "NL/nlMath.h"
#include "Game/Physics/PhysicsBall.h"

class cBall;

class PhysicsAIBall : public PhysicsBall
{
public:
    PhysicsAIBall(float);
    virtual ~PhysicsAIBall() { };

    /* 0x0C */ virtual int GetObjectType() const
    {
        return 0xf;
    };
    /* 0x14 */ virtual void PreUpdate();
    /* 0x18 */ virtual void PostUpdate();
    /* 0x1c */ virtual void PreCollide();
    /* 0x20 */ virtual ContactType Contact(PhysicsObject*, dContact*, int);

    static bool IsBallOutsideNet(const nlVector3&);
    static bool DidBallJustEnterNet(const nlVector3&, nlVector3);
    void CheckIfBallWentThroughGoalPost();
    void CheckIfBallWentThroughGoalie();

    /* 0x40 */ cBall* m_pAIBall;
    /* 0x44 */ nlVector3 m_unk_0x44;
    /* 0x50 */ u32 m_unk_0x50;
    /* 0x54 */ u32 m_unk_0x54;
    /* 0x58 */ bool m_unk_0x58;
    /* 0x59 */ bool m_unk_0x59;

    // differs from DWARF debug infos..

    // /* 0x44 */ int m_goalieContactFramesAgo;            // ???
    // /* 0x48 */ int m_goalPostContactFramesAgo;          // ???
    // /* 0x4C */ bool mbIsInsideNet;                      // ???
    // /* 0x4D */ bool mbGoalPlaneContact;                 // ???
    // /* 0x4E */ bool mbBallSpeedBelowSweepTestThreshold; // ???
};

#endif // _PHYSICSAIBALL_H_
