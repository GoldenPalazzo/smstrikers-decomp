#include "NL/nlMath.h"
#include "dolphin/mtx.h"

#define qr0 0

// Conversion factor from radians to 16-bit fixed-point format: 65536.0f / (2*pi) ~= 10430.378f
#define RAD_TO_FIXED16 10430.378f

/**
 * Offset/Address/Size: 0x800 | 0x801C437C | size: 0x20
 */
void nlMatrix4::SetIdentity()
{
    PSMTX44Identity(e2);
}

/**
 * Offset/Address/Size: 0x724 | 0x801C42A0 | size: 0xDC
 */
void nlMultMatrices(nlMatrix4& out, const nlMatrix4& a, const nlMatrix4& b)
{
    nlMatrix4 sp8;

    if ((out.e2 == a.e2) || (out.e2 == b.e2))
    {
        nlMatrix4 tmp;
        PSMTX44Concat(a.e2, b.e2, tmp.e2);
        out = tmp;
        return;
    }
    PSMTX44Concat(a.e2, b.e2, out.e2);
}

/**
 * Offset/Address/Size: 0x658 | 0x801C41D4 | size: 0xCC
 */
void nlTransposeMatrix(nlMatrix4& out, const nlMatrix4& in)
{
    if (out.e2 == in.e2)
    {
        nlMatrix4 tmp;
        PSMTX44Transpose(in.e2, tmp.e2);
        out = tmp;
        return;
    }
    PSMTX44Transpose(in.e2, out.e2);
}

/**
 * Offset/Address/Size: 0x5A4 | 0x801C4120 | size: 0xB4
 */
void nlInvertMatrix(nlMatrix4& out, const nlMatrix4& in)
{
    nlMatrix4 tmp;
    C_MTX44Inverse(in.e2, tmp.e2);
    out = tmp;
}

/**
 * Offset/Address/Size: 0x548 | 0x801C40C4 | size: 0x5C
 */
void nlMultVectorMatrix(nlVector2& v_out, const nlVector2& v_in, const nlMatrix3& m)
{
    nlVector2 t;
    t.x = m.e[0] * v_in.x + m.e[3] * v_in.y + m.e[6];
    t.y = m.e[1] * v_in.x + m.e[4] * v_in.y + m.e[7];
    v_out = t;
}

/**
 * Offset/Address/Size: 0x4F4 | 0x801C4070 | size: 0x54
 */
#pragma scheduling off
void nlMultPosVectorMatrix(register nlVector3& result, register const nlVector3& pos, register const nlMatrix4& transformMatrix)
{
    // clang-format off
    asm {
        psq_l f2, 0x0(transformMatrix), 0, qr0
        psq_l f0, 0x0(pos), 0, qr0
        psq_l f3, 0x8(transformMatrix), 0, qr0
        ps_muls0 f10, f2, f0
        psq_l f4, 0x10(transformMatrix), 0, qr0
        ps_muls0 f11, f3, f0
        psq_l f5, 0x18(transformMatrix), 0, qr0
        psq_l f6, 0x20(transformMatrix), 0, qr0
        ps_madds1 f10, f4, f0, f10
        psq_l f1, 0x8(pos), 1, qr0
        ps_madds1 f11, f5, f0, f11
        psq_l f7, 0x28(transformMatrix), 0, qr0
        ps_madds0 f10, f6, f1, f10
        psq_l f8, 0x30(transformMatrix), 0, qr0
        ps_madds0 f11, f7, f1, f11
        psq_l f9, 0x38(transformMatrix), 0, qr0
        ps_add f10, f8, f10
        ps_add f11, f9, f11
        psq_st f10, 0x0(result), 0, qr0
        psq_st f11, 0x8(result), 1, qr0
    }
    // clang-format on
}
#pragma scheduling reset

/**
 * Offset/Address/Size: 0x428 | 0x801C3FA4 | size: 0xCC
 */
void nlMultVectorMatrix(nlVector4& v_out, const nlVector4& v_in, const nlMatrix4& m)
{
    nlVector4 tmp;
    tmp.x = m.e2[0][0] * v_in.x + m.e2[1][0] * v_in.y + m.e2[2][0] * v_in.z + m.e2[3][0] * v_in.w;
    tmp.y = m.e2[0][1] * v_in.x + m.e2[1][1] * v_in.y + m.e2[2][1] * v_in.z + m.e2[3][1] * v_in.w;
    tmp.z = m.e2[0][2] * v_in.x + m.e2[1][2] * v_in.y + m.e2[2][2] * v_in.z + m.e2[3][2] * v_in.w;
    tmp.w = m.e2[0][3] * v_in.x + m.e2[1][3] * v_in.y + m.e2[2][3] * v_in.z + m.e2[3][3] * v_in.w;
    v_out = tmp;
}

/**
 * Offset/Address/Size: 0x3E4 | 0x801C3F60 | size: 0x44
 */
#pragma scheduling off
void nlMultDirVectorMatrix(register nlVector3& result, register const nlVector3& direction, register const nlMatrix4& transformMatrix)
{
    // clang-format off
     asm {
         psq_l f2, 0x0(transformMatrix), 0, qr0
         psq_l f0, 0x0(direction), 0, qr0
         psq_l f3, 0x8(transformMatrix), 0, qr0
         ps_muls0 f10, f2, f0
         psq_l f4, 0x10(transformMatrix), 0, qr0
         ps_muls0 f11, f3, f0
         psq_l f5, 0x18(transformMatrix), 0, qr0
         psq_l f6, 0x20(transformMatrix), 0, qr0
         ps_madds1 f10, f4, f0, f10
         psq_l f1, 0x8(direction), 1, qr0
         ps_madds1 f11, f5, f0, f11
         psq_l f7, 0x28(transformMatrix), 0, qr0
         ps_madds0 f10, f6, f1, f10
         ps_madds0 f11, f7, f1, f11
         psq_st f10, 0x0(result), 0, qr0
         psq_st f11, 0x8(result), 1, qr0
     }
    // clang-format on
}
#pragma scheduling reset

