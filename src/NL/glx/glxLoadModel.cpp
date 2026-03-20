#include "NL/glx/glxLoadModel.h"
#include "NL/glx/glxMemory.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/gl/glTexture.h"
#include "NL/gl/glUserData.h"
#include "NL/glx/glxDisplayList.h"
#include "NL/glx/glxTexture.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlString.h"
#include "NL/nlDLRing.h"
#include "NL/nlMemory.h"
#include "NL/platvmath.h"
#include "Game/GL/GLInventory.h"
#include "Game/GL/GLTextureAnim.h"
#include "Game/GL/GLVertexAnim.h"
#include "Game/GL/GLMaterial.h"
#include "Game/GL/ShaderSkinMesh.h"
#include "Game/SAnim.h"
#include "dolphin/os/OSCache.h"
#include <string.h>

extern GLInventory glInventory;

static bool glIgnoreDuplicateModels;
static const char glxModelPath[] = "smstk";
static const char glxSkinExt[] = ".skinmesh";

/**
 * Offset/Address/Size: 0x0 | 0x801BFC20 | size: 0x24
 */
glModel* glplatEndLoadModel(void* data, unsigned long size, unsigned long* pNumModels)
{
    return glxLoadModelFromMemory((char*)data, size, pNumModels, false);
}

/**
 * Offset/Address/Size: 0x24 | 0x801BFC44 | size: 0xA8
 */
bool glplatBeginLoadModel(const char* filename, void (*callback)(void*, unsigned long, void*), void* userData)
{
    char fullname[256];
    nlStrNCat(fullname, glxModelPath, filename, 256);

    if (userData == NULL)
    {
        if (!nlLoadEntireFileAsync(fullname, callback, userData, 32, AllocateEnd))
        {
            return false;
        }
    }
    else
    {
        if (!nlLoadEntireFileAsync(fullname, callback, userData, 32, (eAllocType)23))
        {
            return false;
        }
    }
    return true;
}

/**
 * Offset/Address/Size: 0xCC | 0x801BFCEC | size: 0x104
 */
glModel* glplatLoadModel(const char* filename, unsigned long* pNumModels)
{
    unsigned int alignSize;
    unsigned int fileSize;
    char fullname[256];
    char lowerName[256];
    glModel* retval;
    nlFile* f;
    bool bSkinned;

    glx_FreeMemory0();
    nlStrNCat(fullname, glxModelPath, filename, 256);
    nlStrNCpy(lowerName, fullname, 256);
    nlToLower(lowerName);

    bSkinned = (strstr(lowerName, glxSkinExt) == NULL);

    f = nlOpen(fullname);
    if (f == NULL)
    {
        retval = NULL;
    }
    else
    {
        fileSize = nlFileSize(f, &alignSize);
        nlClose(f);
        if (fileSize == 0)
        {
            retval = NULL;
        }
        else
        {
            char* data = (char*)nlLoadEntireFileToVirtualMemory(fullname, (int*)&fileSize, 0x80000, NULL, AllocateEnd);
            retval = glxLoadModelFromMemory(data, fileSize, pNumModels, bSkinned);
        }
    }

    glx_FreeMemory1(filename);
    return retval;
}

// BMD model chunk type IDs (lower 24 bits of chunk id).
enum BMDChunkType
{
    BMD_CHUNK_REF_DATA = 0x1B002,     // alloc GLM_TextureData, store ptr
    BMD_CHUNK_MODELS = 0x1B004,       // model headers (16 bytes each), AddModel to inventory
    BMD_CHUNK_PACKETS = 0x1B006,      // glModelPacket array (0x4A bytes each)
    BMD_CHUNK_STREAMS = 0x1B007,      // stream descriptor data (relocated with display list base)
    BMD_CHUNK_SKIN = 0x1B008,         // skin data, AddSkinData to inventory
    BMD_CHUNK_DISPLAY_LIST = 0x1B009, // display list / vertex data (GLM_VertexData)
    BMD_CHUNK_INDEX_DATA = 0x1B00F,   // index/aux data (8-byte align)
    BMD_CHUNK_TEXTURE_ANIM = 0x1B010,
    BMD_CHUNK_VERTEX_ANIM = 0x1B011,
    BMD_CHUNK_MATERIAL_LIST = 0x1B012,
};

