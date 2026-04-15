#ifndef _FETEMPLATES_H_
#define _FETEMPLATES_H_

template <class T>
T* FindItemByHashID(T* head, unsigned long hash)
{
    if (head == 0)
        return 0;

    T* cur = head->m_next;

    for (;;)
    {
        unsigned long id = cur->m_hash;
        T* next = cur->m_next;

        if (hash == id)
            return cur;

        if (cur == head)
            break;

        cur = next;
    }

    return 0;
}

template <class T>
T* CastToSomeType(T*, void* pValue)
{
    return (T*)pValue;
}

#endif // _FETEMPLATES_H_
