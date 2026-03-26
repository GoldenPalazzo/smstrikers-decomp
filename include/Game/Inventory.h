#ifndef GAME_INVENTORY_H
#define GAME_INVENTORY_H

#include "NL/nlList.h"

template <typename T>
class cInventory
{
public:
    cInventory()
        : m_nItemCount(0)
    {
    }

    ~cInventory()
    {
        ListEntry<T*>* meshEntry = m_lItemList.m_Head;
        while (meshEntry != NULL)
        {
            meshEntry->data->Destroy();
            meshEntry = meshEntry->next;
        }

        typedef ListContainerBase<T*, NewAdapter<ListEntry<T*> > > ItemListBase;
        void (ItemListBase::*cb)(ListEntry<T*>*) = &ItemListBase::DeleteEntry;
        meshEntry = m_lItemList.m_Head;
        while (meshEntry != NULL)
        {
            ListEntry<T*>* next = meshEntry->next;
            (((ItemListBase*)this)->*cb)(meshEntry);
            meshEntry = next;
        }

        m_lItemList.m_Head = NULL;
        m_lItemList.m_Tail = NULL;

        ListEntry<char*>** pTail = &m_lMemList.m_Tail;
        while (m_lMemList.m_Head != NULL)
        {
            ListEntry<char*>* first = m_lMemList.m_Head;
            if (first == NULL)
            {
                first = NULL;
            }
            else
            {
                if (pTail != NULL)
                {
                    if (m_lMemList.m_Tail == first)
                    {
                        m_lMemList.m_Tail = NULL;
                    }
                }
                ListEntry<char*>* tmp = m_lMemList.m_Head;
                m_lMemList.m_Head = tmp->next;
                first = tmp;
            }
            void* mesh;
            if (&mesh != NULL)
            {
                mesh = first->data;
            }
            ::operator delete(first);
            ::operator delete(mesh);
        }

        m_nItemCount = 0;
    }

    /* 0x0 */ nlListContainer<T*> m_lItemList;
    /* 0xC */ nlListContainer<char*> m_lMemList;
    /* 0x18 */ int m_nItemCount;
}; // total size: 0x1C

#endif // GAME_INVENTORY_H
