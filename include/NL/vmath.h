#ifndef _VMATH_H_
#define _VMATH_H_

#include "NL/nlMath.h"

inline void nlVector3::Set(float _x, float _y, float _z)
{
    f.x = _x;
    f.y = _y;
    f.z = _z;
}

inline void nlMatrix4::SetColumn(int col, const nlVector3& v)
{
    m[0][col] = v.f.x;
    m[1][col] = v.f.y;
    m[2][col] = v.f.z;
}

inline void nlVecAdd(nlVector3& out, const nlVector3& a, const nlVector3& b)
{
    nlVec3Set(out, a.f.x + b.f.x, a.f.y + b.f.y, a.f.z + b.f.z);
}

#endif // _VMATH_H_
