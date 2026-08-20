#ifndef _PLATQMATH_H_
#define _PLATQMATH_H_

#include "types.h"

class nlMatrix4;
#include "NL/nlMath.h"

#pragma cpp_extensions on

class nlQuaternion
{
public:
    union
    {
        float e[4]; // offset 0x0, size 0x10
        struct
        {
            float x; // offset 0x0, size 0x4
            float y; // offset 0x4, size 0x4
            float z; // offset 0x8, size 0x4
            float w; // offset 0xC, size 0x4
        };
    };
}; // total size: 0x10

#pragma cpp_extensions reset

inline void nlQuatIdentity(nlQuaternion& q0)
{
    q0.x = 0.f;
    q0.y = 0.f;
    q0.z = 0.f;
    q0.w = 1.f;
}

void nlQuatScale(nlQuaternion& result, const nlQuaternion& input, float scaleFactor);
f32 nlQuatDot(const nlQuaternion& quat1, const nlQuaternion& quat2);
void nlMultQuat(nlQuaternion& result, const nlQuaternion& quat1, const nlQuaternion& quat2);
void nlMatrixToQuat(nlQuaternion& result, const nlMatrix4& rotationMatrix);
void nlQuatToMatrix(nlMatrix4& resultMatrix, const nlQuaternion& inputQuat);
void nlQuatSlerp(nlQuaternion& result, const nlQuaternion& startQuat, const nlQuaternion& endQuat, float interpolationFactor);

#endif // _PLATQMATH_H_
