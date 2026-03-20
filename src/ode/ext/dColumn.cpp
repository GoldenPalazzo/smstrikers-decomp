#include "ode/ext/dColumn.h"
#include "math.h"

s32 dColumnClassUser = -1;
u8 lengthwiseAxis = 0x2;

/**
 * Offset/Address/Size: 0x5A4 | 0x8021D3D0 | size: 0x34
 */
void dGeomColumnGetParams(dxGeom* geomID, float* radius)
{
    *radius = *(float*)dGeomGetClassData(geomID);
}

/**
 * Offset/Address/Size: 0x514 | 0x8021D340 | size: 0x90
 */
void dColumnAABB(dxGeom* geomID, float* aabb)
{
    f32 radius;
    float* position;

    radius = *(float*)dGeomGetClassData(geomID);
    position = (float*)dGeomGetPosition(geomID);

    aabb[0] = position[0] - radius;
    aabb[1] = position[0] + radius;
    aabb[2] = position[1] - radius;
    aabb[3] = position[1] + radius;
    aabb[4] = -radius;
    aabb[5] = radius;
}

/**
 * Offset/Address/Size: 0x28C | 0x8021D0B8 | size: 0x288
 */
int dCollideColumnColumn(dxGeom* geomID0, dxGeom* geomID1, int arg2, dContactGeom* arg3, int arg4)
{
    f32 sp8;
    f32 temp_f1;
    f32 temp_f2;
    f32 temp_f3;
    f32 radius_0;
    f32 radius_1;
    f32 temp_f4;
    f32 temp_f4_2;
    f32 temp_f4_3;
    f32 var_f5;
    f64 temp_f1_2;
    f64 temp_f1_3;
    f64 temp_f1_4;
    s32 temp_r4;
    s32 temp_r5;
    s32 temp_r6;
    s32 var_r0;
    s32 var_r4;
    s32 var_r5;
    f32* pos_0;
    f32* pos_1;
    f32* temp_r7;
    f32* temp_r8;

    radius_0 = *(f32*)dGeomGetClassData(geomID0);
    radius_1 = *(f32*)dGeomGetClassData(geomID1);
    pos_0 = (f32*)dGeomGetPosition(geomID0);
    pos_1 = (f32*)dGeomGetPosition(geomID1);
    var_r4 = 0xFF;
    var_r5 = 0xFF;
    if (lengthwiseAxis == 0)
    {
        var_r4 = 1;
        var_r5 = 2;
    }
    if (lengthwiseAxis == 1)
    {
        var_r4 = 0;
        var_r5 = 2;
    }
    if (lengthwiseAxis == 2)
    {
        var_r4 = 0;
        var_r5 = 1;
    }

    temp_r6 = ((u8)var_r4) << 2;
    temp_r5 = ((u8)var_r5) << 2;
    temp_f4 = radius_0 + radius_1;

    temp_f3 = *(f32*)((u8*)pos_0 + temp_r6);
    temp_f2 = *(f32*)((u8*)pos_1 + temp_r6);
    temp_f2 = temp_f3 - temp_f2;
    temp_f1 = *(f32*)((u8*)pos_0 + temp_r5);
    temp_f3 = *(f32*)((u8*)pos_1 + temp_r5);
    temp_f1 = temp_f1 - temp_f3;
    temp_f4_2 = temp_f2 * temp_f2;
    temp_f4_2 = temp_f4_2 + (temp_f1 * temp_f1);

    if (temp_f4_2 < (temp_f4 * temp_f4))
    {
        if (temp_f4_2 > 0.0f)
        {
            temp_f1_2 = __frsqrte(temp_f4_2);
            temp_f1_3 = 0.5 * temp_f1_2 * -(((f64)temp_f4_2 * (temp_f1_2 * temp_f1_2)) - 3.0);
            temp_f1_4 = 0.5 * temp_f1_3 * -(((f64)temp_f4_2 * (temp_f1_3 * temp_f1_3)) - 3.0);
            var_f5 = (f32)((f64)temp_f4_2 * (0.5 * temp_f1_4 * -(((f64)temp_f4_2 * (temp_f1_4 * temp_f1_4)) - 3.0)));
        }
        else if (temp_f4_2 < 0.0)
        {
            var_f5 = *(f32*)&__float_nan;
        }
        else
        {
            sp8 = temp_f4_2;
            temp_r4 = *(s32*)&sp8 & 0x7F800000;
            switch (temp_r4)
            {
            case 0x7F800000:
                if (*(s32*)&sp8 & 0x7FFFFF)
                {
                    var_r0 = 1;
                }
                else
                {
                    var_r0 = 2;
                }
                break;
            case 0x0:
                if (*(s32*)&sp8 & 0x7FFFFF)
                {
                    var_r0 = 5;
                }
                else
                {
                    var_r0 = 3;
                }
                break;
            default:
                var_r0 = 4;
                break;
            }
            if (var_r0 == 1)
            {
                var_f5 = *(f32*)&__float_nan;
            }
            else
            {
                var_f5 = temp_f4_2;
            }
        }

        temp_r7 = (f32*)((u8*)arg3 + temp_r6);
        temp_r8 = (f32*)((u8*)arg3 + temp_r5);

        temp_r7[4] = (*(f32*)((u8*)pos_0 + temp_r6) - *(f32*)((u8*)pos_1 + temp_r6)) / var_f5;
        temp_f4_3 = ((var_f5 + radius_1) - radius_0) * 0.5f;
        temp_r8[4] = (*(f32*)((u8*)pos_0 + temp_r5) - *(f32*)((u8*)pos_1 + temp_r5)) / var_f5;

        *(f32*)((u8*)arg3 + (lengthwiseAxis * 4) + 0x10) = 0.0f;
        *(f32*)((u8*)arg3 + temp_r6) = (temp_r7[4] * temp_f4_3) + *(f32*)((u8*)pos_1 + temp_r6);
        *(f32*)((u8*)arg3 + temp_r5) = (temp_r8[4] * temp_f4_3) + *(f32*)((u8*)pos_1 + temp_r5);
        *(f32*)((u8*)arg3 + (lengthwiseAxis * 4)) = 0.0f;
        arg3->depth = radius_1 - (var_f5 - radius_1);
        arg3->g1 = geomID0;
        arg3->g2 = geomID1;
        return 1;
    }

    return 0;
}

