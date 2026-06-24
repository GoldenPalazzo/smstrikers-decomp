#ifndef _DFINITEPLANE_H_
#define _DFINITEPLANE_H_

#include "ode/collision.h"

dGeomID dCreateFinitePlane(dSpaceID space, dReal a, dReal b, dReal c, dReal d, bool flag, float param);
int dCollideFinitePlaneSphere(dxGeom*, dxGeom*, int, dContactGeom*, int);
void dFinitePlaneAABB(dxGeom*, float*);

#endif // _DFINITEPLANE_H_
