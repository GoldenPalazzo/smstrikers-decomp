#ifndef _NLQSORT_H_
#define _NLQSORT_H_

#include "stdlib.h"

template <typename T>
void nlQSort(T* array, int size, int (*compare)(const T*, const T*))
{
    qsort(array, size, sizeof(T), (int (*)(const void*, const void*))compare);
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

#endif // _NLQSORT_H_
