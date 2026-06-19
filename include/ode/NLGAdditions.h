#ifndef _NLGADDITIONS_H_
#define _NLGADDITIONS_H_

#include "ode/common.h"
#include "ode/collision.h"
#include "ode/joint.h"

struct dxJointCharacter : public dxJoint
{
    float direction[4]; // offset 0x50, size 0x10
}; // total size: 0x60

void dGeomCollideAABBs(dxGeom*, dxGeom*, void*, void (*)(void*, dxGeom*, dxGeom*));
void dGeomMarkAABBAsValid(dxGeom*);
void dGeomComputeAABB(dxGeom*);
void dVector3Add(float*, const float*);
void dVectorScale(float*, float);
void dVector4Set(float*, float, float, float, float);
void dVector3Set(float*, float, float, float);
void dExtractColumn3(float* __restrict, const float* __restrict, int);
void dInvertRigidTransformation(float*, const float*, const float*);
void dMultiplyMatrix4Vector4(float*, const float*, const float*);
void dMultiplyMatrix3Vector3(float*, const float*, const float*, bool);
void dGeomSetGFlags(dxGeom*, int);
int dGeomGetGFlags(dxGeom*);
void dJointSetCharacterNoMotionDirection(dxJoint*, float*);
dxJoint* dJointCreateCharacter(dxWorld*, dxJointGroup*);
void dClearCachedData();
void dWorldSetClearAccumulators(dxWorld*, int);
dxBody* dBodyGetNextBody(dxBody*);
dxBody* dWorldGetFirstBody(dxWorld*);
void dBodySetUpdateMode(dxBody*, int, int);

#endif // _NLGADDITIONS_H_
