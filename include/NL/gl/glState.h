#ifndef _GLSTATE_H_
#define _GLSTATE_H_

#include "dolphin/gx/GXEnum.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxTexture.h"
#include "NL/gl/glStateBundle.h"

enum eGLState
{
    GLS_DepthTest = 0,
    GLS_DepthWrite = 1,
    GLS_DepthFunc = 2,
    GLS_AlphaTest = 3,
    GLS_AlphaTestRef = 4,
    GLS_AlphaBlend = 5,
    GLS_Culling = 6,
    GLS_ColourWrite = 7,
    GLS_SolidOffset = 8,
    GLS_FillMode = 9,
    GLS_Num = 10,
};

void glSetDefaultState(bool);
void glUnHandleizeTextureState(unsigned long long);
unsigned long long glHandleizeTextureState();
void glUnHandleizeRasterState(unsigned long);
unsigned long glHandleizeRasterState();
void glSetTextureStateDefaults();
void glSetRasterStateDefaults();
unsigned long glSetTextureState(unsigned long long&, eGLTextureState, unsigned long);
unsigned long glSetTextureState(eGLTextureState, unsigned long);
unsigned long glGetTextureState(unsigned long long, eGLTextureState);
unsigned long glGetTextureState(eGLTextureState);
u32 glSetRasterState(unsigned long& raster, eGLState state, unsigned long value);
u32 glSetRasterState(eGLState state, unsigned long value);
u32 glGetRasterState(unsigned long raster, eGLState state);
u32 glGetRasterState(eGLState state);
u32 glGetTexture(const char* textureName);
u32 glGetProgram(const char* programName);
u32 glSetCurrentMatrix(unsigned long);
u64 glSetCurrentTextureState(unsigned long long state);
u64 glGetCurrentTextureState();
u32 glSetCurrentRasterState(unsigned long state = 0);
u32 glGetCurrentRasterState();
u32 glSetCurrentProgram(unsigned long);
u32 glSetCurrentTexture(unsigned long, eGLTextureType);
void glStateRestore(const glStateBundle&);
void glStateSave(glStateBundle& state);
glStateBundle* gl_GetCurrentStateBundle();
void gl_StateStartup();

#endif // _GLSTATE_H_
