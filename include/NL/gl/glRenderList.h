#ifndef _GLRENDERLIST_H_
#define _GLRENDERLIST_H_

#include "NL/gl/glView.h"

class glModel;
class GLTexturePacketTree;
class GLDepthPacketTree;
class GLPacketList;

class GLRenderList
{
public:
    bool AttachModel(const glModel*, unsigned long);
    void AttachPacket(unsigned long, const glModelPacket*);
    void Iterate(eGLView, void (*)(eGLView, unsigned long, const glModelPacket*));
    bool IsEmpty() const;
    void Compact();
    void Clear();

    GLRenderList();
    ~GLRenderList();

    /* 0x00 */ eGLView view;
    /* 0x04 */ eGLViewSort sortMode;

private:
    /* 0x08 */ unsigned long uDepthInsertNumber;
    /* 0x0C */ GLTexturePacketTree* texPacketTree[7];
    /* 0x28 */ GLDepthPacketTree* depthPacketTree;
    /* 0x2C */ GLPacketList* packetList;
};

#endif // _GLRENDERLIST_H_
