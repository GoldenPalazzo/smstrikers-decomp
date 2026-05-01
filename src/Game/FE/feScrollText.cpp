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

static inline BasicStringInternal* BuildScrollString(const unsigned short* text)
{
    BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = text;
        while (*ptr++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int j;
        int i = 0;
        j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)(data->mData + j) = *text;
            i++;
            text++;
            j += 2;
        }

        data->mRefCount = 1;
    }
    return data;
}

/**
 * Offset/Address/Size: 0x1C8 | 0x800C8B9C | size: 0x198
 * TODO: 95.1% match - MWCC keeps this in r30 and loc/data in r29 (target is this->r29, loc/data->r30)
 */
void FEScrollText::SetDisplayMessage(const char* id)
{
    unsigned long hash = nlStringLowerHash(id);
    const unsigned short* text = LookupLocTextChar(hash);
    BasicStringInternal* data = BuildScrollString(text);

    BasicStringInternal* msgData = data;
    SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&msgData);

    data = msgData;
    if (data != 0)
    {
        if (--data->mRefCount == 0)
        {
            if (data != 0)
            {
                if (data != 0)
                {
                    delete[] data->mData;
                }
                if (data != 0)
                {
                    nlFree(data);
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
 * TODO: 95.4% match - MWCC register allocation: this->r31/text->r29 instead of this->r29/text->r31, li r5,0 instead of mr r5,r4
 */
void FEScrollText::SetDisplayMessage(unsigned long hash)
{
    const unsigned short* text = LookupLocText(hash);

    BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        const unsigned short* ptr = text;
        while (*ptr++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc((data->mSize + 1) * 2, 8, true);
        data->mCapacity = data->mSize;

        int i = 0;
        int j = i;
        while (i < data->mSize)
        {
            *(unsigned short*)(data->mData + j) = *text;
            i++;
            text++;
            j += 2;
        }

        data->mRefCount = 1;
    }

    BasicStringInternal* msgData = data;
    SetDisplayMessage(*(const BasicString<unsigned short, Detail::TempStringAllocator>*)&msgData);

    data = msgData;
    if (data != 0)
    {
        if (--data->mRefCount == 0)
        {
            if (data != 0)
            {
                if (data != 0)
                {
                    delete[] data->mData;
                }
                if (data != 0)
                {
                    nlFree(data);
                }
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x4F0 | 0x800C8EC4 | size: 0x578
 */
void FEScrollText::SetDisplayMessage(const BasicString<unsigned short, Detail::TempStringAllocator>&)
{
    FORCE_DONT_INLINE;
}

static const unsigned short sEmptyString[] = { 0 };

inline void FEScrollText::SetMetrics(int pos, int width)
{
    const gl_ScreenInfo* screenInfo = glGetScreenInfo();
    int boxWidth = width;
    int boxX = pos + screenInfo->ScreenWidth / 2 - width / 2;
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
    , m_message(sEmptyString)
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
