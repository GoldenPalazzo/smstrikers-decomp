#include "Game/Debug/FrameCounter.h"
#include "Game/Debug/TimeRegions.h"
#include "Game/GL/gluMeshWriter.h"

#include "NL/gl/glMatrix.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/nlDebugFile.h"
#include "NL/nlMemory.h"
#include "NL/nlPrint.h"
#include "NL/nlTicker.h"

static u32 UnlitProgram = glGetProgram("3d unlit");
static u32 LitProgram = glGetProgram("3d pointlit");
static u32 LightTexture = glGetTexture("global/lightramp");
static u32 BlackTexture = glGetTexture("global/black");
static u32 WhiteTexture = glGetTexture("global/white");

int FrameCounter::NUM_FRAMES_TO_AVERAGE_OVER = 0x1E;

static float sfHappiness = 1.0f;
static float sfEyeHeight = 0.2f;
static float sfEyeSeparation = 0.4f;
static float sfSmileyRadius = 15.0f;
static float sfSmileRadius = 0.6f;
static float sfEyeRadius = 0.15f;
static float sfSmileAngle = 90.0f;
static int siHappinessLookback = 60;
static nlColour sMadColour = { 140, 48, 0, 255 };
static nlColour sMediumColour = { 192, 192, 0, 255 };
static nlColour sHappyColour = { 0, 192, 0, 255 };

nlListContainer<TimeRegion*> TimeRegion::sTimeRegionList;

/**
 * Offset/Address/Size: 0x1374 | 0x801FDF10 | size: 0x7C
 */
FrameCounter::FrameCounter(const char* firstName, const char* secondName)
{
    m_FirstName = firstName;
    m_SecondName = secondName;

    unk30 = 0;
    unk28 = 0;
    unk34 = 0;
    m_ContinuousFrameHistoryIndex = 0;

    m_CurrTimer[0] = 0.0f;
    m_CurrTimer[1] = 0.0f;

    m_CurrTimerNum = -1;

    memset(m_FrameHistory, 0, 0xA00);
    memset(m_ContinuousFrameHistory, 0, 0x640);
}

/**
 * Offset/Address/Size: 0x12F8 | 0x801FDE94 | size: 0x7C
 */
void FrameCounter::StartTimer(int timerNum)
{
    u32 currentTick = nlGetTicker();

    if (m_CurrTimerNum != -1)
    {
        m_CurrTimer[m_CurrTimerNum] += nlGetTickerDifference(m_StartTick, currentTick);
    }

    m_StartTick = currentTick;
    m_CurrTimerNum = timerNum;
}

/**
 * Offset/Address/Size: 0x1078 | 0x801FDC14 | size: 0x280
 */
void FrameCounter::FinishTiming()
{
    u32 currentTick = nlGetTicker();
    if (m_CurrTimerNum != -1)
    {
        m_CurrTimer[m_CurrTimerNum] += nlGetTickerDifference(m_StartTick, currentTick);
    }

    unk30++;

    float totalFrameTime = 0.0f;

    totalFrameTime += m_CurrTimer[0];
    m_CurrFrame[0] += m_CurrTimer[0];
    m_ContinuousFrameHistory[0][m_ContinuousFrameHistoryIndex] = m_CurrTimer[0];

    totalFrameTime += m_CurrTimer[1];
    m_CurrFrame[1] += m_CurrTimer[1];
    m_ContinuousFrameHistory[1][m_ContinuousFrameHistoryIndex] = m_CurrTimer[1];

    if (unk30 >= (u32)NUM_FRAMES_TO_AVERAGE_OVER)
    {
        m_LastFrame[0] = m_CurrFrame[0] / (float)unk30;
        m_CurrFrame[0] = 0.0f;
        m_LastFrame[1] = m_CurrFrame[1] / (float)unk30;
        m_CurrFrame[1] = 0.0f;

        unk2C = (float)unk28 / (float)unk30;
        unk28 = 0;
        unk30 = 0;
    }

    m_FrameHistory[unk34] = totalFrameTime;
    if (totalFrameTime > 17.0f)
    {
        unk28++;
    }

    unk34 = ((u32)unk34 + 1) % 640;
    m_ContinuousFrameHistoryIndex = (m_ContinuousFrameHistoryIndex + 1) % 200;

    nlListIterator<TimeRegion*> iterator = TimeRegion::sTimeRegionList.Begin();
    while (iterator.IsValid())
    {
        TimeRegion* region = iterator.Current();
        if (region->m_pConditionFunc())
        {
            region->m_unk14++;
            if (totalFrameTime > 17.0f)
            {
                region->m_unk10++;
            }
            region->m_fThreshold += totalFrameTime;
        }

        iterator.Next();
    }

    m_CurrTimer[0] = 0.0f;
    m_CurrTimer[1] = 0.0f;
    m_CurrTimerNum = -1;
}