// Division by 67 for packet count (matches asm magic 0xBAD0914D sequence).
static inline u32 Div67(u32 n)
{
    u32 hi = (u32)(((u64)n * 0xBAD0914Du) >> 32);
    return (((n - hi) >> 1) + hi) >> 6;
}

static u8* GetBMDChunkDataPointer(nlChunk* chunk)
{
    u32 id = chunk->m_ID;
    u32 alignBits = id & 0x7F000000u;
    if (alignBits != 0)
    {
        u32 align = 1u << (alignBits >> 24);
        return (u8*)(((u32)((u8*)chunk + align) + 7) & ~(align - 1));
    }
    return (u8*)chunk + 8;
}

/**
 * Offset/Address/Size: 0x1D0 | 0x801BFDF0 | size: 0xA38
 */
glModel* glxLoadModelFromMemory(char* data, int size, unsigned long* pNumModels, bool bLoadTextures)
{
    FORCE_DONT_INLINE;

    char* chunkStart;
    char* chunkEnd;
    bool hasBmdHeader = false;
    u32 refDataPtr = 0;
    glModel* pModels = nullptr;
    u32 numModels = 0;
    glModelPacket* pPackets = nullptr;
    u8* pStreamData = nullptr;
    u8* pDisplayListData = nullptr;
    u8* pIndexData = nullptr;
    int numPacketEntries = 0;
    int numStreamEntries = 0;
    bool hasSkinData = false;
    f32 streamStrideTemp[2];
    // Stream id 12 stride (from gl_stream_stride[12]; index 12 = 1 in GLMeshWriter).
    static const u8 kStreamId12Stride = 1;

    // If first word looks like BMD magic (0x??B100 in low 16 bits), skip 8-byte header.
    u32 firstWord = *(u32*)data;
    if ((firstWord & 0x0000FFFFu) == 0x0000B100u)
    {
        u32 innerSize = *(u32*)(data + 4);
        chunkStart = data + 8;
        chunkEnd = data + 8 + innerSize;
        hasBmdHeader = true;
    }
    else
    {
        chunkStart = data;
        chunkEnd = data + size;
    }

    while (chunkStart < chunkEnd)
    {
        nlChunk* outerChunk = (nlChunk*)chunkStart;
        u32 outerId = outerChunk->m_ID & 0x00FFFFFFu;
        u32 outerSize = outerChunk->m_Size;
        u8* outerData = (u8*)outerChunk + 8;
        u8* outerEnd = outerData + outerSize;

        if (!hasBmdHeader)
        {
            chunkStart = (char*)(outerData + outerSize);
        }

        // Inner loop: process sub-chunks within this block.
        for (nlChunk* chunk = (nlChunk*)outerData; (u8*)chunk < outerEnd; chunk = (nlChunk*)((u8*)chunk + 8 + chunk->m_Size))
        {
            u32 id = chunk->m_ID & 0x00FFFFFFu;
            u32 chunkSize = chunk->m_Size;
            u8* chunkData = GetBMDChunkDataPointer(chunk);

            switch (id)
            {
            case BMD_CHUNK_REF_DATA:
            {
                void* p = glResourceAlloc(chunkSize, GLM_TextureData);
                refDataPtr = (u32)(uintptr_t)p;
                memcpy(p, chunkData, chunkSize);
                break;
            }
            case BMD_CHUNK_MODELS:
            {
                numModels = chunkSize >> 4;
                if (pNumModels != nullptr)
                    *pNumModels = numModels;
                pModels = (glModel*)glResourceAlloc(chunkSize, GLM_Header);
                memcpy(pModels, chunkData, chunkSize);
                for (u32 i = 0; i < numModels; i++)
                {
                    glModel* pEnt = (glModel*)((u8*)pModels + i * 0x10);
                    if (glIgnoreDuplicateModels)
                    {
                        glModel* existing = glInventory.GetModel(pEnt->id);
                        if (existing != nullptr)
                            continue;
                    }
                    glInventory.AddModel(pEnt->id, pEnt);
                }
                break;
            }
            case BMD_CHUNK_PACKETS:
            {
                numPacketEntries = (int)Div67(chunkSize);
                pPackets = (glModelPacket*)glResourceAlloc(chunkSize, GLM_Header);
                memcpy(pPackets, chunkData, chunkSize);
                break;
            }
            case BMD_CHUNK_STREAMS:
            {
                numStreamEntries = (int)(chunkSize / sizeof(glModelStream));
                pStreamData = (u8*)glResourceAlloc(chunkSize, GLM_Header);
                memcpy(pStreamData, chunkData, chunkSize);
                break;
            }
            case BMD_CHUNK_DISPLAY_LIST:
            {
                pDisplayListData = (u8*)glResourceAlloc(chunkSize, GLM_VertexData);
                memcpy(pDisplayListData, chunkData, chunkSize);
                DCFlushRange(pDisplayListData, chunkSize);
                break;
            }
            case BMD_CHUNK_INDEX_DATA:
            {
                pIndexData = (u8*)nlMalloc(chunkSize, 8, true);
                memcpy(pIndexData, chunkData, chunkSize);
                DCFlushRange(pIndexData, chunkSize);
                break;
            }
            case BMD_CHUNK_TEXTURE_ANIM:
            {
                u32 animId = *(u32*)chunkData;
                if (glInventory.GetTextureAnim(animId) != nullptr)
                {
                    // Duplicate texture anim id - debug print and skip.
                    break;
                }
                {
                    int numTextures = *(int*)(chunkData + 4);
                    u32 mode = *(u32*)(chunkData + 8);
                    memcpy(streamStrideTemp, chunkData + 12, 4);
                    u8* pFrames = chunkData + 16;
                    GLTextureAnim* pAnim = (GLTextureAnim*)nlMalloc(0x20, 8, false);
                    if (pAnim != nullptr)
                        pAnim = new (pAnim) GLTextureAnim();
                    if (pAnim == nullptr)
                        break;
                    pAnim->m_unk_0x00 = (s32)animId;
                    pAnim->SetNumTextures(numTextures);
                    pAnim->m_mode = mode;
                    int frameIdx = (int)streamStrideTemp[0];
                    pAnim->SetFrame(frameIdx);
                    for (int t = 0; t < numTextures; t++)
                    {
                        u32 texHandle = *(u32*)pFrames;
                        pFrames += 4;
                        memcpy(streamStrideTemp, pFrames, 4);
                        pFrames += 4;
                        GLAnimTex animTex;
                        animTex.textureHandle = texHandle;
                        animTex.time = streamStrideTemp[0];
                        streamStrideTemp[0] = animTex.time;
                        pAnim->SetTexture(t, animTex);
                    }
                    glInventory.AddTextureAnim(animId, pAnim);
                }
                break;
            }
            case BMD_CHUNK_VERTEX_ANIM:
            {
                u32 modelId = *(u32*)chunkData;
                int numFrames = *(int*)(chunkData + 4);
                int numVerts = *(int*)(chunkData + 8);
                u32 frameRateU = *(u32*)(chunkData + 12);
                int dataSize = numFrames * numVerts * 12;
                GLVertexAnim* pAnim = (GLVertexAnim*)nlMalloc(0x28, 8, false);
                if (pAnim != nullptr)
                    pAnim = new (pAnim) GLVertexAnim();
                if (pAnim == nullptr)
                    break;
                pAnim->m_uHashID = modelId;
                pAnim->m_nNumFrames = numFrames;
                pAnim->m_nNumVertices = numVerts;
                pAnim->m_fFrameRate = (f32)(s32)frameRateU;
                u8* pAnimData = (u8*)glResourceAlloc(dataSize, GLM_VertexData);
                memcpy(pAnimData, chunkData + 16, dataSize);
                DCFlushRange(pAnimData, dataSize);
                pAnim->m_pVertices = (nlVector3*)pAnimData;
                pAnim->m_pModel = glInventory.GetModel(modelId);
                pAnim->Reset();
                glInventory.AddVertexAnim(modelId, pAnim);
                break;
            }
            case BMD_CHUNK_MATERIAL_LIST:
            {
                u32 listId = *(u32*)chunkData;
                int numMats = *(int*)(chunkData + 4);
                GLMaterialList* pList = (GLMaterialList*)nlMalloc(0x0C, 8, false);
                if (pList != nullptr)
                    pList = new (pList) GLMaterialList();
                if (pList == nullptr)
                    break;
                pList->m_uHashID = listId;
                pList->SetMaterials(numMats, (const GLMaterialEntry*)(chunkData + 8));
                glInventory.AddMaterialList(listId, pList);
                break;
            }
            case BMD_CHUNK_SKIN:
            {
                u32 skinSize = chunkSize + 8;
                nlChunk* pSkinChunk;
                if (nlVirtualLargestBlock() >= skinSize + 0x100)
                {
                    pSkinChunk = (nlChunk*)nlVirtualAlloc(skinSize, false);
                }
                else
                {
                    pSkinChunk = (nlChunk*)nlMalloc(skinSize, 0x20, false);
                }
                memcpy(pSkinChunk, chunk, skinSize);
                glInventory.AddSkinData(pModels->id, pSkinChunk);
                hasSkinData = true;
                break;
            }
            default:
                break;
            }
        }

        if (hasBmdHeader)
        {
            chunkStart = (char*)(outerData + outerSize);
        }
    }

    // Relocate stream pointers in model headers (add packet base to offset at +0xC).
    if (numModels > 0 && pModels != nullptr && pPackets != nullptr)
    {
        glModel* pM = pModels;
        for (u32 i = 0; i < numModels; i++)
        {
            u32* pStreamPtr = (u32*)((u8*)pM + 0x0C);
            *pStreamPtr += (u32)(uintptr_t)pPackets;
            pM = (glModel*)((u8*)pM + 0x10);
        }
    }

    // Relocate packet fields: streams base, index buffer, and ref data.
    if (numPacketEntries > 0 && pPackets != nullptr)
    {
        glModelPacket* pPkt = pPackets;
        for (int i = 0; i < numPacketEntries; i++)
        {
            if (glGetRasterState(pPkt->state.raster, (eGLState)5) == 0)
            {
                if (glTextureLoad(pPkt->state.texture[0]))
                {
                    glUnHandleizeRasterState(pPkt->state.raster);
                    int bits = glTextureGetNumBits(3);
                    if (bits == 1)
                    {
                        glSetRasterState((eGLState)5, 0);
                        glSetRasterState((eGLState)3, 1);
                        glSetRasterState((eGLState)4, 0x40);
                    }
                    else if (bits > 1)
                    {
                        glSetRasterState((eGLState)5, 1);
                        glSetRasterState((eGLState)3, 1);
                        glSetRasterState((eGLState)4, 0);
                        glSetRasterState((eGLState)1, 0);
                    }
                    pPkt->state.raster = glHandleizeRasterState();
                }
            }
            pPkt->streams = (glModelStream*)((u32)(uintptr_t)pPkt->streams + (u32)(uintptr_t)pStreamData);
            pPkt->indexBuffer = (u32)(uintptr_t)pPkt->indexBuffer + (u32)(uintptr_t)pIndexData;
            pPkt->state.matrix = (u32)((u32)pPkt->state.matrix + refDataPtr);
            pPkt = (glModelPacket*)((u8*)pPkt + 0x4A);
        }
    }

    // Relocate stream addresses in stream descriptor block (add display list base).
    if (numStreamEntries > 0 && pStreamData != nullptr && pDisplayListData != nullptr)
    {
        u8* p = pStreamData;
        for (int i = 0; i < numStreamEntries; i++)
        {
            *(u32*)p += (u32)(uintptr_t)pDisplayListData;
            p += 6;
        }
    }

    nlFree(data);

    // Final pass: build display lists and optionally resolve textures.
    if (pModels != nullptr)
    {
        glModel* pModel = pModels;
        for (u32 m = 0; m < numModels; m++)
        {
            glModelPacket* pPacket = (glModelPacket*)pModel->packets;
            u32 n = pModel->numPackets;
            for (u32 p = 0; p < n; p++)
            {
                if (hasSkinData)
                {
                    u8 numStreams = pPacket->numStreams;
                    u32* pRaster = &pPacket->state.raster;
                    if (glGetRasterState(*pRaster, (eGLState)8) == 1)
                    {
                        int newNum = numStreams + 1;
                        glModelStream* pNewStreams = (glModelStream*)glResourceAlloc(newNum * 6, GLM_Header);
                        memcpy(pNewStreams, pPacket->streams, numStreams * 6);
                        glModelStream* pExtra = &pNewStreams[numStreams];
                        pExtra->address = 0;
                        pExtra->id = 12;
                        pExtra->stride = kStreamId12Stride;
                        pPacket->numStreams = (u8)newNum;
                        pPacket->streams = pNewStreams;
                    }
                }
                if (pPacket->indexBuffer != 0)
                {
                    DisplayList* pDL = dlMakeDisplayList(pPacket, true);
                    pPacket->indexBuffer = (u32)(uintptr_t)pDL;
                }
                if (bLoadTextures)
                {
                    u8 texMask = *(u8*)((u8*)pPacket + 0x40);
                    for (int s = 0; s < 6; s++)
                    {
                        if (!((texMask >> s) & 1))
                            continue;
                        u32 texHandle = pPacket->state.texture[s];
                        if (glInventory.GetTextureAnim(texHandle) != nullptr)
                            continue;
                        if (!glTextureLoad(texHandle))
                            continue;
                        pPacket->state.texture[s] = (u32)(uintptr_t)glx_GetTex(texHandle, true, true);
                    }
                }
                pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
            }
            pModel = (glModel*)((u8*)pModel + 0x10);
        }
        nlFree(pIndexData);
    }

    return pModels;
}

