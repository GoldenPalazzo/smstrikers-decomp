#ifndef _NLSTRING_H_
#define _NLSTRING_H_

#include "types.h"
#include "strtold.h"

#ifndef nlPrintf
extern int nlPrintf(const char*, ...);
#endif

struct nlString
{
    char* data;
    u32 length;
    u32 capacity;
    u32 refcount;
};

void nlStrToWcs(const char*, unsigned short*, unsigned long);
void nlZeroMemory(void*, unsigned long);
u32 nlStringLowerHash(const char*);
u32 nlStringHash(const char*);

template <typename CharT>
s32 nlStrLen(const CharT* str)
{
    s32 len = 0;
    while (str[len] != 0)
    {
        len++;
    }
    return len;
}

template <typename CharT>
CharT* nlStrChr(const CharT* str, CharT ch)
{
    // if (!str) return nullptr;
    while (*str != static_cast<CharT>(0))
    {
        if (*str == ch)
            return const_cast<CharT*>(str);
        ++str;
    }
    return nullptr;
}

template <typename CharT>
int nlStrCmp(const CharT* str1, const CharT* str2)
{
    return nlStrNCmp(str1, str2, nlStrLen(str1));
}

/**
 * Offset/Address/Size: 0x0 | 0x801514FC | size: 0x48
 * CrowdMood: void nlStrNCmp<char>(const char*, const char*, unsigned long)
 */
template <typename CharT>
int nlStrNCmp(const CharT* str1, const CharT* str2, unsigned long len)
{
    CharT cVar1;
    CharT cVar2;

    while (true)
    {
        len -= 1;
        cVar1 = *str1;
        cVar2 = *str2;
        str1++;
        str2++;
        if (len == 0)
            break;
        if (((cVar1 == '\0') || (cVar2 == '\0')) || (cVar1 != cVar2))
            break;
    }
    return (int)cVar1 - (int)cVar2;
}

template <typename CharT>
CharT nlToUpper(CharT c)
{
    if ((c >= 0x61) && (c <= 0x7A))
    {
        return c & 0x5F;
    }
    return c;
}

template <typename CharT>
int nlStrICmp(const CharT* str1, const CharT* str2)
{
    CharT c1;
    CharT c2;

    do
    {
        c1 = nlToUpper<CharT>(*str1++);
        c2 = nlToUpper<CharT>(*str2++);
    } while (c1 != 0 && c2 != 0 && c1 == c2);

    return c1 - c2;
}

/**
 * Offset/Address/Size: 0x0 | 0x801D2854 | size: 0x20
 * void nlToLower<unsigned char>(unsigned char)
 */

// Single character version (current implementation)
template <typename CharT>
CharT nlToLower(CharT c)
{
    if ((c >= 0x41) && (c <= 0x5A))
    {
        c = (CharT)(c | 0x20);
    }
    return c;
}

// String version (new addition)
template <typename CharT>
void nlToLower(CharT* str)
{
    while (*str)
    {
        *str = nlToLower<CharT>(*str);
        str++;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x8000DEFC | size: 0x40
 * TODO: 98.75% match - lbz/extsb r0/r7 register swap (3 register-only diffs)
 */
template <typename CharT>
CharT* nlStrNCpy(CharT* str1, const CharT* str2, unsigned long len)
{
    CharT* p;
    CharT c;
    unsigned long n;
    n = len;
    p = str1;
    goto test;
loop:
    p++;
    str2++;
test:
    if (n-- == 0)
        goto done;
    c = *str2;
    *p = c;
    if (c)
        goto loop;
done:
    str1[len - 1] = '\0';
    return str1;
}

/**
 * Offset/Address/Size: 0x0 | 0x8000DEFC | size: 0x90
 * CharT* nlStrNCat<char>(char*, const char*, const char*, unsigned long)
 */
template <typename CharT>
CharT* nlStrNCat(CharT* dest, const CharT* a, const CharT* b, unsigned long maxsize)
{
    CharT* p;
    unsigned long n = 0;

    goto entry1;
body1:
    *p++ = *a++;
    n++;
    if (n >= maxsize)
    {
        dest[maxsize - 1] = (CharT)0;
        return dest;
    }
    goto test1;
entry1:
    p = dest;
test1:
    if (*a)
        goto body1;

    goto entry2;
body2:
    *p++ = *b++;
    n++;
    if (n >= maxsize)
    {
        dest[maxsize - 1] = (CharT)0;
        return dest;
    }
    goto test2;
entry2:
    p = &dest[n];
test2:
    if (*b)
        goto body2;

    dest[n] = (CharT)0;
    return dest;
}

#endif // _NLSTRING_H_
