#include "NL/nlConfig.h"
#include "Game/Sys/debug.h"
#include "NL/nlFileGC.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/ctype.h"

// static Config* sGlobal = nullptr;

/**
 * Offset/Address/Size: 0x0 | 0x801D2C64 | size: 0x13EC
 */
void Config::Parse(const char*, Config::Parser&)
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x13EC | 0x801D4050 | size: 0x21C
 * TODO: 91.0% match - unresolved debug format literal (@2685) and remaining
 * r29/r30/r27 register-allocation differences in buffer sizing and copy loops
 */
BasicString<char, Detail::TempStringAllocator> Config::LoadFileAsString(const char* filename)
{
    tDebugPrintManager::Print(DC_CONFIG_SYSTEM, "Loading config file: %s\n", filename);
    nlFlushFileCash();

    unsigned long fileSize = 0;
    char* src = (char*)nlLoadEntireFile(filename, &fileSize, 0x20, AllocateEnd);
    if (src != 0)
    {
        char* end = src + fileSize;
        BasicStringInternal* data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
        if (data != 0)
        {
            s32 length = end - src;
            s32 size = length + 1;
            data->mData = (char*)nlMalloc(size, 8, true);
            data->mSize = size;
            data->mCapacity = size;

            for (s32 i = 0; i < length + 1; i++)
            {
                data->mData[i] = 0;
            }

            data->mRefCount = 1;
            for (s32 i = 0; i < data->mSize - 1; i++)
            {
                data->mData[i] = src[i];
            }
        }

        BasicString<char, Detail::TempStringAllocator> s(data);
        nlFree(src);
        nlFlushFileCash();
        return s;
    }

    return BasicString<char, Detail::TempStringAllocator>();
}

/**
 * Offset/Address/Size: 0x1608 | 0x801D426C | size: 0x54
 */
void Config::Set(const char* key, const BasicString<char, Detail::TempStringAllocator>& value)
{
    Set(key, value.c_str());
}

static bool IsIntValue(const char* str, int& out)
{
    const char* s = str;
    while (*s != 0)
    {
        if (isdigit(*s) || *s == '-')
            s++;
        else
            return false;
    }
    out = LexicalCast<int, const char*>(str);
    return true;
}

static bool IsFloatValue(const char* str, float& out)
{
    bool seenPeriod = false;
    const char* s = str;
    while (*s != 0)
    {
        char c = *s;
        if (c == '.' || c == ',')
            seenPeriod = true;
        else if (isdigit(c) || c == '-')
            ;
        else
            return false;
        s++;
    }
    out = LexicalCast<float, const char*>(str);
    return seenPeriod;
}

/**
 * Offset/Address/Size: 0x165C | 0x801D42C0 | size: 0x56C
 * TODO: 87.5% match - r28/r30/r31/r28/r27/r29/r29/r31 register allocation swap,
 * nlToUpper<Uc> vs nlToUpper<c> template instantiation (u8 cast needed for byte load pattern),
 * __ctype_map not hoisted to prologue, IsInteger bne+beq vs bne+bne+b branch pattern,
 * copy loop bge vs blt/b branch pattern
 */
