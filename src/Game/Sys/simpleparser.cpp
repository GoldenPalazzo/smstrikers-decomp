#include "Game/Sys/simpleparser.h"

#include "ctype.h"
#include "NL/nlString.h"

/**
 * Offset/Address/Size: 0x988 | 0x801D732C | size: 0x10
 */
SimpleParser::SimpleParser()
{
    m_EndOfLine = nullptr;
    m_CurrentBuffer = 0;
}

/**
 * Offset/Address/Size: 0x81C | 0x801D71C0 | size: 0x16C
 */
bool SimpleParser::StartParsing(char* data, int size, bool bSpacesAreWhitespace)
{
    unsigned char isWhitespace;
    char currentChar;
    char* lineStart;

    m_EndOfLine = data;
    m_AmountLeft = size;
    m_bSpacesAreWhitespace = bSpacesAreWhitespace;

    if (m_AmountLeft <= 1)
        return false;

restart:
    for (;;)
    {
        isWhitespace = 0;
        lineStart = m_EndOfLine;
        currentChar = *lineStart;
        int charValue = (s8) * (u8*)lineStart;
        if ((__ctype_map[(u8)charValue] & 0x6) != 0)
        {
            if (m_bSpacesAreWhitespace || charValue != 0x20)
                isWhitespace = 1;
        }

        if (!isWhitespace)
            break;

        unsigned char canAdvance;
        if (m_AmountLeft <= 1)
        {
            canAdvance = 0;
        }
        else
        {
            canAdvance = 1;
            m_EndOfLine += 1;
            m_AmountLeft -= 1;
        }

        if (canAdvance == 0)
        {
            return false;
        }
    }

    if ((int)(signed char)currentChar == 0x23)
    {
        for (;;)
        {
            unsigned char hasAdvanced;
            if (m_AmountLeft <= 1)
            {
                hasAdvanced = 0;
            }
            else
            {
                hasAdvanced = 1;
                m_EndOfLine += 1;
                m_AmountLeft -= 1;
            }

            unsigned char foundNewline;
            if (hasAdvanced == 0)
            {
                foundNewline = 0;
            }
            else
            {
                if (*m_EndOfLine == '\n')
                {
                    foundNewline = 1;
                }
                else
                {
                    continue;
                }
            }

            if (foundNewline == 0)
                return false;

            goto restart; // I am sure this was not in the original code, but it is here to make the assembly match.
        }
    }

    m_CurrPos = lineStart;

    for (;;)
    {
        unsigned char canStep;
        if (m_AmountLeft <= 1)
        {
            canStep = 0;
        }
        else
        {
            canStep = 1;
            m_EndOfLine += 1;
            m_AmountLeft -= 1;
        }

        if (!canStep)
            break;

        if (*m_EndOfLine == '\n')
            break;
    }

    return true;
}

static inline void AdvanceBuffer(int& currentBuffer)
{
    currentBuffer = (currentBuffer + 1) % 5;
}

/**
 * Offset/Address/Size: 0x33C | 0x801D6CE0 | size: 0x4E0
 */
