#include "NL/glx/glxSwap.h"
#include "NL/nlDebug.h"
#include "NL/nlPrint.h"
#include "NL/nlEndian.h"
#include "NL/nlMemory.h"
#include "NL/nlConfig.h"

#include "dolphin/GX.h"
#include "dolphin/VI.h"
#include "dolphin/os/OSThread.h"
#include "dolphin/PPCArch.h"
#include "NL/glx/glxGX.h"
#include "NL/gl/glPlat.h"
#include "NL/gl/glConstant.h"
#include "Game/ResetTask.h"

#include "direct_io.h"

u32 count0 = 0;
int glx_nBuffer = 0;
int _shotno = 0;
int glx_SwapMode = 0;
u8 glx_ResetCaptureFrame = 0;

static u8 glx_bLoadingIndicator = 0;
static u8 glx_bAllowDrawSync = 0;

u8 glx_ScreenShot = 0;
u8 glx_MovieOutput = 0;
int nFirstFrame = 0;
int glx_nLoadFrame = 0;
int glx_nLoadWaitFrames = 0;

void* glx_FrameBuffer[2];

static u8 bSaved = 0;
static u8 bSavedState = 0;

bool glx_bLoadOtherPosition = false;
int loadCounter = 0;
int nSelected = 0;
static int LoadWaitSeconds = 0;

extern u8 bInRetrace;
extern s32 glx_nBlitXor;
extern u16 _ImageData[0x400];
extern u16 _SelectedImageData[0x400];

/**
 * Offset/Address/Size: 0x0 | 0x801BED50 | size: 0x118
 */
void DrawLoadingIndicator()
{
    u32 targetFPS = glx_GetTargetFPS();
    int counterLimit;
    int xPos;
    int yPos;
    float scale;
    int yPosTemp;

    if (targetFPS == 50)
    {
        xPos = 0x120;
        yPosTemp = 0x1B4;
        if (glx_bLoadOtherPosition)
        {
            yPosTemp = 0x1A2;
        }
        scale = 1.0f;
        yPos = yPosTemp;
        counterLimit = 0x10;
    }
    else
    {
        xPos = 0x120;
        yPosTemp = 0x17E;
        if (glx_bLoadOtherPosition)
        {
            yPosTemp = 0x16E;
        }
        scale = 1.0f;
        yPos = yPosTemp;
        counterLimit = 0x13;
    }

    int spacing = (int)(24.0f * scale);
    if (spacing & 1)
    {
        spacing++;
    }

    for (int i = 0; i < 3; i++)
    {
        BlitImage(xPos, yPos, scale, scale, !(nSelected - i));
        xPos += spacing;
    }

    loadCounter++;
    if (loadCounter >= counterLimit)
    {
        loadCounter = 0;
        nSelected++;
        if (nSelected >= 3)
        {
            nSelected = 0;
        }
    }
}

/**
 * Offset/Address/Size: 0x118 | 0x801BEE68 | size: 0x2D0
 */
