#include "Game/World/worldanim.h"
#include "NL/nlString.h"
#include "NL/nlSlotPool.h"
#include "Game/SAnim.h"

template <>
nlAVLTree<unsigned long, AnimationSet*, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x5C8 | 0x8019B394 | size: 0x94
 */
WorldAnimManager::WorldAnimManager()
{
    cInventory<cSHierarchy>* inv = (cInventory<cSHierarchy>*)nlMalloc(sizeof(cInventory<cSHierarchy>), 8, 0);
    if (inv != NULL)
    {
        inv->m_lItemList.m_Head = NULL;
        inv->m_lItemList.m_Tail = NULL;
        inv->m_lMemList.m_Head = NULL;
        inv->m_lMemList.m_Tail = NULL;
        inv->m_nItemCount = 0;
    }
    m_pHierarchyInventory = inv;
}

/**
 * Offset/Address/Size: 0x14C | 0x8019AF18 | size: 0x41C
 * TODO: 96.88% match - r30/r31 register swap throughout function and cascading stack offset differences
 */
WorldAnimManager::~WorldAnimManager()
{
    typedef AVLTreeEntry<unsigned long, AnimationSet*> TreeEntry;
    typedef ListContainerBase<cSAnim*, NewAdapter<ListEntry<cSAnim*> > > SAnimListBase;
    typedef ListContainerBase<cSHierarchy*, NewAdapter<ListEntry<cSHierarchy*> > > HierListBase;

    struct NodeStack
    {
        TreeEntry** data;
        u32 count;
    };

    NodeStack* stack;
    TreeEntry* node;

    stack = (NodeStack*)nlMalloc(sizeof(NodeStack), 8, false);
    if (stack != NULL)
    {
        u32 numElements = m_animationSetMap.m_NumElements;
        node = m_animationSetMap.m_Root;
        stack->data = (TreeEntry**)nlMalloc((numElements + 1) * sizeof(TreeEntry*), 8, false);
        stack->count = 0;
        if (node != NULL)
        {
            while (node->node.left != NULL)
            {
                stack->data[stack->count] = node;
                stack->count++;
                node = (TreeEntry*)node->node.left;
            }
            stack->data[stack->count] = node;
            stack->count++;
        }
    }

    while (stack->count > 0)
    {
        TreeEntry* entry = stack->data[stack->count - 1];
        AnimationSet* animSet = entry->value;
        if (animSet != NULL)
        {
            if (&animSet->m_animInventory != NULL)
            {
                ListEntry<cSAnim*>* animEntry = animSet->m_animInventory.m_lItemList.m_Head;
                while (animEntry != NULL)
                {
                    animEntry->data->Destroy();
                    animEntry = animEntry->next;
                }
                void (SAnimListBase::*cbSAnim)(ListEntry<cSAnim*>*) = &SAnimListBase::DeleteEntry;
                nlWalkList(animSet->m_animInventory.m_lItemList.m_Head, (SAnimListBase*)animSet, cbSAnim);
                ListEntry<char*>** pTail = &animSet->m_animInventory.m_lMemList.m_Tail;
                animSet->m_animInventory.m_lItemList.m_Head = NULL;
                ListEntry<char*>** pHead = &animSet->m_animInventory.m_lMemList.m_Head;
                animSet->m_animInventory.m_lItemList.m_Tail = NULL;
                while (animSet->m_animInventory.m_lMemList.m_Head != NULL)
                {
                    ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead, pTail);
                    void* mesh;
                    if (&mesh != NULL)
                    {
                        mesh = first->data;
                    }
                    ::operator delete(first);
                    ::operator delete(mesh);
                }
                animSet->m_animInventory.m_nItemCount = 0;
                animSet->m_animInventory.m_lMemList.~nlListContainer();
                animSet->m_animInventory.m_lItemList.~nlListContainer();
            }
            ::operator delete(animSet);
        }
        stack->count--;
        TreeEntry* popped = stack->data[stack->count];
        TreeEntry* right = (TreeEntry*)popped->node.right;
        if (right != NULL)
        {
            while (right->node.left != NULL)
            {
                stack->data[stack->count] = right;
                stack->count++;
                right = (TreeEntry*)right->node.left;
            }
            stack->data[stack->count] = right;
            stack->count++;
        }
    }
    if (stack != NULL)
    {
        ::operator delete[](stack->data);
        ::operator delete(stack);
    }
    cInventory<cSHierarchy>* inv = m_pHierarchyInventory;
    if (inv != NULL)
    {
        ListEntry<cSHierarchy*>* hierEntry = inv->m_lItemList.m_Head;
        while (hierEntry != NULL)
        {
            hierEntry = hierEntry->next;
        }
        void (HierListBase::*cbHier)(ListEntry<cSHierarchy*>*) = &HierListBase::DeleteEntry;
        nlWalkList(inv->m_lItemList.m_Head, (HierListBase*)inv, cbHier);
        ListEntry<char*>** pTail2 = &inv->m_lMemList.m_Tail;
        inv->m_lItemList.m_Head = NULL;
        ListEntry<char*>** pHead2 = &inv->m_lMemList.m_Head;
        inv->m_lItemList.m_Tail = NULL;
        while (inv->m_lMemList.m_Head != NULL)
        {
            ListEntry<char*>* first = nlListRemoveStart<ListEntry<char*> >(pHead2, pTail2);
            void* mesh;
            if (&mesh != NULL)
            {
                mesh = first->data;
            }
            ::operator delete(first);
            ::operator delete(mesh);
        }
        inv->m_nItemCount = 0;
        inv->m_lMemList.~nlListContainer();
        inv->m_lItemList.~nlListContainer();
        ::operator delete(inv);
    }
}

/**
 * Offset/Address/Size: 0x58 | 0x8019AE24 | size: 0xF4
 */
void WorldAnimController::SetAnimation(const char* szAnimationName, ePlayMode playMode)
{
    u32 hash = nlStringLowerHash(szAnimationName);
    cSAnim* anim = m_pAnimationSet->FindAnimationByHash(hash);

    if (m_pPoseTree != NULL)
    {
        delete m_pPoseTree;
    }

    cPN_SAnimController* newController = AllocateSAnimController();
    newController = new (newController) cPN_SAnimController(anim, NULL, playMode, NULL, 0, false);
    m_pPoseTree = newController;
}

/**
 * Offset/Address/Size: 0x44 | 0x8019AE10 | size: 0x14
 */
void WorldAnimController::SetAnimationTime(float fTime)
{
    cPN_SAnimController* cntrl = m_pPoseTree;
    cntrl->m_fPrevTime = cntrl->m_fTime;
    cntrl->m_fTime = fTime;
}

/**
 * Offset/Address/Size: 0x38 | 0x8019AE04 | size: 0xC
 */
float WorldAnimController::GetAnimationTime()
{
    return m_pPoseTree->m_fTime;
}

/**
 * Offset/Address/Size: 0x0 | 0x8019ADCC | size: 0x38
 */
float WorldAnimController::GetAnimationDuration()
{
    return (float)m_pPoseTree->m_pSAnim->m_nNumKeys / 30.0f;
}
