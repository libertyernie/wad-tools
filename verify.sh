#!/bin/sh
set -e
make
rm -r 000100014d425545 || true
cp /cygdrive/c/InjectionArea/common-key.bin .
./wadunpacker /cygdrive/c/InjectionArea/Sonic3DBlast.wad
cd 0001*
mv ../common-key.bin .
../wadpacker *.tik *.tmd *.cert ../Sonic3DBlast.wad
cd ..
echo "64fdf58facb88f845b9281fb30d2887a *Sonic3DBlast.wad" | md5sum -c -
rm Sonic3DBlast.wad 0001*/common-key.bin
rm -r 000100014d425545
make clean
