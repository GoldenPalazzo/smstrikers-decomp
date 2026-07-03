#define BASICSTRING_INLINE_ERASE
#include "Game/AI/Variant.h"
#include "NL/nlFormat.h"
#include "NL/nlLexicalCast.h"
#include "PowerPC_EABI_Support/Runtime/runtime.h"

static Variant gvNotSet;

// /**
//  * Offset/Address/Size: 0x268 | 0x8006A194 | size: 0x3C
//  */
// void 0x8028D2AC..0x8028D2B0 | size: 0x4
// {
// }

// /**
//  * Offset/Address/Size: 0x168 | 0x8006A094 | size: 0x100
//  */
// void Detail::LexicalCastImpl<BasicString<char, Detail::TempStringAllocator>, unsigned long>::Do(unsigned long)
// {
// }

typedef BasicString<char, Detail::TempStringAllocator> NLString;

template NLString LexicalCast<NLString, unsigned long>(const unsigned long&);

// /**
//  * Offset/Address/Size: 0x34 | 0x80069F60 | size: 0x104
//  */
// void Detail::LexicalCastImpl<BasicString<char, Detail::TempStringAllocator>, char>::Do(char)
// {
// }

typedef NLString (*Format1FFn)(const NLString&, const float&);
typedef NLString (*Format3FFn)(const NLString&, const float&, const float&, const float&);

void Variant_stub()
{
    NLString format;
    float value1 = 0.0f;
    float value2 = 0.0f;
    float value3 = 0.0f;
    volatile Format1FFn fn1 = Format<NLString, float>;
    fn1(format, value1);
    volatile Format3FFn fn3 = Format<NLString, float, float, float>;
    fn3(format, value1, value2, value3);
}

/**
 * Offset/Address/Size: 0x1E4C | 0x800690A4 | size: 0xD74
 * TODO: 99.13% match - remaining copy-on-write temp register swaps around insert.
 */
template <>
template <>
FormatImpl<BasicString<char, Detail::TempStringAllocator> >&
    FormatImpl<BasicString<char, Detail::TempStringAllocator> >::operator% <unsigned long>(const unsigned long& t)
{
    BasicString<char, Detail::TempStringAllocator> insert = LexicalCast<BasicString<char, Detail::TempStringAllocator>, unsigned long>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != '{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* marker = &mString[i];
        if (mCurrentPos != marker[1] - '0')
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* markerEnd = &mString[i];
        if (markerEnd[2] != '}')
            continue;

        char* eraseBegin;
        char* eraseEnd;
        mString[0];
        eraseEnd = (mString.m_data ? mString.m_data->mData : (char*)0) + i + 3;
        mString[0];
        eraseBegin = (mString.m_data ? mString.m_data->mData : (char*)0) + i;
        mString[0];
        BasicStringData<char>* eraseData = mString.m_data;
        int eraseSize = eraseEnd - eraseBegin;
        const char* eraseIt = eraseEnd;
        char* eraseAt = eraseData->mData + (eraseBegin - eraseData->mData);
        while (eraseIt != eraseData->mData + eraseData->mSize)
        {
            *eraseAt = *eraseIt;
            eraseIt++;
            eraseAt++;
        }
        eraseData->mSize -= eraseSize;
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        insert[0];
        char* insertBegin = insert.m_data ? insert.m_data->mData : 0;
        insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? insert.m_data->mData + insert.m_data->mSize - 1 : (char*)0);
    }

    mCurrentPos++;
    return *this;
}

// /**
//  * Offset/Address/Size: 0x1D38 | 0x80068F90 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, unsigned long>(const BasicString<char, Detail::TempStringAllocator>&, const unsigned long&)
// {
// }

/**
 * Offset/Address/Size: 0xFC4 | 0x8006821C | size: 0xD74
 * TODO: 98.96% match - remaining erase/insert register swaps and insert pointer branch offsets.
 */
template <>
template <>
FormatImpl<BasicString<char, Detail::TempStringAllocator> >&
    FormatImpl<BasicString<char, Detail::TempStringAllocator> >::operator% <char>(const char& t)
{
    BasicString<char, Detail::TempStringAllocator> insert = LexicalCast<BasicString<char, Detail::TempStringAllocator>, char>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != '{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* marker = &mString[i];
        if (mCurrentPos != marker[1] - '0')
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* markerEnd = &mString[i];
        if (markerEnd[2] != '}')
            continue;

        char* eraseBegin;
        char* eraseEnd;
        mString[0];
        eraseEnd = (mString.m_data ? mString.m_data->mData : (char*)0) + i + 3;
        mString[0];
        eraseBegin = (mString.m_data ? mString.m_data->mData : (char*)0) + i;
        mString.erase(eraseBegin, eraseEnd);
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        insert[0];
        char* insertBegin = insert.m_data ? insert.m_data->mData : 0;
        insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? insert.m_data->mData + insert.m_data->mSize - 1 : (char*)0);
    }

    mCurrentPos++;
    return *this;
}

// /**
//  * Offset/Address/Size: 0xEB0 | 0x80068108 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, char>(const BasicString<char, Detail::TempStringAllocator>&, const char&)
// {
// }

/**
 * Offset/Address/Size: 0x13C | 0x80067394 | size: 0xD74
 * TODO: 99.59% match - copy-on-write temp register swap (r26 vs r27) and marker add operand order.
 */
