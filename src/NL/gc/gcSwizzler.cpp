#include "NL/gc/gcSwizzler.h"
#include "NL/nlColour.h"

/**
 * Offset/Address/Size: 0x4A4 | 0x801C2668 | size: 0x74
 */
u32 GCTextureSize(eGXTextureFormat fmt, int w, int h, int levels, unsigned long /*unused*/)
{
    unsigned long total = 0;

    for (;;)
    {
        int row_bytes;
        if ((int)fmt == 2)
        {
            s32 sign = (u32)w >> 31;
            row_bytes = (sign + w) >> 1;
        }
        else if ((int)fmt == 3)
        {
            row_bytes = w << 2;
        }
        else if ((int)fmt == 6 || (int)fmt == 8 || (int)fmt == 4)
        {
            row_bytes = w;
        }
        else
        {
            row_bytes = w << 1;
        }

        total += (unsigned long)(h * row_bytes);

        levels -= 1;
        if (levels == 0)
            break;

        w >>= 1;
        h >>= 1;
    }

    return total;
}

/**
 * Offset/Address/Size: 0x0 | 0x801C21C4 | size: 0x4A4
 */
void GCSwizzle(void* pSwizzledData, const void* pLinearData, unsigned short width, unsigned short height, eGXTextureFormat format, bool bEndianSwap)
{
    int stride;

    if ((int)format == 2)
    {
        stride = width >> 1;
    }
    else if ((int)format == 3)
    {
        stride = width << 2;
    }
    else if ((int)format == 6 || (int)format == 8 || (int)format == 4)
    {
        stride = width;
    }
    else
    {
        stride = width << 1;
    }

    if (stride == (width << 2))
    {
        nlColour* pSrc = (nlColour*)pLinearData;
        nlColour* pTempSrc;
        unsigned char* pDest = (unsigned char*)pSwizzledData;
        int y, yy, x, i;

        for (y = 0; y < height; y += 4)
        {
            for (x = 0; x < width; x += 4)
            {
                pTempSrc = pSrc;
                i = 0;
                for (yy = 0; yy < 4; yy++)
                {
                    pDest[i + 0x00] = pTempSrc[0].c[3];
                    pDest[i + 0x01] = pTempSrc[0].c[0];
                    pDest[i + 0x20] = pTempSrc[0].c[1];
                    pDest[i + 0x21] = pTempSrc[0].c[2];

                    pDest[i + 0x02] = pTempSrc[1].c[3];
                    pDest[i + 0x03] = pTempSrc[1].c[0];
                    pDest[i + 0x22] = pTempSrc[1].c[1];
                    pDest[i + 0x23] = pTempSrc[1].c[2];

                    pDest[i + 0x04] = pTempSrc[2].c[3];
                    pDest[i + 0x05] = pTempSrc[2].c[0];
                    pDest[i + 0x24] = pTempSrc[2].c[1];
                    pDest[i + 0x25] = pTempSrc[2].c[2];

                    pDest[i + 0x06] = pTempSrc[3].c[3];
                    pDest[i + 0x07] = pTempSrc[3].c[0];
                    pDest[i + 0x26] = pTempSrc[3].c[1];
                    pDest[i + 0x27] = pTempSrc[3].c[2];

                    i += 8;
                    pTempSrc += width;
                }

                pSrc += 4;
                pDest += 64;
            }

            pSrc += width * 3;
        }
    }
    else if (stride == (width << 1))
    {
        unsigned char* pDest = (unsigned char*)pSwizzledData;
        const unsigned char* pSrc = (const unsigned char*)pLinearData;
        const unsigned char* pTempSrc;
        int y, yy, x, i;

        for (y = 0; y < height; y += 4)
        {
            for (x = 0; x < width; x += 4)
            {
                pTempSrc = pSrc;
                unsigned int off;
                for (yy = 0, off = 0, i = 0; yy < 2; yy++, off += 8, i += 8)
                {
                    *(unsigned short*)(pDest + ((i + 0) << 1)) = *(const unsigned short*)(pTempSrc + 0);
                    *(unsigned short*)(pDest + ((i + 1) << 1)) = *(const unsigned short*)(pTempSrc + 2);
                    *(unsigned short*)(pDest + ((i + 2) << 1)) = *(const unsigned short*)(pTempSrc + 4);
                    *(unsigned short*)(pDest + ((i + 3) << 1)) = *(const unsigned short*)(pTempSrc + 6);

                    off += 8;
                    pTempSrc += (width << 1);

                    *(unsigned short*)(pDest + off) = *(const unsigned short*)(pTempSrc + 0);
                    *(unsigned short*)(pDest + ((i + 5) << 1)) = *(const unsigned short*)(pTempSrc + 2);
                    *(unsigned short*)(pDest + ((i + 6) << 1)) = *(const unsigned short*)(pTempSrc + 4);
                    *(unsigned short*)(pDest + ((i + 7) << 1)) = *(const unsigned short*)(pTempSrc + 6);

                    pTempSrc += (width << 1);
                }

                pSrc += 8;
                pDest += off;
            }

            pSrc += width * 6;
        }

        if (bEndianSwap)
        {
            unsigned short* p = (unsigned short*)pSwizzledData;

            for (i = 0; i < width * height; i++)
            {
                unsigned short swapped;
                unsigned short value = p[i];

                ((unsigned char*)&swapped)[0] = ((unsigned char*)&value)[1];
                ((unsigned char*)&swapped)[1] = ((unsigned char*)&value)[0];

                p[i] = swapped;
            }
        }
    }
    else if (stride == (int)((unsigned int)width >> 1))
    {
        // 4 bits per pixel is not supported
    }
    else if (stride == width)
    {
        int y, x, yy;
        unsigned char* pDest = (unsigned char*)pSwizzledData;
        const unsigned char* pSrc = (const unsigned char*)pLinearData;
        const unsigned char* tempSrc;

        for (y = 0; y < height; y += 4)
        {
            for (x = 0; x < width; x += 8)
            {
                tempSrc = pSrc;

                for (yy = 0; yy < 4; yy++)
                {
                    pDest[0] = tempSrc[0];
                    pDest[1] = tempSrc[1];
                    pDest[2] = tempSrc[2];
                    pDest[3] = tempSrc[3];
                    pDest[4] = tempSrc[4];
                    pDest[5] = tempSrc[5];
                    pDest[6] = tempSrc[6];
                    pDest[7] = tempSrc[7];

                    tempSrc += width;
                    pDest += 8;
                }

                pSrc += 8;
            }

            pSrc += width * 3;
        }
    }
}
