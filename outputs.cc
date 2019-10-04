#include "outputs.h"


// The "title" is used as the title of the output window. If empty,
// the output window is disabled.
VideoOutput::VideoOutput(int frame_width, std::string title)
{
  width_ = frame_width;
  title_ = title;
}


// Calling this enables file output, which then cannot be disabled.
void VideoOutput::SetFileOutput(const std::string &filename,
                                int encoding, int fps,
                                const cv::Size &size)
{
  out_ = new cv::VideoWriter();
  out_->open(filename, encoding, fps, size, true);
  if (!out_->isOpened()) {
    std::cerr << "Warning: cannot create output file " << filename << std::endl;
    delete out_;
    out_ = nullptr;
  }
}


// This is method that every other method calls to actually display a frame.
void VideoOutput::DoDisplay(const cv::Mat &frame)
{
  if (frame.empty()) return;

  if (!title_.empty()) {
    cv::imshow(title_, frame);
  }
  if (out_) {
    (*out_) << frame;
  }
}


// Display the given frame (when no area of interest has been detected).
void VideoOutput::Display(const cv::Mat &frame)
{
  DoDisplay(frame);
}


// Display the given frame, taking into account the midpoint of
// symmetry.
// (The midpoint is used by the mirrored outputs. In the parent class,
// it is ignored.)
void VideoOutput::Display(const cv::Mat &frame, int inflection_point)
{
  DoDisplay(frame);
}


// Display a frame made up of two halves.
void VideoOutput::DisplayDual(const cv::Mat &lhs, const cv::Mat &rhs)
{
  int midpoint = width_ / 2;
  cv::Mat result(lhs.rows, width_, CV_8UC3, cv::Scalar(0,0,0));
  lhs.copyTo(result(cv::Rect(midpoint-lhs.cols, 0, lhs.cols, lhs.rows)));
  rhs.copyTo(result(cv::Rect(midpoint, 0, rhs.cols, rhs.rows)));
  DoDisplay(result);
}


VideoOutput::~VideoOutput()
{
  if (out_) {
    delete out_;
  }
}


// The mirrored outputs do nothing when there is no face
// present in the image.
void LeftReflectedVideoOutput::Display(const cv::Mat &frame)
{
}
void RightReflectedVideoOutput::Display(const cv::Mat &frame)
{
}


// Crop left side, flip it to create the right side.
void LeftReflectedVideoOutput::Display(const cv::Mat &frame, int inflection_point)
{
  int midpoint = width_ / 2;
  cv::Mat lhs, rhs;
  if (inflection_point < midpoint) {
    lhs = frame(cv::Rect(0, 0, inflection_point, frame.rows));
  } else {
    lhs = frame(cv::Rect(inflection_point - midpoint, 0, midpoint, frame.rows));
  }
  cv::flip(lhs, rhs, 1);
  DisplayDual(lhs, rhs);
}


// Crop right side, flip it to create the left side.
void RightReflectedVideoOutput::Display(const cv::Mat &frame, int inflection_point)
{
  int midpoint = width_ / 2;
  cv::Mat lhs, rhs;
  if (frame.cols - inflection_point < midpoint) {
    rhs = frame(cv::Rect(inflection_point, 0, frame.cols - inflection_point, frame.rows));
  } else {
    rhs = frame(cv::Rect(inflection_point, 0, midpoint, frame.rows));
  }
  cv::flip(rhs, lhs, 1);
  DisplayDual(lhs, rhs);
}
