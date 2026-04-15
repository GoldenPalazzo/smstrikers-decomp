#ifndef _SHADERSKINMESH_H_
#define _SHADERSKINMESH_H_

#include "NL/nlAVLTree.h"
#include "NL/nlMath.h"

#include "Game/Drawable/DrawableObj.h"
#include "Game/GL/GLSkinMesh.h"
#include "Game/PoseAccumulator.h"

class BoneMapList
{
public:
    /* 0x00 */ BoneMapList* m_next;
    /* 0x04 */ nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> > boneMap;

    ~BoneMapList() { };
}; // total size: 0x18

struct GLSkinUserData
{

    /* 0x00 */ int reg;
    /* 0x04 */ float mat[12]; // size 0x30
}; // total size: 0x34

class UserDataBuilder
{
public:
    void AddEntry(const unsigned long&, unsigned long*);

    /* 0x00 */ GLSkinUserData* m_Bone;
    /* 0x04 */ nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> >* m_PoseMatrices;
}; // total size: 0x8

class cPoseAccumulator;

struct GLSkinMeshMatrix
{
    unsigned long boneID; // offset 0x0
    nlMatrix4 matrix;     // offset 0x4
}; // total size: 0x44

class GLSkinMesh
{
public:
    GLSkinMesh()
        : pModel(NULL)
    {
    }
    virtual ~GLSkinMesh() { };
    virtual void ConnectToPose(cPoseAccumulator*) = 0;
    virtual void Pose(cPoseAccumulator*) = 0;
    virtual void PrepareToRender(unsigned long, const nlMatrix4*) = 0;
    virtual nlMatrix4& GetPoseMatrix(unsigned long) = 0;
    virtual void GetPoseMatrices(GLSkinMeshMatrix*) = 0;
    virtual void SetPoseMatrices(int, GLSkinMeshMatrix*) = 0;

    /* 0x04 */ glModel* pModel;
}; // total size: 0x8

class ShaderSkinMesh : public GLSkinMesh
{
public:
    ShaderSkinMesh();
    virtual ~ShaderSkinMesh();
    virtual void ConnectToPose(cPoseAccumulator*);
    virtual void Pose(cPoseAccumulator*);
    virtual void PrepareToRender(unsigned long, const nlMatrix4*);
    virtual nlMatrix4& GetPoseMatrix(unsigned long);
    virtual void GetPoseMatrices(GLSkinMeshMatrix*);
    virtual void SetPoseMatrices(int, GLSkinMeshMatrix*);

    void SetMorphIDs(const unsigned long*);
    void SetBoneMatrix(unsigned long, const nlMatrix4*);
    void AppendSkinPairList(int, const SkinPair*);
    void SetSoftwareVertices(int, const SkinVertex*);
    void AppendStitchingInfo(int, int, int, const unsigned char*);
    void* MakeUserData(nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >*);
    void SetMorphNumDeltas(const unsigned long*);
    void SetMorphDeltas(int, const MorphDelta*);

    void StitchModel();
    void AttachSkinData(unsigned long, const nlMatrix4*);

    /* 0x08 */ nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> > boneMatrices; // offset 0x8, size 0x14
    /* 0x1C */ nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> > poseMatrices; // offset 0x1C, size 0x14
    /* 0x30 */ BoneMapList* boneMaps;
    /* 0x34 */ int numBaseVerts;
    /* 0x38 */ int numSoftwareVerts;
    /* 0x3C */ SkinVertex* softwareVertices;
    /* 0x40 */ nlVector3* tempNormals;
    /* 0x44 */ nlMatrix4* tempMatrices;
    /* 0x48 */ SkinPairList* skinPairs;
    /* 0x4C */ unsigned char** stitchArray;
    /* 0x50 */ int numPackets;
    /* 0x54 */ int numMorphs;
    /* 0x58 */ unsigned long* morphIDs;
    /* 0x5C */ unsigned long* morphNumDeltas;
    /* 0x60 */ MorphDelta* morphData;
    /* 0x64 */ nlVector3* morphBuffer;
    /* 0x68 */ float morphWeights[8];
}; // total size: 0x88

// class nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> >
// {
// public:
//     void ~nlAVLTree();
// };

// class AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long> >, DefaultKeyCompare<unsigned long> >
// {
// public:
//     void InorderWalk<UserDataBuilder>(AVLTreeEntry<unsigned long, unsigned long>*, UserDataBuilder*, void (UserDataBuilder::*)(const unsigned long&, unsigned long*));
//     void Walk<UserDataBuilder>(UserDataBuilder*, void (UserDataBuilder::*)(const unsigned long&, unsigned long*));
// };

// class AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >
// {
// public:
//     void DeleteEntry(AVLTreeEntry<unsigned long, SkinMatrix>*);
//     void AllocateEntry(void*, void*);
//     void CompareKey(void*, AVLTreeNode*);
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CastUp(AVLTreeNode*) const;
//     void PostorderTraversal(AVLTreeEntry<unsigned long, SkinMatrix>*, void (AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, SkinMatrix>*));
//     void DestroyTree(void (AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, SkinMatrix>*));
//     void Clear();
//     void ~AVLTreeBase();
// };

#endif // _SHADERSKINMESH_H_
