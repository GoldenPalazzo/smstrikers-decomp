#ifndef _BASICSTRING_H_
#define _BASICSTRING_H_

#include "types.h"
#include "NL/nlMemory.h"
#include "NL/nlVector.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/string.h"
#include "NL/nlString.h"

// class Vector
// {
//     // total size: 0xC
//     char* mData;   // offset 0x0, size 0x4
//     int mSize;     // offset 0x4, size 0x4
//     int mCapacity; // offset 0x8, size 0x4
// };

// class Data
// {
//     // total size: 0x10
//     Vector mData;  // offset 0x0, size 0xC
//     int mRefCount; // offset 0xC, size 0x4
// };

// class BasicString
// {
//     // total size: 0x4
//     Data* mData; // offset 0x0, size 0x4
// };

// Forward declarations
namespace Detail
{
class TempStringAllocator;
} // namespace Detail

template <typename CharT, typename Allocator>
class BasicString;

// Format function returning result (SRP), with generic second param
template <typename StringType, typename T>
StringType Format(const StringType& format, const T& value);

// Detail namespace with TempStringAllocator
namespace Detail
{
class TempStringAllocator
{
public:
    enum
    {
        kAtEnd = true
    };

    static inline void* allocate(size_t size)
    {
        return nlMalloc(size, 8, true);
    }

    static void deallocate(void* ptr)
    {
        nlFree(ptr);
    }
};
} // namespace Detail

// BasicString template class - total size: 0x4 (pointer to Data)
template <typename CharT, typename Allocator>
class BasicString
{
public:
    typedef CharT value_type;

    class Data
    {
    public:
        Data(const CharT* string)
            : mData(string)
            , mRefCount(1)
        {
        }

        Data(const CharT* begin, const CharT* end)
            : mData(end - begin + 1, 0)
            , mRefCount(1)
        {
            for (int i = 0; i < mData.mSize - 1; i++)
            {
                mData.mData[i] = *begin++;
            }
        }

        void reserve(int capacity);
        void insertRange(CharT* at, const CharT* begin, const CharT* end);

        Data* AddRef()
        {
            ++mRefCount;
            return this;
        }

        Vector<CharT, Allocator> mData;
        int mRefCount;
    };

    Data* mData; // offset 0x0

    BasicString()
        : mData(0)
    {
    }

    BasicString(const CharT* string)
    {
        void* storage = Allocator::allocate(sizeof(Data));
        mData = new (storage) Data(string);
    }

    BasicString(const CharT* begin, const CharT* end);

    BasicString(Data* p)
        : mData(p)
    {
    }

    BasicString(const BasicString& other)
        : mData(other.mData != 0 ? other.mData->AddRef() : 0)
    {
    }

    ~BasicString()
    {
        if (mData)
        {
            Data* data = mData;
            if (--data->mRefCount == 0)
            {
                if (data)
                {
                    if (data)
                    {
                        delete[] data->mData.mData;
                    }
                    if (data)
                    {
                        nlFree(data);
                    }
                }
            }
        }
    }

    BasicString& operator=(BasicString other);

    BasicString& AppendInPlace(const CharT* str);

    template <typename OtherAllocator>
    BasicString& AppendInPlace(const BasicString<CharT, OtherAllocator>& rhs);

    const CharT* c_str() const
    {
        static CharT emptyString = '\0';
        return mData ? mData->mData.mData : &emptyString;
    }

    int size() const
    {
        return mData ? mData->mData.mSize - 1 : 0;
    }

    CharT* begin()
    {
        (*this)[0];
        return mData ? mData->mData.mData : (CharT*)0;
    }

    CharT* end()
    {
        (*this)[(int)(mData ? mData->mData.mSize - 1 : 0)];
        return mData ? mData->mData.mData + mData->mData.mSize - 1 : (CharT*)0;
    }

    const CharT* begin() const
    {
        return mData ? mData->mData.mData : (const CharT*)0;
    }

    const CharT* end() const
    {
        return mData ? mData->mData.mData + mData->mData.mSize - 1 : (const CharT*)0;
    }

    const CharT& operator[](int index) const
    {
        return mData->mData.mData[index];
    }

