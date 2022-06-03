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
media-ctl -V '"imx219 6-0010":0 [fmt:SRGGB10_1X10/1920x1080]' -d $1
media-ctl -V '"80002000.csiss":0 [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"80002000.csiss":1  [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0000000.isp_accel":0  [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0000000.isp_accel":1  [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0040000.scaler":0  [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0040000.scaler":1  [fmt:RBG888_1X24/1024x768 field:none colorspace:srgb]' -d $1
