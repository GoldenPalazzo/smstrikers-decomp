#include "NL/nlTextBox.h"
#include "NL/nlColour.h"
#include "NL/nlTextEscape.h"
#include "NL/nlString.h"

/**
 * Offset/Address/Size: 0x0 | 0x80211F28 | size: 0x244
 * TODO: 95.10% match - remaining register diffs around DrawInfo, row metadata,
 * pString, and font/matrix locals.
 */
void nlTextBox::DrawString(const nlTextBox::StringDrawInfo& DrawInfo, const nlVector2& DrawAt, const nlColour& Color, eGLView View)
{
    const nlFont* pFont = DrawInfo.pFont;
    int yDir = 1;
    nlVector2 CurrentPos = DrawAt;
    if (DrawInfo.DrawOptions & 0x800)
    {
        yDir = -1;
    }
    float yWithOffset = CurrentPos.f.y + (float)(yDir * DrawInfo.YOffset);
    CurrentPos.f.y = yWithOffset;

    int ascentAdj;
    if (DrawInfo.DrawOptions & 0x200)
    {
        ascentAdj = pFont->m_Metrics.InternalLeading;
    }
    else
    {
        ascentAdj = 0;
    }

    const nlMatrix4* pMatrix = DrawInfo.pMatrix;
    char* pIter = (char*)&DrawInfo;
    int vertOffset = yDir * (int)pFont->m_Metrics.Ascent - ascentAdj;
    nlColour overridecolour;
    overridecolour = Color;
    unsigned long hMatrix;
    bool flipY = (DrawInfo.DrawOptions & 0x800) != 0;
    overridecolour.c[3] = 0;
    float startX = DrawAt.f.x;

    unsigned long row = 0;
    const unsigned short* pString = DrawInfo.String;
    unsigned short rowCount = DrawInfo.RowCount;
    CurrentPos.f.y = yWithOffset + (float)vertOffset;

    while (row < rowCount)
    {
        const Row& CurrentRow = *(Row*)(pIter + 0x14);
        CurrentPos.f.x = startX + (float)CurrentRow.XOffset;

        if (pMatrix)
        {
            unsigned long h = glAllocMatrix();
            if (h + 0x10000 != 0xFFFF)
            {
                glSetMatrix(h, *pMatrix);
            }
            hMatrix = h;
        }

        unsigned short startIdx = CurrentRow.FirstChar;
        unsigned long* matArg = pMatrix != 0 ? &hMatrix : 0;
        {
            FontCharString fontCharStr;
            fontCharStr.m_InternalBuffer = 0;
            fontCharStr.m_pString = (unsigned short*)(pString + startIdx);

            int length = (&CurrentRow + 1)->FirstChar - startIdx;

            DrawInfo.pFont->DrawString(View, fontCharStr, CurrentPos, Color, Color, length, nlFont::PASS_TextAndEffect, flipY, matArg, &overridecolour);
        }

        if (overridecolour.c[3] == 0)
        {
            overridecolour = Color;
        }

        pIter += 4;
        row++;
        CurrentPos.f.y += (float)(yDir * pFont->m_Metrics.Height);
    }
}

/**
 * Offset/Address/Size: 0x244 | 0x8021216C | size: 0x3B0
 * TODO: 96.93% match - remaining blocker: entry setup/store scheduling around
 * DrawInfo initialization.
 */
