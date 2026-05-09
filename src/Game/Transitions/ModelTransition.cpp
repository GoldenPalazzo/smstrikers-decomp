#include "Game/Transitions/ModelTransition.h"

#include "NL/nlMath.h"
#include "NL/nlMemory.h"
#include "NL/nlSlotPool.h"
#include "NL/nlFile.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/nlAVLTree.h"

#include "PowerPC_EABI_Support/MSL_C/MSL_Common/stdlib.h"
#include "Game/SAnim/pnSAnimController.h"
#include "Game/Effects/EffectsGroup.h"
#include "Game/Effects/EmissionManager.h"
#include "NL/gl/gl.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/glx/glxLoadModel.h"
#include "NL/glx/glxDisplayList.h"
#include "Game/GL/gluMeshWriter.h"

extern "C" cSHierarchy* Initialize__11cSHierarchyFP7nlChunk(nlChunk*);

struct TransitionModelStore
{
    glModel* pModels;
    u32 nModels;
};

static nlAVLTree<unsigned long, TransitionModelStore, DefaultKeyCompare<unsigned long> > g_ModelInventory;

struct ModelInventoryFindHelper
{
    AVLTreeEntry<unsigned long, TransitionModelStore>* m_Root;

    inline bool FindGet(unsigned long key, TransitionModelStore** foundValue) const
    {
        AVLTreeEntry<unsigned long, TransitionModelStore>* node = m_Root;
        while (node != NULL)
        {
            int cmpResult;
            if (key == node->key)
                cmpResult = 0;
            else if (key < node->key)
                cmpResult = -1;
            else
                cmpResult = 1;
            if (cmpResult == 0)
            {
                if (foundValue != NULL)
                    *foundValue = &node->value;
                return true;
            }
            else
            {
                if (cmpResult < 0)
                    node = (AVLTreeEntry<unsigned long, TransitionModelStore>*)node->node.left;
                else
                    node = (AVLTreeEntry<unsigned long, TransitionModelStore>*)node->node.right;
            }
        }
        return false;
    }
};

