#ifndef GAME_INVENTORY_PHYSICS_DTOR_H
#define GAME_INVENTORY_PHYSICS_DTOR_H

template <typename T>
inline cInventory<T>::~cInventory()
{
    ListEntry<T*>* itemEntry = m_lItemList.m_Head;
    while (itemEntry != NULL)
    {
        itemEntry->entry->Destroy();
        itemEntry = itemEntry->next;
    }

    typedef ListContainerBase<T*, NewAdapter<ListEntry<T*> > > ItemListBase;
    void (ItemListBase::*cb)(ListEntry<T*>*) = ItemListBase::DeleteEntryFunc();
    nlWalkList(m_lItemList.m_Head, (ItemListBase*)this, cb);

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
        void* memory;
        if (&memory != NULL)
        {
            memory = first->entry;
        }
        ::operator delete(first);
        ::operator delete(memory);
    }

    m_nItemCount = 0;
}

#endif // GAME_INVENTORY_PHYSICS_DTOR_H
