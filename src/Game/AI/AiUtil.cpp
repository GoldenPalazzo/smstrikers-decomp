#include "Game/AI/AiUtil.h"

#include "Game/Field.h"
#include "math.h"

char* g_sPowerupNames[9] = {
    "GreenShell",
    "RedShell",
    "SpinyShell",
    "FreezeShell",
    "Banana",
    "BoBomb",
    "ChainChomp",
    "Mushroom",
    "Star"
};

f32 CANT_COLLIDE = *(f32*)__float_max;

/**
 * Offset/Address/Size: 0x1420 | 0x80006ECC | size: 0x90
 */
void MakePerpendicularPlane(const nlVector3& v3Position, unsigned short aNormalAngle, nlVector4& v4Plane, float fPlaneOffset)
{
    float fSin;
    float fCos;
    nlSinCos(&fSin, &fCos, aNormalAngle);

    v4Plane.f.x = fCos;
    v4Plane.f.y = fSin;
    v4Plane.f.z = 0.0f;

    float t0 = v3Position.f.y * fSin;
    float t1 = v3Position.f.x * fCos + t0;
    v4Plane.f.w = fPlaneOffset + t1;
}

/**
 * Offset/Address/Size: 0x1354 | 0x80006E00 | size: 0xCC
 */
void MakePerpendicularPlane(const nlVector3& v3Position, const nlVector3& v3Normal, nlVector4& v4Plane, float fPlaneOffset)
{
    f32 lenSq = const_cast<nlVector3&>(v3Normal).GetLengthSq3D();
    f32 invLen = nlRecipSqrt(lenSq, true);

    f32 normX = invLen * v3Normal.f.x;
    f32 normY = invLen * v3Normal.f.y;
    f32 normZ = invLen * v3Normal.f.z;
    v4Plane.f.x = normX;
    v4Plane.f.y = normY;
    v4Plane.f.z = normZ;

    v4Plane.f.w = fPlaneOffset + (v3Position.f.x * v4Plane.f.x + v3Position.f.y * v4Plane.f.y + v3Position.f.z * v4Plane.f.z);
}

/**
 * Offset/Address/Size: 0x1204 | 0x80006CB0 | size: 0x150
 * TODO: 98.15% match - inner block normY=f7 (target f9) and zeroVal=f5 (target f7)
 * register allocation shift cascading into dot product computations.
 */
bool IsPointInCone(const nlVector3& v3Point, const nlVector3& v3Pivot, const nlVector3& v3Plane1, const nlVector3& v3Plane2)
{
    f32 distSqA = const_cast<nlVector3&>(v3Plane1).CalculateDistanceSquared2D(v3Pivot);
    f32 distSqP = const_cast<nlVector3&>(v3Plane1).CalculateDistanceSquared2D(v3Point);
    f32 pointY = v3Point.f.y;
    f32 pivotY = v3Pivot.f.y;

    if (distSqP < distSqA)
    {
        f32 dotApexY;
        f32 normZ;
        f32 dotLeftY;
        f32 dotRightY;

        f32 dirX = v3Pivot.f.x - v3Point.f.x;
        f32 zeroVal = 0.0f;
        f32 dirY = pivotY - pointY;

        f32 perpY = -dirY;
        f32 lenSq = perpY * perpY + dirX * dirX;
        f32 invLen = nlRecipSqrt(zeroVal + lenSq, true);

        f32 normY = invLen * perpY;
        f32 normX = invLen * dirX;

        dotApexY = v3Pivot.f.y * normX;
        normZ = invLen * zeroVal;
        dotLeftY = v3Plane1.f.y * normX;
        dotRightY = v3Plane2.f.y * normX;

        f32 dotApex = v3Pivot.f.x * normY + dotApexY;
        dotApex += v3Pivot.f.z * normZ;

        f32 dotLeft = v3Plane1.f.x * normY + dotLeftY;
        f32 dotRight = v3Plane2.f.x * normY + dotRightY;

        f32 bias = zeroVal + dotApex;

        dotLeft += v3Plane1.f.z * normZ;
        dotRight += v3Plane2.f.z * normZ;

        nlVector3 normalStore;
        normalStore.f.x = normY;
        normalStore.f.y = normX;
        normalStore.f.z = normZ;
        *(float*)((u8*)&normalStore + 0xC) = bias;

        f32 sideLeft = dotLeft - bias;
        f32 sideRight = dotRight - bias;

        if (sideLeft * sideRight < zeroVal)
        {
            return true;
        }
    }

    return false;
}

