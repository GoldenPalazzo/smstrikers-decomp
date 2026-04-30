#include "Game/Drawable/DrawablePowerup.h"
#include "Game/Drawable/DrawableModel.h"
#include "Game/ReplaySpecializations.h"
#include "Game/WorldManager.h"
#include "Game/AI/Powerups.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"
#include "NL/nlString.h"

static char* PowerupLookupString(int idx)
{
    static char powerupLookup[] = "powerup_generated_xxx";

    if (idx < 10)
    {
        powerupLookup[18] = idx + '0';
        powerupLookup[19] = '\0';
        powerupLookup[20] = '\0';
    }
    else if (idx < 100)
    {
        powerupLookup[18] = (idx / 10) + '0';
        powerupLookup[19] = (idx % 10) + '0';
        powerupLookup[20] = '\0';
    }
    else
    {
        powerupLookup[18] = (idx / 100) + '0';
        powerupLookup[19] = ((idx % 100) / 10) + '0';
        powerupLookup[20] = (idx % 10) + '0';
    }

    return powerupLookup;
}

/**
 * Offset/Address/Size: 0x0 | 0x8011EC74 | size: 0xF8
 * TODO: 90.8% match - MWCC still reorders FPR usage around duplicated
 * blendFactors[2] loads used for scale interpolation.
 */
void DrawablePowerup::Blend(const float* blendFactors, const DrawablePowerup& lhs, const DrawablePowerup& rhs)
{
    mVisible = lhs.mVisible && rhs.mVisible;
    if (!mVisible)
        return;

    float t = blendFactors[2];
    float one = 1.0f;
    float invT = one - t;

    // Defeat MWCC CSE: force a second independent load of blendFactors[2].
    float t2 = ((const volatile float*)blendFactors)[2];

    mType = lhs.mType;
    mScale = (one - t2) * lhs.mScale + t2 * rhs.mScale;
    mRadius = lhs.mRadius;

    mOrientation = lhs.mOrientation + (s16)((s16)(rhs.mOrientation - lhs.mOrientation) * t);

    mPosition.f.x = invT * lhs.mPosition.f.x + t * rhs.mPosition.f.x;
    mPosition.f.y = invT * lhs.mPosition.f.y + t * rhs.mPosition.f.y;
    mPosition.f.z = invT * lhs.mPosition.f.z + t * rhs.mPosition.f.z;
}

/**
 * Offset/Address/Size: 0xF8 | 0x8011ED6C | size: 0x27C
 */
void DrawablePowerup::Render(int idx) const
{
    World* world = WorldManager::s_World;
    DrawableObject* obj = world->FindDrawableObject(nlStringLowerHash(PowerupLookupString(idx)));

    if (obj == NULL)
    {
        return;
    }

    nlQuaternion quat;
    float angle = 0.0000958738f * (float)mOrientation;
    const nlVector3 axis = { 0.0f, 0.0f, 1.0f };
    nlMakeQuat(quat, axis, angle);

    if (mVisible)
    {
        obj->m_uObjectFlags |= 1;
    }
    else
    {
        obj->m_uObjectFlags &= ~1;
    }

    obj->m_orientation = quat;
    obj->m_worldMatrixUpToDate = false;
    obj->m_translation = mPosition;
    obj->m_worldMatrixUpToDate = false;
    obj->m_scale = mScale;
    obj->m_worldMatrixUpToDate = false;

    if (!mVisible)
    {
        return;
    }

    obj->Draw();
    DrawableModel* model = obj->AsDrawableModel();

    if (model == NULL)
    {
        DrawShadow(mRadius, mPosition.f.x, mPosition.f.y, mPosition.f.z);
    }
    else
    {
        model->DrawPlanarShadow();
    }
}

/**
 * Offset/Address/Size: 0x374 | 0x8011EFE8 | size: 0x190
 */
void DrawablePowerup::Grab(int idx)
{
    PowerupBase* powerup = FindPowerUp(nlStringLowerHash(PowerupLookupString(idx)));
    if (powerup != NULL)
    {
        mType = powerup->m_eType;
        mVisible = true;
        mRadius = powerup->GetRadius();
        mOrientation = powerup->m_aOrientation;
        mPosition = powerup->m_v3Position;
        mScale = powerup->m_scale;
    }
    else
    {
        mVisible = false;
    }
}

/**
 * Offset/Address/Size: 0x504 | 0x8011F178 | size: 0x1A0
 * TODO: 99.47% match - MWCC keeps the clamped height ratio/interpolation pair
 * in f7/f8 swapped versus target in the pre-vertex math block.
 */
