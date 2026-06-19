#include "Game/GL/gluSkinMesh.h"
#include "types.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxDisplayList.h"
#include "NL/nlDLRing.h"
#include "NL/nlString.h"
#include "NL/platvmath.h"
#include "dolphin/PPCArch.h"
#include "dolphin/os/OSCache.h"

#define qr0 0

/**
 * Offset/Address/Size: 0x20 | 0x801B64A8 | size: 0x18
 */

/**
 * Offset/Address/Size: 0x38 | 0x801B64C0 | size: 0x18
 */

/**
 * Offset/Address/Size: 0x0 | 0x801B6488 | size: 0x20
 */

/**
 * Offset/Address/Size: 0x310 | 0x801B6480 | size: 0x8
 */
static AVLTreeEntry<unsigned long, unsigned long>* force_inst(
    const AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long> >, DefaultKeyCompare<unsigned long> >* t,
    AVLTreeNode* n)
{
    return t->CastUp(n);
}

// /**
//  * Offset/Address/Size: 0x44 | 0x801B61B4 | size: 0x2CC
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::InorderWalk<TempMatrixCopier>(AVLTreeEntry<unsigned long, unsigned long>*, TempMatrixCopier*, void (TempMatrixCopier::*)(const
// unsigned long&, unsigned long*))
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801B6170 | size: 0x44
//  */
// void AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned
// long>>::Walk<TempMatrixCopier>(TempMatrixCopier*, void (TempMatrixCopier::*)(const unsigned long&, unsigned long*))
// {
// }

/**
 * Offset/Address/Size: 0x550 | 0x801B6094 | size: 0xDC
 */
void ShaderSkinMesh::StitchModel()
{
    int packetIndex = 0;
    glModelPacket* pPacket = pModel->packets;
    for (; pPacket < pModel->packets + pModel->numPackets; packetIndex++, pPacket++)
    {
        if (glGetRasterState(pPacket->state.raster, GLS_SolidOffset) != 1)
            continue;
        DisplayList* dl = dlGetStruct(pPacket->indexBuffer);
        u8* pWrite = (u8*)dl->list;
        if (*(pWrite += 3) != 0xff)
            continue;
        for (int i = 0; i < pPacket->numVertices; i++)
        {
            *pWrite = (stitchArray[packetIndex][i] + 1) * 3;
            pWrite += (pPacket->numStreams - 1) * 2 + 1;
        }
    }
}

/**
 * Offset/Address/Size: 0x4F0 | 0x801B6034 | size: 0x60
 */
void TempMatrixCopier::CopyMatrix(const unsigned long& boneId, unsigned long* outValue)
{
    SkinMatrix& matrix = (SkinMatrix&)m_Mesh->GetPoseMatrix(boneId);
    matrix.Get(m_TempMatrices[*outValue]);
}

/**
 * Offset/Address/Size: 0x0 | 0x801B5B44 | size: 0x4F0
 * TODO: 96.77% match - inner skin-pair loop keeps pair offset/index and normal-scale constants in different registers.
 */
void ShaderSkinMesh::AttachSkinData(unsigned long program, const nlMatrix4* pReflect)
{
    nlVector3* outVertices = NULL;
    nlVector3* outNormals = NULL;
    nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >* boneMap = &nlRingGetStart<BoneMapList>(boneMaps)->boneMap;

    if (boneMap->m_NumElements != 0)
    {
        if (tempMatrices == NULL)
        {
            tempMatrices = (nlMatrix4*)nlMalloc(boneMap->m_NumElements * sizeof(nlMatrix4), 8, false);
        }

        outVertices = (nlVector3*)glFrameAlloc(numSoftwareVerts * sizeof(nlVector3), GLM_VertexData);
        outNormals = (nlVector3*)glFrameAlloc(numSoftwareVerts * sizeof(nlVector3), GLM_VertexData);

        nlZeroMemory(outVertices, numSoftwareVerts * sizeof(nlVector3));
        nlZeroMemory(outNormals, numSoftwareVerts * sizeof(nlVector3));

        float vertexWeight;

        TempMatrixCopier matCopier;
        matCopier.m_TempMatrices = tempMatrices;
        matCopier.m_Mesh = this;

        boneMap->Walk(&matCopier, &TempMatrixCopier::CopyMatrix);

        SkinPairList* curr = nlRingGetStart<SkinPairList>(skinPairs);
        int matrixOffset = 0;

        if (curr != NULL)
        {
            const float invNormalScale = 0.015625f;

            while (true)
            {
                register const nlMatrix4* pMatrix = &tempMatrices[matrixOffset];

                // clang-format off
                if (curr->num != 0) {
                    asm {
                        psq_l f2, 0x0(pMatrix), 0, qr0
                        psq_l f3, 0x8(pMatrix), 0, qr0
                        psq_l f4, 0x10(pMatrix), 0, qr0
                        psq_l f5, 0x18(pMatrix), 0, qr0
                        psq_l f6, 0x20(pMatrix), 0, qr0
                        psq_l f7, 0x28(pMatrix), 0, qr0
                        psq_l f8, 0x30(pMatrix), 0, qr0
                        psq_l f9, 0x38(pMatrix), 0, qr0
                    }
                }
                // clang-format on

                for (int i = 0; i < (int)curr->num; i++)
                {
                    SkinPair& pair = curr->pairs[i];
                    vertexWeight = (float)pair.vertexWeight / 65535.0f;
                    int index = pair.vertexIndex;

                    register const nlVector3* inVertex;
                    if (morphBuffer != NULL)
                    {
                        inVertex = &morphBuffer[index];
                    }
                    else
                    {
                        inVertex = &softwareVertices[index].position;
                    }

                    const signed char* packed = softwareVertices[index].packed_normal;
                    nlVector3 inNormal;
                    inNormal.f.x = (float)packed[0] * invNormalScale;
                    inNormal.f.y = (float)packed[1] * invNormalScale;
                    inNormal.f.z = (float)packed[2] * invNormalScale;

                    register const nlVector3* pInN = &inNormal;
                    register nlVector3* pOutV = &outVertices[index];
                    register nlVector3* pOutN = &outNormals[index];

                    // clang-format off
                    asm {
                        psq_l f17, 0x0(inVertex), 0, qr0
                        psq_l f18, 0x8(inVertex), 1, qr0
                        ps_muls0 f15, f2, f17
                        ps_muls0 f16, f3, f17
                        ps_madds1 f15, f4, f17, f15
                        ps_madds1 f16, f5, f17, f16
                        ps_madds0 f15, f6, f18, f15
                        ps_madds0 f16, f7, f18, f16
                        ps_add f15, f8, f15
                        ps_add f16, f9, f16
                        psq_l f0, 0x0(pInN), 0, qr0
                        psq_l f1, 0x8(pInN), 1, qr0
                        ps_muls0 f10, f2, f0
                        ps_muls0 f11, f3, f0
                        ps_madds1 f10, f4, f0, f10
                        ps_madds1 f11, f5, f0, f11
                        ps_madds0 f10, f6, f1, f10
                        ps_madds0 f11, f7, f1, f11
                        psq_l f19, 0x0(pOutV), 0, qr0
                        psq_l f20, 0x8(pOutV), 1, qr0
                        psq_l f13, 0x0(pOutN), 0, qr0
                        psq_l f14, 0x8(pOutN), 1, qr0
                        lfs f12, vertexWeight
                        ps_madds0 f15, f15, f12, f19
                        ps_madds0 f16, f16, f12, f20
                        ps_madds0 f10, f10, f12, f13
                        ps_madds0 f11, f11, f12, f14
                        psq_st f10, 0x0(pOutN), 0, qr0
                        psq_st f11, 0x8(pOutN), 1, qr0
                        psq_st f15, 0x0(pOutV), 0, qr0
                        psq_st f16, 0x8(pOutV), 1, qr0
                    }
                    // clang-format on
                }

                if (nlRingIsEnd<SkinPairList>(skinPairs, curr))
                {
                    break;
                }

                curr = curr->m_next;
                matrixOffset++;
            }
        }

        DCFlushRangeNoSync(outVertices, numSoftwareVerts * sizeof(nlVector3));
        DCFlushRangeNoSync(outNormals, numSoftwareVerts * sizeof(nlVector3));
        PPCSync();
    }

    unsigned long matrix;
    if (pReflect == NULL)
    {
        matrix = glGetIdentityMatrix();
    }
    else
    {
        matrix = glAllocMatrix();
        if (matrix != 0xFFFFFFFF)
        {
            glSetMatrix(matrix, *pReflect);
        }
    }

    BoneMapList* mapList = nlRingGetStart<BoneMapList>(boneMaps)->m_next;
    glModelPacket* pPacket = pModel->packets;

    while (pPacket < pModel->packets + pModel->numPackets)
    {
        pPacket->state.matrix = matrix;

        if (program != 0xFFFFFFFF)
        {
            pPacket->state.program = program;
        }

        if (glGetRasterState(pPacket->state.raster, GLS_SolidOffset) == 1)
        {
            glUserAttach(MakeUserData(&mapList->boneMap), pPacket, false);
        }
        else
        {
            pPacket->streams[0].address = (u32)outVertices;
            pPacket->streams[1].address = (u32)outNormals;
            pPacket->streams[1].stride = 0xC;
        }

        mapList = mapList->m_next;
        pPacket++;
    }
}

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler lays out the related fields to match the original binary.
 */
/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler
 * lays out the related fields to match the original binary.
 */
void gluSkinMesh_stub()
{
    bool (*volatile forceNlRingIsEnd)(SkinPairList*, SkinPairList*) = &nlRingIsEnd<SkinPairList>;
    BoneMapList* (*volatile forceBoneMapListStart)(BoneMapList*) = &nlRingGetStart<BoneMapList>;
    SkinPairList* (*volatile forceSkinPairListStart)(SkinPairList*) = &nlRingGetStart<SkinPairList>;
    (void)forceNlRingIsEnd;
    (void)forceSkinPairListStart;
    (void)forceBoneMapListStart;
}
