#
# Copyright 2019 Xilinx Inc.
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

cmake_minimum_required(VERSION 3.5)

project(kws_aa)

SET(CMAKE_INSTALL_PREFIX $ENV{OECORE_TARGET_SYSROOT})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_XOPEN2K8 -g -Wall -O2 -fpermissive -fmessage-length=0 -rdynamic -Wl,--no-undefined" )
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I ${CMAKE_SOURCE_DIR}/src/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(GStreamer REQUIRED)

SET(INSTALL_PATH "opt/xilinx")

add_executable(kws_aa src/main.cpp src/kws_ds_cnn.cpp src/ds_cnn.cpp src/kws.cpp src/mfcc.cpp src/nn.cpp)
target_include_directories(kws_aa PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(kws_aa
  gstapp-1.0 gstreamer-1.0 gstbase-1.0 gobject-2.0 glib-2.0 gstvideo-1.0 gstallocators-1.0 gstrtsp-1.0 gstrtspserver-1.0
  glib-2.0 gobject-2.0 xilinxopencl pthread rt dl crypt stdc++ asound libCMSIS opencv_core opencv_video opencv_videoio opencv_imgproc opencv_imgcodecs opencv_highgui vitis_ai_library-yolov2 vitis_ai_library-facedetect vitis_ai_library-platedetect vitis_ai_library-lanedetect vitis_ai_library-segmentation glog )
install(TARGETS kws_aa DESTINATION usr/bin)


install(PROGRAMS
    script/liblibCMSIS.so
    DESTINATION usr/lib)

install(PROGRAMS
    script/rgb_mipi-dp.sh
    DESTINATION ${INSTALL_PATH}/bin)

install(PROGRAMS
    script/asound.conf
    DESTINATION etc)

install(FILES
    README
    DESTINATION ${INSTALL_PATH}/
    RENAME      README_KWS
    )

#install(DIRECTORY
#    config/facedetect
#    config/refinedet
#    config/ssd
#    DESTINATION ${INSTALL_PATH}/share/smartcam_aa1/)

#install(FILES
#    config/ssd/label.json
#    DESTINATION usr/share/vitis_ai_library/models/ssd_adas_pruned_0_95/
#    )

install(DIRECTORY
    notebook/
    DESTINATION usr/share/notebooks/kws_aa/)

set(VERSION "1.0.1")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "kws_aa")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "mshaik@xilinx.com")
set(CPACK_PACKAGE_VENDOR "xilinx")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
include(CPack)


