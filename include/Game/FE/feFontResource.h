#ifndef _FEFONTRESOURCE_H_
#define _FEFONTRESOURCE_H_

#include "NL/nlFont.h"

class FEFontResource
{
public:
    void SetFontReference(nlFont* font);

    /* 0x00 */ char pad0[0x14];
    /* 0x14 */ nlFont* m_font;
};

#endif // _FEFONTRESOURCE_H_
