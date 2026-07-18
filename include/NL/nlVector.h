#ifndef _NLVECTOR_H_
#define _NLVECTOR_H_

#include "NL/nlMath.h"
#include "NL/nlMemory.h"

template <typename T, typename Allocator = DefaultAllocator>
class Vector
{
public:
    Vector() { }
    Vector(const T* string)
    {
        mData = 0;
        mSize = 0;
        mCapacity = 0;

        const T* scan = string;
        while (*scan++ != 0)
        {
            mSize++;
        }

        mSize++;
        mData = new (8, Allocator::kAtEnd) T[mSize + 1];
        mCapacity = mSize;

        for (int i = 0; i < mSize; i++)
        {
            mData[i] = *string++;
        }
    }
    Vector(int count, const char* name)
    {
        mData = new (8, Allocator::kAtEnd) T[count];
        mSize = count;
        mCapacity = count;
        for (int i = 0; i < count; i++)
        {
            mData[i] = T();
        }
    }
    ~Vector()
    {
        delete[] mData;
    }
    void Swap(Vector& other);
    void reserve(int capacity);
    void resize(int size);
    void push_back(const T& value);
    void insert(T* position, const T* first, const T* last);
    T& operator[](int index)
    {
        return mData[index];
    }

    /* 0x0 */ T* mData;
    /* 0x4 */ int mSize;
    /* 0x8 */ int mCapacity;
}; // total size: 0xC

template <typename T, typename Allocator>
void Vector<T, Allocator>::resize(int size)
{
    if (size > mSize)
    {
        reserve(size);
        for (int i = mSize; i < size; i++)
        {
            T temp;
            mData[i] = temp;
        }
        mSize = size;
    }
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::push_back(const T& value)
{
    insert(mData + mSize, &value, &value + 1);
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::insert(T* at, const T* begin, const T* end)
{
    int size = end - begin;
    int offset = at - mData;
    reserve(mSize + size);
    at = mData + offset;
    T* t = mData + mSize - 1;
    while (t >= at)
    {
        *(t + size) = *t;
        t--;
    }
    while (begin != end)
    {
        *at = *begin;
        begin++;
        at++;
    }
    mSize += size;
}

template <typename T, typename Allocator>
inline void Vector<T, Allocator>::Swap(Vector<T, Allocator>& other)
{
    int oldSize = mSize;
    mSize = other.mSize;
    other.mSize = oldSize;
    int oldCapacity = mCapacity;
    mCapacity = other.mCapacity;
    other.mCapacity = oldCapacity;
    T* oldData = mData;
    mData = other.mData;
    other.mData = oldData;
}

template <typename T, typename Allocator>
void Vector<T, Allocator>::reserve(int capacity)
{
    FORCE_DONT_INLINE;
    if (mCapacity < capacity)
    {
        Vector<T, Allocator> other(capacity, 0);
        for (int i = 0; i < mSize; i++)
        {
            other.mData[i] = mData[i];
        }
        other.mSize = mSize;
        Swap(other);
    }
}

#endif // _NLVECTOR_H_
