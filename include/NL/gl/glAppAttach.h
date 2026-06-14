#ifndef _GLAPPATTACH_H_
#define _GLAPPATTACH_H_

#include "NL/gl/glView.h"
#include "NL/gl/glUserData.h"

class PlatTexture;

extern PlatTexture* ResolvedWhiteTexture;
extern PlatTexture* ResolvedBlackTexture;

void glplatAttachPacket(eGLView, unsigned long, const glModelPacket*);
glModelPacket* glplatModifyPacket(eGLView, const glModelPacket*);
void glAppStartup();
void* glAppGetOnePassFresnelUserData();
void* glAppGetNoFogUserData();
void* glAppGetCoPlanarUserData();
char* gld_ViewName(int);

#endif // _GLAPPATTACH_H_
