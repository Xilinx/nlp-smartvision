for filename in /dev/media*; do
a=$(media-ctl -d $filename -p | grep driver | grep xilinx-video | wc -l)
if [ "$a" -eq "1" ]
then
 echo MIPI node is at: $filename
 echo Running MIPI-DP Pipeline with RGB format.... 
 timeout 10s gst-launch-1.0 mediasrcbin media-device=$filename ! "video/x-raw, width=1024, height=768, format=RGB, framerate=30/1" ! kmssink driver-name=xlnx plane-id=39 fullscreen-overlay=true sync=false -v
fi
done
