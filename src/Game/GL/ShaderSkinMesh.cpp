// TODO: Revisit proper header-style emission for the helper templates below.
// Moving these bodies to private inline headers makes them weak/linkonce, but
// MWCC then orders the AVL tail after the ring block, or reverses the tail,
// which breaks the linked DOL SHA. The explicit source definitions below are
// byte-linkable; WEAKFUNC fixes the ordinary AVL helper bindings, but MWCC
// still emits the function-template/member-template specializations as strong
// globals when they are defined in this .cpp.
#define NL_AVLTREE_DECLARE_ONLY
#include "Game/GL/ShaderSkinMesh.h"
#undef NL_AVLTREE_DECLARE_ONLY

#include "Game/GL/GLSkinMesh.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glUserData.h"
#include "NL/nlDLRing.h"
#include "NL/nlMemory.h"
#include "string.h"
#include "types.h"

static nlVector3 sharedMorphBuffer[0x1000];

template <>
nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >::~nlAVLTree();

typedef AVLTreeEntry<unsigned long, unsigned long> BoneIndexEntry;
typedef AVLTreeBase<unsigned long, unsigned long, NewAdapter<BoneIndexEntry>, DefaultKeyCompare<unsigned long> > BoneIndexBase;
typedef AVLTreeEntry<unsigned long, SkinMatrix> SkinMatrixEntry;
typedef AVLTreeBase<unsigned long, SkinMatrix, NewAdapter<SkinMatrixEntry>, DefaultKeyCompare<unsigned long> > SkinMatrixBase;

/**
 * Offset/Address/Size: 0x14C | 0x801E2240 | size: 0x58
 */
WEAKFUNC BoneMapList::~BoneMapList()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0xD8 | 0x801E21CC | size: 0x74
 */
template <>
WEAKFUNC void nlDeleteRing<BoneMapList>(BoneMapList** head)
{
    FORCE_DONT_INLINE;
    BoneMapList* current;
    BoneMapList* next;

    BoneMapList* headPtr = *head;
    if (headPtr != NULL)
    {
        current = headPtr->m_next;
        for (;;)
        {
            next = current->m_next;
            delete current;
            if (current != *head)
            {
                current = next;
            }
            else
            {
                break;
            }
        }
        *head = NULL;
    }
}

/**
 * Offset/Address/Size: 0xAC | 0x801E21A0 | size: 0x2C
 */
template <>
WEAKFUNC void nlRingAddStart<SkinPairList>(SkinPairList** list, SkinPairList* item)
{
    FORCE_DONT_INLINE;
    SkinPairList* head = *list;
    if (head == nullptr)
    {
        *list = item;
        item->m_next = item;
        return;
    }

    item->m_next = head->m_next;
    head = *list;
    head->m_next = item;
}

/**
 * Offset/Address/Size: 0x70 | 0x801E2164 | size: 0x3C
 */
template <>
WEAKFUNC void nlRingAddEnd<SkinPairList>(SkinPairList** list, SkinPairList* item)
{
    FORCE_DONT_INLINE;
    nlRingAddStart(list, item);
    *list = item;
}

/**
 * Offset/Address/Size: 0x0 | 0x801E20F4 | size: 0x70
 */
template <>
WEAKFUNC void nlDeleteRing<SkinPairList>(SkinPairList** head)
{
    FORCE_DONT_INLINE;
    SkinPairList* current;
    SkinPairList* next;

    SkinPairList* headPtr = *head;
    if (headPtr != NULL)
    {
        current = headPtr->m_next;
        for (;;)
        {
            next = current->m_next;
            delete current;
            if (current != *head)
            {
                current = next;
            }
            else
            {
                break;
            }
        }
        *head = NULL;
    }
}

/**
 * Offset/Address/Size: 0xC9C | 0x801E20D0 | size: 0x24
 */
template <>
WEAKFUNC void SkinMatrixBase::DeleteEntry(SkinMatrixEntry* entry)
{
    m_Allocator.Free(entry);
}

/**
 * Offset/Address/Size: 0xBE0 | 0x801E2014 | size: 0xBC
 */
template <>
WEAKFUNC AVLTreeNode* SkinMatrixBase::AllocateEntry(void* key, void* value)
{
    SkinMatrixEntry* newNode = NULL;

    m_Allocator.Allocate(newNode);

    newNode->node.left = NULL;
    newNode->node.right = NULL;
    newNode->node.heavy = 0;
    newNode->key = *(unsigned long*)key;
    newNode->value = *(SkinMatrix*)value;

    return (AVLTreeNode*)newNode;
}

/**
 * Offset/Address/Size: 0xBB4 | 0x801E1FE8 | size: 0x2C
 */
template <>
WEAKFUNC int SkinMatrixBase::CompareKey(void* key, AVLTreeNode* node)
{
    int result;
    unsigned long k = *(unsigned long*)key;
    SkinMatrixEntry* entry = (SkinMatrixEntry*)node;
    if (k == entry->key)
        result = 0;
    else if (k < entry->key)
        result = -1;
    else
        result = 1;
    return result;
}