/**
 * Offset/Address/Size: 0x110C | 0x80006BB8 | size: 0xF8
 */
#undef abs
extern "C" int abs(int n);

static inline s16 AngleDiff(u16 a, u16 b) { return (s16)(a - b); }

unsigned short SeekDirection(unsigned short aCurrent, unsigned short aDesired, float fSeekSpeed, float fFalloff, float fDeltaT)
{
    u16 current = aCurrent;
    s16 diff = AngleDiff(aDesired, current);
    signed short nDeltaDisplacement;

    if (diff != 0)
    {
        f64 dDiff = (f64)(s16)diff;
        f64 absDiffD = __fabs(dDiff);
        f32 absDiff = (f32)absDiffD;

        if (absDiff < fFalloff)
        {
            f32 fSeekCoefficient = fFalloff * fFalloff / fSeekSpeed;
            nDeltaDisplacement = (s16)(s32)(absDiff - fSeekCoefficient / (fDeltaT + fSeekCoefficient / absDiff));
        }
        else
        {
            nDeltaDisplacement = (s16)(s32)(fDeltaT * fSeekSpeed);
        }

        if (abs(diff) < (s16)nDeltaDisplacement)
        {
            return aDesired;
        }

        if (diff > 0)
        {
            return (u16)(current + (s16)nDeltaDisplacement);
        }
        else
        {
            return (u16)(current - (s16)nDeltaDisplacement);
        }
    }

    return aDesired;
}

/**
 * Offset/Address/Size: 0x10C0 | 0x80006B6C | size: 0x4C
 */
float SeekSpeed(float fCurrent, float fDesired, float fSeekAccel, float fSeekDecel, float fDeltaT)
{
    float seekRate = (fCurrent <= fDesired) ? fSeekAccel : fSeekDecel;
    float delta = seekRate * fDeltaT;

    if (fCurrent <= fDesired)
    {
        float newValue = fCurrent + delta;
        if (newValue > fDesired)
        {
            return fDesired;
        }
        return newValue;
    }

    float newValue = fCurrent - delta;
    if (newValue < fDesired)
    {
        return fDesired;
    }
    return newValue;
}

/**
 * Offset/Address/Size: 0xFA4 | 0x80006A50 | size: 0x11C
 * TODO: 96.5% match - float register allocation (f4/f5/f6/f7/f9/f10 vs expected),
 */
void CalcInterceptXY(const nlVector3& pos1, f32 speed1, f32 speed2, const nlVector3& pos2, const nlVector3& vel, int& count, f32* times)
{
    f32 speed2Sq = speed2 * speed2;
    f32 speed1Sq = speed1 * speed1;

    f32 py2 = pos2.f.y;
    f32 py1 = pos1.f.y;
    f32 px2 = pos2.f.x;
    f32 px1 = pos1.f.x;

    f32 dy = py2 - py1;
    f32 dx = px2 - px1;
    f32 vy = vel.f.y;
    f32 vx = vel.f.x;

    f32 distSq = dx * dx + dy * dy;
    f32 velSq = vx * vx + vy * vy;
    f32 dotVelDelta = vx * dx + vy * dy;

    f32 a = velSq - speed1Sq;

    if (distSq <= speed2Sq)
    {
        count = 1;
        times[0] = 0.0f;
        return;
    }

    f32 b = 2.0f * (dotVelDelta - speed2 * speed1);
    f32 c = distSq - speed2Sq;

    f32 t2, t1;
    int numRoots;
    SolveQuadratic(a, b, c, numRoots, t1, t2);

    count = 0;
    if (numRoots == 0)
    {
        return;
    }

    if (t1 > 0.0f)
    {
        times[count] = t1;
        count++;
    }

    if (t2 > 0.0f)
    {
        times[count] = t2;
        count++;
    }
}

/**
 * Offset/Address/Size: 0xED0 | 0x8000697C | size: 0xD4
 */
