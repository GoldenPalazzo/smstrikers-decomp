#ifndef _FEIMAGE_H_
#define _FEIMAGE_H_

#include "Game/FE/feLibObject.h"

class FETextureResource;

class FEImage : public FELibObject
{
public:
    /* 0x68 */ FETextureResource* m_pFeTextureResource;
}; // total size: 0x6C

#endif // _FEIMAGE_H_
