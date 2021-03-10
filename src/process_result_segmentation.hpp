/*
 * Copyright 2019 Xilinx Inc.
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
#pragma once
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
static cv::Mat process_result_segmentation(cv::Mat &m1,
                              const vitis::ai::SegmentationResult &result,
                              bool is_jpeg, bool left, bool right, bool top, bool bottom) {
  auto s = result.segmentation.size();
  auto s1 = m1.size();
  
  auto row_ind_start = 0;
  auto col_ind_start = 0;
  auto row_ind_max = m1.size().height;
  auto col_ind_max = m1.size().width;
  
	if (top==true){
		row_ind_start=0;
		row_ind_max=m1.size().height/2;
	}
	else if (bottom==true){
		row_ind_start=m1.size().height/2;
		row_ind_max=m1.size().height;
	}
	
	if(left==true){
		col_ind_start=0;
		col_ind_max=m1.size().width/2;
	}
	else if(right==true){
		col_ind_start=m1.size().width/2;
		col_ind_max=m1.size().width;
	}

  
  for (auto row_ind = row_ind_start; row_ind < row_ind_max; ++row_ind) {
    for (auto col_ind = col_ind_start; col_ind < col_ind_max; ++col_ind) {
      int i = row_ind * s.height/s1.height ;
      int j =  col_ind * s.width/s1.width ;
      m1.at<cv::Vec3b>(row_ind, col_ind) =
          m1.at<cv::Vec3b>(row_ind, col_ind) * 0.5 +
          result.segmentation.at<cv::Vec3b>(i, j) * 0.5;
    }
  }
  return m1;
}

