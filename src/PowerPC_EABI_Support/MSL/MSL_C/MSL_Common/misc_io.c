#include "PowerPC_EABI_Support/MSL_C/MSL_Common/misc_io.h"
#include "PowerPC_EABI_Support/MSL_C/MSL_Common/abort_exit.h"

void clearerr(FILE* file)
{
    file->file_state.eof = 0;
    file->file_state.error = 0;
}

void __stdio_atexit(void)
{
    __stdio_exit = __close_all;
}
