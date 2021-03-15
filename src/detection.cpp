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
#include <signal.h> 
#include"global_var.h"

extern cv::VideoCapture input;
extern cv::VideoWriter output;
extern pthread_t thread_id1;
extern cv::Mat cur_frame, process_frame, process_frame2;
extern sem_t sem;

void sigintHandler(int sig_num) 
{ 
    /* Reset handler to catch SIGINT next time. */
    signal(SIGINT, sigintHandler); 
    system("rm -rf /usr/share/vitis_ai_library/models 2>/dev/null");
    system("mv /usr/share/vitis_ai_library/models_backup /usr/share/vitis_ai_library/models 2>/dev/null");
    exit(1); 
} 

void Detection()
{
	system("mkdir /usr/share/vitis_ai_library 2>/dev/null");	
	system("mv /usr/share/vitis_ai_library/models /usr/share/vitis_ai_library/models_backup 2>/dev/null");
	system("ln -s /opt/xilinx/share/vitis_ai_library/models /usr/share/vitis_ai_library/ --force");
	auto ml_task = vitis::ai::FaceDetect::create("densebox_640_360");
	auto ml_task_1 = vitis::ai::YOLOv2::create("yolov2_voc_pruned_0_77");
	auto ml_task_2 = vitis::ai::PlateDetect::create("plate_detect");
	//auto ml_task_3 = vitis::ai::Segmentation8UC3::create("FPN-resnet18_Endov");
	//auto ml_task_4 = vitis::ai::RoadLine::create("vpgnet_pruned_0_99");
	//	    std:: cout << "created" << std::endl;
	//auto t_last = std::chrono::steady_clock::now();
	//auto t_now = std::chrono::steady_clock::now();
	auto t_1 = std::chrono::steady_clock::now();
	auto t_2 = std::chrono::steady_clock::now();
	signal(SIGINT, sigintHandler); 
	while (true)
	{
		t_1 = std::chrono::steady_clock::now();
		auto d_milli = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_2).count();
		t_2 = t_1;
		t_1 = std::chrono::steady_clock::now();

		//input.read(cur_frame);
		switch (model)
		{
		case 0:
		{	if (print_model == true ) {
			print_model = false;
			std:: cout << "facedetect" << std::endl;
			}

			auto res = ml_task->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_facedetect(&cur_frame, res, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
				//process_result(&process_frame, res, false, d_milli);
			}
			break;
		}
		case 1:
		{ if (print_model ==true ) {
			print_model = false;
			std:: cout << "objectdetect" << std::endl;}
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
		std:: cout << "platedetect" << std::endl;}
			auto res2 = ml_task_2->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_platedetect(cur_frame, res2, false, d_milli, bbox_thick, dleft, dright, green, blue, red);
			}
			break;
		}
/*		case 3:
		{ if (print_model ==true) {
			print_model = false;
			std:: cout << "segmentation" << std::endl;}
			auto res3 = ml_task_3->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_segmentation(cur_frame, res3, false, dleft, dright, top, bottom);
			}
			break;
		}
		case 4:
		{ //std:: cout << "lanedetect" << std::endl;
			auto res4 = ml_task_4->run(cur_frame);
			if (bbox_disp == true)
			{
				process_result_lanedetect(cur_frame, res4, false);
			}
			break;
		} */
		}
		
		//The getMat function lags the active/queued buffer by 4 frames
		//usleep(1000);
		output.write(process_frame);
		//t_2 = std::chrono::steady_clock::now();
		//auto d_milli_1 = std::chrono::duration_cast<std::chrono::milliseconds>( t_2 - t_1 ).count();
		//cout << "latency in milli sec is: " << d_milli << "frame rate is: " << 1000/d_milli << std::endl;
	}
}
