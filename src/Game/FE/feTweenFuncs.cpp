#include "Game/FE/feTweenFuncs.h"

#include "math.h"
#include "NL/nlMath.h"

namespace TweenFunctions
{

/**
 * Offset/Address/Size: 0x104 | 0x800A2FC8 | size: 0xF4
 */
float easeinelastic(float t, float b, float c, float d)
{
    float p;

    if (t == 0.0f)
    {
        return b;
    }

    t = t / d;
    if (t == 1.0f)
    {
        return b + c;
    }

    t = t - 1.0f;
    p = (float)(0.3 * (double)d);
    float q = p;
    q *= 0.25f;
    u16 idx = (u16)(s32)(10430.378f * ((6.2831855f * ((t * d) - q)) / p));
    float sinv = nlSin(idx);
    double pow_d = pow(2.0, (double)(10.0f * t));
    double prod = ((double)c) * pow_d;
    return (float)(-((prod * (double)sinv) - (double)b));
}

/**
 * Offset/Address/Size: 0x10 | 0x800A2ED4 | size: 0xF4
 */
float easeoutelastic(float arg0, float arg1, float arg2, float arg3)
{
    float p;
    float q;
    float sinv;
    double prod;

    if (arg0 == 0.0f)
    {
        return arg1;
    }
    arg0 = arg0 / arg3;
    if (arg0 == 1.0f)
    {
        return arg1 + arg2;
    }

    p = (float)(0.3 * (double)arg3);
    q = p;
    q *= 0.25f;
    sinv = nlSin((u16)(s32)(10430.378f * ((6.2831855f * ((arg0 * arg3) - q)) / p)));
    prod = arg2 * pow(2.0, -10.0f * arg0);
    return (float)(arg1 + ((prod * sinv) + arg2));
}

/**
 * Offset/Address/Size: 0x0 | 0x800A2EC4 | size: 0x10
 */
float linear(float t, float b, float c, float d)
{
    return b + (c * t) / d;
}

} // namespace TweenFunctions

/**
 * Stub only for field order; unreferenced so the linker drops it.
 * Forces emission of specific constants/operations so the compiler lays out the related fields to match the original binary.
 */
void feTweenFuncs_stub(float& v0, float& v1, double& v2, float& v3, float& v4, float& v5, double& v6, float& v7, float& v8)
{
    v0 = 0.0f;
    v1 = 1.0f;
    v2 = 0.3;
    v3 = 10430.378f;
    v4 = 6.2831855f;
    v5 = 0.25f;
    v6 = 2.0;
    v7 = -10.0f;
    v8 = 10.0f;
}