// Skin mesh chunk type IDs (lower 24 bits of m_ID); switch index = (type - 0x1B009).
enum SkinChunkType
{
    SKIN_CHUNK_0x1B009 = 0x1B009,
    SKIN_CHUNK_BONE_MATRICES = 0x1B00A,
    SKIN_CHUNK_BONE_MAP_LIST = 0x1B00B,
    SKIN_CHUNK_MORPH = 0x1B00C,
    SKIN_CHUNK_SOFTWARE_VERTICES = 0x1B00D,
    SKIN_CHUNK_SKIN_PAIRS = 0x1B00E,
    SKIN_CHUNK_0x1B00F = 0x1B00F,
    SKIN_CHUNK_STITCHING = 0x1B010,
};

// Node for bone map list during load; layout matches BoneMapList but tree is Ul,Ul for file format.
struct SkinMeshBoneMapNode
{
    SkinMeshBoneMapNode* m_next;
    nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> > boneMap;
};

static u8* GetChunkDataPointer(nlChunk* chunk)
{
    u32 id = chunk->m_ID;
    u32 alignBits = id & 0x7F000000u;
    if (alignBits != 0)
    {
        u32 align = 1u << (alignBits >> 24);
        return (u8*)(((u32)((u8*)chunk + align) + 7) & ~(align - 1));
    }
    return (u8*)chunk + 8;
}

