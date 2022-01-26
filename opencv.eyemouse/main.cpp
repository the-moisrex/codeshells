#include "eye.h"
#include "pch.h"

using namespace cv;

auto main() -> int {
  VideoCapture cap(0);
  if (!cap.isOpened()) {
    std::cerr << "Webcam not detected." << std::endl;
    return -1;
  }

  CascadeClassifier faceCascade;
  CascadeClassifier eyeCascade;
  if (!faceCascade.load("../haarcascade_frontalface_alt.xml")) {
    std::cerr << "Could not load face detector." << std::endl;
    return -1;
  }
  if (!eyeCascade.load("../haarcascade_eye_tree_eyeglasses.xml")) {
    std::cerr << "Could not load eye detector." << std::endl;
    return -1;
  }

  namedWindow("Original Image", WINDOW_AUTOSIZE);

  Mat frame;
  unsigned int fcount = 0;
  for (;;) {
    cap >> frame;
    detectEyes(frame, faceCascade, eyeCascade);
    changeMouse(frame, mousePoint);
    imshow("Original Image", frame);
    if (waitKey(1) >= 0) break;
    //fcount++;
    //if (fcount > 100)
    //  break;
  }

  destroyAllWindows();
  return 0;
}
