#ifndef _GLRENDERLIST_H_
#define _GLRENDERLIST_H_

// void gl_ViewAttachPacket(eGLView, unsigned long, const glModelPacket*);
// void nlDLRingAddEnd<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*);
// void nlDLRingAddStart<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*);

#include "NL/gl/glView.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "NL/nlDLListSlotPool.h"
// Forward-declare instead of #include: only "extern GLRenderBuffer glRenderBuffer;"
// below needs the name, and pulling the full definition (with its inline ctor)
// into every includer forces a spurious DLListContainer DeleteEntry/PTMF
// instantiation in TUs that use GLRenderList only by pointer (e.g. glPlat, glView).
class GLRenderBuffer;

class glModel;

class TextureTreeCompare
{
public:
    int operator()(const glModelPacket* b, const glModelPacket* a) const
    {
        if (a->state.program < b->state.program)
            return -1;
        if (a->state.program > b->state.program)
            return 1;
        if (a->state.texconfig < b->state.texconfig)
            return -1;
        if (a->state.texconfig > b->state.texconfig)
            return 1;
        if (a->state.texture[0] < b->state.texture[0])
            return -1;
        if (a->state.texture[0] > b->state.texture[0])
            return 1;
        if (a->userData < b->userData)
            return -1;
        if (a->userData > b->userData)
            return 1;
        if (a < b)
            return -1;
        if (a > b)
            return 1;
        return 0;
    }
};

struct DepthPacketPair
{
    /* 0x0 */ unsigned long sortKey;
    /* 0x4 */ const glModelPacket* packet;
}; // total size: 0x8

inline bool operator==(const DepthPacketPair& a, const DepthPacketPair& b)
{
    return a.sortKey == b.sortKey;
}
inline bool operator<(const DepthPacketPair& a, const DepthPacketPair& b)
{
    return a.sortKey < b.sortKey;
}

class DepthTreeCompare
{
public:
    int operator()(const DepthPacketPair& b, const DepthPacketPair& a) const
    {
        int result;
        if (a.sortKey == b.sortKey)
            result = 0;
        else if (a.sortKey < b.sortKey)
            result = -1;
        else
            result = 1;
        return result;
    }
};

template <>
inline int AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int> >, DepthTreeCompare>::CompareKey(void* key, AVLTreeNode* node)
{
    int result;
    DepthPacketPair* k = (DepthPacketPair*)key;
    AVLTreeEntry<DepthPacketPair, unsigned int>* entry = (AVLTreeEntry<DepthPacketPair, unsigned int>*)node;
    if (entry->key.sortKey < k->sortKey)
        result = -1;
    else if (entry->key.sortKey > k->sortKey)
        result = 1;
    else if (k->packet < entry->key.packet)
        result = -1;
    else if (k->packet > entry->key.packet)
        result = 1;
    else
        result = 0;
    return result;
}

template <>
inline int AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int> >, DepthTreeCompare>::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    int result;
    AVLTreeEntry<DepthPacketPair, unsigned int>* entryA = (AVLTreeEntry<DepthPacketPair, unsigned int>*)a;
    AVLTreeEntry<DepthPacketPair, unsigned int>* entryB = (AVLTreeEntry<DepthPacketPair, unsigned int>*)b;
    if (entryB->key.sortKey < entryA->key.sortKey)
        result = -1;
    else if (entryB->key.sortKey > entryA->key.sortKey)
        result = 1;
    else if (entryA->key.packet < entryB->key.packet)
        result = -1;
    else if (entryA->key.packet > entryB->key.packet)
        result = 1;
    else
        result = 0;
    return result;
}

template <>
inline int AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int> >, TextureTreeCompare>::CompareKey(void* key, AVLTreeNode* node)
{
    AVLTreeEntry<const glModelPacket*, unsigned int>* entry = (AVLTreeEntry<const glModelPacket*, unsigned int>*)node;
    const glModelPacket* e = entry->key;
    TextureTreeCompare cmp;
    return cmp(e, *(const glModelPacket**)key);
}

inline int CmpTexturePackets(const glModelPacket* keyA, const glModelPacket* keyB)
{
    int result;
    if (keyA->state.program < keyB->state.program)
        result = -1;
    else if (keyA->state.program > keyB->state.program)
        result = 1;
    else if (keyA->state.texconfig < keyB->state.texconfig)
        result = -1;
    else if (keyA->state.texconfig > keyB->state.texconfig)
        result = 1;
    else if (keyA->state.texture[0] < keyB->state.texture[0])
        result = -1;
    else if (keyA->state.texture[0] > keyB->state.texture[0])
        result = 1;
    else if (keyA->userData < keyB->userData)
        result = -1;
    else if (keyA->userData > keyB->userData)
        result = 1;
    else if (keyA < keyB)
        result = -1;
    else if (keyA > keyB)
        result = 1;
    else
        result = 0;
    return result;
}

template <>
inline int AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int> >, TextureTreeCompare>::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    return CmpTexturePackets(
        ((AVLTreeEntry<const glModelPacket*, unsigned int>*)a)->key,
        ((AVLTreeEntry<const glModelPacket*, unsigned int>*)b)->key);
}

class GLTexturePacketTree : public nlAVLTreeSlotPool<const glModelPacket*, unsigned int, TextureTreeCompare>
{
public:
    GLTexturePacketTree()
        : nlAVLTreeSlotPool<const glModelPacket*, unsigned int, TextureTreeCompare>()
    {
    }
    GLTexturePacketTree(int initial, int delta)
        : nlAVLTreeSlotPool<const glModelPacket*, unsigned int, TextureTreeCompare>(initial, delta)
    {
    }
    ~GLTexturePacketTree() { }
};