typedef FormatImpl<NLString> NLFormatImpl;

static inline void EraseRange(NLString& s, const char* begin, const char* end)
{
    s[0];
    BasicStringData<char>* data = s.m_data;
    int size = end - begin;
    int offset = begin - data->mData;
    char* at = data->mData + offset;
    while (end != data->mData + data->mSize)
    {
        *at = *end;
        end++;
        at++;
    }
    data->mSize -= size;
}

template <>
NLFormatImpl& NLFormatImpl::operator% <float>(const float& t)
{
    NLString insert = LexicalCast<NLString, float>(t);

    for (int i = 0; i < (mString.m_data ? mString.m_data->mSize - 1 : 0); i++)
    {
        if (mString[i] != (char)'{')
            continue;

        if (i + 1 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* marker = &mString[i];
        if (mCurrentPos != marker[1] - '0')
            continue;

        if (i + 2 >= (mString.m_data ? mString.m_data->mSize - 1 : 0))
            continue;

        char* markerEnd = &mString[i];
        if (markerEnd[2] != (char)'}')
            continue;

        mString[0];
        EraseRange(mString, ((void)mString[0], (mString.m_data ? mString.m_data->mData : (char*)0) + i), (mString.m_data ? mString.m_data->mData : (char*)0) + i + 3);
        mString[i];
        char* mStringData = mString.m_data ? mString.m_data->mData : 0;
        char* insertBegin = ((void)insert[0], insert.m_data ? insert.m_data->mData : 0);
        insert[(int)(insert.m_data ? insert.m_data->mSize - 1 : 0)];
        mString.insert(mStringData + i, insertBegin, insert.m_data ? insert.m_data->mData + insert.m_data->mSize - 1 : (char*)0);
    }

    mCurrentPos++;
    return *this;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80067258 | size: 0x13C
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, float, float, float>(const BasicString<char, Detail::TempStringAllocator>&, const float&, const float&, const float&)
// {
// }

/**
 * Offset/Address/Size: 0xC4 | 0x8006632C | size: 0xF2C
 */
#pragma optimization_level 2
NLString Variant::ToString() const
{
    NLString toString;

    if (mType != FT_UNSPECIFIED)
    {
        NLString dataString = "???";

        switch (mType)
        {
        case FT_BOOL:
            dataString = mData.b ? "TRUE" : "FALSE";
            break;

        case FT_CHAR:
        {
            NLString format = "{0}";
            dataString = Format(format, mData.c);
            break;
        }

        case FT_SHORT:
        {
            NLString format = "{0}";
            int value = mData.s;
            dataString = Format(format, value);
            break;
        }

        case FT_INT:
        {
            dataString = Format(NLString("{0}"), mData.i);
            break;
        }

        case FT_U32:
        {
            dataString = Format(NLString("{0}"), mData.u);
            break;
        }

        case FT_FLOAT:
        {
            dataString = Format(NLString("{0}"), mData.f);
            break;
        }

        case FT_VECTOR:
        {
            dataString = Format(NLString("({0},{1},{2})"), mData.vector.f.x, mData.vector.f.y, mData.vector.f.z);
            break;
        }

        case FT_PLAYER:
            if (mData.pPlayer != 0)
            {
                dataString = Format(NLString("UPID={0}"), mData.pPlayer->GetUniqueID(-1));
            }
            break;

        case FT_TEAM:
            if (mData.pTeam != 0)
            {
                dataString = Format(NLString("Team={0}"), mData.pTeam->m_nSide == 0 ? "Home" : "Away");
            }
            break;
        }

        {
            BasicStringData<char>* data = dataString.m_data;
            if (data != 0)
            {
                data->mRefCount++;
            }
            else
            {
                data = 0;
            }
            toString = NLString(data);
        }
    }
    else
    {
        toString = "N/A";
    }

    {
        BasicStringData<char>* data = toString.m_data;
        if (data != 0)
        {
            data->mRefCount++;
        }
        else
        {
            data = 0;
        }
        return NLString(data);
    }
}
#pragma optimization_level 4

/**
 * Offset/Address/Size: 0x0 | 0x80066268 | size: 0xC4
 */
unsigned long Variant::GetHash() const
{
    unsigned long hash = 0;

    switch (mType)
    {
    case FT_BOOL:
        return mData.b;

    case FT_CHAR:
        hash = mData.c;
        hash = (s8)hash;
        return hash;

    case FT_SHORT:
        return (unsigned long)mData.s;

    case FT_INT:
        return (unsigned long)mData.i;

    case FT_U32:
        return mData.u;

    case FT_FLOAT:
        return __cvt_fp2unsigned((f64)mData.f);

    case FT_PLAYER:
        return (unsigned long)mData.pPlayer;

    case FT_TEAM:
        return (unsigned long)mData.pTeam;

    case FT_VECTOR:
    {
        unsigned long hash1 = __cvt_fp2unsigned((f64)mData.vector.f.z);
        unsigned long hash2 = __cvt_fp2unsigned((f64)mData.vector.f.y);
        hash2 ^= hash1;
        unsigned long hash3 = __cvt_fp2unsigned((f64)mData.vector.f.x);
        hash = hash3 | hash2;
        break;
    }
    }

    return hash;
}