bool ClipPositionToSidelines(nlVector3& position, float margin)
{
    bool wasClipped = false;

    float rightBound = cField::GetGoalLineX(1U) - margin;
    float topBound = cField::GetSidelineY(1U) - margin;

    if (position.f.x > rightBound)
    {
        position.f.x = rightBound;
        wasClipped = true;
    }
    else
    {
        float temp_f0 = -1.0f * rightBound;
        if (position.f.x < temp_f0)
        {
            position.f.x = temp_f0;
            wasClipped = true;
        }
    }

    float temp_f1 = -1.0f * topBound;
    if (position.f.y < temp_f1)
    {
        position.f.y = temp_f1;
        wasClipped = true;
    }
    else if (position.f.y > topBound)
    {
        position.f.y = topBound;
        wasClipped = true;
    }

    return wasClipped;
}

/**
 * Offset/Address/Size: 0xDF4 | 0x800068A0 | size: 0xDC
 */
bool TestCollision(float rp, const nlVector3& p1, const nlVector3& p2, float rq, const nlVector3& q1, const nlVector3& q2)
{
    float combinedRadius = rp + rq;
    float radiusSq = combinedRadius * combinedRadius;

    // Check start positions
    nlVector3 diff1;
    nlVec3Sub(diff1, q1, p1);
    float distSq1 = nlGetLengthSquared3D(diff1.f.x, diff1.f.y, diff1.f.z);

    if (distSq1 <= radiusSq)
    {
        return true;
    }

    // Check end positions
    nlVector3 diff2;
    nlVec3Sub(diff2, q2, p2);
    float distSq2 = nlGetLengthSquared3D(diff2.f.x, diff2.f.y, diff2.f.z);

    if (distSq2 <= radiusSq)
    {
        return true;
    }

    // Do sweep test
    float t = SweepSpheres(rp, p1, p2, rq, q1, q2);
    if (t >= 0.0f && t <= 1.0f)
    {
        return true;
    }

    return false;
}

/**
 * Offset/Address/Size: 0xD90 | 0x8000683C | size: 0x64
 */
float Exp(float k)
{
    float result = 1.0f;
    float negk = -k;
    float c0 = 0.24999869f;
    float c1 = 0.031257585f;
    float powk;
    float c2 = 0.0025913713f;
    float c3 = 0.000171562f;
    float c4 = 0.0000054302f;
    float c5 = 0.0000006906f;
    float t0;
    float t1;
    float t2;

    result = c0 * negk + result;
    powk = negk * negk;
    result = c1 * powk + result;
    powk *= negk;
    result = c2 * powk + result;
    powk *= negk;
    result = c3 * powk + result;
    powk *= negk;
    result = c4 * powk + result;
    powk *= negk;
    result = c5 * powk + result;

    t0 = result * result;
    t1 = result * t0;
    t2 = result * t1;

    return 1.0f / t2;
}

/**
 * Offset/Address/Size: 0xCE8 | 0x80006794 | size: 0xA8
 */
float GetClosingSpeed2D(const nlVector3& pos1, const nlVector3& vel1, const nlVector3& pos2, const nlVector3& vel2)
{
    float dx, dy;
    dy = pos2.f.y - pos1.f.y;
    dx = pos2.f.x - pos1.f.x;
    float distSq = dx * dx + dy * dy;
    float invDist = nlRecipSqrt(distSq, true);

    float normDx = invDist * dx;
    float normDy = invDist * dy;

    float vel1Proj = normDx * vel1.f.x + normDy * vel1.f.y;
    float vel2Proj = normDx * vel2.f.x + normDy * vel2.f.y;

    return vel1Proj - vel2Proj;
}

/**
 * Offset/Address/Size: 0xC0C | 0x800066B8 | size: 0xDC
 */
float GetClosingSpeed(const nlVector3& pos1, const nlVector3& vel1, const nlVector3& pos2, const nlVector3& vel2)
{
    float dx, dy, dz;
    dz = pos2.f.z - pos1.f.z;
    dy = pos2.f.y - pos1.f.y;
    dx = pos2.f.x - pos1.f.x;
    float distSq = dx * dx + dy * dy + dz * dz;
    float invDist = nlRecipSqrt(distSq, true);

    float normDx = invDist * dx;
    float normDy = invDist * dy;
    float normDz = invDist * dz;

    float vel1Proj = normDx * vel1.f.x + normDy * vel1.f.y + normDz * vel1.f.z;
    float vel2Proj = normDx * vel2.f.x + normDy * vel2.f.y + normDz * vel2.f.z;

    return vel1Proj - vel2Proj;
}

