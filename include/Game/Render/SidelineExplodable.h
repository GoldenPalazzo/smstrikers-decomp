#ifndef _SIDELINEEXPLODABLE_H_
#define _SIDELINEEXPLODABLE_H_

#include "NL/nlList.h"

#include "Game/Physics/PhysicsBox.h"
#include "Game/Effects/EmissionController.h"
// #include "Game/Physics/PhysicsObject.h"

#include "Game/Sys/eventman.h"

class cFielder;

class CollisionExplosionFragmentPlayerData : public EventData
{
public:
    virtual u32 GetID();

    /* 0x04 */ cFielder* pPlayer;
    /* 0x08 */ nlVector3 v3CollisionLocation;
    /* 0x14 */ nlVector3 v3CollisionVelocity;
}; // total size: 0x20

class ExplodableCategoryData
{
public:
    bool LoadGeometry();

    /* 0x00 */ const char* mBaseModelName;
    /* 0x04 */ const char* mFragmentModelName;
    /* 0x08 */ const char* mUnexplodedModelName;
    /* 0x0C */ int mNumFragmentModels;
    /* 0x10 */ unsigned long mUnexplodedModel;
    /* 0x14 */ unsigned long mFragmentModelList[40];
    /* 0xB4 */ int mNumStationaryFragments;
    /* 0xB8 */ nlMatrix4 mInitialTransforms[40];
}; // total size: 0xAB8

class ExplosionFragment
{
public:
    ExplosionFragment();
    virtual ~ExplosionFragment();
    virtual void SetStationaryTransform(const nlMatrix4&);
    virtual void GetRotation(nlMatrix4*) const;
    virtual nlVector3& GetPosition() const;

    /* 0x04 */ PhysicsObject* mpPhysicsObject;
    /* 0x08 */ unsigned short mDrawableFragmentID;
    /* 0x0C */ unsigned long mFragmentModelHash;
    /* 0x10 */ float mfRemainingLifespan;
    /* 0x14 */ bool mbIsActive;
    /* 0x15 */ bool mbInfiniteLifespan;
    /* 0x16 */ bool mbIsStationary;
    /* 0x18 */ nlMatrix4* mStationaryTransform;
    /* 0x1C */ EmissionController* mpSmokeEmissionController;

    static float sfFadeOutTime;
}; // total size: 0x20

class SidelineExplodable
{
public:
    SidelineExplodable();
    virtual ~SidelineExplodable();
    virtual ExplodableCategoryData& GetCategoryData() const;
    void Allocate();
    void Update(float);
    void Initialize(int);
    virtual void SetUnexplodedModelVisibility(bool isVisible);
    virtual const nlMatrix4& GetWorldMatrix() const;
    void Explode();
    void DestroyAllActiveFragments(bool);
    void FindExplosionAngleRange(unsigned short&, unsigned short&) const;

    /* 0x4, */ Vector<ExplosionFragment> mExplosionFragments; // offset 0x4, size 0xC
    /* 0x10 */ int mNumActiveFragments;                       // offset 0x10, size 0x4
    /* 0x14 */ unsigned short mMinExplosionAngle;             // offset 0x14, size 0x2
    /* 0x16 */ unsigned short mMaxExplosionAngle;             // offset 0x16, size 0x2
    /* 0x18 */ bool mbAngleRangeInitialized;                  // offset 0x18, size 0x1
    /* 0x1C */ int mNumFragmentModels;                        // offset 0x1C, size 0x4
    /* 0x20 */ bool mbIsMainModelVisible;                     // offset 0x20, size 0x1
    /* 0x24 */ float mfExplodeTime;                           // offset 0x24, size 0x4
    /* 0x28 */ EmissionController* mpAssociatedEffect;        // offset 0x28, size 0x4
}; // total size: 0x2C

class SidelineExplodableNode
{
public:
    /* 0x0 */ SidelineExplodable* mpExplodable;
    /* 0x4 */ SidelineExplodableNode* next;
    static SlotPool<SidelineExplodableNode> sSidelineExplodableNodeSlotPool;
}; // total size: 0x8

