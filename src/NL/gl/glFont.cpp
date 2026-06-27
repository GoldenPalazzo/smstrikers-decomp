#include "NL/gl/glFont.h"
#include "NL/gl/glState.h"
#include "NL/nlPrint.h"
#include "NL/nlString.h"
#include "NL/glx/glxFont.h"

#include "NL/gl/glDraw2.h"

#include "font_data.h"

static glPoly2 g_poly[128];
static float font_z;
static void* handle;
static bool bInsideBegin;
static bool bDrop;
static bool bEnabled;
static bool bVirtualCoords;

/**
 * Offset/Address/Size: 0x0 | 0x801D8398 | size: 0xE8
 */
int glFontPrintf(eGLView view, int x, int y, const nlColour& col, const char* format, ...)
{
    char string[0x80];
    va_list args;

    if (bEnabled == false)
    {
        return false;
    }

    va_start(args, format);
    nlVSNPrintf(string, 0x80, format, args);
    va_end(args);

    return glFontPrint(view, x, y, col, string);
}

/**
 * Offset/Address/Size: 0xE8 | 0x801D8480 | size: 0xF4
 */
int glFontPrintf(eGLView view, int x, int y, const char* format, ...)
{
    char string[0x80];
    va_list args;

    if (bEnabled == false)
    {
        return false;
    }

    va_start(args, format);
    nlVSNPrintf(string, 0x80, format, args);
    va_end(args);

    nlColour col;
    col.c[0] = 255;
    col.c[1] = 255;
    col.c[2] = 255;
    col.c[3] = 255;

    return glFontPrint(view, x, y, col, string);
}

/**
 * Offset/Address/Size: 0x1DC | 0x801D8574 | size: 0x3EC
 * TODO: 99.1% match - r4/r5 character register swap and r4/r6 drop-shadow loop cursor.
 */
