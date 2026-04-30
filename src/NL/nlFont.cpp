#include "NL/nlFont.h"
#include "NL/nlBSearch.h"
#include "NL/nlList.h"
#include "NL/nlQSort.h"
#include "NL/nlSlotPoolHigh.h"
#include "NL/nlString.h"

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
// void nlFont::DrawString(eGLView, const FontCharString&, const nlVector2&, const nlColour&, const nlColour&, int, nlFont::TextPass, bool,
// unsigned long*, nlColour*) const
// {
// }

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
