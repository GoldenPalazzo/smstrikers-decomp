#ifndef _NLTEXTBOX_H_
#define _NLTEXTBOX_H_

#include "NL/nlMath.h"
#include "NL/nlFont.h"
// #include "NL/nlColour.h"
#include "NL/gl/gl.h"
#include "NL/gl/glMatrix.h"

struct nlColour;
class FontCharString;

struct Row
{
    /* 0x0 */ unsigned short XOffset;
    /* 0x2 */ unsigned short FirstChar;
}; // total size: 0x4

struct FontCharString
{
    /* 0x0 */ unsigned short* m_pString;
    /* 0x4 */ unsigned char m_InternalBuffer;
}; // total size: 0x8

class nlTextBox
{
public:
    struct StringDrawInfo
    {
        /* 0x00 */ const nlFont* pFont;
        /* 0x04 */ const unsigned short* String;
        /* 0x08 */ const nlMatrix4* pMatrix;
        /* 0x0C */ unsigned long DrawOptions;
        /* 0x10 */ unsigned short RowCount;
        /* 0x12 */ signed short YOffset;
        /* 0x14 */ Row Rows[17];
    }; // total size: 0x58

    static void DrawString(const nlTextBox::StringDrawInfo&, const nlVector2&, const nlColour&, eGLView view);
    static void ProcessString(const FontCharString*, const nlFont*, const nlVector2&, unsigned long, const nlMatrix4*, nlTextBox::StringDrawInfo&);
};

#endif // _NLTEXTBOX_H_