static inline void CreateInstance(ModeledScreenTransition* self, TransitionModelStore& modelInfo)
{
    self->m_nModels = modelInfo.nModels;
    self->m_pModels = (glModel*)glModelDupArrayNoStreams(modelInfo.pModels, modelInfo.nModels, false, true);

    for (unsigned long j = 0; j < self->m_nModels; j++)
    {
        for (unsigned long i = 0; i < self->m_pModels[i].numPackets; i++)
        {
            self->m_pModels[j].packets[i].userData = 0;
        }
    }
}

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
    static nlVector3 sZeroInit;
    static nlVector3 sZeroClear;

    Vector<nlVector3, DefaultAllocator> outline;
    outline.mData = NULL;
    outline.mSize = 0;
    outline.mCapacity = 0;

    if (outline.mCapacity < 8)
    {
        nlVector3* newData = (nlVector3*)nlMalloc(8 * sizeof(nlVector3), 8, false);
        newData[0] = sZeroInit;
        newData[1] = sZeroInit;
        newData[2] = sZeroInit;
        newData[3] = sZeroInit;
        newData[4] = sZeroInit;
        newData[5] = sZeroInit;
        newData[6] = sZeroInit;
        newData[7] = sZeroInit;

        for (int i = 0; i < outline.mSize; i++)
        {
            newData[i] = outline.mData[i];
        }

        delete[] outline.mData;
        outline.mData = newData;
        outline.mCapacity = 8;
    }

    glModel* pModel = m_pModels;
    int* pModelMap = m_pModelMap;

    for (int i = 0; i < (int)m_nModels; i++)
    {
        int packetOffset = 0;
        for (int iPacket = 0; iPacket < (int)pModel->numPackets; iPacket++)
        {
            const glModelPacket& packet = *(const glModelPacket*)((u8*)pModel->packets + packetOffset);
            DisplayList* pList = dlGetStruct(packet.indexBuffer);

            for (int iVertex = 0; iVertex < (int)packet.numVertices; iVertex++)
            {
                const u16* p;
                if (((u16*)&pList->indices)[1] != 0)
                {
                    u16 ns = ((u16*)&pList->indices)[0];
                    p = (u16*)((u8*)pList->list + (((ns - 1) * 2 + 1) * iVertex) + 4);
                }
                else
                {
                    u16 ns = ((u16*)&pList->indices)[0];
                    p = (u16*)((u8*)pList->list + ((ns * 2) * iVertex) + 3);
                }

                int index = *p;
                const glModelStream& stream = packet.streams[0];

                nlVector3 current;
                if (stream.stride == 12)
                {
                    memcpy(&current, (const void*)((u8*)stream.address + index * stream.stride), 12);
                }
                else
                {
                    const s16* s = (const s16*)((u8*)stream.address + index * stream.stride);
                    current.f.x = s[0] * 0.0078125f;
                    current.f.y = s[1] * 0.0078125f;
                    current.f.z = s[2] * 0.0078125f;
                }

                nlMultPosVectorMatrix(current, current, m_pPoseAccumulator->GetNodeMatrix(pModelMap[0]));

                outline.insert(outline.mData + outline.mSize, &current, &current + 1);
            }

            ShuffleIntoOutline(outline);

            GLMeshWriter mesh;
            glSetDefaultState(true);
            glSetCurrentMatrix(glGetIdentityMatrix());
            glSetCurrentTexture(glGetTexture("global/white"), GLTT_Diffuse);
            glSetCurrentProgram(glGetProgram("3d unlit"));
            glSetRasterState((eGLState)5, 1);
            glSetCurrentRasterState(glHandleizeRasterState());

            eGLStream streamDecl[3] = {
                GLStream_Position,
                GLStream_Colour,
                GLStream_Diffuse,
            };

            mesh.Begin(outline.mSize + 1, GLP_LineStrip, 3, streamDecl, false);

            for (int k = 0; k < outline.mSize; k++)
            {
                mesh.Colour(m_OutlineColour);
                nlVector2 uv;
                uv.f.x = 0.0f;
                uv.f.y = 0.0f;
                ((GLMeshWriterCore*)&mesh)->Texcoord(uv);
                mesh.Vertex(outline.mData[k]);
            }

            mesh.Colour(m_OutlineColour);
            nlVector2 uv;
            uv.f.x = 0.0f;
            uv.f.y = 0.0f;
            ((GLMeshWriterCore*)&mesh)->Texcoord(uv);
            mesh.Vertex(outline.mData[0]);

            if (mesh.End())
            {
                glViewAttachModel(GLV_Transitions3D, 2, mesh.GetModel());
            }

            for (int k = 0; k < outline.mSize; k++)
            {
                outline.mData[k] = sZeroClear;
            }
            outline.mSize = 0;

            packetOffset += sizeof(glModelPacket);
        }

        pModelMap++;
        pModel = (glModel*)((u8*)pModel + sizeof(glModel));
    }

    delete[] outline.mData;
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
 * TODO: 92.66% match - branch offset diffs remain (consistent 8-byte shift in compiled output)
 */
