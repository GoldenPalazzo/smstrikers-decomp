#ifndef _NLLISTCONTHOST_H_
#define _NLLISTCONTHOST_H_

// Defence in depth: this header is included ONLY by src/Game/CharacterTemplate.cpp.
// Gate the bodies so an accidental include elsewhere is a no-op.
#ifdef CHARACTERTEMPLATE_LISTCONT_HOST
// Body host for the listcont DeleteEntry pair. Two facts drive this file:
//   * BUCKET = the file holding the BODY. A body in the .cpp emits into main
//     .text; hosting it in a header yields the linkonce section the target has.
//   * An `inline` EXPLICIT specialization is a concrete function, so it emits at
//     its DEF-POSITION under the reverse-source drain -- unlike an implicitly
//     instantiated template body, which the address-take would pin 1st.
// So this header's INCLUDE POSITION selects listcont's slot, and the def order
// below is REVERSED (drain is reverse-source) to yield Hier@0x0, Ret@0x24.

template <>
inline void ListContainerBase<AnimRetargetList*, NewAdapter<ListEntry<AnimRetargetList*> > >::DeleteEntry(ListEntry<AnimRetargetList*>* entry)
{
    FORCE_DONT_INLINE;
    m_Allocator.DeleteEntry(entry);
}

template <>
inline void ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*> > >::DeleteEntry(ListEntry<cSHierarchy*>* entry)
{
    FORCE_DONT_INLINE;
    m_Allocator.DeleteEntry(entry);
}

#endif // CHARACTERTEMPLATE_LISTCONT_HOST

#endif // _NLLISTCONTHOST_H_
