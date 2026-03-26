#ifndef _NLMATH_H_
#define _NLMATH_H_

#include "types.h"
#include "dolphin/mtx.h"

float nlBezier(float*, int, float);
float nlATan2f(float, float);
float nlTan(unsigned short);
unsigned short nlACos(float);
void nlSinCos(float* presult_sin, float* presult_cos, unsigned short angle);
float nlSin(unsigned short);
float nlRecipSqrt(float, bool);
float nlSqrt(float, bool);
float nlRandomf(float, float, unsigned int*);
float nlRandomf(float, unsigned int*);
unsigned int nlRandom(unsigned int, unsigned int*);
void nlSetRandomSeed(unsigned int, unsigned int*);
void nlInitRandom();

struct nlVector2_
{
    float f[2];
};

struct nlVector2
{
public:
    union
    {               // inferred
        float e[2]; // offset 0x0, size 0x8
        u32 as_u32[2];
        struct
        {            // inferred
            float x; // offset 0x0, size 0x4
            float y; // offset 0x4, size 0x4
        } f;
    };

    // nlVector2() { }
    // nlVector2(float _x, float _y)
    // {
    //     e[0] = _x;
    //     e[1] = _y;
    // }
};

// #define NL_VECTOR3_SET(v, xval, yval, zval) \
//     do                                      \
//     {                                       \
//         (v).f.x = (xval);                   \
//         (v).f.y = (yval);                   \
//         (v).f.z = (zval);                   \
//     } while (0)

// #define NL_VECTOR4_SET(v, xval, yval, zval) \
//     do                                      \
//     {                                       \
//         (v).f.x = (xval);                   \
//         (v).f.y = (yval);                   \
//         (v).f.z = (zval);                   \
//     } while (0)

struct nlVector3_
{
    float f[3];
};

struct nlVector3
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        } f;
        u32 as_u32[3];
    };

    void Set(float x, float y, float z)
    {
        FORCE_DONT_INLINE;
        FORCE_DONT_INLINE;
        FORCE_DONT_INLINE;
        FORCE_DONT_INLINE;
        f.x = x;
        f.y = y;
        f.z = z;
    }

    inline float CalculateDistanceSquared(const nlVector3& v)
    {
        float dx = f.x - v.f.x;
        float dy = f.y - v.f.y;
        float dz = f.z - v.f.z;
        return dx * dx + dy * dy + dz * dz;
    }

    inline float CalculateDistanceSquared2D(const nlVector3& v)
    {
        float dx = f.x - v.f.x;
        float dy = f.y - v.f.y;
        return dx * dx + dy * dy;
    }

    inline float GetLengthSq2D()
    {
        return f.x * f.x + f.y * f.y;
    }

    inline float GetLengthSq3D()
    {
        return f.x * f.x + f.y * f.y + f.z * f.z;
    }
};

inline void nlVec2Set(nlVector2& v0, float _x, float _y)
{
    v0.f.x = _x;
    v0.f.y = _y;
}

inline void nlVec2Sub(nlVector2& result, const nlVector2& a, const nlVector2& b)
{
    nlVec2Set(result, a.f.x - b.f.x, a.f.y - b.f.y);
}

inline float nlGetLengthSquared2D(float x, float y)
{
    return x * x + y * y;
}

inline float nlGetLength2D(float x, float y)
{
    float xx = x * x;
    float yy = y * y;
    const float lengthSquared = xx + yy;
    return nlSqrt(lengthSquared, true);
}

inline float nlGetLengthSquared3D(float x, float y, float z)
{
    return x * x + y * y + z * z;
}

inline float nlGetLength3D(float x, float y, float z)
{
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    const float lengthSquared = xx + yy + zz;
    return nlSqrt(lengthSquared, true);
}

inline void nlVec3Set(nlVector3& v0, float _x, float _y, float _z)
{
    v0.f.x = _x;
    v0.f.y = _y;
    v0.f.z = _z;
}

inline void nlVec3SetRotatedXY(nlVector3& result, const nlVector3& source, float cosAngle, float sinAngle)
{
    result.f.x = source.f.x * cosAngle - source.f.y * sinAngle;
    result.f.y = source.f.y * cosAngle + source.f.x * sinAngle;
    result.f.z = source.f.z;
}

inline void nlVec3Sub(nlVector3& result, const nlVector3& a, const nlVector3& b)
{
    nlVec3Set(result, a.f.x - b.f.x, a.f.y - b.f.y, a.f.z - b.f.z);
}

inline void nlVec3Add(nlVector3& result, const nlVector3& a, const nlVector3& b)
{
    nlVec3Set(result, a.f.x + b.f.x, a.f.y + b.f.y, a.f.z + b.f.z);
}

inline void nlVec3Add(nlVector3& result, float x, float y, float z)
{
    nlVec3Set(result, x + result.f.x, y + result.f.y, z + result.f.z);
}

inline void nlVec3Scale(nlVector3& result, const nlVector3& v, const float scale)
{
    nlVec3Set(result, scale * v.f.x, scale * v.f.y, scale * v.f.z);
}

