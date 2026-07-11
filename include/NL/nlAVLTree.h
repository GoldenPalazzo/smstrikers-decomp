#ifndef _NLAVLTREE_H_
#define _NLAVLTREE_H_

#include "NL/nlAdapter.h"
#include "NL/nlAVLTreeBase.h"

template <typename KeyType, typename ValueType, typename CompareType>
class nlAVLTree : public AVLTreeBase<KeyType, ValueType, NewAdapter<AVLTreeEntry<KeyType, ValueType> >, CompareType>
{
public:
#ifndef NL_AVLTREE_GLINVENTORY_LINK_ORDER
    nlAVLTree()
        : AVLTreeBase<KeyType, ValueType, NewAdapter<AVLTreeEntry<KeyType, ValueType> >, CompareType>() { };
    ~nlAVLTree();
#endif
};

#if !defined(NL_AVLTREE_DECLARE_ONLY) && !defined(NL_AVLTREE_GLINVENTORY_LINK_ORDER)
template <typename KeyType, typename ValueType, typename CompareType>
nlAVLTree<KeyType, ValueType, CompareType>::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}
#endif

#endif // _NLAVLTREE_H_
