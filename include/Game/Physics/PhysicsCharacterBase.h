#ifndef _PHYSICSCHARACTERBASE_H_
#define _PHYSICSCHARACTERBASE_H_

#include "Game/Physics/PhysicsCompositeObject.h"
#include "Game/Physics/CollisionSpace.h"
#include "Game/Physics/PhysicsSphere.h"
#include "Game/Physics/PhysicsCapsule.h"

#include "NL/nlList.h"

class cPoseAccumulator;
class CharacterPhysicsData;

enum PhysicsBoneID
{
    PHYSBONE_SINGLE_CAPSULE = 0,
    PHYSBONE_FIELDER_R_ARM = 1,
    PHYSBONE_FIELDER_L_ARM = 2,
    PHYSBONE_FIELDER_R_LEG = 3,
    PHYSBONE_FIELDER_L_LEG = 4,
    PHYSBONE_FIELDER_HEAD = 5,
    PHYSBONE_GOALIE_R_SHOULDER = 6,
    PHYSBONE_GOALIE_R_BICEP = 7,
    PHYSBONE_GOALIE_R_FOREARM = 8,
    PHYSBONE_GOALIE_R_WRIST = 9,
    PHYSBONE_GOALIE_R_HAND = 10,
    PHYSBONE_GOALIE_R_THIGH = 11,
    PHYSBONE_GOALIE_R_THIGHLOWER = 12,
    PHYSBONE_GOALIE_R_CALFUPPER = 13,
    PHYSBONE_GOALIE_R_HEEL = 14,
    PHYSBONE_GOALIE_R_TOE = 15,
    PHYSBONE_GOALIE_L_SHOULDER = 16,
    PHYSBONE_GOALIE_L_BICEP = 17,
    PHYSBONE_GOALIE_L_FOREARM = 18,
    PHYSBONE_GOALIE_L_WRIST = 19,
    PHYSBONE_GOALIE_L_HAND = 20,
    PHYSBONE_GOALIE_L_THIGH = 21,
    PHYSBONE_GOALIE_L_THIGHLOWER = 22,
    PHYSBONE_GOALIE_L_CALFUPPER = 23,
    PHYSBONE_GOALIE_L_HEEL = 24,
    PHYSBONE_GOALIE_L_TOE = 25,
    PHYSBONE_GOALIE_HEAD = 26,
    PHYSBONE_GOALIE_STOMACH = 27,
    NUM_PHYSBONES = 28,
    PHYSBONE_UNKNOWN = 29,
    PHYSBONE_ID_INVALID = 29,
};

class PhysicsCharacterBase : public PhysicsCompositeObject
{
public:
    class PhysicsBoneVolume
    {
    public:
        /* 0x00 */ PhysicsObject* m_pObject;
        /* 0x04 */ unsigned int m_BoneIndex;
        /* 0x08 */ nlMatrix4 m_Transform;
        /* 0x48 */ unsigned int m_TransformHandle;
        /* 0x4C */ nlVector3 m_PrevPosition;
        /* 0x58 */ PhysicsBoneID m_ID;
    }; // total size: 0x5C

    class PhysicsSphereBone : public PhysicsSphere
    {
    public:
        virtual ~PhysicsSphereBone();
        virtual int GetObjectType() const { return 0xD; };

        /* 0x2C */ PhysicsBoneVolume* m_boneVolume;
    }; // total size: 0x30

    class PhysicsCapsuleBone : public PhysicsCapsule
    {
    public:
        virtual ~PhysicsCapsuleBone();
        virtual int GetObjectType() const { return 0xE; };

        /* 0x2C */ PhysicsBoneVolume* m_boneVolume;
    }; // total size: 0x30

    PhysicsCharacterBase(CollisionSpace*, PhysicsWorld*, float);
    virtual ~PhysicsCharacterBase();
    virtual int GetObjectType() const { return 0x08; };
    virtual void UpdatePose(cPoseAccumulator*, float);
    virtual PhysicsBoneID ResolvePhysicsBoneIDFromName(const char*) = 0;

