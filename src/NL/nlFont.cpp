#include "NL/nlFont.h"
#include "NL/nlBSearch.h"
#include "NL/nlColour.h"
#include "NL/nlList.h"
#include "NL/nlQSort.h"
#include "NL/nlSlotPoolHigh.h"
#include "NL/nlString.h"
#include "NL/nlTextBox.h"
#include "NL/nlTextEscape.h"
#include "NL/gl/glDraw2.h"
#include "NL/gl/glState.h"
#include "NL/gl/glStruct.h"
#include "NL/gl/glUserData.h"

/**
 * Offset/Address/Size: 0x0 | 0x8021093C | size: 0x144
 * TODO: 98.46% match - instruction scheduling: compiler hoists lwz loads
 * of m_pKernTable/m_KernTableSize before sth writes to stack KernPair struct
 */
unsigned long nlFont::GetCharWidth(unsigned short FontChar, unsigned short PrevFontChar) const
{
    const GlyphInfo* pGlyph;
    unsigned short c = FontChar;

    if (c > 0x7F)
    {
        pGlyph = &m_pExtendedGlyphs[c - 0x80];
    }
    else
    {
        pGlyph = &m_GlyphLookup[c - 0x20];
    }

    if (pGlyph->UnicodeChar == 0xFFFF)
    {
        nlPrintf("nlFont::GetCharWidth: unknown char 0x%x\n", (unsigned short)FontChar);
    }

    signed char offset = pGlyph->Offset;
    unsigned short prevChar = PrevFontChar;
    unsigned char advance = pGlyph->Advance;
    unsigned long ret = advance + offset;

    if (prevChar != 0)
    {
        const GlyphInfo* pPrevGlyph;
        if (prevChar > 0x7F)
        {
            pPrevGlyph = &m_pExtendedGlyphs[prevChar - 0x80];
        }
        else
        {
            pPrevGlyph = &m_GlyphLookup[prevChar - 0x20];
        }

        if (pPrevGlyph->HasKernPairs)
        {
            KernPair kp = { { PrevFontChar, FontChar }, 0 };
            KernPair* pFoundKP = nlBSearch<KernPair, KernPair>(kp, m_pKernTable, m_KernTableSize);
            if (pFoundKP != NULL)
            {
                ret += pFoundKP->Kern;
            }
        }
    }

    return (unsigned long)(ret * m_Metrics.Spacing);
}

/**
 * Offset/Address/Size: 0x144 | 0x80210A80 | size: 0xC
 */
void nlFont::DisableScissorBox() const
{
    m_bScissorBox = false;
}

/**
 * Offset/Address/Size: 0x150 | 0x80210A8C | size: 0x2C
 */
void nlFont::SetScissorBox(const ScissorBox& other) const
{
    m_scissorBox = other;
    m_bScissorBox = true;
}

/**
 * Offset/Address/Size: 0x17C | 0x80210AB8 | size: 0x6B4
 */
