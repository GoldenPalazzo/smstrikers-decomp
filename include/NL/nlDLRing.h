#ifndef _NLDLRING_H_
#define _NLDLRING_H_

// Doubly Linked Ring

#include "types.h"

template <typename T>
class DLListEntry
{
public:
    /* 0x0 */ DLListEntry<T>* m_next;
    /* 0x4 */ DLListEntry<T>* m_prev;
    /* 0x8 */ T m_data;

    DLListEntry()
        : m_next(nullptr)
        , m_prev(nullptr)
        , m_data()
    {
    }

    DLListEntry(T* data)
        : m_next(nullptr)
        , m_prev(nullptr)
        , m_data(data)
    {
    }
}; // total size: 0xC

template <typename T>
void nlDLRingInsert(T** head, T* afterNode, T* newNode)
{

    afterNode->m_next->m_prev = newNode;
    newNode->m_next = afterNode->m_next;
    newNode->m_prev = afterNode;
    afterNode->m_next = newNode;
    if ((void*)*head == afterNode)
    {
        *head = newNode;
    }
}

template <typename T>
void nlDLRingAddStart(T** head, T* newNode)
{
    T* temp;

    temp = *head;
    if (temp == NULL)
    {
        *head = newNode;
        newNode->m_next = newNode;
        newNode->m_prev = newNode;
        return;
    }

    temp->m_next->m_prev = newNode;
    newNode->m_next = temp->m_next;
    newNode->m_prev = temp;
    temp->m_next = newNode;
}

template <typename T>
void nlDLRingAddEnd(T** head, T* newNode)
{
    nlDLRingAddStart(head, newNode);
    *head = newNode;
}

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
T* nlDLRingGetEnd(T* current)
{
    if (current != 0)
    {
        return current;
    }
    return NULL;
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

template <typename T>
bool nlRingIsEnd(T* head, T* element)
{
    if (head == NULL)
    {
        return true;
    }
    return head == element;
}

template <typename T>
u32 nlRingCountElements(T* head)
{
    u32 count = 0;
    if (head == NULL)
    {
        return 0;
    }

    T* current = head->m_next;
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

template <typename T>
u32 nlDLRingCountElements(T* head)
{
    return nlRingCountElements(head);
}

template <typename T>
bool nlDLRingIsStart(T* head, T* current)
{
    if (head == NULL)
    {
        return false;
    }
    return head->m_next == current;
}

template <typename T>
T* nlDLRingRemoveStart(T** current)
{
    T* temp_r31;
    temp_r31 = (*current)->m_next;
    nlDLRingRemove<T>(current, temp_r31);
    return temp_r31;
}

template <typename T>
T* nlDLRingRemoveEnd(T** head)
{
    T* node = *head;
    nlDLRingRemove<T>(head, node);
    return node;
}

template <typename T>
bool nlDLRingValidateContainsElement(T* head, const T* node)
{
    if (head == 0)
        return false;

    T* cur = head->m_next;

    for (;;)
    {
        if (node == cur)
            return true;

        T* next = cur->m_next;

        if (cur == head)
            break;

        cur = next;
    }

    return false;
}

template <typename T>
bool nlDLRingRemoveSafely(T** head, const T* node)
{
    T* temp_r0;

    if (nlDLRingValidateContainsElement(*head, node) == 0)
    {
        return false;
    }

    temp_r0 = node->m_next;
    if (temp_r0 == node)
    {
        *head = NULL;
        return 1;
    }
    node->m_prev->m_next = temp_r0;
    node->m_next->m_prev = node->m_prev;
    if (*head == node)
    {
        *head = node->m_prev;
        return true;
    }

    return true;
}

template <typename T>
void nlDLRingAppendRing(T** head, T* current)
{
    T* currentHead;
    T* nextAfterHead;

    if (current != NULL)
    {
        currentHead = *head;
        if (currentHead == NULL)
        {
            *head = current;
            return;
        }

        nextAfterHead = currentHead->m_next;
        currentHead->m_next = current->m_next;
        current->m_next->m_prev = *head;
        current->m_next = nextAfterHead;
        nextAfterHead->m_prev = current;
        *head = current;
    }
}

template <typename T>
void nlDeleteRing(T** head)
{
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

template <typename T>
void nlDeleteDLRing(T** head)
{
    nlDeleteRing<T>(head);
}

/**
 * (EXAMPLE: SkinPairList) Offset/Address/Size: 0xAC | 0x801E21A0 | size: 0x2C
 */
template <typename T>
void nlRingAddStart(T** list, T* item)
{
    T* head = *list;
    if (head == nullptr)
    {
        *list = item;
        item->m_next = item;
        return;
    }

    item->m_next = head->m_next;
    head = *list;
    head->m_next = item;
}

/**
 * (EXAMPLE: SkinPairList) Offset/Address/Size: 0x70 | 0x801E2164 | size: 0x3C
 */
template <typename T>
void nlRingAddEnd(T** list, T* item)
{
    nlRingAddStart(list, item);
    *list = item;
}

template <typename T>
T* nlRingGetStart(T* head)
{
    if (head == NULL)
    {
        return NULL;
    }
    return head->m_next;
}

template <typename T, typename CallbackType>
void nlWalkDLRing(T* head, CallbackType* callback, void (CallbackType::*callbackFunc)(T*))
{
    void (CallbackType::*func)(T*) = callbackFunc;
    nlWalkRing(head, callback, func);
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

// =======================================================
// Iterator for nlDLRing
// =======================================================

template <typename T>
class nlDLListIterator
{
public:
    typedef T ValueType;
    typedef T* Pointer;
    typedef T& Reference;

    nlDLListIterator(Pointer head)
    {
        m_head = head;
        m_current = (head ? head->m_next : 0);
        m_first = true;
    }

    nlDLListIterator(Pointer head, Pointer current)
    {
        m_head = head;
        m_current = current;
        // m_first = true;
    }

    bool hasNext() const { return (m_current != 0) && (m_first || m_current != m_head); }

    Pointer next()
    {
        if (!hasNext())
            return 0;

        Pointer ret = m_current;
        m_current = m_current->m_next;
        m_first = false;
        return ret;
    }

    void reset()
    {
        m_current = (m_head ? m_head->m_next : 0);
        m_first = true;
    }

    Pointer m_head;
    Pointer m_current;
    bool m_first;
};

#endif // _NLDLRING_H_