    CharT& operator[](int index)
    {
        Data* oldData = mData;
        if (oldData == 0)
        {
            Data* data = (Data*)Allocator::allocate(sizeof(Data));
            if (data != 0)
            {
                data->mData.mData = (CharT*)Allocator::allocate(sizeof(CharT));
                int sz = 1;
                data->mData.mSize = sz;
                data->mData.mCapacity = sz;
                data->mData[0] = 0;
                data->mRefCount = 1;
                for (int j = 0; j < data->mData.mSize - 1; j++)
                {
                    data->mData[j] = ((CharT*)0)[j];
                }
            }
            mData = data;
        }
        else
        {
            if (oldData->mRefCount == 1)
            {
                oldData = mData;
            }
            else
            {
                Data* newData = (Data*)Allocator::allocate(sizeof(Data));
                if (newData != 0)
                {
                    newData->mData.mData = (CharT*)Allocator::allocate(oldData->mData.mSize * sizeof(CharT));
                    newData->mData.mSize = oldData->mData.mSize;
                    newData->mData.mCapacity = oldData->mData.mSize;
                    for (int j = 0; j < newData->mData.mSize; j++)
                    {
                        newData->mData.mData[j] = oldData->mData.mData[j];
                    }
                    newData->mRefCount = 1;
                }
                if (--oldData->mRefCount == 0)
                {
                    if (oldData)
                    {
                        if (oldData)
                        {
                            delete[] oldData->mData.mData;
                        }
                        if (oldData)
                        {
                            nlFree(oldData);
                        }
                    }
                }
                oldData = newData;
            }
            mData = oldData;
        }
        return mData->mData.mData[index];
    }

    void insert(CharT* at, const CharT* begin, const CharT* end);

    void erase(const CharT* begin, const CharT* end);

    void TrimInPlace(const CharT* chars);

    BasicString Trim(const CharT* chars) const;

    BasicString Append(const CharT* rhs) const;

    template <typename OtherAllocator>
    BasicString Append(const BasicString<CharT, OtherAllocator>& rhs) const
    {
        BasicString r(*this);
        r.AppendInPlace(rhs);
        Data* data = r.mData;
        if (data != 0)
        {
            data->mRefCount++;
        }
        else
        {
            data = 0;
        }
        return BasicString(data);
    }
};

template <typename CharT, typename Allocator>
inline BasicString<CharT, Allocator>::BasicString(const CharT* begin, const CharT* end)
    : mData(new (8, Allocator::kAtEnd) Data(begin, end))
{
}

template <typename CharT, typename Allocator>
BasicString<CharT, Allocator>& BasicString<CharT, Allocator>::operator=(BasicString other)
{
    Data* tmp = mData;
    mData = other.mData;
    other.mData = tmp;
    return *this;
}

template <typename CharT, typename Allocator>
BasicString<CharT, Allocator> BasicString<CharT, Allocator>::Append(const CharT* rhs) const
{
    BasicString r(*this);
    r.AppendInPlace(rhs);
    Data* data = r.mData;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }
    return BasicString(data);
}

template <typename CharT, typename Allocator>
template <typename OtherAllocator>
BasicString<CharT, Allocator>& BasicString<CharT, Allocator>::AppendInPlace(const BasicString<CharT, OtherAllocator>& rhs)
{
    (*this)[0];

    CharT* at;
    Data* currentData = mData;
    if (currentData != 0)
    {
        at = currentData->mData.mData + currentData->mData.mSize - 1;
    }
    else
    {
        at = 0;
    }

    typename BasicString<CharT, OtherAllocator>::Data* rhsData = rhs.mData;
    const CharT* begin;
    if (rhsData != 0)
    {
        begin = rhsData->mData.mData;
    }
    else
    {
        begin = 0;
    }

    insert(at, begin, rhsData != 0 ? rhsData->mData.mData + rhsData->mData.mSize - 1 : 0);
    return *this;
}

template <typename CharT, typename Allocator>
BasicString<CharT, Allocator>& BasicString<CharT, Allocator>::AppendInPlace(const CharT* str)
{
    const CharT* rhsEnd = str;
    while (*rhsEnd != 0)
    {
        rhsEnd++;
    }

    (*this)[0];

    CharT* at;
    if (mData != 0)
    {
        at = mData->mData.mData + mData->mData.mSize - 1;
    }
    else
    {
        at = 0;
    }

    insert(at, str, rhsEnd);
    return *this;
}

template <typename CharT, typename Allocator>
BasicString<CharT, Allocator> BasicString<CharT, Allocator>::Trim(const CharT* chars) const
{
    BasicString r(*this);
    r.TrimInPlace(chars);
    Data* data = r.mData;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }
    return BasicString(data);
}

template <typename CharT>
static inline void InitBasicStringVector(Vector<CharT>& vec, int count)
{
    vec.mData = new (8, true) CharT[count];
    vec.mSize = count;
    vec.mCapacity = count;
    for (int i = 0; i < count; i++)
    {
        vec.mData[i] = CharT();
    }
}