/**
 * Offset/Address/Size: 0xB74 | 0x80006620 | size: 0x98
 */
void GetLocalPoint(nlVector3& v3LocalPointOut, const nlVector3& v3WorldPointIn, const nlVector3& v3RefPosition, unsigned short aRefAngle)
{
    float fSin; // r1+0xC
    float fCos; // r1+0x8

    nlSinCos(&fSin, &fCos, aRefAngle);
    float dx = v3WorldPointIn.f.x - v3RefPosition.f.x;
    float dy = v3WorldPointIn.f.y - v3RefPosition.f.y;

    v3LocalPointOut.f.x = (fCos * dx) + (fSin * dy);
    v3LocalPointOut.f.y = (fCos * dy) - (fSin * dx);
    v3LocalPointOut.f.z = v3WorldPointIn.f.z;
}

/**
 * Offset/Address/Size: 0xABC | 0x80006568 | size: 0xB8
 */
void GetWorldPoint(nlVector3& v3WorldPointOut, const nlVector3& v3LocalPointIn, const nlVector3& v3RefPosition, unsigned short aRefAngle)
{
    f32 fSin;
    f32 fCos;

    float localX = v3LocalPointIn.f.x;
    float localY = v3LocalPointIn.f.y;

    nlSinCos(&fSin, &fCos, aRefAngle);

    v3WorldPointOut.f.x = v3RefPosition.f.x + ((fCos * localX) - (fSin * localY));
    v3WorldPointOut.f.y = v3RefPosition.f.y + ((fCos * localY) + (fSin * localX));
    v3WorldPointOut.f.z = v3LocalPointIn.f.z;
}

/**
 * Offset/Address/Size: 0xA24 | 0x800064D0 | size: 0x98
 */
void RotateVectorZAxis(nlVector3& v3Out, const nlVector3& v3In, unsigned short angle)
{
    f32 fSin;
    f32 fCos;

    f32 localX = v3In.f.x;
    f32 localY = v3In.f.y;

    nlSinCos(&fSin, &fCos, angle);

    v3Out.f.x = (fCos * localX) - (fSin * localY);
    v3Out.f.y = (fCos * localY) + (fSin * localX);
    v3Out.f.z = v3In.f.z;
}

/**
 * Offset/Address/Size: 0x814 | 0x800062C0 | size: 0x210
 * TODO: 93.52% match - v3Vec1 components allocated to f7/f6/f5 instead of target
 * f8/f7/f6 (off by 1), causing cascading register diffs throughout function.
 * Extra fmr for ax copy. Likely MWCC register allocation behavior difference.
 */
