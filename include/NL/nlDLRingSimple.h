#ifndef _NLDLRINGSIMPLE_H_
#define _NLDLRINGSIMPLE_H_

#include "types.h"

// Out-of-line home for the simple DL-ring helpers when
// NLDLRING_SIMPLE_SEPARATE is set. Keeping these bodies in a separate header
// gives their weak instantiations a separate linkonce section.

template <typename T>
void nlDLRingRemove(T** head, T* current)
{
    T* tmp_node = current->m_next;

    if (tmp_node == current)
    {
        *head = NULL;
        return;
    }

    current->m_prev->m_next = tmp_node;
    current->m_next->m_prev = current->m_prev;

    if (*head == current)
    {
        *head = current->m_prev;
    }
}

template <typename T>
T* nlDLRingGetStart(T* current)
{
    if (current == NULL)
    {
        return NULL;
    }
    return current->m_next;
}

template <typename T>
bool nlDLRingIsEnd(T* head, T* current)
{
    if (head == NULL)
    {
        return true;
    }
    return head == current;
}

#endif // _NLDLRINGSIMPLE_H_