void SidelineExplodableTextureLoadCallback(unsigned long);
void EmissionControllerFinished(EmissionController&, ExplosionFragment*);
void UpdateEmissionControllerPosition(EmissionController&, ExplosionFragment*);
// void nlListCountElements<SidelineExplodableNode>(SidelineExplodableNode*);
// void nlListRemoveElement<SidelineExplodableNode>(SidelineExplodableNode**, SidelineExplodableNode*, SidelineExplodableNode**);
// void nlListRemoveStart<DrawableFragmentHandleNode>(DrawableFragmentHandleNode**, DrawableFragmentHandleNode**);
// void nlListAddEnd<DrawableFragmentHandleNode>(DrawableFragmentHandleNode**, DrawableFragmentHandleNode**, DrawableFragmentHandleNode*);
// void Bind<void, void (*)(EmissionController&, ExplosionFragment*), Placeholder<0>, ExplosionFragment*>(void (*)(EmissionController&, ExplosionFragment*), const Placeholder<0>&, ExplosionFragment* const&);

class SidelineExplosionPhysicsObject : public PhysicsBox
{
public:
    virtual ~SidelineExplosionPhysicsObject();
    virtual int GetObjectType() const { return 0x1C; };
    virtual bool SetContactInfo(dContact* contact, PhysicsObject* other, bool first);
    virtual ContactType Contact(PhysicsObject* other, dContact* contact, int what, PhysicsObject* otherObject);
    virtual void PostUpdate();

    ExplosionFragment* mpExplosionFragment;
}; // total size: 0x30

class DrawableFragmentHandleNode
{
public:
    /* 0x0 */ unsigned short mID;
    /* 0x4 */ DrawableFragmentHandleNode* next;
    static SlotPool<DrawableFragmentHandleNode> sDrawableFragmentHandleNodePool;
}; // total size: 0x8

class SidelineExplodableManager
{
public:
    static void CleanUp();
    void Update(float);
    static int GetNumExplodables();
    static void GetVisibilityOfExplodableModels(bool* visibility, int numExplodables);
    static void SetVisibilityOfUnexplodedModels(bool* visibility, int numExplodables);
    static void TriggerExplosions(const nlVector3&, float);
    static void DestroyAllActiveFragments(bool renewExplodables);
    static void RemoveSidelineExplodable(SidelineExplodable*);
    static ExplosionFragment* GetFragmentFromHandle(unsigned short);
    static void AssociateEffectWithNearbyFloatingCamera(EmissionController*);
    static void UnAssociateEffectWithNearbyFloatingCamera(EmissionController* pEmissionController);

    static ExplosionFragment** sFragmentLookupTable;
    static nlList<SidelineExplodableNode> sSidelineExplodableList;
    static SlotPool<SidelineExplodableNode> sSidelineExplodableNodeSlotPool;
    static nlList<DrawableFragmentHandleNode> sUnusedDrawableFragments;
    static bool sbIsInitialized;
};

// class PhysicsBox
// {
// public:
//     void GetObjectType() const;
//     ~PhysicsBox();
// };

// class Function1<void, EmissionController&>
// {
// public:
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, ExplosionFragment*), Placeholder<0>, ExplosionFragment*> >::operator()(EmissionController&);
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, ExplosionFragment*), Placeholder<0>, ExplosionFragment*> >::Clone() const;
//     void FunctorImpl<BindExp2<void, void (*)(EmissionController&, ExplosionFragment*), Placeholder<0>, ExplosionFragment*> >::~FunctorImpl();
// };

// class SlotPool<DrawableFragmentHandleNode>
// {
// public:
//     void ~SlotPool();
// };

// class Vector<ExplosionFragment, DefaultAllocator>
// {
// public:
//     void Vector(int, const char*);
//     void reserve(int);
//     void resize(int);
// };

// class CollisionExplosionFragmentPlayerData
// {
// public:
//     void GetID();
// };

// class SlotPool<SidelineExplodableNode>
// {
// public:
//     void ~SlotPool();
// };

#endif // _SIDELINEEXPLODABLE_H_
