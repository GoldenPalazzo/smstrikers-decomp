#ifndef _NLLEXICALCAST_H_
#define _NLLEXICALCAST_H_

#include "types.h"
#include "strtold.h"
#include "NL/nlBasicString.h"
#include "NL/nlPrint.h"

// Expands to a forward-declaration of the by-value LexicalCast primary
// templates inside the enclosing namespace. Used by translation units
// (e.g. TakeGameMemSnapshot) that ship their own parallel copy of the
// template machinery with `static To Do(From)` (by-value) instead of
// the by-const-ref signature used at global scope below.
#define NL_DECLARE_LOCAL_LEXICAL_CAST_BY_VALUE() \
    namespace Detail                             \
    {                                            \
    template <typename To, typename From>        \
    struct LexicalCastImpl                       \
    {                                            \
        static To Do(From);                      \
    };                                           \
    }                                            \
    template <typename To, typename From>        \
    To LexicalCast(const From& value)

namespace Detail
{
template <typename To, typename From>
struct LexicalCastImpl
{
    static To Do(const From& f);
};
} // namespace Detail

namespace Detail
{
template <typename To>
struct LexicalCastImpl<To, const char*>
{
    static To Do(const char* s);
};

template <typename To>
struct LexicalCastImpl<To, int>
{
    static To Do(int t);
};

template <typename To>
struct LexicalCastImpl<To, unsigned long>
{
    static To Do(unsigned long t);
};

template <typename To>
struct LexicalCastImpl<To, char>
{
    static To Do(char t);
};

template <typename To>
struct LexicalCastImpl<To, float>
{
    static To Do(float t);
};

template <typename To>
struct LexicalCastImpl<To, bool>
{
    static To Do(bool t);
};
} // namespace Detail

typedef BasicString<unsigned short, Detail::TempStringAllocator> WideBasicString;
typedef BasicString<char, Detail::TempStringAllocator> NLString;

// Identity cast: WideBasicString -> WideBasicString (copy)
template <>
inline WideBasicString Detail::LexicalCastImpl<WideBasicString, WideBasicString>::Do(
    const WideBasicString& f)
{
    return f;
}

/**
 * Offset/Address/Size: 0x80 | 0x8009CF48 | size: 0xF4
 * TODO: 97.43% match - return-buffer/source-pointer register roles are
 * swapped (r29/r31).
 */
template <>
inline WideBasicString Detail::LexicalCastImpl<WideBasicString, const unsigned short*>::Do(
    const unsigned short* const& f)
{
    return WideBasicString(f);
}

/**
 * Offset/Address/Size: 0x0 | 0x800BDB88 | size: 0xE8
 * TODO: 97.16% match - r29/r30 register swap for s and data pointers
 */
template <>
inline NLString Detail::LexicalCastImpl<NLString, const char*>::Do(const char* s)
{
    return NLString(s);
}

#ifndef NL_NO_LEXICALCAST_NLSTRING_INT
/**
 * Offset/Address/Size: 0x39064 | 0x8003C084 | size: 0x100
 * TODO: 97.66% match - return-buffer/data-pointer register roles remain
 * swapped (r30/r31).
 */
template <>
inline NLString Detail::LexicalCastImpl<NLString, int>::Do(int t)
{
    BasicStringData<char>* data;
    char string[0x40];
    nlSNPrintf(string, 0x40, "%i", t);

    data = (BasicStringData<char>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        char* start = string;
        char* p = start;

        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        while (*p++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
        data->mCapacity = data->mSize;

        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *start;
            start++;
        }

        data->mRefCount = 1;
    }

    return NLString(data);
}
#endif

/**
 * Offset/Address/Size: 0x168 | 0x8006A094 | size: 0x100
 * TODO: 97.66% match - r30/r31 register roles are swapped between return
 * slot and the allocated BasicStringData pointer.
 */
template <>
inline NLString Detail::LexicalCastImpl<NLString, unsigned long>::Do(unsigned long t)
{
    char string[0x40];
    nlSNPrintf(string, 0x40, "%u", t);

    BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        char* start = string;
        char* p = start;

        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        while (*p++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
        data->mCapacity = data->mSize;

        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *start;
            start++;
        }

        data->mRefCount = 1;
    }

    return NLString(data);
}

/**
 * Offset/Address/Size: 0x34 | 0x80069F60 | size: 0x104
 * TODO: 94.69% match - return-slot/data pointer register roles are swapped
 * (r30/r31), and prologue setup order differs before the snprintf call.
 */
template <>
inline NLString Detail::LexicalCastImpl<NLString, char>::Do(char t)
{
    char string[0x40];
    nlSNPrintf(string, 0x40, "%c", t);

    BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        char* start = string;
        char* p = start;

        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        while (*p++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
        data->mCapacity = data->mSize;

        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *start;
            start++;
        }

        data->mRefCount = 1;
    }

    return (NLString)data;
}

/**
 * Offset/Address/Size: 0x390F4 | 0x8003C1B4 | size: 0xFC
 * TODO: 97.62% match - return-slot/data pointer register roles are swapped
 * (r30/r31) in the inlined string construction path.
 */
