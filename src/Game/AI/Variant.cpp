#include "Game/AI/Variant.h"
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

template NLString LexicalCast<NLString, char>(const char&);

// /**
//  * Offset/Address/Size: 0x2BC0 | 0x80069E18 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, float>(const BasicString<char, Detail::TempStringAllocator>&, const float&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1E4C | 0x800690A4 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<unsigned long>(const unsigned long&)
// {
// }

// /**
//  * Offset/Address/Size: 0x1D38 | 0x80068F90 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, unsigned long>(const BasicString<char, Detail::TempStringAllocator>&, const unsigned long&)
// {
// }

// /**
//  * Offset/Address/Size: 0xFC4 | 0x8006821C | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<char>(const char&)
// {
// }

// /**
//  * Offset/Address/Size: 0xEB0 | 0x80068108 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, char>(const BasicString<char, Detail::TempStringAllocator>&, const char&)
// {
// }

// /**
//  * Offset/Address/Size: 0x13C | 0x80067394 | size: 0xD74
//  */
// void FormatImpl<BasicString<char, Detail::TempStringAllocator>>::operator%<float>(const float&)
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80067258 | size: 0x13C
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, float, float, float>(const BasicString<char, Detail::TempStringAllocator>&, const float&, const float&, const float&)
// {
// }

/**
 * Offset/Address/Size: 0xC4 | 0x8006632C | size: 0xF2C
 */
void Variant::ToString() const
{
}

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
