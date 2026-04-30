#ifndef _NLLEXICALCAST_H_
#define _NLLEXICALCAST_H_

#include "types.h"
#include "strtold.h"
#include "NL/nlBasicString.h"

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
struct LexicalCastImpl<To, bool>
{
    static To Do(bool t);
};
} // namespace Detail

typedef BasicString<unsigned short, Detail::TempStringAllocator> WideBasicString;

// Identity cast: WideBasicString -> WideBasicString (copy)
template <>
inline WideBasicString Detail::LexicalCastImpl<WideBasicString, WideBasicString>::Do(
    const WideBasicString& f)
{
    return f;
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
    FORCE_DONT_INLINE;
    return v != 0.0f;
}
template <>
bool LexicalCast<bool, const char*>(const char* const& s)
{
    FORCE_DONT_INLINE;
    // TODO: implement parsing if needed
    return true;
}

#endif // NL_LEXICALCAST_DEFINE_BOOL

#endif // _NLLEXICALCAST_H_
