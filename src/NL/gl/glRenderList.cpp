#include "NL/gl/glRenderList.h"
#include "NL/nlAVLTree.h"
#include "NL/nlDLListContainer.h"

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

#include "NL/WalkHelper.h"
#include "Game/GL/GLRenderBuffer.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glModify.h"
#include "NL/platvmath.h"
#include "NL/nlSlotPool.h"

extern GLRenderBuffer glRenderBuffer;

/**
 * Offset/Address/Size: 0x0 | 0x801D92C0 | size: 0x36C
 * TODO: 97.9% match - pModel/layer register swap (r25/r26), pPacket r31 vs r29,
 *       extra mr before rlwimi
 */
s32 GLRenderList::AttachModel(const glModel* pModel, unsigned long layer)
{
    glModelPacket* pPacket;

    if ((s32)m_unk_0x00 < 0x1A && glRenderBuffer.m_bEnabled && glRenderBuffer.m_bExclusive && !glRenderBuffer.m_bSending)
    {
        return 1;
    }

    pPacket = pModel->packets;

    if (m_unk_0x04 == GLVSort_Texture)
    {
        if ((s32)gl_ModifyGetNum() > 0)
        {
            unsigned long numPackets = pModel->numPackets;
            for (unsigned long index = 0; index < numPackets; index++, pPacket = (glModelPacket*)((u8*)pPacket + 0x4A))
            {
                glModelPacket* newPacket = gl_Modify(pPacket);
                glplatAttachPacket((eGLView)m_unk_0x00, layer, newPacket == NULL ? pPacket : newPacket);
            }
        }
        else
        {
            unsigned long numPackets = pModel->numPackets;
            for (unsigned long index = 0; index < numPackets; index++, pPacket = (glModelPacket*)((u8*)pPacket + 0x4A))
            {
                glplatAttachPacket((eGLView)m_unk_0x00, layer, pPacket);
            }
        }
    }
    else if (m_unk_0x04 == GLVSort_TransformedDepth || m_unk_0x04 == GLVSort_TransformedMatrixDepth)
    {
        DepthPacketPair pair;
        nlMatrix4 m;
        GLDepthPacketTree* pTree;
        unsigned int* pCount;
        unsigned long sortKey;
        nlVector3 out;
        glGetIdentityMatrix();
        glGetMatrix((unsigned long)glViewGetViewMatrix((eGLView)m_unk_0x00), m);
        unsigned long numPackets = pModel->numPackets;

        for (unsigned long index = 0; index < numPackets; index++, pPacket = (glModelPacket*)((u8*)pPacket + 0x4A))
        {
            pair.packet = pPacket;

            if (m_unk_0x04 == GLVSort_TransformedMatrixDepth)
            {
                sortKey = uDepthInsertNumber;
                nlMatrix4 packetMatrix;
                glGetMatrix(pPacket->state.matrix, packetMatrix);
                nlMultPosVectorMatrix(out, *(nlVector3*)&packetMatrix.m[3][0], m);
                sortKey = ((s32)(-out.f.z * 100.0f) << 12) | (sortKey & 0xFFF);
                pair.sortKey = sortKey;
                uDepthInsertNumber++;
            }
            else
            {
                nlMatrix4 packetMatrix2;
                glGetMatrix(pPacket->state.matrix, packetMatrix2);
                nlVector3 out2;
                nlMultPosVectorMatrix(out2, *(nlVector3*)pPacket->streams->address, m);
                pair.sortKey = (s32)(-out2.f.z * 2147483648.0f);
            }

            pTree = depthPacketTree;
            const unsigned int& one = 1;
            pCount = pTree->Add(pair, one);

            if (pCount != NULL)
            {
                *pCount = *pCount + 1;
            }
        }
    }
    else if (m_unk_0x04 == GLVSort_Reverse)
    {
        GLPacketList* pList;
        DLListEntry<const glModelPacket*>* p;
        unsigned long numPackets = pModel->numPackets;
        for (unsigned long index = 0; index < numPackets; index++, pPacket = (glModelPacket*)((u8*)pPacket + 0x4A))
        {
            glModelPacket* modified = glplatModifyPacket((eGLView)m_unk_0x00, pPacket);
            pList = packetList;
            p = NULL;

            if (pList->m_Allocator.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)pList, 0xC);
            }

            DLListEntry<const glModelPacket*>* entry = (DLListEntry<const glModelPacket*>*)pList->m_Allocator.m_FreeList;
            if (entry != NULL)
            {
                p = entry;
                pList->m_Allocator.m_FreeList = (SlotPoolEntry*)entry->m_next;
            }

            if (p != NULL)
            {
                p->m_next = NULL;
                p->m_prev = NULL;
                p->entry = modified;
            }

            nlDLRingAddStart(&pList->m_Head, p);
        }
    }
    else if (m_unk_0x04 == GLVSort_None)
    {
        GLPacketList* pList;
        DLListEntry<const glModelPacket*>* p;
        unsigned long numPackets = pModel->numPackets;
        for (unsigned long index = 0; index < numPackets; index++, pPacket = (glModelPacket*)((u8*)pPacket + 0x4A))
        {
            glModelPacket* modified = glplatModifyPacket((eGLView)m_unk_0x00, pPacket);
            pList = packetList;
            p = NULL;

            if (pList->m_Allocator.m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock((SlotPoolBase*)pList, 0xC);
            }

            DLListEntry<const glModelPacket*>* entry = (DLListEntry<const glModelPacket*>*)pList->m_Allocator.m_FreeList;
            if (entry != NULL)
            {
                p = entry;
                pList->m_Allocator.m_FreeList = (SlotPoolEntry*)entry->m_next;
            }

            if (p != NULL)
            {
                p->m_next = NULL;
                p->m_prev = NULL;
                p->entry = modified;
            }

            nlDLRingAddEnd(&pList->m_Head, p);
        }
    }

    return 1;
}

