#include "dolphin/types.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"
#include "NL/glx/glxMemory.h"
#include "NL/glx/glxTexture.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glConstant.h"
#include "NL/nlDebug.h"
#include "dolphin/os.h"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTexture.h"
#include "Game/Sys/debug.h"
#include "Game/GL/GLInventory.h"

bool glx_MemoryDump;
u32 ResourceMemSize;
u32 p_phys;
u32 n_phys;
u32 i_frame;
u32 glx_mem0;
u32 g_uResourceMarker;

u32 p_frame[4];
u32 n_frame[4];

u32 FrameMemSizeVirt;
u32 FrameMemSizeReal;
u32 FrameMemSizes[2];

GLXMemoryInfo g_uResourceAlloc[16];

// GLInventory glInventory;

extern "C"
{
    typedef void (*ConstructorDestructor)(void*);
    void __construct_array(void* ptr, ConstructorDestructor ctor, ConstructorDestructor dtor, unsigned long size, unsigned long n);
    void __ct__13GLXMemoryInfoFv(void*);
}

/**
 * Offset/Address/Size: 0x930 | 0x801B7258 | size: 0x64
 */
extern "C" void __sinit_glxMemory_cpp(void)
{
    ResourceMemSize = 0xBFD000;
    FrameMemSizeReal = 0xE0000;
    FrameMemSizeVirt = 0xA0000;
    FrameMemSizes[0] = FrameMemSizeReal;
    FrameMemSizes[1] = FrameMemSizeVirt;
    __construct_array(g_uResourceAlloc, (ConstructorDestructor)__ct__13GLXMemoryInfoFv, 0, sizeof(GLXMemoryInfo), 16);
}

const char* szMemoryNames[] = {
    "header",
    "matrix",
    "index",
    "vert",
    "tex",
    "target"
};

/**
 * Offset/Address/Size: 0x0 | 0x801B6928 | size: 0x20
 */
void glplatSetMatrix(unsigned long addr, const nlMatrix4& matrix)
{
    GLMatrix* matrixPtr = (GLMatrix*)addr;
    matrixPtr->Set(matrix);
}

/**
 * Offset/Address/Size: 0x20 | 0x801B6948 | size: 0x20
 */
void glplatGetMatrix(unsigned long addr, nlMatrix4& matrix)
{
    GLMatrix* matrixPtr = (GLMatrix*)addr;
    matrixPtr->Get(matrix);
}

/**
 * Offset/Address/Size: 0x40 | 0x801B6968 | size: 0x100
 */
void glplatFrameAllocNextFrame()
{
    if (glx_MemoryDump)
    {
        tDebugPrintManager::Print(DC_GLPLAT, "memory used: %uKB resource, %uKB frame real, %uKB frame virt\n", n_phys >> 10, n_frame[i_frame * 2] >> 10, n_frame[i_frame * 2 + 1] >> 10);

        tDebugPrintManager::Print(DC_GLPLAT, "       free: %uKB resource, %uKB frame real, %uKB frame virt\n", (ResourceMemSize - n_phys) >> 10, (FrameMemSizes[0] - n_frame[i_frame * 2]) >> 10, (FrameMemSizes[1] - n_frame[i_frame * 2 + 1]) >> 10);

        glx_MemoryDump = false;
    }

    u32 newFrame = i_frame ^ 1;
    i_frame = newFrame;
    n_frame[newFrame * 2] = 0;
    n_frame[newFrame * 2 + 1] = 0;

    GXInvalidateVtxCache();
    GXInvalidateTexAll();
}

/**
 * Offset/Address/Size: 0x140 | 0x801B6A68 | size: 0xC0
 */
u32 glplatFrameAlloc(unsigned long size, eGLMemory mem)
{
    u32 isLow;
    switch (mem)
    {
    case GLM_Header:
    case GLM_Matrix:
    case GLM_IndexData:
        isLow = 1;
        break;
    default:
        isLow = 0;
        break;
    }

    u32(*nf)[2] = (u32(*)[2])n_frame;
    u32(*pf)[2] = (u32(*)[2])p_frame;
    u32 out;
    u32 n = nf[i_frame][isLow];
    u32 p = pf[i_frame][isLow];
    u32 sum = p + n + 0x1F;
    out = sum & ~0x1F;
    u32 next = size + (out - p);

    if (next > FrameMemSizes[isLow])
    {
        OSReport("out of frame memory (%s)\n", szMemoryNames[mem]);
        nlBreak();
        out = 0;
    }
    else
    {
        nf[i_frame][isLow] = next;
    }

    return out;
}

