#ifndef _FEFINDER_H_
#define _FEFINDER_H_

#include "NL/nlString.h"
#include "Game/FE/fePresentation.h"
#include "Game/FE/tlSlide.h"
#include "Game/FE/tlInstance.h"

template <class T>
T* FindItemByHashID(T* head, unsigned long hash);

template <class T>
T* CastToSomeType(T*, void* pValue);

struct InlineHasher
{
    /* 0x0 */ unsigned long m_Hash;
    InlineHasher() { }
    InlineHasher(unsigned long h)
        : m_Hash(h)
    {
    }
}; // total size: 0x4

static TLSlide* FEGetChildren(FEPresentation* p)
{
    return p->m_slides;
}
static TLInstance* FEGetChildren(TLSlide* p)
{
    return p->m_instances;
}
static TLInstance* FEGetChildren(TLInstance* p)
{
    return p->pChildren;
}

template <typename T, int N>
class FEFinder
{
public:
    template <typename U>
    static T* Find(U* pTopLevel, InlineHasher h1, InlineHasher h2, InlineHasher h3 = 0, InlineHasher h4 = 0, InlineHasher h5 = 0, InlineHasher h6 = 0)
    {
        return _Find(pTopLevel, h1.m_Hash, h2.m_Hash, h3.m_Hash, h4.m_Hash, h5.m_Hash, h6.m_Hash);
    }
    template <typename U>
    static T* _Find(U* pTopLevel, const unsigned long Level1, const unsigned long Level2,
        const unsigned long Level3, const unsigned long Level4, const unsigned long Level5, const unsigned long Level6)
    {
        void* pChild = FindItemByHashID(FEGetChildren(pTopLevel), Level1);
        if (pChild == 0)
            return 0;
        if (Level2 == 0)
            return (T*)pChild;
        return _Find(CastToSomeType(FEGetChildren(pTopLevel), pChild), Level2, Level3, Level4, Level5, Level6, 0);
    }
};

#endif // _FEFINDER_H_
