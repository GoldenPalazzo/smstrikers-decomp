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
#include "Game/Sys/debug.h"
#include "dolphin/os/OSCache.h"
#include <string.h>

extern GLInventory glInventory;

static bool glIgnoreDuplicateModels;
static const char glxModelPath[] = "smstk";
static const char glxSkinExt[] = ".skinmesh";

static glModel* glxLoadModelFromMemory(char* data, int size, unsigned long* pNumModels, bool bLoadTextures);

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

// BMD model chunk type IDs.
enum BMDChunkType
{
    BMD_CHUNK_REF_DATA = 0x1B002,
    BMD_CHUNK_MODELS = 0x1B003,
    BMD_CHUNK_PACKETS = 0x1B004,
    BMD_CHUNK_STREAMS = 0x1B005,
    BMD_CHUNK_DISPLAY_LIST = 0x1B006,
    BMD_CHUNK_INDEX_DATA = 0x1B007,
    BMD_CHUNK_SKIN = (int)0x8001B008u,
    BMD_CHUNK_TEXTURE_ANIM = 0x1B00F,
    BMD_CHUNK_VERTEX_ANIM = 0x1B011,
    BMD_CHUNK_MATERIAL_LIST = 0x1B012,
};

static const int gl_stream_stride[15] = {
    12, 3, 4, 4, 4, 4, 4, 4, 4, 12, 12, 12, 1, 16, 16
};

// Division by 67 for packet count (matches asm magic 0xBAD0914D sequence).
static inline u32 Div67(u32 n)
{
    u32 hi = (u32)(((u64)n * 0xBAD0914Du) >> 32);
    return (((n - hi) >> 1) + hi) >> 6;
}

/**
 * Offset/Address/Size: 0x1D0 | 0x801BFDF0 | size: 0xA38
 */
