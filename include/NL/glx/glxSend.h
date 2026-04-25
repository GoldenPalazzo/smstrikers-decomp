#ifndef _GLXSEND_H_
#define _GLXSEND_H_

#include "dolphin/gx/GXEnum.h"
#include "NL/gl/glView.h"

class GLLightUserData;

void glx_SendFrame_cb(eGLView, unsigned long, const glModelPacket*);
void glx_DrawPacket(const glModelPacket*);
void glx_SwitchUserData(const glModelPacket*);
void glud_Skin(void*, const glModelPacket*);
void glud_Specular(void*);
void glud_Light(void*);
void glx_LoadLight(GLLightUserData*, _GXLightID);
void glx_SwitchStreams(const glModelPacket*);
void glx_SwitchRaster(const glModelPacket*);
void glx_SwitchTextureState(const glModelPacket*);
unsigned long glx_SwitchTexConfig(const glModelPacket*);
void glx_SendEnd();
void glx_SendReset();
void GetConstants();

#endif // _GLXSEND_H_
