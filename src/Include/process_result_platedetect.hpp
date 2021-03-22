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
 
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <glog/logging.h>

cv::Mat process_result_platedetect(cv::Mat &image, const vitis::ai::PlateDetectResult &result,
                       bool is_jpeg, long int time, int thickness, bool left, bool right, int green, int blue, int red) {
  LOG_IF(INFO, is_jpeg) << "result.box.score " << result.box.score << " " //
           << "result.box.x " << result.box.x << " toint "<< (int)(result.box.x * image.cols) << " "                   //
           << "result.box.y " << result.box.y << " toint " << (int)(result.box.y * image.rows)<< " "                   //
           << "result.box.width " << result.box.width<< " toint "<< (int)(result.box.width * image.cols) << " "           //
           << "result.box.height " << result.box.height << " toint "<< (int)(result.box.height * image.rows) << std::endl
           << "The real coordinate is: xx: "         //
           << "result.top_left :("  << result.top_left.x << " , "  << result.top_left.y << ") "//
           << "result.top_right :("  << result.top_right.x << " , "  << result.top_right.y << ") "//
           << "result.bottom_left :("  << result.bottom_left.x << " , "  << result.bottom_left.y << ") "//
           << "result.bottom_right :("  << result.bottom_right.x << " , "  << result.bottom_right.y << ") ";
  
  auto rect = cv::Rect {
    (int)(result.box.x * image.cols), (int)(result.box.y * image.rows),
    (int)(result.box.width * image.cols), (int)(result.box.height * image.rows)
  };
  
  if (left==false && right==false){
	  	  	  cv::rectangle(image, rect, cv::Scalar(blue,green,red), thickness);
	  }
  else if(result.box.x <= 0.5 and left==true){
			  cv::rectangle(image, rect, cv::Scalar(blue,green,red), thickness);
	  }
  else if(result.box.x > 0.5 and right==true){
			  cv::rectangle(image, rect, cv::Scalar(blue,green,red), thickness);
	  }
  return image;
}