inline void _nlVec3Scale(nlVector3& result, float scale)
{
    nlVec3Set(result, scale * result.f.x, scale * result.f.y, scale * result.f.z);
}

class nlVector4
{
public:
    union
    {
        struct
        {
            float x; // offset 0x0, size 0x4
            float y; // offset 0x4, size 0x4
            float z; // offset 0x8, size 0x4
            float w; // offset 0xC, size 0x4
        } f;
        float e[4]; // offset 0x0, size 0x10
    };

    // nlVector4() { }
    // nlVector4(float x, float y, float z, float w)
    //     : x(x)
    //     , y(y)
    //     , z(z)
    //     , w(w)
    // {
    // }
}; // total size: 0x10

inline void nlVec4Set(nlVector4& v0, float _x, float _y, float _z, float _w)
{
    v0.f.x = _x;
    v0.f.y = _y;
    v0.f.z = _z;
    v0.f.w = _w;
}

inline float nlPlaneDot(const nlVector4& plane, const nlVector3& v)
{
    return v.f.x * plane.f.x + v.f.y * plane.f.y + v.f.z * plane.f.z + plane.f.w;
}

struct nlMatrix3
{
    // float m[3 * 3]; // Flat array access
    union
    {                  // inferred
        float m[9];    // offset 0x0, size 0x24
        float e[3][3]; // offset 0x0, size 0x24
        struct
        {              // inferred
            float m11; // offset 0x0, size 0x4
            float m12; // offset 0x4, size 0x4
            float m13; // offset 0x8, size 0x4
            float m21; // offset 0xC, size 0x4
            float m22; // offset 0x10, size 0x4
            float m23; // offset 0x14, size 0x4
            float m31; // offset 0x18, size 0x4
            float m32; // offset 0x1C, size 0x4
            float m33; // offset 0x20, size 0x4
        } f;
    };

    inline void SetIdentity()
    {
        f.m32 = 0.0f;
        f.m31 = 0.0f;
        f.m23 = 0.0f;
        f.m21 = 0.0f;
        f.m13 = 0.0f;
        f.m12 = 0.0f;

        f.m33 = 1.0f;
        f.m22 = 1.0f;
        f.m11 = 1.0f;
    }

    inline nlVector2 operator*(const nlVector2& v_in) const
    {
        nlVector2 tmp;
        tmp.f.x = m[6] + ((m[0] * v_in.f.x) + (m[3] * v_in.f.y));
        tmp.f.y = m[7] + ((m[1] * v_in.f.x) + (m[4] * v_in.f.y));
        return tmp;
    }
};

/*
m[0][0] m[0][1] m[0][2] m[0][3]  // Row 0: offset 0x00, 0x04, 0x08, 0x0C
m[1][0] m[1][1] m[1][2] m[1][3]  // Row 1: offset 0x10, 0x14, 0x18, 0x1C
m[2][0] m[2][1] m[2][2] m[2][3]  // Row 2: offset 0x20, 0x24, 0x28, 0x2C
m[3][0] m[3][1] m[3][2] m[3][3]  // Row 3: offset 0x30, 0x34, 0x38, 0x3C
*/
struct nlMatrix4
{
    // Mtx44 m;
    union
    {
        float m[4][4]; // offset 0x0, size 0x40
        float e[16];   // offset 0x0, size 0x40
        struct
        {              // inferred
            float m11; // offset 0x0, size 0x4
            float m12; // offset 0x4, size 0x4
            float m13; // offset 0x8, size 0x4
            float m14; // offset 0xC, size 0x4
            float m21; // offset 0x10, size 0x4
            float m22; // offset 0x14, size 0x4
            float m23; // offset 0x18, size 0x4
            float m24; // offset 0x1C, size 0x4
            float m31; // offset 0x20, size 0x4
            float m32; // offset 0x24, size 0x4
            float m33; // offset 0x28, size 0x4
            float m34; // offset 0x2C, size 0x4
            float m41; // offset 0x30, size 0x4
            float m42; // offset 0x34, size 0x4
            float m43; // offset 0x38, size 0x4
            float m44; // offset 0x3C, size 0x4
        } f;
    };

    void SetIdentity();
    void SetColumn(int col, const nlVector3& v)
    {
        FORCE_DONT_INLINE;
        m[0][col] = v.f.x;
        m[1][col] = v.f.y;
        m[2][col] = v.f.z;
    }

    // inline version
    void SetColumn_(int col, const nlVector3& v)
    {
        m[0][col] = v.f.x;
        m[1][col] = v.f.y;
        m[2][col] = v.f.z;
    }

    // inline version
    void SetRow_(int row, const nlVector3& v)
    {
        m[row][0] = v.f.x;
        m[row][1] = v.f.y;
        m[row][2] = v.f.z;
    }

    inline void SetRow3_(int row, const float x, const float y, const float z)
    {
        m[row][0] = x;
        m[row][1] = y;
        m[row][2] = z;
        m[row][3] = 1.0f;
    }

    void SetRow4_(int row, const float x, const float y, const float z, const float w)
    {
        m[row][0] = x;
        m[row][1] = y;
        m[row][2] = z;
        m[row][3] = w;
    }

