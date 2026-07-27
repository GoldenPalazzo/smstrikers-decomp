#ifndef _GLUMESHWRITER_H_
#define _GLUMESHWRITER_H_

#include "Game/GL/GLMeshWriter.h"

#include "NL/nlMath.h"

class GLMeshWriter : public GLMeshWriterCore
{
public:
    ~GLMeshWriter() { }

    virtual bool End();
    virtual void Normal(const nlVector3&);
    virtual void Texcoord(const nlVector2&);
    void Texcoord(short, short);

}; // total size: 0x70

#endif // _GLUMESHWRITER_H_
