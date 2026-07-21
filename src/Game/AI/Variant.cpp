#include "Game/AI/Variant.h"
#include "NL/nlFormat.h"
#include "NL/nlLexicalCast.h"
#include "PowerPC_EABI_Support/Runtime/runtime.h"

Variant gvNotSet;

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

// /**
//  * Offset/Address/Size: 0x34 | 0x80069F60 | size: 0x104
//  */
// void Detail::LexicalCastImpl<BasicString<char, Detail::TempStringAllocator>, char>::Do(char)
// {
// }

// /**
//  * Offset/Address/Size: 0x1D38 | 0x80068F90 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, unsigned long>(const BasicString<char, Detail::TempStringAllocator>&, const unsigned long&)
// {
// }

// /**
//  * Offset/Address/Size: 0xEB0 | 0x80068108 | size: 0x114
//  */
// void Format<BasicString<char, Detail::TempStringAllocator>, char>(const BasicString<char, Detail::TempStringAllocator>&, const char&)
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
            dataString = Format(NLString("{0}"), mData.c);
            break;
        }

        case FT_SHORT:
        {
            dataString = Format(NLString("{0}"), (int)mData.s);
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

        toString = dataString;
    }
    else
    {
        toString = "N/A";
    }

    return toString;
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
