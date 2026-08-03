#ifndef _SIDELINEEXPLODABLE_H_
#define _SIDELINEEXPLODABLE_H_

#include "NL/nlList.h"
#include "NL/nlVector.h"

#include "Game/Physics/PhysicsBox.h"
#include "Game/Effects/EmissionController.h"
#include "Game/EventDataTypes.h"

#include "Game/Sys/eventman.h"

class ExplodableCategoryData
{
public:
    ExplodableCategoryData() { }
    ExplodableCategoryData(const char* base, const char* frag, const char* unexp)
    {
        mBaseModelName = base;
        mFragmentModelName = frag;
        mUnexplodedModelName = unexp;
        mNumFragmentModels = 0;
        mUnexplodedModel = 0;
        mNumStationaryFragments = 0;
    }
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
    ExplosionFragment()
    {
        mpPhysicsObject = NULL;
        mDrawableFragmentID = 0xFFFF;
        mbIsActive = false;
        mbInfiniteLifespan = false;
        mbIsStationary = false;
        mStationaryTransform = NULL;
        mpSmokeEmissionController = NULL;
    }
    void SetStationaryTransform(const nlMatrix4& transform)
    {
        if (mStationaryTransform == NULL)
        {
            mStationaryTransform = (nlMatrix4*)nlMalloc(sizeof(nlMatrix4), 8, false);
        }
        *mStationaryTransform = transform;
    }
    void GetRotation(nlMatrix4*) const;
    nlVector3& GetPosition() const;
    void Deactivate();
    virtual ~ExplosionFragment();

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
    virtual ExplodableCategoryData& GetCategoryData() const = 0;
    void Allocate();
    void DeAllocate();
    void Update(float);
    void Initialize(int);
    virtual void SetUnexplodedModelVisibility(bool isVisible) = 0;
    virtual const nlMatrix4& GetWorldMatrix() const = 0;
    void Explode();
    void InitializePhysicsObject(PhysicsObject*, const nlMatrix4&, bool);
    void DestroyAllActiveFragments(bool);
    void FindExplosionAngleRange(unsigned short&, unsigned short&) const;
    EmissionController* GetAssociatedEffect() const { return mpAssociatedEffect; }

    /* 0x4, */ Vector<ExplosionFragment> mExplosionFragments; // offset 0x4, size 0xC
    /* 0x10 */ int mNumActiveFragments;                       // offset 0x10, size 0x4
    /* 0x14 */ mutable unsigned short mMinExplosionAngle;     // offset 0x14, size 0x2
    /* 0x16 */ mutable unsigned short mMaxExplosionAngle;     // offset 0x16, size 0x2
    /* 0x18 */ mutable bool mbAngleRangeInitialized;          // offset 0x18, size 0x1
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

class SidelineExplosionPhysicsObject : public PhysicsBox
{
public:
    SidelineExplosionPhysicsObject(CollisionSpace*, PhysicsWorld*, float, float, float, ExplosionFragment*);
    virtual int GetObjectType() const { return 0x1C; }
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
    static void Update(float);
    static int GetNumExplodables();
    static void GetVisibilityOfExplodableModels(bool* visibility, int numExplodables);
    static void SetVisibilityOfUnexplodedModels(bool* visibility, int numExplodables);
    static void TriggerExplosions(const nlVector3&, float);
    static SidelineExplodable* GetClosestExplodable(const nlVector3&);
    static void DestroyAllActiveFragments(bool renewExplodables);
    static void RemoveSidelineExplodable(SidelineExplodable*);
    static ExplosionFragment* GetFragmentFromHandle(unsigned short);
    static void RegisterFragment(ExplosionFragment*, unsigned short);
    static void AddSidelineExplodable(SidelineExplodable*);
    static unsigned short GetDrawableFragmentFromPool();
    static void ReturnDrawableFragmentToPool(unsigned short);
    static void Initialize();
    static void AssociateEffectWithNearbyFloatingCamera(EmissionController*);
    static void UnAssociateEffectWithNearbyFloatingCamera(EmissionController* pEmissionController);

    static ExplosionFragment** sFragmentLookupTable;
    static nlList<SidelineExplodableNode> sSidelineExplodableList;
    static SlotPool<SidelineExplodableNode> sSidelineExplodableNodeSlotPool;
    static nlList<DrawableFragmentHandleNode> sUnusedDrawableFragments;
    static bool sbIsInitialized;
};

#endif // _SIDELINEEXPLODABLE_H_
