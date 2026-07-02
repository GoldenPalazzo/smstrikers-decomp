#ifndef _NLLISTCONTAINERDTOR_H_
#define _NLLISTCONTAINERDTOR_H_

// Out-of-class body for the ListContainerBase<T, BasicSlotPool<ListEntry<T>>>
// dtor (declared in NL/nlListContainer.h). Deliberately housed in its OWN
// header: the weak __dt__90ListContainerBase<...> emission is a phantom
// (absent from the target DOL); giving its body a dedicated file keeps it in
// a dedicated all-phantom linkonce section the linker can drop, instead of
// fusing into DeleteEntry's kept section in nlListContainer.h.
// Included by NL/nlList.h right after nlListContainer.h.

template <typename T>
inline ListContainerBase<T, BasicSlotPool<ListEntry<T> > >::~ListContainerBase()
{
    DestroyAllEntries(this);
}

#endif // _NLLISTCONTAINERDTOR_H_
