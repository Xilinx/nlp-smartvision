#!/bin/bash
# Copyright 2022 Xilinx Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
sdkdir=${1}

conf=${2:-Release}

unset LD_LIBRARY_PATH;
source ${sdkdir}/environment-setup-cortexa72-cortexa53-xilinx-linux;

mkdir -p build
cd build
cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DENABLE_PRECOMPILED_HEADERS=OFF -D CMAKE_NO_SYSTEM_FROM_IMPORTED=ON -DCMAKE_BUILD_TYPE=${conf} -DCMAKE_TOOLCHAIN_FILE=${sdkdir}/sysroots/x86_64-petalinux-linux/usr/share/cmake/OEToolchainConfig.cmake ../ && make -j && make package
cd ..