ModeledScreenTransition* ModeledScreenTransition::LoadFromParser(SimpleParser* parser)
{
    char* pToken = parser->NextToken(true);

    while (pToken != NULL)
    {
        if (nlStrCmp(pToken, "texture") == 0)
        {
            m_nTexture = glHash(parser->NextTokenOnLine(true));
        }
        else if (nlStrCmp(pToken, "name") == 0)
        {
            pToken = parser->NextTokenOnLine(true);
            u32 hash;
            TransitionModelStore* pModelStore;
            u32 fileSize = 0;
            char buf[128];
            hash = glHash(pToken);

            if (((ModelInventoryFindHelper*)&g_ModelInventory.m_Root)->FindGet(hash, &pModelStore))
            {
                CreateInstance(this, *pModelStore);
            }
            else
            {
                glSetIgnoreDuplicateModels(true);

                nlSNPrintf(buf, 128, "transitions/%s.glg", pToken);
                m_pModels = glLoadModel(buf, &m_nModels);

                glSetIgnoreDuplicateModels(false);

                TransitionModelStore newStore;
                newStore.pModels = m_pModels;
                newStore.nModels = m_nModels;
                AVLTreeNode* existingNode;
                g_ModelInventory.AddAVLNode(&((AVLTreeNode*&)g_ModelInventory.m_Root), (void*)&hash, (void*)&newStore, &existingNode, g_ModelInventory.m_NumElements);
                if (existingNode == NULL)
                {
                    g_ModelInventory.m_NumElements++;
                }
            }

            nlSNPrintf(buf, 128, "art/transitions/%s.sanim", pToken);
            m_pAnimFile = (char*)nlLoadEntireFile(buf, &fileSize, 0x20, AllocateStart);
            m_pAnim = cSAnim::Initialize((nlChunk*)m_pAnimFile);

            nlSNPrintf(buf, 128, "art/transitions/%s.shier", pToken);
            m_pSkelFile = (char*)nlLoadEntireFile(buf, &fileSize, 0x20, AllocateStart);
            m_pSkeleton = Initialize__11cSHierarchyFP7nlChunk((nlChunk*)m_pSkelFile);

            m_pModelMap = (int*)nlMalloc(m_nModels * 4, 8, false);
            for (u32 i = 0; i < m_nModels; i++)
            {
                m_pModelMap[i] = m_pSkeleton->GetNodeIndexByID(m_pModels[i].id);
            }
        }
        else if (nlStrCmp(pToken, "screengrab") == 0)
        {
            m_bScreenGrab = true;
        }
        else if (nlStrCmp(pToken, "effect") == 0)
        {
            nlStrNCpy(m_EffectName, parser->NextTokenOnLine(true), 64);
        }
        else if (nlStrCmp(pToken, "outline") == 0)
        {
            m_RenderOutline = true;
        }
        else if (nlStrCmp(pToken, "outline_colour") == 0)
        {
            m_OutlineColour.c[0] = atoi(parser->NextTokenOnLine(true));
            m_OutlineColour.c[1] = atoi(parser->NextTokenOnLine(true));
            m_OutlineColour.c[2] = atoi(parser->NextTokenOnLine(true));
            m_OutlineColour.c[3] = atoi(parser->NextTokenOnLine(true));
        }
        else if (nlStrCmp(pToken, "program") == 0)
        {
            const char* effect = parser->NextTokenOnLine(true);
            char buf[128];
            nlStrNCpy(buf, effect, 128);
            for (int i = 0; buf[i] != '\0'; i++)
            {
                if (buf[i] == '_')
                    buf[i] = ' ';
            }
            m_nProgram = glGetProgram(buf);
        }
        else if (nlStrCmp(pToken, "light") == 0)
        {
            m_pLight = new (nlMalloc(sizeof(TransitionLight), 8, false)) TransitionLight();
            m_pLight->LoadFromParser(parser);
        }
        else if (nlStrCmp(pToken, "end") == 0)
        {
            break;
        }

        pToken = parser->NextToken(true);
    }

    m_pPoseAccumulator = new (nlMalloc(0x58, 8, false)) cPoseAccumulator(m_pSkeleton, false);

    u64 savedTextureState = glGetCurrentTextureState();
    u32 savedRasterState = glGetCurrentRasterState();

    for (u32 i = 0; i < m_nModels; i++)
    {
        if (m_pLight != NULL)
        {
            m_pLight->AttachToModel(&m_pModels[i]);
        }

        for (u32 j = 0; j < m_pModels[i].numPackets; j++)
        {
            glSetCurrentTextureState(m_pModels[i].packets[j].state.texturestate);
            glSetTextureState(GLTS_DiffuseWrap, 3);

            glSetCurrentRasterState(m_pModels[i].packets[j].state.raster);
            glSetRasterState(GLS_AlphaBlend, 0);
            glSetRasterState(GLS_Culling, 0);
            glSetRasterState(GLS_DepthTest, 1);
            glSetRasterState(GLS_DepthWrite, 1);

            glSetCurrentProgram(m_nProgram);

            if (m_nTexture != 0xFFFFFFFF)
            {
                m_pModels[i].packets[j].state.texture[0] = m_nTexture;
            }

            m_pModels[i].packets[j].state.raster = glHandleizeRasterState();

            u64 texHandle = glHandleizeTextureState();
            m_pModels[i].packets[j].state.texturestate = texHandle;

            m_pModels[i].packets[j].state.program = m_nProgram;
        }
    }

    glSetCurrentTextureState(savedTextureState);
    glSetCurrentRasterState(savedRasterState);
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