template <typename CharT, typename Allocator>
inline void BasicString<CharT, Allocator>::Data::reserve(int capacity)
{
    if (mData.mCapacity < capacity)
    {
        Vector<CharT> newVec;
        InitBasicStringVector(newVec, capacity);
        int i = 0;
        for (; i < mData.mSize; i++)
        {
            newVec.mData[i] = mData.mData[i];
        }
        newVec.mSize = mData.mSize;
        int newVecSize = newVec.mSize;
        mData.mSize = newVecSize;
        newVec.mSize = newVecSize;

        int oldCapacity = mData.mCapacity;
        mData.mCapacity = newVec.mCapacity;
        newVec.mCapacity = oldCapacity;

        CharT* oldBuf = mData.mData;
        mData.mData = newVec.mData;
        newVec.mData = oldBuf;
    }
}

template <typename CharT, typename Allocator>
inline void BasicString<CharT, Allocator>::Data::insertRange(CharT* at, const CharT* begin, const CharT* end)
{
    int size = end - begin;
    int offset = at - mData.mData;
    reserve(mData.mSize + size);

    at = mData.mData + offset;
    CharT* t = mData.mData + mData.mSize - 1;
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
    mData.mSize += size;
}

// TODO: register assignments and stack frame still differ across insert instantiations.
template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::insert(CharT* at, const CharT* begin, const CharT* end)
{
    (*this)[0];
    int offset = at - (mData ? mData->mData.mData : (CharT*)0);
    (*this)[0];
    (*this)[0];

    Data* data = mData;
    CharT* dataPtr = data ? data->mData.mData : (CharT*)0;
    data->insertRange(dataPtr + offset, begin, end);
}

template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::erase(const CharT* begin, const CharT* end)
{
    (*this)[0];
    CharT* at;
    int size = end - begin;
    const CharT* eraseEnd = end;
    Data* data = mData;
    at = data->mData.mData + (begin - data->mData.mData);
    while (eraseEnd != data->mData.mData + data->mData.mSize)
    {
        *at = *eraseEnd;
        eraseEnd++;
        at++;
    }
    data->mData.mSize -= size;
}

// TODO: 98.84% match - scan cursor and copy-on-write temporary register swaps remain.
template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::TrimInPlace(const CharT* chars)
{
    int i = 0;
    const CharT* c;
    while (i < (int)(mData ? mData->mData.mSize - 1 : 0))
    {
        c = chars;
        while (*c != 0)
        {
            if (*c == (*this)[i])
                break;
            c++;
        }
        if (*c == 0)
            break;
        i++;
    }

    (*this)[0];
    const CharT* eraseEnd = (mData ? mData->mData.mData : (CharT*)0) + i;
    (*this)[0];
    const CharT* eraseBegin = mData ? mData->mData.mData : (CharT*)0;

    {
        (*this)[0];
        CharT* at;
        int size = eraseEnd - eraseBegin;
        Data* data = mData;
        int offset = eraseBegin - data->mData.mData;
        at = data->mData.mData + offset;
        while (eraseEnd != data->mData.mData + data->mData.mSize)
        {
            *at = *eraseEnd;
            eraseEnd++;
            at++;
        }
        data->mData.mSize -= size;
    }

    int last = (int)(mData ? mData->mData.mSize - 1 : 0) - 1;
    while (last >= 0)
    {
        c = chars;
        while (*c != 0)
        {
            if (*c == (*this)[last])
                break;
            c++;
        }
        if (*c == 0)
            break;
        last--;
    }

    (*this)[0];
    const CharT* trailEnd;
    if (mData)
        trailEnd = mData->mData.mData + mData->mData.mSize - 1;
    else
        trailEnd = (CharT*)0;
    (*this)[0];
    const CharT* trailBegin = (mData ? mData->mData.mData : (CharT*)0) + (last + 1);

    {
        (*this)[0];
        CharT* at;
        int size = trailEnd - trailBegin;
        Data* data = mData;
        int offset = trailBegin - data->mData.mData;
        at = data->mData.mData + offset;
        while (trailEnd != data->mData.mData + data->mData.mSize)
        {
            *at = *trailEnd;
            trailEnd++;
            at++;
        }
        data->mData.mSize -= size;
    }
}

template <typename CharT, typename Allocator>
bool operator==(const BasicString<CharT, Allocator>& lhs, const char* rhs)
{
    unsigned int c;
    typename BasicString<CharT, Allocator>::Data* data = lhs.mData;
    int i = 0;

    while (i < (data != 0 ? data->mData.mSize - 1 : 0))
    {
        c = (u8)*rhs;
        if ((CharT)c == 0)
        {
            return false;
        }
        if ((CharT)c != (CharT)data->mData.mData[i])
        {
            return false;
        }
        rhs++;
        i++;
    }

    return *rhs == '\0';
}

#endif
