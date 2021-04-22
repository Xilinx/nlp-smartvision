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

#include <vitis/ai/segmentation.hpp>
#include <vitis/ai/nnpp/segmentation.hpp>
#include "./process_result_segmentation.hpp"

#include <vitis/ai/platedetect.hpp>
#include <vitis/ai/nnpp/platedetect.hpp>
#include "./process_result_platedetect.hpp"

#include <glog/logging.h>
#include <iostream>
#include<semaphore.h>
#include <memory>
#include <thread>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include"global_var.h"
//Initialisation of capture, drm, ml and thread related
#define HSIZE 1024
#define VSIZE 768
/* 
cv::VideoCapture input;
cv::VideoWriter output; 
*/

cv::Mat cur_frame;

void Detection()
{
	cv::VideoCapture input("v4l2src ! video/x-raw, width=1024, height=768 ! appsink", cv::CAP_GSTREAMER);
	cv::VideoWriter output("appsrc ! kmssink driver-name=xlnx plane-id=39 fullscreen-overlay=true sync=false -v", cv::VideoWriter::fourcc('R', 'X', '2', '4'), 30.0, cv::Size(HSIZE,VSIZE), true);

	auto ml_task = vitis::ai::FaceDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/densebox_640_360/densebox_640_360.xmodel");
	auto ml_task_1 = vitis::ai::YOLOv2::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/yolov2_voc_pruned_0_77/yolov2_voc_pruned_0_77.xmodel");
	auto ml_task_2 = vitis::ai::PlateDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/plate_detect/plate_detect.xmodel");
	auto t_1 = std::chrono::steady_clock::now();
	auto t_2 = std::chrono::steady_clock::now();
	int i = 0;
	auto d_milli = std::chrono::duration_cast<std::chrono::milliseconds>( t_1 - t_1 ).count();
	while (true)
	{
		if (fps) {
		t_1 = std::chrono::steady_clock::now();
		}
		input.read(cur_frame);
		if (cur_frame.empty())
		{
			std::cout << "!!! Failed to read frame. please run rgb-mipi-dp.sh to initialize videopipeline" << std::endl;
			return;
			// don't let the execution continue, else application may crash.
		}
		switch (model)
		{
		case 0:
		{	if (print_model == true ) {
			print_model = false;
			std:: cout << "Facedetect Kernel Loaded" << std::endl;
			}

			auto res = ml_task->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_facedetect(&cur_frame, res, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		case 1:
		{ if (print_model ==true ) {
			print_model = false;
			std:: cout << "Objectdetect Kernel Loaded" << std::endl;}
			auto res1 = ml_task_1->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_objectdetect(cur_frame, res1, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		case 2:
		{ if (print_model ==true ) {
		print_model = false;
		std:: cout << "Platedetect Kernel Loaded" << std::endl;}
			auto res2 = ml_task_2->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_platedetect(cur_frame, res2, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
		}
		
		//The getMat function lags the active/queued buffer by 4 frames
		output.write(cur_frame);
		if (fps) {
		i = i + 1;
		t_2 = std::chrono::steady_clock::now();
		d_milli = d_milli + std::chrono::duration_cast<std::chrono::milliseconds>( t_2 - t_1 ).count();
		if (i >= 10){
			std::cout << "...frame rate is: " << 10000/d_milli << std::endl;
			i = 0;
			d_milli = 0;
		}
		}
	}
}

void test_models(char *file, char* aitask){
	cv::Mat I = cv::imread(file);
	if (I.empty())
	{
		std::cout << "!!! Failed imread(): image not found or empty" << std::endl;
		return;
		// don't let the execution continue, else imshow() will crash.
	}
	
	if(!strcmp(aitask, "densebox_640_360")) {
		// printf(("densebox.\n\n"));
		auto ml_task = vitis::ai::FaceDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/densebox_640_360/densebox_640_360.xmodel");
		auto res = ml_task->run(I);
		process_result_facedetect(&I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else if (!strcmp(aitask, "yolov2_voc_pruned_0_77")) {
		// printf(("yolov2_voc_pruned_0_77.\n\n"));
		auto ml_task = vitis::ai::YOLOv2::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/yolov2_voc_pruned_0_77/yolov2_voc_pruned_0_77.xmodel");
		auto res = ml_task->run(I);
		process_result_objectdetect(I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else if (!strcmp(aitask, "plate_detect")) {
		// printf(("plate_detect.\n\n"));
		auto ml_task = vitis::ai::PlateDetect::create("/opt/xilinx/share/vitis_ai_library/models/kv260-nlp-smartvision/plate_detect/plate_detect.xmodel");
		auto res = ml_task->run(I);
		process_result_platedetect(I, res, true, 0, bbox_thick, dleft, dright, green, blue, red);
	}
	else {
		printf(("Supported models are densebox_640_360, yolov2_voc_pruned_0_77 & plate_detect .\n\n"));
		return;
	}
	cv::imwrite("result.jpeg", I);
}
