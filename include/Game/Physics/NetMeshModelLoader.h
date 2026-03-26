#ifndef _NETMESHMODELLOADER_H_
#define _NETMESHMODELLOADER_H_

#include "Game/Render/NetMesh.h"
#include "NL/gl/glModel.h"
#include "NL/nlAVLTreeSlotPool.h"

class NetMeshModelLoader
{
public:
    class NetMeshVertex
    {
    public:
        NetMeshVertex()
            : mpPacket(NULL)
            , mIndex(0xFFFF)
            , mParticleIndex(-1)
            , mbIsConstrained(0)
        {
        }

        const nlVector3* GetPosition() const;
        void GetNormal(nlVector3& out) const;
        void GetTextureCoord(nlVector2& out) const;

        bool operator==(const NetMeshVertex& o) const { return mpPacket == o.mpPacket && mIndex == o.mIndex; }
        bool operator<(const NetMeshVertex& o) const { return mpPacket < o.mpPacket || (mpPacket == o.mpPacket && mIndex < o.mIndex); }
        bool operator>(const NetMeshVertex& o) const { return o < *this; }

        /* 0x00 */ const glModelPacket* mpPacket;
        /* 0x04 */ unsigned short mIndex;
        /* 0x08 */ int mParticleIndex;
        /* 0x0C */ unsigned char mbIsConstrained;
    };

    class NetMeshEdge
    {
    public:
        bool operator==(const NetMeshEdge& o) const { return mpPacket == o.mpPacket && mpVertex1->mIndex == o.mpVertex1->mIndex && mpVertex2->mIndex == o.mpVertex2->mIndex; }
        bool operator<(const NetMeshEdge& o) const { return mpPacket < o.mpPacket || (mpPacket == o.mpPacket && mpVertex1->mIndex < o.mpVertex1->mIndex) || (mpPacket == o.mpPacket && mpVertex1->mIndex == o.mpVertex1->mIndex && mpVertex2->mIndex < o.mpVertex2->mIndex); }
        bool operator>(const NetMeshEdge& o) const { return o < *this; }

        void operator=(const NetMeshEdge& o)
        {
            NetMeshVertex* v1;
            NetMeshVertex* v2;
            v2 = o.mpVertex2;
            v1 = o.mpVertex1;
            mpPacket = o.mpPacket;
            if (v1->mIndex < v2->mIndex)
            {
                mpVertex1 = v1;
                mpVertex2 = v2;
            }
            else
            {
                mpVertex1 = v2;
                mpVertex2 = v1;
            }
        }

        /* 0x00 */ const glModelPacket* mpPacket;
        /* 0x04 */ NetMeshVertex* mpVertex1;
        /* 0x08 */ NetMeshVertex* mpVertex2;
    };

    typedef nlAVLTreeSlotPool<NetMeshEdge, int, DefaultKeyCompare<NetMeshEdge> > EdgeTree;
    typedef nlAVLTreeSlotPool<NetMeshVertex, int, DefaultKeyCompare<NetMeshVertex> > VertexTree;
    typedef AVLTreeEntry<NetMeshEdge, int> EdgeEntry;
    typedef AVLTreeEntry<NetMeshVertex, int> VertexEntry;

    NetMeshModelLoader(NetMesh&, unsigned long);
    virtual ~NetMeshModelLoader();
    void LoadGeometryFromModel();
    void AddEdge(const glModelPacket&, unsigned short, unsigned short);
    void AddTriangleFromGeometry(const glModelPacket&, unsigned short*);
    void ReadEdgesFromGeometryPacket(const glModelPacket&);
    void ProcessEdges(const glModelPacket&, int);
    void CreateNetMeshFromVertexList();

    /* 0x04 */ NetMesh& m_NetMesh;            // offset 0x4, size 0x4
    /* 0x08 */ u32 m_NetMeshDrawableObjectID; // offset 0x8, size 0x4
    /* 0x0C */ EdgeTree* m_EdgeList;          // offset 0xC, size 0x4
    /* 0x10 */ VertexTree* m_VertexList;      // offset 0x10, size 0x4
    /* 0x14 */ int m_NumParticles;            // offset 0x14, size 0x4
    /* 0x18 */ u16* m_TriStripIndices;        // offset 0x18, size 0x4
    /* 0x1C */ int m_CurrentTriStripIndex;    // offset 0x1C, size 0x4
    /* 0x20 */ int m_NumTriStripIndices;      // offset 0x20, size 0x4
};

// class nlAVLTreeSlotPool<NetMeshModelLoader
// {
// public:
//     void NetMeshEdge, int, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::~nlAVLTreeSlotPool();
//     void NetMeshVertex, int, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::~nlAVLTreeSlotPool();
// };

// class AVLTreeBase<NetMeshModelLoader
// {
// public:
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::DeleteEntry(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*);
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::AllocateEntry(void*, void*);
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::DeleteEntry(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*);
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CompareKey(void*, AVLTreeNode*);
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::AllocateEntry(void*, void*);
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CompareKey(void*, AVLTreeNode*);
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::CastUp(AVLTreeNode*) const;
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::PostorderTraversal(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*, void (AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*));
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::DestroyTree(void (AVLTreeBase<NetMeshModelLoader::NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>*));
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::Clear();
//     void NetMeshEdge, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshEdge, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshEdge>>::~AVLTreeBase();
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::CastUp(AVLTreeNode*) const;
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::PostorderTraversal(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*, void (AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*));
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::DestroyTree(void (AVLTreeBase<NetMeshModelLoader::NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::*)(AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>*));
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::Clear();
//     void NetMeshVertex, int, BasicSlotPool<AVLTreeEntry<NetMeshModelLoader::NetMeshVertex, int>>, DefaultKeyCompare<NetMeshModelLoader::NetMeshVertex>>::~AVLTreeBase();
// };

#endif // _NETMESHMODELLOADER_H_