void nlFont::DrawString(eGLView View, const FontCharString& Text, const nlVector2& Position, const nlColour& Colour, const nlColour& EffectColour, int Length, nlFont::TextPass Passes, bool FlipY, unsigned long* pMatrix, nlColour* pOverrideColour) const
{
    float StartingX;
    float CurrentY = Position.f.y;
    int renderAscent = m_Metrics.RenderAscent;
    if (FlipY)
    {
        renderAscent = -renderAscent;
    }

    StartingX = Position.f.x;
    CurrentY -= (float)renderAscent;

    if (Length == -1)
    {
        Length = nlStrLen(Text.m_pString);
    }

    glPoly2* pQuads = (glPoly2*)__alloca((unsigned long)(Length * sizeof(glPoly2)));
    glPoly2* pCurrentQuad = pQuads;

    gl_ScreenInfo* pScreenInfo = glGetScreenInfo();
    float PixelCenter = pScreenInfo->PixelCentre;
    StartingX += PixelCenter;
    CurrentY += PixelCenter;

    glSetDefaultState(false);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_AlphaTestRef, 0);
    glSetRasterState(GLS_Culling, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentProgram(glGetProgram("2d unlit diffuse"));

    void* pUserData = 0;
    if (m_bScissorBox)
    {
        struct GLScissorUserData
        {
            unsigned short xOrig;
            unsigned short yOrig;
            unsigned short wd;
            unsigned short ht;
        };

        pUserData = glUserAlloc(GLUD_Scissor, sizeof(GLScissorUserData), false);
        GLScissorUserData* pScissor = (GLScissorUserData*)glUserGetData(pUserData);
        pScissor->xOrig = m_scissorBox.X;
        pScissor->yOrig = m_scissorBox.Y;
        pScissor->wd = m_scissorBox.Width;
        pScissor->ht = m_scissorBox.Height;
    }

    float GlyphRenderHeightVOffset = (float)m_Metrics.RenderHeight * m_InvTexSize;

    nlColour OverrideColour = Colour;
    if (pOverrideColour != 0 && pOverrideColour->c[3] != 0)
    {
        OverrideColour = *pOverrideColour;
    }

    const unsigned short EscapeBegin = nlEscapeSequence::ESCAPE_BEGIN;
    const unsigned short* PushColourIndex = 0;
    nlColour LastPushedColour = OverrideColour;

    unsigned long HandledChars = 0;
    unsigned long CurrentPage = 0;

    while (HandledChars < (unsigned long)Length)
    {
        bool useEffectTextures = false;
        if (Passes == PASS_Effect && m_TextureType == SplitFX)
        {
            useEffectTextures = true;
        }

        const unsigned long* textureHandles = useEffectTextures ? m_EffectTextureHandles : m_TextureHandles;
        glSetCurrentTexture(textureHandles[CurrentPage], GLTT_Diffuse);

        float CurrentX = StartingX;
        unsigned long i = 0;
        const unsigned short* pCurrentChar = Text.m_pString;

        while (*pCurrentChar != 0 && HandledChars < (unsigned long)Length && i < (unsigned long)Length)
        {
            unsigned short Char = *pCurrentChar;
            if (Char == EscapeBegin)
            {
                nlEscapeSequence escape(pCurrentChar);
                switch (escape.m_Type)
                {
                case ESC_COLOUR:
                    OverrideColour = escape.GetExtendedColour();

                    if (PushColourIndex < pCurrentChar)
                    {
                        if (OverrideColour.c[3] == 0)
                        {
                            LastPushedColour = Colour;
                            LastPushedColour.c[3] = 0;
                        }
                        else
                        {
                            LastPushedColour = OverrideColour;
                            LastPushedColour.c[3] = 0xFF;
                        }
                        PushColourIndex = pCurrentChar;
                    }

                    if (OverrideColour.c[3] == 0)
                    {
                        OverrideColour = Colour;
                    }
                    break;

                case ESC_NON_BREAKING_SPACE:
                    CurrentX += (float)((int)m_GlyphLookup[0].Advance + (int)m_GlyphLookup[0].Offset);
                    break;
                default:
                    break;
                }

                int consumed = (int)(escape.m_pEnd - pCurrentChar);
                i += consumed;
                i -= 1;
                if (CurrentPage == 0)
                {
                    HandledChars += consumed;
                }
                pCurrentChar = escape.m_pEnd - 1;
            }
            else
            {
                const GlyphInfo* pGlyph;
                if (Char > 0x7F)
                {
                    pGlyph = &m_pExtendedGlyphs[Char - 0x80];
                }
                else
                {
                    pGlyph = &m_GlyphLookup[Char - 0x20];
                }

                unsigned long Page = pGlyph->Page;
                if (Page > 0x10)
                {
                    nlPrintf("Font missing requested character");
                }
                else
                {
                    CurrentX += (float)pGlyph->Offset;
                    if (Page == CurrentPage)
                    {
                        pCurrentQuad->m_pos[1].f.x = CurrentX;
                        pCurrentQuad->m_pos[0].f.x = CurrentX;

                        float EndX = CurrentX + (float)pGlyph->RenderWidth;
                        pCurrentQuad->m_pos[3].f.x = EndX;
                        pCurrentQuad->m_pos[2].f.x = EndX;

                        pCurrentQuad->m_pos[3].f.y = CurrentY;
                        pCurrentQuad->m_pos[0].f.y = CurrentY;

                        int renderHeight = m_Metrics.RenderHeight;
                        if (FlipY)
                        {
                            renderHeight = -renderHeight;
                        }

                        float EndY = CurrentY + (float)renderHeight;
                        pCurrentQuad->m_pos[2].f.y = EndY;
                        pCurrentQuad->m_pos[1].f.y = EndY;

                        pCurrentQuad->depth = 0.0f;

                        pCurrentQuad->m_uv[1].f.x = pGlyph->uv.f.x;
                        pCurrentQuad->m_uv[0].f.x = pGlyph->uv.f.x;

                        float EndU = pGlyph->uv.f.x + (0.999f * ((float)pGlyph->RenderWidth * m_InvTexSize));
                        pCurrentQuad->m_uv[3].f.x = EndU;
                        pCurrentQuad->m_uv[2].f.x = EndU;

                        pCurrentQuad->m_uv[3].f.y = pGlyph->uv.f.y;
                        pCurrentQuad->m_uv[0].f.y = pGlyph->uv.f.y;

                        float EndV = pGlyph->uv.f.y + GlyphRenderHeightVOffset;
                        pCurrentQuad->m_uv[2].f.y = EndV;
                        pCurrentQuad->m_uv[1].f.y = EndV;

                        pCurrentQuad->SetColour(OverrideColour);

                        pCurrentQuad++;
                        HandledChars++;
                    }

                    int FinalAdvance = (int)pGlyph->Advance;
                    if (pGlyph->HasKernPairs && pCurrentChar[1] != 0)
                    {
                        KernPair kp = { { pCurrentChar[0], pCurrentChar[1] }, 0 };
                        KernPair* pValidKp = nlBSearch<KernPair, KernPair>(kp, m_pKernTable, m_KernTableSize);
                        if (pValidKp != 0)
                        {
                            FinalAdvance += pValidKp->Kern;
                        }
                    }

                    CurrentX += (float)FinalAdvance * m_Metrics.Spacing;
                }
            }

            pCurrentChar++;
            i++;
        }

        if (pOverrideColour != 0 && pOverrideColour->c[3] != 0)
        {
            OverrideColour = *pOverrideColour;
        }
        else
        {
            OverrideColour = Colour;
        }

        unsigned long UsedQuads = (unsigned long)(pCurrentQuad - pQuads);
        if (UsedQuads != 0)
        {
            if (m_bScissorBox)
            {
                glAttachPoly2(View, UsedQuads, pQuads, pMatrix, pUserData);
            }
            else
            {
                glAttachPoly2(View, UsedQuads, pQuads, pMatrix, 0);
            }
        }

        pCurrentQuad = pQuads;
        CurrentPage++;
    }

    if (m_TextureType == SplitFX && Passes == PASS_TextAndEffect)
    {
        DrawString(View, Text, Position, EffectColour, EffectColour, Length, PASS_Effect, View != 0, pMatrix, 0);
    }

    if (pOverrideColour != 0)
    {
        if (PushColourIndex != 0 && LastPushedColour.c[3] != 0)
        {
            *pOverrideColour = LastPushedColour;
            pOverrideColour->c[3] = 0xFF;
        }
        else
        {
            pOverrideColour->c[3] = 0;
        }
    }
}

