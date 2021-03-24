#
# Copyright 2021 Xilinx Inc.
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

gst-launch-1.0 mediasrcbin media-device=/dev/media0 v4l2src0::io-mode=5 ! "video/x-raw, width=1024, height=768, format=RGB, framerate=30/1" ! kmssink driver-name=xlnx plane-id=39 fullscreen-overlay=true sync=false -v


