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
#include "version.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

const char *prog_name;

void show_error(const char *format, ...)
{
    va_list ap;
    fprintf(stderr, "%s: Error, ", prog_name);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void show_msg(const char *format, ...)
{
    va_list ap;
    fprintf(stderr, "%s: ", prog_name);
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}

void show_usage(void)
{
    printf("Usage: %s [options] <output_atr> [+attributes] <file_1> [... <file_n>]\n"
           "Options:\n"
           "\t-b\tNext file added will be loaded at boot.\n"
           "\t-x\tOutput image with exact sector count for all available content.\n"
           "\t  \tThis will use non-standard sector counts and 128 byte sector size.\n"
           "\t-s size\tSpecify the minimal image size to the given size in bytes.\n"
           "\t-B page\tRelocate the bootloader to this page address. Please, read\n"
           "\t       \tthe documentation before using this option.\n"
           "\t-h\tShow this help.\n"
           "\t-v\tShow version information.\n"
           "\n"
           "In front of each file, you can also add attributes:\n"
           "\t+h\tHidden from directory.\n"
           "\t+p\tRead-only (protected) file.\n"
           "\t+a\tArchived file.\n",
           prog_name);
    exit(EXIT_SUCCESS);
}

void show_version(void)
{
    printf("mkatr version %s, Copyright (C) 2016-2023 Daniel Serpell\n\n"
           "This is free software under the GNU GENERAL PUBLIC LICENSE.\n"
           "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
           "FOR A PARTICULAR PURPOSE.\n",
           prog_version);
    exit(EXIT_SUCCESS);
}
