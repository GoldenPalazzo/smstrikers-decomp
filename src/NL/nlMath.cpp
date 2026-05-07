#include "types.h"

#include "math.h"

void* memcpy(void* dest, const void* src, size_t num);

static float fUnk_504 = 0.00000000000000000000000000056904566f;

struct coeffs
{
    s32 coeffs0;
    s32 coeffs1;
    s32 coeffs2;
    s32 coeffs3;
    s32 coeffs4;
    s32 coeffs5;
    s32 coeffs6;
    s32 coeffs7;
};

const struct coeffs coeffs1 = { 0, 0x3b85739f, 0x3c92a2f4, 0x3d3741dd, 0x3dad0097, 0x3e09f123, 0x3e44b6ba, 0x3e81b9e9 };
const struct coeffs coeffs2 = {0x3f7f567a, 0x3f77098b, 0x3f690bc9, 0x3f56c9b8, 0x3f42760a, 0x3f2de0fe, 0x3f1a4609, 0x3f0854e4};

extern void seedMT(u32 p);

/**
 * Offset/Address/Size: 0x0 | 0x801D1474 | size: 0x1D4
 */
float nlBezier(float* controlPoints, int degree, float t)
{
    float oneMinusT;
    float powVal;
    float result;
    float tPower;
    float tPowerInv;
    float coeffF;
    float powTerm;
    int factorialResult;
    int factorialInv;
    int i;
    float* currentPoint;
    int factorialDivisor;
    int loopIndex;

    if (t == 1.0f)
    {
        return controlPoints[degree - 1];
    }

    oneMinusT = 1.0f - t;
    factorialResult = 1;
    factorialInv = 1;
    powVal = pow(oneMinusT, (float)degree);
    tPower = powVal;
    tPowerInv = 1.0f;

    for (i = 1; i <= degree; i++)
    {
        factorialResult *= i;
    }

    factorialDivisor = factorialResult;
    currentPoint = controlPoints + 1;
    result = *controlPoints * powVal;

    for (loopIndex = 1; loopIndex <= degree; loopIndex++)
    {
        factorialInv *= loopIndex;
        if (loopIndex != degree)
        {
            factorialDivisor /= (degree - loopIndex) + 1;
        }

        tPower /= oneMinusT;
        tPowerInv *= t;
        coeffF = (float)(factorialResult / (factorialInv * factorialDivisor));
        powTerm = tPowerInv * tPower;
        coeffF = coeffF * powTerm;
        coeffF = coeffF * *currentPoint;
        result = result + coeffF;
        currentPoint += 1;
    }

    return result;
}

/**
 * Offset/Address/Size: 0x1D4 | 0x801D1648 | size: 0x244
 */
float nlATan2f(float arg0, float arg1)
{
    register f32 temp_f5;
    register f32 temp_f4;
    f32 var_f3;
    s32 temp_r25;
    s32 temp_r25_2;
    s32 var_r0;
    s32 var_r0_2;

    // Static arrays for polynomial coefficients
    if ((float)fabs(arg1) > 0.00001f)
    {
        temp_f4 = fabs(arg0 / arg1);
        if (temp_f4 > 1.0f)
        {
            
            coeffs sp48 = coeffs2;
            coeffs sp68 = coeffs1;

            register f32 tmp = 1.0f/ temp_f4;
            temp_f5 = tmp;
            temp_r25 = 8.0f * temp_f5;

            var_r0 = temp_r25 <= 7 ? temp_r25 : 7;

            f32 c2 = ((float*)&sp68)[var_r0];
            f32 c1 = ((float*)&sp48)[var_r0];
            c1 = temp_f5 * c2 + c1;
            var_f3 = 1.5707964f - c1;  
        }
        else
        {
            var_r0_2 = 7;
            temp_r25_2 = 8.0f * temp_f4;

            coeffs sp8 = coeffs1;
            coeffs sp28 = coeffs2;

            var_r0_2 = temp_r25_2 <= 7 ? temp_r25_2 : var_r0_2;
            
            var_f3 = (temp_f4 * ((float*)&sp28)[var_r0_2]) + ((float*)&sp8)[var_r0_2];
        }
        if (arg0 >= 0.0f)
        {
            if (arg1 >= 0.0f)
            {
                return var_f3;
            }
            return 3.1415927f - var_f3;
        }
        if (arg1 < 0.0f)
        {
            return 3.1415927f + var_f3;
        }
        return 6.2831855f - var_f3;
    }
    if (arg0 > 0.0f)
    {
        return 1.5707964f;
    }
    return 4.712389f;
}