/**
 * Offset/Address/Size: 0x830 | 0x8021116C | size: 0x9C0
 */
void nlFont::Load(const char*, char*, unsigned long)
{
}

/**
 * Offset/Address/Size: 0x11F0 | 0x80211B2C | size: 0x6C
 */
nlFont::~nlFont()
{
    ::operator delete[](m_pKernTable);
    m_pKernTable = NULL;
    if (m_pExtendedGlyphs != NULL)
    {
        ::operator delete[](m_pExtendedGlyphs);
    }
}

/**
 * Offset/Address/Size: 0x125C | 0x80211B98 | size: 0x3C
 */
nlFont::nlFont()
{
    memset(m_GlyphLookup, 0xFF, sizeof(m_GlyphLookup));
}

/**
 * Offset/Address/Size: 0x0 | 0x80211BD4 | size: 0x10
 */
template class ListContainerBase<nlFont::GlyphInfo, BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo> > >;

/**
 * Offset/Address/Size: 0x10 | 0x80211BE4 | size: 0x10
 */
template class ListContainerBase<nlFont::KernPair, BasicSlotPoolHigh<ListEntry<nlFont::KernPair> > >;

/**
 * Offset/Address/Size: 0x0 | 0x80211BF4 | size: 0x10
 */
int nlFont::GlyphInfo::SortProc(const nlFont::GlyphInfo* pa, const nlFont::GlyphInfo* pb)
{
    return pa->UnicodeChar - pb->UnicodeChar;
}

