#include "Game/Render/ShootToScoreMeter.h"
#include "Game/Game.h"
#include "NL/nlString.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"

u32 UnlitProgram = glGetProgram("3d unlit");
u32 LitProgram = glGetProgram("3d pointlit");
u32 LightTexture = glGetTexture("global/lightramp");
u32 BlackTexture = glGetTexture("global/black");
u32 WhiteTexture = glGetTexture("global/white");
u32 MeterTexture = glGetTexture("fe/shoot_to_score_track_black");
u32 CaptainMeterTexture = glGetTexture("fe/capt_shoot_to_score_track");
u32 CaptainMeterShadowTexture = glGetTexture("fe/capt_shoot_to_score_track_shadow");

u32 NumberTextures[4] = {
    nlStringLowerHash("fe/controller_1_indicator"),
    nlStringLowerHash("fe/controller_2_indicator"),
    nlStringLowerHash("fe/controller_3_indicator"),
    nlStringLowerHash("fe/controller_4_indicator"),
};

ShootToScoreMeter ShootToScoreMeter::instance;
// static u32 WHITE_COLOR = 0xFFFFFFFF;

float ShootToScoreMeter::MeterWidth = 0.4f;

/**
 * Offset/Address/Size: 0x19C8 | 0x80161C28 | size: 0xB0
 */
void ShootToScoreMeter::TurnOnMeter(ShootToScoreMeter::STSMeterType type, float yellowWidth)
{
    m_bMeterVisible = true;
    m_MeterType = type;
    m_fWhiteBarAngle = 0.0f;

    float scaledYellowWidth = 180.0f * (2.0f * yellowWidth);

    m_fWhiteBarPreviousAngle = m_fWhiteBarAngle;
    m_fSavedWhiteBarAngle = m_fWhiteBarAngle;
    mfRumbleAmount = 0.0f;
    m_fGreenBarAngle = 180.0f * g_pGame->m_pGameTweaks->unk294;
    m_fSavedGreenBarAngle = 180.0f * g_pGame->m_pGameTweaks->unk294;
    m_fGreenRegionWidth = 180.0f * (2.0f * g_pGame->m_pGameTweaks->unk29C);
    m_fSavedGreenRegionWidth = 180.0f * (2.0f * g_pGame->m_pGameTweaks->unk29C);
    m_fYellowRegionWidth = scaledYellowWidth;
    m_fSavedYellowRegionWidth = scaledYellowWidth;
    m_fWhiteBarPreviousAngle = m_fWhiteBarAngle;
    mbShowSavedWhiteBar = 0;
    meHyper = STS_NO_HYPER;
}

/**
 * Offset/Address/Size: 0x1990 | 0x80161BF0 | size: 0x38
 */
void ShootToScoreMeter::RumbleMeter(float rumbleIntensity, float xOffset, float zOffset)
{
    mfRumbleAmount = 0.175f * rumbleIntensity;
    m_v3MeterPosition.f.x = xOffset * mfRumbleAmount + m_v3OriginalMeterPosition.f.x;
    m_v3MeterPosition.f.y = m_v3OriginalMeterPosition.f.y;
    m_v3MeterPosition.f.z = zOffset * mfRumbleAmount + m_v3OriginalMeterPosition.f.z;
}

/**
 * Offset/Address/Size: 0x984 | 0x80160BE4 | size: 0x100C
 */
void ShootToScoreMeter::DrawMeter()
{
    FORCE_DONT_INLINE;
}

/**
 * Offset/Address/Size: 0x3DC | 0x8016063C | size: 0x5A8
 */
void ShootToScoreMeter::DrawColouredRegion(float, float, const nlColour&, const nlColour&, nlMatrix4, float)
{
}

/**
 * Offset/Address/Size: 0x394 | 0x801605F4 | size: 0x48
 */
void ShootToScoreMeter::UpdateAndRender(float)
{
    if (m_bMeterVisible == 0)
    {
        return;
    }

    if (m_MeterType == REGULAR_SHOOT_TO_SCORE_PHASE1 || m_MeterType == REGULAR_SHOOT_TO_SCORE_PHASE2)
    {
        DrawMeter();
    }
    else
    {
        DrawCaptainMeter();
    }
}

/**
 * Offset/Address/Size: 0x37C | 0x801605DC | size: 0x18
 */
