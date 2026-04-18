#ifndef _GLINVENTORY_H_
#define _GLINVENTORY_H_

#include "NL/nlList.h"
#include "NL/nlAVLTree.h"

#include "Game/SAnim.h"
#include "Game/GL/GLMaterial.h"
#include "Game/GL/GLVertexAnim.h"
#include "Game/GL/GLTextureAnim.h"
class GLShadowVolume;
class GLSkinMesh;
class glModel;

template <typename ValueType>
class freeing_GLInventory
{
public:
    ~freeing_GLInventory()
    {
        FORCE_DONT_INLINE;
        Release();
        delete m_pItems;
    }

    void Release()
    {
        FORCE_DONT_INLINE;
        typedef AVLTreeEntry<unsigned long, ValueType*> Entry;
        typedef nlAVLTree<unsigned long, ValueType*, DefaultKeyCompare<unsigned long> > Tree;

        struct NodeStack
        {
            Entry** data;
            u32 count;
        };

        NodeStack* stack;
        Entry* node;
        Tree* tree = m_pItems;

        stack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
        if (stack != NULL)
        {
            u32 numElements = tree->m_NumElements;
            node = tree->m_Root;
            stack->data = (Entry**)nlMalloc((numElements + 1) * sizeof(Entry*), 8, false);
            stack->count = 0;

            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    stack->data[stack->count] = node;
                    stack->count++;
                    node = (Entry*)node->node.left;
                }
                stack->data[stack->count] = node;
                stack->count++;
            }
        }

        while (stack->count != 0)
        {
            Entry* top = stack->data[stack->count - 1];
            nlFree(top->value);
            stack->count--;

            Entry* current = stack->data[stack->count];
            Entry* right = (Entry*)current->node.right;
            if (right != NULL)
            {
                while (right->node.left != NULL)
                {
                    stack->data[stack->count] = right;
                    stack->count++;
                    right = (Entry*)right->node.left;
                }
                stack->data[stack->count] = right;
                stack->count++;
            }
        }

        if (stack != NULL)
        {
            delete[] stack->data;
            delete stack;
        }

        m_pItems->Clear();
    }

    /* 0x0 */ nlAVLTree<unsigned long, ValueType*, DefaultKeyCompare<unsigned long> >* m_pItems; // offset 0x0, size 0x4
}; // total size: 0x4

template <typename ValueType>
class clearing_GLInventory
{
public:
    ~clearing_GLInventory();

    void Release()
    {
        FORCE_DONT_INLINE;
        m_pItems->Clear();
    }

    /* 0x0 */ nlAVLTree<unsigned long, ValueType*, DefaultKeyCompare<unsigned long> >* m_pItems; // offset 0x0, size 0x4
}; // total size: 0x4

template <typename ValueType>
class deleting_GLInventory
{
public:
    ~deleting_GLInventory()
    {
        FORCE_DONT_INLINE;
        Release();
        delete m_pItems;
    }

    void Release()
    {
        FORCE_DONT_INLINE;
        typedef AVLTreeEntry<unsigned long, ValueType*> Entry;

        struct NodeStack
        {
            Entry** array;
            u32 count;
        };

        nlAVLTree<unsigned long, ValueType*, DefaultKeyCompare<unsigned long> >* tree = m_pItems;
        NodeStack* pStack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
        if (pStack != NULL)
        {
            u32 numElements = tree->m_NumElements;
            Entry* node = tree->m_Root;
            pStack->array = (Entry**)nlMalloc((numElements + 1) * sizeof(Entry*), 8, false);
            pStack->count = 0;

            if (node != NULL)
            {
                while (node->node.left != NULL)
                {
                    pStack->array[pStack->count] = node;
                    pStack->count++;
                    node = (Entry*)node->node.left;
                }
                pStack->array[pStack->count] = node;
                pStack->count++;
            }
        }

        while (pStack->count > 0)
        {
            Entry* top = pStack->array[pStack->count - 1];
            delete top->value;
            pStack->count--;

            Entry* right = (Entry*)pStack->array[pStack->count]->node.right;
            if (right != NULL)
            {
                while (right->node.left != NULL)
                {
                    pStack->array[pStack->count] = right;
                    pStack->count++;
                    right = (Entry*)right->node.left;
                }
                pStack->array[pStack->count] = right;
                pStack->count++;
            }
        }

        if (pStack != NULL)
        {
            delete[] pStack->array;
            delete pStack;
        }

        m_pItems->Clear();
    }

    /* 0x0 */ nlAVLTree<unsigned long, ValueType*, DefaultKeyCompare<unsigned long> >* m_pItems;
}; // total size: 0x4

class GLInventory
{
public:
    ~GLInventory();
    void Create();
    void Delete();
    void ReleaseLevel(int);
    void ResourceMark();
    void ResourceRelease(int);
    void AddModel(unsigned long, glModel*);
    glModel* GetModel(unsigned long);
    GLShadowVolume* GetShadowVolume(unsigned long);
    void AddTextureAnim(unsigned long, GLTextureAnim*);
    GLTextureAnim* GetTextureAnim(unsigned long);
    void AddVertexAnim(unsigned long, GLVertexAnim*);
    GLVertexAnim* GetVertexAnim(unsigned long);
    void AddMaterialList(unsigned long, GLMaterialList*);
    GLMaterialList* GetMaterialList(unsigned long);
    void AddSkinData(unsigned long, nlChunk*);
    GLSkinMesh* MakeSkinMesh(unsigned long);
    void Update(float);

    /* 0x000 */ nlListContainer<void*>* m_pFileData[16];
    /* 0x040 */ freeing_GLInventory<nlChunk>* m_pSkinData[16];
    /* 0x080 */ clearing_GLInventory<glModel>* m_pModels[16];
    /* 0x0C0 */ deleting_GLInventory<GLShadowVolume>* m_pShadowVolumes[16];
    /* 0x100 */ deleting_GLInventory<GLTextureAnim>* m_pTextureAnims[16];
    /* 0x140 */ deleting_GLInventory<GLVertexAnim>* m_pVertexAnims[16];
    /* 0x180 */ deleting_GLInventory<GLMaterialList>* m_pMaterialLists[16];
    /* 0x1C0 */ int m_nLevel;
    /* 0x1C4 */ unsigned char m_bCreated;
}; // total size: 0x1C8

extern GLInventory glInventory;

#endif // _GLINVENTORY_H_
