#ifndef NL_ADAPTER_H
#define NL_ADAPTER_H

#include "NL/nlMemory.h"

template <typename T>
class NewAdapter
{
public:
    // For AVL Tree interface
    T* Allocate() { return (T*)nlMalloc(sizeof(T), 8, false); }
    void Allocate(T*& out) { out = (T*)nlMalloc(sizeof(T), 8, false); }
    void Free(T* ptr) { nlFree(ptr); }

    // For List interface
    typedef T EntryType;
    static void DeleteEntry(T* entry);
};

template <typename T>
void NewAdapter<T>::DeleteEntry(T* entry)
{
    delete entry;
}

#endif // NL_ADAPTER_H
