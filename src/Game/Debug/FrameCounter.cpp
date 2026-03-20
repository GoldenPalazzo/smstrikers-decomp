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

extern const u32 WhiteTexture;
extern const u32 UnlitProgram;

int FrameCounter::NUM_FRAMES_TO_AVERAGE_OVER = 0x1E;

// /**
//  * Offset/Address/Size: 0x0 | 0x801FE0D4 | size: 0x68
//  */
// void nlWalkList<ListEntry<TimeRegion*>, ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*> > > >(ListEntry<TimeRegion*>*, ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*> > >*, void (ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*> > >::*)(ListEntry<TimeRegion*>*))
// {
// }

// /**
//  * Offset/Address/Size: 0xBC | 0x801FE048 | size: 0x8C
//  */
// void nlListContainer<TimeRegion*>::~nlListContainer()
// {
// }

// /**
//  * Offset/Address/Size: 0x0 | 0x801FDF8C | size: 0x24
//  */
// void ListContainerBase<TimeRegion*, NewAdapter<ListEntry<TimeRegion*> > >::DeleteEntry(ListEntry<TimeRegion*>*)
// {
// }

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
    if (totalFrameTime > 0.016666668f)
    {
        unk28++;
    }

    unk34 = ((u32)unk34 + 1) % 640;
    m_ContinuousFrameHistoryIndex = (m_ContinuousFrameHistoryIndex + 1) % 200;

    ListEntry<TimeRegion*>* entry = TimeRegion::sTimeRegionList.m_Head;
    while (entry != NULL)
    {
        TimeRegion* region = entry->data;
        if (region->m_pConditionFunc())
        {
            region->m_unk14++;
            if (totalFrameTime > 0.016666668f)
            {
                region->m_unk10++;
            }
            region->m_fThreshold += totalFrameTime;
        }

        entry = entry->next;
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
    void* file = nlOpenFileDebug("framerate_stats.txt", false, true);
    nlWriteLineDebug(file, fileName, false);
    nlWriteLineDebug(file, "========================================", false);
    nlWriteLineDebug(file, "Region                          AvgTime     Percentage", false);
    nlWriteLineDebug(file, "------                          -------     ----------", false);
    nlWriteLineDebug(file, "==========================================", false);

    ListEntry<TimeRegion*>* entry = TimeRegion::sTimeRegionList.m_Head;
    while (entry != NULL)
    {
        TimeRegion* region = entry->data;
        float ratio = (float)region->m_unk10 / (float)region->m_unk14;
        float avgTime = region->m_fThreshold / (float)region->m_unk14;
        float pct = 100.0f * (1.0f - ratio);

        nlSNPrintf(buf, 128, "  %-30s  %8.2f  %8.2f%%\n", region->m_unk14, region->m_pName, pct, avgTime);
        nlWriteLineDebug(file, buf, false);

        entry = entry->next;
    }

    nlCloseFileDebug(file);
}

/**
 * Offset/Address/Size: 0xC5C | 0x801FD7F8 | size: 0x27C
 */
