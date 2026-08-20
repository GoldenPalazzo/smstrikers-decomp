#ifndef _PLATVMATH_H_
#define _PLATVMATH_H_

#include "NL/nlMath.h"

void nlMakeScaleMatrix(nlMatrix4& result, float scaleX, float scaleY, float scaleZ);
void nlMakeRotationMatrixEulerAngles(nlMatrix4& result, float pitch, float yaw, float roll);
void nlMakeRotationMatrixZ(nlMatrix3& result, float angleZ);
void nlMakeRotationMatrixZ(nlMatrix4& result, float angleZ);
void nlMakeRotationMatrixY(nlMatrix4& result, float angleY);
void nlMakeRotationMatrixX(nlMatrix4& result, float angleX);
void nlMultDirVectorMatrix(nlVector3& result, const nlVector3& direction, const nlMatrix4& transformMatrix);
void nlMultVectorMatrix(nlVector4& result, const nlVector4& inputVector, const nlMatrix4& transformMatrix);
void nlMultPosVectorMatrix(nlVector3& result, const nlVector3& position, const nlMatrix4& transformMatrix);
void nlMultVectorMatrix(nlVector2& result, const nlVector2& inputVector, const nlMatrix3& transformMatrix);
void nlInvertMatrix(nlMatrix4& result, const nlMatrix4& inputMatrix);
void nlTransposeMatrix(nlMatrix4& result, const nlMatrix4& inputMatrix);
void nlMultMatrices(nlMatrix4& result, const nlMatrix4& matrixA, const nlMatrix4& matrixB);

#endif // _PLATVMATH_H_
