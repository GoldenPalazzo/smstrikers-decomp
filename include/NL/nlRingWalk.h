#ifndef _NLRINGWALK_H_
#define _NLRINGWALK_H_

// Out-of-line home for nlWalkRing, used together with
// NLDLRING_WALKRING_SEPARATE (see nlDLRing.h). A TU that defines that macro
// gets nlWalkRing's body from this header instead, so the instantiation
// lands in its own linkonce section like the original objects.

template <typename T, typename CallbackType>
void nlWalkRing(T* head, CallbackType* callback, void (CallbackType::*callbackFunc)(T*))
{
    if (head == 0)
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

#endif // _NLRINGWALK_H_
