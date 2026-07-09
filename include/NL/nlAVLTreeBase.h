#ifndef _AVLTREEBASE_H_
#define _AVLTREEBASE_H_

#include "NL/nlMemory.h"

#include "dolphin/types.h"
#include "types.h"

#include "dolphin/types.h"

struct AVLTreeNode
{
    /* 0x0 */ AVLTreeNode* left;
    /* 0x4 */ AVLTreeNode* right;
    /* 0x8 */ s8 heavy;
}; // total size: 0xC

class AVLTreeUntemplated
{
public:
    virtual int CompareNodes(AVLTreeNode* node1, AVLTreeNode* node2) = 0;
    virtual int CompareKey(void* key, AVLTreeNode* n) = 0;
    virtual AVLTreeNode* AllocateEntry(void* key, void* value) = 0;

    AVLTreeNode* RemoveAVLNode(AVLTreeNode** root, void* key, unsigned int height);
    u32 AddAVLNode(AVLTreeNode** rootNode, void* key, void* value, AVLTreeNode** existingNode, unsigned int height);
};

template <typename KeyType, typename ValueType>
class AVLTreeEntry
{
public:
    AVLTreeNode node; // offset 0x0, size 0xC
    KeyType key;      // offset 0xC, size 0x4
    ValueType value;  // offset 0x10, size 0x4
}; // total size: 0x14

// DefaultKeyCompare template implementation
template <typename KeyType>
class DefaultKeyCompare
{
public:
    int operator()(const KeyType& key2, const KeyType& key1) const
    {
        int result;
        if (key1 == key2)
            result = 0;
        else if (key1 < key2)
            result = -1;
        else
            result = 1;
        return result;
    }
};

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
class AVLTreeBase : public AVLTreeUntemplated
{
public:
    AVLTreeBase()
    {
        m_NumElements = 0;
        m_Root = nullptr;
        m_Compare = nullptr;
    };

    ~AVLTreeBase();

#ifdef NL_AVLTREEBASE_DELETEENTRY_INCLASS
    // In-class variant (feResourceManager.cpp only): an in-class-defined
    // member codegens eagerly when called from a dont_inline stub, which
    // anchors DeleteEntry at the head of the TU's AVLTreeBase linkonce
    // section like the original object. The Free logic is spelled out
    // because the eager codegen runs while dont_inline is active, which
    // would otherwise keep m_Allocator.Free() as an out-of-line call.
    void DeleteEntry(AVLTreeEntry<KeyType, ValueType>* entry)
    {
        SlotPoolEntry* e = (SlotPoolEntry*)entry;
        e->m_next = this->m_Allocator.m_FreeList;
        this->m_Allocator.m_FreeList = e;
    }
#else
    void DeleteEntry(AVLTreeEntry<KeyType, ValueType>* entry);
#endif

    typedef void (AVLTreeBase::*ENTRY_DELETE_FUNC)(AVLTreeEntry<KeyType, ValueType>*);

    // Single mint site for the DeleteEntry PTMF const (.data): every caller
    // materializes the anon const here, so MWCC pools one copy per TU.
    static ENTRY_DELETE_FUNC DeleteEntryFunc()
    {
        return &AVLTreeBase::DeleteEntry;
    }

    void Clear();
    void DestroyTree(void (AVLTreeBase::*deleteFunc)(AVLTreeEntry<KeyType, ValueType>*));
    void DeleteValue(AVLTreeEntry<KeyType, ValueType>* entry);
    void DeleteValues();

    void PostorderTraversal(AVLTreeEntry<KeyType, ValueType>* curr, void (AVLTreeBase::*cb)(AVLTreeEntry<KeyType, ValueType>*));

    template <typename CallbackType>
    void Walk(CallbackType* cbClass, void (CallbackType::*cb)(const KeyType&, ValueType*));

    template <typename CallbackType>
    void InorderWalk(AVLTreeEntry<KeyType, ValueType>* curr, CallbackType* cbClass, void (CallbackType::*cb)(const KeyType&, ValueType*));

    // Inline method to find an entry by key in the AVL tree
    inline AVLTreeEntry<KeyType, ValueType>* Find(const KeyType& key) const
    {
        AVLTreeEntry<KeyType, ValueType>* node = m_Root;

        while (node != nullptr)
        {
            int cmpResult;
            if (key == node->key)
            {
                cmpResult = 0;
            }
            else if (key < node->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                return node;
            }
            else
            {
                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.right;
                }
            }
        }