static glModel* glxLoadModelFromMemory(char* data, int size, unsigned long* pNumModels, bool bLoadTextures)
{
    FORCE_DONT_INLINE;

    bool hasBmdHeader = false;
    nlChunk* innerEnd;
    nlChunk* chunk;
    nlChunk* outerChunkPtr;
    nlChunk* outerEnd;
    nlChunk* chunkStart;
    nlChunk* chunkEnd;
    u8* pStreamData;
    int numPacketEntries;
    GLAnimTex animTex;
    u8* pIndexData;
    glModel* pModels;
    int numStreamEntries;
    u32 refDataPtr;
    glModelPacket* pPackets;
    u32 numModels;
    bool hasSkinData;
    u8* pDisplayListData;

    outerChunkPtr = (nlChunk*)data;
    outerEnd = (nlChunk*)(data + size);
    chunkStart = outerChunkPtr;
    chunkEnd = outerEnd;

    if ((*(u32*)outerChunkPtr & ~0x7F000000u) == 0x8001B100u)
    {
        u32 innerSize = outerChunkPtr->m_Size;
        chunkStart = (nlChunk*)((u8*)outerChunkPtr + 8);
        chunkEnd = (nlChunk*)((u8*)outerChunkPtr + innerSize + 8);
        hasBmdHeader = true;
    }

    hasSkinData = false;

    while (chunkStart < chunkEnd)
    {
        if (hasBmdHeader)
        {
            nlChunk* topChunk = (nlChunk*)chunkStart;
            outerChunkPtr = chunkStart;
            outerEnd = (nlChunk*)((u8*)chunkStart + topChunk->m_Size + 8);
        }

        while (outerChunkPtr < outerEnd)
        {
            chunk = (nlChunk*)((u8*)outerChunkPtr + 8);
            innerEnd = (nlChunk*)((u8*)outerChunkPtr + outerChunkPtr->m_Size + 8);

            while (chunk != innerEnd)
            {
                u32 rawId = chunk->m_ID;
                u32 chunkSize = chunk->m_Size;
                u32 alignBits = rawId & 0x7F000000u;
                int id = (int)(rawId & ~0x7F000000u);
                u8* chunkData;
                if (((-alignBits | alignBits) >> 31) != 0)
                {
                    u32 align = 1u << (alignBits >> 24);
                    u32 ptr = (u32)chunk + align;
                    ptr += 7;
                    ptr &= ~(align - 1);
                    chunkData = (u8*)ptr;
                }
                else
                {
                    chunkData = (u8*)chunk + 8;
                }

                switch (id)
                {
                case BMD_CHUNK_REF_DATA:
                {
                    void* p = glResourceAlloc(chunkSize, GLM_Matrix);
                    refDataPtr = (u32)p;
                    memcpy(p, chunkData, chunkSize);
                    break;
                }
                case BMD_CHUNK_MODELS:
                {
                    numModels = chunkSize >> 4;
                    if (pNumModels != NULL)
                        *pNumModels = numModels;
                    pModels = (glModel*)glResourceAlloc(chunkSize, GLM_Header);
                    memcpy(pModels, chunkData, chunkSize);
                    {
                        glModel* pEnt = pModels;
                        glModel* pEntEnd = (glModel*)((u8*)pModels + (numModels << 4));
                        while (pEnt < pEntEnd)
                        {
                            if (glIgnoreDuplicateModels)
                            {
                                if (glInventory.GetModel(pEnt->id) != NULL)
                                {
                                    pEnt++;
                                    continue;
                                }
                            }
                            glInventory.AddModel(pEnt->id, pEnt);
                            pEnt++;
                        }
                    }
                    break;
                }
                case BMD_CHUNK_PACKETS:
                {
                    numPacketEntries = (int)(chunkSize / sizeof(glModelPacket));
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
                    if (glInventory.GetTextureAnim(animId) == NULL)
                    {
                        u32* pTexData = (u32*)(chunkData + 12);
                        int numTextures = *(int*)(chunkData + 4);
                        GLTextureAnim* pAnim;
                        u32 mode = *(u32*)(chunkData + 8);
                        f32 tempRate;
                        memcpy(&tempRate, pTexData, 4);
                        pTexData++;
                        pAnim = (GLTextureAnim*)nlMalloc(0x20, 8, false);
                        new (pAnim) GLTextureAnim();
                        pAnim->m_unk_0x00 = (s32)animId;
                        pAnim->SetNumTextures(numTextures);
                        pAnim->m_mode = mode;
                        pAnim->SetFrame((int)tempRate);
                        for (u32 t = 0; t < (u32)numTextures; t++)
                        {
                            animTex.textureHandle = *(u32*)pTexData;
                            pTexData++;
                            f32 timeVal;
                            memcpy(&timeVal, pTexData, 4);
                            animTex.time = timeVal;
                            pTexData++;
                            pAnim->SetTexture(t, animTex);
                        }
                        glInventory.AddTextureAnim(animId, pAnim);
                    }
                    else
                    {
                        tDebugPrintManager::Print(DC_LOADER, "skipping duplicate texanim 0x%08X\n", animId);
                    }
                    break;
                }
                case BMD_CHUNK_VERTEX_ANIM:
                {
                    u32 modelId = *(u32*)chunkData;
                    u8* pVertData = chunkData + 16;
                    int numFrames = *(int*)(chunkData + 4);
                    int numVerts = *(int*)(chunkData + 8);
                    u32 frameRateU = *(u32*)(chunkData + 12);
                    GLVertexAnim* pAnim = (GLVertexAnim*)nlMalloc(0x28, 8, false);
                    new (pAnim) GLVertexAnim();
                    int dataSize = numFrames * 12 * numVerts;
                    pAnim->m_uHashID = modelId;
                    pAnim->m_nNumFrames = numFrames;
                    pAnim->m_nNumVertices = numVerts;
                    pAnim->m_fFrameRate = (f32)frameRateU;
                    u8* pAnimData = (u8*)glResourceAlloc(dataSize, GLM_VertexData);
                    memcpy(pAnimData, pVertData, dataSize);
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
                    u8* pMatData = chunkData + 8;
                    int numMats = *(int*)(chunkData + 4);
                    GLMaterialList* pList = (GLMaterialList*)nlMalloc(0x0C, 8, false);
                    new (pList) GLMaterialList();
                    pList->m_uHashID = listId;
                    pList->SetMaterials(numMats, (const GLMaterialEntry*)pMatData);
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
                        u32 totalFree = nlVirtualTotalFree();
                        u32 largestBlock = nlVirtualLargestBlock();
                        OSReport("VIRTUAL MEMORY WARNING ~ NLVIRTUALALLOC had to fall back to MRAM\nLargest block: %d Total free: %d\n", largestBlock, totalFree);
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

                chunk = (nlChunk*)((u8*)chunk + chunk->m_Size + 8);
            }

            outerChunkPtr = (nlChunk*)((u8*)outerChunkPtr + outerChunkPtr->m_Size + 8);

            if ((s32)numModels > 0)
            {
                glModel* pM = pModels;
                u32 count = numModels;
                while (count > 0)
                {
                    pM->packets = (glModelPacket*)((u32)pM->packets + (u32)pPackets);
                    pM++;
                    count--;
                }
            }

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
                    pPkt->streams = (glModelStream*)((u32)pPkt->streams + (u32)pStreamData);
                    pPkt->indexBuffer += (u32)pIndexData;
                    pPkt->state.matrix += refDataPtr;
                    pPkt = (glModelPacket*)((u8*)pPkt + 0x4A);
                }
            }

            if ((s32)numStreamEntries > 0)
            {
                u8* p = pStreamData;
                int count = numStreamEntries;
                while (count > 0)
                {
                    *(u32*)p += (u32)pDisplayListData;
                    p += 6;
                    count--;
                }
            }

            {
                glModel* pModel = pModels;
                glModel* pModelEnd = (glModel*)((u8*)pModels + (numModels << 4));
                while (pModel < pModelEnd)
                {
                    glModelPacket* pPacket = pModel->packets;
                    while ((u8*)pPacket < (u8*)pModel->packets + pModel->numPackets * 0x4A)
                    {
                        if (hasSkinData)
                        {
                            if (glGetRasterState(pPacket->state.raster, (eGLState)8) == 1)
                            {
                                u8 numStreams = pPacket->numStreams;
                                int newNum = numStreams + 1;
                                glModelStream* pNewStreams = (glModelStream*)glResourceAlloc(newNum * 6, GLM_Header);
                                memcpy(pNewStreams, pPacket->streams, numStreams * 6);
                                glModelStream* pExtra = (glModelStream*)((u8*)pNewStreams + numStreams * 6);
                                pExtra->id = 12;
                                *(u32*)&pExtra->address = 0;
                                pExtra->stride = (u8)gl_stream_stride[12];
                                pPacket->numStreams = (u8)(pPacket->numStreams + 1);
                                pPacket->streams = pNewStreams;
                            }
                        }
                        if (pPacket->indexBuffer != 0)
                        {
                            pPacket->indexBuffer = (u32)dlMakeDisplayList(pPacket, true);
                        }
                        if (bLoadTextures)
                        {
                            for (int s = 0; s < 6; s++)
                            {
                                if (pPacket->state.texconfig & (1 << s))
                                {
                                    u32 texHandle = pPacket->state.texture[s];
                                    if (glInventory.GetTextureAnim(texHandle) == NULL)
                                    {
                                        if (glTextureLoad(texHandle))
                                        {
                                            pPacket->state.texture[s] = (u32)glx_GetTex(texHandle, true, true);
                                        }
                                    }
                                }
                            }
                        }
                        pPacket = (glModelPacket*)((u8*)pPacket + 0x4A);
                    }
                    pModel++;
                }
                nlFree(pIndexData);
            }
        }

        if (!hasBmdHeader)
            break;

        {
            nlChunk* topChunk = (nlChunk*)chunkStart;
            chunkStart = (nlChunk*)((u8*)chunkStart + topChunk->m_Size + 8);
        }
    }

    nlFree(data);
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
 * TODO: 98.7% match - loop counter/root register diffs remain in bone matrix
 * and bone map cases
 */
GLSkinMesh* glx_MakeSkinMesh(nlChunk* outerChunk, glModel* models)
{
    ShaderSkinMesh* mesh = new (nlMalloc(sizeof(ShaderSkinMesh), 8, false)) ShaderSkinMesh();

    mesh->pModel = models;

    u32 align;
    u32 count;
    u32 i;
    nlChunk* chunkEnd = (nlChunk*)((u8*)outerChunk + outerChunk->m_Size + 8);
    u32 chunkSize;
    u8* data;

    for (nlChunk* chunk = (nlChunk*)((u8*)outerChunk + 8); chunk != chunkEnd; chunk = (nlChunk*)((u8*)chunk + chunk->m_Size + 8))
    {
        u32 id = chunk->m_ID;
        chunkSize = chunk->m_Size;
        u32 alignBits = id & 0x7F000000;
        u32 chunkType = id & ~0x7F000000u;

        u8* result;
        if (((-alignBits | alignBits) >> 31) != 0)
        {
            align = 1 << (alignBits >> 24);
            u32 ptr = (u32)chunk;
            ptr += align;
            ptr += 7;
            result = (u8*)(ptr & ~(align - 1));
        }
        else
        {
            result = (u8*)chunk + 8;
        }
        data = result;

        switch (chunkType)
        {
        case 0x1B009:
            break;
        case 0x1B00A:
        {
            i = 0;
            count = chunkSize / 0x44;
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
        case 0x1B00B:
        {
            SkinMeshBoneMapNode* node = new (nlMalloc(sizeof(SkinMeshBoneMapNode), 8, false)) SkinMeshBoneMapNode;

            i = 0;
            node->m_next = NULL;
            count = chunkSize >> 3;
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
        case 0x1B00D:
            mesh->SetSoftwareVertices((int)(chunkSize >> 4), (const SkinVertex*)data);
            break;
        case 0x1B00E:
            mesh->AppendSkinPairList((int)(chunkSize >> 2), (const SkinPair*)data);
            break;
        case 0x1B00C:
        {
            u32 numMorphs = *(u32*)(data + 0);
            mesh->numMorphs = (int)numMorphs;
            mesh->numBaseVerts = *(u32*)(data + 4);
            data += 8;
            mesh->SetMorphIDs((const u32*)data);
            data += numMorphs * 4;
            mesh->SetMorphNumDeltas((const u32*)data);
            data += numMorphs * 4;
            mesh->SetMorphDeltas(*(int*)data, (const MorphDelta*)(data + 4));
            break;
        }
        case 0x1B00F:
            break;
        case 0x1B010:
            mesh->AppendStitchingInfo(*(int*)(data + 4), *(int*)(data + 0), (int)chunkSize - 8, data + 8);
            break;
        }
    }

    mesh->StitchModel();
    return mesh;
}

/**
 * Offset/Address/Size: 0xF08 | 0x801C0B28 | size: 0x8
 */
void glSetIgnoreDuplicateModels(bool ignore)
{
    glIgnoreDuplicateModels = ignore;
}
