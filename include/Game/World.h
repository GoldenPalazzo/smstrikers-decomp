#ifndef _WORLD_H_
#define _WORLD_H_

#include "NL/nlMath.h"
#include "Game/TerrainTypes.h"
#include "NL/nlAVLTree.h"

#include "Game/World/worldanim.h"
#include "NL/nlDLListContainer.h"

#include "Game/LightObject.h"

// void DoTranslucency(DrawableObject*);
// void nlListAddStart<ListEntry<LightObject*>>(ListEntry<LightObject*>**, ListEntry<LightObject*>*, ListEntry<LightObject*>**);
// void nlDLRingIsEnd<DLListEntry<WorldAnimController*>>(DLListEntry<WorldAnimController*>*, DLListEntry<WorldAnimController*>*);
// void nlDLRingGetStart<DLListEntry<WorldAnimController*>>(DLListEntry<WorldAnimController*>*);
// void nlStrChr<char>(const char*, char);
// void 0x8028D34C..0x8028D350 | size: 0x4;

class WorldAnimManager;
class DrawableObject;
class DrawableModel;
class nlChunk;
class glModelPacket;
class glModel;
class WorldObjectData;
class Event;
class CharacterPhysicsData;

struct WorldHelperChunkData
{
    /* 0x00 */ char m_szName[60];       // size 0x3C
    /* 0x3C */ unsigned long m_uHashID; // size 0x4
    /* 0x40 */ nlMatrix4 m_worldMatrix; // size 0x40
}; // total size: 0x80

class HelperObject
{
public:
    /* 0x00 */ unsigned long m_uHashID; // size 0x4
    /* 0x04 */ nlMatrix4 m_worldMatrix; // size 0x40
    /* 0x44 */ char m_szName[64];       // size 0x40
}; // total size: 0x84

template <>
AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase();

template <>
nlAVLTree<unsigned long, HelperObject*, DefaultKeyCompare<unsigned long> >::~nlAVLTree();

class World
{
public:
    static u32 m_uCurrentFrameCount;
    static bool sbIsHyperShootToScoreRenderingEnabled;

    World(const char*);
    /* 0x04 */ virtual ~World();

    /* 0x08 */ virtual eTerrain GetTerrainType(const nlVector3&) const = 0;
    /* 0x10 */ virtual u8 HandleObjectCreation(WorldObjectData*);
    /* 0x14 */ virtual void Render();
    /* 0x18 */ virtual void Update(float);
    /* 0x1C */ virtual void UpdateInReplay(float);
    /* 0x20 */ virtual void FixedUpdate(float);
    /* 0x24 */ virtual void HandleEvent(Event*, void*);
    /* 0x28 */ virtual void CreateHelperObjFromChunk(nlChunk*);
    /* 0x2C */ virtual bool DoLoad() = 0;
    /* 0x30 */ virtual bool DoInitialize() = 0;

    int CompareNameToGenericName(const char*, const char*);
    unsigned long GetHashIdForGenericName(const char*) const;
    LightObject* GetShadowLight(const nlVector3&, float);
    bool AddDrawableObject(unsigned long, DrawableObject*);
    HelperObject* FindHelperObject(unsigned long uHashId);
    DrawableObject* FindDrawableObject(unsigned long);
    void HandleCameraSwitch();
    bool IsSphereInFrustum(const nlMatrix4&, float);
    void ExtractFrustumPlanes();
    void* GetCustomSpecularData(glModelPacket*, bool);
    void CreateLightUserData();
    bool LoadObjectData(const char*);
    void AddToHyperSTSDrawables(unsigned long, DrawableModel*);
    bool LoadGeometry(glModel*, unsigned long, bool, bool, unsigned long*, int*, bool);
    bool LoadGeometry(const char*, bool, bool, unsigned long*, int*);
    bool Load(bool forfe);