/**
 * Offset/Address/Size: 0x10 | 0x80211C04 | size: 0x10
 */
int nlFont::KernPair::SortProc(const nlFont::KernPair* pa, const nlFont::KernPair* pb)
{
    return pa->hash - pb->hash;
}

/**
 * Offset/Address/Size: 0x0 | 0x80211C14 | size: 0x20
 * BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo>>::freeFN(void*)
 *
 * Offset/Address/Size: 0x20 | 0x80211C34 | size: 0x28
 * BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo>>::allocFN(unsigned long)
 *
 * Offset/Address/Size: 0x48 | 0x80211C5C | size: 0x20
 * BasicSlotPoolHigh<ListEntry<nlFont::KernPair>>::freeFN(void*)
 *
 * Offset/Address/Size: 0x68 | 0x80211C7C | size: 0x28
 * BasicSlotPoolHigh<ListEntry<nlFont::KernPair>>::allocFN(unsigned long)
 */
void nlFont_stub()
{
    BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo> > pool1;
    BasicSlotPoolHigh<ListEntry<nlFont::KernPair> > pool2;
    nlQSort<nlFont::GlyphInfo>((nlFont::GlyphInfo*)0, 0, (int (*)(const nlFont::GlyphInfo*, const nlFont::GlyphInfo*))0);
    nlQSort<nlFont::KernPair>((nlFont::KernPair*)0, 0, (int (*)(const nlFont::KernPair*, const nlFont::KernPair*))0);
    nlWalkList<ListEntry<nlFont::GlyphInfo>, ListContainerBase<nlFont::GlyphInfo, BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo> > > >((ListEntry<nlFont::GlyphInfo>*)0, (ListContainerBase<nlFont::GlyphInfo, BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo> > >*)0, (void (ListContainerBase<nlFont::GlyphInfo, BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo> > >::*)(ListEntry<nlFont::GlyphInfo>*))0);
    nlWalkList<ListEntry<nlFont::KernPair>, ListContainerBase<nlFont::KernPair, BasicSlotPoolHigh<ListEntry<nlFont::KernPair> > > >((ListEntry<nlFont::KernPair>*)0, (ListContainerBase<nlFont::KernPair, BasicSlotPoolHigh<ListEntry<nlFont::KernPair> > >*)0, (void (ListContainerBase<nlFont::KernPair, BasicSlotPoolHigh<ListEntry<nlFont::KernPair> > >::*)(ListEntry<nlFont::KernPair>*))0);
    nlListRemoveStart<ListEntry<nlFont::GlyphInfo> >((ListEntry<nlFont::GlyphInfo>**)0, (ListEntry<nlFont::GlyphInfo>**)0);
    nlListRemoveStart<ListEntry<nlFont::KernPair> >((ListEntry<nlFont::KernPair>**)0, (ListEntry<nlFont::KernPair>**)0);
    nlListAddStart<ListEntry<nlFont::KernPair> >((ListEntry<nlFont::KernPair>**)0, (ListEntry<nlFont::KernPair>*)0, (ListEntry<nlFont::KernPair>**)0);
    nlListAddStart<ListEntry<nlFont::GlyphInfo> >((ListEntry<nlFont::GlyphInfo>**)0, (ListEntry<nlFont::GlyphInfo>*)0, (ListEntry<nlFont::GlyphInfo>**)0);
}