void Config::Set(const char* tag, const char* value)
{
    bool b = false;
    float f = 0.0f;
    int i = 0;

    if (IsIntValue(value, i))
    {
        const char* p = tag;
        u32 hash = 0x1505;
        while (*p != 0)
        {
            s8 c = (s8)nlToUpper((u8)*p++);
            hash = hash + (hash << 5) + c;
        }
        u32 idx = hash & 0x3FF;

        TagValuePair* tvp;
        while (true)
        {
            u32 offset = idx * 12;
            if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
            {
                tvp = (TagValuePair*)((char*)mTvpHash + offset);
                break;
            }
            idx++;
            idx &= 0x3FF;
        }

        tvp->type = _INT;
        tvp->value.i = i;

        if (tvp->tag == NULL)
        {
            char* dest = mStringEnd;
            while (*tag != 0)
            {
                if (mStringEnd - mStringMemory < 0x27FF)
                {
                    *mStringEnd = nlToUpper(*tag);
                    tag++;
                    mStringEnd++;
                }
                else
                {
                    break;
                }
            }
            *mStringEnd = 0;
            mStringEnd++;
            tvp->tag = dest;
        }
    }
    else if (IsFloatValue(value, f))
    {
        const char* p = tag;
        u32 hash = 0x1505;
        while (*p != 0)
        {
            s8 c = (s8)nlToUpper((u8)*p++);
            hash = hash + (hash << 5) + c;
        }
        u32 idx = hash & 0x3FF;

        TagValuePair* tvp;
        while (true)
        {
            u32 offset = idx * 12;
            if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
            {
                tvp = (TagValuePair*)((char*)mTvpHash + offset);
                break;
            }
            idx++;
            idx &= 0x3FF;
        }

        tvp->type = _FLOAT;
        tvp->value.f = f;

        if (tvp->tag == NULL)
        {
            char* dest = mStringEnd;
            while (*tag != 0)
            {
                if (mStringEnd - mStringMemory < 0x27FF)
                {
                    *mStringEnd = nlToUpper(*tag);
                    tag++;
                    mStringEnd++;
                }
                else
                {
                    break;
                }
            }
            *mStringEnd = 0;
            mStringEnd++;
            tvp->tag = dest;
        }
    }
    else if (IsBool(value, b))
    {
        const char* p = tag;
        u32 hash = 0x1505;
        while (*p != 0)
        {
            s8 c = (s8)nlToUpper((u8)*p++);
            hash = hash + (hash << 5) + c;
        }
        u32 idx = hash & 0x3FF;

        TagValuePair* tvp;
        while (true)
        {
            u32 offset = idx * 12;
            if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
            {
                tvp = (TagValuePair*)((char*)mTvpHash + offset);
                break;
            }
            idx++;
            idx &= 0x3FF;
        }

        tvp->type = _BOOL;
        tvp->value.b = b;

        if (tvp->tag == NULL)
        {
            char* dest = mStringEnd;
            while (*tag != 0)
            {
                if (mStringEnd - mStringMemory < 0x27FF)
                {
                    *mStringEnd = nlToUpper(*tag);
                    tag++;
                    mStringEnd++;
                }
                else
                {
                    break;
                }
            }
            *mStringEnd = 0;
            mStringEnd++;
            tvp->tag = dest;
        }
    }
    else
    {
        const char* p = tag;
        u32 hash = 0x1505;
        while (*p != 0)
        {
            s8 c = (s8)nlToUpper((u8)*p++);
            hash = hash + (hash << 5) + c;
        }
        u32 idx = hash & 0x3FF;

        TagValuePair* tvp;
        while (true)
        {
            u32 offset = idx * 12;
            if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
            {
                tvp = (TagValuePair*)((char*)mTvpHash + offset);
                break;
            }
            idx++;
            idx &= 0x3FF;
        }

        tvp->type = _STRING;

        char* valDest = mStringEnd;
        while (*value != 0)
        {
            if (mStringEnd - mStringMemory < 0x27FF)
            {
                *mStringEnd = *value;
                value++;
                mStringEnd++;
            }
            else
            {
                break;
            }
        }
        *mStringEnd = 0;
        mStringEnd++;
        tvp->value.s = valDest;

        if (tvp->tag == NULL)
        {
            char* dest = mStringEnd;
            while (*tag != 0)
            {
                if (mStringEnd - mStringMemory < 0x27FF)
                {
                    *mStringEnd = nlToUpper(*tag);
                    tag++;
                    mStringEnd++;
                }
                else
                {
                    break;
                }
            }
            *mStringEnd = 0;
            mStringEnd++;
            tvp->tag = dest;
        }
    }
}

/**
 * Offset/Address/Size: 0x1BC8 | 0x801D482C | size: 0x12C
 * TODO: 94.3% match - r28/r30 register swap for hash/idx in probe setup,
 * r29/r30 register swap for idx/offset and r28/r29 swap for dest/src in copy loop,
 * nlToUpper<Uc> vs nlToUpper<c> template instantiation (i diff),
 * bge vs blt/b branch pattern in copy loop
 */