void GetRotationBetweenVectors(nlQuaternion& quat, const nlVector3& v3Vec1, const nlVector3& v3Vec2)
{
    f32 t0;
    f32 t1;
    f32 t2;

    f32 fInvR1R2 = 1.0f / nlSqrt((v3Vec1.f.x * v3Vec1.f.x + v3Vec1.f.y * v3Vec1.f.y + v3Vec1.f.z * v3Vec1.f.z) * (v3Vec2.f.x * v3Vec2.f.x + v3Vec2.f.y * v3Vec2.f.y + v3Vec2.f.z * v3Vec2.f.z), true);
    f32 fCosAngle = fInvR1R2 * (v3Vec1.f.x * v3Vec2.f.x + v3Vec1.f.y * v3Vec2.f.y + v3Vec1.f.z * v3Vec2.f.z);

    if (fCosAngle > 0.999999f)
    {
        quat.f.x = 0.0f;
        quat.f.y = 0.0f;
        quat.f.z = 0.0f;
        quat.f.w = 1.0f;
    }
    else if (fCosAngle < -0.999999f)
    {
        f32 ay;
        f32 az;
        f32 ax = 1.0f;

        if (v3Vec1.f.x > v3Vec1.f.z || v3Vec1.f.y > v3Vec1.f.z)
        {
            ax = 0.0f;
            az = 1.0f;
            ay = 0.0f;
        }
        else
        {
            ay = 0.0f;
            az = 0.0f;
        }

        f32 nax = -ax;
        t1 = az * v3Vec1.f.x + nax * v3Vec1.f.z;
        t2 = ay * v3Vec1.f.z - az * v3Vec1.f.y;
        t0 = ax * v3Vec1.f.y - ay * v3Vec1.f.x;

        f32 invLen = nlRecipSqrt(t1 * t1 + t2 * t2 + t0 * t0, true);

        quat.f.x = invLen * t2;
        quat.f.y = invLen * t1;
        quat.f.z = invLen * t0;
        quat.f.w = 0.0f;
    }
    else
    {
        f32 fMagic = nlSqrt((f32)(0.5 * (1.0 + fCosAngle)), true);
        f32 fMultiplier = fInvR1R2 / fMagic;

        f32 cx = v3Vec1.f.z * v3Vec2.f.y;
        f32 cz = v3Vec1.f.y * v3Vec2.f.x;
        f32 negX = -v3Vec1.f.x;

        quat.f.w = 0.5f * fMagic;

        f32 cy = v3Vec1.f.z * v3Vec2.f.x;
        cx = v3Vec1.f.y * v3Vec2.f.z - cx;
        cz = v3Vec1.f.x * v3Vec2.f.y - cz;
        cy = negX * v3Vec2.f.z + cy;

        quat.f.x = cx * fMultiplier;
        quat.f.y = cy * fMultiplier;
        quat.f.z = cz * fMultiplier;
    }
}

/**
 * Offset/Address/Size: 0x70C | 0x800061B8 | size: 0x108
 */
void RotateVector(nlVector3& result, const nlVector3& v, nlQuaternion& q)
{
    f32 xz2;
    f32 R;
    f32 xw2;
    f32 P;
    f32 zz;
    f32 S;
    f32 G;
    f32 N;
    f32 z2;
    f32 C;
    f32 K;
    f32 y2;
    f32 O;
    f32 A;
    f32 H;
    f32 yw2;
    f32 xy2;
    f32 yz2;
    f32 vx;
    f32 I;
    f32 xx;
    f32 T;
    f32 J;
    f32 L;
    f32 F;
    f32 yy;
    f32 D;
    f32 M;
    f32 zw2;
    f32 vy;
    f32 vz;
    f32 ww;
    f32 E;
    f32 U;
    f32 x2;
    f32 B;

    xx = q.f.x * q.f.x;
    zz = q.f.z * q.f.z;
    ww = q.f.w * q.f.w;
    x2 = 2.0f * q.f.x;
    z2 = 2.0f * q.f.z;
    vy = v.f.y;
    y2 = 2.0f * q.f.y;
    vx = v.f.x;
    yy = q.f.y * q.f.y;

    A = ww - xx;
    B = xx + ww;
    vz = v.f.z;

    xy2 = x2 * q.f.y;
    zw2 = z2 * q.f.w;
    C = yy + A;
    D = B - yy;
    E = xy2 - zw2;
    F = C - zz;
    G = A - yy;
    xw2 = x2 * q.f.w;
    yz2 = y2 * q.f.z;
    xz2 = x2 * q.f.z;
    yw2 = y2 * q.f.w;
    H = xw2 + yz2;
    I = D - zz;
    J = vy * E;
    K = zw2 + xy2;
    L = vy * F;
    M = xz2 + yw2;
    N = vx * I + J;
    O = xz2 - yw2;
    P = vy * H;
    result.f.x = vz * M + N;
    R = yz2 - xw2;
    S = vx * K + L;
    T = zz + G;
    U = vx * O + P;
    result.f.y = vz * R + S;
    result.f.z = vz * T + U;
}

/**
 * Offset/Address/Size: 0x700 | 0x800061AC | size: 0xC
 */
float Interpolate(float fMin, float fMax, float fPercent)
{
    return (fPercent * (fMax - fMin)) + fMin;
}

/**
 * Offset/Address/Size: 0x670 | 0x8000611C | size: 0x90
 */
