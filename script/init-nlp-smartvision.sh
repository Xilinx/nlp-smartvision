media-ctl -V '"80000000.csiss":0 [fmt:UYVY8_1X16/1024x768  field:none colorspace:srgb]' -d $1
media-ctl -V '"ap1302.4-003c":2 [fmt:UYVY8_1X16/1024x768 field:none]' -d $1
media-ctl -V '"b0100000.scaler":0  [fmt:UYVY8_1X16/1024x768  field:none colorspace:srgb]' -d $1
media-ctl -V '"b0100000.scaler":1  [fmt:RBG888_1X24/1024x768  field:none  colorspace:srgb]' -d $1
