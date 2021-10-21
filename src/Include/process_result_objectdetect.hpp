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

#pragma once
#include <glog/logging.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

std::string sample_type(int label) {
  int x = label % 19;
  std::string output;
  switch (x) {
  case 0: //
    output = "aeroplane";
    break;
  case 1: //
    output = "bicycle";
    break;
  case 2: //
    output = "bird";
    break;
  case 3: //
    output = "boat";
    break;
  case 4: //
    output = "bottle";
    break;
  case 5: //
    output = "bus";
    break;
  case 6: //
    output = "car";
    break;
  case 7: //
    output = "cat";
    break;
  case 8: //
    output = "chair";
    break;
  case 9: //
    output = "cow";
    break;
  case 10: //
    output = "diningtable";
    break;
  case 11: //
    output = "dog";
    break;
  case 12: //
    output = "horse";
    break;
  case 13: //
    output = "motorbike";
    break;
  case 14: //
    output = "person";
    break;
  case 15: //
    output = "pottedplant";
    break;
  case 16: //
    output = "sheep";
    break;
  case 17: //
    output = "sofa";
    break;
  case 18: //
    output = "train";
    break;
  case 19: //
    output = "tvmonitor";
    break;
  default:
    output = "None";
  }
  return output;
}

cv::Scalar label2Color(int label, int blue, int green, int red) {
  if (blue == 255) {

    static cv::Scalar table[19] = {
        {255, 64, 128},  {255, 35, 232},  {255, 70, 70},   {255, 102, 156},
        {255, 153, 153}, {255, 153, 153}, {255, 170, 30},  {255, 220, 0},
        {255, 142, 35},  {255, 251, 152}, {255, 130, 180}, {255, 20, 60},
        {255, 0, 0},     {255, 0, 142},   {255, 0, 70},    {255, 60, 100},
        {255, 80, 100},  {255, 0, 230},   {255, 11, 32}};
    return table[label % 19];
  } else if (green == 255) {

    static cv::Scalar table[19] = {
        {128, 255, 128}, {244, 255, 232}, {70, 255, 70},  {102, 255, 156},
        {190, 255, 153}, {153, 255, 153}, {250, 255, 30}, {220, 255, 0},
        {107, 255, 35},  {152, 255, 152}, {0, 255, 180},  {220, 255, 60},
        {255, 255, 0},   {0, 255, 142},   {0, 255, 70},   {0, 255, 100},
        {0, 255, 100},   {0, 255, 230},   {119, 255, 32}};
    return table[label % 19];
  } else if (red == 255) {

    static cv::Scalar table[19] = {
        {128, 64, 255},  {244, 35, 255},  {70, 70, 255},   {102, 102, 255},
        {190, 153, 255}, {153, 153, 255}, {250, 170, 255}, {220, 220, 255},
        {107, 142, 255}, {152, 251, 255}, {0, 130, 255},   {220, 20, 255},
        {255, 0, 255},   {0, 0, 255},     {0, 0, 255},     {0, 60, 255},
        {0, 80, 255},    {0, 0, 255},     {119, 11, 255}};
    return table[label % 19];
  }
}

static cv::Mat
process_result_objectdetect(cv::Mat &image,
                            const vitis::ai::YOLOv2Result &result, bool is_jpeg,
                            long int time, int thickness, bool left, bool right,
                            int green, int blue, int red) {
  for (const auto bbox : result.bboxes) {
    int label = bbox.label;
    std::string sample = "Hello";
    float xmin = bbox.x * image.cols + 1;
    float ymin = bbox.y * image.rows + 1;
    float xmax = xmin + bbox.width * image.cols;
    float ymax = ymin + bbox.height * image.rows;
    float confidence = bbox.score;
    if (xmax > image.cols)
      xmax = image.cols;
    if (ymax > image.rows)
      ymax = image.rows;
    LOG_IF(INFO, is_jpeg) << "RESULT: " << label << "\t" << xmin << "\t" << ymin
                          << "\t" << xmax << "\t" << ymax << "\t" << confidence
                          << "\n";

    if (left == false && right == false) {
      cv::rectangle(image, cv::Point(xmin, ymin), cv::Point(xmax, ymax),
                    label2Color(label, blue, green, red), thickness, 1, 0);
      cv::putText(image, sample_type(label), cv::Point(xmin, ymin),
                  cv::FONT_HERSHEY_PLAIN, 1.0,
                  label2Color(label, blue, green, red), 2.0);
    } else if ((bbox.x + (bbox.width / 2)) <= 0.5 and left == true) {
      cv::rectangle(image, cv::Point(xmin, ymin), cv::Point(xmax, ymax),
                    label2Color(label, blue, green, red), thickness, 1, 0);
      cv::putText(image, sample_type(label), cv::Point(xmin, ymin),
                  cv::FONT_HERSHEY_PLAIN, 1.0,
                  label2Color(label, blue, green, red), 2.0);
    } else if ((bbox.x + (bbox.width / 2)) > 0.5 and right == true) {
      cv::rectangle(image, cv::Point(xmin, ymin), cv::Point(xmax, ymax),
                    label2Color(label, blue, green, red), thickness, 1, 0);
      cv::putText(image, sample_type(label), cv::Point(xmin, ymin),
                  cv::FONT_HERSHEY_PLAIN, 1.0,
                  label2Color(label, blue, green, red), 2.0);
    }
  }
  return image;
}