float InterpolateRangeClamped(float fResultMin, float fResultMax, float fInputMin, float fInputMax, float fInput)
{
    float range;

    if (fInputMin < fInputMax)
    {
        fInput = (fInput >= fInputMin) ? fInput : fInputMin;
        fInput = (fInput <= fInputMax) ? fInput : fInputMax;
    }
    else
    {
        fInput = (fInput >= fInputMax) ? fInput : fInputMax;
        fInput = (fInput <= fInputMin) ? fInput : fInputMin;
    }

    range = fInputMax - fInputMin;
    if (fabsf(range) < 0.001f)
    {
        return fResultMax;
    }

    return fResultMin + ((fInput - fInputMin) / range) * (fResultMax - fResultMin);
}

/**
 * Offset/Address/Size: 0x5D0 | 0x8000607C | size: 0xA0
 */
float InterpolateRangeClamped(const nlVector2& outputRange, const nlVector2& inputRange, float value)
{
    float maxVal = inputRange.f.y;
    float minVal = inputRange.f.x;
    float outMax = outputRange.f.y;
    float outMin = outputRange.f.x;

    // Clamp value to input range
    if (minVal < maxVal)
    {
        value = (value >= minVal) ? value : minVal;
        value = (value <= maxVal) ? value : maxVal;
    }
    else
    {
        value = (value >= maxVal) ? value : maxVal;
        value = (value <= minVal) ? value : minVal;
    }

    float range = maxVal - minVal;
    if ((float)__fabs((double)range) < 0.00001f)
    {
        return outMax;
    }

    return (value - minVal) / range * (outMax - outMin) + outMin;
}

/**
 * Offset/Address/Size: 0x580 | 0x8000602C | size: 0x50
 */
float NormalizeVal(float fromVal, const nlVector2& fromExtrema)
{
    if (fromExtrema.f.y == fromExtrema.f.x)
        return 1.0f;

    float t = (fromVal - fromExtrema.f.x) / (fromExtrema.f.y - fromExtrema.f.x);

    float lo = (t >= 0.0f) ? t : 0.0f;
    float res = (lo <= 1.0f) ? lo : 1.0f;

    return res;
}

/**
 * Offset/Address/Size: 0x538 | 0x80005FE4 | size: 0x48
 */
float NormalizeVal(float fromVal, float fromMin, float fromMax)
{
    if (fromMin == fromMax)
        return 1.0f;

    float t = (fromVal - fromMin) / (fromMax - fromMin);

    float lo = (t >= 0.0f) ? t : 0.0f;
    float res = (lo <= 1.0f) ? lo : 1.0f;

    return res;
}

/**
 * Offset/Address/Size: 0x518 | 0x80005FC4 | size: 0x20
 */
float AIsgn(float fValue)
{
    if (fValue >= 0.0f)
    {
        return 1.0f;
    }
    return -1.0f;
}

/**
 * Offset/Address/Size: 0x2E4 | 0x80005D90 | size: 0x234
 * TODO: 96.56% match - remaining diffs are f30/f31 saved FPR swap in MWCC
 * graph-coloring register allocator (lenSq gets f31 instead of f30)
 */
nlVector3 GetClosestPointOnLineABFromPointC(const nlVector3& a, const nlVector3& b, const nlVector3& c)
{
    f32 v12 = c.f.x - a.f.x;
    f32 v10 = b.f.x - a.f.x;
    f32 v13 = c.f.y - a.f.y;
    f32 v31 = b.f.y - a.f.y;
    f32 v29 = c.f.z - a.f.z;
    f32 v28 = b.f.z - a.f.z;

    f32 dot = v13 * v31;
    f32 lenSq = v31 * v31;
    dot = v12 * v10 + dot;
    lenSq = v10 * v10 + lenSq;

    f32 cX = a.f.x + v12;
    dot = v29 * v28 + dot;
    const f32 fLenSq = v28 * v28 + lenSq;
    f32 cY = a.f.y + v13;
    f32 cZ = a.f.z + v29;

    f32 t = dot / fLenSq;
    f32 tx = t * v10;
    f32 v8 = t * v31;
    f32 v1 = t * v28;

    tx = v12 - tx;
    v8 = v13 - v8;
    v1 = v29 - v1;

    f32 projX = cX - tx;
    f32 projY = cY - v8;
    f32 projZ = cZ - v1;

    nlVector3 projected;
    projected.f.x = projX;
    projected.f.y = projY;
    projected.f.z = projZ;

    v29 = a.f.y - projY;
    f32 v27 = a.f.z - projZ;
    v28 = a.f.x - projX;
    v31 = b.f.z - projZ;
    f32 v25 = b.f.y - projY;
    f32 v26 = b.f.x - projX;

    f32 lineLen = nlSqrt(fLenSq, true);
    f32 distASq = v29 * v29 + v28 * v28 + v27 * v27;
    f32 distA = nlSqrt(distASq, true);

    if (distA > lineLen)
        goto lbl_outside;
    {
        f32 lineLen2 = nlSqrt(fLenSq, true);
        f32 distBSq = v25 * v25 + v26 * v26 + v31 * v31;
        f32 distB = nlSqrt(distBSq, true);
        if (!(distB > lineLen2))
            goto lbl_return_proj;
    }
lbl_outside:
{
    f32 distBSq2 = v25 * v25 + v26 * v26 + v31 * v31;
    f32 distB2 = nlSqrt(distBSq2, true);
    f32 distA2 = nlSqrt(distASq, true);
    if (distA2 < distB2)
    {
        return a;
    }
    return b;
}
lbl_return_proj:
    return projected;
}