/**
 * Offset/Address/Size: 0x418 | 0x801D188C | size: 0x48
 */
float nlTan(unsigned short angle)
{
    return (float)tan(angle * 0.0000958738f);
}

/**
 * Offset/Address/Size: 0x460 | 0x801D18D4 | size: 0x12C
 * TODO: 98.20% match - clamp handoff still materializes a temp in f5 (extra fmr)
 *       instead of keeping clamped rad in f7 through the rsqrt/compare path
 */
unsigned short nlACos(float x)
{
    u8 complement = (x < 0.0f);
    f32 y, temp, sqrtVal, rad, rad2;

    x = 1.0f - (f32)fabs(x);

    temp = 0.015098966f * x + 0.005516444f;
    y = x * temp + 0.047654245f;
    y = x * y + 0.16391061f;
    y = x * y + 2.0002916f;
    y = x * y + -0.000007239284f;

    rad2 = 2.0f * x;
    if (rad2 >= 0.00001f)
    {
        rad = rad2;
    }
    else
    {
        rad = 0.00001f;
    }

    if (rad > 0.0f)
    {
        f32 t = (f32)__frsqrte(rad);
        f32 t2 = 0.5f * t * -(rad * (t * t) - 3.0f);
        f32 t3 = 0.5f * t2 * -(rad * (t2 * t2) - 3.0f);
        sqrtVal = 0.5f * t3 * -(rad * (t3 * t3) - 3.0f);
    }
    else
    {
        f32 zero = 0.0f;
        if (rad != zero)
        {
            sqrtVal = *(f32*)&__float_nan;
        }
        else
        {
            sqrtVal = *(f32*)&__float_huge;
        }
    }

    rad = y * sqrtVal;

    if (complement)
    {
        return (unsigned short)(s32)(10430.378f * (3.1415927f - rad));
    }
    return (unsigned short)(s32)(10430.378f * rad);
}

/**
 * Offset/Address/Size: 0x58C | 0x801D1A00 | size: 0x164
 */
void nlSinCos(float* presult_sin, float* presult_cos, unsigned short angle)
{
    float angle_rad = (6.283185f / 65536.0f) * (float)angle;
    float octants = 1.2732395f * angle_rad;
    int k = (int)octants;
    float oct_f = (float)k;
    float y = angle_rad - 0.7853982f * oct_f;

    float y_squared = y * y;
    float s0 = -1.0f / 5040.0f;
    float s1 = 1.0f / 120.0f;
    float s2 = -1.0f / 6.0f;
    float c0 = -1.0f / 720.0f;
    float c1 = 1.0f / 24.0f;
    float s3 = 1.0f;
    float c2 = -0.5f;
    float c3 = 0.99999998f;

    float sin_poly = s0 * y_squared + s1;
    float cos_poly = c0 * y_squared + c1;
    float sin_tmp = y_squared * sin_poly + s2;
    float cos_tmp = y_squared * cos_poly + c2;
    sin_poly = y_squared * sin_tmp + s3;
    cos_poly = y_squared * cos_tmp + c3;
    sin_poly = y * sin_poly;

    float result_cos;
    float result_sin;

    switch (k & 7)
    {
    case 0:
        result_sin = sin_poly;
        result_cos = cos_poly;
        break;
    case 1:
    {
        float c = 0.70710678f;
        float s_1 = c * sin_poly;
        float s_2 = c * cos_poly;
        result_sin = s_1 + s_2;
        result_cos = s_2 - s_1;
        break;
    }
    case 2:
        result_sin = cos_poly;
        result_cos = -sin_poly;
        break;
    case 3:
    {
        float a = 0.70710678f;
        float b = -0.70710678f;
        float t1 = a * cos_poly;
        float t2 = a * sin_poly;
        result_sin = b * sin_poly + t1;
        result_cos = b * cos_poly - t2;
        break;
    }
    case 4:
        result_sin = -sin_poly;
        result_cos = -cos_poly;
        break;
    case 5:
    {
        float c = -0.70710678f;
        float s_1 = c * sin_poly;
        float s_2 = c * cos_poly;
        result_sin = s_1 + s_2;
        result_cos = s_2 - s_1;
        break;
    }
    case 6:
        result_sin = -cos_poly;
        result_cos = sin_poly;
        break;
    case 7:
    {
        float a = -0.70710678f;
        float b = 0.70710678f;
        float t1 = a * cos_poly;
        float t2 = a * sin_poly;
        result_sin = b * sin_poly + t1;
        result_cos = b * cos_poly - t2;
        break;
    }
    }

    *presult_sin = result_sin;
    *presult_cos = result_cos;
}