    /**
     * Offset/Address/Size: 0x0 | 0x80204294 | size: 0x8
     */
    nlVector3& GetTranslation() const
    {
        return *(nlVector3*)&f.m41;
    }

    void SetTranslation(const nlVector3&);

    inline nlVector4 operator*(const nlVector4& v_in) const
    {
        nlVector4 tmp;
        tmp.f.x = m[0][0] * v_in.f.x + m[1][0] * v_in.f.y + m[2][0] * v_in.f.z + m[3][0] * v_in.f.w;
        tmp.f.z = m[0][2] * v_in.f.x + m[1][2] * v_in.f.y + m[2][2] * v_in.f.z + m[3][2] * v_in.f.w;
        tmp.f.y = m[0][1] * v_in.f.x + m[1][1] * v_in.f.y + m[2][1] * v_in.f.z + m[3][1] * v_in.f.w;
        tmp.f.w = m[0][3] * v_in.f.x + m[1][3] * v_in.f.y + m[2][3] * v_in.f.z + m[3][3] * v_in.f.w;
        return tmp;
    }
};

/**
 * Convert a nlMatrix3 [3x3] matrix to a [4x3] ODE matrix
 */
inline void ConvertNLMat3ToDMat3(const nlMatrix3& src, float* dest)
{
    nlVec3Set(*(nlVector3*)dest, src.m[0], src.m[3], src.m[6]);
    nlVec3Set(*(nlVector3*)&dest[4], src.m[1], src.m[4], src.m[7]);
    nlVec3Set(*(nlVector3*)&dest[8], src.m[2], src.m[5], src.m[8]);
}

/**
 * Convert a nlMatrix4 [4x4] matrix to a [4x3] ODE matrix
 * Mathematical Operation: Row-wise transpose of first 3 rows
 *
 * Input 4x4 Matrix:     Output 4x3 Matrix:
 * [m00 m01 m02 m03]     [m00 m01 m02]
 * [m10 m11 m12 m13]  →  [m10 m11 m12]
 * [m20 m21 m22 m23]     [m20 m21 m22]
 * [m30 m31 m32 m33]     [m30 m31 m32]
 *
 * Takes rows 0, 1, 2 from 4x4 matrix and stores them as columns 0, 1, 2 in 4x3 matrix
 */
inline void ConvertNLMat4ToDMat3(const nlMatrix4& src, float* dest)
{
    nlVec4Set(*(nlVector4*)&dest[0], src.e[0], src.e[3], src.e[6], src.e[9]);
    nlVec4Set(*(nlVector4*)&dest[4], src.e[1], src.e[4], src.e[7], src.e[10]);
    nlVec4Set(*(nlVector4*)&dest[8], src.e[2], src.e[5], src.e[8], src.e[11]);
}

/**
 * Convert a nlMatrix4 [4x4] matrix to a [4x3] ODE matrix (Column Transpose)
 * Mathematical Operation: Column-wise transpose of first 3 columns
 *
 * Input 4x4 Matrix:     Output 4x3 Matrix:
 * [m00 m01 m02 m03]     [m00 m10 m20]
 * [m10 m11 m12 m13]  →  [m01 m11 m21]
 * [m20 m21 m22 m23]     [m02 m12 m22]
 * [m30 m31 m32 m33]     [m03 m13 m23]
 *
 * Takes columns 0, 1, 2 from 4x4 matrix and stores them as rows 0, 1, 2 in 4x3 matrix
 */
inline void ConvertNLMat4ToDMat3_Transposed(const nlMatrix4& src, float* dest)
{
    nlVec4Set(*(nlVector4*)&dest[0], src.e[0], src.e[4], src.e[8], src.e[12]);
    nlVec4Set(*(nlVector4*)&dest[4], src.e[1], src.e[5], src.e[9], src.e[13]);
    nlVec4Set(*(nlVector4*)&dest[8], src.e[2], src.e[6], src.e[10], src.e[14]);
}

extern unsigned int nlDefaultSeed;

#include "NL/platvmath.h"
#include "NL/platqmath.h"
#include "NL/utility.h"

struct nlPolar
{
    /* 0x00 */ u16 a;
    /* 0x04 */ float r;
}; // size: 0x8

void nlInvertRotTransMatrix(nlMatrix4&, const nlMatrix4&);
void nlQuatNLerp(nlQuaternion&, const nlQuaternion&, const nlQuaternion&, float);
void nlQuatInverse(nlQuaternion&, const nlQuaternion&);
void nlQuatNormalize(nlQuaternion&, const nlQuaternion&);
void nlMakeQuat(nlQuaternion&, const nlVector3&, float);
void nlCartesianToPolar(nlPolar&, const nlVector3&);
void nlAddPolarToCartesian(nlVector3&, const nlPolar&);
void nlPolarToCartesian(nlVector3&, const nlPolar&);
void nlPolarToCartesian(float&, float&, unsigned short, float);
void nlCartesianToPolar(nlPolar&, float, float);

#endif