/**
 * Offset/Address/Size: 0xC08 | 0x801C0828 | size: 0x2A0
 * TODO: 90.27% match - register allocation still differs for mesh/chunk pointers and the boneMap constructor emits extra addic/beq checks.
 */
GLSkinMesh* glx_MakeSkinMesh(nlChunk* outerChunk, glModel* models)
{
    ShaderSkinMesh* mesh = new (nlMalloc(sizeof(ShaderSkinMesh), 8, false)) ShaderSkinMesh();

    mesh->pModel = models;

    nlChunk* chunk = (nlChunk*)((u8*)outerChunk + 8);
    nlChunk* chunkEnd = (nlChunk*)((u8*)outerChunk + outerChunk->m_Size + 8);

    while (chunk != chunkEnd)
    {
        u32 id = chunk->m_ID;
        u32 chunkSize = chunk->m_Size;
        u32 chunkType = id & 0x00FFFFFF;
        u32 alignBits = id & 0x7F000000;
        u8* data = (u8*)chunk + 8;

        if (((-alignBits | alignBits) >> 31) != 0)
        {
            u32 align = 1 << (alignBits >> 24);
            data = (u8*)(((u32)((u8*)chunk + align) + 7) & ~(align - 1));
        }

        switch (chunkType - 0x1B009)
        {
        case 1:
        {
            u32 i = 0;
            u32 count = chunkSize / 0x44;
            while (i < count)
            {
                u32 boneID = *(u32*)data;
                nlMatrix4 src;
                nlMatrix4 inv;
                memcpy(&src, data + 4, 0x40);
                data += 0x44;
                nlInvertMatrix(inv, src);
                mesh->SetBoneMatrix(boneID, &inv);
                i++;
            }
            break;
        }
        case 2:
        {
            SkinMeshBoneMapNode* node = (SkinMeshBoneMapNode*)nlMalloc(sizeof(SkinMeshBoneMapNode), 8, false);
            if (node != NULL)
            {
                new (&node->boneMap) nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >();
            }

            u32 i = 0;
            u32 count = chunkSize >> 3;
            node->m_next = NULL;
            AVLTreeNode** root = (AVLTreeNode**)&node->boneMap.m_Root;
            while (i < count)
            {
                unsigned long key = *(u32*)(data + 0);
                unsigned long value = *(u32*)(data + 4);
                data += 8;
                AVLTreeNode* existingNode;
                node->boneMap.AddAVLNode(root, &key, &value, &existingNode, node->boneMap.m_NumElements);
                if (existingNode == NULL)
                {
                    node->boneMap.m_NumElements++;
                }
                i++;
            }
            nlRingAddEnd<BoneMapList>(&mesh->boneMaps, (BoneMapList*)node);
            break;
        }
        case 4:
            mesh->SetSoftwareVertices((int)(chunkSize >> 4), (const SkinVertex*)data);
            break;
        case 5:
            mesh->AppendSkinPairList((int)(chunkSize >> 2), (const SkinPair*)data);
            break;
        case 3:
        {
            u32 numMorphs = *(u32*)(data + 0);
            u8* p = data + 8;
            mesh->numMorphs = (int)numMorphs;
            mesh->numBaseVerts = *(u32*)(data + 4);
            mesh->SetMorphIDs((const u32*)p);
            p += numMorphs * 4;
            mesh->SetMorphNumDeltas((const u32*)p);
            p += numMorphs * 4;
            mesh->SetMorphDeltas(*(int*)p, (const MorphDelta*)(p + 4));
            break;
        }
        case 7:
            mesh->AppendStitchingInfo(*(int*)(data + 4), *(int*)(data + 0), (int)chunkSize - 8, data + 8);
            break;
        }

        chunk = (nlChunk*)((u8*)chunk + chunk->m_Size + 8);
    }

    mesh->StitchModel();
    return mesh;
}

