#ifndef _NLALGORITHM_H_
#define _NLALGORITHM_H_

#include "stdlib.h"

// Home header for the generic search/sort algorithm templates. The original
// compiler grouped weak instantiations into linkonce sections keyed by the
// BODY's file; DWARF attributes nlBSearch/nlQSort/nlDefaultQSortComparer to
// core/nlAlgorithm.h, so their bodies must live together here for a TU's
// .text section grouping to match the target link layout.

/**
 * Offset/Address/Size: 0x0 | 0x80213820 | size: 0x8C
 */
template <typename T, typename Key>
T* nlBSearch(const Key& key, T* pBase, int count)
{
#ifdef NL_POOL_DTOR_HOST
    FORCE_DONT_INLINE;
#endif
    const Key* keyPtr = &key;
    int high = count - 1;
    int low = -1;

    while ((high - low) > 1)
    {
        int mid = (high + low) / 2;

        if (pBase[mid].hash > (unsigned long)*keyPtr)
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
    }

    unsigned long highHash = pBase[high].hash;
    if (highHash == (unsigned long)*keyPtr)
    {
        return &pBase[high];
    }

    if (low == -1)
    {
        return nullptr;
    }

    unsigned long lowHash = pBase[low].hash;
    if (lowHash == (unsigned long)*keyPtr)
    {
        return &pBase[low];
    }

    return nullptr;
}

template <typename T>
int nlDefaultQSortComparer(const T* a, const T* b)
{
    if (a->hash > b->hash)
        return 1;
    if (a->hash == b->hash)
        return 0;
    return -1;
}

template <typename T>
void nlQSort(T* array, int size, int (*compare)(const T*, const T*))
{
#ifdef NL_POOL_DTOR_HOST
    FORCE_DONT_INLINE;
#endif
    qsort(array, size, sizeof(T), (int (*)(const void*, const void*))compare);
}

#endif // _NLALGORITHM_H_

// Outside the include guard on purpose: a TU that defines NL_POOL_DTOR_HOST
// and re-includes this header AFTER NL/nlListSlotPool.h hosts the
// ~nlListSlotPool body here, keying its linkonce bucket to this file so the
// weak __dt__36nlListSlotPool<...> emission fuses into the same section as
// nlQSort/nlDefaultQSortComparer (as in the target DOL). Inert for every
// other TU. NOTE: non-inline on purpose - the out-of-line reference from the
// nlListSlotPoolReap phantom is what pulls the dtor out of the post-__sinit
// instantiation wave.
#if defined(NL_POOL_DTOR_HOST) && defined(_NLLISTSLOTPOOL_H_) && !defined(_NL_POOL_DTOR_EMITTED_)
#define _NL_POOL_DTOR_EMITTED_
template <typename T>
nlListSlotPool<T>::~nlListSlotPool()
{
}
#endif