/**
 * Offset/Address/Size: 0x0 | 0x80211CA4 | size: 0x28
 */
// nlQSort<nlFont::GlyphInfo>(nlFont::GlyphInfo*, int, int (*)(const nlFont::GlyphInfo*, const nlFont::GlyphInfo*))
// {
// }

/**
 * Offset/Address/Size: 0x28 | 0x80211CCC | size: 0x28
 */
// nlQSort<nlFont::KernPair>(nlFont::KernPair*, int, int (*)(const nlFont::KernPair*, const nlFont::KernPair*))
// {
// }

/**
 * Offset/Address/Size: 0x50 | 0x80211CF4 | size: 0x8C
 */
// nlBSearch<nlFont::KernPair, nlFont::KernPair>(const nlFont::KernPair&, nlFont::KernPair*, int)
// {
// }

/**
 * Offset/Address/Size: 0x0 | 0x80211D80 | size: 0x68
 */
// nlWalkList<ListEntry<nlFont::GlyphInfo>, ListContainerBase<nlFont::GlyphInfo,
// BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo>>>>(ListEntry<nlFont::GlyphInfo>*, ListContainerBase<nlFont::GlyphInfo,
// BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo>>>*, void (ListContainerBase<nlFont::GlyphInfo,
// BasicSlotPoolHigh<ListEntry<nlFont::GlyphInfo>>>::*)(ListEntry<nlFont::GlyphInfo>*))
// {
// }

/**
 * Offset/Address/Size: 0x68 | 0x80211DE8 | size: 0x68
 */
// nlWalkList<ListEntry<nlFont::KernPair>, ListContainerBase<nlFont::KernPair,
// BasicSlotPoolHigh<ListEntry<nlFont::KernPair>>>>(ListEntry<nlFont::KernPair>*, ListContainerBase<nlFont::KernPair,
// BasicSlotPoolHigh<ListEntry<nlFont::KernPair>>>*, void (ListContainerBase<nlFont::KernPair,
// BasicSlotPoolHigh<ListEntry<nlFont::KernPair>>>::*)(ListEntry<nlFont::KernPair>*))
// {
// }

/**
 * Offset/Address/Size: 0xD0 | 0x80211E50 | size: 0x44
 */
// nlListRemoveStart<ListEntry<nlFont::GlyphInfo>>(ListEntry<nlFont::GlyphInfo>**, ListEntry<nlFont::GlyphInfo>**)
// {
// }

/**
 * Offset/Address/Size: 0x114 | 0x80211E94 | size: 0x44
 */
// nlListRemoveStart<ListEntry<nlFont::KernPair>>(ListEntry<nlFont::KernPair>**, ListEntry<nlFont::KernPair>**)
// {
// }

/**
 * Offset/Address/Size: 0x158 | 0x80211ED8 | size: 0x28
 */
// nlListAddStart<ListEntry<nlFont::KernPair>>(ListEntry<nlFont::KernPair>**, ListEntry<nlFont::KernPair>*, ListEntry<nlFont::KernPair>**)
// {
// }

/**
 * Offset/Address/Size: 0x180 | 0x80211F00 | size: 0x28
 */
// nlListAddStart<ListEntry<nlFont::GlyphInfo>>(ListEntry<nlFont::GlyphInfo>**, ListEntry<nlFont::GlyphInfo>*,
// ListEntry<nlFont::GlyphInfo>**)
// {
// }
