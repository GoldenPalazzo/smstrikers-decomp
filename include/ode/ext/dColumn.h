#ifndef _DCOLUMN_H_
#define _DCOLUMN_H_

#include "ode/collision.h"

dGeomID dCreateColumn(dxSpace*, float);
int dCollideColumnPlane(dxGeom*, dxGeom*, int, dContactGeom*, int);
int dCollideColumnColumn(dxGeom*, dxGeom*, int, dContactGeom*, int);
void dGeomColumnGetParams(dxGeom*, float*);

#endif // _DCOLUMN_H_
