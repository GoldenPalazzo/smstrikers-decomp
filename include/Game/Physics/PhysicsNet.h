#ifndef _PHYSICSNET_H_
#define _PHYSICSNET_H_

#include "Game/Physics/PhysicsObject.h"
#include "Game/Physics/CollisionSpace.h"
#include "Game/Physics/PhysicsFinitePlane.h"
#include "Game/Physics/PhysicsCapsule.h"
#include "Game/Render/NetMesh.h"
#include "Game/Net.h"

#include "Game/Sys/eventman.h"

struct CollisionBallGoalpostData : public EventData
{
    static const u32 ID = 0x10e;

    virtual u32 GetID();

    /* 0x4, */ nlVector3 v3CollisionVelocity;
    /* 0x10 */ nlVector3 v3CollisionPosition;
    /* 0x1C */ u32 uTeamIndex;
}; // total size: 0x20

class PhysicsNet
{
public:
    static PhysicsNet* spPhysNetPositiveX;
    static PhysicsNet* spPhysNetNegativeX;
    static float sfPhysicsNetWidth;
    static float sfPhysicsNetHeight;
    static float sfPhysicsNetDepth;
    static bool sbSweepTestEnabled;
    static float sfWallSoftness;
    // static bool sbTestLowerHorizontalGoalpost;

    PhysicsNet(CollisionSpace*, bool);
    virtual ~PhysicsNet();
    virtual int GetObjectType() const { return 0x1b; };

    static bool IsAGoalPost(PhysicsObject*);
    static bool IsAGoalWall(PhysicsObject*);
    bool SweepTestForBallContact(const nlVector3&, const nlVector3&, const nlVector3&, float, nlVector3&, nlVector3&, PhysicsObject**) const;
    static void StaticInit(CollisionSpace*);
    static void StaticDestroy();

    /* 0x4, */ PhysicsFinitePlane* mpBackWall;
    /* 0x8, */ PhysicsFinitePlane* mpSideWall1;
    /* 0xC, */ PhysicsFinitePlane* mpSideWall2;
    /* 0x10 */ PhysicsCapsule* mpSideGoalPost1;
    /* 0x14 */ PhysicsCapsule* mpSideGoalPost2;
    /* 0x18 */ PhysicsCapsule* mpTopGoalPost;
    /* 0x1C */ NetMesh* mpNetMesh;
    /* 0x20 */ cNet* mpNet;
    /* 0x24 */ float errorCorrectionDepth;

}; // total size: 0x28

#endif // _PHYSICSNET_H_