class GLDepthPacketTree : public nlAVLTreeSlotPool<DepthPacketPair, unsigned int, DepthTreeCompare>
{
public:
    GLDepthPacketTree()
        : nlAVLTreeSlotPool<DepthPacketPair, unsigned int, DepthTreeCompare>()
    {
    }
    GLDepthPacketTree(int initial, int delta)
        : nlAVLTreeSlotPool<DepthPacketPair, unsigned int, DepthTreeCompare>(initial, delta)
    {
    }
    ~GLDepthPacketTree() { }
};

class GLPacketList : public nlDLListSlotPool<const glModelPacket*>
{
};

extern GLRenderBuffer glRenderBuffer;

class GLRenderList // size: 0x30
{
public:
    s32 AttachModel(const glModel*, unsigned long);
    void Iterate(eGLView, void (*)(eGLView, unsigned long, const glModelPacket*));
    bool IsEmpty() const;
    void Compact();
    void Clear();

    GLRenderList();

    /* 0x00 */ u32 m_unk_0x00;
    /* 0x04 */ eGLViewSort m_unk_0x04;
    /* 0x08 */ unsigned long uDepthInsertNumber;
    /* 0x0C */ GLTexturePacketTree* texPacketTree[7];
    /* 0x28 */ GLDepthPacketTree* depthPacketTree;
    /* 0x2C */ GLPacketList* packetList;
};

class PacketCallbackManager
{
public:
    /* 0x00 */ eGLView m_View;
    /* 0x04 */ void (*m_Cb)(eGLView, unsigned long, const glModelPacket*);
    /* 0x08 */ unsigned long m_LastProgram;
    /* 0x0C */ unsigned long m_LastRaster;
    /* 0x10 */ unsigned long long m_LastTextureState;
    /* 0x18 */ unsigned long m_LastMatrix;
    /* 0x1C */ unsigned long m_LastTexconfig;
    /* 0x20 */ unsigned long m_LastUserdata;
    /* 0x24 */ unsigned long m_LastNumStreams;
    /* 0x28 */ glModelStream* m_LastStreams;
    /* 0x2C */ unsigned long m_LastTexture[6];
    /* 0x44 */ unsigned long m_LastUserStateKey;
    /* 0x48 */ unsigned long m_LastMaterialSet;

    void ListCallback(const glModelPacket**);
    void DepthCallback(const DepthPacketPair&, unsigned int*);
    void TexCallback(const glModelPacket* const&, unsigned int*);
    void DoCallback(const glModelPacket*, unsigned int);
};

#include "NL/nlWalkHelper.h"

// class AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>
// {
// public:
//     void DeleteEntry(AVLTreeEntry<const glModelPacket*, unsigned int>*);
//     void Walk<PacketCallbackManager>(PacketCallbackManager*, void (PacketCallbackManager::*)(const glModelPacket* const&, unsigned int*));
//     void InorderWalk<PacketCallbackManager>(AVLTreeEntry<const glModelPacket*, unsigned int>*, PacketCallbackManager*, void (PacketCallbackManager::*)(const glModelPacket* const&, unsigned int*));
//     void CastUp(AVLTreeNode*) const;
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::*)(AVLTreeEntry<const glModelPacket*, unsigned int>*));
//     void PostorderTraversal(AVLTreeEntry<const glModelPacket*, unsigned int>*, void (AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::*)(AVLTreeEntry<const glModelPacket*, unsigned int>*));
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
// };

// class AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>
// {
// public:
//     void DeleteEntry(AVLTreeEntry<DepthPacketPair, unsigned int>*);
//     void Walk<PacketCallbackManager>(PacketCallbackManager*, void (PacketCallbackManager::*)(const DepthPacketPair&, unsigned int*));
//     void InorderWalk<PacketCallbackManager>(AVLTreeEntry<DepthPacketPair, unsigned int>*, PacketCallbackManager*, void (PacketCallbackManager::*)(const DepthPacketPair&, unsigned int*));
//     void CastUp(AVLTreeNode*) const;
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::*)(AVLTreeEntry<DepthPacketPair, unsigned int>*));
//     void PostorderTraversal(AVLTreeEntry<DepthPacketPair, unsigned int>*, void (AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::*)(AVLTreeEntry<DepthPacketPair, unsigned int>*));
//     void CompareNodes(AVLTreeNode*, AVLTreeNode*);
//     void CompareKey(void*, AVLTreeNode*);
//     void AllocateEntry(void*, void*);
// };

// class DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>
// {
// public:
//     void DeleteEntry(DLListEntry<const glModelPacket*>*);
// };

// class nlWalkDLRing<DLListEntry<const glModelPacket*>, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>>(DLListEntry<const glModelPacket*>*, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>*, void (WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>
// {
// public:
//     void *)(DLListEntry<const glModelPacket*>*));
// };

// class nlWalkDLRing<DLListEntry<const glModelPacket*>, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>>(DLListEntry<const glModelPacket*>*, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>*, void (DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>
// {
// public:
//     void *)(DLListEntry<const glModelPacket*>*));
// };

// class nlWalkRing<DLListEntry<const glModelPacket*>, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>>(DLListEntry<const glModelPacket*>*, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>*, void (DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>
// {
// public:
//     void *)(DLListEntry<const glModelPacket*>*));
// };

// class nlWalkRing<DLListEntry<const glModelPacket*>, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>>(DLListEntry<const glModelPacket*>*, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>*, void (WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>
// {
// public:
//     void *)(DLListEntry<const glModelPacket*>*));
// };

#endif // _GLRENDERLIST_H_