/**
 * Offset/Address/Size: 0xED8 | 0x801FDA74 | size: 0x1A0
 */
void FrameCounter::WriteFrameRateStatsToFile(const char* fileName)
{
    char buf[128];
    void* file = nlOpenFileDebug("FrameRateStats.txt", false, true);
    nlWriteLineDebug(file, fileName, false);
    nlWriteLineDebug(file, "============================================\n", false);
    nlWriteLineDebug(file, "First number is percentage of frames that take less than 17ms\n", false);
    nlWriteLineDebug(file, "Second number is average time per frame\n", false);
    nlWriteLineDebug(file, "Third number of frames counted\n\n", false);

    nlListIterator<TimeRegion*> iterator = TimeRegion::sTimeRegionList.Begin();
    while (iterator.IsValid())
    {
        TimeRegion* region = iterator.Current();
        float ratio = (float)region->m_unk10 / (float)region->m_unk14;
        float avgTime = region->m_fThreshold / (float)region->m_unk14;
        float pct = 100.0f * (1.0f - ratio);

        nlSNPrintf(buf, 128, "%4.0f%% %4.0f %5d %s\n", pct, avgTime, region->m_unk14, region->m_pName);
        nlWriteLineDebug(file, buf, false);

        iterator.Next();
    }

    nlCloseFileDebug(file);
}

/**
 * Offset/Address/Size: 0xC5C | 0x801FD7F8 | size: 0x27C
 */
static void DrawCircle(nlVector3 p0, float fRadius, float fScaleX, nlColour colour)
{
    GLMeshWriter mesh;

    glSetDefaultState(true);
    glSetCurrentMatrix(glGetIdentityMatrix());
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetCurrentProgram(UnlitProgram);

    const eGLStream stream_decl[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };

    if (mesh.Begin(31, GLP_TriFan, 3, stream_decl, false))
    {
        nlVector3 v3point;
        nlVector2 uv0;

        v3point.z = p0.z;
        v3point.x = p0.x;
        v3point.y = p0.y;

        float fRadians = 0.0f;

        mesh.Colour(colour);
        uv0.x = 0.0f;
        uv0.y = 0.0f;
        ((GLMeshWriterCore*)&mesh)->Texcoord(uv0);
        mesh.Vertex(v3point);

        const float angleScale = 10430.378f;
        int i = 0;
        const float uvZero = 0.0f;
        const float angleStep = 0.21666157f;
        nlVector2 uv1;

        while (i < 30)
        {
            nlSinCos(&v3point.x, &v3point.y, (unsigned short)(int)(angleScale * fRadians));
            v3point.x = p0.x + fScaleX * (v3point.x * fRadius);
            v3point.y = p0.y + v3point.y * fRadius;

            mesh.Colour(colour);
            uv1.x = uvZero;
            uv1.y = uvZero;
            ((GLMeshWriterCore*)&mesh)->Texcoord(uv1);
            mesh.Vertex(v3point);

            i++;
            fRadians += angleStep;
        }

        if (!mesh.End())
        {
            return;
        }

        glViewAttachModel(GLV_Debug, 2, mesh.GetModel());
    }
}

/**
 * Offset/Address/Size: 0x904 | 0x801FD4A0 | size: 0x358
 */
