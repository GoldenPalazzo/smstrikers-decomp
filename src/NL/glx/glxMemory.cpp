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

static u8 glx_MemoryDump;
static u32 ResourceMemSize;
static u32 FrameMemSizeReal;
static u32 FrameMemSizeVirt;
static u32 p_phys;
static u32 n_phys;
static int i_frame;
static u32 glx_mem0;
static int g_uResourceMarker;

static u32 p_frame[2][2];
static u32 n_frame[2][2];

static u32 FrameMemSizes[2] = { 0x0, 0x0 };

namespace
{
struct glxMemoryGlobalsInit
{
    glxMemoryGlobalsInit()
    {
        ResourceMemSize = 0xBFD000;
        FrameMemSizeReal = 0xE0000;
        FrameMemSizeVirt = 0xA0000;
        FrameMemSizes[0] = FrameMemSizeReal;
        FrameMemSizes[1] = FrameMemSizeVirt;
    }
};
glxMemoryGlobalsInit s_glxMemoryGlobalsInit;
} // namespace

static GLXMemoryInfo g_uResourceAlloc[16];

static char* szMemoryNames[6] = {
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
        tDebugPrintManager::Print(DC_GLPLAT, "memory used: %uKB resource, %uKB frame real, %uKB frame virt\n", n_phys >> 10, n_frame[i_frame][0] >> 10, n_frame[i_frame][1] >> 10);

        tDebugPrintManager::Print(DC_GLPLAT, "       free: %uKB resource, %uKB frame real, %uKB frame virt\n", (ResourceMemSize - n_phys) >> 10, (FrameMemSizes[0] - n_frame[i_frame][0]) >> 10, (FrameMemSizes[1] - n_frame[i_frame][1]) >> 10);

        glx_MemoryDump = false;
    }

    int newFrame = i_frame ^ 1;
    i_frame = newFrame;
    n_frame[newFrame][0] = 0;
    n_frame[newFrame][1] = 0;

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

    u32 out;
    u32 n = n_frame[i_frame][isLow];
    u32 p = p_frame[i_frame][isLow];
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
        n_frame[i_frame][isLow] = next;
    }

    return out;
}

static inline void ResourceAllocMark()
{
    s32 level = g_uResourceMarker + 1;
    g_uResourceMarker = level;

    GLXMemoryInfo* entry = &g_uResourceAlloc[level];
    entry->m_uBytes[0] = 0;
    entry->m_uBytes[1] = 0;
    entry->m_uBytes[2] = 0;
    entry->m_uBytes[3] = 0;
    entry->m_uBytes[4] = 0;
    entry->m_uBytes[5] = 0;
    entry->m_uTexBundle = 0;
}

static inline void ResourceAllocTotal(u32* pTotalAlloc, u32* pTotalTex)
{
    u32 e;
    GLXMemoryInfo* p = g_uResourceAlloc;
    u32 totalAlloc = 0;
    u32 totalTex = totalAlloc;
    u32 marker = g_uResourceMarker;
    u32 i;
    for (i = 0; i <= marker; i++)
    {
        u32 a = p->m_uBytes[0] + p->m_uBytes[1];
        a = a + p->m_uBytes[2];
        a = a + p->m_uBytes[3];
        e = p->m_uBytes[4];
        u32 g = p->m_uTexBundle;
        a = a + e;
        a = a + p->m_uBytes[5];
        totalAlloc = totalAlloc + a;
        totalTex = totalTex + (e - g);
        p++;
    }

    *pTotalAlloc = totalAlloc;
    *pTotalTex = totalTex;
}

/**
 * Offset/Address/Size: 0x200 | 0x801B6B28 | size: 0x110
 */
#pragma optimize_for_size on
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

    u32 totalAlloc;
    u32 totalTex;
    ResourceAllocTotal(&totalAlloc, &totalTex);

    f32 fConst = 1.0f / 1024.0f;
    f32 fMB = (f32)totalAlloc * fConst;
    tDebugPrintManager::Print(DC_GLPLAT, "total : (%0.3fMB) %uKB .. %uKB dTex\n", fMB * fConst, totalAlloc >> 10, totalTex >> 10);
}
#pragma optimize_for_size reset

/**
 * Offset/Address/Size: 0x310 | 0x801B6C38 | size: 0x130
 */
#pragma push
#pragma optimize_for_size on
#pragma use_lmw_stmw off
unsigned long long glplatResourceMark()
{
    unsigned long long marker = glx_GetTexMarkerLevel() | ((unsigned long long)n_phys << 32);

    glx_AdvanceTexMarkerLevel();
    gl_ConstantMarkerAdvance();
    glInventory.ResourceMark();

    ResourceAllocMark();

    u32 totalAlloc;
    u32 totalTex;
    ResourceAllocTotal(&totalAlloc, &totalTex);

    f32 fConst = 1.0f / 1024.0f;
    f32 fMB = (f32)totalAlloc * fConst;
    tDebugPrintManager::Print(DC_GLPLAT, "total : (%0.3fMB) %uKB .. %uKB dTex\n", fMB * fConst, totalAlloc >> 10, totalTex >> 10);

    return marker;
}
#pragma pop

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
    p_frame[0][0] = pMem;
    p_frame[1][0] = pMem + FrameMemSizeReal;

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

    p_frame[0][1] = pVirt;
    p_frame[1][1] = pVirt + FrameMemSizeVirt;
    i_frame = 0;
    n_phys = 0;
    n_frame[1][0] = 0;
    n_frame[0][0] = 0;
    n_frame[1][1] = 0;
    n_frame[0][1] = 0;
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
