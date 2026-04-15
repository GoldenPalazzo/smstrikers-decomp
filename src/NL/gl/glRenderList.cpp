#include "NL/gl/glRenderList.h"
#include "NL/gl/glAppAttach.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glModify.h"
#include "NL/platvmath.h"
#include "NL/nlSlotPool.h"

/**
 * Offset/Address/Size: 0x0 | 0x801D92C0 | size: 0x36C
 * TODO: 97.6% match - pModel/layer register swap (r25/r26), pPacket r31 vs r29,
 *       extra mr before rlwimi, numPackets/index register rotation
 */
s32 GLRenderList::AttachModel(const glModel* pModel, unsigned long layer)
{
    glModelPacket* pPacket;
    DepthPacketPair pair;
    nlMatrix4 m;
    GLDepthPacketTree* pTree;
    AVLTreeNode* existingNode;
    unsigned int* pCount;
    unsigned long sortKey;
    nlVector3 out;
    GLPacketList* pList;
    DLListEntry<const glModelPacket*>* p;

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
            for (unsigned long i = 0; i < numPackets; i++)
            {
                glModelPacket* newPacket = gl_Modify(pPacket);
                glplatAttachPacket((eGLView)m_unk_0x00, layer, newPacket == NULL ? pPacket : newPacket);
                pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
            }
        }
        else
        {
            unsigned long numPackets = pModel->numPackets;
            for (unsigned long i = 0; i < numPackets; i++)
            {
                glplatAttachPacket((eGLView)m_unk_0x00, layer, pPacket);
                pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
            }
        }
    }
    else if (m_unk_0x04 == GLVSort_TransformedDepth || m_unk_0x04 == GLVSort_TransformedMatrixDepth)
    {
        glGetIdentityMatrix();
        glGetMatrix((unsigned long)glViewGetViewMatrix((eGLView)m_unk_0x00), m);
        unsigned long numPackets = pModel->numPackets;

        for (unsigned long i = 0; i < numPackets; i++)
        {
            pair.packet = pPacket;

            if (m_unk_0x04 == GLVSort_TransformedMatrixDepth)
            {
                sortKey = uDepthInsertNumber;
                nlMatrix4 packetMatrix;
                glGetMatrix(pPacket->state.matrix, packetMatrix);
                nlMultPosVectorMatrix(out, *(nlVector3*)&packetMatrix.m[3][0], m);
                sortKey = ((s32)(-out.f.z * 256.0f) << 12) | (sortKey & 0xFFF);
                pair.sortKey = sortKey;
                uDepthInsertNumber++;
            }
            else
            {
                nlMatrix4 packetMatrix2;
                glGetMatrix(pPacket->state.matrix, packetMatrix2);
                nlVector3 out2;
                nlMultPosVectorMatrix(out2, *(nlVector3*)pPacket->streams->address, m);
                pair.sortKey = (s32)(-out2.f.z * 256.0f);
            }

            pTree = depthPacketTree;
            const unsigned int& one = 1;
            pTree->AddAVLNode((AVLTreeNode**)&pTree->m_Root, &pair, (void*)&one, &existingNode, pTree->m_NumElements);

            if (existingNode == NULL)
            {
                pTree->m_NumElements++;
                pCount = NULL;
            }
            else
            {
                pCount = &((AVLTreeEntry<DepthPacketPair, unsigned int>*)existingNode)->value;
            }

            if (pCount != NULL)
            {
                *pCount = *pCount + 1;
            }

            pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
        }
    }
    else if (m_unk_0x04 == GLVSort_Reverse)
    {
        unsigned long numPackets = pModel->numPackets;
        for (unsigned long i = 0; i < numPackets; i++)
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
                p->m_data = modified;
            }

            nlDLRingAddStart(&pList->m_Head, p);
            pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
        }
    }
    else if (m_unk_0x04 == GLVSort_None)
    {
        unsigned long numPackets = pModel->numPackets;
        for (unsigned long i = 0; i < numPackets; i++)
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
                p->m_data = modified;
            }

            nlDLRingAddEnd(&pList->m_Head, p);
            pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
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
    AVLTreeNode* existingNode;
    pTree->AddAVLNode((AVLTreeNode**)&pTree->m_Root, &pKey, (void*)&one, &existingNode, pTree->m_NumElements);

    unsigned int* pCount;
    if (existingNode == NULL)
    {
        pTree->m_NumElements++;
        pCount = NULL;
    }
    else
    {
        pCount = &((AVLTreeEntry<const glModelPacket*, unsigned int>*)existingNode)->value;
    }

    if (pCount != NULL)
    {
        (*pCount)++;
    }
}

