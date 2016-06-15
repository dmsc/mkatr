ATR disk image creation tool
----------------------------

This program creates Atari `ATR` disk images from a list of files given
in the command line, in the SpartaDOS / BW-DOS disk format.

Program Usage
-------------

    mkatr [options] output filenames

Options:

- `-b`  Selects the next file in the command line as the _boot_ file,
        the file will be loaded when the computer boots, and must be
        in the standard Atari binary file format.

- `-h`  Shows a brief help.

- `-v`  Shows version information.

To place files inside a sub-directory, simply add the directory *before*
all the files inside that directory.

The resulting image will be the smaller size that fits all the given files,
from the following list:

| Sector Count | Sector Size | Total Size | Name                     |
|         ---: |        ---: |       ---: | :---                     |
|       720    |       128   |      90k   | SD                       |
|      1040    |       128   |     130k   | ED                       |
|       720    |       256   |     180k   | DD                       |
|      1440    |       256   |     360k   | DSDD                     |
|      2048    |       256   |     512k   | hard disk                |
|      4096    |       256   |       1M   | hard disk                |
|      8192    |       256   |       2M   | hard disk                |
|     16384    |       256   |       4M   | hard disk                |
|     32768    |       256   |       8M   | hard disk                |
|     65536    |       256   |      16M   | biggest possible image   |

Usage Examples
--------------

To create an image named `disk1.atr` with all the files in the "atari"
directory:

    mkatr disk1.atr atari/*

To create a bootable BW-DOS image with the DOS files inside a
subdirectory and a startup.bat file outside:

    mkatr bwdos.atr dos/ -b dos/xbw130.dos startup.bat

To create an image with only one file that will be loaded at boot:

    mkatr game.atr -b mygame.com


Compilation
-----------

Compile with `make` and copy the resulting `mkatr` program to your bin
folder.

