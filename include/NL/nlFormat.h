#ifndef _NLFORMAT_H_
#define _NLFORMAT_H_

#include "NL/nlBasicString.h"

template <typename StringType>
class FormatImpl
{
public:
    StringType mString;
    int mCurrentPos;
    FormatImpl()
        : mCurrentPos(0)
    {
    }

    FormatImpl(BasicStringData<char>* data)
        : mString(data)
        , mCurrentPos(0)
    {
    }

    operator StringType() const
    {
        return mString;
    }

    template <typename T>
    FormatImpl& operator%(const T& t);
};

struct FormatImplLayoutCharTemp
{
    BasicString<char, Detail::TempStringAllocator> mString;
    int mCurrentPos;
};

struct FormatImplLayoutWideTemp
{
    BasicString<unsigned short, Detail::TempStringAllocator> mString;
    int mCurrentPos;

    FormatImplLayoutWideTemp(BasicStringData<unsigned short>* data)
        : mString(data)
        , mCurrentPos(0)
    {
    }
};

template <typename StringType, typename T1, typename T2>
inline StringType Format(const StringType& format, const T1& value1, const T2& value2);

/**
 * Offset/Address/Size: 0xFB4 | 0x800CD994 | size: 0x118
 * TODO: 98.43% match - return copy path still stores null/data via r4 instead of stack reload into r0.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator> Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[32])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % (const unsigned short*)value));
}

/**
 * Offset/Address/Size: 0xE10 | 0x800D6924 | size: 0x124
 * TODO: 98.49% match - result copy still follows the r4/null store path instead of
 * the target's temporary-slot reload path before final assignment.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[32], unsigned short[32]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value1)[32],
    const unsigned short (&value2)[32])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % (const unsigned short*)value1) % (const unsigned short*)value2));
}

/**
 * Offset/Address/Size: 0xF40 | 0x8010473C | size: 0x12C
 * TODO: 97.67% match - store scheduling (li r0,0 before/after stw r4) and
 * return copy register choice (r0 reload vs r4 reuse) still differ.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>,
    unsigned short[32],
    BasicString<unsigned short, Detail::TempStringAllocator> >(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value1)[32],
    const BasicString<unsigned short, Detail::TempStringAllocator>& value2)
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    BasicString<unsigned short, Detail::TempStringAllocator> implString(data);
    ((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)implString)
        .mCurrentPos = 0;

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)implString) % (const unsigned short*)value1) % value2));
}

/**
 * Offset/Address/Size: 0x3DCC | 0x800F5D94 | size: 0x128
 * TODO: 98.51% match - return copy path still reuses r4 for the null/result
 * store instead of reloading the temporary slot before final assignment.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, const unsigned short*>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short* const& value1,
    const unsigned short* const& value2)
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % value1) % value2));
}

/**
 * Offset/Address/Size: 0xE30 | 0x800F4F7C | size: 0x128
 * TODO: 98.51% match - return copy path still reuses r4 for the null/result
 * store instead of reloading the temporary slot before final assignment.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>,
    BasicString<unsigned short, Detail::TempStringAllocator>,
    BasicString<unsigned short, Detail::TempStringAllocator> >(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const BasicString<unsigned short, Detail::TempStringAllocator>& value1,
    const BasicString<unsigned short, Detail::TempStringAllocator>& value2)
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % value1) % value2));
}

/**
 * Offset/Address/Size: 0xE08 | 0x800A9794 | size: 0x118
 * TODO: 98.43% match - return copy path still stores through r4 instead of reloading the temporary slot before final store.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator> Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[16]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[16])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % value));
}

/**
 * Offset/Address/Size: 0x0 | 0x800D5B14 | size: 0x120
 * TODO: 98.47% match - result copy path still stores through r4 and misses the temporary-slot reload into r0 before final write.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator>
Format<BasicString<unsigned short, Detail::TempStringAllocator>, const unsigned short*, unsigned short[16]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short* const& value1,
    const unsigned short (&value2)[16])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)((((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % value1) % value2));
}

template <>
inline BasicString<unsigned short, Detail::TempStringAllocator> Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[8]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[8])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % (const unsigned short*)value));
}

/**
 * Offset/Address/Size: 0x0 | 0x800A898C | size: 0x118
 * TODO: 67.36% match - return/null path store order and temporary cleanup
 * sequencing still differ, with an extra trailing helper block emitted.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator> Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[2]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[2])
{
    struct FormatImplLayoutWideTempU2
    {
        BasicStringData<unsigned short>* m_data;
        int mCurrentPos;
    };

    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTempU2 impl;
    impl.m_data = data;
    impl.mCurrentPos = 0;

    return BasicString<unsigned short, Detail::TempStringAllocator>(
        (BasicString<unsigned short, Detail::TempStringAllocator>)(((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % (const unsigned short*)value));
}
/**
 * Offset/Address/Size: 0xF68 | 0x8009DFA4 | size: 0x118
 * TODO: 9.16% match - direct return path still bypasses the expected temporary copy/refcount block.
 */
