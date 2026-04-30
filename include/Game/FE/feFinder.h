#ifndef _FEFINDER_H_
#define _FEFINDER_H_

#include "NL/nlString.h"

struct InlineHasher
{
    /* 0x0 */ unsigned long m_Hash;
    InlineHasher() { }
    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
}; // total size: 0x4

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U* slide, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6);
    template <typename U>
    static T* _Find(U* slide, const unsigned long hash1, const unsigned long hash2, const unsigned long hash3, const unsigned long hash4, const unsigned long hash5, const unsigned long hash6);
};

template <typename T, int N>
template <typename U>
T* FEFinder<T, N>::Find(U* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3, InlineHasher h4, InlineHasher h5, InlineHasher h6)
{
    return _Find(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
}

#endif // _FEFINDER_H_
