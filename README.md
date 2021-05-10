# Development Guide

If you want to cross compile the source in Linux PC machine, follow these steps, otherwise skip this section.

1. Refer to the `K260 SOM Starter Kit Tutorial` to build the cross-compilation SDK, and install it to the path you choose or default. Suppose it's SDKPATH.

2. Run "./build.sh ${SDKPATH}" in the source code folder of current application, to build the application. <a name="build-app"></a>

3. The build process in [2](#build-app). will produce a rpm package smartcam-1.0.1-1.aarch64.rpm under build/, upload to the board, and run `rpm -ivh --force ./nlp-smartvision-1.0.1-1.aarch64.rpm` to update install.

# Setting up the Board

1. Flash the SD Card

* Download  the [SD Card Image](https://www.xilinx.com/member/forms/download/xef.html?filename=petalinux-sdimage_0415.wic.gz) and save it on your computer.
  
* Connect the microSD to your computer.

* Download the [Balena Etcher tool](https://www.balena.io/etcher/) (recommended; available for Window, Linux, and
  macOS) required to flash the SD card.

* Follow the instructions in the tool and select the downloaded image to flash onto your microSD card.

* Eject the SD card from your computer.

    If you are looking for other OS specific tools to write the image to the SD card refer to [KV260 Getting Started Page](https://www.xilinx.com/products/som/kria/kv260-vision-starter-kit/kv260-getting-started/setting-up-the-sd-card-image.html)

2. Hardware Setup:

    * Monitor:

      Before booting, connect the monitor which supports 1024x768 resolution to the board via DP/HDMI port.
    * IAS sensor:

      Before power on, install an AR1335 sensor module in J7. Make sure there is no other camera interface connected to the setup.

    * UART/JTAG interface:

      For interacting and seeing boot-time information, connect a USB debugger to the J4.

    * USB Microphone:

      Connect the microphone to any of the USB ports.

    * Network connection:

      Connect the Ethernet cable to your local network with DHCP enabled.

3. Power on the board, and boot the Linux image.

    The Linux image will boot into the following login prompt:

    `xilinx-k26-starterkit-2020_2 login:`

    Use the `petalinux` user for login. You will be prompted to set a new password
    on the first login.

    ```bash
    xilinx-k26-starterkit-2020_2 login: petalinux
    You are required to change your password immediately (administrator enforced)
    New password:
    Retype new password:
    ```

    The `petalinux` user does not have root privileges. Most commands used in subsequent tutorials have to be run using `sudo` and you may be prompted to enter your password.

    **Note:** The root user is disabled by default due to security reasons. If you want to login as root user, follow the below steps. Use the petalinux user's password on the first password prompt, then set a new password for the root user. You can now login as root user using the newly set root user password.

    ```bash
    xilinx-k26-starterkit-2020_2:~$ sudo su -l root

    We trust you have received the usual lecture from the local System
    Administrator. It usually boils down to these three things:

        #1) Respect the privacy of others.
        #2) Think before you type.
        #3) With great power comes great responsibility.

    Password:
    root@xilinx-k26-starterkit-2020_2:~# passwd
    New password:
    Retype new password:
    passwd: password updated successfully
    ```

4. Get the latest application package.

    1. Get the list of available packages in the feed.

        `sudo xmutil getpkgs`

    2. Install the package with dnf install:

        `sudo dnf install packagegroup-kv260-nlp-smartvision.noarch`

    Note: For setups without access to the internet, it is possible to download and use the package locally. Please refer to the [Install from a local package feed](../../local_package_feed.md) for instructions.

5. Dynamically load the application package.

    1. Show the list and status of available acceleration platforms and AI Applications:

        `sudo xmutil listapps`

    2. Switch to a different platform for different AI Application:

        * When there is no active accelerator by inspecting with xmutil listapps, just active the one you want to switch.

            `sudo xmutil loadapp kv260-nlp-smartvision`

        * When there's already an accelerator being activated, unload it first, then switch to the one you want.

            `sudo xmutil unloadapp`

            `sudo xmutil loadapp kv260-nlp-smartvision`

# Run the Application

There are two ways to interact with the application.

## Jupyter notebook

  To launch Jupyter notebook on the target, run below command. Use Chrome web-browser to interact with the platform.

```bash
// Fill in ip-address from ifconfig. If the setup uses direct PC connection, use ifconfig eth0 <ip-address> to set the IP address to board
$  sudo jupyter-lab --ip=<ip-address> --allow-root --notebook-dir=/opt/xilinx/share/notebooks/ & 
```

Output example:

```bash
xilinx-SOM-multi-cc-2020_2:~$ sudo jupyter-lab --ip=ip-address --allow-root --notebook-dir=/opt/xilinx/share/notebooks/ &
[1] 1530
xilinx-SOM-multi-cc-2020_2:~$ [W 15:31:44.879 LabApp] JupyterLab server extension not enabled, manually loading...
[I 15:31:44.905 LabApp] JupyterLab extension loaded from /usr/lib/python3.7/site-packages/jupyterlab
[I 15:31:44.906 LabApp] JupyterLab application directory is /usr/share/jupyter/lab
[I 15:31:44.927 LabApp] Serving notebooks from local directory: /opt/xilinx/share/notebooks
[I 15:31:44.928 LabApp] The Jupyter Notebook is running at:
[I 15:31:44.928 LabApp] http://xxx.xx.x.xxx:8888/?token=635db1d645eeccc6a72bf1bb9c125164b1d689696348c97f
[I 15:31:44.928 LabApp]  or http://127.0.0.1:8888/?token=635db1d645eeccc6a72bf1bb9c125164b1d689696348c97f
[I 15:31:44.928 LabApp] Use Control-C to stop this server and shut down all kernels (twice to skip confirmation).
[C 15:31:44.952 LabApp]

    To access the notebook, open this file in a browser:
        file:///home/root/.local/share/jupyter/runtime/nbserver-1531-open.html
    Or copy and paste one of these URLs:
        http://xxx.xx.x.xxx:8888/?token=635db1d645eeccc6a72bf1bb9c125164b1d689696348c97f
     or http://127.0.0.1:8888/?token=635db1d645eeccc6a72bf1bb9c125164b1d689696348c97f
```

In case user has started Jupyter-lab by running another Application supported on KV260 Vision AI Starter kit, user may skip the above step and does not need to restart.
In case user closes Chrome browser and need to find URL for Jupyter lab, run below command

`$ sudo jupyter notebook list`

Output example:

> Currently running servers:
>
> `http://ip:port/?token=xxxxxxxxxxxxxxxxxx`  :: /opt/xilinx/share/notebooks

In the notebook, we will construct the GStreamer pipeline string, you can get it by adding simple python code to print it out, and played with gst-launch-1.0 command in the console, and there are some user options variables that can be changed and run with. For other parts of the pipeline, you can also change and play to see the effect easily.

## Command line

This allow the user to run "nlp-smartvision" application on CLI. These are to be executed using the UART/debug interface.

---

**NOTE**

Before running any of the commandline applications, we need to initialize the board to set media nodes and library path. Current application supports frames at 1024x768 resolution and RGB format

* Set media nodes configurations by running the below command. It will intialize the MIPI capture and DP/HDMI display pipeline. It will exit automatically after 10 sec.

```bash
init-nlp-smartvision.sh
```

---

Run the following command to launch the application for live audio input via USB microphone. The user needs to pronounce any of the ten keywords (Yes, No, Off, On, Up, Down, Left, Right, Stop, Go) after running the following command.

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision -l
```

<p align="center"> (or) </p>

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision --live-audio
```

The detected keyword will be displayed on the terminal and the corresponding action on the input video stream will be displayed on the monitor, which is connected to the board through DP/HDMI cable.

To print FPS along with the above application use -v or --verbose flag shown in the below command. The FPS is measured as average over 90 consecutive frames. Also the latency of keywords spotting + action is printed while the keyword is detected.

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision -l -v
```

> You should be able to see the video the camera is capturing on the monitor connected to the board
>
> * The application starts with facedetect. When there is a face captured by the camera, there should be a blue bounding box drawn around the face, and the box should follow the movement of the face.
> * Speak the desired keyword into the microphone, application will perform the following assigned tasks as mentioned below.
> Note: Google Command dataset has audio clips of 1 second duration. Thus, the expectation by KWS task is that one keyword is spoken within a duration of 1 second.

# File based Testing and Accuracy Measurement of KWS Only

NLP SmartVision provides a mode which is dedicated for testing the accuracy of keyword spotting (no vision task is running during this mode) on pre-recorded audio files. User needs to provide audio files along with a text file that consists of paths to the audio files which are to be tested.

The following command tests the audio files listed in the testing_list.txt file. Please refer to [Testing Accuracy on Google Command Dataset](#testing-accuracy-on-google-command-dataset) to find out how testing_list.txt is created

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision -f testing_list.txt
```

<p align="center"> (or) </p>

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision --file-audio testing_list.txt
```

## Testing Accuracy on Google Command Dataset

Users can download the open source Google’s speech command dataset for testing the application in file input mode. This dataset consists of pre-recorded audio files for 30 keywords and the audio files that are separated for testing are listed in the testing_list.txt file. Use the following commands on a linux local host machine to download and extract this dataset. These commands also create the datafiles that are required for testing the application with 10 keywords for which the model has been trained.

**Tip :** You can copy the below commands and create a single script. Then directly execute that script to do all the required steps one after the other.

```bash
mkdir Google_dataset
cd Google_dataset
wget http://download.tensorflow.org/data/speech_commands_v0.01.tar.gz
tar -xf speech_commands_v0.01.tar.gz
mkdir keywords
mv -t ./keywords/ on off up down left right yes no stop go
sed -n -e '/down\//p; /go\//p; /left\//p; /no\//p; /off\//p; /on\//p; /right\//p; /stop\//p; /yes\//p; /up\//p ' testing_list.txt > ./keywords/testing_list.txt
find . -maxdepth 1 ! -name keywords -print0|xargs -0 rm -r --
```

These commands will create a directory with the name ``Google_dataset/keywords`` inside the current working directory on your local machine. Now, all the contents inside this keywords directory needs to copied onto to the microSD card which can be done without removing the microSD from board by using scp or via a USB stick. Otherwise remove and connect the microSD card to your local machine and copy the contents and place the microSD card back and boot the board again.

**Note :** The commands may take few minutes (depending on the internet speed) to download and process the dataset.

Output after running the command for file based testing will also report the accuracy. Sample output on Google Command Dataset is shown below:

```bash
Ground truth : yes            Predicted : yes
Ground truth : yes            Predicted : yes
Ground truth : yes            Predicted : yes
=========================================
Number of keywords tested = 2552
Number of keywords detected correctly = 2383
Accuracy = 93.3777%
```

## Testing Custom Input Audio Files

The application expects audio file names to be stored as ``keyword/audio_filename.wav`` format into the audio files list file. For example, a pre-recorded audio file of keyword ‘yes’ needs to be listed as ``yes/file_001.wav``. The application uses main directory name (‘yes’ in this example) as ground truth to compare against the detected keyword. New line character must be placed after every audio file name to differentiate multiple audio files (even after the last file name).  Moreover, audio file needs to be copied to the SD card into the directory from which the application will be invoked. For example, ``/home/petalinux/keywords/yes/file_001.wav``.

The test audio files should have the following specifications:

* Sampling rate: 16 kHz
* Sample width: 16 bits per sample
* Sample encoding: Little endian
* Number of channels: 1 (mono)
* Supported format: S16_LE (PCM signed 16-bit little-endian)
* Audio Length: 1 second

# Image based Testing of DPU Only

NLP SmartVision provides a mode which is dedicated for testing the Vision models on DPU (no KWS task is running during this mode) on image files. User needs to provide image files along with the AI model thats under test

The following command tests the image files.

```bash
sudo LD_LIBRARY_PATH=/opt/xilinx/lib nlp-smartvision -t <image.jpg/image.png> <model>
```

The command returns the metadata along with a jpg fine containing bounding box on the input image

# Files structure of the application

The application is installed as:

* Binary File: => /opt/xilinx/bin

  | filename | description |
  |----------|-------------|
  | nlp-smartvision | Main application |

* Script File: => /opt/xilinx/bin/

  | filename | description |
  |----------|-------------|
  | init-nlp-smartvision.sh | Configures media nodes to run RGB - MIPI DP/HDMI Pipeline |

* Jupyter notebook file: => /opt/xilinx/share/notebooks/nlp-smartvision

  | filename | description |
  |----------|-------------|
  | nlp-smartvision.ipynb | Jupyter notebook file for nlp-smartvision demo.|

<p align="center"><sup>Copyright&copy; 2021 Xilinx</sup></p>