/**
 * Offset/Address/Size: 0x200 | 0x801B6B28 | size: 0x110
 * TODO: 87.1% match - loop uses addic./bne instead of mtctr/bdnz (CTR loop),
 * causing register allocation differences (r8 vs r4 accumulator) and loop setup reordering
 */
void glplatResourceRelease(unsigned long long resourceId)
{
    int level = (int)(resourceId & 0xFFFFFFFF);

    n_phys = (u32)(resourceId >> 32);

    glx_BackupTexMarkerLevel(level);
    gl_ConstantMarkerBackup(level);
    glInventory.ResourceRelease(level);

    while ((s32)g_uResourceMarker != level)
    {
        g_uResourceMarker--;
    }

    u32 totalAlloc = 0;
    u32 totalTex = 0;
    s32 n = g_uResourceMarker;
    s32 count = n + 1;
    GLXMemoryInfo* p = g_uResourceAlloc;
    if (n >= 0)
    {
        do
        {
            u32 a = p->m_uBytes[0];
            u32 b = p->m_uBytes[1];
            u32 c = p->m_uBytes[2];
            a = a + b;
            b = p->m_uBytes[3];
            a = a + c;
            u32 e = p->m_uBytes[4];
            a = a + b;
            u32 g = p->m_uTexBundle;
            u32 f = p->m_uBytes[5];
            a = a + e;
            u32 tex = e - g;
            p++;
            a = a + f;
            totalAlloc = totalAlloc + a;
            totalTex = totalTex + tex;
        } while (--count != 0);
    }

    f32 fConst = 1.0f / 1024.0f;
    f32 fMB = (f32)totalAlloc * fConst;
    tDebugPrintManager::Print(DC_GLPLAT, "total : (%0.3fMB) %uKB .. %uKB dTex\n", totalAlloc >> 10, totalTex >> 10, fMB * fConst);
}

/**
 * Offset/Address/Size: 0x310 | 0x801B6C38 | size: 0x130
 * TODO: 81.3% match - Known MWCC limitation: do/while(--count) with complex
 * loop body (7 lwz + adds) produces addic./bne instead of mtctr/bdnz; also
 * or operand order swap at line 24 (r4,r0 vs r0,r4). See mwcc-patterns.md.
 */
unsigned long long glplatResourceMark()
{
    int texLevel = glx_GetTexMarkerLevel();
    unsigned long long resourceId = texLevel | ((unsigned long long)n_phys << 32);

    glx_AdvanceTexMarkerLevel();
    gl_ConstantMarkerAdvance();
    glInventory.ResourceMark();

    s32* resourceMarker = (s32*)&g_uResourceMarker;
    s32 newMarker = *resourceMarker;
    newMarker = newMarker + 1;
    GLXMemoryInfo* p = g_uResourceAlloc;
    u32 totalAlloc = 0;
    u32 totalTex = totalAlloc;
    *resourceMarker = newMarker;

    GLXMemoryInfo* entry = &p[newMarker];
    entry->m_uBytes[0] = 0;
    entry->m_uBytes[1] = 0;
    entry->m_uBytes[2] = 0;
    entry->m_uBytes[3] = 0;
    entry->m_uBytes[4] = 0;
    entry->m_uBytes[5] = 0;
    entry->m_uTexBundle = 0;

    u32 count = (u32)(newMarker + 1);
    if (newMarker >= 0)
    {
        do
        {
            u32 a = p->m_uBytes[0];
            u32 b = p->m_uBytes[1];
            u32 c = p->m_uBytes[2];
            a = a + b;
            b = p->m_uBytes[3];
            a = a + c;
            u32 e = p->m_uBytes[4];
            a = a + b;
            u32 g = p->m_uTexBundle;
            u32 f = p->m_uBytes[5];
            a = a + e;
            u32 tex = e - g;
            p++;
            a = a + f;
            totalAlloc = totalAlloc + a;
            totalTex = totalTex + tex;
        } while (--count);
    }

    f32 fConst = 1.0f / 1024.0f;
    f32 fMB = (f32)totalAlloc * fConst;
    tDebugPrintManager::Print(DC_GLPLAT, "total : (%0.3fMB) %uKB .. %uKB dTex\n", totalAlloc >> 10, totalTex >> 10, fMB * fConst);

    return resourceId;
}

