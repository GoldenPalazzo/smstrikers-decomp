#ifndef _NLSTRING_H_
#define _NLSTRING_H_

#include "types.h"
#include "strtold.h"
#include "NL/nlMemory.h"

namespace Detail
{
class TempStringAllocator
{
public:
    enum
    {
        kAtEnd = true
    };

    template <typename T>
    static T* New(int count, const char* name)
    {
        return new (8, kAtEnd, name) T[count];
    }

    template <typename T>
    static void Delete(T* ptr)
    {
        delete[] ptr;
    }

    static void* Alloc(int size)
    {
        return nlMalloc(size, 8, kAtEnd);
    }

    static void Free(void* ptr)
    {
        nlFree(ptr);
    }
};
} // namespace Detail

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
unsigned long nlStrLen(const CharT*);

template <typename CharT>
CharT* nlStrNCpy(CharT*, const CharT*, unsigned long);

template <typename CharT>
CharT* nlStrNCat(CharT*, const CharT*, const CharT*, unsigned long);

template <typename CharT>
int nlStrCmp(const CharT*, const CharT*);

template <typename CharT>
int nlStrICmp(const CharT*, const CharT*);

template <typename CharT>
int nlStrNICmp(const CharT*, const CharT*, unsigned long);

template <typename CharT>
int nlStrNCmp(const CharT*, const CharT*, unsigned long);

template <typename CharT>
CharT nlToUpper(CharT);

template <typename CharT>
CharT nlToLower(CharT);

template <typename CharT>
CharT* nlToLower(CharT*);

template <typename CharT>
CharT* nlStrChr(const CharT*, CharT);

#include "NL/nlstring_tmpl.h"

#endif // _NLSTRING_H_
