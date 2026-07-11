#ifndef NL_ADAPTER_H
#define NL_ADAPTER_H

#include "NL/nlMemory.h"

#ifdef NL_NEWADAPTER_EXPLICIT_LINK_ORDER
#define NL_NEWADAPTER_DELETE_DECL WEAKFUNC void
#else
#define NL_NEWADAPTER_DELETE_DECL void
#endif

template <typename T>
class NewAdapter
{
public:
    // For AVL Tree interface
    T* Allocate() { return (T*)nlMalloc(sizeof(T), 8, false); }
    void Allocate(T*& out) { out = (T*)nlMalloc(sizeof(T), 8, false); }
    void Free(T* ptr) { delete ptr; }
    NL_NEWADAPTER_DELETE_DECL Delete(T* ptr);

    // For List interface
    typedef T EntryType;
    static void DeleteEntry(T* entry) { delete entry; }
};

template <typename T>
NL_NEWADAPTER_DELETE_DECL NewAdapter<T>::Delete(T* ptr)
{
    delete ptr;
}

#undef NL_NEWADAPTER_DELETE_DECL

#endif // NL_ADAPTER_H
