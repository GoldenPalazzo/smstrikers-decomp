#ifndef _GLXMEMORY_H_
#define _GLXMEMORY_H_

#include "NL/nlMath.h"

enum eGLMemory
{
    GLM_Header = 0,
    GLM_Matrix = 1,
    GLM_IndexData = 2,
    GLM_VertexData = 3,
    GLM_TextureData = 4,
    GLM_Target = 5,
    GLM_Num = 6,
};

void glplatSetMatrix(unsigned long addr, const nlMatrix4& matrix);
void glplatGetMatrix(unsigned long addr, nlMatrix4& matrix);
void glplatFrameAllocNextFrame();
u32 glplatFrameAlloc(unsigned long size, eGLMemory mem);
void glplatResourceRelease(unsigned long long resourceId);
unsigned long long glplatResourceMark();
unsigned long glplatResourceAlloc(unsigned long uSize, eGLMemory eMemory);
bool glxInitMemory();
void glx_FreeMemory1(const char* filename);
void glx_FreeMemory0();
u32 glx_GetFreeMemory();
// void 0x8028D514..0x8028D518 | size: 0x4;

class GLXMemoryInfo
{
public:
    GLXMemoryInfo();

    // /* 0x00 */ u32 m_unk_0x00;
    // /* 0x04 */ u32 m_unk_0x04;
    // /* 0x08 */ u32 m_unk_0x08;
    // /* 0x0C */ u32 m_unk_0x0C;
    // /* 0x10 */ u32 m_unk_0x10;
    // /* 0x14 */ u32 m_unk_0x14;
    // /* 0x18 */ u32 m_unk_0x18;

    /* 0x00 */ unsigned long m_uBytes[6];
    /* 0x18 */ unsigned long m_uTexBundle;
}; // total size: 0x1C

#endif // _GLXMEMORY_H_
