#ifndef _NLVECTOR_H_
#define _NLVECTOR_H_

#include "NL/nlMath.h"
#include "NL/nlMemory.h"

template <typename T, typename Allocator = DefaultAllocator>
class Vector
{
public:
    ~Vector()
    {
        delete[] mData;
    }
    void reserve(int capacity);
    void resize(int size);
    void push_back(const T& value);
    void insert(T* position, const T* first, const T* last);

    /* 0x0 */ T* mData;
    /* 0x4 */ int mSize;
    /* 0x8 */ int mCapacity;
}; // total size: 0xC

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& value)
{
    insert(mData + mSize, &value, &value + 1);
}

#endif // _NLVECTOR_H_
