#ifndef _DROUNDEDCORNER_H_
#define _DROUNDEDCORNER_H_

#include "ode/collision.h"

dGeomID dCreateRoundedCorner(dxSpace*, float, bool, bool);
int dCollideRoundedCornerColumn(dxGeom*, dxGeom*, int, dContactGeom*, int);
int dCollideRoundedCornerSphere(dxGeom*, dxGeom*, int, dContactGeom*, int);

#endif // _DROUNDEDCORNER_H_
