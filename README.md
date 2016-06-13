ATR disk image creation tool
----------------------------

This program creates Atari `ATR` disk images from a list of files given
in the command line, in the SpartaDOS / BW-DOS disk format.

Program Usage
-------------

    mkatr [options] output filenames

Options:

- `-h`  Shows a brief help

- `-b`  Selects the next file in the command line as the _boot_ file,
        the file will be loaded when the computer boots, and must be
        in the standard Atari binary file format.

To place files inside a sub-directory, simply add the directory *before*
all the files inside that directory.

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

