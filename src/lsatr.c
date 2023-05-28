/*
 *  Copyright (C) 2023 Daniel Serpell
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
 * Loads an ATR with a SpartaDOS file-system and list contents.
 */
#define _GNU_SOURCE
#include "atr.h"
#include "msg.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------
// Global options
static int atari_list;
static int lower_case;

//---------------------------------------------------------------------
static void show_usage(void)
{
    printf("Usage: %s [options] <atr_image_file>\n"
           "Options:\n"
           "\t-a\tShow listing in Atari instead of UNIX format.\n"
           "\t-l\tConvert filenames to lower-case.\n"
           "\t-h\tShow this help.\n"
           "\t-v\tShow version information.\n",
           prog_name);
    exit(EXIT_SUCCESS);
}

//---------------------------------------------------------------------
static uint16_t read16(const uint8_t *p)
{
    return p[0] | (p[1] << 8);
}

static unsigned read24(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16);
}

// Read up to size bytes from file at given map sector
static unsigned read_file(struct atr_image *atr, unsigned map, unsigned size,
                          uint8_t *data)
{
    if( map < 2 || map > atr->sec_count )
    {
        show_msg("invalid sector map");
        return 0;
    }
    const uint8_t *m = atr->data + atr->sec_size * (map - 1);
    unsigned s       = 4;
    unsigned pos     = 0;

    while( size )
    {
        if( s >= atr->sec_size )
        {
            // next map
            map = read16(m);
            if( !map )
                return pos;
            if( map < 2 || map > atr->sec_count )
            {
                show_msg("invalid next sector map");
                return pos;
            }
            m = atr->data + atr->sec_size * (map - 1);
            s = 4;
        }
        unsigned rem = size > atr->sec_size ? atr->sec_size : size;
        unsigned sec = read16(m + s);
        s += 2;
        if( !sec )
            memset(data + pos, 0, rem);
        else if( sec < 2 || sec > atr->sec_count )
        {
            show_msg("invalid data sector");
            return pos;
        }
        else
            memcpy(data + pos, atr->data + atr->sec_size * (sec - 1), rem);
        pos += rem;
        size -= rem;
    }
    return pos;
}

// Read data and write a UNIX filename and an "Atari" filename.
static unsigned get_name(char *name, char *aname, const uint8_t *data)
{
    unsigned l = 0;
    int dot    = 0;
    memset(aname, ' ', 12);
    aname[12] = 0;
    for( int i = 0; i < 11; i++ )
    {
        uint8_t c = data[i];
        if( c >= 'A' && c <= 'Z' && lower_case )
            c = c - 'A' + 'a';
        if( c < ' ' || c == '/' || c == '.' || c == '?' || c == '\\' || c == 96 ||
            c > 'z' )
            c = '_';
        else if( c == ' ' )
            continue;
        if( i > 7 && !dot )
        {
            dot       = 1;
            name[l++] = '.';
        }
        name[l++]      = c;
        aname[i + dot] = c;
    }
    name[l] = 0;
    return l;
}