void Config::Set(const char* tag, float value)
{
    TagValuePair* tvp;
    u32 hash = 0x1505;
    const char* p = tag;
    while (*p != 0)
    {
        s8 c = (s8)nlToUpper((u8)*p++);
        hash += (hash << 5) + c;
    }
    u32 idx = hash & 0x3FF;

    while (true)
    {
        u32 offset = idx * 12;
        if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
        {
            tvp = (TagValuePair*)((u8*)mTvpHash + offset);
            break;
        }
        idx++;
        idx &= 0x3FF;
    }

    tvp->type = _FLOAT;
    tvp->value.f = value;

    if (tvp->tag == NULL)
    {
        const char* src = tag;
        char* dest = mStringEnd;
        while (*src != 0)
        {
            if (mStringEnd - mStringMemory >= 0x27FF)
            {
                break;
            }
            *mStringEnd = nlToUpper((u8)*src);
            src++;
            mStringEnd++;
        }
        *mStringEnd = 0;
        mStringEnd++;
        tvp->tag = dest;
    }
}

/**
 * Offset/Address/Size: 0x1CF4 | 0x801D4958 | size: 0x120
 * TODO: 92.9% match - r28/r29 register swap throughout (idx/offset/tvp allocation),
 * hash loop *p++ lbz r0 + extra extsb r3,r0 (same as Exists), add operand order swap,
 * copy loop extsb. r3,r3 vs r0,r3, bge vs blt/b branch pattern
 */
void Config::Set(const char* tag, bool value)
{
    const char* p = tag;
    u32 hash = 0x1505;
    while (*p != 0)
    {
        s32 c = (s8)nlToUpper(*p++);
        hash += (hash << 5) + c;
    }
    u32 idx = hash & 0x3FF;

    TagValuePair* tvp;
    do
    {
        u32 offset = idx * 12;
        if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
        {
            tvp = (TagValuePair*)((u8*)mTvpHash + offset);
            break;
        }
        idx++;
        idx &= 0x3FF;
    } while (true);

    tvp->type = _BOOL;
    tvp->value.b = value;

    if (tvp->tag == NULL)
    {
        char* dest = mStringEnd;
        s32 ch;
        while ((ch = *tag) != 0)
        {
            if (mStringEnd - mStringMemory >= 0x27FF)
            {
                goto done;
            }
            *mStringEnd = nlToUpper((char)ch);
            tag++;
            mStringEnd++;
        }
        *mStringEnd = 0;
        mStringEnd++;
    done:
        tvp->tag = dest;
    }
}

/**
 * Offset/Address/Size: 0x1E14 | 0x801D4A78 | size: 0x120
 * TODO: 92.5% match - r28/r29 register swap in probe/copy path (idx/offset/tvp and strStart/tvp->tag),
 * hash loop emits pre-call extsb/add scheduling differences, copy loop branch shape (bge vs blt+b),
 */
void Config::Set(const char* tag, int value)
{
    const char* p = tag;
    u32 hash = 0x1505;
    while ((s8)*p != 0)
    {
        s32 c = nlToUpper(*p++);
        hash += (hash << 5) + (s8)c;
    }

    u32 h = hash;
    u32 idx = h & 0x3FF;

    TagValuePair* tvp;
    while (true)
    {
        u32 offset = idx * 12;
        if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
        {
            tvp = (TagValuePair*)((char*)mTvpHash + offset);
            break;
        }
        idx++;
        idx &= 0x3FF;
    }

    tvp->type = _INT;
    tvp->value.i = value;

    if (tvp->tag == NULL)
    {
        char* strStart = mStringEnd;
        char ch;
        while ((ch = *tag) != 0)
        {
            if (mStringEnd - mStringMemory < 0x27FF)
            {
                *mStringEnd = nlToUpper(ch);
                tag++;
                mStringEnd++;
            }
            else
            {
                break;
            }
        }
        *mStringEnd = '\0';
        mStringEnd++;
        tvp->tag = strStart;
    }
}

