wad-tools
=========

A fork of BFGR WadTools v0.39a with some small tweaks.

I couldn't find the original source code on the internet anymore, so I merged
together a couple of different versions to use as a base:

* https://github.com/asdfjkluiop/wadpacker
* https://github.com/AuroraWright/FunKii

The biggest change is the new optional arguments to wadpacker and wadunpacker.
wadunpacker can now take two additional arguments after the .wad filename: the
folder to extract to (default is derived from the .wad itself) and the path to
common-key.bin (default is to look in the current directory). wadpacker also
has a new `-k` argument that lets you specify the path to common-key.bin.

Other fixes include:

* All gcc compiler warnings taken care of
* Unused code removed
