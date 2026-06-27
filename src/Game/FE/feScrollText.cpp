#include "Game/FE/feScrollText.h"
#include "Game/FE/feFontResource.h"
#include "NL/gl/glStruct.h"
#include "NL/nlTextEscape.h"
#include "NL/nlBSearch.h"

struct LOCHeader
{
    char Thumbprint[4];
    unsigned long Version;
    unsigned long Language;
    unsigned long StringCount;
    unsigned long Flags;
};

class nlLocalization
{
public:
    struct StringLookup
    {
        unsigned long hash;
        unsigned long StringOffset;

        operator unsigned long() const { return hash; }
    };

    LOCHeader* m_pFile;
    StringLookup* m_LookupTable;
    unsigned short* m_FirstString;
};

extern void* g_pLocalization;
extern const unsigned short LocalizationTableNotFound[];
extern const unsigned short MissingLocString[];

/**
 * Offset/Address/Size: 0x0 | 0x800C89D4 | size: 0x38
 */
void FEScrollText::ApplyNewTextInstancePointer(TLTextInstance* controltext, float boxwidth, float boxheight)
{
    nlVector2 boxSize;
    boxSize.f.y = boxheight;

    this->m_controlText = controltext;
    TLTextInstance* text = this->m_controlText;

    boxSize.f.x = boxwidth;

    text->m_OverloadedAttributes.BoxSize = boxSize;
    text->m_OverloadFlags |= 0x4;
}
static float TEXT_TIME;

/**
 * Offset/Address/Size: 0x38 | 0x800C8A0C | size: 0x190
 */
void FEScrollText::Update(float fDeltaT)
{
    if (m_textFont == NULL)
    {
        SetDisplayMessage(m_message);
        if (m_textFont == NULL)
            return;
    }

    m_controlText->m_bVisible = true;
    m_msgTime += fDeltaT;

    float pixPerSec = (float)m_width / TEXT_TIME;

    feVector3 pos = m_controlText->GetPosition();

    float x = (float)(m_pos + m_width / 2);

    x = x - m_msgTime * pixPerSec;

    m_controlText->SetAssetPosition(x, pos.f.y, pos.f.z);

    if (x + (float)m_messageWidth < m_leftEdge)
    {
        if (m_messageFinishedCB.mTag != EMPTY)
        {
            if (m_messageFinishedCB.mTag == FREE_FUNCTION)
            {
                ((void (*)())m_messageFinishedCB.mFreeFunction)();
            }
            else
            {
                (*((FunctorBase*)m_messageFinishedCB.mFunctor))();
            }
        }
        else
        {
            m_msgTime = 0.0f;
        }
    }
}

static inline const unsigned short* LookupLocTextChar(unsigned long hash)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* lookup = nlBSearch<nlLocalization::StringLookup, unsigned long>(hash, loc->m_LookupTable, loc->m_pFile->StringCount);
    if (lookup != 0)
    {
        return loc->m_FirstString + lookup->StringOffset;
    }
    return MissingLocString;
}

static inline BasicStringData<unsigned short>* BuildScrollString(const unsigned short* str)
{
    BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)Detail::TempStringAllocator::allocate(sizeof(BasicStringData<unsigned short>));
    if (data != 0)
    {
        const unsigned short* src = str;
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;
        const unsigned short* s = src;
        while (*s++ != 0)
        {
            data->mSize++;
        }
        data->mSize++;
        data->mData = (unsigned short*)Detail::TempStringAllocator::allocate((data->mSize + 1) * sizeof(unsigned short));
        data->mCapacity = data->mSize;
        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *src++;
        }
        data->mRefCount = 1;
    }
    return data;
}

static inline BasicStringData<unsigned short>* BuildEmptyScrollStringData()
{
    if (BasicStringData<unsigned short>* data = (BasicStringData<unsigned short>*)Detail::TempStringAllocator::allocate(sizeof(BasicStringData<unsigned short>)))
    {
        const unsigned short* str = (const unsigned short*)L"";
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;
        const unsigned short* s = str;
        while (*s++ != 0)
        {
            data->mSize++;
        }
        data->mSize++;
        data->mData = (unsigned short*)Detail::TempStringAllocator::allocate((data->mSize + 1) * sizeof(unsigned short));
        data->mCapacity = data->mSize;
        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *str++;
        }
        data->mRefCount = 1;
        return data;
    }
    else
    {
        return data;
    }
}

