#include "Game/GL/gluSkinMesh.h"
#include "types.h"
#include "NL/gl/glMatrix.h"
#include "NL/gl/glMemory.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxDisplayList.h"
#include "NL/nlDLRing.h"
#include "NL/nlString.h"
#include "dolphin/PPCArch.h"
#include "dolphin/os/OSCache.h"

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
 */
void ShaderSkinMesh::AttachSkinData(unsigned long program, const nlMatrix4* pReflect)
{
    nlAVLTree<unsigned long, unsigned long, DefaultKeyCompare<unsigned long> >* boneMap = &nlRingGetStart<BoneMapList>(boneMaps)->boneMap;
    nlVector3* outVertices = NULL;
    nlVector3* outNormals = NULL;

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
                const nlMatrix4& matrix = tempMatrices[matrixOffset];

                for (unsigned int i = 0; i < curr->num; i++)
                {
                    const SkinPair& pair = curr->pairs[i];
                    float vertexWeight = (float)pair.vertexWeight / 65535.0f;
                    int index = pair.vertexIndex;

                    const nlVector3* inVertex;
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
                    inNormal.f.x = (float)packed[1] * invNormalScale;
                    inNormal.f.y = (float)packed[0] * invNormalScale;
                    inNormal.f.z = (float)packed[2] * invNormalScale;

                    nlVector3 transformedVertex;
                    transformedVertex.f.x = matrix.m[0][0] * inVertex->f.x + matrix.m[1][0] * inVertex->f.y + matrix.m[2][0] * inVertex->f.z + matrix.m[3][0];
                    transformedVertex.f.y = matrix.m[0][1] * inVertex->f.x + matrix.m[1][1] * inVertex->f.y + matrix.m[2][1] * inVertex->f.z + matrix.m[3][1];
                    transformedVertex.f.z = matrix.m[0][2] * inVertex->f.x + matrix.m[1][2] * inVertex->f.y + matrix.m[2][2] * inVertex->f.z + matrix.m[3][2];

                    nlVector3 transformedNormal;
                    transformedNormal.f.x = matrix.m[0][0] * inNormal.f.x + matrix.m[1][0] * inNormal.f.y + matrix.m[2][0] * inNormal.f.z;
                    transformedNormal.f.y = matrix.m[0][1] * inNormal.f.x + matrix.m[1][1] * inNormal.f.y + matrix.m[2][1] * inNormal.f.z;
                    transformedNormal.f.z = matrix.m[0][2] * inNormal.f.x + matrix.m[1][2] * inNormal.f.y + matrix.m[2][2] * inNormal.f.z;

                    outVertices[index].f.x += transformedVertex.f.x * vertexWeight;
                    outVertices[index].f.y += transformedVertex.f.y * vertexWeight;
                    outVertices[index].f.z += transformedVertex.f.z * vertexWeight;

                    outNormals[index].f.x += transformedNormal.f.x * vertexWeight;
                    outNormals[index].f.y += transformedNormal.f.y * vertexWeight;
                    outNormals[index].f.z += transformedNormal.f.z * vertexWeight;
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
    glModelPacket* pEndPacket = pPacket + pModel->numPackets;

    while (pPacket < pEndPacket)
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
