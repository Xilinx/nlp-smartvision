#include "kws_ds_cnn.h"
#include "wav_data.h"

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
	char output_class[12][8] = {"Silence", "Unknown", "yes", "no", "up", "down", "left", "right", "on", "off", "stop", "go"};
    float window_energy=0;
	float window[6];
	int wpos = 5;
    float silence_threshold = (float)(pow(2,30)*window_size*6*15*pow(10,-5));    //Energy less than this is a silence
    int cut_start = 0;
    int cut_end = 0;
    int sample_count = 0;   // First 1 sec is considered as non-silence
    int one_count = 0;
    int flag_pos = 0;
	short InputChunk[16000]; // Audio Input samples are in signed int16 precision

	while(1){
		
		for(int j=0; j<5; j++){
			while(1){
				if(Record_flag[flag_pos]==1){
					window[j]=0;
					for(int i=0; i<window_size; i++){
						window[j] +=  (float)pow(InputData[sample_count+i],2);}	
					Record_flag[flag_pos] = 0;			
					flag_pos++;
					if(flag_pos>=(50*ddr_buffer)){
						flag_pos = 0;			
					}
					break;
				}
				this_thread::sleep_for(chrono::milliseconds(5) );	
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
						window[wpos] +=  (float)pow(InputData[sample_count+i],2);
					Record_flag[flag_pos] = 0;
					flag_pos++;
					if(flag_pos>=(50*ddr_buffer)){
						flag_pos = 0;			
					}
					break;
				}
				this_thread::sleep_for(chrono::milliseconds(5) );	
			}
			sample_count = flag_pos*window_size;
			wpos++;
			if(wpos>5)
				wpos=0;
			
			window_energy=0;
			for(int i=0; i<6; i++)
				window_energy += window[i];		// store latest window energy

			if(window_energy>silence_threshold){    // Voice detection
				one_count++;
			}
			else{
				one_count=0;
			}

			if(one_count>4){
				one_count=0;
				if(sample_count>=(11*window_size))
					cut_start = sample_count-(11*window_size);
				else
					cut_start = 0;
				break;
			}
		}
		
		for(int i=0; i<37 ; i++){ // (37+11)*341 > 16000
			while(1){ // 			
			   if(Record_flag[flag_pos]==true){
					Record_flag[flag_pos] = false;
					flag_pos++;
					if(flag_pos>=(50*ddr_buffer)){
						flag_pos = 0;			
					}
					break;
			   }
				this_thread::sleep_for(chrono::milliseconds(5) );			   
			}
		}
		sample_count = flag_pos*window_size; // 25
		
		cut_end = sample_count;
		
		if (cut_end > cut_start)
			memcpy(InputChunk, InputData+cut_start, 16000);
		else{
			memcpy(InputChunk, InputData+cut_start, (ddr_buffer*50*window_size-cut_start));
			memcpy(InputChunk+(ddr_buffer*50*window_size-cut_start), InputData, 16000-(ddr_buffer*50*window_size-cut_start));
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

