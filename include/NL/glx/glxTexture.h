#ifndef _GLXTEXTURE_H_
#define _GLXTEXTURE_H_

#include "types.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"

#include "NL/nlMemory.h"

typedef unsigned long (*glxTextureLoadCallback_t)(unsigned long);

enum eGXTextureFormat
{
    GXTex_RGB565 = 0,
    GXTex_RGB5A3 = 1,
    GXTex_CMPR = 2,
    GXTex_RGBA8 = 3,
    GXTex_I8 = 4,
    GXTex_I4 = 5,
    GXTex_A8 = 6,
    GXTex_IA8 = 7,
    GXTex_CI8 = 8,
    GXTex_Num = 9,
};

enum eGLTextureType
{
    GLTT_Diffuse = 0,
    GLTT_Detail = 1,
    GLTT_Shadow = 2,
    GLTT_SelfIllum = 3,
    GLTT_Gloss = 4,
    GLTT_BumpLocal = 5,
    GLTT_Num = 6,
};

enum eGLTextureState
{
    GLTS_DiffuseWrap = 0,
    GLTS_DetailWrap = 1,
    GLTS_ShadowWrap = 2,
    GLTS_SelfIllumWrap = 3,
    GLTS_GlossWrap = 4,
    GLTS_BumpLocalWrap = 5,
    GLTS_DiffuseFilter = 6,
    GLTS_DetailFilter = 7,
    GLTS_ShadowFilter = 8,
    GLTS_SelfIllumFilter = 9,
    GLTS_GlossFilter = 10,
    GLTS_BumpLocalFilter = 11,
    GLTS_DiffuseLevel = 12,
    GLTS_DetailLevel = 13,
    GLTS_ShadowLevel = 14,
    GLTS_SelfIllumLevel = 15,
    GLTS_GlossLevel = 16,
    GLTS_BumpLocalLevel = 17,
    GLTS_Num = 18,
};

enum eGLTextureFormatType
{
    eGLTextureFormatType_0,
};

struct BundleEntry
{
    /* 0x0 */ unsigned long hash;
    /* 0x4 */ unsigned long offset;
    /* 0x8 */ unsigned long fileSize;
    /* 0xC */ unsigned long pad;
}; // total size: 0x10

struct glTexBundleDict : public BundleEntry
{
}; // total size: 0x10

struct BundleHeader
{
    /* 0x0 */ unsigned long magic;
    /* 0x4 */ unsigned long numTextures;
    /* 0x8 */ unsigned long pad1;
    /* 0xC */ unsigned long pad2;
}; // total size: 0x10

struct glTexBundleHeader : public BundleHeader
{
    /* 0x10 */ unsigned long pad[4]; // size 0x10
}; // total size: 0x20

struct GXTextureHeader
{
    /* 0x00 */ unsigned long numLevels;
    /* 0x04 */ eGXTextureFormat format;
    /* 0x08 */ unsigned char numBits[4];
    /* 0x0C */ unsigned char missingTexture;
    /* 0x0E */ unsigned short width;
    /* 0x10 */ unsigned short height;
    /* 0x14 */ unsigned long numEntries;
    /* 0x18 */ unsigned long pad[2];
}; // total size: 0x20

class PlatTexture
{
public:
    PlatTexture()
        : m_unk8(0x50544558)
        , m_Width(0)
        , m_Height(0)
        , m_Levels(0)
        , m_MaxLevel(0)
        , m_Format(GXTex_Num)
        , m_nPaletteEntries(0)
        , m_bMissingTexture(false)
        , m_SwizzledData(NULL)
        , m_LinearData(NULL)
        , m_PaletteData(NULL)
    {
        memset(&m_TexObj, 0, sizeof(m_TexObj));
        memset(&m_TlutObj, 0, sizeof(m_TlutObj));
        memset(m_Bits, 0xFF, sizeof(m_Bits));
    }
    ~PlatTexture();

    void Prepare();
    void Swizzle(bool);
    void Create(int, int, eGXTextureFormat, int, bool, bool);
    void CreateWithMemory(int, int, eGXTextureFormat, int, const void*);

    /* 0x00 */ int m_unk8;
    /* 0x04 */ u16 m_Width;
    /* 0x06 */ u16 m_Height;
    /* 0x08 */ u8 m_Levels;
    /* 0x09 */ u8 m_MaxLevel;
    /* 0x0C */ eGXTextureFormat m_Format;
    /* 0x10 */ s16 m_nPaletteEntries;
    /* 0x12 */ bool m_bMissingTexture;
    /* 0x14 */ void* m_SwizzledData;
    /* 0x18 */ void* m_LinearData;
    /* 0x1C */ u16* m_PaletteData;
    /* 0x20 */ u8 m_Bits[4];
    /* 0x24 */ _GXTexObj m_TexObj;   // size 0x20
    /* 0x44 */ _GXTlutObj m_TlutObj; // size 0xC
}; // total size: 0x50

