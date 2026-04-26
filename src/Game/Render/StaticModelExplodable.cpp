#include "Game/Render/StaticModelExplodable.h"
#include "Game/Drawable/DrawableObj.h"
#include "NL/nlBasicString.h"

u8 StaticModelExplodable::bIsModelLoaded[2];
nlList<SidelineExplodableNode> StaticModelExplodable::sStaticModelExplodableList;
ExplodableCategoryData StaticModelExplodable::sCategoryData[NUM_STATIC_MODEL_EXPLODABLE_CATEGORIES];

class World;
extern World* s_World__12WorldManager;

extern "C" StaticModelExplodable* __ct__21StaticModelExplodableF29StaticModelExplodableCategoryRC9nlMatrix4(
    StaticModelExplodable*, StaticModelExplodableCategory, const nlMatrix4&);

struct StaticModelExplodableHelperTreeWalkData
{
    AVLTreeEntry<unsigned long, HelperObject*>** mStack;
    unsigned int mCount;
};

struct StaticModelExplodableWorldHelperMapView
{
    char _pad0[0x74];
    AVLTreeEntry<unsigned long, HelperObject*>* mRoot;
    DefaultKeyCompare<unsigned long>* mCompare;
    unsigned int mNumElements;
};

// /**
//  * Offset/Address/Size: 0x0 | 0x80165D2C | size: 0x138
//  */
// void 0x8028D320..0x8028D324 | size: 0x4
// {
// }

/**
 * Offset/Address/Size: 0x5AC | 0x80165A70 | size: 0x2BC
 */
void StaticModelExplodable::CreateExplodablesFromHelperObjects()
{
    StaticModelExplodableHelperTreeWalkData* pWalkData;
    World* pWorld;
    void* pNode;

    pWorld = s_World__12WorldManager;
    pWalkData = (StaticModelExplodableHelperTreeWalkData*)nlMalloc(8, 8, false);

    if (pWalkData != NULL)
    {
        pNode = ((StaticModelExplodableWorldHelperMapView*)pWorld)->mRoot;
        pWalkData->mStack = (AVLTreeEntry<unsigned long, HelperObject*>**)nlMalloc((((StaticModelExplodableWorldHelperMapView*)pWorld)->mNumElements + 1) * 4, 8, false);
        pWalkData->mCount = 0;

        if (pNode != NULL)
        {
            while (((AVLTreeEntry<unsigned long, HelperObject*>*)pNode)->node.left != NULL)
            {
                pWalkData->mStack[pWalkData->mCount] = (AVLTreeEntry<unsigned long, HelperObject*>*)pNode;
                pWalkData->mCount++;
                pNode = ((AVLTreeEntry<unsigned long, HelperObject*>*)pNode)->node.left;
            }

            pWalkData->mStack[pWalkData->mCount] = (AVLTreeEntry<unsigned long, HelperObject*>*)pNode;
            pWalkData->mCount++;
        }
    }

    SlotPool<SidelineExplodableNode>* pPool = &SidelineExplodableNode::sSidelineExplodableNodeSlotPool;
    SidelineExplodableNode** pTail = &sStaticModelExplodableList.m_pEnd;

    while (pWalkData->mCount != 0)
    {
        HelperObject* helper = pWalkData->mStack[pWalkData->mCount - 1]->value;

        if (pWorld->CompareNameToGenericName(helper->m_szName, "bench1") == 0)
        {
            pNode = NULL;
            if (pPool->m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&SidelineExplodableNode::sSidelineExplodableNodeSlotPool, 8);
            }

            SlotPoolEntry* pEntry = pPool->m_FreeList;
            if (pEntry != NULL)
            {
                pNode = pEntry;
                pPool->m_FreeList = pEntry->m_next;
            }

            if (pNode != NULL)
            {
                ((SidelineExplodableNode*)pNode)->mpExplodable = NULL;
                ((SidelineExplodableNode*)pNode)->next = NULL;
            }

            StaticModelExplodable* pExplodable = (StaticModelExplodable*)nlMalloc(0x74, 8, false);
            if (pExplodable != NULL)
            {
                pExplodable = __ct__21StaticModelExplodableF29StaticModelExplodableCategoryRC9nlMatrix4(
                    pExplodable, EXPLODABLE_BENCH1, helper->m_worldMatrix);
            }

            ((SidelineExplodableNode*)pNode)->mpExplodable = pExplodable;
            nlListAddEnd<SidelineExplodableNode>(&sStaticModelExplodableList.m_pStart, pTail, (SidelineExplodableNode*)pNode);
        }

        if (pWorld->CompareNameToGenericName(helper->m_szName, "bench2") == 0)
        {
            pNode = NULL;
            if (pPool->m_FreeList == NULL)
            {
                SlotPoolBase::BaseAddNewBlock(&SidelineExplodableNode::sSidelineExplodableNodeSlotPool, 8);
            }

            SlotPoolEntry* pEntry = pPool->m_FreeList;
            if (pEntry != NULL)
            {
                pNode = pEntry;
                pPool->m_FreeList = pEntry->m_next;
            }

            if (pNode != NULL)
            {
                ((SidelineExplodableNode*)pNode)->mpExplodable = NULL;
                ((SidelineExplodableNode*)pNode)->next = NULL;
            }

            StaticModelExplodable* pExplodable = (StaticModelExplodable*)nlMalloc(0x74, 8, false);
            if (pExplodable != NULL)
            {
                pExplodable = __ct__21StaticModelExplodableF29StaticModelExplodableCategoryRC9nlMatrix4(
                    pExplodable, EXPLODABLE_BENCH2, helper->m_worldMatrix);
            }

            ((SidelineExplodableNode*)pNode)->mpExplodable = pExplodable;
            nlListAddEnd<SidelineExplodableNode>(&sStaticModelExplodableList.m_pStart, pTail, (SidelineExplodableNode*)pNode);
        }

        pWalkData->mCount--;
        pNode = (void*)pWalkData->mStack[pWalkData->mCount]->node.right;

        if (pNode != NULL)
        {
            while (((AVLTreeEntry<unsigned long, HelperObject*>*)pNode)->node.left != NULL)
            {
                pWalkData->mStack[pWalkData->mCount] = (AVLTreeEntry<unsigned long, HelperObject*>*)pNode;
                pWalkData->mCount++;
                pNode = ((AVLTreeEntry<unsigned long, HelperObject*>*)pNode)->node.left;
            }

            pWalkData->mStack[pWalkData->mCount] = (AVLTreeEntry<unsigned long, HelperObject*>*)pNode;
            pWalkData->mCount++;
        }
    }

    if (pWalkData != NULL)
    {
        delete[] (u8*)pWalkData->mStack;
        delete (u8*)pWalkData;
    }
}

