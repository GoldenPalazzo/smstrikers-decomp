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

    /* 0x0 */ nlListContainer<T*> m_lItemList;
    /* 0xC */ nlListContainer<T*> m_lMemList;
    /* 0x18 */ int m_nItemCount;
}; // total size: 0x1C

#endif // GAME_INVENTORY_H
