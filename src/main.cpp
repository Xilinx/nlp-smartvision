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
#include <getopt.h>
#include <glog/logging.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <vitis/ai/demo.hpp>
#include <signal.h>
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
bool fps = false;

void Keyword_Spotting();	//Function to do kewyword spotting
void Keyword_Spotting_Debug(char *testfile); //Function to test kewyword spotting
void test_models(char *file, char* aitask);
void Detection();			//function to perform the Vision Task
void Capture_Audio();		//Function to capture audio continuously

static void usage(char *command){
	printf(("\n"
	"Usage: %s [OPTION] [arg1] [arg2]\n"
	"\n"
	"-h (or) --help                               help\n"
	"-l (or) --live-audio                         test the application with live audio input\n"
	"-f (or) --file-audio  <testing_list>.txt     test the keyword spotting with audio files listed in the .txt file\n"
	"-t (or) --test <sample_image> <model>        test the DPU with sample images. Input is Model and sample jpeg\n"
	"                                             Supported models are densebox_640_360, yolov2_voc_pruned_0_77 & plate_detect\n" 
	"-v (or) --verbose                            use along with -l to print fps and kws latency\n"
	"\n"
	  ), command);
}

void signal_callback_handler(int signum) {
   cout << "Caught signal " << signum << endl;
   // Terminate program
   exit(signum);
}

int main(int argc, char *argv[])
{	
	// Register signal and signal handler
   	signal(SIGINT, signal_callback_handler);
	char *command = argv[0];
	int option_index, c;
	static const char short_options[] = "hlf:tv";
	static const struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"live-audio", 0, 0, 'l'},
		{"file-audio", 1, 0, 'f'},
		{"test", 0, 0, 't'},
		{"verbose", 0, 0, 'v'},
		{0, 0, 0, 0}
	};
	
	if (argc == 1 || argc > 4) {
		usage(command);  	// print the usage of application
		return 1;
	}
	
	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
		switch (c) {
		case 'h':
			usage(command);			// print the usage of application
			return 1;
		case 'l':
			{
			if(argc > 2 ) {
				if ( (strcmp ("-v", argv[2]) == 0 ) || (strcmp ("--verbose", argv[2]) == 0) ){
					fps = true;
				}
				else {
					printf(("Try `%s --help' for more information.\n\n"), command); 
					usage(command);
					return 1;
				}
			}
			thread CA(Capture_Audio); 			//Start Capturing audio in live
			thread KWS(Keyword_Spotting);		//Start Detecting the keyword
			thread FD(Detection);				//Start Processing of Vision Task
			CA.join();
			KWS.join();
			FD.join();
			break;
			}
		case 'f':
			Keyword_Spotting_Debug(optarg);		//Testing of keyword spotting algorithm using .wav audio files
			break;
		case 't':
			if(argc != 4 ) { 
				printf(("Try `%s --help' for more information.\n\n"), command);
				usage(command);
				return 1;
			}
			test_models(argv[2], argv[3]);
			break;
		default:
			printf(("Try `%s --help' for more information.\n\n"), command);	
			return 1;
		}
	}
	
	return 1;
}
