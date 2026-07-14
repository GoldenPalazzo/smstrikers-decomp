#ifndef _NLAVLTREE_H_
#define _NLAVLTREE_H_

#include "NL/nlAdapter.h"
#include "NL/nlAVLTreeBase.h"
#include "NL/nlMemory.h"

template <typename KeyType, typename ValueType, typename CompareType>
class nlAVLTree : public AVLTreeBase<KeyType, ValueType, NewAdapter<AVLTreeEntry<KeyType, ValueType> >, CompareType>
{
public:
#ifndef NL_AVLTREE_GLINVENTORY_LINK_ORDER
#ifdef NL_AVLTREE_EXPLICIT_CONSTRUCTORS
    nlAVLTree();
#else
    nlAVLTree()
        : AVLTreeBase<KeyType, ValueType, NewAdapter<AVLTreeEntry<KeyType, ValueType> >, CompareType>() { };
#endif
    ~nlAVLTree();
#endif
};

template <typename KeyType, typename ValueType, typename CompareType>
class nlAVLTreeIterator
{
public:
    typedef AVLTreeEntry<KeyType, ValueType> Entry;

    nlAVLTreeIterator(nlAVLTree<KeyType, ValueType, CompareType>& tree)
    {
        Entry* node = tree.m_Root;
        m_Stack = (Entry**)nlMalloc((tree.m_NumElements + 1) * sizeof(Entry*), 8, false);
        m_NumStackEntries = 0;
        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                m_Stack[m_NumStackEntries] = node;
                m_NumStackEntries++;
                node = (Entry*)node->node.left;
            }
            m_Stack[m_NumStackEntries] = node;
            m_NumStackEntries++;
        }
    }

    ~nlAVLTreeIterator()
    {
        delete[] m_Stack;
    }

    void Next()
    {
        m_NumStackEntries--;
        Entry* entry = m_Stack[m_NumStackEntries];
        Entry* right = (Entry*)entry->node.right;
        if (right != NULL)
        {
            while (right->node.left != NULL)
            {
                m_Stack[m_NumStackEntries] = right;
                m_NumStackEntries++;
                right = (Entry*)right->node.left;
            }
            m_Stack[m_NumStackEntries] = right;
            m_NumStackEntries++;
        }
    }

    bool IsValid() const
    {
        return m_NumStackEntries > 0;
    }

    Entry* Current() const
    {
        return m_Stack[m_NumStackEntries - 1];
    }

    /* 0x0 */ Entry** m_Stack;
    /* 0x4 */ u32 m_NumStackEntries;
}; // total size: 0x8

#if !defined(NL_AVLTREE_DECLARE_ONLY) && !defined(NL_AVLTREE_GLINVENTORY_LINK_ORDER)
template <typename KeyType, typename ValueType, typename CompareType>
nlAVLTree<KeyType, ValueType, CompareType>::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}
#endif

#endif // _NLAVLTREE_H_
