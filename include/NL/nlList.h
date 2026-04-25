#ifndef _NLLIST_H_
#define _NLLIST_H_

#include "types.h"

#include "NL/nlAdapter.h"
#include "NL/nlDLRing.h"
#include "NL/nlSlotPool.h"

template <typename T>
class ListEntry
{
public:
    /* 0x00 */ ListEntry<T>* next;
    /* 0x04 */ T data;

    ListEntry()
        : next(nullptr)
        , data(nullptr)
    {
    }

    ListEntry(T data)
        : next(nullptr)
        , data(data)
    {
    }
};

template <typename T, typename Adapter>
class ListContainerBase
{
public:
    ListContainerBase()
        : m_Head(NULL)
        , m_Tail(NULL)
    {
    }

    void DeleteEntry(ListEntry<T>* entry);

    // Add more list operations as needed
    void AddEntry(ListEntry<T>* entry)
    {
        // Implementation for adding entries
    }

    void RemoveEntry(ListEntry<T>* entry)
    {
        // Implementation for removing entries
    }

    // offsets and sizes are dependent on the adapter
    /* 0x0 */ Adapter m_Allocator;
    ListEntry<T>* m_Head;
    ListEntry<T>* m_Tail;
};

template <typename T, typename Adapter>
void ListContainerBase<T, Adapter>::DeleteEntry(ListEntry<T>* entry)
{
    m_Allocator.DeleteEntry(entry);
}

template <typename T>
class nlListContainer : public ListContainerBase<T, NewAdapter<ListEntry<T> > >
{
public:
    ~nlListContainer()
    {
        if (this != NULL)
        {
            nlWalkList(this->m_Head, static_cast<ListContainerBase<T, NewAdapter<ListEntry<T> > >*>(this), &ListContainerBase<T, NewAdapter<ListEntry<T> > >::DeleteEntry);
            this->m_Head = NULL;
            this->m_Tail = NULL;
        }
    }
}; // total size: 0xC

template <typename T>
ListEntry<T>* nlListRemoveStart(ListEntry<T>** head, ListEntry<T>** tail)
{
    ListEntry<T>* first = *head;
    if (first == 0)
        return 0;

    if (tail != 0)
    {
        if (*tail == first)
            *tail = 0;
    }

    // Remove first element
    ListEntry<T>* tmp = *head;
    *head = tmp->next;
    return tmp;
}

template <typename T>
void nlListAddStart(ListEntry<T>** head, ListEntry<T>* entry, ListEntry<T>** tail)
{
    if (tail != 0)
    {
        if (*head == 0)
        {
            *tail = entry;
        }
    }

    // Insert at start
    entry->next = *head;
    *head = entry;
}

template <typename T>
void nlDeleteList(T** head)
{
    T* next;
    while (*head != NULL)
    {
        next = (*head)->next;
        delete *head;
        *head = next;
    }
    *head = NULL;
}

template <typename T>
T* nlListRemoveStart(T** head, T** tail)
{
    T* current_node = *head;
    if (current_node == NULL)
    {
        return NULL;
    }
    if ((tail != NULL) && (*tail == current_node))
    {
        *tail = NULL;
    }
    T* node_to_remove = *head;
    *head = node_to_remove->next;
    return node_to_remove;
}

template <typename T>
void nlListAddEnd(T** head, T** tail, T* node)
{
    node->next = NULL;
    T* temp_r6 = *tail;
    if (temp_r6 != NULL)
    {
        temp_r6->next = node;
        *tail = node;
        return;
    }
    *tail = node;
    *head = node;
}

template <typename T>
class nlList
{
public:
    T* m_pStart; // offset 0x0, size 0x4
    T* m_pEnd;   // offset 0x4, size 0x4
}; // total size: 0x8

template <typename T>
T* nlListRemoveElement(T** head, T* element, T** tail)
{
    FORCE_DONT_INLINE;
    if (head == NULL)
        return NULL;

    if (*head == element)
    {
        if (tail != NULL && *tail == *head)
        {
            *tail = NULL;
        }
        T* first = *head;
        *head = first->next;
        return NULL;
    }

    T* prev = *head;
    T* current = prev->next;
    while (current != NULL)
    {
        if (current == element)
        {
            prev->next = current->next;
            if (tail == NULL)
                return prev;
            if (*tail == current)
            {
                *tail = prev;
            }
            return prev;
        }
        prev = current;
        current = current->next;
    }
    return NULL;
}

template <typename T>
s32 nlListCountElements(T* head)
{
    T* node = head;
    s32 count = 0;
    while (node != NULL)
    {
        node = node->next;
        count += 1;
    }
    return count;
}

template <typename EntryT, typename ContainerT>
void nlWalkList(EntryT* list, ContainerT* cbClass, void (ContainerT::*cb)(EntryT*))
{
    while (list != NULL)
    {
        EntryT* next = list->next;
        (cbClass->*cb)(list);
        list = next;
    }
}

#endif
