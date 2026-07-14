template <>
inline EffectsGroupTreeBase::ENTRY_DELETE_FUNC EffectsGroupTreeBase::DeleteEntryFunc()
{
    return &EffectsGroupTreeBase::DeleteEntry;
}

template <>
inline EffectsTerrainTreeBase::ENTRY_DELETE_FUNC EffectsTerrainTreeBase::DeleteEntryFunc()
{
    return &EffectsTerrainTreeBase::DeleteEntry;
}

// Explicit specializations are listed in reverse target order. MWCC's
// deferred codegen flushes them back into the target linkonce order.
template <>
AVLTreeNode* EffectsGroupTreeBase::AllocateEntry(void* key, void* value)
{
    EffectsGroupTreeEntry* newNode = NULL;
    m_Allocator.Allocate(newNode);
    newNode->node.left = NULL;
    newNode->node.right = NULL;
    newNode->node.heavy = 0;
    newNode->key = *(unsigned long*)key;
    newNode->value = *(EffectsGroup**)value;
    return (AVLTreeNode*)newNode;
}

template <>
AVLTreeNode* EffectsTerrainTreeBase::AllocateEntry(void* key, void* value)
{
    EffectsTerrainTreeEntry* newNode = NULL;
    m_Allocator.Allocate(newNode);
    newNode->node.left = NULL;
    newNode->node.right = NULL;
    newNode->node.heavy = 0;
    newNode->key = *(unsigned long*)key;
    newNode->value = *(EffectsTerrainSpec**)value;
    return (AVLTreeNode*)newNode;
}

template <>
int EffectsTerrainTreeBase::CompareKey(void* key, AVLTreeNode* node)
{
    unsigned long k = *(unsigned long*)key;
    EffectsTerrainTreeEntry* entry = (EffectsTerrainTreeEntry*)node;
    int result;
    if (k == entry->key)
        result = 0;
    else if (k < entry->key)
        result = -1;
    else
        result = 1;
    return result;
}

template <>
int EffectsTerrainTreeBase::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    const unsigned long& keyA = ((EffectsTerrainTreeEntry*)a)->key;
    const unsigned long& keyB = ((EffectsTerrainTreeEntry*)b)->key;
    int result;
    if (keyA == keyB)
        result = 0;
    else if (keyA < keyB)
        result = -1;
    else
        result = 1;
    return result;
}

template <>
int EffectsGroupTreeBase::CompareKey(void* key, AVLTreeNode* node)
{
    unsigned long k = *(unsigned long*)key;
    EffectsGroupTreeEntry* entry = (EffectsGroupTreeEntry*)node;
    int result;
    if (k == entry->key)
        result = 0;
    else if (k < entry->key)
        result = -1;
    else
        result = 1;
    return result;
}

template <>
int EffectsGroupTreeBase::CompareNodes(AVLTreeNode* a, AVLTreeNode* b)
{
    const unsigned long& keyA = ((EffectsGroupTreeEntry*)a)->key;
    const unsigned long& keyB = ((EffectsGroupTreeEntry*)b)->key;
    int result;
    if (keyA == keyB)
        result = 0;
    else if (keyA < keyB)
        result = -1;
    else
        result = 1;
    return result;
}

template <>
void EffectsGroupTreeBase::DeleteValues()
{
    FORCE_DONT_INLINE;
    DestroyTree(&EffectsGroupTreeBase::DeleteValue);
    m_NumElements = 0;
}

template <>
WEAKFUNC EffectsGroupTreeEntry* EffectsGroupTreeBase::CastUp(AVLTreeNode* node) const
{
    return (EffectsGroupTreeEntry*)node;
}

template <>
void EffectsGroupTreeBase::PostorderTraversal(
    EffectsGroupTreeEntry* curr,
    void (EffectsGroupTreeBase::*cb)(EffectsGroupTreeEntry*))
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

template <>
void EffectsGroupTreeBase::DestroyTree(
    void (EffectsGroupTreeBase::*deleteFunc)(EffectsGroupTreeEntry*))
{
    if (m_Root != NULL)
    {
        PostorderTraversal(m_Root, deleteFunc);
        m_Root = NULL;
        m_NumElements = 0;
    }
}

template <>
void EffectsGroupTreeBase::Clear()
{
    DestroyTree(DeleteEntryFunc());
    m_NumElements = 0;
}

template <>
EffectsGroupTreeBase::~AVLTreeBase()
{
    Clear();
}

template <>
WEAKFUNC nlAVLTree<unsigned long, EffectsGroup*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

template <>
void EffectsTerrainTreeBase::DeleteValues()
{
    FORCE_DONT_INLINE;
    DestroyTree(&EffectsTerrainTreeBase::DeleteValue);
    m_NumElements = 0;
}

template <>
WEAKFUNC EffectsTerrainTreeEntry* EffectsTerrainTreeBase::CastUp(AVLTreeNode* node) const
{
    return (EffectsTerrainTreeEntry*)node;
}

template <>
void EffectsTerrainTreeBase::PostorderTraversal(
    EffectsTerrainTreeEntry* curr,
    void (EffectsTerrainTreeBase::*cb)(EffectsTerrainTreeEntry*))
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

template <>
void EffectsTerrainTreeBase::DestroyTree(
    void (EffectsTerrainTreeBase::*deleteFunc)(EffectsTerrainTreeEntry*))
{
    if (m_Root != NULL)
    {
        PostorderTraversal(m_Root, deleteFunc);
        m_Root = NULL;
        m_NumElements = 0;
    }
}

template <>
void EffectsTerrainTreeBase::Clear()
{
    DestroyTree(DeleteEntryFunc());
    m_NumElements = 0;
}

template <>
EffectsTerrainTreeBase::~AVLTreeBase()
{
    Clear();
}

template <>
void EffectsGroupTreeBase::DeleteEntry(EffectsGroupTreeEntry* entry)
{
    m_Allocator.Free(entry);
}

template <>
void EffectsTerrainTreeBase::DeleteEntry(EffectsTerrainTreeEntry* entry)
{
    m_Allocator.Free(entry);
}

#pragma inline_depth(0)
template <>
void EffectsTerrainTreeBase::DeleteValue(EffectsTerrainTreeEntry* entry)
{
    delete entry->value;
    m_Allocator.Delete(entry);
}

template <>
void EffectsGroupTreeBase::DeleteValue(EffectsGroupTreeEntry* entry)
{
    delete entry->value;
    m_Allocator.Delete(entry);
}
#pragma inline_depth(255)
