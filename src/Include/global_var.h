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

#ifndef __GlobalVar_H__
#define __GlobalVar_H__

extern int window_size;
extern int ddr_buffer;	
extern short *InputData;
extern bool *Record_flag;
extern bool bbox_disp;
extern int model;
extern int num_model;
extern bool print_model;
extern int bbox_thick;
extern bool dleft;
extern bool dright;
extern int red;
extern int green;
extern int blue;
extern bool top;
extern bool bottom;
extern int x[10];

#endif