/**
 * Offset/Address/Size: 0x1F34 | 0x801D4B98 | size: 0xB8
 * TODO: 98.9% match - nlToUpper<Uc> vs nlToUpper<c> template instantiation (i diff),
 * r30/r31 register swap for idx/offset in probe loop (MWCC allocator quirk)
 */
TagValuePair& Config::FindTvp(const char* tag)
{
    u32 hash = 0x1505;
    const char* p = tag;
    while (*p != 0)
    {
        s8 c = (s8)nlToUpper((u8)*p++);
        hash = hash + (hash << 5) + c;
    }
    u32 idx = hash & 0x3FF;

    while (true)
    {
        u32 offset = idx * 12;
        if (mTvpHash[idx].tag == NULL || nlStrICmp(mTvpHash[idx].tag, tag) == 0)
        {
            return *(TagValuePair*)((char*)mTvpHash + offset);
        }
        idx = (idx + 1) & 0x3FF;
    }
}

/**
 * Offset/Address/Size: 0x1FEC | 0x801D4C50 | size: 0x534
 * TODO: 81.18% match - 4 vs 5 callee-saved register allocation (stw individual vs stmw r27),
 * dead loop in operator[] null case not reproducible (MWCC optimizes self-assignment away),
 * bne+b vs beq branch pattern for refcount==1 check
 */
bool Config::IsBool(const char* str, bool& b) const
{
    BasicString<char, Detail::TempStringAllocator> s(str);
    for (int i = 0; i < (int)s.size() - 1; i++)
    {
        char& r = s[i];
        r = _tolower(s[i]);
    }
    if (s == "true" || s == "yes" || s == "on" || s == "enable")
    {
        b = true;
        return true;
    }
    if (s == "false" || s == "no" || s == "off" || s == "disable")
    {
        b = false;
        return false;
    }
    return false;
}

/**
 * Offset/Address/Size: 0x2520 | 0x801D5184 | size: 0xC8
 * TODO: 99.9% match - nlToUpper<Uc> call site (from (u8)*p++) vs target nlToUpper<c>
 * template instantiation (i diff only)
 */
bool Config::Exists(const char* tag) const
{
    const char* p = tag;
    u32 hash = 0x1505;
    while (*p != 0)
    {
        s32 c = (s8)nlToUpper((u8)*p++);
        u32 h = hash;
        hash = h + (h << 5) + c;
    }
    u32 startIdx = hash & 0x3FF;

    TagValuePair* tvpHash = mTvpHash;
    u32 idx = startIdx;
    do
    {
        if (tvpHash[idx].tag == NULL)
        {
            return false;
        }
        if (nlStrICmp(tvpHash[idx].tag, tag) == 0)
        {
            return true;
        }
        idx++;
        idx &= 0x3FF;
    } while (idx != startIdx);
    return false;
}

/**
 * Offset/Address/Size: 0x25E8 | 0x801D524C | size: 0x138
 */
void Config::LoadFromFile(const char* filename)
{
    const BasicString<char, Detail::TempStringAllocator>& s = LoadFileAsString(filename);
    const char* str = s.c_str();
    SetTagValuePair stvp(*this);
    Parse(str, stvp);
}

/**
 * Offset/Address/Size: 0x2834 | 0x801D5498 | size: 0xCC
 */
Config::Config(Config::AllocateWhere allocateWhere)
{
    if (allocateWhere == ALLOCATE_LOW)
    {
        mTvpHash = new (nlMalloc(sizeof(TagValuePair) * 1024 + 0x10, 8, false)) TagValuePair[1024];
        mStringMemory = (char*)nlMalloc(0x2800, 8, false);
    }
    else
    {
        mTvpHash = new (nlMalloc(sizeof(TagValuePair) * 1024 + 0x10, 0x20, true)) TagValuePair[1024];
        mStringMemory = (char*)nlMalloc(0x2800, 0x20, true);
    }
    mStringEnd = mStringMemory;
    mStringMemory[0x27FF] = '\0';
}

