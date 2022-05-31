#
#  Copyright 2021 Xilinx Inc.
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
# 
#      http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

cmake_minimum_required(VERSION 3.5)

project(nlp-smartvision)

SET(CMAKE_INSTALL_PREFIX $ENV{OECORE_TARGET_SYSROOT})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_XOPEN2K8 -DENABLE_PRECOMPILED_HEADERS=OFF -g -Wall -O2 -fpermissive -fmessage-length=0 -rdynamic -Wl,--no-undefined -Wno-narrowing" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I ${CMAKE_SOURCE_DIR}/src/")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(GStreamer REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(alsa-lib REQUIRED)

SET(INSTALL_PATH "opt/xilinx")


# Add ARM CMSIS shared library (ARM-DSP and ARM-NN used by hello_edge implementation)
file(GLOB_RECURSE SOURCES_CMSIS RELATIVE ${CMAKE_SOURCE_DIR} "CMSIS/*.c")
add_library(CMSIS SHARED ${SOURCES_CMSIS})
target_include_directories(CMSIS PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/Core/Include)
target_include_directories(CMSIS PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/Include)
target_include_directories(CMSIS PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/NN/Include)
target_include_directories(CMSIS PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/PrivateInclude)
install(TARGETS CMSIS DESTINATION ${INSTALL_PATH}/lib)


# Add hello_edge shared library (run the hello_edge model for keyword spotting)
file(GLOB_RECURSE SOURCES_HELLOEDGE RELATIVE ${CMAKE_SOURCE_DIR} "Hello_edge/*.cpp")
add_library(hello_edge SHARED ${SOURCES_HELLOEDGE})
target_link_libraries(hello_edge CMSIS)
target_include_directories(hello_edge PRIVATE ${CMAKE_SOURCE_DIR}/Hello_edge/src)
target_include_directories(hello_edge PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/Core/Include)
target_include_directories(hello_edge PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/Include)
target_include_directories(hello_edge PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/NN/Include)
target_include_directories(hello_edge PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/PrivateInclude)
install(TARGETS hello_edge DESTINATION ${INSTALL_PATH}/lib)


# build the executable 
add_executable(nlp-smartvision.app src/main.cpp src/detection.cpp src/keyword_spotting.cpp src/aplay.cpp)
target_include_directories(nlp-smartvision.app PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(nlp-smartvision.app PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/acapture/src)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/src/Include)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/Hello_edge/src)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/Core/Include)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/Include)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/NN/Include)
target_include_directories(nlp-smartvision.app PRIVATE ${CMAKE_SOURCE_DIR}/CMSIS/DSP/PrivateInclude)
target_link_directories(nlp-smartvision.app PRIVATE ${INSTALL_PATH}/lib)

include_directories(${CMAKE_SOURCE_DIR}/script)


target_link_libraries(nlp-smartvision.app
  xilinxopencl pthread rt dl crypt stdc++ asound CMSIS hello_edge opencv_core opencv_video opencv_videoio opencv_imgproc opencv_imgcodecs opencv_highgui vitis_ai_library-yolov2 vitis_ai_library-facedetect vitis_ai_library-platedetect vitis_ai_library-lanedetect vitis_ai_library-segmentation glog gstapp-1.0 gstreamer-1.0 gstbase-1.0 gobject-2.0 glib-2.0 gstvideo-1.0 gstallocators-1.0 
  glib-2.0 gobject-2.0)
install(TARGETS nlp-smartvision.app
	DESTINATION ${INSTALL_PATH}/bin)

install(PROGRAMS script/nlp-smartvision 
	DESTINATION ${INSTALL_PATH}/bin)

install(PROGRAMS
    script/init-isp-smartvision.sh
    script/init-imx-smartvision.sh
    script/nlp-smartvision-install.py
    DESTINATION ${INSTALL_PATH}/bin)

install(FILES
    README.md
    DESTINATION ${INSTALL_PATH}/
    RENAME  README_NLP_SMARTVISION
    )

install(DIRECTORY
    notebook/
    DESTINATION ${INSTALL_PATH}/share/notebooks/nlp-smartvision/)

set(VERSION "1.0.1")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "nlp-smartvision")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "mshaik@xilinx.com")
set(CPACK_PACKAGE_VENDOR "xilinx")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
include(CPack)