/**
 * Offset/Address/Size: 0x36C | 0x801D962C | size: 0xD4
 */
void gl_ViewAttachPacket(eGLView view, unsigned long layer, const glModelPacket* pPacket)
{
    GLRenderList* pList = gl_ViewGetRenderList(view);
    const glModelPacket* pKey = pPacket;

    if ((s32)pList->m_unk_0x00 < 0x1A && glRenderBuffer.m_bEnabled && glRenderBuffer.m_bExclusive && !glRenderBuffer.m_bSending)
    {
        return;
    }

    GLTexturePacketTree* pTree = pList->texPacketTree[layer];
    const unsigned int& one = 1;
    unsigned int* pCount = pTree->Add(pKey, one);

    if (pCount != NULL)
    {
        (*pCount)++;
    }
}

typedef WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager> GLRenderPacketWalkHelper;
typedef void (GLRenderPacketWalkHelper::*GLRenderPacketWalkCallback)(DLListEntry<const glModelPacket*>*);

static inline void WalkPacketList(GLPacketList* list, PacketCallbackManager& pkCallback)
{
    GLRenderPacketWalkHelper helper;
    GLRenderPacketWalkCallback walkCb;
    helper.m_CBClass = &pkCallback;
    helper.m_CB = &PacketCallbackManager::ListCallback;
    walkCb = &GLRenderPacketWalkHelper::Callback;
    nlWalkDLRing(list->m_Head, &helper, walkCb);
}

/**
 * Offset/Address/Size: 0x440 | 0x801D9700 | size: 0x290
 */