static void DrawSmile(nlVector3 p0, float fRadius, float fScaleX, nlColour colour, float fLineThickness)
{
    GLMeshWriter mesh;
    float degrees = sfSmileAngle;

    glSetDefaultState(true);
    glSetCurrentMatrix(glGetIdentityMatrix());
    glSetCurrentTexture(WhiteTexture, GLTT_Diffuse);
    glSetCurrentProgram(UnlitProgram);

    const eGLStream stream_decl[3] = { GLStream_Position, GLStream_Colour, GLStream_Diffuse };
    float yScale = (2.0f * sfHappiness) + -1.0f;

    if (mesh.Begin(20, GLP_TriStrip, 3, stream_decl, false))
    {
        nlVector3 v3point;
        nlVector2 uv0;
        nlVector2 uv1;

        float fRadians = -((3.1415927f * (0.5f * degrees)) / 180.0f);
        v3point.z = p0.z;
        nlSinCos(&v3point.x, &v3point.y, (u16)(int)(10430.378f * fRadians));

        float fXFromAngle = v3point.x * fRadius;
        v3point.x = (fScaleX * fXFromAngle) + p0.x;
        v3point.y = (v3point.y * fRadius) + p0.y;
        float fYFromAngle = v3point.y;
        float fYTop = p0.y + fRadius;
        float middleY = 0.5f * (fYFromAngle + fYTop);

        int i = 0;
        while (i < 10)
        {
            nlSinCos(&v3point.x, &v3point.y, (u16)(int)(10430.378f * fRadians));

            v3point.x = p0.x + fScaleX * (v3point.x * fRadius);
            v3point.y = p0.y + v3point.y * fRadius;

            v3point.y = v3point.y - middleY;
            v3point.y = v3point.y * yScale;
            v3point.y = v3point.y + middleY;

            mesh.Colour(colour);
            uv0.x = 0.0f;
            uv0.y = 0.0f;
            ((GLMeshWriterCore*)&mesh)->Texcoord(uv0);
            mesh.Vertex(v3point);

            v3point.y += fLineThickness;

            mesh.Colour(colour);
            uv1.x = 0.0f;
            uv1.y = 0.0f;
            ((GLMeshWriterCore*)&mesh)->Texcoord(uv1);
            mesh.Vertex(v3point);

            i++;
            fRadians += ((3.1415927f * degrees) / 180.0f) / 9.0f;
        }

        if (mesh.End() == 0)
        {
            return;
        }

        glViewAttachModel(GLV_Debug, 2, mesh.GetModel());
    }
}

/**
 * Offset/Address/Size: 0x588 | 0x801FD124 | size: 0x37C
 */
static void DrawBrow(const nlVector3& leftEyeCentre, const nlVector3& rightEyeCentre, float distanceAboveEye, float width, float height)
{
    GLMeshWriter m0;
    const eGLStream streams[2] = { GLStream_Position, GLStream_Colour };

    glSetDefaultState(false);
    glSetCurrentProgram(glGetProgram("2d unlit"));

    float yScale = (2.0f * sfHappiness) + -1.0f;

    if (m0.Begin(4, GLP_LineList, 2, streams, false))
    {
        nlVector3 p1;
        nlVector3 p2;

        float upper = -distanceAboveEye + (height * yScale);
        float lower = -distanceAboveEye - (height * yScale);

        p1 = leftEyeCentre;
        p2 = leftEyeCentre;

        p1.x = p1.x - width;
        p2.x = p2.x + width;
        p1.y = p1.y + upper;
        p2.y = p2.y + lower;

        nlColour c0;
        c0.c[0] = 0;
        c0.c[1] = 0;
        c0.c[2] = 0;
        c0.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c0);

        nlVector3 v0;
        v0.x = p1.x;
        v0.y = p1.y;
        v0.z = p1.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v0);

        nlColour c1;
        c1.c[0] = 0;
        c1.c[1] = 0;
        c1.c[2] = 0;
        c1.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c1);

        nlVector3 v1;
        v1.x = p2.x;
        v1.y = p2.y;
        v1.z = p2.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v1);

        p1 = rightEyeCentre;
        p2 = rightEyeCentre;

        p1.x = p1.x + width;
        p2.x = p2.x - width;
        p1.y = p1.y + upper;
        p2.y = p2.y + lower;

        nlColour c2;
        c2.c[0] = 0;
        c2.c[1] = 0;
        c2.c[2] = 0;
        c2.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c2);

        nlVector3 v2;
        v2.x = p1.x;
        v2.y = p1.y;
        v2.z = p1.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v2);

        nlColour c3;
        c3.c[0] = 0;
        c3.c[1] = 0;
        c3.c[2] = 0;
        c3.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c3);

        nlVector3 v3;
        v3.x = p2.x;
        v3.y = p2.y;
        v3.z = p2.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v3);

        if (m0.End())
        {
            glViewAttachModel(GLV_Debug, m0.GetModel());
        }
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801FCB9C | size: 0x588
 */
