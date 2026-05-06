#include "Game/Physics/PhysicsFinitePlane.h"
#include "ode/ext/dFinitePlane.h"

/**
 * Offset/Address/Size: 0x0 | 0x801FFAE4 | size: 0x218
 */
//  void * PhysicsFinitePlane::PhysicsFinitePlane(class CollisionSpace * space /* r27 */, class nlVector3 & centre /* r28 */, class nlVector3 & v1 /* r29 */, class nlVector3 & v2 /* r30 */, unsigned char isOneSided /* r31 */, float errorCorrectionDepth /* f31 */) {

inline void nlVec3Cross(nlVector3& result, const nlVector3& a, const nlVector3& b)
{
    nlVec3Set(result,
        (a.f.y * b.f.z) - (a.f.z * b.f.y),
        (a.f.z * b.f.x) - (a.f.x * b.f.z),
        (a.f.x * b.f.y) - (a.f.y * b.f.x));
}

PhysicsFinitePlane::PhysicsFinitePlane(CollisionSpace* collision_space, nlVector3& centre, nlVector3& v1, nlVector3& v2, bool isOneSided, float errorCorrectionDepth)
    : PhysicsObject(NULL)
{
    mErrorCorrectionDepth = errorCorrectionDepth;

    xMin = 0.f;
    xMax = 0.f;
    yMin = 0.f;
    yMax = 0.f;

    xMax = nlSqrt(v1.f.x * v1.f.x + v1.f.y * v1.f.y + v1.f.z * v1.f.z, true);
    yMax = nlSqrt(v2.f.x * v2.f.x + v2.f.y * v2.f.y + v2.f.z * v2.f.z, true);

    xMin = -xMax;
    yMin = -yMax;

    const float l = 1.f / xMax;
    nlVec3Set(v1, l * v1.f.x, l * v1.f.y, l * v1.f.z);

    const float l2 = 1.f / yMax;
    nlVec3Set(v2, l2 * v2.f.x, l2 * v2.f.y, l2 * v2.f.z);

    nlMatrix3 mat;

    float temp_f5 = v1.f.y;
    float temp_f1_3 = v2.f.x;
    float temp_f3_2 = v1.f.z;
    float temp_f6 = v2.f.y;
    float temp_f4_2 = v2.f.z;
    float temp_f7 = v1.f.x;

    volatile nlVector3& vv1 = (volatile nlVector3&)v1;
    volatile nlVector3& vv2 = (volatile nlVector3&)v2;

    mat.m[0] = temp_f7;
    mat.m[1] = vv1.f.y;
    mat.m[2] = vv1.f.z;
    mat.m[3] = vv2.f.x;
    mat.m[4] = vv2.f.y;
    mat.m[5] = vv2.f.z;
    mat.m[6] = (temp_f5 * temp_f4_2) - (temp_f3_2 * temp_f6);
    mat.m[7] = (-temp_f7 * temp_f4_2) + (temp_f3_2 * temp_f1_3);
    mat.m[8] = (temp_f7 * temp_f6) - (temp_f5 * temp_f1_3);

    dSpaceID space = NULL;
    if (collision_space != NULL)
    {
        space = collision_space->m_spaceID;
    }

    m_geomID = dCreateFinitePlane(space, xMin, xMax, yMin, yMax, isOneSided, errorCorrectionDepth);
    dGeomSetData(m_geomID, this);
    SetRotation(mat);
    SetPosition(centre, WORLD_COORDINATES);
    SetDefaultCollideBits();
}
