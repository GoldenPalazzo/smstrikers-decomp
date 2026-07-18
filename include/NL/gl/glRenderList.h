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
    s32 AttachModel(const glModel*, unsigned long);
    void Iterate(eGLView, void (*)(eGLView, unsigned long, const glModelPacket*));
    bool IsEmpty() const;
    void Compact();
    void Clear();

    GLRenderList();

    /* 0x00 */ u32 m_unk_0x00;
    /* 0x04 */ eGLViewSort m_unk_0x04;
    /* 0x08 */ unsigned long uDepthInsertNumber;
    /* 0x0C */ GLTexturePacketTree* texPacketTree[7];
    /* 0x28 */ GLDepthPacketTree* depthPacketTree;
    /* 0x2C */ GLPacketList* packetList;
};

#endif // _GLRENDERLIST_H_