template <>
inline BasicString<unsigned short, Detail::TempStringAllocator> Format<BasicString<unsigned short, Detail::TempStringAllocator>, unsigned short[4]>(
    const BasicString<unsigned short, Detail::TempStringAllocator>& format,
    const unsigned short (&value)[4])
{
    BasicStringData<unsigned short>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutWideTemp impl(data);
    return (BasicString<unsigned short, Detail::TempStringAllocator>)(((FormatImpl<BasicString<unsigned short, Detail::TempStringAllocator> >&)impl) % (const unsigned short*)value);
}
/**
 * Offset/Address/Size: 0x0 | 0x80060960 | size: 0x114
 * TODO: 61.38% match - extra helper ctor/dtor wrapper calls and stack frame size mismatch remain.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, int>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const int& value)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutCharTemp impl;
    impl.mString.m_data = data;
    impl.mCurrentPos = 0;

    ((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)impl) % value;

    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)impl));
}

/**
 * Offset/Address/Size: 0x0 | 0x801935F8 | size: 0x128
 * TODO: 98.51% match - return copy/null handling still stores through r4
 * instead of reloading the temporary slot before the final store.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, int, int>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const int& value1,
    const int& value2)
{
    struct FormatImplLayoutCharTempI2
    {
        BasicString<char, Detail::TempStringAllocator> mString;
        int mCurrentPos;

        FormatImplLayoutCharTempI2(BasicStringData<char>* data)
            : mString(data)
            , mCurrentPos(0)
        {
        }
    };

    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutCharTempI2 impl(data);

    return BasicString<char, Detail::TempStringAllocator>(
        (const BasicString<char, Detail::TempStringAllocator>&)(BasicString<char, Detail::TempStringAllocator>)((((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)impl) % value1) % value2));
}
/**
 * Offset/Address/Size: 0xEB0 | 0x80068108 | size: 0x114
 * TODO: 95.52% match - format data is kept in r6 with an extra zero-store before impl.mString assignment.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, char>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const char& value)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImplLayoutCharTemp impl;
    impl.mString.m_data = data;
    impl.mCurrentPos = 0;

    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)(((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)impl) % value));
}
/**
 * Offset/Address/Size: 0x0 | 0x8002FED4 | size: 0x114
 * TODO: 92.39% match - an extra early zero-initialization store remains for impl.mString,
 * shifting branch targets and a null-path move/store register pair.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, const char*>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const char* const& value)
{
    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl;
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    impl.mString.m_data = data;
    impl.mCurrentPos = 0;
    impl % value;

    return BasicString<char, Detail::TempStringAllocator>((BasicString<char, Detail::TempStringAllocator>)impl);
}
/**
 * Offset/Address/Size: 0x2BC0 | 0x80069E18 | size: 0x114
 * TODO: 98.41% match - return copy path still uses r4 for null handling and
 * direct store, rather than reloading/storing through the expected temporary slot.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, float>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const float& value)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }

    FormatImpl<BasicString<char, Detail::TempStringAllocator> > impl(data);

    return BasicString<char, Detail::TempStringAllocator>((BasicString<char, Detail::TempStringAllocator>)(impl % value));
}

/**
 * Offset/Address/Size: 0x3DC | 0x800C3820 | size: 0x114
 * TODO: 97.46% match - store scheduling (li r0,0 before/after stw r4) and
 * copy ctor register convergence (r0 reload vs r4 reuse) differ from target.
 */
template <>
inline BasicString<char, Detail::TempStringAllocator> Format<BasicString<char, Detail::TempStringAllocator>, unsigned long>(
    const BasicString<char, Detail::TempStringAllocator>& format,
    const unsigned long& value)
{
    BasicStringData<char>* data = format.m_data;
    if (data != 0)
    {
        data->mRefCount++;
    }
    else
    {
        data = 0;
    }
    BasicString<char, Detail::TempStringAllocator> implString(data);
    ((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)implString).mCurrentPos = 0;
    return BasicString<char, Detail::TempStringAllocator>(
        (BasicString<char, Detail::TempStringAllocator>)(((FormatImpl<BasicString<char, Detail::TempStringAllocator> >&)implString) % value));
}

#endif // _NLFORMAT_H_