void BlitImage(int arg0, int arg1, float arg2, float arg3, bool arg4)
{
    float limit = 24.0f;
    float xStep = limit / (float)(int)(limit * arg2);
    float yStep = limit / (float)(int)(limit * arg3);
    float ySample = 0.0f;
    int x2 = arg0 << 1;
    u8 useSelected = arg4;
    const u16* selectedBase = _SelectedImageData;
    const u16* imageBase = _ImageData;
    int yOffset = 0;

    while (ySample < limit)
    {
        int srcRow = (int)ySample;
        int dstRow = (arg1 + yOffset) * 0x500;
        float xSample = 0.0f;
        int xOffset = 0;
        const u16* selectedRow = selectedBase + (srcRow << 5);
        const u16* imageRow = imageBase + (srcRow << 5);
        int prev0;
        int older0;
        int prev1;
        int older1;

        while (xSample < limit)
        {
            int srcCol = (int)xSample;
            u16 pixel;
            if (useSelected)
            {
                pixel = selectedRow[srcCol];
            }
            else
            {
                pixel = imageRow[srcCol];
            }

            int alpha = (pixel & 0xF) | ((pixel & 0xF) << 4);
            int yv = ((pixel >> 12) & 0xF) | (((pixel >> 12) & 0xF) << 4);
            int uv0 = ((pixel >> 8) & 0xF) | (((pixel >> 8) & 0xF) << 4);
            int uv1 = ((pixel >> 4) & 0xF) | (((pixel >> 4) & 0xF) << 4);

            if (alpha > 0)
            {
                int dstX = arg0 + xOffset;
                u8* dst = (u8*)glx_FrameBuffer[glx_nBuffer ^ glx_nBlitXor] + dstRow + (dstX << 1);
                dst[0] = (u8)yv;

                if ((dstX & 1) != 0)
                {
                    dst[-1] = (u8)(0.5f * (float)prev0 + 0.25f * (float)older0 + 0.25f * (float)uv0);
                    dst[1] = (u8)(0.5f * (float)prev1 + 0.25f * (float)older1 + 0.25f * (float)uv1);
                }
            }

            if (xOffset == 0)
            {
                prev0 = uv0;
            }
            older0 = prev0;
            prev0 = uv0;

            if (xOffset == 0)
            {
                prev1 = uv1;
            }
            older1 = prev1;
            prev1 = uv1;

            xSample += xStep;
            xOffset++;
        }

        DCStoreRangeNoSync((u8*)glx_FrameBuffer[glx_nBuffer ^ glx_nBlitXor] + dstRow + x2,
            (u32)(xOffset << 1));

        ySample += yStep;
        yOffset++;
    }

    PPCSync();
}

/**
 * Offset/Address/Size: 0x3E8 | 0x801BF138 | size: 0xC0
 */
void hitz_Post(bool arg0)
{
    if ((u8)glx_ResetCaptureFrame != 0)
    {
        _shotno = 0;
    }
    if ((u8)glx_ScreenShot != 0)
    {
        glx_ScreenCapture(false);
        glx_ScreenShot = 0;
    }
    if ((u8)glx_MovieOutput != 0)
    {
        glx_ScreenCapture(true);
    }

    GXColor clearColor = { 0, 0, 0, 0 };
    GXSetCopyClear(clearColor, 0xFFFFFF);
    gxSetZMode(true, GX_LEQUAL, true);
    gxSetColourUpdate(true);
    gxSetAlphaUpdate(true);
    GXCopyDisp(glx_FrameBuffer[glx_nBuffer], true);
    GXSetDrawDone();
    GXFlush();
    count0 = VIGetRetraceCount();
}

/**
 * Offset/Address/Size: 0x4A8 | 0x801BF1F8 | size: 0xD4
 */
void hitz_Pre(bool)
{
    GXWaitDrawDone();
    u32 retraceCount = VIGetRetraceCount();
    float value = glConstantGet("glxswap/vwait").f.x;

    if (value == 2.0f)
    {
        hitz_AdvanceFrame();
        VIWaitForRetrace();
        u32 newRetraceCount = VIGetRetraceCount();
        if (newRetraceCount < count0 + 2)
        {
            VIWaitForRetrace();
        }
    }
    else if (value == 1.0f)
    {
        hitz_AdvanceFrame();
        VIWaitForRetrace();
    }
    else if (count0 == retraceCount)
    {
        hitz_AdvanceFrame();
        VIWaitForRetrace();
    }
    else if (value == 0.5f)
    {
        while (bInRetrace != 0)
        {
            OSYieldThread();
        }
        hitz_AdvanceFrame();
    }
    else
    {
        hitz_AdvanceFrame();
    }

    hitz_SwapBuffers();
}

/**
 * Offset/Address/Size: 0x57C | 0x801BF2CC | size: 0x10
 */
