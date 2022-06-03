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
media-ctl -V '"80000000.csiss":0 [fmt:UYVY8_1X16/1024x768  field:none colorspace:srgb]' -d $1
media-ctl -V '"ap1302.4-003c":2 [fmt:UYVY8_1X16/1024x768 field:none]' -d $1
media-ctl -V '"b00c0000.scaler":0  [fmt:UYVY8_1X16/1024x768  field:none colorspace:srgb]' -d $1
media-ctl -V '"b00c0000.scaler":1  [fmt:RBG888_1X24/1024x768  field:none  colorspace:srgb]' -d $1