void nlTextBox::ProcessString(const FontCharString* pString, const nlFont* pFont, const nlVector2& BoxSize, unsigned long DrawOptions, const nlMatrix4* pMatrix, nlTextBox::StringDrawInfo& DrawInfo)
{
    DrawInfo.pFont = pFont;
    const unsigned short* str = pString->m_pString;
    DrawInfo.DrawOptions = DrawOptions;
    DrawInfo.String = str;
    DrawInfo.pMatrix = pMatrix;
    DrawInfo.RowCount = 0;

    const unsigned short* pCurrentChar = str;

    unsigned short escapeBegin = nlEscapeSequence::ESCAPE_BEGIN;
    unsigned long wordWrapDisabled = DrawOptions & 0x400;
    unsigned long alignment = DrawOptions & 0x3;
    unsigned long centerAlign = DrawOptions & 0x1;
    unsigned long CurrentRowWidth = 0;
    float maxWidth = BoxSize.f.x;
    const unsigned short* pLastSpace = 0;
    const unsigned short* pLastNonEsc = 0;
    unsigned long WidthAtLastSpace = 0;
    unsigned char FirstChar = 1;
    unsigned char IsNewParagraph = 0;
    unsigned long CharWidth;

    while (*pCurrentChar != 0)
    {
        if (*pCurrentChar == escapeBegin)
        {
            nlEscapeSequence esc(pCurrentChar);

            if (esc.m_Type == ESC_NON_BREAKING_SPACE)
            {
                unsigned long prevChar = FirstChar ? 0 : (pLastNonEsc != 0 ? (unsigned long)*pLastNonEsc : 0);
                CharWidth = pFont->GetCharWidth(' ', (unsigned short)prevChar);
            }
            else if (esc.m_Type == ESC_PARAGRAPH)
            {
                CharWidth = (unsigned long)(0.5f + BoxSize.f.x);
                WidthAtLastSpace = CurrentRowWidth;
                IsNewParagraph = 1;
                pLastSpace = esc.m_pEnd - 1;
            }
            else
            {
                CharWidth = 0;
            }
            pCurrentChar = esc.m_pEnd - 1;
        }
        else
        {
            unsigned long prevChar = FirstChar ? 0 : (pLastNonEsc != 0 ? (unsigned long)*pLastNonEsc : 0);
            CharWidth = pFont->GetCharWidth(*pCurrentChar, (unsigned short)prevChar);
            pLastNonEsc = pCurrentChar;
            if (*pCurrentChar == ' ')
            {
                pLastSpace = pCurrentChar;
                WidthAtLastSpace = CurrentRowWidth;
            }
        }

        FirstChar = 0;

        if ((float)(CurrentRowWidth + CharWidth) > maxWidth)
        {
            if (!wordWrapDisabled && pLastSpace != 0)
            {
                CurrentRowWidth = WidthAtLastSpace;
                pCurrentChar = pLastSpace + 1;
                pLastSpace = 0;
                WidthAtLastSpace = 0;
            }

            int xOffset;
            if (alignment)
            {
                int remaining = (int)(maxWidth - (float)CurrentRowWidth);
                xOffset = remaining >> (centerAlign ? 1u : 0u);
            }
            else
            {
                xOffset = 0;
            }

            DrawInfo.Rows[DrawInfo.RowCount].XOffset = xOffset;
            unsigned short charIdx = (unsigned short)(pCurrentChar - pString->m_pString);
            DrawInfo.RowCount++;
            DrawInfo.Rows[DrawInfo.RowCount].FirstChar = charIdx;

            if (*pCurrentChar != ' ')
            {
                pCurrentChar--;
            }
            if (*pCurrentChar == ' ')
            {
                CharWidth = 0;
            }
            CurrentRowWidth = 0;
        }

        CurrentRowWidth += CharWidth;

        if (IsNewParagraph)
        {
            CurrentRowWidth = 0;
            IsNewParagraph = 0;
        }

        pCurrentChar++;
    }

    // Final row
    int xOffset;
    if (alignment)
    {
        int remaining = (int)(BoxSize.f.x - (float)CurrentRowWidth);
        xOffset = remaining >> (centerAlign ? 1u : 0u);
    }
    else
    {
        xOffset = 0;
    }

    DrawInfo.Rows[DrawInfo.RowCount].XOffset = xOffset;
    DrawInfo.RowCount++;

    s32 strLen = nlStrLen(pString->m_pString);
    DrawInfo.Rows[DrawInfo.RowCount].FirstChar = (unsigned short)strLen;
    DrawInfo.Rows[0].FirstChar = 0;

    if (DrawOptions & 0x30)
    {
        int totalHeight = (int)DrawInfo.RowCount * (int)pFont->m_Metrics.Height;
        if ((float)totalHeight > BoxSize.f.y)
        {
            DrawInfo.YOffset = 0;
        }
        else if (DrawOptions & 0x10)
        {
            DrawInfo.YOffset = (signed short)((int)(BoxSize.f.y / 2.0f) - (totalHeight >> 1));
        }
        else
        {
            DrawInfo.YOffset = (signed short)((int)BoxSize.f.y - totalHeight);
        }
    }
    else
    {
        DrawInfo.YOffset = 0;
    }
}