float nlSin(unsigned short angle)
{
    float a = (float)angle * (6.283185f / 65536.0f);
    float working_a = a;
    float flip_sign = 1.0f;

    if (a >= 4.7123889f) // 3*PI/2
    {
        working_a = a - 6.283185f; // 2*PI
    }
    else if (a >= 1.5707963f) // PI/2
    {
        flip_sign = -flip_sign;
        working_a = a - 3.14159265f; // PI
    }

    // Taylor series
    float a_squared = working_a * working_a;
    float result = working_a * (1.0f + a_squared * (-0.16666667f + a_squared * (0.008332208f + a_squared * (-0.00019841f + a_squared * 0.0000027557319f))));

    return flip_sign * result;
}

/**
 * Offset/Address/Size: 0x790 | 0x801D1C04 | size: 0x70
 * TODO: 96.79% match - register allocation diffs in Newton-Raphson iterations
 */
float nlRecipSqrt(float x, bool)
{
    float zero = 0.0f;
    if (x > zero)
    {
        float y = __frsqrte(x);
        y = 0.5f * y * (3.0f - y * y * x);
        y = 0.5f * y * (3.0f - y * y * x);
        y = 0.5f * y * (3.0f - y * y * x);
        return y;
    }
    else if (x != zero)
    {
        return NAN;
    }
    else
    {
        return INFINITY;
    }
}

/**
 * Offset/Address/Size: 0x800 | 0x801D1C74 | size: 0xEC
 */
float nlSqrt(float x, bool bAccurate)
{
    if (x > 0.0f)
    {
        double guess = __frsqrte((double)x);
        guess = 0.5 * guess * (3.0 - guess * guess * x);
        if (bAccurate)
        {
            guess = 0.5 * guess * (3.0 - guess * guess * x);
            guess = 0.5 * guess * (3.0 - guess * guess * x);
        }
        return (float)(x * guess);
    }
    else if (x < 0.0)
    {
        return NAN;
    }
    else
    {
        if (__fpclassifyf(x) == FP_NAN)
        {
            return NAN;
        }
        return x;
    }
}

/**
 * Offset/Address/Size: 0x8EC | 0x801D1D60 | size: 0x84
 */
#pragma fp_contract off
float nlRandomf(float fMin, float fMax, unsigned int* pSeed)
{
    unsigned int next;
    unsigned int temp;
    unsigned int mod;
    unsigned int seed;

    float range = fMax - fMin;
    float scale = (1.0f / 2147483647.0f) * range;
    mod = (seed = *pSeed) % 0x7FFFFFFFu;
    next = seed ^ 0x1d872b41;
    temp = next ^ (next >> 5);
    *pSeed = temp ^ (next ^ (temp << 0x1b));
    f32 fVal = scale * (f32)mod;
    return fMin + fVal;
}
#pragma fp_contract on

/**
 * Offset/Address/Size: 0x970 | 0x801D1DE4 | size: 0x7C
 */
float nlRandomf(float fMax, unsigned int* pSeed)
{
    unsigned int next;
    unsigned int temp;
    unsigned int mod;
    unsigned int seed;

    mod = (seed = *pSeed) % 0x7FFFFFFFu;
    next = seed ^ 0x1d872b41;
    temp = next ^ (next >> 5);
    *pSeed = temp ^ (next ^ (temp << 0x1b));
    return (1.0f / 2147483647.0f) * fMax * (f32)mod;
}

/**
 * Offset/Address/Size: 0x9EC | 0x801D1E60 | size: 0x34
 */
uint nlRandom(uint range, uint* seed)
{
    uint next;
    uint temp;

    uint result = *seed % range;
    next = *seed ^ 0x1d872b41;
    temp = next ^ (next >> 5);
    *seed = temp ^ (next ^ (temp << 0x1b));
    return result;
}

/**
 * Offset/Address/Size: 0xA20 | 0x801D1E94 | size: 0x2C
 */
void nlSetRandomSeed(uint initialSeed, uint* seedStorage)
{
    uint shiftedValue;
    uint mixedValue;

    *seedStorage = initialSeed;

    mixedValue = *seedStorage ^ 0x1d872b41;        // Mix with constant
    shiftedValue = mixedValue ^ (mixedValue >> 5); // Mix with right-shifted value

    *seedStorage = shiftedValue ^ (mixedValue ^ (shiftedValue << 0x1b)); // Final mixing
}

/**
 * Offset/Address/Size: 0xA4C | 0x801D1EC0 | size: 0x24
 */
void nlInitRandom()
{
    seedMT(0x1105);
}