template <>
inline NLString Detail::LexicalCastImpl<NLString, float>::Do(float f)
{
    BasicStringData<char>* data;
    char string[0x40];
    nlSNPrintf(string, 0x40, "%f", f);

    data = (BasicStringData<char>*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        char* start = string;
        char* p = start;

        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;

        while (*p++ != 0)
        {
            data->mSize++;
        }

        data->mSize++;
        data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
        data->mCapacity = data->mSize;

        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *start;
            start++;
        }

        data->mRefCount = 1;
    }

    return NLString(data);
}

template <>
inline NLString Detail::LexicalCastImpl<NLString, bool>::Do(bool t)
{
    if (t)
    {
        BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(sizeof(BasicStringData<char>), 8, true);
        if (data != 0)
        {
            const char* s = "true";
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;

            const char* p = s;
            while (*p++ != 0)
            {
                data->mSize++;
            }

            data->mSize++;
            data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
            data->mCapacity = data->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = *s++;
            }

            data->mRefCount = 1;
        }
        return NLString((BasicStringData<char>*)data);
    }
    else
    {
        BasicStringData<char>* data = (BasicStringData<char>*)nlMalloc(sizeof(BasicStringData<char>), 8, true);
        if (data != 0)
        {
            const char* s = "false";
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;

            const char* p = s;
            while (*p++ != 0)
            {
                data->mSize++;
            }

            data->mSize++;
            data->mData = (char*)nlMalloc(data->mSize + 1, 8, true);
            data->mCapacity = data->mSize;

            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = *s++;
            }

            data->mRefCount = 1;
        }
        return NLString((BasicStringData<char>*)data);
    }
}

template <typename To, typename From>
To LexicalCast(const From& f)
{
    return Detail::LexicalCastImpl<To, From>::Do(f);
}

template <>
int LexicalCast<int, float>(const float& value);
template <>
int LexicalCast<int, int>(const int& value);
template <>
int LexicalCast<int, bool>(const bool& value);
template <>
int LexicalCast<int, const char*>(const char* const& value);

template <>
float LexicalCast<float, float>(const float& value);
template <>
float LexicalCast<float, int>(const int& value);
template <>
float LexicalCast<float, bool>(const bool& value);
template <>
float LexicalCast<float, const char*>(const char* const& value);

template <>
bool LexicalCast<bool, bool>(const bool& value);
template <>
bool LexicalCast<bool, int>(const int& value);
template <>
bool LexicalCast<bool, float>(const float& value);
template <>
bool LexicalCast<bool, const char*>(const char* const& value);

template <>
const char* LexicalCast<const char*, const char*>(const char* const& value);
template <>
const char* LexicalCast<const char*, int>(const int& value);
template <>
const char* LexicalCast<const char*, float>(const float& value);
template <>
const char* LexicalCast<const char*, bool>(const bool& value);

#ifdef NL_LEXICALCAST_DEFINE

/* ----------------------------------------------------------------------
   Definitions (emitted in EXACT order below) -- in ONE TU only.
   Do this in exactly one .cpp:
       #define NL_LEXICALCAST_DEFINE
       #include "nlLexicalCast.h"

   Implementation locations:
   - int/float specializations: CharacterTweaks.cpp
   - bool and BasicString specializations: To be determined
   ---------------------------------------------------------------------- */

template <>
int LexicalCast<int, float>(const float& v)
{
    FORCE_DONT_INLINE;
    return (int)v;
}
template <>
int LexicalCast<int, int>(const int& v)
{
    FORCE_DONT_INLINE;
    return v;
}
template <>
int LexicalCast<int, bool>(const bool& v)
{
    FORCE_DONT_INLINE;
    return (int)v;
}
template <>
float LexicalCast<float, float>(const float& v)
{
    FORCE_DONT_INLINE;
    return v;
}
template <>
float LexicalCast<float, int>(const int& v)
{
    FORCE_DONT_INLINE;
    return (float)v;
}
template <>
float LexicalCast<float, bool>(const bool& v)
{
    FORCE_DONT_INLINE;
    return (float)v;
}
template <>
float LexicalCast<float, const char*>(const char* const& s)
{
    FORCE_DONT_INLINE;
    return (float)atof(s);
}
template <>
int LexicalCast<int, const char*>(const char* const& s)
{
    FORCE_DONT_INLINE;
    return (int)atof(s);
}
#endif // NL_LEXICALCAST_DEFINE

#ifdef NL_LEXICALCAST_DEFINE_BOOL

template <>
bool LexicalCast<bool, bool>(const bool& v)
{
    FORCE_DONT_INLINE;
    return v;
}
template <>
bool LexicalCast<bool, int>(const int& v)
{
    FORCE_DONT_INLINE;
    return v != 0;
}
template <>
bool LexicalCast<bool, float>(const float& v)
{
    bool result;
    if (v)
        result = true;
    else
        result = false;
    return result;
}
template <>
bool LexicalCast<bool, const char*>(const char* const& s)
{
    FORCE_DONT_INLINE;
    return strcmp("true", s) == 0;
}

#endif // NL_LEXICALCAST_DEFINE_BOOL

#endif // _NLLEXICALCAST_H_