/**
 * Offset/Address/Size: 0x2720 | 0x801D5384 | size: 0xAC
 */
Config& Config::Global()
{
    static Config sGlobal(ALLOCATE_LOW);
    return sGlobal;
}

/**
 * Offset/Address/Size: 0x27CC | 0x801D5430 | size: 0x68
 */
Config::~Config()
{
    if (mTvpHash != NULL)
    {
        ::operator delete[]((char*)mTvpHash - 0x10);
    }

    ::operator delete[](mStringMemory);
}

/**
 * Offset/Address/Size: 0x2900 | 0x801D5564 | size: 0x4
 */
void SetTagValuePair::Comment(const BasicString<char, Detail::TempStringAllocator>&)
{
}

/**
 * Offset/Address/Size: 0x2904 | 0x801D5568 | size: 0x94
 */
void SetTagValuePair::Section(const BasicString<char, Detail::TempStringAllocator>& section)
{
    mCurrentSection = section;
}

/**
 * Offset/Address/Size: 0x1C | 0x801D5830 | size: 0x20
 */
#pragma dont_inline on
template <typename T>
void Config::Set(const char* key, T value)
{
    Set(key, value);
}
template void Config::Set<BasicString<char, Detail::TempStringAllocator> >(const char*, BasicString<char, Detail::TempStringAllocator>);
#pragma dont_inline reset

/**
 * Offset/Address/Size: 0x2998 | 0x801D55FC | size: 0x218
 */
void SetTagValuePair::TagValuePair(const BasicString<char, Detail::TempStringAllocator>& tag,
    const BasicString<char, Detail::TempStringAllocator>& rhs)
{
    BasicString<char, Detail::TempStringAllocator> tagWithSection(tag);

    s32 sectionLen = mCurrentSection.m_data ? mCurrentSection.m_data->mSize - 1 : 0;
    if (sectionLen > 0)
    {
        tagWithSection = mCurrentSection.Append(".").Append(tag);
    }

    mConfig.Set<BasicString<char, Detail::TempStringAllocator> >(tagWithSection.c_str(), rhs);
}

/**
 * Offset/Address/Size: 0x0 | 0x801D5814 | size: 0x4
 */
// Config::Parser::TagValuePair(const BasicString<char, Detail::TempStringAllocator>&, const BasicString<char,
// Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0x4 | 0x801D5818 | size: 0x4
 */
// void Config::Parser::Section(const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0x8 | 0x801D581C | size: 0x4
 */
// void Config::Parser::Comment(const BasicString<char, Detail::TempStringAllocator>&)
// {
// }

/**
 * Offset/Address/Size: 0xC | 0x801D5820 | size: 0x4
 */
// void Config::Parser::EmptyLine()
// {
// }

/**
 * Offset/Address/Size: 0x10 | 0x801D5824 | size: 0xC
 */
Config::TagValuePair::TagValuePair()
{
    tag = NULL;
}

/**
 * Offset/Address/Size: 0x0 | 0x801D5850 | size: 0xBC
 */
// Trim is defined in nlBasicString.h (weak/template out-of-line body).
// Force instantiation via nlConfig_stub below.

/**
 * Offset/Address/Size: 0xBC | 0x801D590C | size: 0xBC4
 */
// BasicString<char, Detail::TempStringAllocator>::TrimInPlace(const char*)
// {
// }

// Force template instantiation - REMOVE once real callers exist
void nlConfig_stub()
{
    BasicString<char, Detail::TempStringAllocator> s;
    s.Trim("");
}

/**
 * Offset/Address/Size: 0xC80 | 0x801D64D0 | size: 0xBC
 */
// BasicString<char, Detail::TempStringAllocator>::Append<Detail::TempStringAllocator>(const BasicString<char,
// Detail::TempStringAllocator>&) const
// {
// }

/**
 * Offset/Address/Size: 0xD3C | 0x801D658C | size: 0x1E4
 */
// BasicString<char, Detail::TempStringAllocator>::AppendInPlace<Detail::TempStringAllocator>(const BasicString<char,
// Detail::TempStringAllocator>&)
// {
// }
