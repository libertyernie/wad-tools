#!/bin/sh
set -e

./x64/Debug/wadunpacker C:/InjectionArea/Sonic3DBlast.wad outdir C:/InjectionArea/common-key.bin
cd outdir
echo "1a23576eceaef0d1ac1cebb3e3ada9ad *00000000.app
8f112fff907d017f51a4a096eec67696 *00000001.app
b4a3f49e9ce69b4a7190b7e7878c8647 *00000002.app
e8d0a2f5a8a503940b8853b559829ac3 *00000003.app
40918fb084fae8be0f496fc721eb3c5b *00000004.app
ad95f75b3d519b6dfc2639c00dd58b5b *00000005.app
2b7bd73156bff464d9797e71e39f660f *00000006.app
7677ad47baa5d6e3e313e72661fbdc16 *000100014d425545.cert
04ed04edc6aafc1398c1ca4f36640f45 *000100014d425545.tik
f5c3752d837071675ec1597749890ffd *000100014d425545.tmd
1a23576eceaef0d1ac1cebb3e3ada9ad *000100014d425545.trailer" | md5sum -c -
../x64/Debug/wadpacker *.tik *.tmd *.cert ../Sonic3DBlast.wad -k C:/InjectionArea/common-key.bin
cd ..
echo "64fdf58facb88f845b9281fb30d2887a *Sonic3DBlast.wad" | md5sum -c -
rm Sonic3DBlast.wad
rm -r outdir

make clean
