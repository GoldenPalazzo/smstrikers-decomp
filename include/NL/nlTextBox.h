#ifndef _NLTEXTBOX_H_
#define _NLTEXTBOX_H_

#include "NL/nlMath.h"
#include "NL/nlFont.h"
// #include "NL/nlColour.h"
#include "NL/gl/gl.h"
#include "NL/gl/glMatrix.h"

struct nlColour;

struct Row
{
    /* 0x0 */ unsigned short XOffset;
    /* 0x2 */ unsigned short FirstChar;
}; // total size: 0x4

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

    static void DrawString(const nlTextBox::StringDrawInfo& DrawInfo, const nlVector2& DrawAt, const nlColour& Color, eGLView View);
    static void ProcessString(const FontCharString* pString, const nlFont* pFont, const nlVector2& BoxSize, unsigned long DrawOptions, const nlMatrix4* pMatrix, nlTextBox::StringDrawInfo& DrawInfo);
};

#endif // _NLTEXTBOX_H_