void GLRenderList::Iterate(eGLView view, void (*cb)(eGLView, unsigned long, const glModelPacket*))
{
    typedef void (PacketCallbackManager::*TexCallbackType)(const glModelPacket* const&, unsigned int*);
    typedef void (PacketCallbackManager::*DepthCallbackType)(const DepthPacketPair&, unsigned int*);

    PacketCallbackManager pkCallback;
    TexCallbackType texCb;
    DepthCallbackType depthCb;

    pkCallback.m_View = view;
    pkCallback.m_Cb = cb;
    pkCallback.m_LastProgram = (unsigned long)-1;
    pkCallback.m_LastRaster = (unsigned long)-1;
    pkCallback.m_LastTextureState = 0xFFFFFFFF;
    pkCallback.m_LastMatrix = (unsigned long)-1;
    pkCallback.m_LastTexconfig = (unsigned long)-1;
    pkCallback.m_LastUserdata = (unsigned long)-1;
    pkCallback.m_LastUserStateKey = (unsigned long)-1;
    pkCallback.m_LastNumStreams = 0;
    pkCallback.m_LastStreams = NULL;
    pkCallback.m_LastMaterialSet = (unsigned long)-1;
    pkCallback.m_LastTexture[0] = (unsigned long)-1;
    pkCallback.m_LastTexture[1] = (unsigned long)-1;
    pkCallback.m_LastTexture[2] = (unsigned long)-1;
    pkCallback.m_LastTexture[3] = (unsigned long)-1;
    pkCallback.m_LastTexture[4] = (unsigned long)-1;
    pkCallback.m_LastTexture[5] = (unsigned long)-1;

    if (m_unk_0x04 == GLVSort_Texture)
    {
        for (int layer = 0; layer < 7; layer++)
        {
            if (texPacketTree[layer]->m_Root != NULL)
            {
                cb(view, 1, NULL);
                texCb = &PacketCallbackManager::TexCallback;
                texPacketTree[layer]->Walk(&pkCallback, texCb);
            }
        }
    }
    else if (m_unk_0x04 == GLVSort_TransformedDepth || m_unk_0x04 == GLVSort_TransformedMatrixDepth)
    {
        if (depthPacketTree->m_Root != NULL)
        {
            cb(view, 1, NULL);
            depthCb = &PacketCallbackManager::DepthCallback;
            depthPacketTree->Walk(&pkCallback, depthCb);
        }
    }
    else if (m_unk_0x04 == GLVSort_Reverse)
    {
        if (packetList->m_Head != NULL)
        {
            cb(view, 1, NULL);
            WalkPacketList(packetList, pkCallback);
        }
    }
    else
    {
        if (packetList->m_Head != NULL)
        {
            cb(view, 1, NULL);
            WalkPacketList(packetList, pkCallback);
        }
    }
}

/**
 * Offset/Address/Size: 0x6D0 | 0x801D9990 | size: 0x28
 */
void PacketCallbackManager::ListCallback(const glModelPacket** pPacketPtr)
{
    DoCallback(*pPacketPtr, 1);
}

/**
 * Offset/Address/Size: 0x6F8 | 0x801D99B8 | size: 0x28
 */
void PacketCallbackManager::DepthCallback(const DepthPacketPair& key, unsigned int* count)
{
    DoCallback(key.packet, *count);
}

/**
 * Offset/Address/Size: 0x720 | 0x801D99E0 | size: 0x28
 */
void PacketCallbackManager::TexCallback(const glModelPacket* const& key, unsigned int* count)
{
    DoCallback(key, *count);
}

struct PacketCallbackManagerLayout
{
    /* 0x00 */ eGLView m_View;
    /* 0x04 */ glViewPacketCallback m_Cb;
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
}; // total size: 0x50

static unsigned long glv_UserDataChanged __attribute__((section(".sdata2"))) = 0x100;
static unsigned long glv_MaterialChanged __attribute__((section(".sdata2"))) = 0x400;
static unsigned long glv_UserStateKeyChanged __attribute__((section(".sdata2"))) = 0x200;
static unsigned long glv_RasterChanged __attribute__((section(".sdata2"))) = 0x08;
static unsigned long glv_TextureStateChanged __attribute__((section(".sdata2"))) = 0x10;
static unsigned long glv_MatrixChanged __attribute__((section(".sdata2"))) = 0x20;
static unsigned long glv_TexConfigChanged __attribute__((section(".sdata2"))) = 0x80;
static unsigned long glv_TextureChanged __attribute__((section(".sdata2"))) = 0x4;
static unsigned long glv_StreamsChanged __attribute__((section(".sdata2"))) = 0x40;

