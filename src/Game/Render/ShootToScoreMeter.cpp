#include "Game/Render/ShootToScoreMeter.h"
#include "Game/Camera/CameraMan.h"
#include "Game/Game.h"
#include "NL/nlString.h"
#include "NL/gl/glDraw3.h"
#include "NL/gl/glState.h"
#include "NL/platvmath.h"

static u32 UnlitProgram = glGetProgram("3d unlit");
static u32 LitProgram = glGetProgram("3d pointlit");
static u32 LightTexture = glGetTexture("global/lightramp");
static u32 BlackTexture = glGetTexture("global/black");
static u32 WhiteTexture = glGetTexture("global/white");
static u32 MeterTexture = glGetTexture("fe/shoot_to_score_track_black");
static u32 CaptainMeterTexture = glGetTexture("fe/capt_shoot_to_score_track");
static u32 CaptainMeterShadowTexture = glGetTexture("fe/capt_shoot_to_score_track_shadow");

u32 NumberTextures[4] = {
    nlStringLowerHash("fe/controller_1_indicator"),
    nlStringLowerHash("fe/controller_2_indicator"),
    nlStringLowerHash("fe/controller_3_indicator"),
    nlStringLowerHash("fe/controller_4_indicator"),
};

ShootToScoreMeter ShootToScoreMeter::instance;
// static u32 WHITE_COLOR = 0xFFFFFFFF;

static nlColour sWhiteBarColour = { 255, 255, 255, 255 };
static nlColour sHyperColour = { 181, 240, 255, 255 };
static nlColour sGreenRegionColour = { 5, 150, 5, 255 };
static nlColour sYellowRegionColour = { 200, 200, 10, 255 };
static s32 sfNumBarsInTrail = 20;
static float sfTrailIntensity = 0.6f;
static float sfTrailLengthScale = 0.25f;
static u8 sbMakeSTSMeterOrthographic = 1;

static inline void InterpolateColours(const nlColour& colour0, const nlColour& colour1, float alpha, nlColour& result)
{
    float oneMinusAlpha = 1.0f - alpha;
    result.c[0] = (u8)(s32)(oneMinusAlpha * (float)colour0.c[0] + alpha * (float)colour1.c[0]);
    result.c[1] = (u8)(s32)(oneMinusAlpha * (float)colour0.c[1] + alpha * (float)colour1.c[1]);
    result.c[2] = (u8)(s32)(oneMinusAlpha * (float)colour0.c[2] + alpha * (float)colour1.c[2]);
    result.c[3] = (u8)(s32)(oneMinusAlpha * (float)colour0.c[3] + alpha * (float)colour1.c[3]);
}

static inline void NDCToScreenCoordinates(const nlVector3& v3NormalizedScreenPos, nlVector3& v3ScreenPos)
{
    v3ScreenPos.f.x = 320.0f * v3NormalizedScreenPos.f.x;
    v3ScreenPos.f.y = 240.0f * v3NormalizedScreenPos.f.y;
    v3ScreenPos.f.x += 320.0f;
    v3ScreenPos.f.y += 240.0f;
    v3ScreenPos.f.z = -0.1f;
}

static inline float clamp_ge(float x, float limit)
{
    if (x >= limit)
        return x;
    else
        return limit;
}

static inline float clamp_le(float x, float limit)
{
    if (x <= limit)
        return x;
    else
        return limit;
}

