#! /bin/sh

sdkdir=${1}

conf=${2:-Release}

unset LD_LIBRARY_PATH;
source ${sdkdir}/environment-setup-cortexa72-cortexa53-xilinx-linux;

mkdir -p build
cd build
cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DENABLE_PRECOMPILED_HEADERS=OFF -D CMAKE_NO_SYSTEM_FROM_IMPORTED=ON -DCMAKE_BUILD_TYPE=${conf} -DCMAKE_TOOLCHAIN_FILE=${sdkdir}/sysroots/x86_64-petalinux-linux/usr/share/cmake/OEToolchainConfig.cmake ../ && make -j && make package
cd ..
