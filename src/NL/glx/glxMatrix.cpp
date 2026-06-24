#include "NL/glx/glxMatrix.h"
#include "math.h"

#include <stddef.h>

extern "C"
{
    void* memcpy(void* dest, const void* src, size_t num);
}

/**
 * Offset/Address/Size: 0x0 | 0x801B6568 | size: 0x64
 */
void glxCopyMatrix(float (&arg0)[3][4], const nlMatrix4& arg1)
{
    // Row 0: copy from nlMatrix4 row 0 to target row 0
    arg0[0][0] = arg1.m[0][0]; // offset 0x00
    arg0[0][1] = arg1.m[1][0]; // offset 0x04
    arg0[0][2] = arg1.m[2][0]; // offset 0x08
    arg0[0][3] = arg1.m[3][0]; // offset 0x0C

    // Row 1: copy from nlMatrix4 row 1 to target row 1
    arg0[1][0] = arg1.m[0][1]; // offset 0x10
    arg0[1][1] = arg1.m[1][1]; // offset 0x14
    arg0[1][2] = arg1.m[2][1]; // offset 0x18
    arg0[1][3] = arg1.m[3][1]; // offset 0x1C

    // Row 2: copy from nlMatrix4 row 2 to target row 2
    arg0[2][0] = arg1.m[0][2]; // offset 0x20
    arg0[2][1] = arg1.m[1][2]; // offset 0x24
    arg0[2][2] = arg1.m[2][2]; // offset 0x28
    arg0[2][3] = arg1.m[3][2]; // offset 0x2C
}

/**
 * Offset/Address/Size: 0x64 | 0x801B65CC | size: 0x24
 */
void glxCopyMatrix(float (&arg0)[4][4], const nlMatrix4& arg1)
{
    memcpy(arg0, arg1.m, sizeof(arg1.m));
}

/**
 * Offset/Address/Size: 0x88 | 0x801B65F0 | size: 0x1CC
 * TODO: 97.26% match - remaining f-register allocation differs in initial eye delta loads and side/up translation math.
 */
void glplatMatrixLookAt(nlMatrix4& arg0, const nlVector3& arg1, const nlVector3& arg2, const nlVector3& arg3)
{
    float ay = arg1.f.y;
    float by = arg2.f.y;
    float ax = arg1.f.x;
    float bx = arg2.f.x;
    float f27 = ax - bx;
    float az = arg1.f.z;
    float f26 = ay - by;
    float bz = arg2.f.z;
    float f0 = f26 * f26;
    float f28 = az - bz;

    float f1 = nlRecipSqrt(f28 * f28 + (f27 * f27 + f0), true);
    float upz = arg3.f.z;
    float upy = arg3.f.y;
    float f31 = f1 * f27;
    float upx = arg3.f.x;
    float f30 = f1 * f26;
    float f29 = f1 * f28;

    float negUpx = -upx;
    float fA = upz * f31;
    float fB = upz * f30;
    float fC = upy * f31;
    f28 = upy * f29 - fB;
    f26 = negUpx * f29 + fA;
    f27 = upx * f30 - fC;

    f1 = nlRecipSqrt(f27 * f27 + (f28 * f28 + f26 * f26), true);
    f27 *= f1;
    float eyeY = arg1.f.y;
    f26 *= f1;
    float eyeX = arg1.f.x;
    float zDot = f30 * eyeY;
    float eyeZ = arg1.f.z;
    float sideDot = f27 * eyeY;
    arg0.m[0][0] = f26;
    f28 *= f1;
    float zero = 0.0f;
    float negZx = -f31;
    arg0.m[1][0] = f27;
    float upYBase = f29 * f26;
    arg0.m[2][0] = f28;
    sideDot = f26 * eyeX + sideDot;
    float one = 1.0f;
    float upX = f29 * f27;
    float upY = negZx * f28 + upYBase;
    sideDot = f28 * eyeZ + sideDot;
    float upXBase = f30 * f26;
    upX = f30 * f28 - upX;
    float upDot = upY * eyeY;
    sideDot = -sideDot;
    float upZ = f31 * f27 - upXBase;
    upDot = upX * eyeX + upDot;
    arg0.m[3][0] = sideDot;
    zDot = f31 * eyeX + zDot;
    arg0.m[0][1] = upX;
    upDot = upZ * eyeZ + upDot;
    zDot = f29 * eyeZ + zDot;
    arg0.m[1][1] = upY;
    upDot = -upDot;
    arg0.m[2][1] = upZ;
    zDot = -zDot;

    arg0.m[3][1] = upDot;
    arg0.m[0][2] = f31;
    arg0.m[1][2] = f30;
    arg0.m[2][2] = f29;
    arg0.m[3][2] = zDot;
    arg0.m[0][3] = zero;
    arg0.m[1][3] = zero;
    arg0.m[2][3] = zero;
    arg0.m[3][3] = one;
}

/**
 * Offset/Address/Size: 0x254 | 0x801B67BC | size: 0xB8
 */
void glplatMatrixPerspective(nlMatrix4& matrix, float fovY, float aspect, float near, float far)
{
    f32 temp_f2 = tan(0.5f * fovY);
    f32 atanVal = (f32)atan((1.0f / aspect) / (1.0f / temp_f2));
    C_MTXPerspective(matrix.m, (2.f * atanVal * 180.0f) / 3.1415927f, aspect, near, far);
}

/**
 * Offset/Address/Size: 0x30C | 0x801B6874 | size: 0x48
 */
void glplatMatrixOrthographicCentered(nlMatrix4& matrix, float width, float height, float near, float far)
{
    float half = 0.5f;
    C_MTXOrtho(matrix.m, height * half, -height * half, -width * half, width * half, near, far);
}

/**
 * Offset/Address/Size: 0x354 | 0x801B68BC | size: 0x6C
 */
void glplatMatrixOrthographic(nlMatrix4& matrix, float width, float height)
{
    static float fNear = 0.0f;
    static float fFar = 16777215.0f;
    C_MTXOrtho(matrix.m, 0.f, height, 0.f, width, fNear, fFar);
}
