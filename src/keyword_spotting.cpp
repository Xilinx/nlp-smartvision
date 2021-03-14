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
 
#include "kws_ds_cnn.h"
#include<thread>
#include<memory>
#include<pthread.h>
#include <semaphore.h>
#include <glog/logging.h>
#include <chrono>
#include <stdio.h>
#include <bits/stdc++.h>
#include"global_var.h"

using namespace std;
using namespace std::chrono;

void Keyword_Spotting()
{
	char command[256];
	//char output_class[12][8] = {"Silence", "Unknown", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"};
    float window_energy=0;
	float window[6];
	int wpos = 5;
    float silence_threshold = (float)(pow(2,30)*window_size*6*15*pow(10,-5));    //Total energy of 6 consecutive audio sample windows, which is less than this is a silence
    int Voice_start = 0;
    int Voice_end = 0;
    int sample_count = 0;   
    int one_count = 0;
    int flag_pos = 0;
	short InputChunk[16000]; // Audio Input samples are in signed int16 precision

	while(1){
		
		for(int j=0; j<5; j++){				// Calculation of energy for first 5 audio windows
			while(1){
				if(Record_flag[flag_pos]==1){		// Checking whether this audio window is filled with samples or not
					window[j]=0;
					for(int i=0; i<window_size; i++){
						window[j] +=  (float)pow(InputData[sample_count+i],2);}		// Energy of audio window
					Record_flag[flag_pos] = 0;			
					flag_pos++;
					if(flag_pos>=(ddr_buffer)){		// If all the windows are completed, then back to 0
						flag_pos = 0;			
					}
					break;
				}
				this_thread::sleep_for(chrono::milliseconds(5) );	// If the audio is not captured, then wait for 5 milli sec
			}
			sample_count = flag_pos*window_size;
		}

		wpos=5;
		one_count = 0;

		while(1){
			window[wpos] = 0;
			while(1){
				if(Record_flag[flag_pos]==1){
					for(int i=0; i<window_size; i++)
						window[wpos] +=  (float)pow(InputData[sample_count+i],2);		// Energy of 6th audio window. It will be stored into window[wpos] in a cyclic manner
					Record_flag[flag_pos] = 0;
					flag_pos++;
					if(flag_pos>=(ddr_buffer)){
						flag_pos = 0;			
					}
					break;
				}
				this_thread::sleep_for(chrono::milliseconds(5) );	
			}
			sample_count = flag_pos*window_size;
			wpos++;
			if(wpos>5)
				wpos=0;			// window[wpos] becomes cyclic between 0-5 (six windows)
			
			window_energy=0;
			for(int i=0; i<6; i++)
				window_energy += window[i];		// Total energy of six consecutive windows

			if(window_energy>silence_threshold){    // Voice window detection
				one_count++;
			}
			else{
				one_count=0;			
			}

			if(one_count>4){	// If five such continous voice windows are detected then it is treated as a true voice
				one_count=0;
				if(sample_count>=(11*window_size))
					Voice_start = sample_count-(11*window_size);		// Voice starting point = present sample count - (5+6) windows // Five such 6 consecutive windows  
				else
					Voice_start = 0;
				break;
			}
		}
		
		for(int i=0; i<37 ; i++){ 			// (37+11)*341 > 16000. Once voice is detected we are skipping window energy calulation untill next 1 sec. 
			while(1){ // 			
			   if(Record_flag[flag_pos]==true){
					Record_flag[flag_pos] = false;
					flag_pos++;
					if(flag_pos>=(ddr_buffer)){	
						flag_pos = 0;			
					}
					break;
			   }
				this_thread::sleep_for(chrono::milliseconds(5) );			   
			}
		}
		sample_count = flag_pos*window_size; 
		
		Voice_end = sample_count;
		
		if (Voice_end > Voice_start)
			memcpy(InputChunk, InputData+Voice_start, 16000);		// After a voice detection, Copying the next 1 sec data directly to InputChunk
		else{
			memcpy(InputChunk, InputData+Voice_start, (ddr_buffer*window_size-Voice_start));		// If the voice is detected at the edge of audio memory buffer
			memcpy(InputChunk+(ddr_buffer*window_size-Voice_start), InputData, 16000-(ddr_buffer*window_size-Voice_start));
		}

		KWS_DS_CNN kws(InputChunk);

		kws.extract_features(); //extract mfcc features
		kws.classify();			//classify using dnn
		
		int max_ind = kws.get_top_class(kws.output);
		int temp;
		//		char output_class[12][8] = {"Silence", "Unknown","yes","no","up","down","left","right","on","off","stop","go"};
		char Tasks[4][15] = {"Face Detect", "Object Detect" , "Plate Detect", "Segmentation"};
		
		switch (max_ind)
		{

		case 2:
			bbox_disp = true;
			printf("Keyword Detected : \"Yes\" \t Task : Display Bounding Box / Segmented Pixels \n");
			break; //yes -> Display Bounding Box
		case 3:
			bbox_disp = false;
			printf("Keyword Detected : \"No\" \t Task : Do Not Display Bounding Box/ Segmented Pixels \n");
			break; //no -> DO not display Bounding Box
		case 4: //up
			model = model + 1; 
			model = model % num_model;
			printf("Keyword Detected : \"Up\" \t Task : Switched to %s \n", Tasks[model]);
			//print_model  = true;
			//printf("\n");
			break; //up   -> change to next model
		case 5: //down
			model = model - 1;
			model = model % num_model;
			if (model<0){ model = model +num_model;}
			printf("Keyword Detected : \"Down\" \t Task : Switched to %s \n", Tasks[model]);
			//print_model  = true;
			//printf("\n");
			break; //down -> change to previous model
		case 6:
			dleft = true;
			dright = false;
			printf("Keyword Detected : \"Left\" \t Task : Display Processed Result for only left side of screen\n");
			break; //left  -> show bounding boxes only in left of the screen;
		case 7:
			dright = true;
			dleft = false;
			printf("Keyword Detected : \"Right\" \t Task : Display Processed Result for only Right side of screen\n");
			break; //right -> show bounding boxes only in right of the screen;
		case 10:
			bbox_disp = true;
			bbox_thick = 2;
			dleft = false;
			dright = false;
			red = 255;
			green = 0;
			blue = 0;
			top = false;
			bottom = false;
			printf("Keyword Detected : \"Stop\" \t Task : Stop Current Settings And reset to Default\n");
			break; //stop  -> reset to defaults;
		case 11:
			
			if(model==3){
				if (top==false && bottom==false){
					top=true;
				}
				else if (top==true){
					bottom=true;
					top=false;
				}
				else if (bottom==true){
					bottom=false;
				}
				printf("Keyword Detected : \"Go\" \t Task : Toggle Processed Display between Top,Bottom and Complete Screen\n");
			}
			else {
			temp = blue;
			blue = red;
			red = green;
			green = temp;
			printf("Keyword Detected : \"Go\" \t Task : Toggle Bounding Box Color B -> R -> G \n");
			}
			break; //go -> Change colors
		case 8:	   //on -> Switch On display on Monitor
			snprintf(command, sizeof(command), "modetest -M xlnx -D fd4a0000.zynqmp-display -w 40:alpha:0");
			printf("Keyword Detected : \"On\" \t Task : Switch On Monitor Display\n");
			system(command);
			break;
		case 9: //off -> Switch off the Display on Monitor
			snprintf(command, sizeof(command), "modetest -M xlnx -D fd4a0000.zynqmp-display -w 40:alpha:255");
			printf("Keyword Detected : \"Off\" \t Task : Switch Off Monitor Display\n");
			system(command);
			break;
		}
	}
}

