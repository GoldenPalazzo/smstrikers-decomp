#ifndef _NLRING_H_
#define _NLRING_H_

// Singly/doubly linked ring deletion helpers (engine/global/src/core/nlRing.h).
// nlDeleteRing lives here, separate from nlDLRing.h, so MWCC emits it into its
// own COMDAT/linkonce section (matches the target's section layout).

#include "types.h"

template <typename T>
void nlDeleteRing(T** head)
{
    FORCE_DONT_INLINE;
    T* current;
    T* next;

    T* headPtr = *head;
    if (headPtr != NULL)
    {
        current = headPtr->m_next;
        for (;;)
        {
            next = current->m_next;
            delete current;
            if (current != *head)
            {
                current = next;
            }
            else
            {
                break;
            }
        }
        *head = NULL;
    }
}

#endif // _NLRING_H_
