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

#include <glog/logging.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>


void process_result_facedetect(cv::Mat *m1,
                               const vitis::ai::FaceDetectResult &result,
                               bool is_jpeg, long int time, int thickness,
                               bool left, bool right, int green, int blue,
                               int red) {
  for (const auto &r : result.rects) {
    LOG_IF(INFO, is_jpeg) << " " << r.score << " " //
                          << r.x << " "            //
                          << r.y << " "            //
                          << r.width << " "        //
                          << r.height;

    if (left == false && right == false) {
      cv::rectangle((*m1),
                    cv::Rect{cv::Point(r.x * m1->cols, r.y * m1->rows),
                             cv::Size{(int)(r.width * m1->cols),
                                      (int)(r.height * m1->rows)}},
                    cv::Scalar(blue, green, red), thickness);
    } else if ((r.x + (r.width / 2)) <= 0.5 and left == true) {
      cv::rectangle((*m1),
                    cv::Rect{cv::Point(r.x * m1->cols, r.y * m1->rows),
                             cv::Size{(int)(r.width * m1->cols),
                                      (int)(r.height * m1->rows)}},
                    cv::Scalar(blue, green, red), thickness);
    } else if ((r.x + (r.width / 2)) > 0.5 and right == true) {
      cv::rectangle((*m1),
                    cv::Rect{cv::Point(r.x * m1->cols, r.y * m1->rows),
                             cv::Size{(int)(r.width * m1->cols),
                                      (int)(r.height * m1->rows)}},
                    cv::Scalar(blue, green, red), thickness);
    }
  }
  return;
}
