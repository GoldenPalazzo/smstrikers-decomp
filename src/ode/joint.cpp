/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

/*

design note: the general principle for giving a joint the option of connecting
to the static environment (i.e. the absolute frame) is to check the second
body (joint->node[1].body), and if it is zero then behave as if its body
transform is the identity.

*/

#include <ode/odemath.h>
#include <ode/rotation.h>
#include <ode/matrix.h>
#include "joint.h"

//****************************************************************************
// contact

/**
 * Offset/Address/Size: 0x830 | 0x8021EAD8 | size: 0x4
 */
static void contactInit(dxJointContact* j)
{
    // default frictionless contact. hmmm, this info gets overwritten straight
    // away anyway, so why bother?
#if 0 /* so don't bother ;) */
  j->contact.surface.mode = 0;
  j->contact.surface.mu = 0;
  dSetZero (j->contact.geom.pos,4);
  dSetZero (j->contact.geom.normal,4);
  j->contact.geom.depth = 0;
#endif
}

/**
 * Offset/Address/Size: 0x76C | 0x8021EA14 | size: 0xC4
 */
static void contactGetInfo1(dxJointContact* j, dxJoint::Info1* info)
{
    // make sure mu's >= 0, then calculate number of constraint rows and number
    // of unbounded rows.
    int m = 1, nub = 0;
    if (j->contact.surface.mu < 0)
        j->contact.surface.mu = 0;
    if (j->contact.surface.mode & dContactMu2)
    {
        if (j->contact.surface.mu > 0)
            m++;
        if (j->contact.surface.mu2 < 0)
            j->contact.surface.mu2 = 0;
        if (j->contact.surface.mu2 > 0)
            m++;
        if (j->contact.surface.mu == dInfinity)
            nub++;
        if (j->contact.surface.mu2 == dInfinity)
            nub++;
    }
    else
    {
        if (j->contact.surface.mu > 0)
            m += 2;
        if (j->contact.surface.mu == dInfinity)
            nub += 2;
    }

    j->the_m = m;
    info->m = m;
    info->nub = nub;
}

/**
 * Offset/Address/Size: 0x0 | 0x8021E2A8 | size: 0x76C
 */