/**
 * Offset/Address/Size: 0x440 | 0x801B6D68 | size: 0xB4
 */
unsigned long glplatResourceAlloc(unsigned long uSize, eGLMemory eMemory)
{
    u32 base = p_phys;
    unsigned long aligned = (base + n_phys + 0x1F) & ~0x1FU;
    n_phys = uSize + (aligned - base);
    if (n_phys > ResourceMemSize)
    {
        OSReport("out of resource memory (%s)\n", szMemoryNames[eMemory]);
        nlBreak();
    }
    g_uResourceAlloc[g_uResourceMarker].m_uBytes[eMemory] += uSize;
    return aligned;
}

/**
 * Offset/Address/Size: 0x4F4 | 0x801B6E1C | size: 0x3F0
 */
bool glxInitMemory()
{
    bool bDeveloper = GetConfigBool(Config::Global(), "e3_build", false);
    const char* szResourceKey = bDeveloper ? "e3 resource total memory" : "resource total memory";

    u32 resMemSize;
    {
        u32 oldSize = ResourceMemSize;
        if (Config::Global().Exists(szResourceKey))
        {
            f32 fValue = GetConfigFloat(Config::Global(), szResourceKey, 0.0f);
            resMemSize = (u32)(1024.0f * (1024.0f * fValue));
        }
        else
        {
            resMemSize = oldSize;
        }
    }
    ResourceMemSize = resMemSize;

    u32 pMem = (u32)nlMalloc(ResourceMemSize, 32, false);
    if (pMem == 0)
        return false;
    p_phys = pMem;

    u32 frameMemReal;
    {
        u32 oldSize = FrameMemSizeReal;
        if (Config::Global().Exists("frame vertex memory"))
        {
            f32 fValue = GetConfigFloat(Config::Global(), "frame vertex memory", 0.0f);
            frameMemReal = (u32)(1024.0f * (1024.0f * fValue));
        }
        else
        {
            frameMemReal = oldSize;
        }
    }
    FrameMemSizeReal = frameMemReal;

    pMem = (u32)nlMalloc(frameMemReal * 2, 32, false);
    if (pMem == 0)
        return false;
    p_frame[0] = pMem;
    p_frame[2] = pMem + FrameMemSizeReal;

    u32 frameMemVirt;
    {
        u32 oldSize = FrameMemSizeVirt;
        if (Config::Global().Exists("frame header memory"))
        {
            f32 fValue = GetConfigFloat(Config::Global(), "frame header memory", 0.0f);
            frameMemVirt = (u32)(1024.0f * (1024.0f * fValue));
        }
        else
        {
            frameMemVirt = oldSize;
        }
    }
    FrameMemSizeVirt = frameMemVirt;

    u32 pVirt = (u32)nlVirtualAlloc(frameMemVirt * 2, false);
    if (pVirt == 0)
        return false;

    p_frame[1] = pVirt;
    p_frame[3] = pVirt + FrameMemSizeVirt;
    i_frame = 0;
    n_phys = 0;
    n_frame[2] = 0;
    n_frame[0] = 0;
    n_frame[3] = 0;
    n_frame[1] = 0;
    FrameMemSizes[0] = FrameMemSizeReal;
    FrameMemSizes[1] = FrameMemSizeVirt;

    return true;
}

/**
 * Offset/Address/Size: 0x8E4 | 0x801B720C | size: 0x4
 */
void glx_FreeMemory1(const char*)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x8E8 | 0x801B7210 | size: 0x14
 */
void glx_FreeMemory0()
{
    glx_mem0 = ResourceMemSize - n_phys;
}

/**
 * Offset/Address/Size: 0x8FC | 0x801B7224 | size: 0x10
 */
u32 glx_GetFreeMemory()
{
    return ResourceMemSize - n_phys;
}

/**
 * Offset/Address/Size: 0x90C | 0x801B7234 | size: 0x24
 */
GLXMemoryInfo::GLXMemoryInfo()
{
    for (int i = 0; i < 6; i++)
    {
        m_uBytes[i] = 0;
    }
    m_uTexBundle = 0;
}
