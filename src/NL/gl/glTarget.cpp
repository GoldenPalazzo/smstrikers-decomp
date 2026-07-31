#include "NL/gl/glTarget.h"
#include "NL/glx/glxTarget.h"

#include "types.h"

u8 targets[0x1C];
s32 targetTextures[4];

/**
 * Offset/Address/Size: 0x0 | 0x801DE3F8 | size: 0x20
 */
void glGrabFrameBufferToTexture(unsigned long arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3, unsigned int arg4, unsigned int arg5, unsigned int arg6)
{
    glPlatGrabFrameBufferToTexture(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
}

/**
 * Offset/Address/Size: 0x20 | 0x801DE418 | size: 0x8C
 */
void gl_TargetStartup()
{
    targetTextures[0] = -1;
    targets[0] = 1;
    targets[1] = 1;
    targets[2] = 0;
    targets[3] = 0;
    targets[4] = 0;
    targets[5] = 0;
    targetTextures[1] = -1;
    targets[7] = 1;
    targets[8] = 1;
    targets[9] = 0;
    targets[10] = 0;
    targets[11] = 0;
    targets[12] = 0;
    targetTextures[2] = -1;
    targets[14] = 1;
    targets[15] = 1;
    targets[16] = 0;
    targets[17] = 0;
    targets[18] = 0;
    targets[19] = 0;
    targetTextures[3] = -1;
    targets[21] = 1;
    targets[22] = 1;
    targets[23] = 0;
    targets[24] = 0;
    targets[25] = 0;
    targets[26] = 0;
}