static void contactGetInfo2(dxJointContact* j, dxJoint::Info2* info)
{
    int i, s = info->rowskip;
    int s2 = 2 * s;

    // get normal, with sign adjusted for body1/body2 polarity
    dVector3 normal;
    if (j->flags & dJOINT_REVERSE)
    {
        normal[0] = -j->contact.geom.normal[0];
        normal[1] = -j->contact.geom.normal[1];
        normal[2] = -j->contact.geom.normal[2];
    }
    else
    {
        normal[0] = j->contact.geom.normal[0];
        normal[1] = j->contact.geom.normal[1];
        normal[2] = j->contact.geom.normal[2];
    }
    normal[3] = 0; // @@@ hmmm

    // c1,c2 = contact points with respect to body PORs
    dVector3 c1, c2;
    for (i = 0; i < 3; i++)
        c1[i] = j->contact.geom.pos[i] - j->node[0].body->pos[i];

    // set jacobian for normal
    info->J1l[0] = normal[0];
    info->J1l[1] = normal[1];
    info->J1l[2] = normal[2];
    dCROSS(info->J1a, =, c1, normal);
    if (j->node[1].body)
    {
        for (i = 0; i < 3; i++)
            c2[i] = j->contact.geom.pos[i] - j->node[1].body->pos[i];
        info->J2l[0] = -normal[0];
        info->J2l[1] = -normal[1];
        info->J2l[2] = -normal[2];
        dCROSS(info->J2a, = -, c2, normal);
    }

    // set right hand side and cfm value for normal
    dReal erp = info->erp;
    if (j->contact.surface.mode & dContactSoftERP)
        erp = j->contact.surface.soft_erp;
    dReal k = info->fps * erp;
    dReal depth = j->contact.geom.depth - j->world->contactp.min_depth;
    if (depth < 0)
        depth = 0;
    dReal maxvel = j->world->contactp.max_vel;
    if (k * depth > maxvel)
        info->c[0] = maxvel;
    else
        info->c[0] = k * depth;
    if (j->contact.surface.mode & dContactSoftCFM)
        info->cfm[0] = j->contact.surface.soft_cfm;

    // deal with bounce
    if (j->contact.surface.mode & dContactBounce)
    {
        // calculate outgoing velocity (-ve for incoming contact)
        dReal outgoing = dDOT(info->J1l, j->node[0].body->lvel) + dDOT(info->J1a, j->node[0].body->avel);
        if (j->node[1].body)
        {
            outgoing += dDOT(info->J2l, j->node[1].body->lvel) + dDOT(info->J2a, j->node[1].body->avel);
        }
        // only apply bounce if the outgoing velocity is greater than the
        // threshold, and if the resulting c[0] exceeds what we already have.
        if (j->contact.surface.bounce_vel >= 0 && (-outgoing) > j->contact.surface.bounce_vel)
        {
            dReal newc = -j->contact.surface.bounce * outgoing;
            if (newc > info->c[0])
                info->c[0] = newc;
        }
    }

    // set LCP limits for normal
    info->lo[0] = 0;
    info->hi[0] = dInfinity;

    // now do jacobian for tangential forces
    dVector3 t1, t2; // two vectors tangential to normal

    // first friction direction
    if (j->the_m >= 2)
    {
        if (j->contact.surface.mode & dContactFDir1)
        { // use fdir1 ?
            t1[0] = j->contact.fdir1[0];
            t1[1] = j->contact.fdir1[1];
            t1[2] = j->contact.fdir1[2];
            dCROSS(t2, =, normal, t1);
        }
        else
        {
            dPlaneSpace(normal, t1, t2);
        }
        info->J1l[s + 0] = t1[0];
        info->J1l[s + 1] = t1[1];
        info->J1l[s + 2] = t1[2];
        dCROSS(info->J1a + s, =, c1, t1);
        if (j->node[1].body)
        {
            info->J2l[s + 0] = -t1[0];
            info->J2l[s + 1] = -t1[1];
            info->J2l[s + 2] = -t1[2];
            dCROSS(info->J2a + s, = -, c2, t1);
        }
        // set right hand side
        if (j->contact.surface.mode & dContactMotion1)
        {
            info->c[1] = j->contact.surface.motion1;
        }
        // set LCP bounds and friction index. this depends on the approximation
        // mode
        info->lo[1] = -j->contact.surface.mu;
        info->hi[1] = j->contact.surface.mu;
        if (j->contact.surface.mode & dContactApprox1_1)
            info->findex[1] = 0;

        // set slip (constraint force mixing)
        if (j->contact.surface.mode & dContactSlip1)
            info->cfm[1] = j->contact.surface.slip1;
    }

    // second friction direction
    if (j->the_m >= 3)
    {
        info->J1l[s2 + 0] = t2[0];
        info->J1l[s2 + 1] = t2[1];
        info->J1l[s2 + 2] = t2[2];
        dCROSS(info->J1a + s2, =, c1, t2);
        if (j->node[1].body)
        {
            info->J2l[s2 + 0] = -t2[0];
            info->J2l[s2 + 1] = -t2[1];
            info->J2l[s2 + 2] = -t2[2];
            dCROSS(info->J2a + s2, = -, c2, t2);
        }
        // set right hand side
        if (j->contact.surface.mode & dContactMotion2)
        {
            info->c[2] = j->contact.surface.motion2;
        }
        // set LCP bounds and friction index. this depends on the approximation
        // mode
        if (j->contact.surface.mode & dContactMu2)
        {
            info->lo[2] = -j->contact.surface.mu2;
            info->hi[2] = j->contact.surface.mu2;
        }
        else
        {
            info->lo[2] = -j->contact.surface.mu;
            info->hi[2] = j->contact.surface.mu;
        }
        if (j->contact.surface.mode & dContactApprox1_2)
            info->findex[2] = 0;

        // set slip (constraint force mixing)
        if (j->contact.surface.mode & dContactSlip2)
            info->cfm[2] = j->contact.surface.slip2;
    }
}

dxJoint::Vtable __dcontact_vtable = {
    sizeof(dxJointContact),
    (dxJoint::init_fn*)contactInit,
    (dxJoint::getInfo1_fn*)contactGetInfo1,
    (dxJoint::getInfo2_fn*)contactGetInfo2,
    dJointTypeContact
};
