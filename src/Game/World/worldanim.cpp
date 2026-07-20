#include "Game/World/worldanim.h"
#include "NL/nlString.h"
#include "NL/nlSlotPool.h"
#include "Game/SAnim.h"

/**
 * Offset/Address/Size: 0x5C8 | 0x8019B394 | size: 0x94
 */
WorldAnimManager::WorldAnimManager()
{
    m_pHierarchyInventory = new (nlMalloc(sizeof(cInventory<cSHierarchy>), 8, false)) cInventory<cSHierarchy>();
}

/**
 * Offset/Address/Size: 0x14C | 0x8019AF18 | size: 0x41C
 * TODO: 99.87% match - remaining r26/r31 cursor differences in animation tree traversal
 */
WorldAnimManager::~WorldAnimManager()
{
    typedef AVLTreeEntry<unsigned long, AnimationSet*> TreeEntry;
    struct NodeStack
    {
        TreeEntry** data;
        u32 count;
    };

    cInventory<cSHierarchy>* inv;
    AnimationSet* animSet;
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
        animSet = entry->value;
        delete animSet;
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
    inv = m_pHierarchyInventory;
    delete inv;
}

/**
 * Offset/Address/Size: 0x58 | 0x8019AE24 | size: 0xF4
 */
void WorldAnimController::SetAnimation(const char* szAnimationName, ePlayMode playMode)
{
    u32 hash = nlStringLowerHash(szAnimationName);
    cSAnim* anim = m_pAnimationSet->m_animInventory.Find((unsigned int)hash);

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
