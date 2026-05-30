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
}

template <typename CharT, typename Allocator>
class BasicString;

// Format function for single float argument (no variadic templates)
template <typename StringType>
void Format(StringType& result, const StringType& format, const float& value);

// Format function for two float arguments
template <typename StringType>
void Format(StringType& result, const StringType& format, const float& value1, const float& value2);

// Format function returning result (SRP), with generic second param
template <typename StringType, typename T>
StringType Format(const StringType& format, const T& value);

// Detail namespace with TempStringAllocator
namespace Detail
{
class TempStringAllocator
{
public:
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

// BasicString data storage - templated on character type
template <typename CharT>
struct BasicStringData
{
    CharT* mData;  // offset 0x0
    int mSize;     // offset 0x4
    int mCapacity; // offset 0x8
    int mRefCount; // offset 0xC
};

// Backward-compatible typedef for char specialization
typedef BasicStringData<char> BasicStringInternal;

// BasicString template class - total size: 0x4 (pointer to BasicStringData)
template <typename CharT, typename Allocator>
class BasicString
{
public:
    typedef CharT value_type;
    BasicStringData<CharT>* m_data; // offset 0x0

    BasicString()
        : m_data(0)
    {
    }

    BasicString(const CharT* str)
#ifndef BASICSTRING_OUTLINE_CTOR
    {
        BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
        if (data != 0)
        {
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;
            const CharT* s = str;
            while (*s++ != 0)
            {
                data->mSize++;
            }
            data->mSize++;
            data->mData = (CharT*)Allocator::allocate((data->mSize + 1) * sizeof(CharT));
            data->mCapacity = data->mSize;
            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = *str++;
            }
            data->mRefCount = 1;
        }
        m_data = data;
    }
#endif
    ;

    BasicString(BasicStringData<CharT>* p)
        : m_data(p)
    {
    }

    BasicString(const BasicString& other)
    {
        BasicStringData<CharT>* data = other.m_data;
        if (data != 0)
        {
            data->mRefCount++;
#ifndef BASICSTRING_NO_COPY_REREAD
            data = other.m_data;
#endif
        }
        else
        {
            data = 0;
        }
        m_data = data;
    }

