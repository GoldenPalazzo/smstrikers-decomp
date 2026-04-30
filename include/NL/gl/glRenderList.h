#ifndef _GLRENDERLIST_H_
#define _GLRENDERLIST_H_

// void gl_ViewAttachPacket(eGLView, unsigned long, const glModelPacket*);
// void nlDLRingAddEnd<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*);
// void nlDLRingAddStart<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*);

#include "NL/gl/glView.h"
#include "NL/nlAVLTreeSlotPool.h"
#include "NL/nlDLListSlotPool.h"
#include "Game/GL/GLRenderBuffer.h"

class glModel;
class TextureTreeCompare;

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

class DepthTreeCompare;

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
public:
    GLPacketList()
        : nlDLListSlotPool<const glModelPacket*>()
    {
    }
    GLPacketList(int initial, int delta)
        : nlDLListSlotPool<const glModelPacket*>(initial, delta)
    {
    }
    ~GLPacketList() { }
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

template <typename KeyType, typename EntryType, typename CallbackType>
class WalkHelper
{
public:
    CallbackType* m_CBClass;
    void (CallbackType::*m_CB)(KeyType*);
    void Callback(EntryType*);
};

template <typename KeyType, typename EntryType, typename CallbackType>
void WalkHelper<KeyType, EntryType, CallbackType>::Callback(EntryType* listEntry)
{
    (m_CBClass->*m_CB)(&listEntry->m_data);
}

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