static inline const unsigned short* LookupLocTextChar(nlLocalization* loc, unsigned long hash)
{
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* lookup = nlBSearch<nlLocalization::StringLookup, unsigned long>(hash, loc->m_LookupTable, loc->m_pFile->StringCount);
    if (lookup != 0)
    {
        return loc->m_FirstString + lookup->StringOffset;
    }
    return MissingLocString;
}

/**
 * Offset/Address/Size: 0x1C8 | 0x800C8B9C | size: 0x198
 * TODO: 98.7% match - temporary string cleanup uses data pointer instead of stack-reloaded string data
 */
void FEScrollText::SetDisplayMessage(const char* locMessage)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    unsigned long hash = nlStringLowerHash(locMessage);
    const unsigned short* text = LookupLocTextChar(loc, hash);
    BasicStringData<unsigned short>* data = BuildScrollString(text);

    {
        BasicStringData<unsigned short>* localMsgData = data;
        SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&localMsgData);
    }

    BasicStringData<unsigned short>* msgData = data;
    if (msgData != 0)
    {
        if (--msgData->mRefCount == 0)
        {
            if (msgData != 0)
            {
                if (msgData != 0)
                {
                    delete[] msgData->mData;
                }
                if (msgData != 0)
                {
                    nlFree(msgData);
                }
            }
        }
    }
}

static inline const unsigned short* LookupLocText(unsigned long hash)
{
    nlLocalization* loc = (nlLocalization*)g_pLocalization;
    if (loc->m_LookupTable == 0)
    {
        return LocalizationTableNotFound;
    }
    nlLocalization::StringLookup* lookup = nlBSearch<nlLocalization::StringLookup, unsigned long>(hash, loc->m_LookupTable, loc->m_pFile->StringCount);
    if (lookup != 0)
    {
        return loc->m_FirstString + lookup->StringOffset;
    }
    return MissingLocString;
}

/**
 * Offset/Address/Size: 0x360 | 0x800C8D34 | size: 0x190
 * TODO: 98.65% match - cleanup uses data pointer instead of stack-reloaded string data
 */
void FEScrollText::SetDisplayMessage(unsigned long hash)
{
    const unsigned short* text = LookupLocText(hash);
    BasicStringData<unsigned short>* data = BuildScrollString(text);

    {
        BasicStringData<unsigned short>* localMsgData = data;
        SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&localMsgData);
    }

    BasicStringData<unsigned short>* msgData = data;
    if (msgData != 0)
    {
        if (--msgData->mRefCount == 0)
        {
            if (msgData != 0)
            {
                if (msgData != 0)
                {
                    delete[] msgData->mData;
                }
                if (msgData != 0)
                {
                    nlFree(msgData);
                }
            }
        }
    }
}

static inline void BuildFontCharStringForScroll(FontCharString& fontcharstring, FEScrollText* self)
{
    fontcharstring.m_pString = self->m_textBuffer;
    fontcharstring.m_InternalBuffer = 0;

    const unsigned short* src = self->m_message.c_str();
    nlFont* font = self->m_textFont;
    unsigned short* dest = fontcharstring.m_pString;
    unsigned short escBegin = nlEscapeSequence::ESCAPE_BEGIN;

    while (*src != 0)
    {
        unsigned int ch = *src;
        if (ch == escBegin)
        {
            nlEscapeSequence EscSeq(src);
            int count = (int)((unsigned int)((int)EscSeq.m_pEnd + 1 - (int)src) >> 1);
            if (src < EscSeq.m_pEnd)
            {
                for (int k = count; k > 0; k--)
                    *dest++ = *src++;
            }
        }
        else
        {
            if (ch > 0x7F)
            {
                nlFont::GlyphInfo key;
                key.UnicodeChar = ch;
                nlFont::GlyphInfo* result;
                if (font->m_pExtendedGlyphs != NULL && font->m_ExtendedGlyphCount != 0 && (result = nlBSearch<nlFont::GlyphInfo, nlFont::GlyphInfo>(key, font->m_pExtendedGlyphs, font->m_ExtendedGlyphCount)) != NULL)
                {
                    ch = (unsigned short)((result - font->m_pExtendedGlyphs) + 0x80);
                }
                else
                {
                    ch = 0x30;
                    while (((unsigned short)ch > 0x7F ? font->m_pExtendedGlyphs[(unsigned short)ch - 0x80] : font->m_GlyphLookup[(unsigned short)ch - 0x20]).UnicodeChar == 0xFFFF)
                    {
                        ch++;
                    }
                }
            }
            *dest++ = ch;
            src++;
        }
    }

    *dest = 0;
}