/**
 * Offset/Address/Size: 0x748 | 0x801D9A08 | size: 0x298
 * TODO: 98.61% match - register swaps remain in texconfig, texture,
 * and stream update blocks around 0x801D9B1C-0x801D9C50.
 */
void PacketCallbackManager::DoCallback(const glModelPacket* p, unsigned int count)
{
    unsigned long flags = 0;

    if (p->state.program != m_LastProgram)
    {
        flags |= 0x86;
        m_LastProgram = p->state.program;
    }

    if (p->userData == 0)
    {
        if (m_LastUserdata != 0)
        {
            unsigned long userDataChanged = glv_UserDataChanged;
            m_LastUserdata = p->userData;
            flags |= userDataChanged;
        }
    }
    else
    {
        unsigned long userDataChanged = glv_UserDataChanged;
        m_LastUserdata = p->userData;
        flags |= userDataChanged;
    }

    if (p->materialset != m_LastMaterialSet)
    {
        unsigned long materialChanged = glv_MaterialChanged;
        flags |= materialChanged;
    }

    if (p->state.userStateKey != m_LastUserStateKey)
    {
        unsigned long userStateKeyChanged = glv_UserStateKeyChanged;
        m_LastUserStateKey = p->state.userStateKey;
        flags |= userStateKeyChanged;
    }

    if (p->state.raster != m_LastRaster)
    {
        unsigned long rasterChanged = glv_RasterChanged;
        m_LastRaster = p->state.raster;
        flags |= rasterChanged;
    }

    unsigned long textureStateLow = ((const unsigned long*)&p->state.texturestate)[0];
    unsigned long lastTextureStateLow = ((const unsigned long*)&m_LastTextureState)[0];
    unsigned long textureStateHigh = ((const unsigned long*)&p->state.texturestate)[1];
    unsigned long lastTextureStateHigh = ((const unsigned long*)&m_LastTextureState)[1];
    unsigned long textureStateDiffLow = textureStateLow ^ lastTextureStateLow;
    unsigned long textureStateDiffHigh = textureStateHigh ^ lastTextureStateHigh;
    if ((textureStateDiffHigh | textureStateDiffLow) != 0)
    {
        ((unsigned long*)&m_LastTextureState)[1] = textureStateHigh;
        unsigned long textureStateChanged = glv_TextureStateChanged;
        ((unsigned long*)&m_LastTextureState)[0] = textureStateLow;
        flags |= textureStateChanged;
    }

    if (p->state.matrix != m_LastMatrix)
    {
        unsigned long matrixChanged = glv_MatrixChanged;
        m_LastMatrix = p->state.matrix;
        flags |= matrixChanged;
    }

    if (p->state.texconfig != m_LastTexconfig)
    {
        unsigned long texConfigChanged = glv_TexConfigChanged;
        unsigned long textureChanged = glv_TextureChanged;
        flags |= texConfigChanged;
        m_LastTexconfig = p->state.texconfig;
        flags |= textureChanged;
    }

    {
        unsigned long textureChanged = glv_TextureChanged;
        unsigned long texture;
        int i;
        for (i = 0; i < 6; i++)
        {
            if (m_LastTexture[i] != (texture = p->state.texture[i]))
            {
                m_LastTexture[i] = texture;
                flags |= textureChanged;
            }
        }
    }

    glModelStream* lastStreams = m_LastStreams;
    glModelStream* streams;
    int numStreams;
    unsigned int streamChanged;

    if (m_LastNumStreams != (numStreams = p->numStreams))
    {
        streamChanged = 1;
    }
    else
    {
        streams = p->streams;

        for (int i = numStreams; i > 0; i--)
        {
            if (streams->address != lastStreams->address)
            {
                streamChanged = 1;
                goto stream_compare_done;
            }
            lastStreams = (glModelStream*)((u8*)lastStreams + 6);
            streams = (glModelStream*)((u8*)streams + 6);
        }

        streamChanged = 0;
    }

stream_compare_done:
    if ((streamChanged & 0xFF) != 0)
    {
        m_LastNumStreams = numStreams;
        glModelStream* streams = p->streams;
        unsigned long streamsChanged = glv_StreamsChanged;
        m_LastStreams = streams;
        flags |= streamsChanged;
    }

    unsigned int stage = flags | 0x800;

    while (count != 0)
    {
        m_Cb(m_View, stage, p);
        count--;
    }
}