/**
 * Offset/Address/Size: 0xEA8 | 0x801C0AC8 | size: 0x60
 */
// void nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long>>::~nlAVLTree()
// {
// }

/**
 * Offset/Address/Size: 0xF08 | 0x801C0B28 | size: 0x8
 */
void glSetIgnoreDuplicateModels(bool ignore)
{
    glIgnoreDuplicateModels = ignore;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x801C0B30 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::DeleteEntry(AVLTreeEntry<unsigned long, unsigned long>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x24 | 0x801C0B54 | size: 0x5C
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x80 | 0x801C0BB0 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0xD8 | 0x801C0C08 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::DestroyTree(void (AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>,
// DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, unsigned long>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x13C | 0x801C0C6C | size: 0x758
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::PostorderTraversal(AVLTreeEntry<unsigned long, unsigned long>*, void (AVLTreeBase<unsigned long, unsigned long,
// NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, unsigned
// long>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x894 | 0x801C13C4 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x8F8 | 0x801C1428 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x924 | 0x801C1454 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801C1480 | size: 0x3C
//  */
// void nlRingAddEnd<BoneMapList>(BoneMapList**, BoneMapList*)
// {
// }

// /**
//  * Offset/Address/Size: 0x3C | 0x801C14BC | size: 0x2C
//  */
// void nlRingAddStart<BoneMapList>(BoneMapList**, BoneMapList*)
// {
// }
