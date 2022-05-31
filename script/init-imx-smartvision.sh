media-ctl -V '"imx219 6-0010":0 [fmt:SRGGB10_1X10/1920x1080]' -d $1
media-ctl -V '"80002000.csiss":0 [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"80002000.csiss":1  [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0000000.isp_accel":0  [fmt:SRGGB10_1X10/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0000000.isp_accel":1  [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0040000.scaler":0  [fmt:RBG888_1X24/1920x1080 field:none colorspace:srgb]' -d $1
media-ctl -V '"b0040000.scaler":1  [fmt:RBG888_1X24/1024x768 field:none colorspace:srgb]' -d $1
