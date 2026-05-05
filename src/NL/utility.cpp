#include "NL/utility.h"
#include "math.h"

static f32 CANT_COLLIDE = *(f32*)__float_max;

/**
 * Offset/Address/Size: 0x0 | 0x801F08C4 | size: 0x1A8
 */
void SolveQuadratic(float a, float b, float c, int& numRoots, float& x1, float& x2)
{
    float absA = (float)fabs(a);
    float absB = (float)fabs(b);
    float absC = (float)fabs(c);
    float q;
    float p;
    float r;

    if (absA < 0.00001f && absB < 0.00001f)
    {
        numRoots = 0;
        return;
    }

    if (absA > absB && absA > absC)
    {
        q = b / a;
        p = 1.0f;
        r = c / a;
    }
    else if (absB > absA && absB > absC)
    {
        p = a / b;
        q = 1.0f;
        r = c / b;
    }
    else
    {
        p = a / c;
        r = 1.0f;
        q = b / c;
    }

    if (absA < 0.00001f)
    {
        numRoots = 1;
        x1 = -r / q;
        return;
    }

    float d = q * q - (4.0f * p * r);
    if (d < 0.0f)
    {
        numRoots = 0;
        return;
    }

    float sqrtd = nlSqrt(d, true);
    float z;
    if (q > 0.0f)
    {
        z = -0.5f * (q + sqrtd);
    }
    else
    {
        z = -0.5f * (q - sqrtd);
    }

    x1 = z / p;
    if ((float)fabs(z) < 0.00001f)
    {
        numRoots = 1;
        return;
    }

    x2 = r / z;
    numRoots = 2;
}

/**
 * Offset/Address/Size: 0x1A8 | 0x801F0A6C | size: 0x148
 */
float SweepSpheres(float rp, const nlVector3& p1, const nlVector3& p2, float rq,
    const nlVector3& q1, const nlVector3& q2)
{
    nlVector3 d, v;
    d.f.x = p1.f.x - q1.f.x;
    d.f.y = p1.f.y - q1.f.y;
    d.f.z = p1.f.z - q1.f.z;
    v.f.x = (p2.f.x - p1.f.x) - (q2.f.x - q1.f.x);
    v.f.y = (p2.f.y - p1.f.y) - (q2.f.y - q1.f.y);
    v.f.z = (p2.f.z - p1.f.z) - (q2.f.z - q1.f.z);

    float dsq = d.GetLengthSq3D();
    float a = v.GetLengthSq3D();

    if ((float)fabs(a) <= 0.00001f)
    {
        return CANT_COLLIDE;
    }

    float dot = d.f.x * v.f.x + d.f.y * v.f.y + d.f.z * v.f.z;
    float r = rp + rq;
    float rsq = r * r;

    float dotSq = dot * dot;
    float closest = dsq - dotSq / a;
    if (closest > rsq)
    {
        return CANT_COLLIDE;
    }

    if (dsq <= rsq)
    {
        return 0.0f;
    }

    float disc = dotSq - a * (dsq - rsq);
    if (disc < 0.0f)
    {
        return CANT_COLLIDE;
    }

    float t = (-dot - nlSqrt(disc, true)) / a;
    return t;
}

// /**
//  * Offset/Address/Size: 0x2F0 | 0x801F0BB4 | size: 0x10
//  */
// void 0x8028D55C..0x8028D560 | size : 0x4
// {
// }
