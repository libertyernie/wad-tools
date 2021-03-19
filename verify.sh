#!/bin/sh
set -e
make
rm -r 000100014d425545 || true
cp /cygdrive/c/InjectionArea/common-key.bin .
./wadunpacker /cygdrive/c/InjectionArea/Sonic3DBlast.wad
cd 000100014d425545
mv ../common-key.bin .
../wadpacker *.tik *.tmd *.cert ../Sonic3DBlast.wad
cd ..
echo "64fdf58facb88f845b9281fb30d2887a *Sonic3DBlast.wad" | md5sum -c -
rm Sonic3DBlast.wad 000100014d425545/common-key.bin
rm -r 000100014d425545

./wadunpacker /cygdrive/c/InjectionArea/Sonic3DBlast.wad outdir /cygdrive/c/InjectionArea/common-key.bin
cd outdir
../wadpacker *.tik *.tmd *.cert ../Sonic3DBlast.wad -k /cygdrive/c/InjectionArea/common-key.bin
cd ..
echo "64fdf58facb88f845b9281fb30d2887a *Sonic3DBlast.wad" | md5sum -c -
rm Sonic3DBlast.wad
rm -r outdir

make clean