void glplatTextureReplace(unsigned long, const void*, unsigned long);
void glplatTextureAdd(unsigned long, const void*, unsigned long);
PlatTexture* glx_CreatePlatTexture();
int glplatTextureGetNumBits(int);
u32 glplatTextureGetHeight();
u32 glplatTextureGetWidth();
bool glplatTextureLoad(unsigned long);
bool glplatEndLoadTextureBundle(void* data, unsigned long size);
bool glplatBeginLoadTextureBundle(const char* filename, void (*callback)(void*, unsigned long, void*), void* param);
bool glxParseTextureBundle(const char*);
bool glplatLoadTextureBundle(const char* filename);
WEAKFUNC int BundleSortProc(const glTexBundleDict* a, const glTexBundleDict* b);
PlatTexture* glx_MakeTexture(GXTextureHeader* header, unsigned long handle);
bool glx_AddTex(unsigned long handle, PlatTexture* platTex);
PlatTexture* glx_GetTex(unsigned long, bool, bool);
PlatTexture* glx_GetGridTexture(int width, int height);
PlatTexture* glx_MakeGridTexture(int width, int height);
bool glx_SetGridMode(bool bGrid);
void glxInitTex();
void glx_BackupTexMarkerLevel(int level);
void glx_AdvanceTexMarkerLevel();
int glx_GetTexMarkerLevel();
glxTextureLoadCallback_t glx_SetLoadCallback(glxTextureLoadCallback_t callback);

class TexDestructor
{
public:
    /**
     * Offset/Address/Size: 0x197C | 0x801B8C38 | size: 0x48
     */
    void CallDestructor(const unsigned long&, PlatTexture** texture)
    {
        PlatTexture* texture_ptr;
        void* linear_data;

        texture_ptr = *texture;
        if (texture_ptr != NULL)
        {
            linear_data = texture_ptr->m_LinearData;
            if (linear_data != NULL)
            {
                nlFree(linear_data);
                texture_ptr->m_LinearData = NULL;
            }
        }
    }
};

// void nlQSort<glTexBundleDict>(glTexBundleDict*, int, int (*)(const glTexBundleDict*, const glTexBundleDict*));
// void nlListAddStart<ListEntry<PlatTexture*>>(ListEntry<PlatTexture*>**, ListEntry<PlatTexture*>*, ListEntry<PlatTexture*>**);

// class AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void DeleteEntry(AVLTreeEntry<unsigned long, PlatTexture*>*);
//     void Clear();
//     void DestroyTree(void (AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>,
//     DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, PlatTexture*>*)); void PostorderTraversal(AVLTreeEntry<unsigned
//     long, PlatTexture*>*, void (AVLTreeBase<unsigned long, PlatTexture*, NewAdapter<AVLTreeEntry<unsigned long, PlatTexture*>>,
//     DefaultKeyCompare<unsigned long>>::*)(AVLTreeEntry<unsigned long, PlatTexture*>*)); void CastUp(AVLTreeNode*) const; void
//     Walk<TexDestructor>(TexDestructor*, void (TexDestructor::*)(const unsigned long&, PlatTexture**)); void
//     InorderWalk<TexDestructor>(AVLTreeEntry<unsigned long, PlatTexture*>*, TexDestructor*, void (TexDestructor::*)(const unsigned long&,
//     PlatTexture**)); void CompareNodes(AVLTreeNode*, AVLTreeNode*); void CompareKey(void*, AVLTreeNode*); void AllocateEntry(void*,
//     void*);
// };

// class nlWalkList<ListEntry<PlatTexture*>, ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>>(ListEntry<PlatTexture*>*,
// ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>*, void (ListContainerBase<PlatTexture*,
// NewAdapter<ListEntry<PlatTexture*>>>
// {
// public:
//     void *)(ListEntry<PlatTexture*>*));
// };

// class ListContainerBase<PlatTexture*, NewAdapter<ListEntry<PlatTexture*>>>
// {
// public:
//     void DeleteEntry(ListEntry<PlatTexture*>*);
// };

// class nlListContainer<PlatTexture*>
// {
// public:
//     void ~nlListContainer();
// };

#endif // _GLXTEXTURE_H_
