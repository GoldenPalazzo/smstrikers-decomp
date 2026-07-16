#ifndef _NLRINGCOUNT_H_
#define _NLRINGCOUNT_H_

// Standalone definition for TUs that emit this algorithm before nlDLRing.h.

#include "types.h"

template <typename T>
u32 nlRingCountElements(T* head)
{
    T* current;
    u32 count = 0;

    if (head == NULL)
    {
        return 0;
    }

    current = head->m_next;
    while (true)
    {
        T* next = current->m_next;
        count++;
        if (current == head)
        {
            break;
        }
        current = next;
    }
    return count;
}

#endif // _NLRINGCOUNT_H_
