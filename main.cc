// See README.md for license, description and usage instructions.

#include <iostream>

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>

#include <tclap/CmdLine.h>

#include "VideoFaceDetector.h"
#include "outputs.h"


int frame_count = 0;
int start_frame = 0;
int end_frame = -1;
bool show_landmarks = false;
bool show_midpoint = false;

std::vector<VideoOutput*> outputs;


void draw_polyline(cv::Mat &img, const dlib::full_object_detection &d, const int start,
                   const int end, bool isClosed = false)
{
  std::vector <cv::Point> points;
  for (int i = start; i <= end; ++i)
    points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
  cv::polylines(img, points, isClosed, cv::Scalar(255,0,0), 1, 16);
}


// Draw facial landmarks (from dlib) on the given frame.
void draw_landmarks(cv::Mat &frame, const dlib::full_object_detection &shape)
{
  draw_polyline(frame, shape, 0, 16);           // Jaw line
  draw_polyline(frame, shape, 17, 21);          // Left eyebrow
  draw_polyline(frame, shape, 22, 26);          // Right eyebrow
  draw_polyline(frame, shape, 27, 30);          // Nose bridge
  draw_polyline(frame, shape, 30, 35, true);    // Lower nose
  draw_polyline(frame, shape, 36, 41, true);    // Left eye
  draw_polyline(frame, shape, 42, 47, true);    // Right Eye
  draw_polyline(frame, shape, 48, 59, true);    // Outer lip
  draw_polyline(frame, shape, 60, 67, true);    // Inner lip
}


static dlib::rectangle openCVRectToDlib(const cv::Rect &r) {
  return dlib::rectangle((long)r.tl().x, (long)r.tl().y,
                         (long)r.br().x - 1, (long)r.br().y - 1);
}


// This function contains the actual frame processing loop.
void process(cv::VideoCapture &capture)
{
  int key = 0;
  dlib::shape_predictor pose_model;
  dlib::deserialize("data/landmarks68.dat") >> pose_model;
  VideoFaceDetector vdetector("data/haarcascade_frontalface_default.xml", capture);

  // Keys q and ESC exit the loop.
  while (key != 'q' && key != 27) {
    switch (key) {
      case 'l':
        show_landmarks = !show_landmarks;
        break;
      case 'm':
        show_midpoint = !show_midpoint;
        break;
      case 'p':
        cv::waitKey(0);
    }

    cv::Mat frame;
    vdetector >> frame;
    frame_count++;
    if (frame_count < start_frame)
      continue;
    if (end_frame > -1 && frame_count > end_frame)
      break;

    if (frame.empty()) {
      // Input file finished or camera disconnected.
      break;
    }

    if (vdetector.isFaceFound()) {
      dlib::cv_image<dlib::bgr_pixel> cimg(frame);
      dlib::full_object_detection shape = pose_model(cimg, openCVRectToDlib(vdetector.face()));

      // Mirror at the top point of the nose bridge.
      int middle = shape.part(27).x();

      // The landmarks and middle line are added before reflection, so that
      // they also appear in the mirrored images. This is intentional.
      if (show_landmarks) {
        draw_landmarks(frame, shape);
      }
      if (show_midpoint) {
        cv::line(frame, cv::Point(middle, 0), cv::Point(middle, frame.rows), cv::Scalar(0,0,255));
      }

      for (auto out : outputs) {
        out->Display(frame, middle);
      }
    } else {
      for (auto out : outputs) {
        out->Display(frame);
      }
    }

    //display(frame);
    key = cv::waitKey(1);
  }
}