        return nullptr;
    }

    inline ValueType* FindGet(const KeyType& key) const
    {
        ValueType* foundValue;
        AVLTreeEntry<KeyType, ValueType>* node = m_Root;

        while (node != nullptr)
        {
            int cmpResult;
            if (key == node->key)
            {
                cmpResult = 0;
            }
            else if (key < node->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (&foundValue != nullptr)
                {
                    foundValue = &node->value;
                }
                break;
            }
            else
            {
                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.right;
                }
            }
        }

        return foundValue;
    }

    inline bool FindGet(const KeyType& key, ValueType** foundValue) const
    {
        AVLTreeEntry<KeyType, ValueType>* node = m_Root;

        while (node != nullptr)
        {
            int cmpResult;
            if (key == node->key)
            {
                cmpResult = 0;
            }
            else if (key < node->key)
            {
                cmpResult = -1;
            }
            else
            {
                cmpResult = 1;
            }

            if (cmpResult == 0)
            {
                if (foundValue != nullptr)
                {
                    *foundValue = &node->value;
                }
                return true;
            }
            else
            {
                if (cmpResult < 0)
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.left;
                }
                else
                {
                    node = (AVLTreeEntry<KeyType, ValueType>*)node->node.right;
                }
            }
        }

        return false;
    }

    virtual int CompareNodes(AVLTreeNode* a, AVLTreeNode* b);

    virtual int CompareKey(void* key, AVLTreeNode* node);

    virtual AVLTreeNode* AllocateEntry(void* key, void* value);

    AVLTreeEntry<KeyType, ValueType>* CastUp(AVLTreeNode* node) const;

public:
    /* 0x04 */ AllocatorType m_Allocator;
    /* 0x08 */ AVLTreeEntry<KeyType, ValueType>* m_Root;
    /* 0x0C */ CompareType* m_Compare;
    /* 0x10 */ unsigned int m_NumElements;
}; // total size: 0x14

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
inline AVLTreeEntry<KeyType, ValueType>* AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::CastUp(AVLTreeNode* node) const
{
    return (AVLTreeEntry<KeyType, ValueType>*)node;
}

#ifndef NL_AVLTREEBASE_DELETEENTRY_INCLASS
template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::DeleteEntry(AVLTreeEntry<KeyType, ValueType>* entry)
{
    m_Allocator.Free(entry);
}
#endif

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::DeleteValue(AVLTreeEntry<KeyType, ValueType>* entry)
{
    delete entry->value;
    m_Allocator.Delete(entry);
}

#ifndef NL_AVLTREEBASE_DECLARE_ONLY
template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::~AVLTreeBase()
{
    Clear();
}
#endif

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::DestroyTree(void (AVLTreeBase::*deleteFunc)(AVLTreeEntry<KeyType, ValueType>*))
{
    if (m_Root != NULL)
    {
        PostorderTraversal(m_Root, deleteFunc);
        m_Root = nullptr;
        m_NumElements = 0;
    }
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::PostorderTraversal(AVLTreeEntry<KeyType, ValueType>* curr,
    void (AVLTreeBase::*cb)(AVLTreeEntry<KeyType, ValueType>*))
{
    if (curr->node.left != NULL)
    {
        PostorderTraversal(CastUp(curr->node.left), cb);
    }
    if (curr->node.right != NULL)
    {
        PostorderTraversal(CastUp(curr->node.right), cb);
    }
    (this->*cb)(curr);
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
int AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    const KeyType& keyA = ((AVLTreeEntry<KeyType, ValueType>*)a)->key;
    const KeyType& keyB = ((AVLTreeEntry<KeyType, ValueType>*)b)->key;
    int result;
    if (keyA == keyB)
        result = 0;
    else if (keyA < keyB)
        result = -1;
    else
        result = 1;
    return result;
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
int AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::CompareKey(void* key, AVLTreeNode* node)
{
    int result;
    KeyType k = *(KeyType*)key;
    AVLTreeEntry<KeyType, ValueType>* entry = (AVLTreeEntry<KeyType, ValueType>*)node;
    if (k == entry->key)
        result = 0;
    else if (k < entry->key)
        result = -1;
    else
        result = 1;
    return result;
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
AVLTreeNode* AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::AllocateEntry(void* key, void* value)
{
    AVLTreeEntry<KeyType, ValueType>* newNode = NULL;

    m_Allocator.Allocate(newNode);

    newNode->node.left = NULL;
    newNode->node.right = NULL;
    newNode->node.heavy = 0;
    newNode->key = *(KeyType*)key;
    newNode->value = *(ValueType*)value;

    return (AVLTreeNode*)newNode;
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
template <typename CallbackType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::Walk(CallbackType* cbClass,
    // void (CallbackType::*cb)(void*, void*, unsigned long&, unsigned long*))
    void (CallbackType::*cb)(const KeyType&, ValueType*))
{
    InorderWalk(m_Root, cbClass, cb);
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
template <typename CallbackType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::InorderWalk(AVLTreeEntry<KeyType, ValueType>* curr,
    // CallbackType* cbClass, void (CallbackType::*cb)(void*, void*, unsigned long&, unsigned long*))
    CallbackType* cbClass, void (CallbackType::*cb)(const KeyType&, ValueType*))
{
    while (curr != nullptr)
    {
        InorderWalk(CastUp(curr->node.left), cbClass, cb);
        (cbClass->*cb)(curr->key, &curr->value);
        curr = CastUp(curr->node.right);
    }
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::Clear()
{
    DestroyTree(DeleteEntryFunc());
    m_NumElements = 0;
}

template <typename KeyType, typename ValueType, typename AllocatorType, typename CompareType>
void AVLTreeBase<KeyType, ValueType, AllocatorType, CompareType>::DeleteValues()
{
    DestroyTree(&AVLTreeBase::DeleteValue);
    m_NumElements = 0;
}

#endif // _AVLTREEBASE_H_