void ShootToScoreMeter::SetWhiteBarPosition(float position)
{
    m_fWhiteBarPreviousAngle = m_fWhiteBarAngle;
    m_fWhiteBarAngle = 180.0f * position;
}

/**
 * Offset/Address/Size: 0x36C | 0x801605CC | size: 0x10
 */
void ShootToScoreMeter::SetSavedWhiteBarPosition(float position)
{
    m_fSavedWhiteBarAngle = 180.0f * position;
}

/**
 * Offset/Address/Size: 0x35C | 0x801605BC | size: 0x10
 */
void ShootToScoreMeter::SetGreenBarPosition(float position)
{
    m_fGreenBarAngle = 180.0f * position;
}

/**
 * Offset/Address/Size: 0x34C | 0x801605AC | size: 0x10
 */
void ShootToScoreMeter::SetGreenRegionWidth(float width)
{
    m_fGreenRegionWidth = 180.0f * width;
}

/**
 * Offset/Address/Size: 0x0 | 0x80160260 | size: 0x34C
 * TODO: f30/f31 register swap for -500.0f and 1.0f constants
 */
void ShootToScoreMeter::DrawCaptainMeter()
{
    glSetDefaultState(false);
    glSetRasterState(GLS_Culling, 0);
    glSetRasterState(GLS_AlphaBlend, 1);
    glSetRasterState(GLS_AlphaTest, 1);
    glSetRasterState(GLS_AlphaTestRef, 0);
    glSetCurrentRasterState(glHandleizeRasterState());

    glQuad3 quad;
    nlMatrix4 matrix;

    matrix.SetIdentity();
    nlMakeRotationMatrixX(matrix, 1.2217306f);

    for (int i = 0; i < 12; i++)
    {
        glSetCurrentTexture(CaptainMeterShadowTexture, GLTT_Diffuse);
        glSetTextureState(GLTS_DiffuseWrap, 0);
        glSetCurrentTextureState(glHandleizeTextureState());

        float scale = (float)(i + 1) / 12.0f;
        float yOffset = 30.0f * scale;
        float yPos = 365.0f + (yOffset);
        matrix.m[3][0] = 320.0f;
        matrix.m[3][1] = yPos;
        matrix.m[3][2] = -500.0f;
        matrix.m[3][3] = 1.0f;

        nlColour c = { 255, 255, 255, 255 };
        quad.SetColour(c);
        quad.SetupRotatedRectangle(400.0f, 400.0f, matrix, false, false);
        glAttachQuad3(GLV_FrontEnd, 1, &quad, true);
    }

    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    matrix.m[3][0] = 320.0f;
    matrix.m[3][1] = 365.0f;
    matrix.m[3][2] = -500.0f;
    matrix.m[3][3] = 1.0f;

    nlColour c = { 255, 255, 255, 255 };
    quad.SetColour(c);
    quad.SetupRotatedRectangle(400.0f, 400.0f, matrix, false, false);
    glAttachQuad3(GLV_FrontEnd, 1, &quad, true);

    float whiteBarAngle;
    float whiteBarHeightScaled = 400.0f * (0.039f / MeterWidth);
    float whiteBarWidthScaled = 400.0f * (0.0035f / MeterWidth);
    whiteBarAngle = (3.1415927f * m_fWhiteBarAngle) / 180.0f;

    nlMatrix4 matrix2;
    nlMakeRotationMatrixZ(matrix2, whiteBarAngle);

    float sinVal = nlSin((u16)(s32)(10430.378f * whiteBarAngle));
    float cosVal = nlSin((u16)((u16)(s32)(10430.378f * whiteBarAngle) + 0x4000));

    matrix2.SetRow4_(3, 79.2f * cosVal, 79.2f * sinVal, 0.f, 1.0f);
    nlMultMatrices(matrix2, matrix2, matrix);

    glSetRasterState(GLS_AlphaBlend, 0);
    glSetCurrentRasterState(glHandleizeRasterState());
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    glQuad3 whiteBarQuad;
    whiteBarQuad.SetupRotatedRectangle(whiteBarHeightScaled, whiteBarWidthScaled, matrix2, false, false);
    glAttachQuad3(GLV_FrontEnd, 1, &whiteBarQuad, true);
    glSetDefaultState(false);
}
