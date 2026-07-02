#ifndef _NLLISTSLOTPOOLDTOR_H_
#define _NLLISTSLOTPOOLDTOR_H_

// Body of ~nlListSlotPool, housed separately so its weak emission
// (__dt__36nlListSlotPool<...>, KEPT in the target DOL) gets a linkonce
// section of its own instead of fusing with the phantom inline ctors that a
// `template class nlListSlotPool<T>;` directive also emits.
// The AudioScriptEventMgr TU (NL_POOL_DTOR_HOST) hosts this body at the end
// of NL/nlAlgorithm.h instead, to key its linkonce bucket there; every other
// TU keeps this definition exactly as before.
#ifndef NL_POOL_DTOR_HOST
template <typename T>
inline nlListSlotPool<T>::~nlListSlotPool()
{
}
#endif

#endif // _NLLISTSLOTPOOLDTOR_H_