void DrawCircle(nlVector3 p0, float fRadius, float fScaleX, nlColour colour)
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

        v3point.f.z = p0.f.z;
        v3point.f.x = p0.f.x;
        v3point.f.y = p0.f.y;

        float fRadians = 0.0f;

        mesh.Colour(colour);
        uv0.f.x = 0.0f;
        uv0.f.y = 0.0f;
        ((GLMeshWriterCore*)&mesh)->Texcoord(uv0);
        mesh.Vertex(v3point);

        const float angleScale = 10430.378f;
        int i = 0;
        const float uvZero = 0.0f;
        const float angleStep = 0.20943952f;
        nlVector2 uv1;

        while (i < 30)
        {
            nlSinCos(&v3point.f.x, &v3point.f.y, (unsigned short)(int)(angleScale * fRadians));
            v3point.f.x = p0.f.x + fScaleX * (v3point.f.x * fRadius);
            v3point.f.y = p0.f.y + v3point.f.y * fRadius;

            mesh.Colour(colour);
            uv1.f.x = uvZero;
            uv1.f.y = uvZero;
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
 * TODO: 99.51% match - temp FPR allocation differs in setup/loop body sections.
 */
void DrawSmile(nlVector3 p0, float fRadius, float fScaleX, nlColour colour, float fLineThickness)
{
    extern float sfSmileAngle;
    extern float sfHappiness;

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
        v3point.f.z = p0.f.z;
        nlSinCos(&v3point.f.x, &v3point.f.y, (u16)(int)(10430.378f * fRadians));

        float fXFromAngle = v3point.f.x * fRadius;
        float fYFromAngle = (v3point.f.y * fRadius) + p0.f.y;
        float fYTop = p0.f.y + fRadius;

        v3point.f.y = fYFromAngle;
        v3point.f.x = (fScaleX * fXFromAngle) + p0.f.x;

        float middleY = 0.5f * (fYFromAngle + fYTop);

        int i = 0;
        while (i < 10)
        {
            nlSinCos(&v3point.f.x, &v3point.f.y, (u16)(int)(10430.378f * fRadians));

            float fXCurrent = v3point.f.x * fRadius;
            float fYCurrent = (v3point.f.y * fRadius) + p0.f.y;

            v3point.f.y = fYCurrent;
            v3point.f.x = (fScaleX * fXCurrent) + p0.f.x;

            v3point.f.y = fYCurrent - middleY;
            v3point.f.y = v3point.f.y * yScale;
            v3point.f.y = v3point.f.y + middleY;

            mesh.Colour(colour);
            uv0.f.x = 0.0f;
            uv0.f.y = 0.0f;
            ((GLMeshWriterCore*)&mesh)->Texcoord(uv0);
            mesh.Vertex(v3point);

            v3point.f.y += fLineThickness;

            mesh.Colour(colour);
            uv1.f.x = 0.0f;
            uv1.f.y = 0.0f;
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
 * TODO: 99.98% match - small literal pool/sdata21 offset differences remain.
 */
void DrawBrow(const nlVector3& leftEyeCentre, const nlVector3& rightEyeCentre, float distanceAboveEye, float width, float height)
{
    extern float sfHappiness;

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

        p1.f.x = p1.f.x - width;
        p2.f.x = p2.f.x + width;
        p1.f.y = p1.f.y + upper;
        p2.f.y = p2.f.y + lower;

        nlColour c0;
        c0.c[0] = 0;
        c0.c[1] = 0;
        c0.c[2] = 0;
        c0.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c0);

        nlVector3 v0;
        v0.f.x = p1.f.x;
        v0.f.y = p1.f.y;
        v0.f.z = p1.f.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v0);

        nlColour c1;
        c1.c[0] = 0;
        c1.c[1] = 0;
        c1.c[2] = 0;
        c1.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c1);

        nlVector3 v1;
        v1.f.x = p2.f.x;
        v1.f.y = p2.f.y;
        v1.f.z = p2.f.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v1);

        p1 = rightEyeCentre;
        p2 = rightEyeCentre;

        p1.f.x = p1.f.x + width;
        p2.f.x = p2.f.x - width;
        p1.f.y = p1.f.y + upper;
        p2.f.y = p2.f.y + lower;

        nlColour c2;
        c2.c[0] = 0;
        c2.c[1] = 0;
        c2.c[2] = 0;
        c2.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c2);

        nlVector3 v2;
        v2.f.x = p1.f.x;
        v2.f.y = p1.f.y;
        v2.f.z = p1.f.z;
        ((GLMeshWriterCore*)&m0)->Vertex(v2);

        nlColour c3;
        c3.c[0] = 0;
        c3.c[1] = 0;
        c3.c[2] = 0;
        c3.c[3] = 0xFF;
        ((GLMeshWriterCore*)&m0)->Colour(c3);

        nlVector3 v3;
        v3.f.x = p2.f.x;
        v3.f.y = p2.f.y;
        v3.f.z = p2.f.z;
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
}
