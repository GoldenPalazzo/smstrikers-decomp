#include "NL/glx/glxTexture.h"

#include "NL/nlAVLTree.h"
#include "NL/nlList.h"
#include "NL/nlQSort.h"
#include "NL/nlFile.h"
#include "NL/nlFileGC.h"
#include "NL/nlString.h"
#include "NL/gl/glTexture.h"
#include "NL/gl/glMemory.h"
#include "NL/gc/gcSwizzler.h"
#include "dolphin/gx/GXTexture.h"
#include "Game/GL/GLInventory.h"
#include "Game/Sys/debug.h"

extern GLInventory glInventory;

static glxTextureLoadCallback_t glxTextureLoad_cb;
static int currentMarkerLevel = 0;
static bool glx_bGridMode = false;

static nlListContainer<PlatTexture*> gridTextures;
static unsigned long nGridMemory;
static const u8 kDefaultBits[4] = { 5, 6, 5, 0 };

static nlAVLTree<unsigned long, PlatTexture*, DefaultKeyCompare<unsigned long> >* textures[16];
static PlatTexture texobj;

static GXTexFmt gx_format[9] = {
    GX_TF_RGB565,
    GX_TF_RGB5A3,
    GX_TF_CMPR,
    GX_TF_RGBA8,
    GX_TF_I8,
    GX_TF_I4,
    GX_TF_I8,
    GX_TF_IA8,
    (GXTexFmt)0x9
};

/**
 * Offset/Address/Size: 0x0 | 0x801B72BC | size: 0x230
 */
