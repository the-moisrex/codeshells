#ifndef MOUSE
#define MOUSE

#include "pch.h"

using namespace cv;

void mouseMove(int x, int y) {
  Display *displayMain = XOpenDisplay(NULL);

  if (displayMain == NULL) {
    fprintf(stderr, "Errore nell'apertura del Display !!!\n");
    exit(EXIT_FAILURE);
  }

  XWarpPointer(displayMain, None, None, 0, 0, 0, 0, x, y);

  XCloseDisplay(displayMain);
}

void changeMouse(cv::Mat &frame, cv::Point &location) {
  if (location.x > frame.cols)
    location.x = frame.cols;
  if (location.x < 0)
    location.x = 0;
  if (location.y > frame.rows)
    location.y = frame.rows;
  if (location.y < 0)
    location.y = 0;
  mouseMove(location.x, location.y);
}

#endif