    /* 0x004 */ WorldAnimManager* m_pWorldAnimManager;
    /* 0x008 */ nlAVLTree<unsigned long, LightObject*, DefaultKeyCompare<unsigned long> > m_lightMap;
    /* 0x01C */ bool m_Locked;
    /* 0x020 */ struct glModel* m_pModels;
    /* 0x024! */ unsigned long m_uNumModels;
    /* 0x028! */ DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*> > > m_animControllerList;
    /* 0x030 */ void* m_pLightData;
    // /* 0x034 */ void* m_pPlayerNISLightData;
    /* 0x038 */ void* m_pIntensityPerm;
    /* 0x03C */ void* m_pIntensityData;
    /* 0x040 */ void* m_pSTSIntensity;
    /* 0x044 */ void* m_pSpecularData;
    /* 0x044 */ nlAVLTree<unsigned long, DrawableObject*, DefaultKeyCompare<unsigned long> > m_drawableMap;
    /* 0x060! */ nlAVLTree<unsigned long, DrawableObject*, DefaultKeyCompare<unsigned long> > m_hyperSTSDrawableMap; // verified
    /* 0x070 */ nlAVLTree<unsigned long, HelperObject*, DefaultKeyCompare<unsigned long> > m_helperMap;
    /* 0x080 */ nlVector4 m_frustumPlane[6];
    /* 0x0E0 */ char m_WorldNamePrefix[64];
    /* 0x120 */ int m_WorldNameLength;
    /* 0x124 */ u32 m_LightRampTexA;
    /* 0x128 */ u32 m_LightRampTexB;
    /* 0x12C */ u32 m_PlayerLightRampTex;
    /* 0x130 */ u32 m_GlobalLightRampSTSTex;
}; // total size: 0x134

// class nlAVLTree<unsigned long, LightObject*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTree();
// };

// class AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void DeleteEntry(AVLTreeEntry<unsigned long, DrawableObject*>*);
//     void ~AVLTreeBase();
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, DrawableObject*>*));
//     void PostorderTraversal(AVLTreeEntry<unsigned long, DrawableObject*>*, void (AVLTreeBase<unsigned long, DrawableObject*, NewAdapter<AVLTreeEntry<unsigned long, DrawableObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, DrawableObject*>*));
//     void CastUp(AVLTreeNode*) const;
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
// };

// class AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~AVLTreeBase();
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LightObject*>*));
//     void PostorderTraversal(AVLTreeEntry<unsigned long, LightObject*>*, void (AVLTreeBase<unsigned long, LightObject*, NewAdapter<AVLTreeEntry<unsigned long, LightObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, LightObject*>*));
//     void CastUp(AVLTreeNode*) const;
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
//     void DeleteEntry(AVLTreeEntry<unsigned long, LightObject*>*);
// };

// class nlAVLTree<unsigned long, DrawableObject*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTree();
// };

// class nlAVLTree<unsigned long, HelperObject*, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~nlAVLTree();
// };

// class AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void ~AVLTreeBase();
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, HelperObject*>*));
//     void PostorderTraversal(AVLTreeEntry<unsigned long, HelperObject*>*, void (AVLTreeBase<unsigned long, HelperObject*, NewAdapter<AVLTreeEntry<unsigned long, HelperObject*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, HelperObject*>*));
//     void CastUp(AVLTreeNode*) const;
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
//     void DeleteEntry(AVLTreeEntry<unsigned long, HelperObject*>*);
// };

// class ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>
// {
// public:
//     void DeleteEntry(ListEntry<LightObject*>*);
// };

// class DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<WorldAnimController*>*);
// };

// class nlWalkList<ListEntry<LightObject*>, ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>>(ListEntry<LightObject*>*, ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>*, void (ListContainerBase<LightObject*, NewAdapter<ListEntry<LightObject*>>>
// {
// public:
//     void *)(ListEntry<LightObject*>*));
// };

// class nlWalkDLRing<DLListEntry<WorldAnimController*>, DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>>(DLListEntry<WorldAnimController*>*, DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>*, void (DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>
// {
// public:
//     void *)(DLListEntry<WorldAnimController*>*));
// };

// class nlWalkRing<DLListEntry<WorldAnimController*>, DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>>(DLListEntry<WorldAnimController*>*, DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>*, void (DLListContainerBase<WorldAnimController*, NewAdapter<DLListEntry<WorldAnimController*>>>
// {
// public:
//     void *)(DLListEntry<WorldAnimController*>*));
// };

#endif // _WORLD_H_
