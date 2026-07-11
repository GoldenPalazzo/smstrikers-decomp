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
struct EmptyStringTag;
} // namespace Detail

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

struct EmptyStringTag
{
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

#ifdef BASICSTRING_DELEGATING_CTOR
    BasicStringData(const CharT* text)
    {
        mData = 0;
        mSize = 0;
        mCapacity = 0;
        const CharT* scan = text;
        while (*scan++ != 0)
        {
            mSize++;
        }
        mSize++;
        mData = (CharT*)nlMalloc((mSize + 1) * sizeof(CharT), 8, true);
        mCapacity = mSize;
        for (int i = 0; i < mSize; i++)
        {
            mData[i] = *text++;
        }
        mRefCount = 1;
    }
#endif
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
#ifdef BASICSTRING_DELEGATING_CTOR
        m_data = new (8, true) BasicStringData<CharT>(str);
#else
        BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
        if (data != 0)
        {
            const CharT* src = str;
            data->mData = 0;
            data->mSize = 0;
            data->mCapacity = 0;
            const CharT* s = src;
            while (*s++ != 0)
            {
                data->mSize++;
            }
            data->mSize++;
            data->mData = (CharT*)Allocator::allocate((data->mSize + 1) * sizeof(CharT));
            data->mCapacity = data->mSize;
            for (int i = 0; i < data->mSize; i++)
            {
                data->mData[i] = *src++;
            }
            data->mRefCount = 1;
        }
        m_data = data;
#endif
    }
#endif
    ;

    BasicString(BasicStringData<CharT>* p)
        : m_data(p)
    {
    }

    BasicString(Detail::EmptyStringTag)
    {
        BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
        if (data != 0)
        {
            const CharT* str = (const CharT*)L"";
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

    BasicString(const BasicString& other)
    {
#ifdef BASICSTRING_COPY_REREAD_TEMP
        BasicStringData<CharT>* data = other.m_data;
        m_data = (data != 0) ? (data->mRefCount++, other.m_data) : 0;
#else
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
#endif
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

    CharT* begin()
    {
        (*this)[0];
        return m_data ? m_data->mData : (CharT*)0;
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
#ifdef BASICSTRING_INDEX_EMPTY_COPY_BYTE_OFFSET
                CharT* src;
                int offset;
                int j;
                j = 0;
                offset = 0;
                src = 0;
                for (; j < data->mSize - 1; j++)
                {
                    *(CharT*)((char*)data->mData + offset) = *src;
                    src++;
                    offset += sizeof(CharT);
                }
#else
                for (int j = 0; j < data->mSize - 1; j++)
                {
                    data->mData[j] = ((CharT*)0)[j];
                }
#endif
            }
            m_data = data;
        }
        else
        {
            if (oldData->mRefCount == 1)
            {
                oldData = m_data;
            }
            else
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

    void erase(const CharT* begin, const CharT* end)
#ifdef BASICSTRING_INLINE_ERASE
    {
        (*this)[0];
#ifdef BASICSTRING_ERASE_AT_FIRST
        CharT* at;
        int size = end - begin;
        const CharT* eraseEnd = end;
        BasicStringData<CharT>* data = m_data;
        at = data->mData + (begin - data->mData);
#else
        BasicStringData<CharT>* data = m_data;
        int size = end - begin;
        const CharT* eraseEnd = end;
        CharT* at = data->mData + (begin - data->mData);
#endif
        while (eraseEnd != data->mData + data->mSize)
        {
            *at = *eraseEnd;
            eraseEnd++;
            at++;
        }
        data->mSize -= size;
    }
#endif
    ;

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
    const CharT* src = str;
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
            data->mData[i] = *src++;
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
    (*this)[0];

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

    BasicStringData<CharT>* rhsData = rhs.m_data;
    const CharT* begin;
    if (rhsData != 0)
    {
        begin = rhsData->mData;
    }
    else
    {
        begin = 0;
    }

    insert(at, begin, rhsData != 0 ? rhsData->mData + rhsData->mSize - 1 : 0);
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

// TODO: 98.65% match - this/begin register swap and offset/new data register split.
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
        InitBasicStringVector(newVec, newSize);
        int i = 0;
        for (; i < data->mSize; i++)
        {
            newVec.mData[i] = data->mData[i];
        }
        newVec.mSize = data->mSize;
        int newVecSize = newVec.mSize;
        data->mSize = newVecSize;
        newVec.mSize = newVecSize;

        int oldCapacity = data->mCapacity;
        data->mCapacity = newVec.mCapacity;
        newVec.mCapacity = oldCapacity;

        CharT* oldBuf = data->mData;
        data->mData = newVec.mData;
        newVec.mData = oldBuf;
    }

    at = data->mData + insertPos;
    CharT* t = data->mData + data->mSize - 1;
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
    data->mSize += size;
}

#ifndef BASICSTRING_INLINE_ERASE
template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::erase(const CharT* begin, const CharT* end)
{
    (*this)[0];
#ifdef BASICSTRING_ERASE_AT_FIRST
    CharT* at;
    int size = end - begin;
    const CharT* eraseEnd = end;
    BasicStringData<CharT>* data = m_data;
    at = data->mData + (begin - data->mData);
#else
    BasicStringData<CharT>* data = m_data;
    int size = end - begin;
    const CharT* eraseEnd = end;
    CharT* at = data->mData + (begin - data->mData);
#endif
    while (eraseEnd != data->mData + data->mSize)
    {
        *at = *eraseEnd;
        eraseEnd++;
        at++;
    }
    data->mSize -= size;
}
#endif

// TODO: 98.84% match - scan cursor and copy-on-write temporary register swaps remain.
template <typename CharT, typename Allocator>
void BasicString<CharT, Allocator>::TrimInPlace(const CharT* chars)
{
    int i = 0;
    const CharT* c;
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
        CharT* at;
        int size = eraseEnd - eraseBegin;
        BasicStringData<CharT>* data = m_data;
        int offset = eraseBegin - data->mData;
        at = data->mData + offset;
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
        CharT* at;
        int size = trailEnd - trailBegin;
        BasicStringData<CharT>* data = m_data;
        int offset = trailBegin - data->mData;
        at = data->mData + offset;
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