/**
 * Offset/Address/Size: 0x36C | 0x801C3EE8 | size: 0x78
 */
void nlMakeRotationMatrixX(nlMatrix4& m, float angle)
{
    f32 spC;
    f32 sp8;

    nlSinCos(&spC, &sp8, (short)(RAD_TO_FIXED16 * angle));
    PSMTX44Identity(m.e2);
    m.e2[1][1] = sp8;        // cos(theta) at offset 0x14
    m.e2[1][2] = spC;        // sin(theta) at offset 0x18
    m.e2[2][1] = -m.e2[1][2]; // -sin(theta) at offset 0x24
    m.e2[2][2] = m.e2[1][1];  // cos(theta) at offset 0x28
}

/**
 * Offset/Address/Size: 0x2F0 | 0x801C3E6C | size: 0x7C
 */
void nlMakeRotationMatrixY(nlMatrix4& m, float angle)
{
    f32 spC;
    f32 sp8;

    nlSinCos(&spC, &sp8, (short)(RAD_TO_FIXED16 * angle));
    PSMTX44Identity(m.e2);
    m.e2[0][0] = sp8;        // cos(theta) at offset 0x00
    m.e2[0][2] = -spC;       // sin(theta) at offset 0x08
    m.e2[2][0] = -m.e2[0][2]; // -sin(theta) at offset 0x18
    m.e2[2][2] = m.e2[0][0];  // cos(theta) at offset 0x20
}
/**
 * Offset/Address/Size: 0x278 | 0x801C3DF4 | size: 0x78
 */
void nlMakeRotationMatrixZ(nlMatrix4& m, float angle)
{
    f32 sn;
    f32 cs;

    nlSinCos(&sn, &cs, (short)(RAD_TO_FIXED16 * angle));
    PSMTX44Identity(m.e2);

    m.e2[0][0] = cs;         // cos(theta) at offset 0x00
    m.e2[0][1] = sn;         // sin(theta) at offset 0x04
    m.e2[1][0] = -m.e2[0][1]; // -sin(theta) at offset 0x10
    m.e2[1][1] = m.e2[0][0];  // cos(theta) at offset 0x14
}

/**
 * Offset/Address/Size: 0x1DC | 0x801C3D58 | size: 0x9C
 */
void nlMakeRotationMatrixZ(nlMatrix3& m, float angle)
{
    f32 sn;
    f32 cs;

    nlSinCos(&sn, &cs, (short)(10430.378f * angle));

    m.SetIdentity();

    m.e[0] = cs;
    m.e[1] = sn;
    m.e[3] = -m.e[1];
    m.e[4] = m.e[0];
}

/**
 * Offset/Address/Size: 0x20 | 0x801C3B9C | size: 0x1BC
 */
void nlMakeRotationMatrixEulerAngles(nlMatrix4& m, float pitch, float yaw, float roll)
{
    float zero = 0.0f;
    float one = 1.0f;
    f32 sinRoll;
    f32 cosRoll;
    f32 sinYaw;
    f32 cosYaw;
    f32 sinPitch;
    f32 cosPitch;

    nlSinCos(&sinRoll, &cosRoll, (short)(RAD_TO_FIXED16 * roll));
    nlSinCos(&sinYaw, &cosYaw, (short)(RAD_TO_FIXED16 * yaw));
    nlSinCos(&sinPitch, &cosPitch, (short)(RAD_TO_FIXED16 * pitch));

    m.e2[2][3] = zero;
    m.e2[1][3] = zero;
    m.e2[0][3] = zero;
    m.e2[3][2] = zero;
    m.e2[3][1] = zero;
    m.e2[3][0] = zero;
    m.e2[3][3] = one;
    m.e2[0][0] = cosYaw * cosRoll;
    m.e2[0][1] = cosYaw * sinRoll;
    m.e2[0][2] = -sinYaw;
    m.e2[1][0] = (cosRoll * (sinPitch * sinYaw)) - (cosPitch * sinRoll);
    m.e2[1][1] = (sinRoll * (sinPitch * sinYaw)) + (cosPitch * cosRoll);
    m.e2[1][2] = cosYaw * sinPitch;
    m.e2[2][0] = (cosRoll * (cosPitch * sinYaw)) + (sinPitch * sinRoll);
    m.e2[2][1] = (sinRoll * (cosPitch * sinYaw)) - (sinPitch * cosRoll);
    m.e2[2][2] = cosYaw * cosPitch;
}

/**
 * Offset/Address/Size: 0x0 | 0x801C3B7C | size: 0x20
 */
void nlMakeScaleMatrix(nlMatrix4& m, float sx, float sy, float sz)
{
    PSMTX44Scale(m.e2, sx, sy, sz);
}

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler lays out the related fields to match the original binary.
 */
void platvmath_stub(float& sx, float& sy, float& sz)
{
    sy = 1.0f;
    sx = 0.0f;
    sz = RAD_TO_FIXED16;
}