/**
 * Offset/Address/Size: 0x440 | 0x801D9700 | size: 0x290
 * TODO: 87.27% match - callback/member-pointer temporaries still land in different
 * stack slots than target in texture/depth and list-walk branches.
 */
void GLRenderList::Iterate(eGLView view, void (*cb)(eGLView, unsigned long, const glModelPacket*))
{
    typedef WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager> PacketWalkHelper;
    typedef void (PacketCallbackManager::*TexCallbackType)(const glModelPacket* const&, unsigned int*);
    typedef void (PacketCallbackManager::*DepthCallbackType)(const DepthPacketPair&, unsigned int*);
    typedef void (PacketWalkHelper::*WalkCallbackType)(DLListEntry<const glModelPacket*>*);

    PacketCallbackManager pkCallback;
    TexCallbackType texCb;
    DepthCallbackType depthCb;
    WalkCallbackType walkCb;
    PacketWalkHelper helper;
    WalkCallbackType walkCb2;
    PacketWalkHelper helper2;

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
        GLPacketList* pList = packetList;
        if (pList->m_Head != NULL)
        {
            cb(view, 1, NULL);
            helper.m_CBClass = &pkCallback;
            helper.m_CB = &PacketCallbackManager::ListCallback;
            walkCb = &PacketWalkHelper::Callback;
            nlWalkDLRing(pList->m_Head, &helper, walkCb);
        }
    }
    else
    {
        GLPacketList* pList2 = packetList;
        if (pList2->m_Head != NULL)
        {
            cb(view, 1, NULL);
            helper2.m_CBClass = &pkCallback;
            helper2.m_CB = &PacketCallbackManager::ListCallback;
            walkCb2 = &PacketWalkHelper::Callback;
            nlWalkDLRing(pList2->m_Head, &helper2, walkCb2);
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

static unsigned long glv_UserDataChanged;
static unsigned long glv_MaterialChanged;
static unsigned long glv_UserStateKeyChanged;
static unsigned long glv_RasterChanged;
static unsigned long glv_TextureStateChanged;
static unsigned long glv_MatrixChanged;
static unsigned long glv_TexConfigChanged;
static unsigned long glv_TextureChanged;
static unsigned long glv_StreamsChanged;

/**
 * Offset/Address/Size: 0x748 | 0x801D9A08 | size: 0x298
 * TODO: 92.38% match - MWCC strength-reduction optimization for texture array
 * (addi r5,r30,4 pointer iteration) not reproducible in decomp.me compiler.
 */
void PacketCallbackManager::DoCallback(const glModelPacket* p, unsigned int count)
{
    PacketCallbackManagerLayout* self = (PacketCallbackManagerLayout*)this;
    unsigned long flags = 0;

    if (p->state.program != self->m_LastProgram)
    {
        flags |= 0x86;
        self->m_LastProgram = p->state.program;
    }

    if (p->userData == 0)
    {
        if (self->m_LastUserdata != 0)
        {
            unsigned long userDataChanged = glv_UserDataChanged;
            self->m_LastUserdata = p->userData;
            flags |= userDataChanged;
        }
    }
    else
    {
        unsigned long userDataChanged = glv_UserDataChanged;
        self->m_LastUserdata = p->userData;
        flags |= userDataChanged;
    }

    if (p->materialset != self->m_LastMaterialSet)
    {
        unsigned long materialChanged = glv_MaterialChanged;
        flags |= materialChanged;
    }

    if (p->state.userStateKey != self->m_LastUserStateKey)
    {
        unsigned long userStateKeyChanged = glv_UserStateKeyChanged;
        self->m_LastUserStateKey = p->state.userStateKey;
        flags |= userStateKeyChanged;
    }

    if (p->state.raster != self->m_LastRaster)
    {
        unsigned long rasterChanged = glv_RasterChanged;
        self->m_LastRaster = p->state.raster;
        flags |= rasterChanged;
    }

    unsigned long textureStateLow = ((const unsigned long*)&p->state.texturestate)[0];
    unsigned long lastTextureStateLow = ((const unsigned long*)&self->m_LastTextureState)[0];
    unsigned long textureStateHigh = ((const unsigned long*)&p->state.texturestate)[1];
    unsigned long lastTextureStateHigh = ((const unsigned long*)&self->m_LastTextureState)[1];
    unsigned long textureStateDiffLow = textureStateLow ^ lastTextureStateLow;
    unsigned long textureStateDiffHigh = textureStateHigh ^ lastTextureStateHigh;
    if ((textureStateDiffHigh | textureStateDiffLow) != 0)
    {
        ((unsigned long*)&self->m_LastTextureState)[1] = textureStateHigh;
        unsigned long textureStateChanged = glv_TextureStateChanged;
        ((unsigned long*)&self->m_LastTextureState)[0] = textureStateLow;
        flags |= textureStateChanged;
    }

    if (p->state.matrix != self->m_LastMatrix)
    {
        unsigned long matrixChanged = glv_MatrixChanged;
        self->m_LastMatrix = p->state.matrix;
        flags |= matrixChanged;
    }

    unsigned long texConfig = p->state.texconfig;
    unsigned long lastTexConfig = self->m_LastTexconfig;
    if (texConfig != lastTexConfig)
    {
        unsigned long texConfigChanged = glv_TexConfigChanged;
        unsigned long textureChanged = glv_TextureChanged;
        flags |= texConfigChanged;
        self->m_LastTexconfig = texConfig;
        flags |= textureChanged;
    }

    {
        unsigned long textureChanged = glv_TextureChanged;
        int i;
        for (i = 0; i < 6; i++)
        {
            if (p->state.texture[i] != self->m_LastTexture[i])
            {
                self->m_LastTexture[i] = p->state.texture[i];
                flags |= textureChanged;
            }
        }
    }

    glModelStream* lastStreams = self->m_LastStreams;
    unsigned int numStreams = p->numStreams;
    unsigned int streamChanged;

    if (self->m_LastNumStreams != numStreams)
    {
        streamChanged = 1;
    }
    else
    {
        glModelStream* streams = p->streams;

        for (unsigned int i = numStreams; i > 0; i--)
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
        self->m_LastNumStreams = numStreams;
        glModelStream* streams = p->streams;
        unsigned long streamsChanged = glv_StreamsChanged;
        self->m_LastStreams = streams;
        flags |= streamsChanged;
    }

    unsigned int stage = flags | 0x800;

    while (count != 0)
    {
        self->m_Cb(self->m_View, stage, p);
        count--;
    }
}

/**
 * Offset/Address/Size: 0x9E0 | 0x801D9CA0 | size: 0x108
 * TODO: 93.5% match - MWCC emits addi r3,r3,8 + lwz 8(r3) for the second
 * texture check instead of addi r3,r3,4 + lwz 0xC(r3), shifting later offsets
 */
bool GLRenderList::IsEmpty() const
{
    if (m_unk_0x04 == GLVSort_Texture)
    {
        for (int layer = 1; layer <= 7; layer++)
        {
            if (texPacketTree[layer - 1]->m_Root != NULL)
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

// /**
//  * Offset/Address/Size: 0x0 | 0x801DA078 | size: 0x34
//  */
// void WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>::Callback(DLListEntry<const glModelPacket*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801DA0AC | size: 0x10
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::DeleteEntry(AVLTreeEntry<const glModelPacket*, unsigned int>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x10 | 0x801DA0BC | size: 0x10
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::DeleteEntry(AVLTreeEntry<DepthPacketPair, unsigned int>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x20 | 0x801DA0CC | size: 0x44
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::Walk<PacketCallbackManager>(PacketCallbackManager*, void (PacketCallbackManager::*)(const DepthPacketPair&, unsigned int*))
// {
// }

// /**
//  * Offset/Address/Size: 0x64 | 0x801DA110 | size: 0x2CC
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::InorderWalk<PacketCallbackManager>(AVLTreeEntry<DepthPacketPair, unsigned int>*, PacketCallbackManager*, void (PacketCallbackManager::*)(const DepthPacketPair&, unsigned int*))
// {
// }

// /**
//  * Offset/Address/Size: 0x330 | 0x801DA3DC | size: 0x8
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x338 | 0x801DA3E4 | size: 0x44
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::Walk<PacketCallbackManager>(PacketCallbackManager*, void (PacketCallbackManager::*)(const glModelPacket* const&, unsigned int*))
// {
// }

// /**
//  * Offset/Address/Size: 0x37C | 0x801DA428 | size: 0x2CC
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::InorderWalk<PacketCallbackManager>(AVLTreeEntry<const glModelPacket*, unsigned int>*, PacketCallbackManager*, void (PacketCallbackManager::*)(const glModelPacket* const&, unsigned int*))
// {
// }

// /**
//  * Offset/Address/Size: 0x648 | 0x801DA6F4 | size: 0x8
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x650 | 0x801DA6FC | size: 0x58
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x6A8 | 0x801DA754 | size: 0x64
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::DestroyTree(void (AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::*)(AVLTreeEntry<DepthPacketPair, unsigned int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x70C | 0x801DA7B8 | size: 0x758
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::PostorderTraversal(AVLTreeEntry<DepthPacketPair, unsigned int>*, void (AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::*)(AVLTreeEntry<DepthPacketPair, unsigned int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xE64 | 0x801DAF10 | size: 0x58
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0xEBC | 0x801DAF68 | size: 0x64
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::DestroyTree(void (AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::*)(AVLTreeEntry<const glModelPacket*, unsigned int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xF20 | 0x801DAFCC | size: 0x758
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::PostorderTraversal(AVLTreeEntry<const glModelPacket*, unsigned int>*, void (AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::*)(AVLTreeEntry<const glModelPacket*, unsigned int>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x1678 | 0x801DB724 | size: 0x50
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x16C8 | 0x801DB774 | size: 0x50
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1718 | 0x801DB7C4 | size: 0xBC
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x17D4 | 0x801DB880 | size: 0xBC
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1890 | 0x801DB93C | size: 0xA0
//  */
// void AVLTreeBase<const glModelPacket*, unsigned int, BasicSlotPool<AVLTreeEntry<const glModelPacket*, unsigned int>>, TextureTreeCompare>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x1930 | 0x801DB9DC | size: 0xA8
//  */
// void AVLTreeBase<DepthPacketPair, unsigned int, BasicSlotPool<AVLTreeEntry<DepthPacketPair, unsigned int>>, DepthTreeCompare>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801DBA84 | size: 0x10
//  */
// void DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>::DeleteEntry(DLListEntry<const glModelPacket*>*)
// {
// }

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

// /**
//  * Offset/Address/Size: 0x78 | 0x801DBB0C | size: 0x3C
//  */
// void nlDLRingAddEnd<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB4 | 0x801DBB48 | size: 0x38
//  */
// void nlDLRingAddStart<DLListEntry<const glModelPacket*>>(DLListEntry<const glModelPacket*>**, DLListEntry<const glModelPacket*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801DBB80 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<const glModelPacket*>, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>>(DLListEntry<const glModelPacket*>*, DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>*, void (DLListContainerBase<const glModelPacket*, BasicSlotPool<DLListEntry<const glModelPacket*>>>::*)(DLListEntry<const glModelPacket*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x60 | 0x801DBBE0 | size: 0x60
//  */
// void nlWalkRing<DLListEntry<const glModelPacket*>, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>>(DLListEntry<const glModelPacket*>*, WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>*, void (WalkHelper<const glModelPacket*, DLListEntry<const glModelPacket*>, PacketCallbackManager>::*)(DLListEntry<const glModelPacket*>*))
// {
// }
