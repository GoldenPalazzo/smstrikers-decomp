#include "NL/nlDebugFile.h"

#include "file_io.h"
#include "char_io.h"

/**
 * Offset/Address/Size: 0x0 | 0x801CE954 | size: 0x20
 */
void nlCloseFileDebug(void* fp)
{
    fclose((FILE*)fp);
}

/**
 * Offset/Address/Size: 0x20 | 0x801CE974 | size: 0x20
 */
void nlFlushFileDebug(void* fp)
{
    fflush((FILE*)fp);
}

/**
 * Offset/Address/Size: 0x40 | 0x801CE994 | size: 0x50
 */
void nlWriteLineDebug(void* fp, const char* str, bool arg2)
{
    fputs(str, (FILE*)fp);
    if (arg2 != 0)
    {
        fflush((FILE*)fp);
    }
}

/**
 * Offset/Address/Size: 0x90 | 0x801CE9E4 | size: 0x4C
 */
void* nlOpenFileDebug(const char* str, bool arg1, bool arg2)
{
    if (arg1 != 0)
    {
        return (void*)fopen(str, "wb");
    }
    if (arg2 != 0)
    {
        return (void*)fopen(str, "awt");
    }
    return (void*)fopen(str, "wt");
}
