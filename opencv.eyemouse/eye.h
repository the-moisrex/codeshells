#ifndef EYE
#define EYE

#include "mouse.h"
#include "pch.h"

using namespace cv;

Rect getLeftmostEye(std::vector<Rect> &eyes) {
  int leftmost = 99999999;
  int leftmostIndex = -1;
  for (int i = 0; i < eyes.size(); i++) {
    if (eyes[i].tl().x < leftmost) {
      leftmost = eyes[i].tl().x;
      leftmostIndex = i;
    }
  }
  return eyes[leftmostIndex];
}

Vec3f getEyeball(Mat &eye, std::vector<Vec3f> &circles) {
  std::vector<int> sums(circles.size(), 0);
  for (int y = 0; y < eye.rows; y++) {
    uchar *ptr = eye.ptr<uchar>(y);
    for (int x = 0; x < eye.cols; x++) {
      int value = static_cast<int>(*ptr);
      for (int i = 0; i < circles.size(); i++) {
        Point center((int)std::round(circles[i][0]),
                     (int)std::round(circles[i][1]));
        int radius = (int)std::round(circles[i][2]);
        if (std::pow(x - center.x, 2) + std::pow(y - center.y, 2) <
            std::pow(radius, 2)) {
          sums[i] += value;
        }
      }
      ++ptr;
    }
  }
  int smallestSum = 9999999;
  int smallestSumIndex = -1;
  for (int i = 0; i < circles.size(); i++) {
    if (sums[i] < smallestSum) {
      smallestSum = sums[i];
      smallestSumIndex = i;
    }
  }
  return circles[smallestSumIndex];
}

std::vector<Point> centers;
cv::Point lastPoint;
cv::Point mousePoint;

Point stabilize(std::vector<Point> &points, int windowSize) {
  float sumX = 0;
  float sumY = 0;
  int count = 0;
  for (int i = std::max(0, (int)(points.size() - windowSize));
       i < points.size(); i++) {
    sumX += points[i].x;
    sumY += points[i].y;
    ++count;
  }
  if (count > 0) {
    sumX /= count;
    sumY /= count;
  }
  return Point(sumX, sumY);
}

void detectEyes(Mat &frame, CascadeClassifier &faceCascade,
                CascadeClassifier &eyeCascade) {
  Mat grayscale;
  cvtColor(frame, grayscale, COLOR_BGR2GRAY); // convert image to grayscale
  equalizeHist(grayscale, grayscale);         // enhance image contrast
  std::vector<Rect> faces;
  faceCascade.detectMultiScale(grayscale, faces, 1.1, 2,
                               0 | CASCADE_SCALE_IMAGE, Size(150, 150));

  if (faces.size() == 0)
    return;                   // none face was detected
  Mat face = frame(faces[0]); // crop the face
  std::vector<Rect> eyes;
  eyeCascade.detectMultiScale(face, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE,
                              Size(150, 150)); // same thing as above

  rectangle(frame, faces[0].tl(), faces[0].br(), Scalar(255, 0, 0), 2);
  if (eyes.size() != 2)
    return; // both eyes were not detected
  for (Rect &eye : eyes) {
    rectangle(frame, faces[0].tl() + eye.tl(), faces[0].tl() + eye.br(),
              Scalar(0, 255, 0), 2);
  }

  Rect eyeRect = getLeftmostEye(eyes);

  Mat eye = face(eyeRect); // crop the leftmost eye
  equalizeHist(eye, eye);
  std::vector<Vec3f> circles;
  HoughCircles(eye, circles, HOUGH_GRADIENT, 1, eye.cols / 8, 250, 15,
               eye.rows / 8, eye.rows / 3);

  if (circles.size() > 0) {
    Vec3f eyeball = getEyeball(eye, circles);
    Point center(eyeball[0], eyeball[1]);
    centers.push_back(center);
    center = stabilize(centers, 5); // we are using the last 5
    if (centers.size() > 1) {
      cv::Point diff;
      diff.x = (center.x - lastPoint.x) * 20;
      diff.y = (center.x - lastPoint.y) *
               -30; // diff in y is higher because it's "harder" to move the
                    // eyeball up/down instead of left/right
    }
    lastPoint = center;
    int radius = (int)eyeball[2];
    circle(frame, faces[0].tl() + eyeRect.tl() + center, radius,
           Scalar(0, 0, 255), 2);
    circle(eye, center, radius, Scalar(255, 255, 255), 2);
  }

  imshow("Eye", eye);
}


#endif