    ~BasicString()
    {
        if (m_data)
        {
            BasicStringData<CharT>* data = m_data;
            if (--data->mRefCount == 0)
            {
                if (data)
                {
                    if (data)
                    {
                        delete[] data->mData;
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
        return m_data ? m_data->mData : &emptyString;
    }

    u32 size() const
    {
        return m_data ? m_data->mSize : 0;
    }

    CharT& operator[](int index)
    {
        BasicStringData<CharT>* oldData = m_data;
        if (oldData == 0)
        {
            BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
            if (data != 0)
            {
                data->mData = (CharT*)Allocator::allocate(sizeof(CharT));
                int sz = 1;
                data->mSize = sz;
                data->mCapacity = sz;
                data->mData[0] = 0;
                data->mRefCount = 1;
                for (int j = 0; j < data->mSize - 1; j++)
                {
                    data->mData[j] = ((CharT*)0)[j];
                }
            }
            m_data = data;
        }
        else
        {
            if (oldData->mRefCount != 1)
            {
                BasicStringData<CharT>* newData = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
                if (newData != 0)
                {
                    newData->mData = (CharT*)Allocator::allocate(oldData->mSize * sizeof(CharT));
                    newData->mSize = oldData->mSize;
                    newData->mCapacity = oldData->mSize;
                    for (int j = 0; j < newData->mSize; j++)
                    {
                        newData->mData[j] = oldData->mData[j];
                    }
                    newData->mRefCount = 1;
                }
                if (--oldData->mRefCount == 0)
                {
                    if (oldData)
                    {
                        if (oldData)
                        {
                            delete[] oldData->mData;
                        }
                        if (oldData)
                        {
                            nlFree(oldData);
                        }
                    }
                }
                oldData = newData;
            }
            m_data = oldData;
        }
        return m_data->mData[index];
    }

    void insert(CharT* at, const CharT* begin, const CharT* end);

    void erase(const CharT* begin, const CharT* end);

    void TrimInPlace(const CharT* chars);

    BasicString Trim(const CharT* chars) const;

    BasicString Append(const CharT* rhs) const
#ifdef BASICSTRING_NO_INLINE_APPEND
        ;
#else
    {
        BasicString r(*this);
        r.AppendInPlace(rhs);
        BasicStringData<CharT>* data = r.m_data;
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
#endif

    template <typename OtherAllocator>
    BasicString Append(const BasicString<CharT, OtherAllocator>& rhs) const
    {
        BasicString r(*this);
        r.AppendInPlace(rhs);
        BasicStringData<CharT>* data = r.m_data;
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

#ifdef BASICSTRING_OUTLINE_CTOR
template <typename CharT, typename Allocator>
BasicString<CharT, Allocator>::BasicString(const CharT* str)
{
    BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
    if (data != 0)
    {
        data->mData = 0;
        data->mSize = 0;
        data->mCapacity = 0;
        const CharT* s = str;
        while (*s++ != 0)
        {
            data->mSize++;
        }
        data->mSize++;
        data->mData = (CharT*)Allocator::allocate((data->mSize + 1) * sizeof(CharT));
        data->mCapacity = data->mSize;
        for (int i = 0; i < data->mSize; i++)
        {
            data->mData[i] = *str++;
        }
        data->mRefCount = 1;
    }
    m_data = data;
}
#endif

#ifndef NO_BASICSTRING_IMPL
template <typename CharT, typename Allocator>
BasicString<CharT, Allocator>& BasicString<CharT, Allocator>::operator=(BasicString other)
{
    FORCE_DONT_INLINE;
    BasicStringData<CharT>* tmp = m_data;
    m_data = other.m_data;
    other.m_data = tmp;
    return *this;
}
#endif

#ifdef BASICSTRING_NO_INLINE_APPEND
template <typename CharT, typename Allocator>
BasicString<CharT, Allocator> BasicString<CharT, Allocator>::Append(const CharT* rhs) const
{
    BasicString r(*this);
    r.AppendInPlace(rhs);
    BasicStringData<CharT>* data = r.m_data;
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
#endif

template <typename CharT, typename Allocator>
template <typename OtherAllocator>
BasicString<CharT, Allocator>& BasicString<CharT, Allocator>::AppendInPlace(const BasicString<CharT, OtherAllocator>& rhs)
{
    BasicStringData<CharT>* oldData = m_data;
    BasicStringData<CharT>* data;
    if (oldData == 0)
    {
        data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
        if (data != 0)
        {
            data->mData = (CharT*)Allocator::allocate(1);
            int sz = 1;
            int j = 0;
            data->mSize = sz;
            data->mCapacity = sz;
            data->mData[0] = 0;
            data->mRefCount = sz;
            for (; j < data->mSize - 1; j++)
            {
                data->mData[j] = ((CharT*)0)[j];
            }
        }
        m_data = data;
    }
    else
    {
        if (oldData->mRefCount != 1)
        {
            data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
            if (data != 0)
            {
                data->mData = (CharT*)Allocator::allocate(oldData->mSize);
                data->mSize = oldData->mSize;
                data->mCapacity = oldData->mSize;
                for (int j = 0; j < data->mSize; j++)
                {
                    data->mData[j] = oldData->mData[j];
                }
                data->mRefCount = 1;
            }
            if (--oldData->mRefCount == 0)
            {
                if (oldData)
                {
                    if (oldData)
                    {
                        delete[] oldData->mData;
                    }
                    if (oldData)
                    {
                        nlFree(oldData);
                    }
                }
            }
            oldData = data;
        }
        m_data = oldData;
    }

    CharT* at;
    BasicStringData<CharT>* currentData = m_data;
    if (currentData != 0)
    {
        at = currentData->mData + currentData->mSize - 1;
    }
    else
    {
        at = 0;
    }

    const CharT* begin;
    BasicStringData<CharT>* rhsData = rhs.m_data;
    if (rhsData != 0)
    {
        begin = rhsData->mData;
    }
    else
    {
        begin = 0;
    }

    const CharT* end;
    if (rhsData != 0)
    {
        end = rhsData->mData + rhsData->mSize - 1;
    }
    else
    {
        end = 0;
    }

    insert(at, begin, end);
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
    if (m_data != 0)
    {
        at = m_data->mData + m_data->mSize - 1;
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
    BasicStringData<CharT>* data = r.m_data;
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
void BasicString<CharT, Allocator>::insert(CharT* at, const CharT* begin, const CharT* end)
{
    (*this)[0];
    int offset = at - (m_data ? m_data->mData : (CharT*)0);
    (*this)[0];
    (*this)[0];

    BasicStringData<CharT>* data = m_data;
    CharT* dataPtr = data ? data->mData : (CharT*)0;
    int size = end - begin;
    int insertPos = (dataPtr + offset) - data->mData;
    int newSize = data->mSize + size;

    if (data->mCapacity < newSize)
    {
        Vector<CharT> newVec;
        newVec.mData = (CharT*)Allocator::allocate(newSize * sizeof(CharT));
        newVec.mSize = newSize;
        newVec.mCapacity = newSize;
        for (int i = 0; i < newSize; i++)
        {
            newVec.mData[i] = 0;
        }
        CharT* dst = newVec.mData;
        for (int i = 0; i < data->mSize; i++)
        {
            *dst++ = data->mData[i];
        }
        CharT* oldBuf = data->mData;
        data->mData = newVec.mData;
        data->mCapacity = newVec.mCapacity;
        newVec.mData = oldBuf;
    }

    CharT* pos = data->mData + insertPos;
    CharT* t = data->mData + data->mSize - 1;
    while (t >= pos)
    {
        *(t + size) = *t;
        t--;
    }
    while (begin != end)
    {
        *pos = *begin;
        begin++;
        pos++;
    }
    data->mSize += size;
}

template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::TrimInPlace(const CharT* chars)
{
    const CharT* c;
    int i = 0;
    while (i < (int)(m_data ? m_data->mSize - 1 : 0))
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
    const CharT* eraseEnd = (m_data ? m_data->mData : (CharT*)0) + i;
    (*this)[0];
    const CharT* eraseBegin = m_data ? m_data->mData : (CharT*)0;

    {
        (*this)[0];
        BasicStringData<CharT>* data = m_data;
        int size = eraseEnd - eraseBegin;
        CharT* at = data->mData + (eraseBegin - data->mData);
        while (eraseEnd != data->mData + data->mSize)
        {
            *at = *eraseEnd;
            eraseEnd++;
            at++;
        }
        data->mSize -= size;
    }

    int last = (int)(m_data ? m_data->mSize - 1 : 0) - 1;
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
    if (m_data)
        trailEnd = m_data->mData + m_data->mSize - 1;
    else
        trailEnd = (CharT*)0;
    (*this)[0];
    const CharT* trailBegin = (m_data ? m_data->mData : (CharT*)0) + (last + 1);

    {
        (*this)[0];
        BasicStringData<CharT>* data = m_data;
        int size = trailEnd - trailBegin;
        CharT* at = data->mData + (trailBegin - data->mData);
        while (trailEnd != data->mData + data->mSize)
        {
            *at = *trailEnd;
            trailEnd++;
            at++;
        }
        data->mSize -= size;
    }
}

// Format template function for single float argument
template <typename StringType>
void Format(StringType& result, const StringType& format, const float& value)
{
    FORCE_DONT_INLINE;
}

// Format template function for two float arguments
template <typename StringType>
void Format(StringType& result, const StringType& format, const float& value1, const float& value2)
{
    FORCE_DONT_INLINE;
}

template <typename CharT, typename Allocator>
bool operator==(const BasicString<CharT, Allocator>& lhs, const char* rhs)
{
    unsigned int c;
    BasicStringData<CharT>* data = lhs.m_data;
    int i = 0;

    while (i < (data != 0 ? data->mSize - 1 : 0))
    {
        c = (u8)*rhs;
        if ((CharT)c == 0)
        {
            return false;
        }
        if ((CharT)c != (CharT)data->mData[i])
        {
            return false;
        }
        rhs++;
        i++;
    }

    return *rhs == '\0';
}

#endif