/**
 * Offset/Address/Size: 0x500 | 0x801659C4 | size: 0xAC
 */
void StaticModelExplodable::CleanUp()
{
    long zero = 0;
    SlotPoolBase* pPool = &SidelineExplodableNode::sSidelineExplodableNodeSlotPool;
    SidelineExplodableNode** pTail = &sStaticModelExplodableList.m_pEnd;
    SidelineExplodableNode* node;
    while ((node = sStaticModelExplodableList.m_pStart) != NULL)
    {
        nlListRemoveStart<SidelineExplodableNode>(&sStaticModelExplodableList.m_pStart, pTail);
        SidelineExplodable* pExplodable = node->mpExplodable;
        if (pExplodable != NULL)
        {
            SidelineExplodableManager::RemoveSidelineExplodable(pExplodable);
            delete node->mpExplodable;
        }
        node->mpExplodable = (SidelineExplodable*)zero;
        ((SlotPoolEntry*)node)->m_next = pPool->m_FreeList;
        pPool->m_FreeList = (SlotPoolEntry*)node;
    }
}

/**
 * Offset/Address/Size: 0xC4 | 0x80165588 | size: 0x43C
 * TODO: 95.9% match - r30/r31 register swap for worldMatrix and bIsModelLoaded
 */
StaticModelExplodable::StaticModelExplodable(StaticModelExplodableCategory category, const nlMatrix4& worldMatrix)
{
    mCategory = category;
    mWorldMatrix = worldMatrix;

    if (!bIsModelLoaded[category])
    {
        sCategoryData[category].LoadGeometry();
        bIsModelLoaded[category] = 1;
    }

    Initialize(GetCategoryData().mNumFragmentModels);

    DrawableObject* drawable = s_World__12WorldManager->FindDrawableObject(GetCategoryData().mUnexplodedModel);
    drawable->m_uObjectFlags &= ~1u;
    m_pUnexplodedModel = drawable->Clone();

    static int cloneCount = 0;

    BasicString<char, Detail::TempStringAllocator> name = Format(BasicString<char, Detail::TempStringAllocator>("UndestroyedModelClone_{0}"), cloneCount);

    unsigned long hash = nlStringHash(name.c_str());
    s_World__12WorldManager->AddDrawableObject(hash, m_pUnexplodedModel);

    const nlMatrix4& wm = GetWorldMatrix();
    m_pUnexplodedModel->m_worldMatrix = wm;

    m_pUnexplodedModel->m_uObjectFlags |= 1;

    cloneCount++;
}

/**
 * Offset/Address/Size: 0x64 | 0x80165528 | size: 0x60
 */
StaticModelExplodable::~StaticModelExplodable()
{
}

/**
 * Offset/Address/Size: 0x4C | 0x80165510 | size: 0x18
 */
ExplodableCategoryData& StaticModelExplodable::GetCategoryData() const
{
    return sCategoryData[mCategory];
}

/**
 * Offset/Address/Size: 0x44 | 0x80165508 | size: 0x8
 */
const nlMatrix4& StaticModelExplodable::GetWorldMatrix() const
{
    return mWorldMatrix;
}

/**
 * Offset/Address/Size: 0x18 | 0x801654DC | size: 0x2C
 */
void StaticModelExplodable::SetUnexplodedModelVisibility(bool visible)
{
    DrawableObject* obj = m_pUnexplodedModel;
    if (visible)
    {
        obj->m_uObjectFlags |= 1;
    }
    else
    {
        obj->m_uObjectFlags &= ~1;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801654C4 | size: 0x18
 */
bool StaticModelExplodable::LoadGeometry()
{
    bIsModelLoaded[0] = 0;
    bIsModelLoaded[1] = 0;
    return true;
}