/**
 * Offset/Address/Size: 0xE0 | 0x8021CF0C | size: 0x1AC
 */
int dCollideColumnPlane(dxGeom* o1, dxGeom* o2, int flags, dContactGeom* contact, int skip)
{
    dVector4 n;
    dReal radius;
    dReal d;
    const dReal* pos;
    u8 ax1;
    u8 ax2;
    u8 perpendicular;
    dReal distance;

    radius = *(dReal*)dGeomGetClassData(o1);
    dGeomPlaneGetParams(o2, n);
    d = n[3];
    pos = (const dReal*)dGeomGetPosition(o1);
    ax1 = 0xFF;
    ax2 = 0xFF;
    perpendicular = 0;
    if (lengthwiseAxis == 0)
    {
        ax1 = 1;
        ax2 = 2;
        if (n[0] < 0.001f)
        {
            perpendicular = 1;
        }
    }
    if (lengthwiseAxis == 1)
    {
        ax1 = 0;
        ax2 = 2;
        if (n[1] < 0.001f)
        {
            perpendicular = 1;
        }
    }
    if (lengthwiseAxis == 2)
    {
        ax1 = 0;
        ax2 = 1;
        if (n[2] < 0.001f)
        {
            perpendicular = 1;
        }
    }
    if (!perpendicular)
    {
        return 0;
    }
    distance = n[ax1] * pos[ax1] + n[ax2] * pos[ax2] - d;
    if (distance < radius)
    {
        contact->normal[0] = n[0];
        contact->normal[1] = n[1];
        contact->normal[2] = n[2];
        contact->pos[ax1] = pos[ax1] - contact->normal[ax1] * radius;
        contact->pos[ax2] = pos[ax2] - contact->normal[ax2] * radius;
        contact->pos[lengthwiseAxis] = 0.0f;
        contact->depth = distance - radius;
        contact->g1 = o1;
        contact->g2 = o2;
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0xB0 | 0x8021CEDC | size: 0x30
 */
dColliderFn* dColumnColliderFn(int num)
{
    if (num == (s32)dColumnClassUser)
    {
        return &dCollideColumnColumn;
    }
    if (num == 4)
    {
        return &dCollideColumnPlane;
    }
    return NULL;
}

/**
 * Offset/Address/Size: 0x0 | 0x8021CE2C | size: 0xB0
 */
dGeomID dCreateColumn(dxSpace* space, float radius)
{
    dGeomClass geomClass;
    dGeomID geomID;

    if ((s32)dColumnClassUser == -1)
    {
        geomClass.bytes = 4;
        geomClass.collider = &dColumnColliderFn;
        geomClass.aabb = &dColumnAABB;
        geomClass.aabb_test = 0;
        geomClass.dtor = 0;
        dColumnClassUser = dCreateGeomClass(&geomClass);
    }
    geomID = dCreateGeom(dColumnClassUser);
    if (space != NULL)
    {
        dSpaceAdd(space, geomID);
    }
    *(float*)dGeomGetClassData(geomID) = radius;
    return geomID;
}
