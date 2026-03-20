#include "Game/Transitions/ModelTransition.h"

#include "NL/nlMath.h"
#include "NL/nlMemory.h"
#include "NL/nlSlotPool.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionManager.h"
#include "NL/gl/gl.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"

eGLView ModeledScreenTransition::s_3DView = GLV_Transitions;

// /**
//  * Offset/Address/Size: 0xA04 | 0x80204CA0 | size: 0x24
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::DeleteEntry(AVLTreeEntry<unsigned long, TransitionModelStore>*)
// {
// }

// /**
//  * Offset/Address/Size: 0x998 | 0x80204C34 | size: 0x6C
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::AllocateEntry(void*, void*)
// {
// }

// /**
//  * Offset/Address/Size: 0x96C | 0x80204C08 | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::CompareKey(void*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x940 | 0x80204BDC | size: 0x2C
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::CompareNodes(AVLTreeNode*, AVLTreeNode*)
// {
// }

// /**
//  * Offset/Address/Size: 0x938 | 0x80204BD4 | size: 0x8
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::CastUp(AVLTreeNode*) const
// {
// }

// /**
//  * Offset/Address/Size: 0x1E0 | 0x8020447C | size: 0x758
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::PostorderTraversal(AVLTreeEntry<unsigned long, TransitionModelStore>*, void (AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, TransitionModelStore>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x17C | 0x80204418 | size: 0x64
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::DestroyTree(void (AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::*)(AVLTreeEntry<unsigned long, TransitionModelStore>*))
// {
// }

// /**
//  * Offset/Address/Size: 0x124 | 0x802043C0 | size: 0x58
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::Clear()
// {
// }

// /**
//  * Offset/Address/Size: 0xC8 | 0x80204364 | size: 0x5C
//  */
// void AVLTreeBase<unsigned long, TransitionModelStore, NewAdapter<AVLTreeEntry<unsigned long, TransitionModelStore> >, DefaultKeyCompare<unsigned long> >::~AVLTreeBase()
// {
// }

// /**
//  * Offset/Address/Size: 0x68 | 0x80204304 | size: 0x60
//  */
// void nlAVLTree<unsigned long, TransitionModelStore, DefaultKeyCompare<unsigned long> >::~nlAVLTree()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80204294 | size: 0x8
//  */
// void nlMatrix4::GetTranslation() const
// {
// }

// /**
//  * Offset/Address/Size: 0x50 | 0x80204290 | size: 0x4
//  */
// void ScreenTransition::DoSanityCheck()
// {
// }

// /**
//  * Offset/Address/Size: 0x8 | 0x80204248 | size: 0x48
//  */
// ScreenTransition::~ScreenTransition()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x80204240 | size: 0x8
//  */
// void ScreenTransition::CutTime() const
// {
// }

/**
 * Offset/Address/Size: 0x1E60 | 0x80203F24 | size: 0x31C
 */
int GetNumLeafNodesInHierarchy(cSHierarchy& hierarchy, int nodeIndex, int count)
{
    if (hierarchy.GetNumChildren(nodeIndex) == 0)
    {
        return count + 1;
    }

    for (int p = 0; p < hierarchy.GetNumChildren(nodeIndex); p++)
    {
        count = GetNumLeafNodesInHierarchy(hierarchy, hierarchy.GetChild(nodeIndex, p), count);
    }

    return count;
}

/**
 * Offset/Address/Size: 0x1BCC | 0x80203C90 | size: 0x294
 * TODO: 98.88% match - remaining callee-saved GPR allocation drift in loop-carried
 * index/offset registers (polygon base and outer-loop counters).
 */
void ShuffleIntoOutline(Vector<nlVector3, DefaultAllocator>& polygon)
{
    float min = 9999.0f;

    for (int i = 1; i < polygon.mSize; i++)
    {
        float dist = nlGetLengthSquared3D(polygon.mData[0].f.x - polygon.mData[i].f.x, polygon.mData[0].f.y - polygon.mData[i].f.y, polygon.mData[0].f.z - polygon.mData[i].f.z);

        if (dist < min)
        {
            nlVector3 tmp = polygon.mData[i];
            min = dist;
            polygon.mData[i] = polygon.mData[1];
            polygon.mData[1] = tmp;
        }
    }

    nlVector3 dir;

    for (int i = 1; i < polygon.mSize - 1; i++)
    {
        float max = 1.0f;
        nlRecipSqrt(dir.f.x * dir.f.x + dir.f.y * dir.f.y + dir.f.z * dir.f.z, true);

        int prev = i;
        prev -= 1;
        nlVec3Set(dir, polygon.mData[i].f.x - polygon.mData[prev].f.x, polygon.mData[i].f.y - polygon.mData[prev].f.y, polygon.mData[i].f.z - polygon.mData[prev].f.z);

        int next = i;
        next += 1;

        for (int j = next; j < polygon.mSize; j++)
        {
            float x, y, z;
            y = polygon.mData[i].f.y - polygon.mData[j].f.y;
            x = polygon.mData[i].f.x - polygon.mData[j].f.x;
            z = polygon.mData[i].f.z - polygon.mData[j].f.z;

            float recip = nlRecipSqrt(x * x + y * y + z * z, true);
            float dot = dir.f.x * (recip * x) + dir.f.y * (recip * y) + dir.f.z * (recip * z);

            if (dot <= max)
            {
                max = dot;
                nlVector3 tmp = polygon.mData[next];
                polygon.mData[next] = polygon.mData[j];
                polygon.mData[j] = tmp;
            }
        }
    }
}

/**
 * Offset/Address/Size: 0x188C | 0x80203950 | size: 0x340
 */
inline int UpdateEffectsFromLeafNodes(cPoseAccumulator& pa, EmissionController** ecs, cSHierarchy& skeleton, int leaf, int node)
{
    if (skeleton.GetNumChildren(node) == 0)
    {
        EmissionController* ec = ecs[leaf];
        if (ec != NULL)
        {
            if (EmissionManager::IsStillAlive(ec))
            {
                ecs[leaf]->SetPosition(pa.GetNodeMatrix(node).GetTranslation());
            }
            else
            {
                ecs[leaf] = NULL;
            }
        }
        leaf++;
    }
    else
    {
        for (int i = 0; i < skeleton.GetNumChildren(node); i++)
        {
            leaf = UpdateEffectsFromLeafNodes(pa, ecs, skeleton, leaf, skeleton.GetChild(node, i));
        }
    }

    return leaf;
}

/**
 * Offset/Address/Size: 0x17E0 | 0x802038A4 | size: 0xAC
 */
ModeledScreenTransition::ModeledScreenTransition()
{
    m_pModels = NULL;
    m_nModels = 0;
    m_pSkeleton = NULL;
    m_pAnim = NULL;
    m_pSkelFile = NULL;
    m_pAnimFile = NULL;
    m_nTexture = 0xFFFFFFFF;
    m_pPoseAccumulator = NULL;
    m_pPoseTree = NULL;
    m_bScreenGrab = false;
    m_bEnableGrab = false;
    m_pModelMap = NULL;
    m_pLight = NULL;
    m_nProgram = glGetProgram("3d unlit");
    m_Effects = NULL;
    m_RenderOutline = false;
    m_mWorldMatrix.SetIdentity();
    m_EffectName[0] = '\0';
}

/**
 * Offset/Address/Size: 0x15C8 | 0x8020368C | size: 0x218
 */
ModeledScreenTransition::~ModeledScreenTransition()
{
    if (m_pPoseAccumulator != NULL)
    {
        delete m_pPoseAccumulator;
        m_pPoseAccumulator = NULL;
    }

    if (m_pLight != NULL)
    {
        for (u32 i = 0; i < m_nModels; i++)
        {
            m_pLight->DetachFromModel(&m_pModels[i]);
        }

        if (m_pLight != NULL)
        {
            delete m_pLight;
        }
        m_pLight = NULL;
    }

    if (m_pPoseTree != NULL)
    {
        delete m_pPoseTree;
        m_pPoseTree = NULL;
    }

    if (m_pSkeleton != NULL)
    {
        m_pSkeleton = NULL;
        delete[] m_pSkelFile;
        m_pSkelFile = NULL;
    }

    if (m_pAnim != NULL)
    {
        m_pAnim->Destroy();
        m_pAnim = NULL;
        delete[] m_pAnimFile;
        m_pAnimFile = NULL;
    }

    if (m_pModelMap != NULL)
    {
        delete[] m_pModelMap;
        m_pModelMap = NULL;
    }

    if (m_Effects != NULL)
    {
        delete[] m_Effects;
    }

    m_pModels = NULL;
    m_nModels = 0;
}

/**
 * Offset/Address/Size: 0x15C4 | 0x80203688 | size: 0x4
 */
void ModeledScreenTransition::DoSanityCheck()
{
}

/**
 * Offset/Address/Size: 0x1520 | 0x802035E4 | size: 0xA4
 */
void ModeledScreenTransition::Update(float dt)
{
    if (m_pPoseTree != NULL)
    {
        bool skipUpdate = false;
        if (m_pPoseTree->m_ePlayMode == PM_HOLD && m_pPoseTree->m_fTime == 1.0f)
        {
            skipUpdate = true;
        }

        if (!skipUpdate)
        {
            m_pPoseTree->Update(dt);
            m_pPoseAccumulator->Pose(*m_pPoseTree, m_mWorldMatrix);
        }
    }

    if (m_EffectName[0] != '\0')
    {
        UpdateEffectsFromLeafNodes(*m_pPoseAccumulator, m_Effects, *m_pSkeleton, 0, 0);
    }
}

/**
 * Offset/Address/Size: 0x13EC | 0x802034B0 | size: 0x134
 * TODO: 99.29% match - remaining r26/r29/r30 register cycle between modelOffset, pNodeMatrix, and matrixHandle.
 */
void ModeledScreenTransition::Render(eGLView)
{
    const nlMatrix4* pNodeMatrix;
    u32 matrixHandle;

    if (m_pLight != NULL && m_pPoseTree != NULL)
    {
        m_pLight->ApplyLight(m_pPoseTree->m_fTime);
    }

    int modelMapIndex = 0;
    int modelOffset = 0;

    for (u32 modelIndex = 0; modelIndex < m_nModels; modelIndex++)
    {
        for (u32 packetIndex = 0; packetIndex < m_pModels[modelOffset / 0x10].numPackets; packetIndex++)
        {
            pNodeMatrix = &m_pPoseAccumulator->GetNodeMatrix(m_pModelMap[modelMapIndex]);
            matrixHandle = glAllocMatrix();
            if (matrixHandle != 0xFFFFFFFF)
            {
                glSetMatrix(matrixHandle, *pNodeMatrix);
            }
            m_pModels[modelOffset / 0x10].packets[packetIndex].state.matrix = matrixHandle;
        }

        glViewAttachModel(s_3DView, &m_pModels[modelOffset / 0x10]);
        modelMapIndex++;
        modelOffset += 0x10;
    }

    if (m_bEnableGrab)
    {
        glViewSetFilter(GLV_ScreenGrab, GLFilter_Blt);
        glViewSetFilterSource(GLV_ScreenGrab, GLTG_Main);
        m_bEnableGrab = false;
    }
    else
    {
        glViewSetFilter(GLV_ScreenGrab, GLFilter_None);
    }

    if (m_RenderOutline)
    {
        RenderOutline();
    }
}

/**
 * Offset/Address/Size: 0x918 | 0x802029DC | size: 0xAD4
 */
void ModeledScreenTransition::RenderOutline() const
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x8DC | 0x802029A0 | size: 0x3C
 */
bool ModeledScreenTransition::IsFinished()
{
    if (!m_pPoseTree)
    {
        return true;
    }

    return m_pPoseTree->m_ePlayMode == PM_HOLD && m_pPoseTree->m_fTime == 1.0f;
}

/**
 * Offset/Address/Size: 0x8C0 | 0x80202984 | size: 0x1C
 */
float ModeledScreenTransition::Time() const
{
    if (m_pPoseTree != NULL)
    {
        return m_pPoseTree->m_fTime;
    }
    return 0.0f;
}

/**
 * Offset/Address/Size: 0x76C | 0x80202830 | size: 0x154
 */
void ModeledScreenTransition::Reset()
{
    cPN_SAnimController* poseTree;
    cPN_SAnimController* controller;

    if (m_pPoseTree == NULL && m_pAnim != NULL)
    {
        controller = AllocateSAnimController();
        controller = new (controller) cPN_SAnimController(m_pAnim, NULL, PM_HOLD, NULL, 0, false);
        m_pPoseTree = controller;
    }

    poseTree = m_pPoseTree;
    if (poseTree != NULL)
    {
        poseTree->m_fPrevTime = poseTree->m_fTime;
        poseTree->m_fTime = 0.0f;
    }

    m_bEnableGrab = m_bScreenGrab;

    if (m_EffectName[0] != '\0')
    {
        const int numLeafNodes = GetNumLeafNodesInHierarchy(*m_pSkeleton, 0, 0);
        if (m_Effects == NULL)
        {
            m_Effects = (EmissionController**)nlMalloc(numLeafNodes * sizeof(EmissionController*), 8, false);
        }

        for (int i = 0; i < numLeafNodes; i++)
        {
            m_Effects[i] = EmissionManager::Create(fxGetGroup(m_EffectName), 0);
            m_Effects[i]->m_GlView = GLV_Transitions3D;
        }
    }
}

/**
 * Offset/Address/Size: 0x70C | 0x802027D0 | size: 0x60
 */
void ModeledScreenTransition::Cancel()
{
    delete m_pPoseTree;
    m_pPoseTree = NULL;

    delete[] m_Effects;
    m_Effects = NULL;
}

/**
 * Offset/Address/Size: 0x44 | 0x80202108 | size: 0x6C8
 */
ModeledScreenTransition* ModeledScreenTransition::LoadFromParser(SimpleParser*)
{
    return nullptr;
}

/**
 * Offset/Address/Size: 0x0 | 0x802020C4 | size: 0x44
 */
float ModeledScreenTransition::GetTransitionLength()
{
    if (m_pAnim == NULL)
    {
        return 0.0f;
    }
    return m_pAnim->m_nNumKeys / 30.0f;
}
