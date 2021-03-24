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

#include <stdio.h>
#include <bits/stdc++.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <pthread.h>
#include <semaphore.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vitis/ai/demo.hpp>
#include"global_var.h"

using namespace std;
using namespace std::chrono;



int window_size = 341;		// Window size supported by PCM sound card
int ddr_buffer = 15000;		//To allocate memory that can store 15000 audio windows each of size 341
short *InputData = (short *)malloc(ddr_buffer * window_size * sizeof(short)); // Memory to store audio input data coming from mike
bool *Record_flag = (bool *)calloc(ddr_buffer, sizeof(bool));		// Flags to set when each window is filled up with audio data

bool bbox_disp = true;  	//default bounding box display is ON
int model = 0;				//default model number is 0 = facedtect
int num_model = 3;			//Total number of models	
bool print_model = true;	
int bbox_thick = 2;			//Set Bounding Box thickness
bool dleft = false;			//Default state to process output for complete screen; Left=False
bool dright = false;		//Default state to process output for complete screen; Right=False
int red = 255;				
int green = 0;
int blue = 0;
bool top = false;			//Default state to process output for complete screen; top=False
bool bottom = false;		//Default state to process output for complete screen; bottom=False


void Keyword_Spotting();	//Function to do kewyword spotting
void Detection();			//function to perform the Vision Task
void Capture_Audio();		//Function to capture audio continuously

int main()
{
		#ifdef ENABLE_NLS
		setlocale(LC_ALL, "");
		textdomain(PACKAGE);
	#endif

	thread CA(Capture_Audio); 			//Start Capturing audio data
	thread KWS(Keyword_Spotting);		//Start Detecting the keyword
	thread FD(Detection);				//Start Processing of Vision Task
	
	CA.join();
	KWS.join();
	FD.join();

	return 0;
}