void glplatTextureReplace(unsigned long handle, const void* textureData, unsigned long size)
{
    FORCE_DONT_INLINE;
    const GXTextureHeader* pHeader = (GXTextureHeader*)textureData;
    PlatTexture* pTex = glx_GetTex(handle, false, false);

    memcpy(pTex->m_SwizzledData, ((u8*)textureData) + sizeof(GXTextureHeader), GCTextureSize(pTex->m_Format, pTex->m_Width, pTex->m_Height, pTex->m_Levels, -1));

    if (pHeader->numEntries != 0)
    {
        const u8* src = (const u8*)textureData;
        src += GCTextureSize(pTex->m_Format, pTex->m_Width, pTex->m_Height, pTex->m_Levels, -1);
        src += sizeof(GXTextureHeader);
        memcpy(pTex->m_PaletteData, src, pHeader->numEntries * 2);
    }
    DCStoreRange(pTex->m_SwizzledData, GCTextureSize(pTex->m_Format, pTex->m_Width, pTex->m_Height, pTex->m_Levels, -1));

    if (pTex->m_nPaletteEntries > 0)
    {
        DCStoreRange(pTex->m_PaletteData, pTex->m_nPaletteEntries * 2);
        GXInitTlutObj(&pTex->m_TlutObj, pTex->m_PaletteData, GX_TL_RGB5A3, pTex->m_nPaletteEntries);
    }

    if (pTex->m_Format == GXTex_CI8)
    {
        GXInitTexObjCI(&pTex->m_TexObj, pTex->m_SwizzledData, pTex->m_Width, pTex->m_Height, (GXCITexFmt)gx_format[pTex->m_Format], GX_CLAMP, GX_CLAMP, pTex->m_Levels > 1 ? 1 : 0, 0);
        GXInitTexObjLOD(&pTex->m_TexObj, (pTex->m_Levels == 1) ? GX_LINEAR : GX_LIN_MIP_NEAR, GX_LINEAR, 0.0f, (float)(pTex->m_MaxLevel - 1), 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
        return;
    }

    GXInitTexObj(&pTex->m_TexObj, pTex->m_SwizzledData, pTex->m_Width, pTex->m_Height, gx_format[pTex->m_Format], GX_CLAMP, GX_CLAMP, pTex->m_Levels > 1 ? 1 : 0);
    GXInitTexObjLOD(&pTex->m_TexObj, (pTex->m_Levels == 1) ? GX_LINEAR : GX_LIN_MIP_LIN, GX_LINEAR, 0.0f, (float)(pTex->m_MaxLevel - 1), 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
}

/**
 * Offset/Address/Size: 0x230 | 0x801B74EC | size: 0x98
 */
void glplatTextureAdd(unsigned long handle, const void* textureData, unsigned long size)
{
    FORCE_DONT_INLINE;
    unsigned long handle_copy;
    PlatTexture* platTex = glx_MakeTexture((GXTextureHeader*)textureData, handle);
    AVLTreeNode* existingNode;
    nlAVLTree<unsigned long, PlatTexture*, DefaultKeyCompare<unsigned long> >* textureTree;
    handle_copy = handle;

    if ((platTex != NULL) && (handle != -1))
    {
        textureTree = textures[currentMarkerLevel];

        textureTree->AddAVLNode((AVLTreeNode**)&textureTree->m_Root, &handle_copy, &platTex, &existingNode, textureTree->m_NumElements);

        if (existingNode == NULL)
        {
            textureTree->m_NumElements += 1;
        }
    }
}

/**
 * Offset/Address/Size: 0x2C8 | 0x801B7584 | size: 0x1B4
 */
void PlatTexture::Prepare()
{
    DCStoreRange(m_SwizzledData, GCTextureSize(m_Format, m_Width, m_Height, m_Levels, -1));
    if (m_nPaletteEntries > 0)
    {
        DCStoreRange(m_PaletteData, m_nPaletteEntries * 2);
        GXInitTlutObj(&m_TlutObj, m_PaletteData, GX_TL_RGB5A3, m_nPaletteEntries);
    }
    if (m_Format == GXTex_CI8)
    {
        GXInitTexObjCI(&m_TexObj, m_SwizzledData, m_Width, m_Height, (GXCITexFmt)gx_format[m_Format], GX_CLAMP, GX_CLAMP, m_Levels > 1 ? 1 : 0, 0);
        GXInitTexObjLOD(&m_TexObj, (m_Levels == 1) ? GX_LINEAR : GX_LIN_MIP_NEAR, GX_LINEAR, 0.0f, (float)(m_MaxLevel - 1), 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
        return;
    }
    GXInitTexObj(&m_TexObj, m_SwizzledData, m_Width, m_Height, gx_format[m_Format], GX_CLAMP, GX_CLAMP, m_Levels > 1 ? 1 : 0);
    GXInitTexObjLOD(&m_TexObj, (m_Levels == 1) ? GX_LINEAR : GX_LIN_MIP_LIN, GX_LINEAR, 0.0f, (float)(m_MaxLevel - 1), 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);
}

/**
 * Offset/Address/Size: 0x47C | 0x801B7738 | size: 0x68
 */
void PlatTexture::Swizzle(bool bDeleteLinear)
{
    GCSwizzle(m_SwizzledData, m_LinearData, m_Width, m_Height, m_Format, false);

    if (bDeleteLinear)
    {
        nlFree(m_LinearData);
        m_LinearData = NULL;
    }
}

/**
 * Offset/Address/Size: 0x4E4 | 0x801B77A0 | size: 0xD8
 */
void PlatTexture::Create(int width, int height, eGXTextureFormat format, int numLevels, bool bLinearData, bool bNewResourceMemory)
{
    if (m_LinearData != NULL)
    {
        nlFree(m_LinearData);
        m_LinearData = NULL;
    }

    m_Width = width;
    m_Height = height;
    m_Levels = (u8)numLevels;
    m_MaxLevel = (u8)numLevels;
    m_Format = format;

    u32 textureSize = GCTextureSize(format, width, height, numLevels, -1);

    if (bNewResourceMemory)
    {
        m_SwizzledData = nlMalloc(textureSize, 0x20, false);
    }
    else
    {
        m_SwizzledData = glResourceAlloc(textureSize, GLM_TextureData);
    }

    if (bLinearData)
    {
        m_LinearData = nlMalloc(textureSize, 0x20, false);
    }
    else
    {
        m_LinearData = NULL;
    }
}

/**
 * Offset/Address/Size: 0x5BC | 0x801B7878 | size: 0x24
 */
void PlatTexture::CreateWithMemory(int width, int height, eGXTextureFormat format, int numLevels, const void* pTextureData)
{
    m_Width = width;
    m_Height = height;
    m_Levels = (u8)numLevels;
    m_MaxLevel = (u8)numLevels;
    m_Format = format;
    m_SwizzledData = (void*)pTextureData;
    m_LinearData = NULL;
}

/**
 * Offset/Address/Size: 0x5E0 | 0x801B789C | size: 0x64
 */
PlatTexture::~PlatTexture()
{
    if (m_LinearData != NULL)
    {
        nlFree(m_LinearData);
        m_LinearData = NULL;
    }
}

/**
 * Offset/Address/Size: 0x644 | 0x801B7900 | size: 0xA8
 */
PlatTexture* glx_CreatePlatTexture()
{
    PlatTexture* pTex = (PlatTexture*)glResourceAlloc(sizeof(PlatTexture), GLM_Header);
    if (pTex != NULL)
    {
        pTex->m_unk8 = 0x50544558;
        pTex->m_Width = 0;
        pTex->m_Height = 0;
        pTex->m_Levels = 0;
        pTex->m_MaxLevel = 0;
        pTex->m_Format = GXTex_Num;
        pTex->m_nPaletteEntries = 0;
        pTex->m_bMissingTexture = false;
        pTex->m_SwizzledData = NULL;
        pTex->m_LinearData = NULL;
        pTex->m_PaletteData = NULL;
        memset(&pTex->m_TexObj, 0, 0x20);
        memset(&pTex->m_TlutObj, 0, 0xC);
        memset(pTex->m_Bits, 0xFF, 4);
    }
    return pTex;
}

/**
 * Offset/Address/Size: 0x6EC | 0x801B79A8 | size: 0x104
 */
int glplatTextureGetNumBits(int component)
{
    if (texobj.m_Bits[component] == 0xFF)
    {
        u32 format = texobj.m_Format;
        u8 bits[4] = { 5, 6, 5, 0 };

        switch (format)
        {
        case GXTex_RGB565:
            bits[0] = 5;
            bits[1] = 6;
            bits[2] = 5;
            bits[3] = 0;
            break;

        case GXTex_RGB5A3:
            bits[0] = 5;
            bits[1] = 5;
            bits[2] = 5;
            bits[3] = 3;
            break;

        case GXTex_CMPR:
            break;

        case GXTex_RGBA8:
            bits[0] = 8;
            bits[1] = 8;
            bits[2] = 8;
            bits[3] = 8;
            break;

        case GXTex_I8:
            bits[0] = 8;
            bits[1] = 0;
            bits[2] = 0;
            bits[3] = 0;
            break;

        case GXTex_I4:
            bits[0] = 4;
            bits[1] = 0;
            bits[2] = 0;
            bits[3] = 0;
            break;

        case GXTex_A8:
            break;

        case GXTex_IA8:
            bits[0] = 8;
            bits[1] = 0;
            bits[2] = 0;
            bits[3] = 8;
            break;
        }

        return bits[component];
    }

    return texobj.m_Bits[component];
}

/**
 * Offset/Address/Size: 0x7F0 | 0x801B7AAC | size: 0x10
 */
u32 glplatTextureGetHeight()
{
    return texobj.m_Height;
}

/**
 * Offset/Address/Size: 0x800 | 0x801B7ABC | size: 0x10
 */
u32 glplatTextureGetWidth()
{
    return texobj.m_Width;
}

/**
 * Offset/Address/Size: 0x810 | 0x801B7ACC | size: 0x64
 */
bool glplatTextureLoad(unsigned long texture)
{
    PlatTexture* tex = glx_GetTex(texture, 0, 0);
    if (tex == NULL)
    {
        memset(&texobj, 0, 0x50);
        return false;
    }
    memcpy(&texobj, tex, 0x50);
    return true;
}

/**
 * Offset/Address/Size: 0x874 | 0x801B7B30 | size: 0x20
 */
bool glplatEndLoadTextureBundle(void* data, unsigned long size)
{
    return glxParseTextureBundle((const char*)data);
}

/**
 * Offset/Address/Size: 0x894 | 0x801B7B50 | size: 0xA8
 */
bool glplatBeginLoadTextureBundle(const char* filename, void (*callback)(void*, unsigned long, void*), void* param)
{
    char fullname[256];
    nlStrNCat<char>(fullname, "art/", filename, 0x100);
    if (param == NULL)
    {
        if (nlLoadEntireFileAsync(fullname, callback, param, 0x20, AllocateEnd) == 0)
        {
            return false;
        }
    }
    else if (nlLoadEntireFileAsync(fullname, callback, param, 0x20, (eAllocType)0x17) == 0)
    {
        return false;
    }
    return true;
}

/**
 * Offset/Address/Size: 0x93C | 0x801B7BF8 | size: 0xB8
 */
bool glxParseTextureBundle(const char* filedata)
{
    const int numTextures = *(int*)(filedata + 4);
    const glTexBundleDict* dict = (glTexBundleDict*)(filedata + 0x20);
    const char* textureData = (char*)dict + (numTextures * 0x10);

    for (int i = 0; i < numTextures; i++)
    {
        GXTextureHeader* currentTextureHeader = (GXTextureHeader*)(textureData + dict[i].offset);

        if (glxTextureLoad_cb == NULL)
        {
            glplatTextureAdd(dict[i].hash, currentTextureHeader, dict[i].fileSize);
        }
        else
        {
            unsigned long textureHandle = glxTextureLoad_cb(dict[i].hash);
            if (textureHandle != -1 && glTextureLoad(textureHandle) != 0)
            {
                glplatTextureReplace(textureHandle, currentTextureHeader, dict[i].fileSize);
            }
        }
    }

    GXInvalidateTexAll();
    return true;
}

/**
 * Offset/Address/Size: 0x9F4 | 0x801B7CB0 | size: 0x1D0
 */
/**
 * Offset/Address/Size: 0xAF4 | 0x801B7CB0 | size: 0x1D0
 * TODO: 99.05% match - MWCC still elides uSize/uBaseOffset register copy
 *       before dictionary nlMalloc (missing mr r25,r29; uses r29 directly)
 */
bool glplatLoadTextureBundle(const char* filename)
{
    char fullFilename[256];
    unsigned long uBaseOffset;
    nlFile* pFile;
    glTexBundleDict* pDictionary;
    glTexBundleHeader* pHeader;
    unsigned char* pData;
    unsigned long uNumFiles;
    unsigned long uSize;

    glx_FreeMemory0();
    nlStrNCat<char>(fullFilename, "art/", filename, 0x100);

    pFile = nlOpen(fullFilename);
    if (pFile == NULL)
    {
        nlPrintf("file '%s' not found\n", filename);
    }

    pHeader = (glTexBundleHeader*)nlMalloc(0x20, 0x20, 1);
    nlRead(pFile, pHeader, 0x20);

    uNumFiles = pHeader->numTextures;
    uSize = uNumFiles * sizeof(glTexBundleDict);

    pDictionary = (glTexBundleDict*)nlMalloc((uBaseOffset = uSize), 0x20, 1);
    nlRead(pFile, pDictionary, uSize);

    pData = (unsigned char*)nlMalloc(0x40800, 0x20, 1);
    nlQSort<glTexBundleDict>(pDictionary, uNumFiles, BundleSortProc);

    uBaseOffset = uSize + 0x20;

    for (unsigned long i = 0; i < uNumFiles; i++)
    {
        nlSeek(pFile, uBaseOffset + pDictionary[i].offset, 0);
        nlRead(pFile, pData, pDictionary[i].fileSize);
        if (glxTextureLoad_cb == NULL)
        {
            glplatTextureAdd(pDictionary[i].hash, pData, pDictionary[i].fileSize);
        }
        else
        {
            unsigned long newHash = glxTextureLoad_cb(pDictionary[i].hash);
            if (newHash != -1)
            {
                if (glTextureLoad(newHash) != 0)
                {
                    glplatTextureReplace(newHash, pData, pDictionary[i].fileSize);
                }
                else
                {
                    glplatTextureAdd(newHash, pData, pDictionary[i].fileSize);
                }
            }
        }
    }

    nlFree(pData);
    nlFree(pDictionary);
    nlFree(pHeader);
    nlClose(pFile);
    GXInvalidateTexAll();
    glx_FreeMemory1(filename);

    return true;
}

/**
 * Offset/Address/Size: 0xBC4 | 0x801B7E80 | size: 0x10
 */
int BundleSortProc(const glTexBundleDict* a, const glTexBundleDict* b)
{
    return a->offset - b->offset;
}

/**
 * Offset/Address/Size: 0xBD4 | 0x801B7E90 | size: 0x32C
 */
PlatTexture* glx_MakeTexture(GXTextureHeader* header, unsigned long texhandle)
{
    PlatTexture* pTex;
    u16 width;
    u16 height;
    eGXTextureFormat format;
    unsigned long numLevels;
    unsigned long numEntries;
    int textureSize;

    textureSize = GCTextureSize(header->format, header->width, header->height, header->numLevels, texhandle);

    // Allocate PlatTexture (inline version of glx_CreatePlatTexture)
    pTex = (PlatTexture*)glResourceAlloc(sizeof(PlatTexture), GLM_Header);
    if (pTex != NULL)
    {
        pTex->m_unk8 = 0x50544558;
        pTex->m_Width = 0;
        pTex->m_Height = 0;
        pTex->m_Levels = 0;
        pTex->m_MaxLevel = 0;
        pTex->m_Format = GXTex_Num;
        pTex->m_nPaletteEntries = 0;
        pTex->m_bMissingTexture = false;
        pTex->m_SwizzledData = NULL;
        pTex->m_LinearData = NULL;
        pTex->m_PaletteData = NULL;
        memset(&pTex->m_TexObj, 0, 0x20);
        memset(&pTex->m_TlutObj, 0, 0xC);
        memset(pTex->m_Bits, 0xFF, 4);
    }

    // Load header values into local vars
    numLevels = header->numLevels;
    format = header->format;
    height = header->height;
    width = header->width;

    // Free linear data if present
    if (pTex->m_LinearData != NULL)
    {
        nlFree(pTex->m_LinearData);
        pTex->m_LinearData = NULL;
    }

    // Set dimensions
    pTex->m_Width = width;
    pTex->m_Height = height;
    pTex->m_Levels = (u8)numLevels;
    pTex->m_MaxLevel = (u8)numLevels;
    pTex->m_Format = format;

    // Allocate swizzled data
    pTex->m_SwizzledData = glResourceAlloc(GCTextureSize(format, width, height, numLevels, -1), GLM_TextureData);
    pTex->m_LinearData = NULL;

    // Copy bits from header
    memcpy(pTex->m_Bits, header->numBits, 4);

    // Set missing texture flag
    pTex->m_bMissingTexture = header->missingTexture ? true : false;

    // Handle palette
    numEntries = header->numEntries;
    if (numEntries != 0)
    {
        pTex->m_PaletteData = (u16*)glResourceAlloc(numEntries * 2, GLM_TextureData);
        pTex->m_nPaletteEntries = (s16)numEntries;
        memcpy(pTex->m_PaletteData, (u8*)&header[1] + textureSize, header->numEntries * 2);
    }

    // Copy texture data
    memcpy(pTex->m_SwizzledData, (const u8*)header + 0x20, textureSize);

    pTex->Prepare();

    return pTex;
}

/**
 * Offset/Address/Size: 0xF00 | 0x801B81BC | size: 0x94
 */
bool glx_AddTex(unsigned long handle, PlatTexture* platTex)
{
    AVLTreeNode* existingNode;
    nlAVLTree<unsigned long, PlatTexture*, DefaultKeyCompare<unsigned long> >* textureTree;

    if ((platTex == NULL) || (handle == -1))
    {
        return false;
    }
    textureTree = textures[currentMarkerLevel];

    textures[currentMarkerLevel]->AddAVLNode((AVLTreeNode**)&textures[currentMarkerLevel]->m_Root, &handle, &platTex, &existingNode, textures[currentMarkerLevel]->m_NumElements);

    if (existingNode == NULL)
    {
        textureTree->m_NumElements += 1;
    }
    return true;
}

/**
 * Offset/Address/Size: 0xF94 | 0x801B8250 | size: 0x194
 * TODO: 99.90% match - grid-call still computes shifted args into r3/r4 in the
 *       opposite order (target wants r4 from height, r3 from width), and the
 *       missing-text debug literal still binds to a different pool label
 */

struct TextureFindHelper
{
    char pad[0x8];
    AVLTreeEntry<unsigned long, PlatTexture*>* m_Root;

    inline bool FindGet(unsigned long key, PlatTexture*** foundValue) const
    {
        AVLTreeEntry<unsigned long, PlatTexture*>* node = m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;

            if (cmpResult == 0)
            {
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<unsigned long, PlatTexture*>*)node->node.left;
                else
                    node = (AVLTreeEntry<unsigned long, PlatTexture*>*)node->node.right;
            }
        }
        return false;
    }
};

PlatTexture* glx_GetTex(unsigned long handle, bool bMissingFatal, bool bAllowGrids)
{
    PlatTexture** tex;
    GLTextureAnim* pAnim;

    // Check if handle is already a PlatTexture pointer (high byte 0x80)
    if (((handle & 0xFF000000) + 0x80000000) == 0)
    {
        // Check for "PTEX" signature (0x50544558)
        if ((*(unsigned long*)handle - 0x50540000) == 0x4558)
        {
            return (PlatTexture*)handle;
        }
    }

    // Try to get texture animation
    pAnim = glInventory.GetTextureAnim(handle);
    if (pAnim != NULL)
    {
        handle = pAnim->GetTexture(-1)->textureHandle;
    }

    for (int index = currentMarkerLevel; index >= 0; index--)
    {
        bool found = ((TextureFindHelper*)textures[index])->FindGet(handle, &tex);

        if (found)
        {
            if (bAllowGrids && glx_bGridMode)
            {
                PlatTexture* gridTex = glx_GetGridTexture((u32)(u16)(*tex)->m_Height >> 2, (u32)(*tex)->m_Width >> 2);
                if (gridTex != NULL)
                {
                    return gridTex;
                }
            }
            return *tex;
        }
    }

    // Texture not found
    if (bMissingFatal)
    {
        tDebugPrintManager::Print(DC_GLPLAT, "texture 0x%08X not found\n", handle);
    }
    return NULL;
}

inline bool IsValidTextureDimension(int dimension)
{
    bool validDimensions = false;
    switch (dimension)
    {
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
    case 512:
        validDimensions = true;
        break;
    default:
        validDimensions = false;
        break;
    }
    return validDimensions;
}

/**
 * Offset/Address/Size: 0x1128 | 0x801B83E4 | size: 0x1C4
 */
PlatTexture* glx_GetGridTexture(int width, int height)
{
    FORCE_DONT_INLINE;
    if (!IsValidTextureDimension(width) || !IsValidTextureDimension(height))
    {
        return nullptr;
    }

    if (width < 8 || height < 8)
    {
        return nullptr;
    }

    ListEntry<PlatTexture*>* current = gridTextures.m_Head;
    while (current != nullptr)
    {
        PlatTexture* texture = current->data;
        if (width == texture->m_Width && height == texture->m_Height)
        {
            return texture;
        }
        current = current->next;
    }

    PlatTexture* newTexture = glx_MakeGridTexture(width, height);
    ListEntry<PlatTexture*>* listEntry = new (nlMalloc(8, 8, false)) ListEntry<PlatTexture*>(newTexture);

    nlListAddStart<PlatTexture*>(&gridTextures.m_Head, listEntry, &gridTextures.m_Tail);
    return gridTextures.m_Head->data;
}

/**
 * Offset/Address/Size: 0x12EC | 0x801B85A8 | size: 0x53C
 * TODO: ~90% match - kDefaultBits load scheduling (context-dependent),
 *       gridColor 0xFFFF not materialized as lis+addi (compiler folds u16 constant),
 *       loop register allocation shifted by missing gridColor register
 */
PlatTexture* glx_MakeGridTexture(int w, int h)
{
    PlatTexture* pTex;
    u8 bits[4];

    // Store default bits
    *(u32*)bits = *(u32*)kDefaultBits;

    // Allocate PlatTexture
    pTex = (PlatTexture*)nlMalloc(sizeof(PlatTexture), 8, false);

    if (pTex != NULL)
    {
        pTex->m_unk8 = 0x50544558;
        pTex->m_Width = 0;
        pTex->m_Height = 0;
        pTex->m_Levels = 0;
        pTex->m_MaxLevel = 0;
        pTex->m_Format = GXTex_Num;
        pTex->m_nPaletteEntries = 0;
        pTex->m_bMissingTexture = false;
        pTex->m_SwizzledData = NULL;
        pTex->m_LinearData = NULL;
        pTex->m_PaletteData = NULL;
        memset(&pTex->m_TexObj, 0, 0x20);
        memset(&pTex->m_TlutObj, 0, 0xC);
        memset(pTex->m_Bits, 0xFF, 4);
    }

    // Free linear data if present
    if (pTex->m_LinearData != NULL)
    {
        nlFree(pTex->m_LinearData);
        pTex->m_LinearData = NULL;
    }

    // Set dimensions and format
    pTex->m_Width = (u16)w;
    pTex->m_Height = (u16)h;
    pTex->m_Levels = 1;
    pTex->m_MaxLevel = 1;
    pTex->m_Format = GXTex_RGB565;

    // Allocate texture data
    u32 textureSize = GCTextureSize(GXTex_RGB565, w, h, 1, -1);
    pTex->m_SwizzledData = nlMalloc(textureSize, 0x20, false);
    pTex->m_LinearData = nlMalloc(textureSize, 0x20, false);

    // Copy bits
    memcpy(pTex->m_Bits, bits, 4);

    // Generate checkerboard pattern
    u16 gridColor = 0xFFFF;
    u16* pData = (u16*)pTex->m_LinearData;

    for (int y = 0; y < h; y++)
    {
        if ((y / 4) & 1)
        {
            // Odd row of grid blocks
            for (int x = 0; x < w; x++)
            {
                int gridx = x / 4;
                pData[y * w + x] = gridColor & ~(gridx & 1 ? 0 : -1);
            }
        }
        else
        {
            // Even row of grid blocks
            for (int x = 0; x < w; x++)
            {
                int gridx = x / 4;
                pData[y * w + x] = gridColor & ~(gridx & 1 ? -1 : 0);
            }
        }
    }

    // Update grid memory counter - includes sizeof(PlatTexture)
    nGridMemory += w * h * 2 + 0x50;
    tDebugPrintManager::Print(DC_GL, "grid [%d %d] now using %uKB\n", w, h, nGridMemory / 1024);

    // Swizzle and prepare texture
    pTex->Swizzle(true);
    pTex->Prepare();

    return pTex;
}

/**
 * Offset/Address/Size: 0x1828 | 0x801B8AE4 | size: 0x10
 */
bool glx_SetGridMode(bool bGrid)
{
    bool prevGridMode = glx_bGridMode;
    glx_bGridMode = bGrid;
    return prevGridMode;
}

/**
 * Offset/Address/Size: 0x1838 | 0x801B8AF4 | size: 0xA4
 */
void glxInitTex()
{
    currentMarkerLevel = -1;
    for (int i = 0; i < 16; i++)
    {
        textures[i] = new (nlMalloc(0x14, 8, 0)) nlAVLTree<unsigned long, PlatTexture*, DefaultKeyCompare<unsigned long> >();
    }
    currentMarkerLevel += 1;
}

/**
 * Offset/Address/Size: 0x18DC | 0x801B8B98 | size: 0xA0
 */
void glx_BackupTexMarkerLevel(int level)
{
    while (currentMarkerLevel != level)
    {
        TexDestructor texDtor;
        textures[currentMarkerLevel]->Walk<TexDestructor>(&texDtor, &TexDestructor::CallDestructor);
        textures[currentMarkerLevel]->Clear();
        currentMarkerLevel -= 1;
    }
}

/**
 * Offset/Address/Size: 0x19C4 | 0x801B8C80 | size: 0x10
 */
void glx_AdvanceTexMarkerLevel()
{
    currentMarkerLevel += 1;
}

/**
 * Offset/Address/Size: 0x19D4 | 0x801B8C90 | size: 0x8
 */
int glx_GetTexMarkerLevel()
{
    return currentMarkerLevel;
}

/**
 * Offset/Address/Size: 0x19DC | 0x801B8C98 | size: 0x10
 */
glxTextureLoadCallback_t glx_SetLoadCallback(glxTextureLoadCallback_t callback)
{
    unsigned long (*oldCallback)(unsigned long) = glxTextureLoad_cb;
    glxTextureLoad_cb = callback;
    return oldCallback;
}
// /**
//  * Offset/Address/Size: 0x0 | 0x801B8CA8 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::DeleteEntry(AVLTreeEntry<unsigned long, PlatTexture*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x24 | 0x801B8CCC | size: 0x58
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0x7C | 0x801B8D24 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::DestroyTree(void (AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>,
// DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, PlatTexture*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xE0 | 0x801B8D88 | size: 0x758
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::PostorderTraversal(AVLTreeEntry<unsigned long, PlatTexture*>*, void (AVLTreeBase<unsigned long, PlatTexture*,
// NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, PlatTexture*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x838 | 0x801B94E0 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::CastUp(AVLTreeNode*) const
// {
// }

/**
 * Offset/Address/Size: 0x840 | 0x801B94E8 | size: 0x44
 * TODO: 96.47% match - prologue scheduling differs (`lwz r8, 0(r5)` placement).
 */
// /**
//  * Offset/Address/Size: 0x884 | 0x801B952C | size: 0x2CC
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::InorderWalk<TexDestructor>(AVLTreeEntry<unsigned long, PlatTexture*>*, TexDestructor*, void (TexDestructor::*)(const unsigned
// long&, PlatTexture**))
// {
// }

// /**
//  * Offset/Address/Size: 0xB50 | 0x801B97F8 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0xB7C | 0x801B9824 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0xBA8 | 0x801B9850 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned
// long>>::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801B98B4 | size: 0x28
//  */
// void nlQSort<glTexBundleDict>(glTexBundleDict*, int, int (*)(const glTexBundleDict*, const glTexBundleDict*))
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x801B98DC | size: 0x28
 */
template void nlListAddStart<ListEntry<PlatTexture*> >(ListEntry<PlatTexture*>**, ListEntry<PlatTexture*>*, ListEntry<PlatTexture*>**);

// /**
//  * Offset/Address/Size: 0x28 | 0x801B9904 | size: 0x68
//  */
// void nlWalkList<ListEntry<PlatTexture*>, ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>>(ListEntry<PlatTexture*>*,
// ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>*, void (ListContainerBase<PlatTexture*,
// NewAdapter<ListEntry<PlatTexture*>>>::*)(ListEntry<PlatTexture*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xE4 | 0x801B9A50 | size: 0x24
//  */
// void ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>::DeleteEntry(ListEntry<PlatTexture*>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x108 | 0x801B9A74 | size: 0x8C
//  */
// void nlListContainer<PlatTexture*>::~nlListContainer()
// {
// }