inline void ShootToScoreMeter::DrawIndicatorBar(float angle, const nlColour& colour, const nlMatrix4& meterMatrix, float scale)
{
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());
    cCameraManager::GetDistanceFromCameraToObject(m_v3MeterPosition);

    glQuad3 barQuad;
    float zDepth;
    float angleRadians;
    float scaledMeterWidth = MeterWidth * scale;
    float scaledWhiteBarWidth = 0.039f * scale;
    float scaledWhiteBarHeight = 0.0035f * scale;
    angleRadians = (3.1415927f * angle) / 180.0f;

    nlMatrix4 barMatrix;
    nlMakeRotationMatrixZ(barMatrix, angleRadians);

    float radius = 0.198f * scaledMeterWidth;
    if (sbMakeSTSMeterOrthographic)
    {
        zDepth = -100.0f;
    }
    else
    {
        zDepth = 0.0f;
    }

    s32 angle_s32 = (s32)(10430.378f * angleRadians);
    float sinVal = nlSin((u16)angle_s32);
    float cosVal = nlSin((u16)((u16)angle_s32 + 0x4000));

    barMatrix.m[3][0] = radius * cosVal;
    barMatrix.m[3][1] = radius * sinVal;
    barMatrix.m[3][2] = zDepth;
    barMatrix.m[3][3] = 1.0f;
    nlMultMatrices(barMatrix, barMatrix, meterMatrix);

    barQuad.SetupRotatedRectangle(scaledWhiteBarWidth, scaledWhiteBarHeight, barMatrix, false, false);
    barQuad.SetColour(colour);

    eGLView view = GLV_UnsortedPerspective;
    if (sbMakeSTSMeterOrthographic)
    {
        view = GLV_UnsortedOrtho;
    }

    glAttachQuad3(view, 1, &barQuad, true);
}

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
    glSetRasterState((eGLState)6, 0);
    glSetDefaultState(true);
    glSetRasterState((eGLState)1, 0);
    glSetRasterState((eGLState)0, 0);
    glSetRasterState((eGLState)5, 1);
    glSetRasterState((eGLState)6, 0);
    glSetCurrentRasterState(glHandleizeRasterState());

    if (m_MeterType == REGULAR_SHOOT_TO_SCORE_PHASE1)
    {
        glSetCurrentTexture(MeterTexture, GLTT_Diffuse);
    }
    else
    {
        glSetCurrentTexture(CaptainMeterTexture, GLTT_Diffuse);
    }

    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    nlMatrix4 matrix;
    float scale;
    float scaledMeterWidth;
    glQuad3 quad;

    matrix.SetIdentity();
    nlMakeRotationMatrixZ(matrix, 1.5707964f);

    if (sbMakeSTSMeterOrthographic)
    {
        static nlVector3 screenPosition;

        matrix.SetIdentity();
        glViewProjectPoint(GLV_Unshadowed, m_v3MeterPosition, screenPosition);

        NDCToScreenCoordinates(screenPosition, screenPosition);
        screenPosition.f.y += -20.0f;

        scale = 640.0f;
        scaledMeterWidth = MeterWidth * scale;

        screenPosition.f.x = clamp_le(clamp_ge(screenPosition.f.x, 92.0f), 548.0f);
        screenPosition.f.y = clamp_le(clamp_ge(screenPosition.f.y, 84.0f), 396.0f);

        matrix.m[3][0] = screenPosition.f.x;
        matrix.m[3][1] = screenPosition.f.y;
        matrix.m[3][2] = screenPosition.f.z;
        matrix.m[3][3] = 1.0f;
    }
    else
    {
        scale = cCameraManager::GetDistanceFromCameraToObject(m_v3MeterPosition);

        matrix.m[3][0] = m_v3MeterPosition.f.x;
        matrix.m[3][1] = m_v3MeterPosition.f.y;
        matrix.m[3][2] = m_v3MeterPosition.f.z;
        matrix.m[3][3] = 1.0f;

        scaledMeterWidth = MeterWidth * scale;
    }

    quad.SetupRotatedRectangle(
        scaledMeterWidth,
        scaledMeterWidth,
        matrix,
        m_MeterType == REGULAR_SHOOT_TO_SCORE_PHASE1,
        false);

    eGLView view = GLV_UnsortedPerspective;
    if (sbMakeSTSMeterOrthographic)
    {
        view = GLV_UnsortedOrtho;
    }
    glAttachQuad3(view, 1, &quad, true);

    nlColour green = sGreenRegionColour;
    green.c[3] = (u8)(s32)(255.0f * m_fGreenAndYellonRegionIntensity);
    nlColour yellow = sYellowRegionColour;
    yellow.c[3] = (u8)(s32)(255.0f * m_fGreenAndYellonRegionIntensity);
    nlColour black = { 0, 0, 0, 255 };
    nlColour Hyper = sHyperColour;

    if (meHyper == STS_GOT_HYPER)
    {
        DrawIndicatorBar(m_fWhiteBarAngle, black, matrix, scale);
        DrawIndicatorBar(m_fSavedWhiteBarAngle, black, matrix, scale);
    }
    else
    {
        DrawColouredRegion(
            m_fGreenBarAngle - (0.5 * m_fYellowRegionWidth),
            m_fGreenBarAngle - (0.5f * m_fGreenRegionWidth),
            yellow,
            yellow,
            matrix,
            scale);
        DrawColouredRegion(
            m_fGreenBarAngle + (0.5 * m_fYellowRegionWidth),
            m_fGreenBarAngle + (0.5f * m_fGreenRegionWidth),
            yellow,
            yellow,
            matrix,
            scale);
        DrawColouredRegion(
            m_fGreenBarAngle - (0.5f * m_fGreenRegionWidth),
            m_fGreenBarAngle + (0.5f * m_fGreenRegionWidth),
            green,
            green,
            matrix,
            scale);

        DrawIndicatorBar(m_fWhiteBarAngle, sWhiteBarColour, matrix, scale);

        if (mbShowSavedWhiteBar)
        {
            DrawColouredRegion(
                m_fSavedGreenBarAngle - (0.5 * m_fSavedYellowRegionWidth),
                m_fSavedGreenBarAngle - (0.5f * m_fSavedGreenRegionWidth),
                yellow,
                yellow,
                matrix,
                scale);
            DrawColouredRegion(
                m_fSavedGreenBarAngle + (0.5 * m_fSavedYellowRegionWidth),
                m_fSavedGreenBarAngle + (0.5f * m_fSavedGreenRegionWidth),
                yellow,
                yellow,
                matrix,
                scale);

            if (meHyper == STS_POSSIBLE_HYPER)
            {
                DrawColouredRegion(
                    m_fSavedGreenBarAngle - (0.5f * m_fSavedGreenRegionWidth),
                    m_fSavedGreenBarAngle + (0.5f * m_fSavedGreenRegionWidth),
                    Hyper,
                    Hyper,
                    matrix,
                    scale);
                DrawIndicatorBar(m_fSavedWhiteBarAngle, black, matrix, scale);
            }
            else
            {
                DrawColouredRegion(
                    m_fSavedGreenBarAngle - (0.5f * m_fSavedGreenRegionWidth),
                    m_fSavedGreenBarAngle + (0.5f * m_fSavedGreenRegionWidth),
                    green,
                    green,
                    matrix,
                    scale);
                DrawIndicatorBar(m_fSavedWhiteBarAngle, sWhiteBarColour, matrix, scale);
            }
        }

        float diffCurrentPrev = m_fWhiteBarPreviousAngle - m_fWhiteBarAngle;

        for (int i = 0; i < sfNumBarsInTrail; i++)
        {
            nlColour fadedColour = sWhiteBarColour;
            fadedColour.c[3] = (u8)(s32)(255.0f * (sfTrailIntensity * (1.0f - ((float)i / (float)sfNumBarsInTrail))));

            DrawIndicatorBar(sfTrailLengthScale * ((float)i * diffCurrentPrev) + m_fWhiteBarAngle, fadedColour, matrix, scale);
        }
    }

    glSetDefaultState(false);
}

