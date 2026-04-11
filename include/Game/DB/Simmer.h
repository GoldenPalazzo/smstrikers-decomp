#ifndef _SIMMER_H_
#define _SIMMER_H_

#include "NL/nlBasicString.h"

class Simulator
{
public:
    Simulator();
    void InitializeStats();
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
        u32 m_field0;
        u32 m_field1;
        u32 m_field2;
        StringType m_token;

        iterator& operator++();

        bool operator!=(const iterator& other) const
        {
            return m_field1 != other.m_field1;
        }
    };

    Tokenizer(const StringType& source, const StringType& delimiter)
        : m_source(source)
        , m_delimiter(delimiter)
    {
    }

    iterator begin() const;
    iterator end() const;
};

#endif // _SIMMER_H_
