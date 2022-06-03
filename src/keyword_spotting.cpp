/*
 * Copyright 2022 Xilinx Inc.
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
#include <fstream>
#include <iostream>
#include<memory>
#include <glog/logging.h>
#include <chrono>
#include <stdio.h>
#include <bits/stdc++.h>
#include"global_var.h"

using namespace std;
using namespace std::chrono;

void Keyword_Spotting()
{
	//char command[256];
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
    int total_energy = 0;	
	short InputChunk[16000]; // Audio Input samples are in signed int16 precision
	bool silence_flag = false;
	
	auto T1 = high_resolution_clock::now();
	auto T2 = high_resolution_clock::now();
	auto Time_taken = duration_cast<microseconds>(T2 - T1);

	for(int j=0; j<100; j++){				// Calculation of noise threshold based on first 2 sec audio (~100 windows)
		while(1){
			if(Record_flag[flag_pos]==1){		// Checking whether this audio window is filled with samples or not
				for(int i=0; i<window_size; i++){
					total_energy +=  (float)pow(InputData[sample_count+i],2);}		// Accumulating energy of audio windows
				Record_flag[flag_pos] = 0;			
				flag_pos++;
				if(flag_pos>=(ddr_buffer)){		// If all the windows are completed, then back to 0
					flag_pos = 0;			
				}
				break;
			}
			else{
			this_thread::sleep_for(chrono::milliseconds(5) );	// If the audio is not captured, then wait for 5 milli sec
			}
		}
		sample_count = flag_pos*window_size;
	}
	
	// 100 times of average noise enrergy is used to detect presence of voice in 6 consecutive windows
	//Total energy of 6 consecutive audio sample windows, which is less than this is treated as silence
	 silence_threshold = total_energy * 6.0;
	
	cout << "Noise Threshold is set. You can start speaking the keywords now.." << endl;


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
				else{
				this_thread::sleep_for(chrono::milliseconds(5) );	// If the audio is not captured, then wait for 5 milli sec
				}
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
		
		if (fps) {
			T1 = high_resolution_clock::now();
		}
		
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
		case 0:
			silence_flag = true;
			break;
		case 1: //unknown keyword
			printf("Unknown Keyword \n");
			silence_flag = true;
			break;
		case 2:	//yes
			bbox_disp = true;
			printf("Keyword Detected : \"Yes\" \t Task : Display Bounding Box / Segmented Pixels \n");
			break;  
		case 3: //no
			bbox_disp = false;
			printf("Keyword Detected : \"No\" \t Task : Do Not Display Bounding Box/ Segmented Pixels \n");
			break;  
		case 4: //up
			model = model + 1; 
			model = model % num_model;
			printf("Keyword Detected : \"Up\" \t Task : Switched to %s \n", Tasks[model]);
			break; 
		case 5: //down
			model = model - 1;
			model = model % num_model;
			if (model<0){ model = model +num_model;}
			printf("Keyword Detected : \"Down\" \t Task : Switched to %s \n", Tasks[model]);
			break; 
		case 6: //left
			dleft = true;
			dright = false;
			printf("Keyword Detected : \"Left\" \t Task : Display Processed Result for only left side of screen\n");
			break; 
		case 7: //right
			dright = true;
			dleft = false;
			printf("Keyword Detected : \"Right\" \t Task : Display Processed Result for only Right side of screen\n");
			break; 
		case 10: //stop
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
			break; 
		case 11: //go
			
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
			break; 
		case 8:	   //on 
			//snprintf(command, sizeof(command), "modetest -M xlnx -D fd4a0000.zynqmp-display -w 40:alpha:0");
			display_on = true;			
			printf("Keyword Detected : \"On\" \t Task : Switch On Monitor Display\n");
			//system(command);
			break;
		case 9: //off 
			//snprintf(command, sizeof(command), "modetest -M xlnx -D fd4a0000.zynqmp-display -w 40:alpha:255");
			display_on = false;			
			printf("Keyword Detected : \"Off\" \t Task : Switch Off Monitor Display\n");
			//system(command);
			break;
		}
		if (fps) {		
			T2 = high_resolution_clock::now();
			Time_taken = duration_cast<microseconds>(T2 - T1);
			if(silence_flag == false){
				cout << "Time taken for Keyword Spotting and corresponding action = " << (double)((Time_taken.count())/1000) << " milli sec" << endl;
			}		
			else {silence_flag = false;}
		}
	}
}

void Keyword_Spotting_Debug(char *testfile)
{
	//char command[256];
	//char output_class[12][8] = {"Silence", "Unknown", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"};
	string Actual_Keyword;
	string Detected_Keyword;
	short InputChunk[16000]; // Audio Input samples are in signed int16 precision
	string FileName;
	int file_count = 0;
	int detect_count = 0;
	FILE *fp;
	
	ifstream infile;
	infile.open (testfile);
	if(!infile){
		cout << endl;
		cout << "Exception opening/reading the file " << testfile << endl << endl;
		return;
	}
	else{
		while(getline(infile, FileName))
		{	
			if(FileName.back() == '\n' || FileName.back() == '\r'){ // removing the last end line character
				FileName.pop_back();
			}

			fp = fopen(FileName.c_str(), "rb");
			if(fp == NULL){
				printf("\nException opening/reading the audio file %s \n",FileName.c_str());
				printf("Expected way of storing the audio file names in %s file is keyword/audio_filename.wav \n\n",testfile);
				return;
			}
			size_t result = fread(&InputChunk[0],sizeof(short),16000,fp);  // Read the audio data
			if (result == 0) {
				printf("\nException reading the audio file %s \n",FileName.c_str());
				return;
			}
			fclose(fp);
			
			file_count++;
			Actual_Keyword = "";
			
			for(unsigned int i=0; i < strlen(FileName.c_str()); i++){
				if(FileName[i] != '/'){
					Actual_Keyword = Actual_Keyword + FileName[i];
				}
				else{
					break;
				}
			}
			
			cout << "Ground truth : " << Actual_Keyword << "\t\t";
			
			KWS_DS_CNN kws(InputChunk);

			kws.extract_features(); //extract mfcc features
			kws.classify();			//classify using dnn
			
			int max_ind = kws.get_top_class(kws.output);
			//int temp;
			//		char output_class[12][8] = {"Silence", "Unknown","yes","no","up","down","left","right","on","off","stop","go"};
			
			switch (max_ind)
			{
			case 0:
				Detected_Keyword = "silence";
				break;
			
			case 1:
				Detected_Keyword = "unknown";
				break;

			case 2:
				Detected_Keyword = "yes";
				break; 
			case 3:
				Detected_Keyword = "no";
				break; 
			case 4: 
				Detected_Keyword = "up";
				break; 
			case 5: 
				Detected_Keyword = "down";
				break; 
			case 6:
				Detected_Keyword = "left";
				break; 
			case 7:
				Detected_Keyword = "right";
				break; 
			case 10:
				Detected_Keyword = "stop";
				break; 
			case 11:
				Detected_Keyword = "go";
				break; 
			case 8:	   
				Detected_Keyword = "on";
				break;
			case 9: 
				Detected_Keyword = "off";
				break;
			}
			cout << "Prediction : " << Detected_Keyword << endl;
			if(strcmp(Actual_Keyword.c_str(),Detected_Keyword.c_str())==0){
				detect_count++;
			}
		}
		infile.close();
	  
	cout<< "=========================================" << endl;
	cout << "Number of keywords tested = " << file_count << endl;
	cout << "Number of keywords detected correctly = " << detect_count << endl;
	cout << "Accuracy = " << ((float)detect_count / (float)file_count)*100 << "%" << endl << endl; 
	}
}
