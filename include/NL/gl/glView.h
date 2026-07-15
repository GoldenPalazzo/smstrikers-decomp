#ifndef _GLVIEW_H_
#define _GLVIEW_H_

#include "NL/nlMath.h"
#include "NL/gl/gl.h"

#include "NL/gl/glModel.h"

class GLRenderList;

typedef void (*glViewIterateCallback)(eGLView, unsigned long);
typedef void (*glViewPacketCallback)(eGLView, unsigned long, const glModelPacket*);

struct glView
{
    /* 0x00 */ eGLViewSort sortMode;
    /* 0x04 */ u32 vpX;
    /* 0x08 */ u32 vpY;
    /* 0x0C */ u32 vpWidth;
    /* 0x10 */ u32 vpHeight;
    /* 0x14 */ nlMatrix4* viewMatrix;
    /* 0x18 */ nlMatrix4* projMatrix;
    /* 0x1C */ eGLTarget target;
    /* 0x20 */ nlMatrix4 viewm;
    /* 0x60 */ nlMatrix4 projm;
    /* 0xA0 */ nlMatrix4 concatm; // projm * viewm, rebuilt when bConcatDirty
    /* 0xE0 */ bool bConcatDirty;
    /* 0xE4 */ eGLFilter filter;
    /* 0xE8 */ eGLTarget filterSource;
    /* 0xEC */ bool bClearColour;
    /* 0xED */ bool bClearDepth;
    /* 0xEE */ bool pad_0xEE;
    /* 0xEF */ bool pad_0xEF;
    /* 0xF0 */ GLRenderList* renderList;
    /* 0xF4 */ glViewIterateCallback preViewCallback;
    /* 0xF8 */ glViewIterateCallback postViewCallback;
};

bool glViewSetEnable(eGLView, bool);
bool glViewGetEnable(eGLView);
eGLTarget glViewSetFilterSource(eGLView, eGLTarget);
eGLFilter glViewGetFilter(eGLView);
eGLFilter glViewSetFilter(eGLView, eGLFilter);
eGLTarget glViewSetTarget(eGLView, eGLTarget);
void glViewProjectPoint(eGLView, const nlVector3&, nlVector3&);
void glViewGetProjectionMatrix(eGLView, nlMatrix4&);
void glViewGetViewMatrix(eGLView, nlMatrix4&);
nlMatrix4* glViewSetProjectionMatrix(eGLView, unsigned long);
nlMatrix4* glViewGetProjectionMatrix(eGLView);
nlMatrix4* glViewSetViewMatrix(eGLView, unsigned long);
nlMatrix4* glViewGetViewMatrix(eGLView);
eGLViewSort glViewSetSortMode(eGLView, eGLViewSort);
bool glViewSetDepthClear(eGLView, bool);
bool glViewGetDepthClear(eGLView);
void gl_ViewStartup();
void gl_ViewIterate(eGLView, glViewPacketCallback);
void glViewCompact();
void gl_ViewReset();
void glViewAttachPacket(eGLView, const glModelPacket*);
void glViewAttachModel(eGLView, const glModel*);
void glViewAttachModel(eGLView, unsigned long, const glModel*);
GLRenderList* gl_ViewGetRenderList(eGLView);

#endif // _GLVIEW_H_