/**
 * Offset/Address/Size: 0x9E0 | 0x801D9CA0 | size: 0x108
 */
bool GLRenderList::IsEmpty() const
{
    if (m_unk_0x04 == GLVSort_Texture)
    {
        for (int layer = 0; layer < 7; layer++)
        {
            if (texPacketTree[layer]->m_Root != NULL)
            {
                return false;
            }
        }
        return true;
    }
    if (m_unk_0x04 == GLVSort_TransformedDepth || m_unk_0x04 == GLVSort_TransformedMatrixDepth)
    {
        return depthPacketTree->m_Root == NULL;
    }
    return packetList->m_Head == NULL;
}

/**
 * Offset/Address/Size: 0xAE8 | 0x801D9DA8 | size: 0x80
 */
void GLRenderList::Compact()
{
    Clear();

    for (int i = 0; i < 7; i++)
    {
        u32* ptr = (u32*)((u8*)this + 0x0C + i * 4);
        SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)(*ptr + 4), 0x14);
    }

    u32 poolAddr2 = *(u32*)((u8*)this + 0x28);
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)(poolAddr2 + 4), 0x18);

    u32 poolAddr3 = *(u32*)((u8*)this + 0x2C);
    SlotPoolBase::BaseFreeBlocks((SlotPoolBase*)poolAddr3, 0x0C);
}

/**
 * Offset/Address/Size: 0xB68 | 0x801D9E28 | size: 0xA0
 */
typedef DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*> > > GLPacketListBase;

void GLRenderList::Clear()
{
    GLPacketListBase* pList;
    int i;
    for (i = 0; i < 7; i++)
    {
        texPacketTree[i]->Clear();
    }
    depthPacketTree->Clear();

    pList = packetList;
    nlWalkDLRing(pList->m_Head, pList, &GLPacketListBase::DeleteEntry);
    pList->m_Head = NULL;
    uDepthInsertNumber = 0;
}

/**
 * Offset/Address/Size: 0xC08 | 0x801D9EC8 | size: 0x1B0
 */
GLRenderList::GLRenderList()
{
    int layer;
    uDepthInsertNumber = 0;

    for (layer = 0; layer < 7; layer++)
    {
        GLTexturePacketTree* tree = new (nlMalloc(0x28, 8, false)) GLTexturePacketTree(16, 16);
        texPacketTree[layer] = tree;
    }

    GLDepthPacketTree* dTree = new (nlMalloc(0x28, 8, false)) GLDepthPacketTree(16, 16);
    depthPacketTree = dTree;

    GLPacketList* list = new (nlMalloc(0x1C, 8, false)) GLPacketList();
    packetList = list;
}

/**
 * Offset/Address/Size: 0x0 | 0x801DBA94 | size: 0x3C
 * TODO: 96% match - stw LR save scheduling differs by one slot
 * (target emits first lwz from callbackFunc before stw r0,0x24(r1)).
 */
typedef WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager> PacketWalkHelper_t;
template void nlWalkDLRing<DLListEntry<const glModelPacket*>, PacketWalkHelper_t>(
    DLListEntry<const glModelPacket*>* head,
    PacketWalkHelper_t* callback,
    void (PacketWalkHelper_t::*callbackFunc)(DLListEntry<const glModelPacket*>*));

/**
 * Offset/Address/Size: 0x3C | 0x801DBAD0 | size: 0x3C
 * TODO: 96% match - stw LR save scheduling differs by one slot
 * (target emits first lwz from callbackFunc before stw r0,0x24(r1)).
 */
template void nlWalkDLRing<DLListEntry<const glModelPacket*>, GLPacketListBase>(
    DLListEntry<const glModelPacket*>* head,
    GLPacketListBase* callback,
    void (GLPacketListBase::*callbackFunc)(DLListEntry<const glModelPacket*>*));
