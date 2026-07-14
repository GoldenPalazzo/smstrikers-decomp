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

template <typename T, typename CallbackType>
void nlWalkRing(T* head, CallbackType* callback, void (CallbackType::*callbackFunc)(T*))
{
    if (head == NULL)
    {
        return;
    }

    T* current = head->m_next;
    while (true)
    {
        T* next = current->m_next;
        (callback->*callbackFunc)(current);
        if (current == head)
        {
            break;
        }
        current = next;
    }
}