void FrameCounter::DisplayFrameSmiler()
{
    float happiness = 0.0f;
    unsigned int i;
    for (i = 0; i < (unsigned int)siHappinessLookback; i++)
    {
        float fps = m_FrameHistory[(unk34 - i + 640) % 640] == 0.0f
            ? 60.0f
            : 1000.0f / m_FrameHistory[(unk34 - i + 640) % 640];
        happiness += (fps - 30.0f) / 30.0f;
    }
    happiness /= (float)siHappinessLookback;
    if (happiness > 1.0f)
        happiness = 1.0f;
    if (happiness < 0.0f)
        happiness = 0.0f;

    sfHappiness = happiness;

    float circleRadius = sfSmileyRadius;
    float smileRadius = sfSmileRadius * sfSmileyRadius;
    float eyeRadius = sfEyeRadius * sfSmileyRadius;

    nlVector3 circleCentre = { 0, 0, 0 };
    circleCentre.x = 2.0f * sfSmileyRadius;
    circleCentre.y = 480.0f - 3.0f * sfSmileyRadius;

    nlColour black = { 0, 0, 0, 255 };

    nlColour colour;
    if (happiness < 0.5f)
    {
        float alpha = 2.0f * happiness;
        nlColourSet(colour,
            (int)((float)sMediumColour.c[0] * alpha + (float)sMadColour.c[0] * (1.0f - 2.0f * happiness)),
            (int)((float)sMediumColour.c[1] * alpha + (float)sMadColour.c[1] * (1.0f - alpha)),
            (int)((float)sMediumColour.c[2] * alpha + (float)sMadColour.c[2] * (1.0f - alpha)),
            (int)((float)sMediumColour.c[3] * alpha + (float)sMadColour.c[3] * (1.0f - alpha)));
    }
    else
    {
        float alpha = 2.0f * (happiness - 0.5f);
        nlColourSet(colour,
            (int)((float)sHappyColour.c[0] * alpha + (float)sMediumColour.c[0] * (1.0f - 2.0f * (happiness - 0.5f))),
            (int)((float)sHappyColour.c[1] * alpha + (float)sMediumColour.c[1] * (1.0f - alpha)),
            (int)((float)sHappyColour.c[2] * alpha + (float)sMediumColour.c[2] * (1.0f - alpha)),
            (int)((float)sHappyColour.c[3] * alpha + (float)sMediumColour.c[3] * (1.0f - alpha)));
    }

    nlVector3 leftEyeCentre = { 0, 0, 0 };
    nlVector3 rightEyeCentre = { 0, 0, 0 };

    leftEyeCentre.x = -circleRadius * sfEyeSeparation;
    leftEyeCentre.y = -circleRadius * sfEyeHeight;

    rightEyeCentre.x = circleRadius * sfEyeSeparation;
    rightEyeCentre.y = -circleRadius * sfEyeHeight;

    nlVec3Add(leftEyeCentre, leftEyeCentre, circleCentre);
    nlVec3Add(rightEyeCentre, rightEyeCentre, circleCentre);

    DrawCircle(circleCentre, 3.0f + circleRadius, 1.2f, black);
    DrawCircle(circleCentre, circleRadius, 1.2f, colour);
    DrawCircle(leftEyeCentre, eyeRadius, 1.2f, black);
    DrawCircle(rightEyeCentre, eyeRadius, 1.2f, black);
    DrawSmile(circleCentre, smileRadius, 1.2f, black, 3.0f);
    DrawBrow(leftEyeCentre, rightEyeCentre, 3.0f * eyeRadius, 2.0f * eyeRadius, 1.5f * eyeRadius);
}
