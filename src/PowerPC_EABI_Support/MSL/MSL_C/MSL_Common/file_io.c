#include "file_io.h"
#include "ctype.h"

FILE* __find_unopened_file(void);
void __begin_critical_region(int region);
void __end_critical_region(int region);

/**
 * Offset/Address/Size: 0x17C | 0x80230854 | size: 0x17C
 * TODO: 96.21% match - register allocation: mode_char r7 vs r6, open_mode r5 vs r7 swap;
 * also missing addi r5,r3,2 pointer materialization for mode[2] access
 */
int __get_file_modes(const char* mode, file_modes* modes)
{
    int mode_char;
    int open_mode;
    int io_mode;

    modes->file_kind = __disk_file;
    modes->file_orientation = 0;
    modes->binary_io = 0;

    mode_char = *mode;

    switch (mode_char)
    {
    case 'r':
        open_mode = 0;
        break;
    case 'w':
        open_mode = 2;
        break;
    case 'a':
        open_mode = 1;
        break;
    default:
        return 0;
    }

    modes->open_mode = open_mode;

    switch (mode[1])
    {
    case 'b':
        modes->binary_io = 1;
        if (mode[2] == '+')
        {
            mode_char = (mode_char << 8) | '+';
        }
        break;
    case '+':
        mode_char = (mode_char << 8) | '+';
        if (mode[2] == 'b')
        {
            modes->binary_io = 1;
        }
        break;
    }

    switch (mode_char)
    {
    case 'r':
        io_mode = __read;
        break;
    case 'w':
        io_mode = __write;
        break;
    case 'a':
        io_mode = __write | __append;
        break;
    case ('r' << 8) | '+':
        io_mode = __read_write;
        break;
    case ('w' << 8) | '+':
        io_mode = __read_write;
        break;
    case ('a' << 8) | '+':
        io_mode = __read | __write | __append;
        break;
    }

    modes->io_mode = io_mode;
    return 1;
}

/**
 * Offset/Address/Size: 0x3DC | 0x802309D0 | size: 0x250
 * TODO: 99.01% match - dead beq from inlined fflush NULL check not generated;
 * stack offset swap (modes at sp+0x10 vs sp+0x08)
 */
FILE* fopen(const char* filename, const char* mode)
{
    FILE* file;
    file_modes modes;

    __begin_critical_region(2);

    file = __find_unopened_file();
    __stdio_atexit();

    if (!file)
    {
        file = NULL;
    }
    else
    {
        if (file->file_mode.file_kind != __closed_file)
        {
            fflush(file);
            (*file->close_fn)(file->handle);
            file->file_mode.file_kind = __closed_file;
            file->handle = 0;
            if (file->file_state.free_buffer)
                free(file->buffer);
        }

        clearerr(file);

        if (!__get_file_modes(mode, &modes))
        {
            file = NULL;
        }
        else
        {
            __init_file(file, modes, NULL, 0x400);

            if (__open_file(filename, modes, (__file_handle*)file))
            {
                file->file_mode.file_kind = __closed_file;
                if (file->file_state.free_buffer)
                    free(file->buffer);
                file = NULL;
            }
            else if (modes.io_mode & __append)
            {
                fseek(file, 0, SEEK_END);
            }
        }
    }

    __end_critical_region(2);
    return file;
}

/* 803659F8-80365BB4 360338 01BC+00 0/0 1/1 0/0 .text            fclose */
int fclose(FILE* file)
{
    int flush_result, close_result;

    if (file == NULL)
        return (-1);
    if (file->file_mode.file_kind == __closed_file)
        return (0);

    flush_result = fflush(file);

    close_result = (*file->close_fn)(file->handle);

    file->file_mode.file_kind = __closed_file;
    file->handle = 0;

    if (file->file_state.free_buffer)
        free((FILE*)file->buffer);
    return ((flush_result || close_result) ? -1 : 0);
}

/* 803658C0-803659F8 360200 0138+00 0/0 4/4 0/0 .text            fflush */
int fflush(FILE* file)
{
    int pos;

    if (file == NULL)
    {
        return __flush_all();
    }

    if (file->file_state.error != 0 || file->file_mode.file_kind == __closed_file)
    {
        return -1;
    }

    if (file->file_mode.io_mode == __read)
    {
        return 0;
    }

    if (file->file_state.io_state >= __rereading)
    {
        file->file_state.io_state = __reading;
    }

    if (file->file_state.io_state == __reading)
    {
        file->buffer_length = 0;
    }

    if (file->file_state.io_state != __writing)
    {
        file->file_state.io_state = __neutral;
        return 0;
    }

    if (file->file_mode.file_kind != __disk_file)
    {
        pos = 0;
    }
    else
    {
        pos = ftell(file);
    }

    if (__flush_buffer(file, 0) != 0)
    {
        file->file_state.error = 1;
        file->buffer_length = 0;
        return -1;
    }

    file->file_state.io_state = __neutral;
    file->position = pos;
    file->buffer_length = 0;
    return 0;
}

/* 8036581C-803658C0 36015C 00A4+00 0/0 1/1 0/0 .text            __msl_strnicmp */
int __msl_strnicmp(const char* str1, const char* str2, int n)
{
    int i;
    char c1, c2;

    for (i = 0; i < n; i++)
    {
        c1 = _tolower(*str1++);
        c2 = _tolower(*str2++);

        if (c1 < c2)
        {
            return -1;
        }

        if (c1 > c2)
        {
            return 1;
        }

        if (c1 == '\0')
        {
            return 0;
        }
    }

    return 0;
}