char* SimpleParser::NextToken(bool bToLower)
{
    char* result;
    char* currentToken;

    if (m_CurrPos == this->m_EndOfLine)
    {
        currentToken = NULL;
    }
    else
    {
        for (;;)
        {
            unsigned char isWhitespaceChar = 0;
            char* tokenStart = m_CurrPos;
            char currentChar = *tokenStart;
            int charAsciiValue = (s8) * (u8*)tokenStart;
            if ((__ctype_map[(u8)charAsciiValue] & 0x6) != 0)
            {
                if (m_bSpacesAreWhitespace || charAsciiValue != 0x20)
                    isWhitespaceChar = 1;
            }

            if (!isWhitespaceChar)
                break;

            unsigned char canAdvancePosition;
            // if (m_CurrPos == m_EndOfLine)
            if (tokenStart == this->m_EndOfLine)
            {
                canAdvancePosition = 0;
            }
            else
            {
                canAdvancePosition = 1;
                m_CurrPos += 1;
            }

            if (canAdvancePosition == 0)
            {
                currentToken = NULL;
                goto skip_1;
            }
        }
        int bufferIndex = 0;
        for (;;)
        {
            if (bToLower != 0)
            {
                m_TokenBuffer[m_CurrentBuffer][bufferIndex] = nlToLower<char>(*m_CurrPos);
            }
            else
            {
                m_TokenBuffer[m_CurrentBuffer][bufferIndex] = *m_CurrPos;
            }

            // char* temp_r3 = m_CurrPos;
            bufferIndex++;

            unsigned char canAdvancePosition;
            if (m_CurrPos == m_EndOfLine)
            {
                canAdvancePosition = 0;
            }
            else
            {
                canAdvancePosition = 1;
                m_CurrPos += 1;
            }

            if (canAdvancePosition == 0)
            {
                break;
            }

            unsigned char isNextCharWhitespace = 0;
            int nextCharAsciiValue = (s8) * (u8*)m_CurrPos;

            if ((__ctype_map[(u8)nextCharAsciiValue] & 0x6) != 0)
            {
                if (m_bSpacesAreWhitespace || nextCharAsciiValue != 0x20)
                    isNextCharWhitespace = 1;
            }

            if (isNextCharWhitespace != 0)
            {
                break;
            }
        }

        m_TokenBuffer[m_CurrentBuffer][bufferIndex] = 0;
        int nPrevBuffer = m_CurrentBuffer;
        m_CurrentBuffer = (nPrevBuffer + 1) % 5;
        currentToken = m_TokenBuffer[nPrevBuffer];
    }

skip_1:
    result = currentToken;

    if (currentToken == NULL)
    {
        unsigned char isWhitespace;
        char currentChar;
        char* lineStart;

        unsigned char shouldExitLoop;
        if (m_AmountLeft <= 1)
        {
            shouldExitLoop = 0;
        }
        else
        {
        restart:
            for (;;)
            {
                isWhitespace = 0;
                lineStart = m_EndOfLine;
                currentChar = *lineStart;
                int charValue = (s8) * (u8*)lineStart;
                if ((__ctype_map[(u8)charValue] & 0x6) != 0)
                {
                    if (m_bSpacesAreWhitespace || charValue != 0x20)
                        isWhitespace = 1;
                }

                if (!isWhitespace)
                    break;

                unsigned char canAdvance;
                if (m_AmountLeft <= 1)
                {
                    canAdvance = 0;
                }
                else
                {
                    canAdvance = 1;
                    m_EndOfLine += 1;
                    m_AmountLeft -= 1;
                }

                if (canAdvance == 0)
                {
                    shouldExitLoop = 0;
                    goto skip_2;
                }
            }

            if ((int)(signed char)currentChar == 0x23)
            {
                for (;;)
                {
                    unsigned char hasAdvanced;
                    if (m_AmountLeft <= 1)
                    {
                        hasAdvanced = 0;
                    }
                    else
                    {
                        hasAdvanced = 1;
                        m_EndOfLine += 1;
                        m_AmountLeft -= 1;
                    }

                    unsigned char foundNewline;
                    if (hasAdvanced == 0)
                    {
                        foundNewline = 0;
                    }
                    else
                    {
                        if (*m_EndOfLine == '\n')
                        {
                            foundNewline = 1;
                        }
                        else
                        {
                            continue;
                        }
                    }

                    if (foundNewline == 0)
                    {
                        shouldExitLoop = 0;
                        goto skip_2;
                    }

                    goto restart; // I am sure this was not in the original code, but it is here to make the assembly match.
                }
            }
            else
            {
                m_CurrPos = lineStart;
                unsigned char canAdvance;
                do
                {
                    if (m_AmountLeft <= 1)
                    {
                        canAdvance = 0;
                    }
                    else
                    {
                        canAdvance = 1;
                        m_EndOfLine += 1;
                        m_AmountLeft -= 1;
                    }
                } while ((canAdvance != 0) && (*m_EndOfLine != '\n'));
                shouldExitLoop = 1;
            }
        }
    skip_2:
        if (shouldExitLoop == 0)
        {
            return NULL;
        }

        char* nextToken;

        if (m_CurrPos == m_EndOfLine)
        {
            nextToken = NULL;
        }
        else
        {
            unsigned char isWhitespaceChar;
            char currentChar;
            char* tokenStart;

            for (;;)
            {
                isWhitespaceChar = 0;
                tokenStart = m_CurrPos;
                currentChar = *tokenStart;
                int charAsciiValue = (s8) * (u8*)tokenStart;
                if ((__ctype_map[(u8)charAsciiValue] & 0x6) != 0)
                {
                    if (m_bSpacesAreWhitespace || charAsciiValue != 0x20)
                        isWhitespaceChar = 1;
                }

                if (!isWhitespaceChar)
                    break;

                unsigned char canAdvancePosition;
                if (tokenStart == this->m_EndOfLine)
                {
                    canAdvancePosition = 0;
                }
                else
                {
                    canAdvancePosition = 1;
                    m_CurrPos += 1;
                }

                if (canAdvancePosition == 0)
                {
                    nextToken = NULL;
                    goto skip_3;
                }
            }
            u32 tokenLength = 0;
            for (;;)
            {
                if (bToLower != 0)
                {
                    m_TokenBuffer[m_CurrentBuffer][tokenLength] = nlToLower<char>(*m_CurrPos);
                }
                else
                {
                    m_TokenBuffer[m_CurrentBuffer][tokenLength] = *m_CurrPos;
                }

                tokenLength += 1;

                unsigned char canAdvancePosition;
                if (m_CurrPos == m_EndOfLine)
                {
                    canAdvancePosition = 0;
                }
                else
                {
                    canAdvancePosition = 1;
                    m_CurrPos = m_CurrPos + 1;
                }

                if (canAdvancePosition == 0)
                {
                    break;
                }

                unsigned char isNextCharWhitespace = 0;
                int nextCharAsciiValue = (s8) * (u8*)m_CurrPos;

                if ((__ctype_map[(u8)nextCharAsciiValue] & 0x6) != 0)
                {
                    if (m_bSpacesAreWhitespace || nextCharAsciiValue != 0x20)
                        isNextCharWhitespace = 1;
                }

                if (isNextCharWhitespace != 0)
                {
                    break;
                }
            }

            m_TokenBuffer[m_CurrentBuffer][tokenLength] = 0;
            int nPrevBuffer2 = m_CurrentBuffer;
            m_CurrentBuffer = (nPrevBuffer2 + 1) % 5;
            nextToken = m_TokenBuffer[nPrevBuffer2];
        }
    skip_3:
        result = nextToken;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x160 | 0x801D6B04 | size: 0x1DC
 */
char* SimpleParser::NextTokenOnLine(bool bToLower)
{
    unsigned char isWhitespaceChar;
    char currentChar;
    char* tokenStart;

    unsigned int tokenLength; // r30
    int nPrevBuffer;          // r0

    if (m_CurrPos == m_EndOfLine)
    {
        return NULL;
    }

    // Skip leading whitespace
    for (;;)
    {
        isWhitespaceChar = 0;
        tokenStart = m_CurrPos;
        currentChar = *tokenStart;
        int charAsciiValue = (s8) * (u8*)tokenStart;
        if ((__ctype_map[(u8)charAsciiValue] & 0x6) != 0)
        {
            if (m_bSpacesAreWhitespace || charAsciiValue != 0x20)
                isWhitespaceChar = 1;
        }

        if (!isWhitespaceChar)
            break;

        unsigned char canAdvancePosition;
        if (tokenStart == this->m_EndOfLine)
        {
            canAdvancePosition = 0;
        }
        else
        {
            canAdvancePosition = 1;
            m_CurrPos += 1;
        }

        if (canAdvancePosition == 0)
        {
            return NULL;
        }
    }

    // Extract token characters
    tokenLength = 0;
    for (;;)
    {
        if (bToLower != 0)
        {
            m_TokenBuffer[m_CurrentBuffer][tokenLength] = nlToLower<char>(*m_CurrPos);
        }
        else
        {
            m_TokenBuffer[m_CurrentBuffer][tokenLength] = *m_CurrPos;
        }

        tokenLength += 1;

        unsigned char canAdvancePosition;
        if (m_CurrPos == m_EndOfLine)
        {
            canAdvancePosition = 0;
        }
        else
        {
            canAdvancePosition = 1;
            m_CurrPos = m_CurrPos + 1;
        }

        if (canAdvancePosition == 0)
        {
            break;
        }

        unsigned char isNextCharWhitespace = 0;
        int nextCharAsciiValue = (s8) * (u8*)m_CurrPos;

        if ((__ctype_map[(u8)nextCharAsciiValue] & 0x6) != 0)
        {
            if (m_bSpacesAreWhitespace || nextCharAsciiValue != 0x20)
                isNextCharWhitespace = 1;
        }

        if (isNextCharWhitespace != 0)
        {
            break;
        }
    }

    m_TokenBuffer[m_CurrentBuffer][tokenLength] = 0;
    nPrevBuffer = m_CurrentBuffer;
    m_CurrentBuffer = (nPrevBuffer + 1) % 5;
    return m_TokenBuffer[nPrevBuffer];
}

/**
 * Offset/Address/Size: 0x0 | 0x801D69A4 | size: 0x160
 */
bool SimpleParser::AdvanceLine()
{
    unsigned char skip;
    unsigned char step;
    char cur;
    char* p;

    if (m_AmountLeft <= 1)
        return false;

    unsigned char* ctype = __ctype_map;

restart:
    for (;;)
    {
        skip = 0;
        p = m_EndOfLine;
        cur = *p;

        int ch = (signed char)(*(unsigned char*)p);

        if ((ctype[(unsigned char)ch] & 0x06) != 0)
        {
            if (m_bSpacesAreWhitespace || ch != 0x20)
                skip = 1;
        }

        if (!skip)
            break;

        if (m_AmountLeft <= 1)
        {
            step = 0;
        }
        else
        {
            step = 1;
            m_EndOfLine += 1;
            m_AmountLeft -= 1;
        }

        if (step == 0)
            return false;
    }

    if ((int)(signed char)cur == 0x23)
    {
        for (;;)
        {
            unsigned char hasAdvanced;
            if (m_AmountLeft <= 1)
            {
                hasAdvanced = 0;
            }
            else
            {
                hasAdvanced = 1;
                m_EndOfLine += 1;
                m_AmountLeft -= 1;
            }

            unsigned char foundNewline;
            if (hasAdvanced == 0)
            {
                foundNewline = 0;
            }
            else
            {
                if (*m_EndOfLine == '\n')
                {
                    foundNewline = 1;
                }
                else
                {
                    continue;
                }
            }

            if (foundNewline == 0)
                return false;

            goto restart; // I am sure this was not in the original code, but it is here to make the assembly match.
        }
    }

    m_CurrPos = p;

    for (;;)
    {
        if (m_AmountLeft <= 1)
        {
            step = 0;
        }
        else
        {
            step = 1;
            m_EndOfLine += 1;
            m_AmountLeft -= 1;
        }

        if (!step)
            break;

        if (*m_EndOfLine == '\n')
            break;
    }

    return true;
}
