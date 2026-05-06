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
        nlWalkList(m_lItemList.m_Head, (ItemListBase*)this, cb);

        m_lItemList.m_Head = NULL;
        m_lItemList.m_Tail = NULL;

        nlListContainer<char*>* memList = &m_lMemList;
        ListEntry<char*>** pTail = &memList->m_Tail;
        ListEntry<char*>** pHead = &memList->m_Head;
        while (memList->m_Head != NULL)
        {
            ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
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
