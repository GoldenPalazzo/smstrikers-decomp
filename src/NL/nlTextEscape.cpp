#include "NL/nlTextEscape.h"

#include "NL/nlQSort.h"
#include "NL/nlBSearch.h"

extern "C" unsigned long wcstoul(const unsigned short* str, unsigned short** end, int base);

/**
 * Offset/Address/Size: 0x0 | 0x8021251C | size: 0xB0
 */
nlColour nlEscapeSequence::GetExtendedColour()
{
    unsigned short str[3] = { 0, 0, 0 };
    nlColour colour;

    if ((int)m_Extended[0] == 0x70)
    {
        colour.c[3] = 0;
        return colour;
    }

    int channel = 0;
    for (channel = 0; channel < 3; channel++)
    {
        str[0] = m_Extended[channel * 2];
        str[1] = m_Extended[channel * 2 + 1];
        colour.c[channel] = (u8)wcstoul(str, 0, 16);
    }
    colour.c[3] = 0xFF;
    return colour;
}

/**
 * Offset/Address/Size: 0xB0 | 0x802125CC | size: 0x174
 */
nlEscapeSequence::nlEscapeSequence(const unsigned short* str)
{
    char Seq[4] = { ' ', ' ', ' ', ' ' };
    const unsigned short* ExtendedStart = 0;
    const unsigned short* p = str;
    char* pSeq = Seq;
    unsigned long Char = 0;
    unsigned long key;
    ESCAPE_LOOKUP* pEscape;
    ESCAPE_TYPE type;

    while (true)
    {
        unsigned long ch = p[1];

        if (ch == '}')
        {
            break;
        }

        if (ch == ':')
        {
            const unsigned short* pExtended = str + Char;
            ExtendedStart = pExtended + 2;
            break;
        }

        if (Char < 4)
        {
            *pSeq = (char)ch;
        }

        p++;
        pSeq++;
        Char++;
    }

    if (ExtendedStart != 0)
    {
        Char = 0;
        while (Char < 15)
        {
            unsigned long ch = *ExtendedStart;
            if (ch == '}')
            {
                break;
            }

            m_Extended[Char] = (unsigned short)ch;
            ExtendedStart++;
            Char++;
        }
    }

    m_Extended[Char] = 0;
    m_pEnd = (ExtendedStart ? ExtendedStart : str + 1) + Char + 1;

    key = *(unsigned long*)Seq;
    pEscape = nlBSearch(key, s_EscapeLookup, ESC_COUNT);
    if (pEscape != 0)
    {
        type = pEscape->type;
    }
    else
    {
        type = ESC_UNKNOWN;
    }
    m_Type = type;
}

// /**
//  * Offset/Address/Size: 0x0 | 0x80212740 | size: 0x8C
//  */
// template <typename T, typename U>
// T* nlBSearch(const U& key, T* array, int size)
// {
// }

// /**
//  * Offset/Address/Size: 0x124 | 0x80212864 | size: 0x28
//  */
// template <typename T>
// void nlQSort(T* array, int size, int (*compare)(const T*, const T*))
// {
// }

// /**
//  * Offset/Address/Size: 0x14C | 0x8021288C | size: 0x2C
//  */
// template <typename T>
// int nlDefaultQSortComparer(const T* a, const T* b)
// {
// }

/**
 * nlBSearch<nlEscapeSequence::ESCAPE_LOOKUP, unsigned long>(const unsigned long&, nlEscapeSequence::ESCAPE_LOOKUP*, int)
 * nlQSort<nlEscapeSequence::ESCAPE_LOOKUP>(nlEscapeSequence::ESCAPE_LOOKUP*, int, int (*)(const nlEscapeSequence::ESCAPE_LOOKUP*, const nlEscapeSequence::ESCAPE_LOOKUP*))
 * nlDefaultQSortComparer<nlEscapeSequence::ESCAPE_LOOKUP>(const nlEscapeSequence::ESCAPE_LOOKUP*, const nlEscapeSequence::ESCAPE_LOOKUP*)
 */
void nlTextEscape_stub()
{
    nlQSort<nlEscapeSequence::ESCAPE_LOOKUP>((nlEscapeSequence::ESCAPE_LOOKUP*)0, 0, &nlDefaultQSortComparer<nlEscapeSequence::ESCAPE_LOOKUP>);
}
