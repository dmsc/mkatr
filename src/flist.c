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
 * Manages the list of files & directories
 */
#include "flist.h"
#include "msg.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static char *read_file(const char *fname, size_t size)
{
    char *data;
    FILE *f = fopen(fname, "rb");
    if( !f )
        show_error("can't open file '%s': %s\n", fname, strerror(errno));
    data = malloc(size);
    if( size != fread(data, 1, size, f) )
        show_error("error reading file '%s': %s\n", fname, strerror(errno));
    fclose(f);
    return data;
}

static char *atari_name(const char *fname)
{
    int pos = 0;

    // Convert to 8+3 filename
    char *out = strdup("           ");

    // Search last "/"
    const char *in = strrchr(fname, '/');
    if( in && !in[1] )
    {
        // Remove all '/'s at the end of filename
        while( in != fname && *in == '/' )
            in--;
        // And search the next '/' again
        while( in != fname && *in != '/' )
            in--;
        if( *in == '/' )
            in++;
    }
    else if( in )
        in++;
    else
        in = fname;

    // Copy up to 8 characters
    while( pos < 11 )
    {
        char c = *in++;
        if( !c )
            break;
        else if( c == '.' && pos < 8 )
            pos = 7;
        else if( c >= 'a' && c <= 'z' )
            out[pos] = c - 'a' + 'A';
        else if( (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
            out[pos] = c;
        else if( c == '/' )
            break;
        else
            out[pos] = '_';
        pos++;
    }
    return out;
}

static char *path_name(const char *dir, const char *name)
{
    size_t n = strlen(dir);
    char *ret = malloc(n+14);
    strcpy(ret, dir);
    ret[n++] = '>';
    int i;
    for(i=0;i<11;i++)
    {
        if( name[i] != ' ' )
        {
            if( i == 8 )
                ret[n++] = '.';
            ret[n++] = name[i];
        }
    }
    ret[n] = 0;
    return ret;
}

void flist_add_main_dir(file_list *flist)
{
    // Creates MAIN directory
    struct afile *dir = malloc(sizeof(struct afile));
    // Convert time to broken time
    time_t ttim = time(0);
    struct tm *tim = localtime(&ttim);

    dir->date[0] = tim->tm_mday;
    dir->date[1] = tim->tm_mon+1;
    dir->date[2] = tim->tm_year % 100;
    dir->time[0] = tim->tm_hour;
    dir->time[1] = tim->tm_min;
    dir->time[2] = tim->tm_sec;
    dir->fname = "";
    dir->aname = "MAIN       ";
    dir->pname = "";
    dir->dir = 0;
    dir->size = 23;
    dir->is_dir = 1;
    dir->boot_file = 0;
    dir->data = malloc(32768); // Maximum length of a directory: 32k
    dir->level = 0;

    darray_add(flist, dir);
}

void flist_add_file(file_list *flist, const char *fname, int boot_file)
{
    struct stat st;

    if( 0 != stat(fname, &st) )
        show_error("reading input file '%s': %s\n", fname, strerror(errno));

    if( S_ISREG(st.st_mode) || S_ISDIR(st.st_mode) )
    {
        struct afile *f = malloc(sizeof(struct afile));

        // Search in the file list if the path is inside an added directory
        struct afile *dir = 0, **ptr;
        darray_foreach(ptr, flist)
        {
            struct afile *af = *ptr;
            if( af->is_dir && fname == strstr(fname, af->fname) &&
                ( !dir || strlen(dir->fname) < strlen(af->fname) ) )
                dir = af;
        }

        if( !dir )
            show_error("internal error - no main directory\n");

        // Convert time to broken time
        struct tm *tim = localtime(&st.st_mtime);

        f->date[0] = tim->tm_mday;
        f->date[1] = tim->tm_mon+1;
        f->date[2] = tim->tm_year % 100;
        f->time[0] = tim->tm_hour;
        f->time[1] = tim->tm_min;
        f->time[2] = tim->tm_sec;
        f->fname = fname;
        f->aname = atari_name(fname);
        f->pname = path_name( dir->pname, f->aname);
        f->dir = dir;
        f->level = dir->level + 1;

        if( !f->aname || !strcmp(f->aname, "           ") )
            show_error("can't add file/directory named '%s'\n", fname);

        // Search for repeated files
        darray_foreach(ptr, flist)
        {
            struct afile *af = *ptr;
            if( af->dir == f->dir && !strncmp(af->aname, f->aname, 11) )
                show_error("repeated file/directory named '%s'\n", f->pname);
        }

        if( S_ISDIR(st.st_mode) )
        {
            f->size = 23;
            f->is_dir = 1;
            f->boot_file = 0;
            f->data = malloc(32768); // Maximum length of a directory: 32k

            show_msg("added dir  '%-20s', from '%s'.\n", f->pname, f->fname);
        }
        else
        {
            if( st.st_size > 0x1000000 )
                show_error("file size too big '%s'\n", fname);

            f->size = st.st_size;
            f->is_dir = 0;
            f->boot_file = boot_file;
            f->data = read_file(f->fname, f->size);

            show_msg("added file '%-20s', %5ld bytes, from '%s'%s.\n", f->pname, (long)f->size,
                    f->fname, boot_file ? " (boot)" : "");
        }
        darray_add(flist, f);
    }
    else
        show_error("invalid file type '%s'\n", fname);
}

