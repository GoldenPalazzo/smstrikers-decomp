#include "NL/nlMath.h"
#include "dolphin/mtx.h"
#include "types.h"

#define qr0 0

/**
 * Offset/Address/Size: 0x0 | 0x801C439C | size: 0x2C
 */
void nlQuatScale(nlQuaternion& out, const nlQuaternion& in, float scale)
{
    PSQUATScale((Quaternion*)&in, (Quaternion*)&out, scale);
}

/**
 * Offset/Address/Size: 0x2C | 0x801C43C8 | size: 0x20
 */
f32 nlQuatDot(const nlQuaternion& q1, const nlQuaternion& q2)
{
    return PSQUATDotProduct((Quaternion*)&q1, (Quaternion*)&q2);
}

/**
 * Offset/Address/Size: 0x4C | 0x801C43E8 | size: 0x74
 */
void nlMultQuat(nlQuaternion& out, const nlQuaternion& q1, const nlQuaternion& q2)
{
    out.f.w = q1.f.w * q2.f.w - q1.f.x * q2.f.x - q1.f.y * q2.f.y - q1.f.z * q2.f.z;
    out.f.x = q1.f.w * q2.f.x + q1.f.x * q2.f.w + q1.f.y * q2.f.z - q1.f.z * q2.f.y;
    out.f.y = q1.f.w * q2.f.y - q1.f.x * q2.f.z + q1.f.y * q2.f.w + q1.f.z * q2.f.x;
    out.f.z = q1.f.w * q2.f.z + q1.f.x * q2.f.y - q1.f.y * q2.f.x + q1.f.z * q2.f.w;
}

/**
 * Offset/Address/Size: 0xC0 | 0x801C445C | size: 0x278
 */
void nlMatrixToQuat(nlQuaternion& out, const nlMatrix4& in)
{
    f32 tr;
    f32 s;
    f32 q[4];
    int i, j, k;
    f32 m[4][4];

    m[0][0] = in.m[0][0];
    m[0][1] = in.m[0][1];
    m[0][2] = in.m[0][2];
    m[0][3] = in.m[0][3];
    m[1][0] = in.m[1][0];
    m[1][1] = in.m[1][1];
    m[1][2] = in.m[1][2];
    m[1][3] = in.m[1][3];
    m[2][0] = in.m[2][0];
    m[2][1] = in.m[2][1];
    m[2][2] = in.m[2][2];
    m[2][3] = in.m[2][3];
    m[3][0] = in.m[3][0];
    m[3][1] = in.m[3][1];
    m[3][2] = in.m[3][2];
    m[3][3] = in.m[3][3];

    tr = m[0][0] + m[1][1] + m[2][2];

    int nxt[3] = { 1, 2, 0 };

    if (tr > 0.0)
    {
        s = nlSqrt(1.0f + tr, true);
        out.f.w = s / 2.0f;
        s = 0.5f / s;
        out.f.x = s * (m[1][2] - m[2][1]);
        out.f.y = s * (m[2][0] - m[0][2]);
        out.f.z = s * (m[0][1] - m[1][0]);
        return;
    }

    i = 0;
    if (m[1][1] > m[0][0])
    {
        i = 1;
    }
    if (m[2][2] > m[i][i])
    {
        i = 2;
    }
    j = nxt[i];
    k = nxt[j];

    s = nlSqrt(1.0f + (m[i][i] - (m[j][j] + m[k][k])), true);
    q[i] = 0.5f * s;
    if (s != 0.0f)
    {
        s = 0.5f / s;
    }
    q[3] = s * (m[j][k] - m[k][j]);
    q[j] = s * (m[i][j] + m[j][i]);
    q[k] = s * (m[i][k] + m[k][i]);

    out.f.x = q[0];
    out.f.y = q[1];
    out.f.z = q[2];
    out.f.w = q[3];
}

/**
 * Offset/Address/Size: 0x338 | 0x801C46D4 | size: 0xF4
 */
void nlQuatToMatrix(nlMatrix4& out, const nlQuaternion& quat)
{
    f32 x = quat.f.x;
    f32 y = quat.f.y;
    f32 z = quat.f.z;
    f32 w = quat.f.w;

    out.m[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
    out.m[1][0] = 2.0f * x * y - 2.0f * w * z;
    out.m[2][0] = 2.0f * x * z + 2.0f * w * y;
    out.m[3][0] = 0.0f;
    out.m[0][1] = 2.0f * x * y + 2.0f * w * z;
    out.m[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z;
    out.m[2][1] = 2.0f * y * z - 2.0f * w * x;
    out.m[3][1] = 0.0f;
    out.m[0][2] = 2.0f * x * z - 2.0f * w * y;
    out.m[1][2] = 2.0f * y * z + 2.0f * w * x;
    out.m[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;
    out.m[3][2] = 0.0f;
    out.m[0][3] = 0.0f;
    out.m[1][3] = 0.0f;
    out.m[2][3] = 0.0f;
    out.m[3][3] = 1.0f;
}

/**
 * Offset/Address/Size: 0x42C | 0x801C47C8 | size: 0x34
 */
void nlQuatSlerp(nlQuaternion& out, const nlQuaternion& q1, const nlQuaternion& q2, float t)
{
    C_QUATSlerp((Quaternion*)&q1, (Quaternion*)&q2, (Quaternion*)&out, t);
}
