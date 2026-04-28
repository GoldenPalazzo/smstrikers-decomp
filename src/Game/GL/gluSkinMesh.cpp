#include "Game/GL/gluSkinMesh.h"
#include "types.h"
#include "NL/gl/glModel.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxDisplayList.h"
#include "NL/nlDLRing.h"

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
void ShaderSkinMesh::AttachSkinData(unsigned long, const nlMatrix4*)
{
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
