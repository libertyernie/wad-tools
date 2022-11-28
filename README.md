wad-tools
=========

A fork of BFGR WadTools v0.39a with some small tweaks.

The biggest change is the new optional arguments to wadpacker and wadunpacker.
wadunpacker can now take two additional arguments after the .wad filename: the
folder to extract to (default is derived from the .wad itself) and the path to
common-key.bin (default is to look in the current directory). wadpacker also
has a new `-k` argument that lets you specify the path to common-key.bin.

Other fixes include:

* All gcc compiler warnings taken care of
* Unused code removed
* Code can now compile as C++
* Code can now compile as C++/CLI, which avoids the openssl dependency by using .NET