void hitz_SwapBuffers()
{
    glx_nBuffer ^= 1;
}

/**
 * Offset/Address/Size: 0x58C | 0x801BF2DC | size: 0x54
 */
void hitz_AdvanceFrame()
{
    VISetNextFrameBuffer(glx_FrameBuffer[glx_nBuffer]);
    if (nFirstFrame > 0)
    {
        nFirstFrame--;
        if (nFirstFrame == 0)
        {
            VISetBlack(false);
        }
    }
    VIFlush();
}

/**
 * Offset/Address/Size: 0x5E0 | 0x801BF330 | size: 0xA0
 */
void simple_Post(bool arg0)
{
    gxSetZMode(true, GX_LEQUAL, true);
    gxSetColourUpdate(true);
    gxSetAlphaUpdate(true);
    GXCopyDisp(glx_FrameBuffer[glx_nBuffer], true);
    GXDrawDone();
    VISetNextFrameBuffer(glx_FrameBuffer[glx_nBuffer]);
    if (nFirstFrame > 0)
    {
        nFirstFrame--;
        if (nFirstFrame == 0)
        {
            VISetBlack(false);
        }
    }
    VIFlush();
    VIWaitForRetrace();
    glx_nBuffer ^= 1;
}

/**
 * Offset/Address/Size: 0x680 | 0x801BF3D0 | size: 0x4
 */
void simple_Pre(bool)
{
    // EMPTY
}

/**
 * Offset/Address/Size: 0x684 | 0x801BF3D4 | size: 0x58
 */
void glxSwapPost(bool bSend)
{
    if (glx_bLoadingIndicator == false)
    {
        switch ((s32)glx_SwapMode)
        {
        case 0:
            simple_Post(bSend);
            return;
        case 1:
            hitz_Post(bSend);
            return;
        default:
            nlBreak();
            break;
        }
    }
}

/**
 * Offset/Address/Size: 0x6DC | 0x801BF42C | size: 0x58
 */
void glxSwapPre(bool bSend)
{
    if (glx_bLoadingIndicator == false)
    {
        switch ((s32)glx_SwapMode)
        {
        case 0:
            simple_Pre(bSend);
            return;
        case 1:
            hitz_Pre(bSend);
            return;
        default:
            nlBreak();
            break;
        }
    }
}

static inline void fillStringData(BasicStringInternal* data, const char* str)
{
    data->mData = 0;
    const char* s = str;
    data->mSize = 0;
    data->mCapacity = 0;
    while ((signed char)*s++ != 0)
    {
        data->mSize++;
    }
    data->mSize++;
    data->mData = (char*)Detail::TempStringAllocator::allocate(data->mSize + 1);
    data->mCapacity = data->mSize;
    for (s32 i = 0; i < data->mSize; i++)
    {
        data->mData[i] = *str++;
    }
    data->mRefCount = 1;
}

/**
 * Offset/Address/Size: 0x734 | 0x801BF484 | size: 0x260
 * TODO: 97.99% match - r30/r31 register swap for data and str pointers
 * in the string construction section (offsets 0x48-0xE4). MWCC allocator
 * assigns data=r31/str=r30 instead of target data=r30/str=r31.
 */
void glxInitSwap(void* arg0, void* arg1)
{
    BasicStringInternal* data;

    glx_FrameBuffer[0] = arg0;
    glx_FrameBuffer[1] = arg1;
    glx_nBuffer = 0;
    nFirstFrame = 3;
    glx_bAllowDrawSync = 1;

    data = (BasicStringInternal*)nlMalloc(0x10, 8, true);
    if (data != 0)
    {
        fillStringData(data, "hitz");
    }

    BasicString<char, Detail::TempStringAllocator> mode(
        Config::Global().Get<BasicString<char, Detail::TempStringAllocator> >(
            "swapmode", BasicString<char, Detail::TempStringAllocator>(data)));

    glx_SwapMode = !(mode == "simple");
    switch (glx_SwapMode)
    {
    case 1:
        GXSetDrawDone();
        GXFlush();
        VISetPreRetraceCallback(vi_pre_cb);
        VISetPostRetraceCallback(vi_post_cb);
        break;
    case 0:
        break;
    default:
        nlBreak();
        break;
    }
}