// Main: parse command line arguments, open inputs and outputs.
int main(int argc, char *argv[])
{
  int left_width=0, right_width=0;
  std::string inputfile;
  bool use_mjpg = false;
  bool enable_windows = true;
  bool save_video = false;
  try {
    TCLAP::CmdLine cmd("Detect and mirror faces in videos", ' ', "VERSION");
    TCLAP::SwitchArg landmarkSwitch("f","face-landmarks","Show facial landmarks", cmd, false);
    TCLAP::SwitchArg midpointSwitch("m","show-midpoint","Show midpoint of mirroring", cmd, false);
    TCLAP::SwitchArg windowsSwitch("w","disable-windows","Disable GUI", cmd, false);
    TCLAP::SwitchArg saveSwitch("s","save","Save results to video files", cmd, false);
    TCLAP::SwitchArg mjpgSwitch("j","mjpg","Save in motion JPEG format", cmd, false);
    TCLAP::ValueArg<std::string> fileArg("i","input","Input file",false,"","file", cmd);
    TCLAP::ValueArg<int> leftSizeArg("l","left","Width of left reflected video in pixels, 0 to match input, -1 to disable",false,0,"integer", cmd);
    TCLAP::ValueArg<int> rightSizeArg("r","right","Width of right reflected video in pixels, 0 to match input, -1 to disable",false,0,"integer", cmd);
    TCLAP::ValueArg<int> startArg("b","begin-frame","Start from specified frame",false,0,"integer", cmd);
    TCLAP::ValueArg<int> endArg("e","end-frame","Stop at specified frame",false,-1,"integer", cmd);
    cmd.parse(argc, argv);
    show_landmarks = landmarkSwitch.getValue();
    show_midpoint = midpointSwitch.getValue();
    enable_windows = !windowsSwitch.getValue();
    inputfile = fileArg.getValue();
    save_video = saveSwitch.getValue();
    left_width = leftSizeArg.getValue();
    right_width = rightSizeArg.getValue();
    use_mjpg = mjpgSwitch.getValue();
    start_frame = startArg.getValue();
    end_frame = endArg.getValue();
  } catch (TCLAP::ArgException &e) {
    std::cerr << "Exception: " << e.typeDescription() << std::endl;
  }

  cv::VideoCapture capture;
  if (inputfile.empty()) {
    capture = cv::VideoCapture(0);
  } else {
    capture = cv::VideoCapture(inputfile);
  }

  if (!capture.isOpened()) {
    std::cerr << "Unable to open input. Check camera is connected "
                 "or file is readable." << std::endl;
    return -1;
  }

  int encoding;
  std::string extension;
  if (use_mjpg) {
    encoding = CV_FOURCC('M', 'J', 'P', 'G');
    extension = ".avi";
  } else {
    encoding = CV_FOURCC('D', 'I', 'V', 'X');
    extension = ".mp4";
  }

  int fps = capture.get(CV_CAP_PROP_FPS);
  int input_width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
  int input_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

  if (left_width >= 0) {
    if (left_width == 0) left_width = input_width;
    auto out = new LeftReflectedVideoOutput(left_width,
                                            enable_windows ? "Left" : "");
    if (save_video) {
      out->SetFileOutput("left" + extension, encoding, fps,
                         cv::Size(left_width, input_height));
    }
    outputs.push_back(out);
  }

  if (right_width >= 0) {
    if (right_width == 0) right_width = input_width;

    auto out = new RightReflectedVideoOutput(right_width,
                                             enable_windows ? "Right" : "");
    if (save_video) {
      out->SetFileOutput("right" + extension, encoding, fps,
                         cv::Size(right_width, input_height));
    }
    outputs.push_back(out);
  }

  {
    auto out = new VideoOutput(input_width, enable_windows ? "Input" : "");

    // Only save the input feed when it comes from a camera.
    if (save_video && inputfile.empty()) {
      cv::Size size(input_width, input_height);
      out->SetFileOutput("camera" + extension, encoding, fps, size);
    }
    outputs.push_back(out);
  }

  int64 timer = cv::getTickCount();

  process(capture);

  int tdiff = (cv::getTickCount() - timer) / cv::getTickFrequency();
  std::cout << "Processed " << frame_count << " frames in " << tdiff
            << " seconds; " << (double)frame_count / tdiff << " FPS." << std::endl;

  if (enable_windows) {
    cv::destroyAllWindows();
  }

  return 0;
}

