#ifndef _GLUSKINMESH_H_
#define _GLUSKINMESH_H_

#include "NL/nlMath.h"

class SkinPairList;
class BoneMapList;

// void nlRingIsEnd<SkinPairList>(SkinPairList*, SkinPairList*);
// nlRingGetStart<T> is defined as a template in NL/nlDLRing.h.

class ShaderSkinMesh
{
public:
    void AttachSkinData(unsigned long, const nlMatrix4*);
    void StitchModel();
};

// class TempMatrixCopier
// {
// public:
//     void CopyMatrix(const unsigned long&, unsigned long*);
// };

// class AVLTreeBase<unsigned long, unsigned long, NewAdapter<AVLTreeEntry<unsigned long, unsigned long>>, DefaultKeyCompare<unsigned long>>
// {
// public:
//     void Walk<TempMatrixCopier>(TempMatrixCopier*, void (TempMatrixCopier::*)(const unsigned long&, unsigned long*));
//     void InorderWalk<TempMatrixCopier>(AVLTreeEntry<unsigned long, unsigned long>*, TempMatrixCopier*, void (TempMatrixCopier::*)(const unsigned long&, unsigned long*));
//     void CastUp(AVLTreeNode*) const;
// };

#endif // _GLUSKINMESH_H_