#pragma optimize_for_size on
int glFontPrint(eGLView view, int virtual_x, int virtual_y, const nlColour& colour, const char* str)
{
    if (nlStrLen(str) == 0)
    {
        return 0;
    }
    if (bEnabled == false)
    {
        return 0;
    }
    int screen_x = virtual_x;
    int screen_y = virtual_y;
    if ((u8)bVirtualCoords != 0)
    {
        screen_x = (screen_x * 10) + 40;
        screen_y = (screen_y * 11) + 32;
    }
    nlStrLen(str);
    int numChars = 0;
    glPoly2* pPoly = g_poly;
    const char* cp = str;
    while (*cp != '\0')
    {
        if (((s8)*cp >= 0x20) && ((s8)*cp <= 0x7E))
        {
            int j = (((s8)*cp - 0x20) % 8) * 8;
            int i = (((s8)*cp - 0x20) / 8) * 8;
            float us = 0.015625f;
            float vs = 0.0078125f;
            float s = (float)j;
            float t = (float)i;
            float s0 = s * us;
            float t0 = t * vs;
            pPoly->m_uv[0].f.x = s0;
            pPoly->m_uv[0].f.y = t0;
            pPoly->m_uv[1].f.x = s0;
            pPoly->m_uv[1].f.y = (t + 8.0f) * vs;
            pPoly->m_uv[2].f.x = (s + 8.0f) * us;
            pPoly->m_uv[2].f.y = (t + 8.0f) * vs;
            pPoly->m_uv[3].f.x = (s + 8.0f) * us;
            pPoly->m_uv[3].f.y = t0;
            pPoly->m_pos[0].f.x = screen_x;
            pPoly->m_pos[0].f.y = screen_y;
            pPoly->m_pos[1].f.x = screen_x;
            pPoly->m_pos[1].f.y = 10.0f + screen_y;
            pPoly->m_pos[2].f.x = 10.0f + screen_x;
            pPoly->m_pos[2].f.y = 10.0f + screen_y;
            pPoly->m_pos[3].f.x = 10.0f + screen_x;
            pPoly->m_pos[3].f.y = screen_y;
            pPoly->depth = font_z;
            *(u32*)&pPoly->m_colour[0].c[0] = *(u32*)&colour.c[0];
            *(u32*)&pPoly->m_colour[1].c[0] = *(u32*)&colour.c[0];
            *(u32*)&pPoly->m_colour[2].c[0] = *(u32*)&colour.c[0];
            *(u32*)&pPoly->m_colour[3].c[0] = *(u32*)&colour.c[0];
            pPoly++;
            numChars++;
        }
        else if ((s8)*cp == '\n')
        {
            screen_x = 30;
            screen_y += 11;
        }
        screen_x += 10;
        cp++;
    }
    if (bDrop != false)
    {
        pPoly = g_poly;
        glPoly2* p = pPoly;
        int n = numChars;
        while (n > 0)
        {
            p->m_colour[0].c[0] = 0;
            p->m_colour[0].c[1] = 0;
            p->m_colour[0].c[2] = 0;
            p->m_colour[0].c[3] = 0xFF;
            p->m_pos[0].f.x += 3.0f;
            p->m_pos[0].f.y += 3.0f;
            p->m_colour[1].c[0] = 0;
            p->m_colour[1].c[1] = 0;
            p->m_colour[1].c[2] = 0;
            p->m_colour[1].c[3] = 0xFF;
            p->m_pos[1].f.x += 3.0f;
            p->m_pos[1].f.y += 3.0f;
            p->m_colour[2].c[0] = 0;
            p->m_colour[2].c[1] = 0;
            p->m_colour[2].c[2] = 0;
            p->m_colour[2].c[3] = 0xFF;
            p->m_pos[2].f.x += 3.0f;
            p->m_pos[2].f.y += 3.0f;
            p->m_colour[3].c[0] = 0;
            p->m_colour[3].c[1] = 0;
            p->m_colour[3].c[2] = 0;
            p->m_colour[3].c[3] = 0xFF;
            p->m_pos[3].f.x += 3.0f;
            p->m_pos[3].f.y += 3.0f;
            p->depth += -0.001f;
            p++;
            n--;
        }
        glAttachPoly2(view, numChars, g_poly, 0, 0);
        n = numChars;
        while (n > 0)
        {
            {
                float fz = font_z;
                *(u32*)&pPoly->m_colour[0].c[0] = *(u32*)&colour.c[0];
                pPoly->m_pos[0].f.x -= 3.0f;
                pPoly->m_pos[0].f.y -= 3.0f;
                *(u32*)&pPoly->m_colour[1].c[0] = *(u32*)&colour.c[0];
                pPoly->m_pos[1].f.x -= 3.0f;
                pPoly->m_pos[1].f.y -= 3.0f;
                *(u32*)&pPoly->m_colour[2].c[0] = *(u32*)&colour.c[0];
                pPoly->m_pos[2].f.x -= 3.0f;
                pPoly->m_pos[2].f.y -= 3.0f;
                *(u32*)&pPoly->m_colour[3].c[0] = *(u32*)&colour.c[0];
                pPoly->m_pos[3].f.x -= 3.0f;
                pPoly->m_pos[3].f.y -= 3.0f;
                pPoly->depth = fz;
                pPoly++;
            }
            n--;
        }
    }
    glAttachPoly2(view, numChars, g_poly, 0, 0);
    return numChars;
}
#pragma optimize_for_size reset

/**
 * Offset/Address/Size: 0x5C8 | 0x801D8960 | size: 0x18
 */
void glFontEnd()
{
    if (bEnabled != false)
    {
        bInsideBegin = false;
    }
}

/**
 * Offset/Address/Size: 0x5E0 | 0x801D8978 | size: 0x7C
 */
void glFontBegin(bool drop)
{
    if (bEnabled != false)
    {
        glSetDefaultState(0);
        glSetCurrentTexture((unsigned long)handle, GLTT_Diffuse);
        glSetRasterState(GLS_AlphaTest, 1);
        glSetCurrentRasterState(glHandleizeRasterState());
        glSetTextureState(GLTS_DiffuseFilter, 1);
        glSetCurrentTextureState(glHandleizeTextureState());
        bDrop = drop;
        bInsideBegin = true;
    }
}

/**
 * Offset/Address/Size: 0x65C | 0x801D89F4 | size: 0x24
 */
void glFontVirtualPosToScreenCoordPos(float x, float y, float& outX, float& outY)
{
    outX = (10.0f * x) + 40.0f;
    outY = (11.0f * y) + 32.0f;
}

/**
 * Offset/Address/Size: 0x680 | 0x801D8A18 | size: 0x5C
 */
void gl_FontStartup()
{
    handle = (void*)glGetTexture("font/fixed8x8");
    glplatCreateFont(0x40, 0x80, _fontData, (unsigned long)handle);
    bInsideBegin = 0;
    bEnabled = 1;
    bDrop = 0;
    bVirtualCoords = 1;
}