    int GetNumBoneVolumePoints(bool includeEndpoints) const;
    void GetBoneVolumePoints(nlVector3*, bool);
    void SetCharacterPosition(const nlVector3&);
    bool BaseSetContactInfo(dContact*, PhysicsObject*, bool);
    void BasePreCollide();
    void ContainObject(PhysicsObject*);
    void ReleaseObject();
    void SetFacingDirection(unsigned short);
    void AddBoneVolumes(PhysicsWorld*, CollisionSpace*, cPoseAccumulator*, const CharacterPhysicsData*, unsigned long, unsigned long);
    PhysicsBoneID GetBoneIDForSubObject(const PhysicsObject* obj) const;

    /* 0x38 */ unsigned char m_IsSupported;                             // offset 0x38, size 0x1
    /* 0x3C */ float m_CentreOfMassHeight;                              // offset 0x3C, size 0x4
    /* 0x40 */ struct dxJoint* m_CharMoveJoint;                         // offset 0x40, size 0x4
    /* 0x44 */ class PhysicsTransform m_SubObject;                      // offset 0x44, size 0x30
    /* 0x74 */ class nlListContainer<PhysicsBoneVolume*> m_BoneVolumes; // offset 0x74, size 0xC
}; // total size: 0x80

// class ListContainerBase < PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume*, NewAdapter < ListEntry < PhysicsCharacterBase::PhysicsBoneVolume* >>> ::DeleteEntry(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*);
// };

// class nlWalkList < ListEntry < PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume* >, ListContainerBase < PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter < ListEntry < PhysicsCharacterBase::PhysicsBoneVolume* >>>> (ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > >*, void (ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*> > >::*)(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*));
// };

// class nlListAddStart < ListEntry < PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume* >> (ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**);
// };

// class PhysicsCharacterBase : public PhysicsCompositeObject
// {
// public:
//     virtual ~PhysicsCharacterBase();
//     PhysicsCharacterBase(CollisionSpace*, PhysicsWorld*, float);

//     void GetBoneIDForSubObject(const PhysicsObject*) const;
//     void AddBoneVolumes(PhysicsWorld*, CollisionSpace*, cPoseAccumulator*, const CharacterPhysicsData*, unsigned long, unsigned long);
//     virtual void UpdatePose(cPoseAccumulator*, float);
//     void SetFacingDirection(unsigned short);
//     void ReleaseObject();
//     void ContainObject(PhysicsObject*);
//     void BasePreCollide();
//     bool BaseSetContactInfo(dContact*, PhysicsObject*, bool);
//     void SetCharacterPosition(const nlVector3&);
//     void GetBoneVolumePoints(nlVector3*, bool);
//     void GetNumBoneVolumePoints(bool) const;

//     virtual int GetObjectType() const { return 0x08; };
//     // void PhysicsCapsuleBone::GetObjectType() const;
//     // void PhysicsSphereBone::~PhysicsSphereBone();
//     // void PhysicsSphereBone::GetObjectType() const;
//     // void PhysicsCapsuleBone::~PhysicsCapsuleBone();

//     /* 0x38 */ unsigned char m_IsSupported;                             // offset 0x38, size 0x1
//     /* 0x3C */ float m_CentreOfMassHeight;                              // offset 0x3C, size 0x4
//     /* 0x40 */ struct dxJoint* m_CharMoveJoint;                         // offset 0x40, size 0x4
//     /* 0x44 */ class PhysicsTransform m_SubObject;                      // offset 0x44, size 0x30
//     /* 0x74 */ class nlListContainer<PhysicsBoneVolume*> m_BoneVolumes; // offset 0x74, size 0xC
// }; // total size: 0x80

// class ListContainerBase<PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>::DeleteEntry(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*);
// };

// class nlWalkList<ListEntry<PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume*>, ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>>(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>*, void (ListContainerBase<PhysicsCharacterBase::PhysicsBoneVolume*, NewAdapter<ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>>>::*)(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*));
// };

// class nlListAddStart<ListEntry<PhysicsCharacterBase
// {
// public:
//     void PhysicsBoneVolume*>>(ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>*, ListEntry<PhysicsCharacterBase::PhysicsBoneVolume*>**);
// };

#endif // _PHYSICSCHARACTERBASE_H_