/**
 * Offset/Address/Size: 0x994 | 0x801BF6E4 | size: 0x28
 */
void glxSwapWaitDrawDone()
{
    GXSetDrawDone();
    GXFlush();
    GXWaitDrawDone();
}

/**
 * Offset/Address/Size: 0x9BC | 0x801BF70C | size: 0x50
 */
void vi_post_cb(unsigned long)
{
    HandleSoftReset();
    if (glx_bLoadingIndicator != false)
    {
        glx_nLoadFrame++;
        if (glx_nLoadFrame >= glx_nLoadWaitFrames)
        {
            loading_indicator();
        }
    }
    bInRetrace = 0;
}

/**
 * Offset/Address/Size: 0xA0C | 0x801BF75C | size: 0xC
 */
void vi_pre_cb(unsigned long)
{
    bInRetrace = 1;
}

/**
 * Offset/Address/Size: 0xA18 | 0x801BF768 | size: 0x88
 */
void loading_indicator()
{
    if (nFirstFrame == 0)
    {
        DrawLoadingIndicator();
        VISetNextFrameBuffer(glx_FrameBuffer[glx_nBuffer]);
        VIFlush();
        glx_nBuffer ^= 1;
    }

    if (nFirstFrame > 0)
    {
        nFirstFrame--;
        if (nFirstFrame == 0)
        {
            glx_ClearXFB(glx_FrameBuffer[0]);
            glx_ClearXFB(glx_FrameBuffer[1]);
            VISetBlack(false);
            VIFlush();
        }
    }
}

/**
 * Offset/Address/Size: 0xAA0 | 0x801BF7F0 | size: 0x54
 */
void glxLoadRestoreState()
{
    bSaved = 0;
    glx_bLoadingIndicator = *(u8*)&bSavedState;
    if (bSavedState)
    {
        glxSwapLoading(true, false);
        glx_ClearXFB(glx_FrameBuffer[0]);
        glx_ClearXFB(glx_FrameBuffer[1]);
    }
}

/**
 * Offset/Address/Size: 0xAF4 | 0x801BF844 | size: 0x40
 */
void glxLoadSaveState()
{
    bSaved = 1;
    bSavedState = glx_bLoadingIndicator;
    if (glx_bLoadingIndicator)
    {
        glxSwapLoading(false, false);
    }
}

/**
 * Offset/Address/Size: 0xB34 | 0x801BF884 | size: 0x4C
 */
void glxSwapLoading(bool arg0, bool arg1)
{
    u32 loadWaitFrames;
    u32 targetFPS;
    u32 lws;

    glx_bLoadOtherPosition = arg1;
    glx_bLoadingIndicator = arg0;
    glx_nLoadFrame = 0;
    targetFPS = glx_GetTargetFPS();
    lws = LoadWaitSeconds;
    loadWaitFrames = lws * targetFPS;
    loadCounter = 0;
    nSelected = 0;
    LoadWaitSeconds = 0;
    glx_nLoadWaitFrames = loadWaitFrames;
}

/**
 * Offset/Address/Size: 0xB80 | 0x801BF8D0 | size: 0x14
 */
void* glxGetBackBuffer()
{
    return glx_FrameBuffer[glx_nBuffer];
}

/**
 * Offset/Address/Size: 0xB94 | 0x801BF8E4 | size: 0x18
 */
void* glxGetDisplayedBuffer()
{
    return glx_FrameBuffer[glx_nBuffer ^ 1];
}

