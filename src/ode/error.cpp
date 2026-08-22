/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include <ode/config.h>
#include <ode/error.h>

#include <stdlib.h>

extern void nlPrintf(const char*, ...);
extern void nlBreak();

static dMessageFunction* debug_function = 0;
static dMessageFunction* message_function = 0;

inline void printMessage(int num, const char* msg1, const char* msg2, va_list ap)
{
    fflush(stderr);
    fflush(stdout);
    if (num)
        fprintf(stderr, "\n%s %d: ", msg1, num);
    else
        fprintf(stderr, "\n%s: ", msg1);

    // vfprintf (stderr,msg2,ap);
    nlPrintf(msg2, ap);

    fprintf(stderr, "\n");
    fflush(stderr);
}

/**
 * Offset/Address/Size: 0xF4 | 0x8021E1B0 | size: 0xF8
 */
extern "C" void dDebug(int num, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    if (debug_function)
    {
        debug_function(num, msg, ap);
    }
    else
    {
        if (num)
        {
            nlPrintf("\n%s %d: ", "ODE INTERNAL ERROR", num);
        }
        else
        {
            nlPrintf("\n%s: ", "ODE INTERNAL ERROR");
        }
        nlPrintf("\n");
    }
    // *((char *)0) = 0;   ... commit SEGVicide
    nlBreak();
}

/**
 * Offset/Address/Size: 0x0 | 0x8021E0BC | size: 0xF4
 */
extern "C" void dMessage(int num, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    if (message_function)
    {
        message_function(num, msg, ap);
    }
    else
    {
        if (num)
        {
            nlPrintf("\n%s %d: ", "ODE Message", num);
        }
        else
        {
            nlPrintf("\n%s: ", "ODE Message");
        }
        nlPrintf("\n");
    }
}
