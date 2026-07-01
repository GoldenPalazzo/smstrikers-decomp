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
 * TODO: 93.98% match - remaining RGBA8 temp-register swaps and
 *   16-bit swizzle indexed-store shape differences.
 */
void GCSwizzle(void* dst_, const void* src_, unsigned short w, unsigned short h, eGXTextureFormat fmt, bool swap16)
{
    int pitch_by_fmt;
    if ((int)fmt == 2)
    {
        pitch_by_fmt = w >> 1;
    }
    else if ((int)fmt == 3)
    {
        pitch_by_fmt = w << 2;
    }
    else
    {
        if ((int)fmt == 6 || (int)fmt == 8 || (int)fmt == 4)
        {
            pitch_by_fmt = w;
        }
        else
        {
            pitch_by_fmt = w << 1;
        }
    }

    unsigned int pitch4 = w << 2;
    if (pitch_by_fmt == (int)pitch4)
    {
        unsigned int ww = w;
        unsigned int row_advance_after_strip = ww * 12;
        nlColour* s = (nlColour*)src_;
        unsigned char* d = (unsigned char*)dst_;
        int y = 0;
        while (y < h)
        {
            int x = 0;
            while (x < (int)ww)
            {
                nlColour* rs = s;
                unsigned int d_off = 0;
                for (int i = 0; i < 4; ++i)
                {
                    unsigned char* rd = d + d_off;

                    rd[0x00] = rs[0].c[0x03];
                    rd[0x01] = rs[0].c[0x00];
                    rd[0x20] = rs[0].c[0x01];
                    rd[0x21] = rs[0].c[0x02];

                    rd[0x02] = rs[1].c[0x03];
                    rd[0x03] = rs[1].c[0x00];
                    rd[0x22] = rs[1].c[0x01];
                    rd[0x23] = rs[1].c[0x02];

                    rd[0x04] = rs[2].c[0x03];
                    rd[0x05] = rs[2].c[0x00];
                    rd[0x24] = rs[2].c[0x01];
                    rd[0x25] = rs[2].c[0x02];

                    rd[0x06] = rs[3].c[0x03];
                    rd[0x07] = rs[3].c[0x00];
                    rd[0x26] = rs[3].c[0x01];
                    rd[0x27] = rs[3].c[0x02];

                    d_off += 8;
                    rs += ww;
                }

                s += 4;
                d += 0x40;
                x += 4;
            }
            s += ww * 3;
            y += 4;
        }
        return;
    }

    if (pitch_by_fmt == (w << 1))
    {
        unsigned int ww = w;
        unsigned int row_advance_after_strip = ww * 6;
        unsigned char* d = (unsigned char*)dst_;
        const unsigned char* s = (const unsigned char*)src_;
        int y = 0;
        while (y < h)
        {
            int x = 0;
            while (x < (int)ww)
            {
                const unsigned char* rs = s;
                unsigned int rindex = 0;
                unsigned int off = 0;

                for (int k = 0; k < 2; ++k)
                {
                    *(unsigned short*)(d + (rindex << 1)) = *(const unsigned short*)(rs + 0);
                    *(unsigned short*)(d + ((rindex + 1) << 1)) = *(const unsigned short*)(rs + 2);
                    *(unsigned short*)(d + ((rindex + 2) << 1)) = *(const unsigned short*)(rs + 4);
                    *(unsigned short*)(d + ((rindex + 3) << 1)) = *(const unsigned short*)(rs + 6);

                    off += 8;
                    rs += (w << 1);

                    *(unsigned short*)(d + off) = *(const unsigned short*)(rs + 0);
                    *(unsigned short*)(d + ((rindex + 5) << 1)) = *(const unsigned short*)(rs + 2);
                    *(unsigned short*)(d + ((rindex + 6) << 1)) = *(const unsigned short*)(rs + 4);
                    *(unsigned short*)(d + ((rindex + 7) << 1)) = *(const unsigned short*)(rs + 6);

                    rindex += 8;
                    off += 8;
                    rs += (w << 1);
                }

                s += 8;
                d += 0x20;
                x += 4;
            }
            s += row_advance_after_strip;
            y += 4;
        }

        if (swap16)
        {
            int count = ww * h;
            unsigned short* p = (unsigned short*)dst_;
            int i = 0;
            while (i < count)
            {
                unsigned short swapped;
                unsigned short val = p[i];
                ((unsigned char*)&swapped)[0] = ((unsigned char*)&val)[1];
                ((unsigned char*)&swapped)[1] = ((unsigned char*)&val)[0];
                p[i] = swapped;
                i++;
            }
        }
        return;
    }

    if (pitch_by_fmt != (int)((unsigned int)w >> 1) && pitch_by_fmt == (int)(unsigned int)w)
    {
        unsigned int ww = w;
        unsigned int row_advance_after_strip = ww * 3;
        unsigned char* d = (unsigned char*)dst_;
        const unsigned char* s = (const unsigned char*)src_;
        for (int y = 0; y < h; y += 4)
        {
            for (int x = 0; x < (int)ww; x += 8)
            {
                const unsigned char* rs = s;

                for (int k = 0; k < 2; ++k)
                {
                    d[0] = rs[0];
                    d[1] = rs[1];
                    d[2] = rs[2];
                    d[3] = rs[3];
                    d[4] = rs[4];
                    d[5] = rs[5];
                    d[6] = rs[6];
                    d[7] = rs[7];

                    rs += ww;

                    d[8] = rs[0];
                    d[9] = rs[1];
                    d[10] = rs[2];
                    d[11] = rs[3];
                    d[12] = rs[4];
                    d[13] = rs[5];
                    d[14] = rs[6];
                    d[15] = rs[7];

                    rs += ww;
                    d += 16;
                }

                s += 8;
            }
            s += row_advance_after_strip;
        }
    }
}
