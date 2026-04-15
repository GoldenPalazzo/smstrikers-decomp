#ifndef _NLLEXICALCAST_H_
#define _NLLEXICALCAST_H_

#include "types.h"
#include "strtold.h"
#include "NL/nlBasicString.h"

// Primary template declaration (not defined/omited yet)
template <typename To, typename From>
To LexicalCast(const From&);

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
BasicString<char, Detail::TempStringAllocator> LexicalCast<BasicString<char, Detail::TempStringAllocator>, int>(const int& value);

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
