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
 * Creates an ATR with the given files as contents.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "msg.h"
#include "flist.h"
#include "disksizes.h"
#include "spartafs.h"

static void write_atr(const char *out, const char *data, int ssec, int nsec)
{
    int size = (nsec > 3) ? ssec * (nsec - 3) + 128 * 3 : 128 * nsec;
    show_msg("writing image with %d sectors of %d bytes, total %d bytes.\n",
             nsec, ssec, size);
    FILE *f = fopen(out, "wb");
    if( !f )
        show_error("can't open output file '%s': %s\n", out, strerror(errno));
    putc(0x96, f);
    putc(0x02, f);
    putc(size>>4, f);
    putc(size>>12, f);
    putc(ssec, f);
    putc(ssec>>8, f);
    putc(size>>20, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);
    putc(0, f);

    for(int i=0; i<nsec; i++)
    {
        // First three sectors are 128 bytes
        if( i < 3 )
            fwrite(data + ssec * i, 128, 1, f);
        else
            fwrite(data + ssec * i, ssec, 1, f);
    }
    if( 0 != fclose(f) )
        show_error("can't write output fil '%s': %s\n", out, strerror(errno));
}

int main(int argc, char **argv)
{
    char *out = 0;
    int i;
    unsigned boot_addr = 0x07; // Standard boot address: $800
    int boot_file = 0; // Next file is boot file
    int exact_size = 0; // Use image of exact size
    int min_size = 0; // Minimum image size

    prog_name = argv[0];

    file_list flist;
    darray_init(flist, 1);
    flist_add_main_dir(&flist);

    for(i=1; i<argc; i++)
    {
        char *arg = argv[i];
        if( arg[0] == '-' )
        {
            char op;
            while( 0 != (op = *++arg) )
            {
                if( op == 'h' || op == '?' )
                    show_usage();
                else if( op == 'b' )
                {
                    if( boot_file )
                        show_error("can specify only one boot file\n");
                    boot_file = 1;
                }
                else if( op == 'x' )
                    exact_size = 1;
                else if( op == 'B' )
                {
                    char *ep;
                    if( i+1 >= argc )
                        show_error("option '-B' needs an argument\n");
                    i++;
                    boot_addr = strtol(argv[i], &ep, 0);
                    if( boot_addr <= 3 || boot_addr >= 0xF0 || !ep || *ep )
                        show_error("argument for option '-B' must be from 3 to 240\n");
                }
                else if( op == 's' )
                {
                    char *ep;
                    if( i+1 >= argc )
                        show_error("option '-s' needs an argument\n");
                    i++;
                    min_size = strtol(argv[i], &ep, 0);
                    if( min_size <= 0 || !ep || *ep )
                        show_error("argument for option '-s' must be a positive integer\n");
                    if( min_size > 65535 * 256 )
                        show_error("maximum image size is 16776960 bytes\n");
                }
                else if( op == 'v' )
                    show_version();
                else
                    show_error("invalid command line option '-%c'. Try '%s -h' for help.\n",
                               op, prog_name);
            }
        }
        else if( !out && boot_file != 1 )
            out = arg;
        else
        {
            flist_add_file(&flist, arg, boot_file == 1);
            if( boot_file )
                boot_file = -1;
        }
    }
    if( !out )
        show_error("missing output file name. Try '%s -h' for help.\n", prog_name);

    struct sfs *sfs = 0;
    if( exact_size )
    {
        // Try biggest size and the try reducing:
        if( min_size < 128*65535 )
            sfs = build_spartafs(128, 65535, boot_addr, &flist);
        if( !sfs )
            sfs = build_spartafs(256, 65535, boot_addr, &flist);
        if( sfs )
        {
            int nsec = 65535 - sfs_get_free_sectors(sfs);
            int ssec = sfs_get_sector_size(sfs);

            if( nsec*ssec < min_size )
                nsec = (min_size + ssec - 1) / ssec;

            for(; nsec>5 && (nsec*ssec)>=min_size; nsec--)
            {
                struct sfs *n = build_spartafs(ssec, nsec, boot_addr, &flist);
                if( !n )
                    break;
                sfs_free(sfs);
                sfs = n;
                if( sfs_get_free_sectors(sfs) > 0 && (nsec-1)*ssec > min_size )
                {
                    nsec = nsec - sfs_get_free_sectors(sfs) + 1;
                    if( nsec*ssec < min_size )
                        nsec = 1 + (min_size + ssec - 1) / ssec;
                }
            }
        }
    }
    else
    {
        for(i=0; !sfs && sectors[i].size; i++)
        {
            if( sectors[i].size * sectors[i].num < min_size )
                continue;
            sfs = build_spartafs(sectors[i].size, sectors[i].num, boot_addr, &flist);
        }
    }
    if( sfs )
        write_atr(out, sfs_get_data(sfs), sfs_get_sector_size(sfs), sfs_get_num_sectors(sfs));
    else
        show_error("can't create an image big enough\n");
    return 0;
}
