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
    int size  = ssec * nsec;
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
    fwrite(data, ssec, nsec, f);
    fclose(f);
}

int main(int argc, char **argv)
{
    char *out = 0;
    int i;
    unsigned boot_addr = 0x07; // Standard boot address: $800
    int boot_file = 0; // Next file is boot file
    int exact_size = 0; // Use image of exact size

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
        // Brute force - try all sizes!
        for(i=6; !sfs && i<65536; i++)
            sfs = build_spartafs(128, i, boot_addr, &flist);
        if( !sfs )
            for(i=32490; !sfs && i<65536; i++)
                sfs = build_spartafs(256, i, boot_addr, &flist);
    }
    else
    {
        for(i=0; !sfs && sectors[i].size; i++)
            sfs = build_spartafs(sectors[i].size, sectors[i].num, boot_addr, &flist);
    }
    if( sfs )
        write_atr(out, sfs_get_data(sfs), sfs_get_sector_size(sfs), sfs_get_num_sectors(sfs));
    return 0;
}
