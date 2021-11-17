/*
 * Copyright 2021 Xilinx Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vitis/ai/facedetect.hpp>
#include <vitis/ai/nnpp/facedetect.hpp>
#include "./process_result_facedetect.hpp"

#include <vitis/ai/yolov2.hpp>
#include <vitis/ai/nnpp/yolov2.hpp>
#include "./process_result_objectdetect.hpp"

#include <vitis/ai/platedetect.hpp>
#include <vitis/ai/nnpp/platedetect.hpp>
#include "./process_result_platedetect.hpp"

#include <glog/logging.h>
#include <iostream>
#include <semaphore.h>
#include <memory>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <glob.h>
#include <stdio.h>
#include <gst/gst.h>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>

#include "global_var.h"

static char *msgFirmware = (char *)"Please make sure that the HW accelerator firmware is loaded via xmutil loadapp kv260-nlp-smartvision.\n";

static std::string mipidev("");
static char usbdev;
static std::string mipimediadev("");

static std::string exec(const char *cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

static std::string findmipidevice(std::string s)
{
	glob_t globbuf;
	std::string dev("");
	glob("/dev/video*", 0, NULL, &globbuf);
	for (int i = 0; (unsigned)i < globbuf.gl_pathc; i++)
	{
		std::ostringstream cmd;
		cmd << "v4l2-ctl -d " << globbuf.gl_pathv[i] << " --all | grep " << s.c_str() << " | wc -l";
		std::string a = exec(cmd.str().c_str());
		a = a.substr(0, a.find("\n"));
		if (a == std::string("3"))
		{
			dev = globbuf.gl_pathv[i];
			break;
		}
	}

	globfree(&globbuf);
	std::string FindMIPIDev = dev;
	std::cout << "found mipi video node at: " << FindMIPIDev << std::endl;
	std::string mipi_dev = FindMIPIDev;
	if (mipi_dev == "")
	{
		g_printerr("ERROR: MIPI device is not ready.\n%s", msgFirmware);
		exit(EXIT_FAILURE);
	}
	if (access(mipi_dev.c_str(), F_OK) != 0)
	{
		g_printerr("ERROR: Device %s is not ready.\n%s", mipi_dev.c_str(), msgFirmware);
		exit(EXIT_FAILURE);
	}
	std::cout << "mipi device is ready" << std::endl;
	return mipi_dev;
}

static std::string findmipimedianode(std::string s)
{
	glob_t globbuf;
	std::string dev("");
	glob("/dev/media*", 0, NULL, &globbuf);
	for (int i = 0; (unsigned)i < globbuf.gl_pathc; i++)
	{
		std::ostringstream cmd1;
		cmd1 << "media-ctl -d " << globbuf.gl_pathv[i] << " -p | grep " << s.c_str() << " | wc -l";
		std::string a = exec(cmd1.str().c_str());
		a = a.substr(0, a.find("\n"));
		if (a == std::string("2"))
		{
			dev = globbuf.gl_pathv[i];
			break;
		}
	}

	globfree(&globbuf);
	std::string FindMIPIDev = dev;
	return FindMIPIDev;
}

static char findusbdevice()
{
	glob_t globbuf;
	std::string dev("");
	glob("/dev/video*", 0, NULL, &globbuf);
	for (int i = 0; (unsigned)i < globbuf.gl_pathc; i++)
	{
		std::ostringstream cmd;
		cmd << "v4l2-ctl -d " << globbuf.gl_pathv[i] << " --all | grep Driver | grep name | grep uvcvideo | wc -l";
		std::string a = exec(cmd.str().c_str());
		a = a.substr(0, a.find("\n"));
		if (a == std::string("2"))
		{
			dev = globbuf.gl_pathv[i];
			break;
		}
	}

	globfree(&globbuf);
	std::string FindMIPIDev = dev;
	std::cout << "found USB video node at: " << FindMIPIDev << std::endl;
	char mipi_dev = FindMIPIDev[10];

	std::cout << "USB device is ready at:" << mipi_dev << std::endl;
	return mipi_dev;
}

void Detection()
{
	cv::Mat image_off;
	int HSIZE, VSIZE;
	if (usb){
		HSIZE = 800;
		VSIZE = 600;
	}
	else{
		HSIZE = 1024;
		VSIZE = 768;
	}

	if (usb)
	{
		usbdev = findusbdevice();
		input.open((int)usbdev - 48);
		input.set(cv::CAP_PROP_FRAME_WIDTH, 800);
		input.set(cv::CAP_PROP_FRAME_HEIGHT, 600);
		if (!input.isOpened())
		{
			std::cerr << "ERROR: Could not open camera" << std::endl;
			return;
		}
		//double fps1 = input.get(cv::CAP_PROP_FPS);
                //std::cout << "FPS on USB Cam is:" << fps1 << std::endl;

		image_off = cv::Mat (600, 800, CV_8UC3, cv::Scalar(0, 0, 0));
	}
	else
	{
		mipidev = findmipidevice(mipi_type);
		mipimediadev = findmipimedianode(mipi_type);
		std::string tmp;
	std::cout << "Configuring ";
	if(strcmp("isp_vcap_csi", mipi_type.c_str())==0){
		std::cout << "isp ";
        	tmp = "init-isp-smartvision.sh '" + mipimediadev + "'";
	}
	else {
		std::cout << "imx ";
		tmp = "init-imx-smartvision.sh " + mipimediadev + " " + mipidev +" ";
	}
	std::cout << "node for RGB/1024x768 pipeline \n" << tmp << std::endl;
        int systemRet = system(tmp.c_str());
	if(systemRet == -1){
  	// The system method failed
		std::cout << "failed to initialize mipi with RGB/1024x768 resolution, check if nlp-smartvision firmware is loaded";
		return;
	}

		char pip[2500];
		pip[0] = '\0';
		sprintf(pip + strlen(pip), "v4l2src device=%s ! video/x-raw, width=1024, height=768 ! appsink ", mipidev.c_str());

		input.open(pip, cv::CAP_GSTREAMER);
		image_off = cv::Mat (768, 1024, CV_8UC3, cv::Scalar(0, 0, 0));
	}

	std::ostringstream desktop_cmd;
	const char* env_p = std::getenv("XDG_SESSION_TYPE");
	std::string a = "tty";
	// std::cout << "XDG_SESSION_TYPE is:" << env_p << std::endl;
	int compare = std::strcmp(env_p , a.c_str());
	// std::cout << "string compare is:" << compare << std::endl;

		std::cout << "Running remotly or Desktop environment not detected, Application will use kmssink to display video over monitor" << std::endl;
		output.open("appsrc ! kmssink driver-name=xlnx plane-id=39 fullscreen-overlay=true sync=false", cv::VideoWriter::fourcc('R', 'X', '2', '4'), 30.0, cv::Size(HSIZE, VSIZE), true);

	auto ml_task = vitis::ai::FaceDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/densebox_640_360/densebox_640_360.xmodel");
	auto ml_task_1 = vitis::ai::YOLOv2::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/yolov2_voc_pruned_0_77/yolov2_voc_pruned_0_77.xmodel");
	auto ml_task_2 = vitis::ai::PlateDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/plate_detect/plate_detect.xmodel");

	cv::Mat cur_frame;
	auto t_1 = std::chrono::steady_clock::now();
	auto t_2 = std::chrono::steady_clock::now();
	int i = 0;
	auto d_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_1).count();
	while (true)
	{
		if (fps)
		{
			t_1 = std::chrono::steady_clock::now();
		}
		input.read(cur_frame);
		if (cur_frame.empty())
		{
			std::cout << "!!! Failed to read frame from mipi media node. Please make sure the design is loaded" << std::endl;
			exit(EXIT_FAILURE);
		}
		switch (model)
		{
		case 0:
		{
			if (print_model == true)
			{
				print_model = false;
				std::cout << "Facedetect Kernel Loaded" << std::endl;
			}

			auto res = ml_task->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_facedetect(&cur_frame, res, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		case 1:
		{
			if (print_model == true)
			{
				print_model = false;
				std::cout << "Objectdetect Kernel Loaded" << std::endl;
			}
			auto res1 = ml_task_1->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_objectdetect(cur_frame, res1, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		case 2:
		{
			if (print_model == true)
			{
				print_model = false;
				std::cout << "Platedetect Kernel Loaded" << std::endl;
			}
			auto res2 = ml_task_2->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_platedetect(cur_frame, res2, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		}

		if (display_on)
			output.write(cur_frame);
		else
			output.write(image_off);

		if (fps)
		{
			i = i + 1;
			t_2 = std::chrono::steady_clock::now();
			d_milli = d_milli + std::chrono::duration_cast<std::chrono::milliseconds>(t_2 - t_1).count();
			if (i >= 90)
			{
				std::cout << "...frame rate is: " << 90000 / d_milli << std::endl;
				i = 0;
				d_milli = 0;
			}
		}
	}
}

void test_models(char *file, char *aitask)
{
	cv::Mat I = cv::imread(file);
	if (I.empty())
	{
		std::cout << "!!! Failed imread(): image not found or empty" << std::endl;
		return;
		// don't let the execution continue, else imshow() will crash.
	}

	if (!strcmp(aitask, "densebox_640_360"))
	{
		// printf(("densebox.\n\n"));
		auto ml_task = vitis::ai::FaceDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/densebox_640_360/densebox_640_360.xmodel");
		auto res = ml_task->run(I);
		process_result_facedetect(&I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else if (!strcmp(aitask, "yolov2_voc_pruned_0_77"))
	{
		// printf(("yolov2_voc_pruned_0_77.\n\n"));
		auto ml_task = vitis::ai::YOLOv2::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/yolov2_voc_pruned_0_77/yolov2_voc_pruned_0_77.xmodel");
		auto res = ml_task->run(I);
		process_result_objectdetect(I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else if (!strcmp(aitask, "plate_detect"))
	{
		// printf(("plate_detect.\n\n"));
		auto ml_task = vitis::ai::PlateDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/plate_detect/plate_detect.xmodel");
		auto res = ml_task->run(I);
		process_result_platedetect(I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else
	{
		printf(("Supported models are densebox_640_360, yolov2_voc_pruned_0_77 & plate_detect .\n\n"));
		return;
	}
	cv::imwrite("result.jpeg", I);
}
