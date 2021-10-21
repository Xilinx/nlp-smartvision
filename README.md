## Running the NLP application:
* To run the application, Use the below commands
   ``` /bin/bash
   xlnx-nlp-smartvision.nlp-smartvision -m # for MIPI as video source 
   ```
  
   ``` /bin/bash
   xlnx-nlp-smartvision.nlp-smartvision -u # for USB as video source 
   ```
* The application automatically switches between ximagesink and kmssink as mentioned below
	* **ximagesink**: If launched via Gnome Desktop Environments.
	* **kmssink**: If the Desktop Environment is disabled and application launched via SSH/Serial-COM
	

* Below is the full usage of the application

``` \bin\bash
Usage: /snap/xlnx-nlp-smartvision/x1/bin/nlp-smartvision [OPTION] [arg1] [arg2]

-h (or) --help                                  help
-m (or) --mipi                                  test the application with live video from mipi camera
-u (or) --usb                                   test the application with live video from USB camera
-f (or) --file-audio  <testing_list>.txt        test the keyword spotting with audio files listed in the .txt file
-t (or) --test <sample_image> <model>           test the DPU with sample images. Input is Model and sample jpeg
                                                Supported models are densebox_640_360, yolov2_voc_pruned_0_77 & plate_detect
-v (or) --verbose                               use along with -m or -u to print fps and kws latency

```
	