/**
 * Offset/Address/Size: 0x4F0 | 0x800C8EC4 | size: 0x578
 * TODO: 98.0% match - escape-copy guard and temporary glyph key stack slot differ
 */
void FEScrollText::SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>& theMessage)
{
    m_message = theMessage;
    m_msgTime = 0.0f;

    if (m_textFont == NULL)
    {
        m_textFont = ((FEFontResource*)m_controlText->m_component->pChildren)->m_font;
    }

    if (m_textFont == NULL)
    {
        return;
    }

    FontCharString fontcharstring;
    BuildFontCharStringForScroll(fontcharstring, this);
    m_messageWidth = 0;

    unsigned short escBegin = nlEscapeSequence::ESCAPE_BEGIN;
    int i = 0;
    while (i < (m_message.m_data != 0 ? m_message.m_data->mSize - 1 : 0))
    {
        const unsigned short* str = m_message.c_str();
        unsigned short* charPtr = (unsigned short*)(str) + i;
        unsigned short origCh = *charPtr;

        if (origCh == escBegin)
        {
            nlEscapeSequence esc2(charPtr);
            int skipCount = ((int)esc2.m_pEnd - (int)charPtr) / 2;
            i += skipCount - 1;
        }
        else
        {
            unsigned short* fontBuffer = fontcharstring.m_pString;
            unsigned short mappedCh = fontBuffer[i];
            int charWidth;
            if (i != 0)
            {
                int prev = i - 1;
                unsigned short prevChar = fontBuffer[prev];
                charWidth = (int)m_textFont->GetCharWidth(mappedCh, prevChar);
            }
            else
            {
                charWidth = (int)m_textFont->GetCharWidth(mappedCh, 0);
            }
            m_messageWidth += charWidth;
        }
        i++;
    }

    const unsigned short* finalStr = m_message.c_str();
    memcpy(m_textBuffer, finalStr, 0x200);
    m_controlText->SetString(m_textBuffer);

    const feVector3& scale = m_controlText->GetScale();
    m_messageWidth = (int)((float)m_messageWidth * scale.f.x);

    Update(0.0f);
}

inline void FEScrollText::SetMetrics(int pos, int width)
{
    const gl_ScreenInfo* screenInfo = glGetScreenInfo();
    int boxX = pos + screenInfo->ScreenWidth / 2 - width / 2;
    int boxWidth = width;
    if (boxX < 0)
        boxX = 0;
    if (width + boxX >= screenInfo->ScreenWidth)
        boxWidth = screenInfo->ScreenWidth - pos - 1;
    m_controlText->SetScissorBox((u16)boxX, 0, (u16)boxWidth, (u16)screenInfo->ScreenHeight);
}

/**
 * Offset/Address/Size: 0xA68 | 0x800C943C | size: 0x1E8
 */
FEScrollText::FEScrollText(TLTextInstance* controlText, int pos, int width)
    : m_controlText(controlText)
    , m_message(BuildEmptyScrollStringData())
{
    m_messageWidth = 0;

    nlVector2 boxSize;
    boxSize.f.x = 8000.0f;

    m_msgTime = 0.0f;
    m_messageFinishedCB.mTag = EMPTY;
    m_textFont = NULL;

    boxSize.f.y = 100.0f;

    TLTextInstance* text = m_controlText;
    text->m_OverloadedAttributes.BoxSize = boxSize;
    text->m_OverloadFlags |= 0x4;

    m_controlText->m_bVisible = false;

    m_pos = pos;
    m_width = width;

    SetMetrics(pos, width);

    m_leftEdge = (float)(m_pos - m_width / 2);
}

/**
 * Offset/Address/Size: 0x0 | 0x800C9624 | size: 0x90
 */
void feScrollText_stub()
{
    nlFont::GlyphInfo key;
    nlBSearch<nlFont::GlyphInfo, nlFont::GlyphInfo>(key, (nlFont::GlyphInfo*)0, 0);
}
