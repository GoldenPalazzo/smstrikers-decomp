#include "Game/Physics/PhysicsFinitePlane.h"
#include "ode/ext/dFinitePlane.h"
#include "NL/nlMath.h"

/**
 * Offset/Address/Size: 0x0 | 0x801FFAE4 | size: 0x218
 */
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

    nlMatrix3 R;
    nlVector3 normal;
    nlVec3CrossProductAlt(normal, v1, v2);
    R.m[0] = v1.f.x;
    R.m[1] = v1.f.y;
    R.m[2] = v1.f.z;
    R.m[3] = v2.f.x;
    R.m[4] = v2.f.y;
    R.m[5] = v2.f.z;
    R.m[6] = normal.f.z;
    R.m[7] = normal.f.y;
    R.m[8] = normal.f.x;

    dSpaceID space = NULL;
    if (collision_space != NULL)
    {
        space = collision_space->m_spaceID;
    }

    m_geomID = dCreateFinitePlane(space, xMin, xMax, yMin, yMax, isOneSided, errorCorrectionDepth);
    dGeomSetData(m_geomID, this);
    SetRotation(R);
    SetPosition(centre, WORLD_COORDINATES);
    SetDefaultCollideBits();
}
