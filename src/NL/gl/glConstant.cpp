#define NL_AVLTREE_DECLARE_ONLY
#define NL_AVLTREEBASE_DECLARE_ONLY

#include "NL/gl/glConstant.h"
#include "NL/nlAVLTree.h"
#include "NL/nlString.h"

static int level = 0;

static nlAVLTree<unsigned long, nlVector4, DefaultKeyCompare<unsigned long> >* constants[16] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static nlVector4 vZero = { 0.0f, 0.0f, 0.0f, 0.0f };

typedef AVLTreeEntry<unsigned long, nlVector4> GLCEntry;
typedef AVLTreeBase<unsigned long, nlVector4, NewAdapter<GLCEntry>, DefaultKeyCompare<unsigned long> > GLCBase;

template <>
void GLCBase::DeleteEntry(GLCEntry*);
template <>
void GLCBase::Clear();
template <>
void GLCBase::DestroyTree(void (GLCBase::*)(GLCEntry*));
template <>
void GLCBase::PostorderTraversal(GLCEntry*, void (GLCBase::*)(GLCEntry*));
template <>
GLCEntry* GLCBase::CastUp(AVLTreeNode*) const;

static inline bool glConstantFindInTree(nlAVLTree<unsigned long, nlVector4, DefaultKeyCompare<unsigned long> >* tree, unsigned long key, nlVector4*& foundValue)
{
    AVLTreeEntry<unsigned long, nlVector4>* node = tree->m_Root;

    while (node != NULL)
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
            if (&foundValue != NULL)
            {
                foundValue = &node->value;
            }
            return true;
        }

        if (cmpResult < 0)
        {
            node = (AVLTreeEntry<unsigned long, nlVector4>*)node->node.left;
        }
        else
        {
            node = (AVLTreeEntry<unsigned long, nlVector4>*)node->node.right;
        }
    }

    return false;
}

static inline nlVector4 glConstantGet(unsigned long constantHash)
{
    nlVector4* foundValue;
    nlVector4* result;

    for (int i = level; i >= 0; i--)
    {
        if (glConstantFindInTree(constants[i], constantHash, foundValue))
        {
            result = foundValue;
            goto found;
        }
    }

    result = NULL;

found:
    if (result == NULL)
    {
        result = &vZero;
    }

    return *result;
}

/**
 * Offset/Address/Size: 0x0 | 0x801DF220 | size: 0x10C
 */
nlVector4 glConstantGet(const char* constantName)
{
    return glConstantGet(nlStringHash(constantName));
}

/**
 * Offset/Address/Size: 0x10C | 0x801DF32C | size: 0x10C
 */
bool glConstantGet(const char* constantName, nlVector4& result)
{
    u32 hash = nlStringHash(constantName);
    nlVector4* foundValue;
    nlVector4* out;

    for (int i = level; i >= 0; i--)
    {
        if (glConstantFindInTree(constants[i], hash, foundValue))
        {
            out = foundValue;
            goto found;
        }
    }

    out = NULL;

found:
    if (out == NULL)
    {
        return false;
    }

    result = *out;
    return true;
}

/**
 * Offset/Address/Size: 0x218 | 0x801DF438 | size: 0x14C
 */
void glConstantSet(const char* constantName, const nlVector4& value)
{
    u32 hash = nlStringHash(constantName);
    nlVector4* foundValue;
    nlVector4* result;

    for (int i = level; i >= 0; i--)
    {
        if (glConstantFindInTree(constants[i], hash, foundValue))
        {
            result = foundValue;
            goto found;
        }
    }

    result = NULL;

found:
    if (result == NULL)
    {
        AVLTreeNode* existingNode;
        nlAVLTree<unsigned long, nlVector4, DefaultKeyCompare<unsigned long> >* tree = constants[level];

        tree->AddAVLNode((AVLTreeNode**)&tree->m_Root, &hash, (void*)&value, &existingNode, tree->m_NumElements);

        if (existingNode == NULL)
        {
            tree->m_NumElements++;
        }
    }
    else
    {
        *result = value;
    }
}

/**
 * Offset/Address/Size: 0x364 | 0x801DF584 | size: 0x60
 */
void gl_ConstantMarkerBackup(int arg)
{
    while (level != arg)
    {
        constants[level]->Clear();
        level--;
    }
}

/**
 * Offset/Address/Size: 0x3C4 | 0x801DF5E4 | size: 0x10
 */
void gl_ConstantMarkerAdvance()
{
    level++;
}

/**
 * Offset/Address/Size: 0x3D4 | 0x801DF5F4 | size: 0x98
 */
void gl_ConstantStartup()
{
    for (int i = 0; i < 16; i++)
    {
        constants[i] = new (nlMalloc(0x14, 8, 0)) nlAVLTree<unsigned long, nlVector4, DefaultKeyCompare<unsigned long> >();
    }
    level = 0;
}

template <>
void GLCBase::DeleteEntry(GLCEntry* entry)
{
    delete entry;
}

template <>
void GLCBase::Clear()
{
    DestroyTree(&GLCBase::DeleteEntry);
    m_NumElements = 0;
}

template <>
void GLCBase::DestroyTree(void (GLCBase::*deleteFunc)(GLCEntry*))
{
    if (m_Root != NULL)
    {
        PostorderTraversal(m_Root, deleteFunc);
        m_Root = NULL;
        m_NumElements = 0;
    }
}

template <>
void GLCBase::PostorderTraversal(GLCEntry* curr, void (GLCBase::*cb)(GLCEntry*))
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
GLCEntry* GLCBase::CastUp(AVLTreeNode* node) const
{
    return (GLCEntry*)node;
}
