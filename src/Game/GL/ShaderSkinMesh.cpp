#include "Game/GL/ShaderSkinMesh.h"

#include "Game/GL/GLSkinMesh.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glUserData.h"
#include "NL/nlDLRing.h"
#include "NL/nlMemory.h"
#include "string.h"
#include "types.h"

extern nlVector3 sharedMorphBuffer[];

/**
 * Offset/Address/Size: 0x14C | 0x801E2240 | size: 0x58
 */
BoneMapList::~BoneMapList()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xD8 | 0x801E21CC | size: 0x74
 */
template void nlDeleteRing<BoneMapList>(BoneMapList** head);

// /**
//  * Offset/Address/Size: 0xAC | 0x801E21A0 | size: 0x2C
//  */
// void nlRingAddStart<SkinPairList>(SkinPairList**, SkinPairList*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801E20F4 | size: 0x70
//  */
// void nlDeleteRing<SkinPairList>(SkinPairList**)
// {
// }

// /**
//  * Offset/Address/Size: 0xC9C | 0x801E20D0 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::DeleteEntry(AVLTreeEntry<unsigned long, SkinMatrix>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xBE0 | 0x801E2014 | size: 0xBC
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0xBB4 | 0x801E1FE8 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB88 | 0x801E1FBC | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB80 | 0x801E1FB4 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x428 | 0x801E185C | size: 0x758
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::PostorderTraversal(AVLTreeEntry<unsigned long, SkinMatrix>*, void (AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, SkinMatrix>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x3C4 | 0x801E17F8 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::DestroyTree(void (AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, SkinMatrix>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x36C | 0x801E17A0 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x310 | 0x801E1744 | size: 0x5C
//  */
// void AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<AVLTreeEntry<unsigned long, SkinMatrix> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x44 | 0x801E1478 | size: 0x2CC
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long> >, DefaultKeyCompare<unsigned long> >::InorderWalk<UserDataBuilder>(AVLTreeEntry<unsigned long, unsigned long>*, UserDataBuilder*, void (UserDataBuilder::*)(const unsigned long&, unsigned long*))
// {
// }

/**
 * Offset/Address/Size: 0xD28 | 0x801E136C | size: 0xC8
 */
ShaderSkinMesh::ShaderSkinMesh()
{
    numBaseVerts = 0;
    numMorphs = 0;
    morphNumDeltas = NULL;
    morphData = NULL;
    morphBuffer = NULL;
    boneMaps = NULL;
    morphIDs = NULL;

    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;
    morphWeights[0] = 0.0f;

    numSoftwareVerts = 0;
    softwareVertices = NULL;
    tempNormals = NULL;
    tempMatrices = NULL;
    skinPairs = NULL;
    stitchArray = NULL;
    numPackets = 0;
}

/**
 * Offset/Address/Size: 0xBE4 | 0x801E1228 | size: 0xE4
 */
ShaderSkinMesh::~ShaderSkinMesh()
{
    delete[] morphNumDeltas;
    delete[] morphData;
    delete[] morphIDs;

    if (boneMaps != nullptr)
    {
        void (*deleteFn)(BoneMapList**) = nlDeleteRing<BoneMapList>;
        deleteFn(&boneMaps);
    }

    delete[] tempNormals;
    delete[] tempMatrices;

    if (skinPairs != nullptr)
    {
        nlDeleteRing<SkinPairList>(&skinPairs);
    }

    if (stitchArray != nullptr)
    {
        delete[] stitchArray;
    }
}

/**
 * Offset/Address/Size: 0xB74 | 0x801E11B8 | size: 0x70
 */
void ShaderSkinMesh::SetMorphIDs(const unsigned long* ids)
{
    if (morphIDs != nullptr)
    {
        delete[] morphIDs;
    }
    morphIDs = (unsigned long*)nlMalloc(numMorphs * sizeof(unsigned long), 8, false);
    memcpy(morphIDs, ids, numMorphs * sizeof(unsigned long));
}

/**
 * Offset/Address/Size: 0xB70 | 0x801E11B4 | size: 0x4
 */
void ShaderSkinMesh::ConnectToPose(cPoseAccumulator*)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0xB04 | 0x801E1148 | size: 0x6C
 */
void ShaderSkinMesh::SetBoneMatrix(unsigned long boneID, const nlMatrix4* matrix)
{
    AVLTreeNode* existingNode;
    SkinMatrix skinMatrix;

    skinMatrix.Set(*matrix);

    boneMatrices.AddAVLNode((AVLTreeNode**)&boneMatrices.m_Root, &boneID, &skinMatrix, &existingNode, boneMatrices.m_NumElements);

    if (existingNode == NULL)
    {
        boneMatrices.m_NumElements++;
    }
}

/**
 * Offset/Address/Size: 0xA94 | 0x801E10D8 | size: 0x70
 */
nlMatrix4& ShaderSkinMesh::GetPoseMatrix(unsigned long boneID)
{
    nlMatrix4* foundMatrix = (nlMatrix4*)poseMatrices.FindGet(boneID);
    return *foundMatrix;
}

/**
 * Offset/Address/Size: 0x8E8 | 0x801E0F2C | size: 0x1AC
 */
struct TreeStack
{
    AVLTreeEntry<unsigned long, SkinMatrix>** m_Stack;
    unsigned int m_Count;
};

void ShaderSkinMesh::GetPoseMatrices(GLSkinMeshMatrix* pMatrices)
{
    TreeStack* stack = (TreeStack*)nlMalloc(sizeof(TreeStack), 8, false);
    if (stack != NULL)
    {
        unsigned int numElements = poseMatrices.m_NumElements;
        AVLTreeEntry<unsigned long, SkinMatrix>* node = poseMatrices.m_Root;

        stack->m_Stack = (AVLTreeEntry<unsigned long, SkinMatrix>**)nlMalloc((numElements + 1) * sizeof(AVLTreeEntry<unsigned long, SkinMatrix>*), 8, false);
        stack->m_Count = 0;

        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                stack->m_Stack[stack->m_Count] = node;
                stack->m_Count++;
                node = (AVLTreeEntry<unsigned long, SkinMatrix>*)node->node.left;
            }

            stack->m_Stack[stack->m_Count] = node;
            stack->m_Count++;
        }
    }

    while (stack->m_Count > 0)
    {
        pMatrices->boneID = stack->m_Stack[stack->m_Count - 1]->key;
        memcpy(&pMatrices->matrix, &stack->m_Stack[stack->m_Count - 1]->value, sizeof(pMatrices->matrix));
        pMatrices++;

        stack->m_Count--;

        AVLTreeEntry<unsigned long, SkinMatrix>* right = (AVLTreeEntry<unsigned long, SkinMatrix>*)stack->m_Stack[stack->m_Count]->node.right;
        if (right == NULL)
            continue;

        while (right->node.left != NULL)
        {
            stack->m_Stack[stack->m_Count] = right;
            stack->m_Count++;
            right = (AVLTreeEntry<unsigned long, SkinMatrix>*)right->node.left;
        }

        stack->m_Stack[stack->m_Count] = right;
        stack->m_Count++;
    }

    if (stack != NULL)
    {
        delete[] stack->m_Stack;
        delete stack;
    }
}

/**
 * Offset/Address/Size: 0x76C | 0x801E0DB0 | size: 0x17C
 */
struct EqualFirstCompare
{
    int operator()(unsigned long key1, unsigned long key2) const
    {
        if (key1 == key2)
            return 0;
        if (key1 < key2)
            return -1;
        return 1;
    }
};

inline bool avlFindCheck(AVLTreeEntry<unsigned long, SkinMatrix>* node, unsigned long key, SkinMatrix*& outValue)
{
    while (node != NULL)
    {
        int cmpResult = EqualFirstCompare()(key, node->key);
        if (cmpResult == 0)
        {
            if (&outValue != NULL)
            {
                outValue = &node->value;
            }
            return true;
        }
        else
        {
            if (cmpResult < 0)
            {
                node = (AVLTreeEntry<unsigned long, SkinMatrix>*)node->node.left;
            }
            else
            {
                node = (AVLTreeEntry<unsigned long, SkinMatrix>*)node->node.right;
            }
        }
    }
    return false;
}

void ShaderSkinMesh::SetPoseMatrices(int num, GLSkinMeshMatrix* pMatrices)
{
    AVLTreeNode** poseRoot = (AVLTreeNode**)&poseMatrices.m_Root;
    int i = 0;
    for (; i < num; i++)
    {
        SkinMatrix* foundValue;
        unsigned long boneID = pMatrices[i].boneID;

        if (avlFindCheck(boneMatrices.m_Root, boneID, foundValue))
        {
            SkinMatrix skinMatrix;
            skinMatrix.Set(pMatrices[i].matrix);

            AVLTreeNode* existingNode;
            poseMatrices.AddAVLNode(
                poseRoot,
                (void*)&boneID,
                (void*)&skinMatrix,
                &existingNode,
                poseMatrices.m_NumElements);

            SkinMatrix* dest;
            if (existingNode == NULL)
            {
                poseMatrices.m_NumElements++;
                dest = NULL;
            }
            else
            {
                dest = &((AVLTreeEntry<unsigned long, SkinMatrix>*)existingNode)->value;
            }
            foundValue = dest;
            if (foundValue != NULL)
            {
                *foundValue = skinMatrix;
            }
        }
    }
}

static inline void ClearUserData(ShaderSkinMesh* mesh)
{
    if (mesh->pModel != NULL)
    {
        glModelPacket* pPacket = mesh->pModel->packets;
        glModelPacket* pEndPacket = pPacket + mesh->pModel->numPackets;
        while (pPacket < pEndPacket)
        {
            pPacket->userData = 0;
            pPacket++;
        }
    }
}

static inline void CopySoftwareVerts(ShaderSkinMesh* mesh)
{
    for (int i = 0; i < mesh->numSoftwareVerts; i++)
    {
        mesh->morphBuffer[i].f.x = mesh->softwareVertices[i].position.f.x;
        mesh->morphBuffer[i].f.y = mesh->softwareVertices[i].position.f.y;
        mesh->morphBuffer[i].f.z = mesh->softwareVertices[i].position.f.z;
    }
}

/**
 * Offset/Address/Size: 0x58C | 0x801E0BD0 | size: 0x1E0
 * TODO: 97.3% match - remaining differences are in morph loop register
 *       assignment for morph weight and delta-offset cursors.
 */
void ShaderSkinMesh::PrepareToRender(unsigned long flags, const nlMatrix4* pMatrix)
{
    ClearUserData(this);

    if (numMorphs == 0)
    {
        morphBuffer = NULL;
    }
    else
    {
        morphBuffer = sharedMorphBuffer;
        CopySoftwareVerts(this);

        int m = 0;
        MorphDelta* pDelta = morphData;
        u32 deltaOffset = (u32)m;
        ShaderSkinMesh* weightCursor = this;

        while (m < numMorphs)
        {
            u32 nDeltas = morphNumDeltas[deltaOffset >> 2];
            MorphDelta* end = pDelta + nDeltas;

            if (weightCursor->morphWeights[0] > 0.0f)
            {
                while (pDelta != end)
                {
                    float w = weightCursor->morphWeights[0];
                    nlVector3* dst = &morphBuffer[pDelta->index];
                    float rx = dst->f.x + w * pDelta->delta.f.x;
                    float rz = dst->f.z + w * pDelta->delta.f.z;
                    float ry = dst->f.y + w * pDelta->delta.f.y;
                    dst->f.x = rx;
                    dst->f.y = ry;
                    dst->f.z = rz;
                    pDelta++;
                }
            }
            else
            {
                pDelta = end;
            }

            deltaOffset += 4;
            weightCursor = (ShaderSkinMesh*)((char*)weightCursor + 4);
            m++;
        }
    }

    AttachSkinData(flags, pMatrix);
}

/**
 * Offset/Address/Size: 0x500 | 0x801E0B44 | size: 0x8C
 */
void ShaderSkinMesh::AppendSkinPairList(int numPairs, const SkinPair* pairs)
{
    SkinPairList* node = (SkinPairList*)nlMalloc(sizeof(SkinPairList), 8, false);

    if (node != nullptr)
    {
        node->pairs = nullptr;
        node->m_next = nullptr;
    }

    node->num = numPairs;

    if (numPairs == 0)
    {
        node->pairs = nullptr;
    }
    else
    {
        node->pairs = (SkinPair*)pairs;
    }

    nlRingAddEnd<SkinPairList>(&skinPairs, node);
}

/**
 * Offset/Address/Size: 0x4F4 | 0x801E0B38 | size: 0xC
 */
void ShaderSkinMesh::SetSoftwareVertices(int num, const SkinVertex* skinVertices)
{
    numSoftwareVerts = num;
    softwareVertices = (SkinVertex*)skinVertices;
}

/**
 * Offset/Address/Size: 0x458 | 0x801E0A9C | size: 0x9C
 */
void ShaderSkinMesh::AppendStitchingInfo(int packetIndex, int _numPackets, int num, const unsigned char* pIndices)
{
    if (stitchArray == NULL)
    {
        numPackets = _numPackets;
        stitchArray = (unsigned char**)nlMalloc(numPackets * sizeof(unsigned char*), 8, false);
        memset(stitchArray, 0, numPackets * sizeof(unsigned char*));
    }

    if (num > 0)
    {
        stitchArray[packetIndex] = (unsigned char*)pIndices;
    }
}

/**
 * Offset/Address/Size: 0x398 | 0x801E09DC | size: 0xC0
 */
void UserDataBuilder::AddEntry(const unsigned long& boneID, unsigned long* registerIndex)
{
    unsigned long id = boneID;
    SkinMatrix* foundMatrix = (SkinMatrix*)m_PoseMatrices->FindGet(id);

    m_Bone->reg = (*registerIndex) * 3 - 0x60;
    foundMatrix->Get4x3(m_Bone->mat);

    m_Bone++;
}

/**
 * Offset/Address/Size: 0x2E4 | 0x801E0928 | size: 0xB4
 */
void* ShaderSkinMesh::MakeUserData(nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >* boneMap)
{
    unsigned int count = boneMap->m_NumElements;
    unsigned long size = count * 0x34 + 4;

    void* userData = glUserAlloc(GLUD_Skin, size, false);
    if (userData == nullptr)
    {
        return nullptr;
    }

    void* data = glUserGetData(userData);
    *(unsigned int*)data = count;

    GLSkinUserData* skinDataArray = (GLSkinUserData*)((char*)data + 4);

    UserDataBuilder builder;
    builder.m_Bone = skinDataArray;
    builder.m_PoseMatrices = (nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> >*)&poseMatrices;

    boneMap->Walk(&builder, &UserDataBuilder::AddEntry);

    return userData;
}

/**
 * Offset/Address/Size: 0x108 | 0x801E074C | size: 0x1DC
 */
static inline cSHierarchy* GetPoseHierarchy(cPoseAccumulator* pPoseAccumulator)
{
    return pPoseAccumulator->m_BaseSHierarchy;
}

static inline nlMatrix4* GetPoseNodeMatrixPtr(cPoseAccumulator* pPoseAccumulator, int i)
{
    return &pPoseAccumulator->GetNodeMatrix(i);
}

void ShaderSkinMesh::Pose(cPoseAccumulator* pPoseAccumulator)
{
    SkinMatrix* foundMatrix;
    unsigned long nodeID;
    AVLTreeNode* existingNode;
    SkinMatrix result;
    SkinMatrix skinMat;
    AVLTreeNode** pRoot = (AVLTreeNode**)&poseMatrices.m_Root;

    for (int i = 0; i < pPoseAccumulator->GetNumNodes(); i++)
    {
        cSHierarchy* hierarchy = GetPoseHierarchy(pPoseAccumulator);
        nlMatrix4* nodeMatrix = GetPoseNodeMatrixPtr(pPoseAccumulator, i);
        nodeID = hierarchy->GetNodeID(i);

        u8 found;
        {
            AVLTreeEntry<unsigned long, SkinMatrix>* node = boneMatrices.m_Root;
            while (node != nullptr)
            {
                int cmpResult;
                if (nodeID == node->key)
                    cmpResult = 0;
                else if (nodeID < node->key)
                    cmpResult = -1;
                else
                    cmpResult = 1;

                if (cmpResult == 0)
                {
                    if (&foundMatrix != nullptr)
                    {
                        foundMatrix = &node->value;
                    }
                    found = 1;
                    goto check_found;
                }
                else if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<unsigned long, SkinMatrix>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<unsigned long, SkinMatrix>*)node->node.right;
                }
            }
            found = 0;
        }

    check_found:
        if (found)
        {
            skinMat.Set(*nodeMatrix);

            nlMultMatrices(result, *foundMatrix, skinMat);

            poseMatrices.AddAVLNode(pRoot, &nodeID, &result, &existingNode, poseMatrices.m_NumElements);

            SkinMatrix* existing;
            if (existingNode == nullptr)
            {
                poseMatrices.m_NumElements++;
                existing = nullptr;
            }
            else
            {
                existing = (SkinMatrix*)(((char*)existingNode) + 0x10);
            }

            foundMatrix = existing;
            if (foundMatrix != nullptr)
            {
                *foundMatrix = result;
            }
        }
    }

    for (int j = 0; j < numMorphs; j++)
    {
        morphWeights[j] = pPoseAccumulator->m_MorphWeights.mData[j];
    }
}

/**
 * Offset/Address/Size: 0x98 | 0x801E06DC | size: 0x70
 */
void ShaderSkinMesh::SetMorphNumDeltas(const unsigned long* numDeltas)
{
    if (morphNumDeltas != nullptr)
    {
        delete[] morphNumDeltas;
    }
    morphNumDeltas = (unsigned long*)nlMalloc(numMorphs * sizeof(unsigned long), 8, false);
    memcpy(morphNumDeltas, numDeltas, numMorphs * sizeof(unsigned long));
}

/**
 * Offset/Address/Size: 0x0 | 0x801E0644 | size: 0x98
 */
void ShaderSkinMesh::SetMorphDeltas(int numDeltas, const MorphDelta* p)
{
    if (morphData != nullptr)
    {
        delete[] morphData;
    }

    unsigned int largestBlock = nlVirtualLargestBlock();
    unsigned long size = numDeltas * sizeof(MorphDelta);
    MorphDelta* newData;

    if (largestBlock >= size + 0x100)
    {
        newData = (MorphDelta*)nlVirtualAlloc(size, false);
    }
    else
    {
        newData = (MorphDelta*)nlMalloc(size, 0x20, false);
    }

    morphData = newData;
    memcpy(morphData, p, size);
}
