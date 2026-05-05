#include "Game/Physics/NetMeshModelLoader.h"
#include "Game/WorldManager.h"
#include "Game/Drawable/DrawableModel.h"
#include "NL/glx/glxDisplayList.h"

// Use typedefs from header for convenience
typedef NetMeshModelLoader::NetMeshVertex NetMeshVertex;
typedef NetMeshModelLoader::NetMeshEdge NetMeshEdge;
typedef NetMeshModelLoader::VertexTree VertexTree;
typedef NetMeshModelLoader::EdgeTree EdgeTree;
typedef NetMeshModelLoader::VertexEntry VertexEntry;
typedef NetMeshModelLoader::EdgeEntry EdgeEntry;

static int s_initialEdgeCount = 1;
static int s_initialVertexCount = 1;

// /**
//  * Offset/Address/Size: 0x1554 | 0x80132B00 | size: 0x10
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::DeleteEntry(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x149C | 0x80132A48 | size: 0xB8
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x148C | 0x80132A38 | size: 0x10
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::DeleteEntry(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x13B8 | 0x80132964 | size: 0xD4
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x12E4 | 0x80132890 | size: 0xD4
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1218 | 0x801327C4 | size: 0xCC
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x11A4 | 0x80132750 | size: 0x74
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1130 | 0x801326DC | size: 0x74
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1128 | 0x801326D4 | size: 0x8
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x9D0 | 0x80131F7C | size: 0x758
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::PostorderTraversal(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*, void (AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x96C | 0x80131F18 | size: 0x64
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::DestroyTree(void (AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x914 | 0x80131EC0 | size: 0x58
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x898 | 0x80131E44 | size: 0x7C
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x890 | 0x80131E3C | size: 0x8
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x138 | 0x801316E4 | size: 0x758
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::PostorderTraversal(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*, void (AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xD4 | 0x80131680 | size: 0x64
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::DestroyTree(void (AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x7C | 0x80131628 | size: 0x58
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801315AC | size: 0x7C
//  */
// void AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::~AVLTreeBase()
// {
// }

/**
 * Offset/Address/Size: 0x1400 | 0x80131558 | size: 0x54
 */
NetMeshModelLoader::NetMeshModelLoader(NetMesh& netMesh, unsigned long netMeshDrawableObjectID)
    : m_NetMesh(netMesh)
    , m_NetMeshDrawableObjectID(netMeshDrawableObjectID)
    , m_NumParticles(0)
    , m_TriStripIndices(NULL)
    , m_CurrentTriStripIndex(0)
{
    LoadGeometryFromModel();
}

/**
 * Offset/Address/Size: 0x12C8 | 0x80131420 | size: 0x78
 */
NetMeshModelLoader::~NetMeshModelLoader()
{
    delete m_EdgeList;
    delete m_VertexList;
    delete m_TriStripIndices;
}

/**
 * Offset/Address/Size: 0xE78 | 0x80130FD0 | size: 0x450
 */
void NetMeshModelLoader::LoadGeometryFromModel()
{
    extern void* nlMalloc(unsigned long, unsigned int, bool);

    m_EdgeList = new (nlMalloc(sizeof(EdgeTree), 8, false)) EdgeTree(0x10, 0x10);
    m_VertexList = new (nlMalloc(sizeof(VertexTree), 8, false)) VertexTree(0x10, 0x10);

    DrawableModel* pDrawable = (DrawableModel*)WorldManager::s_World->FindDrawableObject(m_NetMeshDrawableObjectID);
    u16 maxVertex = 0;
    u16 vertexOffset = 0;
    glModelPacket* pPacket = NULL;
    u16 numPackets = (u16)pDrawable->m_pModel->numPackets;
    s32 packetCount = 0;
    s32 packetSumIndex = 0;

    m_NumTriStripIndices = packetCount;
    m_CurrentTriStripIndex = packetCount;

    for (packetSumIndex = 0; packetSumIndex < numPackets; packetSumIndex++)
    {
        m_NumTriStripIndices += pDrawable->m_pModel->packets[packetSumIndex].numVertices;
    }

    m_TriStripIndices = (u16*)nlMalloc((unsigned long)(m_NumTriStripIndices * 2), 8, false);
    for (s32 i = 0; i < m_NumTriStripIndices; i++)
    {
        m_TriStripIndices[i] = 0xFFFF;
    }

    s32 packetIndex = 0;
    s32 packetOffset = 0;
    while (packetIndex < numPackets)
    {
        maxVertex = 0;
        vertexOffset = (u16)m_NumParticles;
        pPacket = (glModelPacket*)((u8*)pDrawable->m_pModel->packets + packetOffset);

        DisplayList* pList = dlGetStruct(pPacket->indexBuffer);
        s32 i = 0;
        s32 triStripOffset = 0;

        while (i < pPacket->numVertices)
        {
            u16* ptr;
            if (((u16*)&pList->indices)[1] != 0)
            {
                u16 ns = ((u16*)&pList->indices)[0];
                s32 stride = (ns - 1) * 2 + 1;
                s32 offset = stride * i;
                u8* ptr8 = (u8*)pList->list + offset;
                ptr = (u16*)ptr8;
                ptr8 = (u8*)ptr;
                ptr8 += 4;
                ptr = (u16*)ptr8;
            }
            else
            {
                u16 ns = ((u16*)&pList->indices)[0];
                s32 stride = ns * 2;
                s32 offset = stride * i;
                u8* ptr8 = (u8*)pList->list;
                ptr8 += offset;
                ptr = (u16*)ptr8;
                ptr = (u16*)((u8*)ptr + 3);
            }

            u16 idx = *ptr;
            if (idx > maxVertex)
            {
                maxVertex = idx;
            }

            *(u16*)((u8*)m_TriStripIndices + triStripOffset) = idx + vertexOffset;
            i++;
            triStripOffset += 2;
            m_CurrentTriStripIndex++;
        }

        m_NetMesh.SetTexture(pPacket->state.texture[0]);

        u16 maxV = maxVertex;
        s32 i2 = 0;
        while (i2 <= maxV)
        {
            NetMeshVertex vertex;
            vertex.mpPacket = pPacket;
            vertex.mIndex = i2;
            vertex.mParticleIndex = m_NumParticles;

            VertexTree* vertexList = m_VertexList;
            AVLTreeNode* existing;
            vertexList->AddAVLNode(
                (AVLTreeNode**)&vertexList->m_Root,
                &vertex,
                &s_initialVertexCount,
                &existing,
                vertexList->m_NumElements);

            if (existing == NULL)
            {
                vertexList->m_NumElements++;
            }

            m_NumParticles++;
            i2++;
        }

        ReadEdgesFromGeometryPacket(*pPacket);
        packetOffset += sizeof(glModelPacket);
        packetIndex++;
    }

    CreateNetMeshFromVertexList();
}

/**
 * Offset/Address/Size: 0xC44 | 0x80130D9C | size: 0x234
 */
void NetMeshModelLoader::AddEdge(const glModelPacket& packet, unsigned short idx1, unsigned short idx2)
{
    NetMeshEdge edge;
    edge.mpPacket = NULL;
    edge.mpVertex1 = NULL;
    edge.mpVertex2 = NULL;

    NetMeshVertex* pVertex1;
    NetMeshVertex* pVertex2;
    int* pValue;

    // First vertex lookup - find vertex entry for (packet, idx1)
    {
        VertexEntry* vnode = m_VertexList->m_Root;
        while (vnode != NULL)
        {
            const glModelPacket* nodePacket = vnode->key.mpPacket;
            bool found = (&packet == nodePacket && idx1 == vnode->key.mIndex);
            int cmp;
            if (found)
            {
                cmp = 0;
            }
            else
            {
                bool less = (&packet < nodePacket || (&packet == nodePacket && idx1 < vnode->key.mIndex));
                if (less)
                {
                    cmp = -1;
                }
                else
                {
                    cmp = 1;
                }
            }

            if (cmp == 0)
            {
                int** ppValue = &pValue;
                if (ppValue != NULL)
                {
                    *ppValue = &vnode->value;
                }
                NetMeshVertex** ppKey = &pVertex1;
                if (ppKey != NULL)
                {
                    *ppKey = &vnode->key;
                }
                break;
            }
            else if (cmp < 0)
            {
                vnode = (VertexEntry*)vnode->node.left;
            }
            else
            {
                vnode = (VertexEntry*)vnode->node.right;
            }
        }
    }

    // Second vertex lookup - find vertex entry for (packet, idx2)
    {
        VertexEntry* vnode = m_VertexList->m_Root;
        while (vnode != NULL)
        {
            const glModelPacket* nodePacket = vnode->key.mpPacket;
            bool found = (&packet == nodePacket && idx2 == vnode->key.mIndex);
            int cmp;
            if (found)
            {
                cmp = 0;
            }
            else
            {
                bool less = (&packet < nodePacket || (&packet == nodePacket && idx2 < vnode->key.mIndex));
                if (less)
                {
                    cmp = -1;
                }
                else
                {
                    cmp = 1;
                }
            }

            if (cmp == 0)
            {
                int** ppValue = &pValue;
                if (ppValue != NULL)
                {
                    *ppValue = &vnode->value;
                }
                NetMeshVertex** ppKey = &pVertex2;
                if (ppKey != NULL)
                {
                    *ppKey = &vnode->key;
                }
                break;
            }
            else if (cmp < 0)
            {
                vnode = (VertexEntry*)vnode->node.left;
            }
            else
            {
                vnode = (VertexEntry*)vnode->node.right;
            }
        }
    }

    // Construct edge with vertices ordered by mIndex (smaller first)
    edge.mpPacket = &packet;

    NetMeshVertex* v2 = pVertex2;
    if (pVertex1->mIndex < v2->mIndex)
    {
        edge.mpVertex1 = pVertex1;
        edge.mpVertex2 = v2;
    }
    else
    {
        edge.mpVertex1 = v2;
        edge.mpVertex2 = pVertex1;
    }

    // Insert edge into edge tree
    EdgeTree* edgeTree = m_EdgeList;
    AVLTreeNode* existing;
    edgeTree->AddAVLNode((AVLTreeNode**)&edgeTree->m_Root, &edge, &s_initialEdgeCount, &existing, edgeTree->m_NumElements);

    // If new edge: increment element count
    // If existing edge: increment reference count
    int* pRefCount;
    if (existing == NULL)
    {
        edgeTree->m_NumElements++;
        pRefCount = NULL;
    }
    else
    {
        pRefCount = &((EdgeEntry*)existing)->value;
    }

    if (pRefCount != NULL)
    {
        (*pRefCount)++;
    }
}

extern void AddTriangleFromGeometry__18NetMeshModelLoaderFRC13glModelPacketPUs(
    NetMeshModelLoader*, const glModelPacket&, unsigned short*);

/**
 * Offset/Address/Size: 0xA80 | 0x80130BD8 | size: 0x110
 * TODO: 96.18% match - callee-saved register mapping is still shifted for
 * this/packet/numVerts, and the inner loop keeps j/base/index temporaries in
 * r4/r5/r6/r3 instead of r7/r3/r6/r4.
 */
void NetMeshModelLoader::ReadEdgesFromGeometryPacket(const glModelPacket& packet)
{
    u16 maxVertex = 0;

    if (packet.primType != 1)
        return;

    DisplayList* pList = dlGetStruct(packet.indexBuffer);

    s32 i = 2;
    s32 numVerts = packet.numVertices;
    while (i < numVerts)
    {
        u16 vertexIndices[3];
        s32 j = 0;

        while (j < 3)
        {
            u16* ptr;
            if (((u16*)&pList->indices)[1] != 0)
            {
                u16 ns = ((u16*)&pList->indices)[0];
                s32 vertOff = i;
                vertOff += j;
                vertOff -= 2;
                s32 stride = (ns - 1) * 2 + 1;
                s32 offset = stride * vertOff;
                u8* ptr8 = (u8*)pList->list;
                ptr8 += offset;
                ptr = (u16*)ptr8;
                ptr8 = (u8*)ptr;
                ptr8 += 4;
                ptr = (u16*)ptr8;
            }
            else
            {
                u16 ns = ((u16*)&pList->indices)[0];
                s32 vertOff = i;
                vertOff += j;
                vertOff -= 2;
                s32 stride = ns * 2;
                s32 offset = stride * vertOff;
                u8* ptr8 = (u8*)pList->list;
                ptr8 += offset;
                ptr = (u16*)ptr8;
                ptr8 = (u8*)ptr;
                ptr8 += 3;
                ptr = (u16*)ptr8;
            }

            vertexIndices[j] = *ptr;
            if (vertexIndices[j] > maxVertex)
                maxVertex = vertexIndices[j];
            j++;
        }

        AddTriangleFromGeometry__18NetMeshModelLoaderFRC13glModelPacketPUs(this, packet, vertexIndices);
        i++;
    }

    ProcessEdges(packet, (s32)maxVertex);
}

/**
 * Offset/Address/Size: 0xB90 | 0x80130CE8 | size: 0xB4
 */
void NetMeshModelLoader::AddTriangleFromGeometry(const glModelPacket& packet, unsigned short* vertexIndices)
{
    unsigned char isThin = 0;

    if (vertexIndices[0] == vertexIndices[1] || vertexIndices[1] == vertexIndices[2] || vertexIndices[0] == vertexIndices[2])
    {
        isThin = 1;
    }

    if (!isThin)
    {
        for (int j = 0; j < 3; j++)
        {
            AddEdge(packet, vertexIndices[j], vertexIndices[(j + 1) % 3]);
        }
    }
}

/**
 * Offset/Address/Size: 0x780 | 0x801308D8 | size: 0x300
 */
void NetMeshModelLoader::ProcessEdges(const glModelPacket& packet, int maxVertex)
{
    extern void* nlMalloc(unsigned long, unsigned int, bool);

    struct EdgeIter
    {
        EdgeEntry** m_Stack;
        unsigned int m_NumStackEntries;
    };

    EdgeIter* iter;
    unsigned char* allReady;
    unsigned char* readyIndicator;

    allReady = (unsigned char*)nlMalloc(maxVertex + 1, 8, false);
    {
        int i = 0;
        while (i < maxVertex)
        {
            allReady[i] = 0;
            i++;
        }
    }

    readyIndicator = (unsigned char*)nlMalloc(maxVertex + 1, 8, false);
    {
        int i = 0;
        while (i < maxVertex)
        {
            readyIndicator[i] = 0;
            i++;
        }
    }

    EdgeTree* edgeTree = m_EdgeList;
    iter = (EdgeIter*)nlMalloc(sizeof(EdgeIter), 8, false);

    if (iter != NULL)
    {
        unsigned int numEntries = edgeTree->m_NumElements;
        EdgeEntry* node = edgeTree->m_Root;

        iter->m_Stack = (EdgeEntry**)nlMalloc((numEntries + 1) * 4, 8, false);
        iter->m_NumStackEntries = 0;

        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                iter->m_Stack[iter->m_NumStackEntries] = node;
                iter->m_NumStackEntries++;
                node = (EdgeEntry*)node->node.left;
            }

            iter->m_Stack[iter->m_NumStackEntries] = node;
            iter->m_NumStackEntries++;
        }
    }

    while (iter->m_NumStackEntries != 0)
    {
        unsigned short index1;
        unsigned short index2;
        EdgeEntry* edgeEntry = iter->m_Stack[iter->m_NumStackEntries - 1];

        if (edgeEntry->key.mpPacket == &packet)
        {
            index1 = edgeEntry->key.mpVertex1->mIndex;
            index2 = edgeEntry->key.mpVertex2->mIndex;

            if (edgeEntry->value == 1)
            {
                allReady[index1] = 1;
                allReady[index2] = 1;
                edgeEntry->key.mpVertex1->mbIsConstrained = 1;
                edgeEntry->key.mpVertex2->mbIsConstrained = 1;
            }

            readyIndicator[index1] = 1;
            readyIndicator[index2] = 1;
        }

        iter->m_NumStackEntries--;
        edgeEntry = iter->m_Stack[iter->m_NumStackEntries];

        EdgeEntry* right = (EdgeEntry*)edgeEntry->node.right;
        if (right != NULL)
        {
            while (right->node.left != NULL)
            {
                iter->m_Stack[iter->m_NumStackEntries] = right;
                iter->m_NumStackEntries++;
                right = (EdgeEntry*)right->node.left;
            }

            iter->m_Stack[iter->m_NumStackEntries] = right;
            iter->m_NumStackEntries++;
        }
    }

    if (iter != NULL)
    {
        delete[] iter->m_Stack;
        delete iter;
    }

    delete readyIndicator;
    delete allReady;
}

/**
 * Offset/Address/Size: 0x0 | 0x80130158 | size: 0x780
 */
void NetMeshModelLoader::CreateNetMeshFromVertexList()
{
    FORCE_DONT_INLINE;
}