static void read_dir(struct atr_image *atr, unsigned map, const char *name)
{
    if( atari_list )
        printf("Directory of %s\n\n", *name ? name : "/");

    uint8_t *data = malloc(65536); // max directory size (2848 entries)
    unsigned len  = read_file(atr, map, 65536, data);
    if( !len )
    {
        show_msg("%s: can´t get directory data", name);
        free(data);
        return;
    }
    else if( len == 65536 )
        show_msg("%s: directory too big", name);

    // traverse dir
    for( unsigned i = 23; i < len; i += 23 )
    {
        unsigned flags = data[i];
        if( !flags )
            break; // no more entries
        if( 0 == (flags & 0x08) )
            continue; // unused
        if( 0x10 == (flags & 0x10) )
            continue; // erased
        int is_dir     = flags & 0x20;
        unsigned fmap  = read16(data + i + 1);
        unsigned fsize = read24(data + i + 3);
        int fd_day     = data[i + 17];
        int fd_mon     = data[i + 18];
        int fd_yea     = data[i + 19];
        int ft_hh      = data[i + 20];
        int ft_mm      = data[i + 21];
        int ft_ss      = data[i + 22];
        char fname[32], aname[32];
        if( !get_name(fname, aname, data + i + 6) )
        {
            show_msg("%s: invalid file name, skip", name);
            continue;
        }
        char *new_name;
        asprintf(&new_name, "%s/%s", name, fname);
        if( is_dir )
        {
            if( atari_list )
            {
                // Print entry, but don´t recurse
                printf("%-12s  <DIR>  %02d-%02d-%02d %02d:%02d\n", aname, fd_day, fd_mon,
                       fd_yea, ft_hh, ft_mm);
            }
            else
            {
                printf("%8u\t%02d-%02d-%02d %02d:%02d:%02d\t%s/\n", 0, fd_day, fd_mon,
                       fd_yea, ft_hh, ft_mm, ft_ss, new_name);
                read_dir(atr, fmap, new_name);
            }
        }
        else
        {
            uint8_t *fdata = malloc(fsize);
            unsigned r     = read_file(atr, fmap, fsize, fdata);
            if( r != fsize )
                show_msg("%s: short file in disk", new_name);
            if( atari_list )
                printf("%-12s %7u %02d-%02d-%02d %02d:%02d\n", aname, fsize, fd_day,
                       fd_mon, fd_yea, ft_hh, ft_mm);
            else
                printf("%8u\t%02d-%02d-%02d %02d:%02d:%02d\t%s\n", fsize, fd_day, fd_mon,
                       fd_yea, ft_hh, ft_mm, ft_ss, new_name);
            free(fdata);
        }
        free(new_name);
    }
    // traverse dir again if listing in Atari format, to show sub directories
    if( atari_list )
    {
        printf("\n");
        for( unsigned i = 23; i < len; i += 23 )
        {
            unsigned flags = data[i];
            if( !flags )
                break; // no more entries
            if( 0 == (flags & 0x08) )
                continue; // unused
            if( 0x10 == (flags & 0x10) )
                continue; // erased
            if( 0 == (flags & 0x20) )
                continue; // not directory
            unsigned fmap  = read16(data + i + 1);
            char fname[32], aname[32];
            if( !get_name(fname, aname, data + i + 6) )
                continue;
            char *new_name;
            asprintf(&new_name, "%s/%s", name, fname);
            read_dir(atr, fmap, new_name);
            free(new_name);
        }
    }
    free(data);
}

int main(int argc, char **argv)
{
    const char *atr_name = 0;
    lower_case           = 0;
    atari_list           = 0;
    prog_name            = argv[0];
    for( int i = 1; i < argc; i++ )
    {
        char *arg = argv[i];
        if( arg[0] == '-' )
        {
            char op;
            while( 0 != (op = *++arg) )
            {
                if( op == 'h' || op == '?' )
                    show_usage();
                else if( op == 'l' )
                    lower_case = 1;
                else if( op == 'a' )
                    atari_list = 1;
                else if( op == 'v' )
                    show_version();
                else
                    show_opt_error("invalid command line option '-%c'", op);
            }
        }
        else if( !atr_name )
            atr_name = arg;
        else
            show_opt_error("multiple ATR files in command line");
    }
    if( !atr_name )
        show_opt_error("ATR file name expected");

    // Load ATR image file
    struct atr_image *atr = load_atr_image(atr_name);

    // Check SFS filesystem
    if( atr->sec_count < 6 )
        show_error("%s: ATR image with too few sectors.", atr_name);
    // Read superblock
    unsigned signature   = atr->data[7];
    unsigned rootdir_map = read16(atr->data + 9);
    unsigned num_sect    = read16(atr->data + 11);
    unsigned bitmap_sect = read16(atr->data + 16);
    unsigned sector_size = atr->data[31] ? atr->data[31] : 256;

    if( signature != 0x80 )
        show_error("%s: ATR image does not holds a SpartaDOS file system.", atr_name);
    if( sector_size != atr->sec_size )
        show_error("%s: invalid SpartaDOS file system, mismatch sector sizes (%d!=%d).",
                   atr_name, sector_size, atr->sec_size);
    if( num_sect < atr->sec_count )
        show_msg("%s: ATR image is bigger than file system.", atr_name);
    if( num_sect > atr->sec_count )
        show_msg("%s: WARNING: ATR image is smaller than file system.", atr_name);
    if( rootdir_map < 2 || rootdir_map > atr->sec_count )
        show_error("%s: invalid SpartaDOS file system, root dir map outside disk.",
                   atr_name);
    if( bitmap_sect < 2 || bitmap_sect > atr->sec_count )
        show_error("%s: invalid SpartaDOS file system, bitmap outside disk.", atr_name);

    printf("%s: %u sectors of %u bytes.\n", atr_name, atr->sec_count, atr->sec_size);
    read_dir(atr, rootdir_map, "");
    atr_free(atr);

    return 0;
}
