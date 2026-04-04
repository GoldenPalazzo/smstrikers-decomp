#ifndef _TLTEXTINSTANCE_H_
#define _TLTEXTINSTANCE_H_

#include "types.h"
#include "NL/nlColour.h"
#include "NL/gl/gl.h"
#include "NL/nlFont.h"
#include "NL/nlMath.h"

#include "Game/FE/tlInstance.h"

struct FETextLibObjectAttributes
{
    /* 0x0 */ nlColour EffectColour;
    /* 0x4 */ nlVector2 BoxSize;
}; // total size: 0xC

struct Row
{
    /* 0x00 */ unsigned short XOffset;
    /* 0x02 */ unsigned short FirstChar;
}; // total size: 0x4

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

class FontCharString
{
public:
    FontCharString() { }
    ~FontCharString()
    {
        if (m_InternalBuffer != 0)
        {
            delete[] m_pString;
        }
    }
    template <typename T>
    FontCharString(const T*, const nlFont*, T*);

    /* 0x0 */ unsigned short* m_pString;
    /* 0x4 */ unsigned char m_InternalBuffer;
}; // total size: 0x8

class TLTextInstance : public TLInstance
{
public:
    void SetScissorBox(u16 left, u16 top, u16 width, u16 height);
    void SetString(const unsigned short* utf16);
    void SetStringId(const char* name);
    const unsigned short* GetString() const;
    void SetMatrix(nlMatrix4*);
    void Render(eGLView, const nlColour&) const;

    /* 0x80 */ unsigned long m_LocStrId;
    /* 0x84 */ FETextLibObjectAttributes m_OverloadedAttributes;
    /* 0x90 */ unsigned long m_OverloadFlags;
    /* 0x94 */ StringDrawInfo m_DrawInfo;
    /* 0xEC */ FontCharString* m_pFontString;
    /* 0xF0 */ unsigned long m_DrawOptions;
    /* 0xF4 */ const unsigned short* m_wcUserString;
    /* 0xF8 */ bool m_UseScissorRect;
    /* 0xFA */ ScissorBox m_ScissorRect;

}; // total size: 0x104

#endif // _TLTEXTINSTANCE_H_