void DrawShadow(float radius, float x, float y, float z)
{
    float one = 1.0f;
    float zero = 0.0f;
    float t = z / 10.0f;

    if (t < zero)
    {
        t = zero;
    }

    if (t > one)
    {
        t = one;
    }

    float invT = one - t;
    float alphaT = 48.0f * t;
    float scale = 4.0f * radius;
    float alphaF = 150.0f * invT + alphaT;
    float shadowT = t * scale;
    float shadowRadius = (float)(invT * (1.75 * radius) + shadowT);
    int alpha = (int)alphaF;

    if (alpha < 0)
    {
        alpha = 0;
    }

    if (alpha > 0xFF)
    {
        alpha = 0xFF;
    }

    glQuad3 quad;
    quad.m_pos[0].f.x = x - shadowRadius;
    quad.m_pos[0].f.y = y - shadowRadius;
    quad.m_pos[0].f.z = 0.015625f;
    quad.m_pos[1].f.x = x - shadowRadius;
    quad.m_pos[1].f.y = y + shadowRadius;
    quad.m_pos[1].f.z = 0.015625f;
    quad.m_pos[2].f.x = x + shadowRadius;
    quad.m_pos[2].f.y = y + shadowRadius;
    quad.m_pos[2].f.z = 0.015625f;
    quad.m_pos[3].f.x = x + shadowRadius;
    quad.m_pos[3].f.y = y - shadowRadius;
    quad.m_pos[3].f.z = 0.015625f;

    quad.m_uv[0].f.x = 1.0f;
    quad.m_uv[0].f.y = 1.0f;
    quad.m_uv[1].f.x = 0.0f;
    quad.m_uv[1].f.y = 1.0f;
    quad.m_uv[2].f.x = 0.0f;
    quad.m_uv[2].f.y = 0.0f;
    quad.m_uv[3].f.x = 1.0f;
    quad.m_uv[3].f.y = 0.0f;

    unsigned char colour[4];
    colour[0] = 0xFF;
    colour[1] = 0xFF;
    colour[2] = 0xFF;
    colour[3] = (unsigned char)alpha;

    unsigned long packed = *(unsigned long*)colour;
    *(unsigned long*)(&quad.m_colour[3]) = packed;
    *(unsigned long*)(&quad.m_colour[2]) = packed;
    *(unsigned long*)(&quad.m_colour[1]) = packed;
    *(unsigned long*)(&quad.m_colour[0]) = packed;

    glSetDefaultState(true);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_DepthWrite, 0);
    glSetCurrentRasterState(glHandleizeRasterState());

    unsigned long texture = glGetTexture("global/ball_shadow");
    glSetCurrentTexture(texture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 3);
    glSetCurrentTextureState(glHandleizeTextureState());

    quad.Attach(GLV_Unshadowed, 0, true);
}

/**
 * Offset/Address/Size: 0x0 | 0x8011F318 | size: 0x88
 */
#pragma inline_depth(0)
template <>
void DrawablePowerup::Replay<LoadFrame>(LoadFrame& frame)
{
    Replayable<3, LoadFrame, bool>(frame, mVisible);
    if (mVisible)
    {
        Replayable<3, LoadFrame, char>(frame, (char&)mType);
        Replayable<3, LoadFrame, unsigned short>(frame, mOrientation);
        Replayable<3, LoadFrame, nlVector3>(frame, mPosition);
        Replayable<3, LoadFrame, float>(frame, mScale);
        Replayable<3, LoadFrame, float>(frame, mRadius);
    }
}
#pragma inline_depth

/**
 * Offset/Address/Size: 0x88 | 0x8011F3A0 | size: 0x88
 */
template <>
void DrawablePowerup::Replay<SaveFrame>(SaveFrame& frame)
{
    Replayable<3, SaveFrame, bool>(frame, mVisible);
    if (mVisible)
    {
        Replayable<3, SaveFrame, char>(frame, (char&)mType);
        Replayable<3, SaveFrame, unsigned short>(frame, mOrientation);
        Replayable<3, SaveFrame, nlVector3>(frame, mPosition);
        Replayable<3, SaveFrame, float>(frame, mScale);
        Replayable<3, SaveFrame, float>(frame, mRadius);
    }
}

// /**
//  * Offset/Address/Size: 0x0 | 0x8011F428 | size: 0x8
//  */
// DrawableModel* DrawableObject::AsDrawableModel()
// {
//     return NULL;
// }

/**
 * Offset/Address/Size: 0x0 | 0x8011F430 | size: 0x64
 */
// Replayable<3, SaveFrame, bool> defined in ReplaySpecializations.h

/**
 * Offset/Address/Size: 0x64 | 0x8011F494 | size: 0x50
 */
// void Replayable<3, SaveFrame, char>(SaveFrame&, char&)
// {
// }

/**
 * Offset/Address/Size: 0xB4 | 0x8011F4E4 | size: 0x50
 */
// void Replayable<3, SaveFrame, unsigned short>(SaveFrame&, unsigned short&)
// {
// }

/**
 * Offset/Address/Size: 0x104 | 0x8011F534 | size: 0x50
 */
// void Replayable<3, SaveFrame, nlVector3>(SaveFrame&, nlVector3&)
// {
// }

/**
 * Offset/Address/Size: 0x154 | 0x8011F584 | size: 0x50
 */
// Replayable<3, SaveFrame, float> defined in ReplaySpecializations.h

/**
 * Offset/Address/Size: 0x1A4 | 0x8011F5D4 | size: 0x7C
 */
// void Replayable<3, LoadFrame, bool>(LoadFrame&, bool&)
// {
// }

/**
 * Offset/Address/Size: 0x220 | 0x8011F650 | size: 0x54
 */
// void Replayable<3, LoadFrame, char>(LoadFrame&, char&)
// {
// }

/**
 * Offset/Address/Size: 0x274 | 0x8011F6A4 | size: 0x54
 */
// void Replayable<3, LoadFrame, unsigned short>(LoadFrame&, unsigned short&)
// {
// }

/**
 * Offset/Address/Size: 0x2C8 | 0x8011F6F8 | size: 0x54
 */
template <>
void Replayable<3, LoadFrame, nlVector3>(LoadFrame& frame, nlVector3& value)
{
    FORCE_DONT_INLINE;
    if (frame.mInterval == 3)
    {
        if (frame.mInterval == 3)
        {
            memcpy(&value, frame.mStream.mStorage, sizeof(nlVector3));
            frame.mStream.mStorage += sizeof(nlVector3);
        }
    }
}

/**
 * Offset/Address/Size: 0x31C | 0x8011F74C | size: 0x54
 */
// Replayable<3, LoadFrame, float> defined in ReplaySpecializations.h
