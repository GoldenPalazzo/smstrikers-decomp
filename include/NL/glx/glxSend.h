#ifndef _GLXSEND_H_
#define _GLXSEND_H_

#include "dolphin/gx/GXEnum.h"
#include "NL/gl/glView.h"

class GLLightUserData;

void glx_SendFrame_cb(eGLView, unsigned long, const glModelPacket*);
void glx_SendEnd();
void glx_SendReset();

#endif // _GLXSEND_H_