/**
 * Offset/Address/Size: 0xB88 | 0x801E1FBC | size: 0x2C
 */
template <>
WEAKFUNC int SkinMatrixBase::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    const unsigned long& keyA = ((SkinMatrixEntry*)a)->key;
    const unsigned long& keyB = ((SkinMatrixEntry*)b)->key;
    int result;
    if (keyA == keyB)
        result = 0;
    else if (keyA < keyB)
        result = -1;
    else
        result = 1;
    return result;
}

/**
 * Offset/Address/Size: 0xB80 | 0x801E1FB4 | size: 0x8
 */
template <>
WEAKFUNC SkinMatrixEntry* SkinMatrixBase::CastUp(AVLTreeNode* node) const
{
    return (SkinMatrixEntry*)node;
}

/**
 * Offset/Address/Size: 0x428 | 0x801E185C | size: 0x758
 */
template <>
WEAKFUNC void SkinMatrixBase::PostorderTraversal(SkinMatrixEntry* curr, void (SkinMatrixBase::*cb)(SkinMatrixEntry*))
{
    if (curr->node.left != NULL)
    {
        PostorderTraversal(CastUp(curr->node.left), cb);
    }
    if (curr->node.right != NULL)
    {
        PostorderTraversal(CastUp(curr->node.right), cb);
    }
    (this->*cb)(curr);
}

/**
 * Offset/Address/Size: 0x3C4 | 0x801E17F8 | size: 0x64
 */
template <>
WEAKFUNC void SkinMatrixBase::DestroyTree(void (SkinMatrixBase::*deleteFunc)(SkinMatrixEntry*))
{
    if (m_Root != NULL)
    {
        PostorderTraversal(m_Root, deleteFunc);
        m_Root = nullptr;
        m_NumElements = 0;
    }
}

/**
 * Offset/Address/Size: 0x36C | 0x801E17A0 | size: 0x58
 */
template <>
WEAKFUNC void SkinMatrixBase::Clear()
{
    DestroyTree(&SkinMatrixBase::DeleteEntry);
    m_NumElements = 0;
}

/**
 * Offset/Address/Size: 0x310 | 0x801E1744 | size: 0x5C
 */
template <>
WEAKFUNC SkinMatrixBase::~AVLTreeBase()
{
    Clear();
}

/**
 * Offset/Address/Size: 0x44 | 0x801E1478 | size: 0x2CC
 */
template <>
template <>
WEAKFUNC void BoneIndexBase::InorderWalk<UserDataBuilder>(
    BoneIndexEntry* curr,
    UserDataBuilder* cbClass,
    void (UserDataBuilder::*cb)(const unsigned long&, unsigned long*))
{
    while (curr != nullptr)
    {
        InorderWalk(CastUp(curr->node.left), cbClass, cb);
        (cbClass->*cb)(curr->key, &curr->value);
        curr = CastUp(curr->node.right);
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801E1434 | size: 0x44
 */
template <>
template <>
WEAKFUNC void BoneIndexBase::Walk<UserDataBuilder>(
    UserDataBuilder* cbClass,
    void (UserDataBuilder::*cb)(const unsigned long&, unsigned long*))
{
    InorderWalk(m_Root, cbClass, cb);
}

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

inline void ShaderSkinMesh::CreateMorphBuffer()
{
    morphBuffer = sharedMorphBuffer;
    CopySoftwareVerts(this);

    MorphDelta* pCurrentMorph = morphData;
    int morphIndex = 0;

    while (morphIndex < numMorphs)
    {
        MorphDelta* pEndMorph = pCurrentMorph + morphNumDeltas[morphIndex];

        if (morphWeights[morphIndex] > 0.0f)
        {
            while (pCurrentMorph != pEndMorph)
            {
                float w = morphWeights[morphIndex];
                nlVector3* dst = &morphBuffer[pCurrentMorph->index];
                float rx = dst->f.x + w * pCurrentMorph->delta.f.x;
                float rz = dst->f.z + w * pCurrentMorph->delta.f.z;
                float ry = dst->f.y + w * pCurrentMorph->delta.f.y;
                dst->f.x = rx;
                dst->f.y = ry;
                dst->f.z = rz;
                pCurrentMorph++;
            }
        }
        else
        {
            pCurrentMorph = pEndMorph;
        }

        morphIndex++;
    }
}

/**
 * Offset/Address/Size: 0x58C | 0x801E0BD0 | size: 0x1E0
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
        CreateMorphBuffer();
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
inline void UserDataBuilder::AddEntry(const unsigned long& boneID, unsigned long* registerIndex)
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
 * Offset/Address/Size: 0xCC8 | 0x801E130C | size: 0x60
 */
template <>
inline nlAVLTree<unsigned long, SkinMatrix, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
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
