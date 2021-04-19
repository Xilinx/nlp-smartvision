# Development Guide:
  
If you want to cross compile the source in Linux PC machine, follow these steps, otherwise skip this section.
   
   1) Refer to the `K260 SOM Starter Kit Tutorial` to build the cross-compilation SDK, and install it to the path you choose or default. Suppose it's SDKPATH.
   2) Run "./build.sh ${SDKPATH}" to build the somapp application.
   3) The build process in 2 will produce a rpm package kws_aa-1.0.1-1.aarch64.rpm under build/, upload to the board,
      and run "rpm -ivh --force ./nlp_smartvision-1.0.1-1.aarch64.rpm" to update install.
 
 # File structure:
 
 The application is installed as:
 
 Binary File: => /usr/bin/nlp_smartvision         main app

 Script File: => /opt/xilinx/bin/rgb_mipi-dp.sh        Initialized all media IPs and starts running RGB MIPI-DP pipeline at 1920x1080/30FPS
        
 Jupyter notebook file: => /usr/share/notebooks/nlp_smartvision/nlp_smartvision.ipynb              Jupyter notebook file for running demo.

The following file outlines the file structure and instructions on how to use
the nlp_smartvision application. This file outlines the HW requirements for the application,
how to run the application, and an overview of the application file structure. 

# Hardware and software set-up:
1. **Monitor**:
    Before booting, connect the monitor to the board via DP port.
2. **IAS sensor**:
    Before power on, install a AR1335 sensor module in J7.
3. **UART/JTAG interface**:
    For interacting and seeing boot time information connect a USB debugger to the J4.
4. **Microphone**: Connect USB microphone to one of the usb ports
5. **Network connection**:
    Connect the Ethernet cable to your local network with DHCP enabled or a direct PC connection
    with a static IP configuration.
       
# Firmware Loading
    
The accelerated application (AA) firmware consiting of bitstream, device tree overlay (dtbo), and xclbin
file are loaded dynamically on user request once Linux is fully booted. The xmutil utility can be used
for that purpose.

  1. To list the available AA applications, run:

      `xmutil listapps`

      You should see similar output to this:

>       Accelerator,             Type,   Active
>       kv260-aibox-aa2,         flat,   0
>       kv260-smartcamera-aa1,   flat,   0
>       kv260-nlp-aa7,           flat,   0

      The Active column shows which AA is currently loaded in the system. It will change to 1 after the firmware is loaded.

  2. To load the AA7 application firmware consisting of PL bitstream, device tree overlay and xclbin,
    run the following command:
    
      `xmutil loadapp kv260-nlp-aa7`

  3. After you are done running the application, you can unload the curently loaded AA application
    firmware by running:

        `xmutil unloadapp`


# Running Application
    
## Using Juypter notebook.
  Use a web-browser (e.g. Chrome, Firefox) to interact with the platform.
    1. The Jupyter notebook URL will be printed to the UART if the board is connected and allocated an IP address at boot. 
        Example: http://<board_ip_addr>:8888
    2. If using a direct connection (no DHCP) see public documentation on how to configure your PC with a static IP on the same subnet. 
        For the SOM target set the desired IP address within the same subnet using ifconfig.
        Example: ifconfig eth0 10.0.1.15 netmask 255.255.255.0
        The notebook will be available at http://<your defined IP addr>:8888
       
## Using Command line.
  Enter the following commands using the UART/debug interface. (make sure the kv260-nlp-aa7 app is already loaded using xmutils)
  
  `export LD_LIBRARY_PATH=/opt/xilinx/lib`
  
   
  `timeout 10s /opt/xilinx/bin/rgb_mipi-dp.sh`       #It will initialize all the IPs and starts running MIPI-DP pipeline at 1920x1080/30FPS.
  
  `nlp_smartvision`
  
  It will print the below task-keyword pair table and starts running facedetect. 
  Based on input from microphone, it will print the detected keyword and will perform the corresponding action.

| | |  
| - | - |
| Task | Keyword
| Start Displaying Bounding Boxes | YES
| Do Not Display Bounding Boxes | NO
| Monitor display OFF | OFF 
| Monitor display ON | ON
| Change to Next Vision Task | UP
| Change to Previous Vision Task | DOWN
| Show objects only in LEFT of the screen | LEFT
| Show objects only in RIGHT of the screen | RIGHT
| Reset to default displpay properties | STOP
| Switch the colors of box (B-> G-> R) | GO
  
            
  Note: if you get "audio open error: No such file or directory" error, run "aplay -l" and note the number assigned to USB microphone. open /etc/asound.conf and edit card number to the one assigned to USB microphone
  
  
# Details of third party sources used/modifed
| | | | | |
| - | - | - | - | - |
|Directory / File Name | Original Source | License Type | Link to License | Commit ID |
| CMSIS | [Github Link](https://github.com/ARM-software/CMSIS_5/tree/a65b7c9a3e6502127fdb80eb288d8cbdf251a6f4) | Apache License 2.0 | [Link](https://github.com/ARM-software/CMSIS_5/blob/a65b7c9a3e6502127fdb80eb288d8cbdf251a6f4/LICENSE.txt) | a65b7c9a3e6502127fdb80eb288d8cbdf251a6f4 |
| Hello_edge/src | [Github Link](https://github.com/ARM-software/ML-KWS-for-MCU/tree/8151349b110f4d1c194c085fcc5b3535bdf7ce4a) | Apache License 2.0 | [Link](https://github.com/ARM-software/ML-KWS-for-MCU/blob/8151349b110f4d1c194c085fcc5b3535bdf7ce4a/LICENSE) | 8151349b110f4d1c194c085fcc5b3535bdf7ce4a |
| src/aplay.cpp <br> src/include/aconfig.h <br> src/include/getetxt.h | [Github Link](https://github.com/alsa-project/alsa-utils/tree/b2ae0b074669f976c53a52bcd0129227321f88c9) | GNU GPL v2.0 | [Link](https://github.com/alsa-project/alsa-utils/blob/b2ae0b074669f976c53a52bcd0129227321f88c9/COPYING) | b2ae0b074669f976c53a52bcd0129227321f88c9 |
