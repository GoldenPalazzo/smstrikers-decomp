#include "Game/Render/AnimatedModelExplodable.h"

nlList<SidelineExplodableNode> AnimatedModelExplodable::sAnimatedModelExplodableList;
ExplodableCategoryData AnimatedModelExplodable::sCategoryData[NUM_ANIMATED_MODEL_EXPLODABLE_CATEGORIES];
u8 AnimatedModelExplodable::bIsModelLoaded[2];

/**
 * Offset/Address/Size: 0x70 | 0x80158C4C | size: 0x128
 */
extern "C" void __sinit_AnimatedModelExplodable_cpp(void)
{
    ExplodableCategoryData data0;
    ExplodableCategoryData data1;

    AnimatedModelExplodable::sAnimatedModelExplodableList.m_pEnd = 0;
    AnimatedModelExplodable::sAnimatedModelExplodableList.m_pStart = 0;

    data0.mBaseModelName = "environment/Sideline_Objects/camera_base";
    data0.mFragmentModelName = "environment/Sideline_Objects/camera_d";
    data0.mUnexplodedModelName = 0;
    data0.mNumFragmentModels = 0;
    data0.mUnexplodedModel = 0;
    data0.mNumStationaryFragments = 0;
    AnimatedModelExplodable::sCategoryData[0] = data0;

    data1.mBaseModelName = "environment/Sideline_Objects/standupcamera_base";
    data1.mFragmentModelName = "environment/Sideline_Objects/standupcamera_d";
    data1.mUnexplodedModelName = 0;
    data1.mNumFragmentModels = 0;
    data1.mUnexplodedModel = 0;
    data1.mNumStationaryFragments = 0;
    AnimatedModelExplodable::sCategoryData[1] = data1;
}

/**
 * Offset/Address/Size: 0x44 | 0x80158C20 | size: 0x2C
 * nlListAddEnd<SidelineExplodableNode> -- defined in NL/nlList.h; instantiated by AnimatedModelExplodable_stub below.
 */

// /**
//  * Offset/Address/Size: 0x0 | 0x80158BDC | size: 0x44
//  */
// void nlListRemoveStart<SidelineExplodableNode>(SidelineExplodableNode**, SidelineExplodableNode**)
// {
// }

/**
 * Offset/Address/Size: 0x1C4 | 0x80158B30 | size: 0xAC
 */
void AnimatedModelExplodable::CleanUp()
{
    long zero = 0;
    SlotPoolBase* pPool = &SidelineExplodableNode::sSidelineExplodableNodeSlotPool;
    SidelineExplodableNode** pTail = &sAnimatedModelExplodableList.m_pEnd;
    SidelineExplodableNode* node;
    while ((node = sAnimatedModelExplodableList.m_pStart) != NULL)
    {
        nlListRemoveStart<SidelineExplodableNode>(&sAnimatedModelExplodableList.m_pStart, pTail);
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
 * Offset/Address/Size: 0xA8 | 0x80158A14 | size: 0x11C
 * TODO: 98.80% match - r29/r30 register swap for pAnimatedNPC and bIsModelLoaded accesses
 */
AnimatedModelExplodable::AnimatedModelExplodable(AnimatedModelExplodableCategory category, SkinAnimatedNPC* pAnimatedNPC)
{
    mpAnimatedNPC = pAnimatedNPC;
    mCategory = category;

    if (!bIsModelLoaded[category])
    {
        sCategoryData[category].LoadGeometry();
        bIsModelLoaded[category] = 1;
    }

    Initialize(GetCategoryData().mNumFragmentModels);

    SidelineExplodableNode* node = NULL;

    if (SidelineExplodableNode::sSidelineExplodableNodeSlotPool.m_FreeList == NULL)
    {
        SlotPoolBase::BaseAddNewBlock(&SidelineExplodableNode::sSidelineExplodableNodeSlotPool, 8);
    }

    SlotPoolEntry* entry = SidelineExplodableNode::sSidelineExplodableNodeSlotPool.m_FreeList;
    if (entry != NULL)
    {
        node = (SidelineExplodableNode*)entry;
        SidelineExplodableNode::sSidelineExplodableNodeSlotPool.m_FreeList = entry->m_next;
    }

    if (node != NULL)
    {
        node->mpExplodable = NULL;
        node->next = NULL;
    }

    node->mpExplodable = this;
    nlListAddEnd<SidelineExplodableNode>(&sAnimatedModelExplodableList.m_pStart, &sAnimatedModelExplodableList.m_pEnd, node);
}

/**
 * Offset/Address/Size: 0x48 | 0x801589B4 | size: 0x60
 */
AnimatedModelExplodable::~AnimatedModelExplodable()
{
}

/**
 * Offset/Address/Size: 0x30 | 0x8015899C | size: 0x18
 */
ExplodableCategoryData& AnimatedModelExplodable::GetCategoryData() const
{
    return sCategoryData[mCategory];
}

/**
 * Offset/Address/Size: 0x24 | 0x80158990 | size: 0xC
 */
const nlMatrix4& AnimatedModelExplodable::GetWorldMatrix() const
{
    return mpAnimatedNPC->mWorldMatrix;
}

/**
 * Offset/Address/Size: 0x18 | 0x80158984 | size: 0xC
 */
void AnimatedModelExplodable::SetUnexplodedModelVisibility(bool isVisible)
{
    mpAnimatedNPC->mbIsVisible = isVisible;
}

/**
 * Offset/Address/Size: 0x0 | 0x8015896C | size: 0x18
 */
bool AnimatedModelExplodable::LoadGeometry()
{
    bIsModelLoaded[0] = 0;
    bIsModelLoaded[1] = 0;
    return true;
}

// Force template instantiation -- REMOVE once real callers exist.
void AnimatedModelExplodable_stub()
{
    nlListAddEnd<SidelineExplodableNode>((SidelineExplodableNode**)0, (SidelineExplodableNode**)0, (SidelineExplodableNode*)0);
}