/**
 * Offset/Address/Size: 0x160 | 0x80005C0C | size: 0x184
 */
void SortToMinOrMaxTotalSum(unsigned int* result, float (*data)[4], bool findMin)
{
    f32 bestSum;

    if (findMin)
    {
        bestSum = 100000.0f;
    }
    else
    {
        bestSum = -100000.0f;
    }

    for (s32 i = 0; i < 4; i++)
    {
        f32 sum = data[0][i];
        for (s32 j = 0; j < 4; j++)
        {
            if (j == i)
                continue;
            f32 save2 = sum;
            sum += data[1][j];
            for (s32 k = 0; k < 4; k++)
            {
                if (k == j)
                    continue;
                if (k == i)
                    continue;
                f32 save3 = sum;
                sum += data[2][k];
                for (s32 l = 0; l < 4; l++)
                {
                    if (l == k)
                        continue;
                    if (l == j)
                        continue;
                    if (l == i)
                        continue;
                    f32 save4 = sum;
                    sum += data[3][l];
                    if ((findMin && sum < bestSum) || (!findMin && sum > bestSum))
                    {
                        result[0] = i;
                        result[1] = j;
                        result[2] = k;
                        result[3] = l;
                        bestSum = sum;
                    }
                    sum = save4;
                }
                sum = save3;
            }
            sum = save2;
        }
    }
}

/**
 * Offset/Address/Size: 0x134 | 0x80005BE0 | size: 0x2C
 */
char* GetPowerupName(int powerup)
{
    if ((powerup >= 0) && (powerup < 9))
    {
        return g_sPowerupNames[powerup];
    }
    return "Unknown";
}

/**
 * Offset/Address/Size: 0x0 | 0x80005AAC | size: 0x134
 */
nlVector3 GetClosestPointOnSidelines(const nlVector3& v3Position)
{
    nlVector3 v3WallPosition;
    f32 fSign;
    f32 fDistToGoalLine;
    f32 fDistToSideline;

    fDistToGoalLine = (f32)fabs(cField::GetGoalLineX(1U) - (f32)fabs(v3Position.f.x));
    fDistToSideline = (f32)fabs(cField::GetSidelineY(1U) - (f32)fabs(v3Position.f.y));

    if (fDistToGoalLine <= fDistToSideline)
    {
        v3WallPosition.f.y = v3Position.f.y;
        if (v3Position.f.x >= 0.0f)
        {
            fSign = 1.0f;
        }
        else
        {
            fSign = -1.0f;
        }
        f32 fGoalX = cField::GetGoalLineX(1U);
        v3WallPosition.f.x = fGoalX * fSign;
    }
    else
    {
        if (v3Position.f.y >= 0.0f)
        {
            fSign = 1.0f;
        }
        else
        {
            fSign = -1.0f;
        }
        f32 fSideY = cField::GetSidelineY(1U);
        v3WallPosition.f.y = fSideY * fSign;
        v3WallPosition.f.x = v3Position.f.x;
    }

    v3WallPosition.f.z = 0.0f;
    return v3WallPosition;
}
