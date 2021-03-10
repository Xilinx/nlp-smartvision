#! /bin/sh

sdkdir=/wrk/paeg/users/mshaik/Documents/SOM/functional_422/xilinx-SOM-bgr/images/sdk_install
conf=${2:-Release}

unset LD_LIBRARY_PATH;
source ${sdkdir}/environment-setup-aarch64-xilinx-linux;
mkdir -p build
cd build
cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DCMAKE_BUILD_TYPE=${conf} -DCMAKE_TOOLCHAIN_FILE=${sdkdir}/sysroots/x86_64-petalinux-linux/usr/share/cmake/OEToolchainConfig.cmake ../ && make -j && make package
cd ..
