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
unsigned long nlStrLen(const CharT* str)
{
    unsigned long n = 0;
    if (str)
    {
        for (; *str; n++, str++)
            ;
    }
    return n;
}

template <typename CharT>
CharT* nlStrChr(const CharT* str, CharT ch)
{
    while (*str != static_cast<CharT>(0))
    {
        if (*str == ch)
            return const_cast<CharT*>(str);
        ++str;
    }
    return nullptr;
}

template <typename CharT>
int nlStrCmp(const CharT* a, const CharT* b)
{
    CharT c1;
    CharT c2;

    do
    {
        c1 = *a++;
        c2 = *b++;
    } while (c1 != 0 && c2 != 0 && c1 == c2);

    return c1 - c2;
}

/**
 * Offset/Address/Size: 0x0 | 0x801514FC | size: 0x48
 * TODO: 87.78% match - two lbz instructions in swapped order (c2/b loaded before c1/a), beq branch prediction bit
 */
template <typename CharT>
int nlStrNCmp(const CharT* a, const CharT* b, unsigned long maxsize)
{
    int c1;
    int c2;

    do
    {
        --maxsize;
        c2 = (unsigned char)*b;
        c1 = (unsigned char)*a;
        a++;
        b++;
        if (maxsize == 0)
            break;
        if ((CharT)c1 == 0)
            break;
        if ((CharT)c2 == 0)
            break;
    } while ((CharT)c1 == (CharT)c2);
    return (CharT)c1 - (CharT)c2;
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

template <typename CharT>
CharT* nlToLower(CharT* str)
{
    CharT* cp = str;
    while (*cp)
    {
        *cp = nlToLower<CharT>(*cp);
        cp++;
    }
    return str;
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
