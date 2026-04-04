#ifndef _BASICSTRING_H_
#define _BASICSTRING_H_

#include "types.h"
#include "NL/nlMemory.h"
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
    static void* allocate(size_t size)
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
    BasicStringData<CharT>* m_data; // offset 0x0

    BasicString()
        : m_data(nullptr)
    {
    }

    BasicString(const CharT* str)
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
        if (m_data == 0)
        {
            BasicStringData<CharT>* data = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
            if (data != 0)
            {
                data->mData = (CharT*)Allocator::allocate(1);
                int sz = 1;
                data->mSize = sz;
                data->mCapacity = sz;
                data->mData[0] = 0;
                data->mRefCount = 1;
            }
            m_data = data;
        }
        else
        {
            if (m_data->mRefCount == 1)
            {
            }
            else
            {
                BasicStringData<CharT>* newData = (BasicStringData<CharT>*)Allocator::allocate(sizeof(BasicStringData<CharT>));
                if (newData != 0)
                {
                    newData->mData = (CharT*)Allocator::allocate(m_data->mSize);
                    newData->mSize = m_data->mSize;
                    newData->mCapacity = m_data->mSize;
                    for (int j = 0; j < newData->mSize; j++)
                    {
                        newData->mData[j] = m_data->mData[j];
                    }
                    newData->mRefCount = 1;
                }
                if (--m_data->mRefCount == 0)
                {
                    if (m_data)
                    {
                        if (m_data)
                        {
                            delete[] m_data->mData;
                        }
                        if (m_data)
                        {
                            nlFree(m_data);
                        }
                    }
                }
                m_data = newData;
            }
        }
        return m_data->mData[index];
    }

    BasicString Append(const char* rhs) const;

    template <typename OtherAllocator>
    BasicString Append(const BasicString<CharT, OtherAllocator>& rhs) const;
};

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

// Forward declaration for the operator== function
template <typename CharT, typename Allocator>
bool operator==(const BasicString<CharT, Allocator>& lhs, const char* rhs);

#ifdef NL_BASICSTRING_DEFINE
// Specialization for char with TempStringAllocator
template <>
bool operator== <char, Detail::TempStringAllocator>(const BasicString<char, Detail::TempStringAllocator>& lhs, const char* rhs)
{
    if (!rhs)
    {
        return lhs.m_data == nullptr || lhs.m_data->mSize == 0;
    }

    if (!lhs.m_data || !lhs.m_data->mData)
    {
        return *rhs == '\0';
    }

    return strcmp(lhs.m_data->mData, rhs) == 0;
}
#endif

#endif
