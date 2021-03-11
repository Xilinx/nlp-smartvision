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

//Initialisation of capture,drm, ml and thread related
#define HSIZE 1920
#define VSIZE 1080
cv::VideoCapture input("v4l2src ! video/x-raw, width=1920, height=1080 ! appsink", cv::CAP_GSTREAMER);
cv::VideoWriter output("appsrc ! kmssink driver-name=xlnx plane-id=39 fullscreen-overlay=true sync=false -v", cv::VideoWriter::fourcc('R', 'X', '2', '4'), 30.0, cv::Size(HSIZE,VSIZE), true);
pthread_t thread_id1;
cv::Mat cur_frame, process_frame, process_frame2;
sem_t sem;

static int window_size = 341;
static int ddr_buffer = 300;		// Maximum length of the audio (in sec) written into DDR without over-writing
short *InputData = (short *)malloc(ddr_buffer * 50 * window_size * sizeof(short)); // Audio Input samples are in signed int16 precision
bool *Record_flag = (bool *)calloc(ddr_buffer * 50, sizeof(bool));

bool bbox_disp = true;
int model = 0;
int num_model = 3;
bool print_model = true;
int bbox_thick = 2;
bool dleft = false;
bool dright = false;
int red = 255;
int green = 0;
int blue = 0;
bool top = false;
bool bottom = false;
int x[10];

void Keyword_Spotting();
void Detection();
void Capture_Audio();


void* myCaptureThreadFun(void)
{
  int i=0;
  //printf("in thread\n");
  while(1)
  {
    input.read(cur_frame);
    process_frame = cur_frame.clone();
    //printf("After input read\n");
    sem_post(&sem);
    i++;
     
  }
   pthread_exit(NULL);
}


int main()
{
	
	sem_init(&sem, 0, 0);

	#ifdef ENABLE_NLS
		setlocale(LC_ALL, "");
		textdomain(PACKAGE);
	#endif

	if(pthread_create(&thread_id1, NULL, myCaptureThreadFun, NULL) != 0)
	{
		printf("Failed to create thread\n");
	}

	thread CA(Capture_Audio);
	thread KWS(Keyword_Spotting);
	thread FD(Detection);
	
	CA.join();
	KWS.join();
	FD.join();

	return 0;
}
