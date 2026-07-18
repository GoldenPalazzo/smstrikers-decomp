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

    ~cInventory();

    void Clear();

    /* 0x0 */ nlListContainer<T*> m_lItemList;
    /* 0xC */ nlListContainer<char*> m_lMemList;
    /* 0x18 */ int m_nItemCount;
}; // total size: 0x1C

template <typename T>
inline cInventory<T>::~cInventory()
{
    Clear();
}

template <typename T>
inline void cInventory<T>::Clear()
{
    ListEntry<T*>* meshEntry = m_lItemList.m_Head;
    while (meshEntry != NULL)
    {
        meshEntry->entry->Destroy();
        meshEntry = meshEntry->next;
    }

    m_lItemList.Clear();

    while (m_lMemList.m_Head != NULL)
    {
        ListEntry<char*>* first = m_lMemList.RemoveStart();
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

#endif // GAME_INVENTORY_H
