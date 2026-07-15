#ifndef _SIMMER_H_
#define _SIMMER_H_

#include "NL/nlBasicString.h"

struct StatsPair
{
    /* 0x000 */ float mMean;
    /* 0x004 */ float mStandardDeviation;
}; // total size: 0x8

class Simulator
{
public:
    Simulator();
    void InitializeStats();

    /* 0x000 */ StatsPair mStatistics[23];
};

template <typename StringType>
class Tokenizer
{
public:
    StringType m_source;
    StringType m_delimiter;

    class iterator
    {
    public:
        const Tokenizer* mTokenizer;
        const char* mIter;
        const char* mEnd;
        StringType mToken;

        iterator(const Tokenizer& tokenizer, const char* endPtr);
#ifdef BASICSTRING_SELECTIVE_NO_COPY_REREAD
        iterator(const iterator& other)
            : mTokenizer(other.mTokenizer)
            , mIter(other.mIter)
            , mEnd(other.mEnd)
            , mToken(other.mToken, false)
        {
        }
#endif
        iterator& operator++();
        void FindNextToken();

        bool operator!=(const iterator& other) const
        {
            return mIter != other.mIter;
        }
    };

    Tokenizer(const StringType& source, const StringType& delimiter)
#ifdef BASICSTRING_SELECTIVE_NO_COPY_REREAD
        : m_delimiter(delimiter, true)
        , m_source(source, false)
#else
        : m_source(source)
        , m_delimiter(delimiter)
#endif
    {
    }

    iterator begin() const;
    iterator end() const;
};

#endif // _SIMMER_H_
