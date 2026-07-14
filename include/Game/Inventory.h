#ifndef GAME_INVENTORY_H
#define GAME_INVENTORY_H

#include "Game/SAnim.h"
#include "NL/nlFile.h"
#include "NL/nlList.h"
#include "NL/nlString.h"

template <typename T>
class cInventory
{
public:
    cInventory()
        : m_nItemCount(0)
    {
    }

    void ParseChunks(nlChunk* chunk, nlChunk* end)
    {
        while (chunk != end)
        {
            if (T::IsValidChunkID(chunk->m_ID))
            {
                T* item = T::Initialize(chunk);
                m_lItemList.AddStart(item);
                m_nItemCount++;
            }
            else
            {
                nlPrintf("Warning: inventory encountered an unknown chunk type\n");
            }
            chunk = (nlChunk*)((char*)chunk + chunk->m_Size + 8);
        }
    }

    void AddFile(char* memory, unsigned long length)
    {
        m_lMemList.AddStart(memory);
        ParseChunks((nlChunk*)memory, (nlChunk*)(memory + length));
    }

    void AddFile(const char* filename)
    {
        unsigned long length;
        char* memory = (char*)nlLoadEntireFile(filename, &length, 0x20, AllocateStart);
        m_lMemList.AddStart(memory);
        ParseChunks((nlChunk*)memory, (nlChunk*)(memory + length));
    }

    ~cInventory()
    {
        ListEntry<char*>** pTail;
        ListEntry<char*>** pHead;
        ListEntry<T*>* meshEntry = m_lItemList.m_Head;
        while (meshEntry != NULL)
        {
            meshEntry->entry->Destroy();
            meshEntry = meshEntry->next;
        }

        typedef ListContainerBase<T*, NewAdapter<ListEntry<T*> > > ItemListBase;
        void (ItemListBase::*cb)(ListEntry<T*>*) = ItemListBase::DeleteEntryFunc();
        nlWalkList(m_lItemList.m_Head, (ItemListBase*)this, cb);

        m_lItemList.m_Head = NULL;
        m_lItemList.m_Tail = NULL;

        nlListContainer<char*>* memList = &m_lMemList;
        pTail = &memList->m_Tail;
        pHead = &memList->m_Head;
        while (m_lMemList.m_Head != NULL)
        {
            ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
            void* mesh;
            if (&mesh != NULL)
            {
                mesh = first->entry;
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
