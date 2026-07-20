#ifndef _NLLEXICALCAST_H_
#define _NLLEXICALCAST_H_

#include "types.h"
#include "strtold.h"
#include "NL/nlBasicString.h"
#include "NL/nlPrint.h"

namespace Detail
{
template <typename To, typename From>
struct LexicalCastImpl
{
    static To Do(const From& f);
};
} // namespace Detail

template <typename To, typename From>
To LexicalCast(const From& from)
{
    return Detail::LexicalCastImpl<To, From>::Do(from);
}

namespace Detail
{

template <typename To>
struct LexicalCastImpl<To, const char*>
{
    static To Do(const char* s);
};

template <>
struct LexicalCastImpl<bool, const char*>
{
    static bool Do(const char* s) { return strcmp("true", s) == 0; }
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
struct LexicalCastImpl<To, unsigned int>
{
    static To Do(unsigned int t);
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

template <typename Allocator>
struct LexicalCastImpl<BasicString<char, Allocator>, int>
{
    static BasicString<char, Allocator> Do(int t);
};

template <typename Allocator>
struct LexicalCastImpl<BasicString<char, Allocator>, float>
{
    static BasicString<char, Allocator> Do(float t);
};

template <typename Allocator>
struct LexicalCastImpl<BasicString<char, Allocator>, bool>
{
    static BasicString<char, Allocator> Do(bool t);
};

template <>
struct LexicalCastImpl<bool, bool>
{
    static bool Do(bool t) { return t; }
};

template <>
struct LexicalCastImpl<bool, float>
{
    static bool Do(float t)
    {
        bool result;
        if (t)
            result = true;
        else
            result = false;
        return result;
    }
};
} // namespace Detail

typedef BasicString<unsigned short, Detail::TempStringAllocator> WideBasicString;
typedef BasicString<char, Detail::TempStringAllocator> NLString;

template <>
inline WideBasicString Detail::LexicalCastImpl<WideBasicString, WideBasicString>::Do(
    const WideBasicString& f)
{
    return f;
}

template <>
inline WideBasicString Detail::LexicalCastImpl<WideBasicString, const unsigned short*>::Do(
    const unsigned short* const& f)
{
    return WideBasicString(f);
}

template <>
inline NLString Detail::LexicalCastImpl<NLString, const char*>::Do(const char* s)
{
    return NLString(s);
}

template <>
inline int Detail::LexicalCastImpl<int, const char*>::Do(const char* s)
{
    return (int)atof(s);
}

template <>
inline float Detail::LexicalCastImpl<float, const char*>::Do(const char* s)
{
    return (float)atof(s);
}

template <>
inline NLString Detail::LexicalCastImpl<NLString, unsigned long>::Do(unsigned long t)
{
    char s[0x40];
    nlSNPrintf(s, 0x40, "%u", t);
    return NLString(s);
}

template <>
inline NLString Detail::LexicalCastImpl<NLString, unsigned int>::Do(unsigned int t)
{
    char s[0x40];
    nlSNPrintf(s, 0x40, "%u", t);
    return NLString(s);
}

template <>
inline NLString Detail::LexicalCastImpl<NLString, char>::Do(char t)
{
    char s[0x40];
    nlSNPrintf(s, 0x40, "%c", t);
    return NLString(s);
}

namespace Detail
{
template <typename Allocator>
inline BasicString<char, Allocator> LexicalCastImpl<BasicString<char, Allocator>, int>::Do(int t)
{
    char s[0x40];
    nlSNPrintf(s, 0x40, "%i", t);
    return BasicString<char, Allocator>(s);
}

template <typename Allocator>
inline BasicString<char, Allocator> LexicalCastImpl<BasicString<char, Allocator>, float>::Do(float t)
{
    char s[0x40];
    nlSNPrintf(s, 0x40, "%f", t);
    return BasicString<char, Allocator>(s);
}

template <typename Allocator>
inline BasicString<char, Allocator> LexicalCastImpl<BasicString<char, Allocator>, bool>::Do(bool t)
{
    if (t)
    {
        return BasicString<char, Allocator>("true");
    }
    return BasicString<char, Allocator>("false");
}

template <typename To>
To LexicalCastImpl<To, const char*>::Do(const char* s)
{
    return (To)atof(s);
}

template <typename To>
inline To LexicalCastImpl<To, int>::Do(int t)
{
    return (To)t;
}

template <typename To>
inline To LexicalCastImpl<To, float>::Do(float t)
{
    return (To)t;
}

template <typename To>
inline To LexicalCastImpl<To, bool>::Do(bool t)
{
    return (To)t;
}
} // namespace Detail

template <>
inline NLString LexicalCast<NLString, char>(const char& f)
{
    return Detail::LexicalCastImpl<NLString, char>::Do(const_cast<char&>(f));
}

template <>
const char* LexicalCast<const char*, const char*>(const char* const& value);
template <>
const char* LexicalCast<const char*, int>(const int& value);
template <>
const char* LexicalCast<const char*, float>(const float& value);
template <>
const char* LexicalCast<const char*, bool>(const bool& value);

#endif // _NLLEXICALCAST_H_
