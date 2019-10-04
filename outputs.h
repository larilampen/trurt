#pragma once

#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>


// A video output stream that displays frames in
// a window and/or writes them into a video file.
class VideoOutput
{
private:
  cv::VideoWriter *out_ = nullptr;
  std::string title_;
protected:
  int width_;
  void DisplayDual(const cv::Mat &lhs, const cv::Mat &rhs);
  void DoDisplay(const cv::Mat &frame);
public:
  VideoOutput(int frame_width, std::string title);
  void SetFileOutput(const std::string &filename, int encoding, int fps, const cv::Size &size);
  virtual void Display(const cv::Mat &frame);
  virtual void Display(const cv::Mat &frame, int inflection_point);
  ~VideoOutput();
};


// A video stream where the left side is reflected and
// superimposed on the the right side.
class LeftReflectedVideoOutput: public VideoOutput
{
  using VideoOutput::VideoOutput;
  //using VideoOutput::Display(const cv::Mat&, const cv::Mat&);
public:
  void Display(const cv::Mat &frame) override;
  void Display(const cv::Mat &frame, int inflection_point) override;
};


// A video stream where the right side is reflected and
// superimposed on the the left side.
class RightReflectedVideoOutput: public VideoOutput
{
  using VideoOutput::VideoOutput;
  //using VideoOutput::Display;
public:
  void Display(const cv::Mat &frame) override;
  void Display(const cv::Mat &frame, int inflection_point) override;
};