#pragma push
#pragma pack(1)
struct TargaHeader
{
    /* 0x00 */ unsigned char imageIDLength;
    /* 0x01 */ unsigned char colorMapType;
    /* 0x02 */ unsigned char imageType;
    /* 0x03 */ unsigned short firstEntry;
    /* 0x05 */ unsigned short mapLength;
    /* 0x07 */ unsigned char paletteBitsPerPixel;
    /* 0x08 */ unsigned short xOrigin;
    /* 0x0A */ unsigned short yOrigin;
    /* 0x0C */ unsigned short width;
    /* 0x0E */ unsigned short height;
    /* 0x10 */ unsigned char bitsPerPixel;
    /* 0x11 */ unsigned char imageDescriptor;
}; // total size: 0x12
#pragma pop

/**
 * Offset/Address/Size: 0xBAC | 0x801BF8FC | size: 0x1B0
 * TODO: 99.35% match - r27/r28/r29 register allocation cycle (imageData, dead copy,
 * rowPixelOffset). MWCC allocates r27=imageData r28=rowPixelOffset r29=dead_copy
 * instead of target r29=imageData r28=dead_copy r27=rowPixelOffset.
 */
void glx_ScreenCapture(bool isMovie)
{
    char filename[0x40];
    FILE* file;
    TargaHeader header;
    u32 argbColor;
    union
    {
        u32 word;
        u8 bytes[4];
    } colorBytes;
    s32 pixelOffset;
    u8* imageData;
    s32 y, x;

    if (isMovie != 0)
    {
        nlSNPrintf(filename, 0x40, "../shot%03d.tga", _shotno);
    }
    else
    {
        nlSNPrintf(filename, 0x40, "shot%03d.tga", _shotno);
    }

    _shotno++;
    file = fopen(filename, "wb");

    if (file != NULL)
    {
        header.imageIDLength = 0;
        header.colorMapType = 0;
        header.imageType = 2;
        header.firstEntry = 0;
        header.mapLength = 0;
        header.paletteBitsPerPixel = 0;
        header.xOrigin = 0;
        header.yOrigin = 0;
        header.width = 0x280;
        header.height = 0x1C0;
        header.bitsPerPixel = 0x18;
        header.imageDescriptor = 0x20;

        nlSwapEndian(header.width, &(header.width));
        nlSwapEndian(header.height, &(header.height));

        imageData = (u8*)nlMalloc(0xD2000, 8, false);
        GXDrawDone();

        for (y = 0; y < 0x1C0; y++)
        {
            for (x = 0; x < 0x280; x++)
            {
                GXPeekARGB((u16)x, (u16)y, &argbColor);

                pixelOffset = (y * 0x280 + x) * 3;
                colorBytes.word = argbColor;
                imageData[pixelOffset] = colorBytes.bytes[3];
                imageData[pixelOffset + 1] = colorBytes.bytes[2];
                imageData[pixelOffset + 2] = colorBytes.bytes[1];
            }
        }

        fwrite(&header, 1, sizeof(TargaHeader), file);
        fwrite(imageData, 3, 0x46000, file);
        fclose(file);
        delete[] imageData;
    }
}

/**
 * Offset/Address/Size: 0xD5C | 0x801BFAAC | size: 0x48
 */
void glxSwapSetBlack(bool black)
{
    if (black)
    {
        VISetBlack(1);
        nFirstFrame = 3;
    }
    else
    {
        VISetBlack(0);
        nFirstFrame = 0;
    }
}

/**
 * Offset/Address/Size: 0x0 | 0x801BFAF4 | size: 0xA8
 */
// void Config::Get<BasicString<char, Detail::TempStringAllocator>>(const char*, BasicString<char, Detail::TempStringAllocator>)
// {
// }

/**
 * Offset/Address/Size: 0xA8 | 0x801BFB9C | size: 0x84
 */
// void Config::TagValuePair::Get<BasicString<char, Detail::TempStringAllocator>>() const
// {
// }