/**
 * Offset/Address/Size: 0x3DC | 0x8016063C | size: 0x5A8
 */
void ShootToScoreMeter::DrawColouredRegion(float startAngle, float endAngle, const nlColour& startColour, const nlColour& endColour, nlMatrix4 meterMatrix, float scale)
{
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetTextureState(GLTS_DiffuseWrap, 0);
    glSetCurrentTextureState(glHandleizeTextureState());

    float scaledWhiteBarWidth;
    float radius;
    nlVector3 v;
    float widthAngle;
    int i;
    glQuad3 barQuad;
    float frac0;
    float frac1;
    float radius0;
    float radius1;

    scaledWhiteBarWidth = 0.039f * scale;
    widthAngle = endAngle - startAngle;
    radius = 0.198f * (MeterWidth * scale);

    float step = 0.125f;
    float sinScale = 10430.378f;
    float pi = 3.1415927f;
    float deg = 180.0f;

    for (i = 0; i < 8; i++)
    {
        frac0 = (float)i * step;
        frac1 = (float)(i + 1) * step;
        radius0 = radius - scaledWhiteBarWidth / 2.0f;
        radius1 = radius + scaledWhiteBarWidth / 2.0f;

        float angle0 = frac0 * widthAngle + startAngle;
        s32 angle0_i = (s32)(sinScale * (pi * angle0 / deg));
        float cos0 = nlSin((u16)((u16)angle0_i + 0x4000));
        float sin0 = nlSin((u16)angle0_i);

        float angle1 = frac1 * widthAngle + startAngle;
        s32 angle1_i = (s32)(sinScale * (pi * angle1 / deg));
        float cos1 = nlSin((u16)((u16)angle1_i + 0x4000));
        float sin1 = nlSin((u16)angle1_i);

        float zDepth;
        if (sbMakeSTSMeterOrthographic)
        {
            zDepth = -100.0f;
        }
        else
        {
            zDepth = 0.0f;
        }

        v.f.x = radius0 * cos0;
        v.f.y = radius0 * sin0;
        v.f.z = zDepth;
        nlMultPosVectorMatrix(v, v, meterMatrix);
        barQuad.m_pos[0] = v;

        v.f.x = radius1 * cos0;
        v.f.y = radius1 * sin0;
        v.f.z = zDepth;
        nlMultPosVectorMatrix(v, v, meterMatrix);
        barQuad.m_pos[1] = v;

        v.f.x = radius1 * cos1;
        v.f.y = radius1 * sin1;
        v.f.z = zDepth;
        nlMultPosVectorMatrix(v, v, meterMatrix);
        barQuad.m_pos[2] = v;

        v.f.x = radius0 * cos1;
        v.f.y = radius0 * sin1;
        v.f.z = zDepth;
        nlMultPosVectorMatrix(v, v, meterMatrix);
        barQuad.m_pos[3] = v;

        InterpolateColours(startColour, endColour, frac0, barQuad.m_colour[0]);
        barQuad.m_colour[1] = barQuad.m_colour[0];

        InterpolateColours(startColour, endColour, frac1, barQuad.m_colour[2]);
        barQuad.m_colour[3] = barQuad.m_colour[2];

        eGLView view = GLV_UnsortedPerspective;
        if (sbMakeSTSMeterOrthographic)
        {
            view = GLV_UnsortedOrtho;
        }

        glAttachQuad3(view, 1, &barQuad, true);
    }
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
