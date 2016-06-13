/*
 *  Copyright (C) 2016 Daniel Serpell
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program.  If not, see <http://www.gnu.org/licenses/>
 */
/*
 * Shows error messages.
 */
#include "msg.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

const char *prog_name;

void show_error(const char *format, ...)
{
    va_list ap;
    printf("%s: Error, ", prog_name);
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

void show_msg(const char *format, ...)
{
    va_list ap;
    printf("%s: ", prog_name);
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}

void show_usage(void)
{
    printf("Usage: %s [options] <output_atr> <file_1> [... <file_n>]\n"
           "Options:\n"
           "\t-b\tNext file added will be loaded at boot.\n"
           "\t-h\tShow this help.\n",
           prog_name);
    exit(EXIT_SUCCESS);
}

