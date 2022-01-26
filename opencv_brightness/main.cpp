#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;

void slideshow(std::initializer_list<Mat> mats) {
  namedWindow("Image", WINDOW_NORMAL);
  for (;;) {
    for (auto &mat : mats) {
      imshow("Image", mat);
      if (waitKey(100) == 32) {
        destroyAllWindows();
        return;
      }
    }
  }
}

auto main() -> int {
  Mat image = imread("../imgs/selena-orange.jpg", 1);
  if (!image.data) {
    std::cerr << "Cannot load the iamge" << std::endl;
    return EXIT_FAILURE;
  }

  auto brighten = image.clone(); // copying the image

  auto size = brighten.size();
  std::cout << "width: " << size.width << "; height: " << size.height
            << std::endl;
  for (int i = 0; i < size.height; i++) {
    for (int j = 0; j < size.width; j++) {
      uchar *col = brighten.ptr<uchar>(i, j);
      uchar *B = col + 0;
      uchar *G = col + 1;
      uchar *R = col + 2;

      auto val = static_cast<uchar>(abs(static_cast<double>(j) * 100.0 /
                                        static_cast<double>(size.width)));

      *B = static_cast<uchar>(std::min(*B + val, 255));
      *G = static_cast<uchar>(std::min(*G + val, 255));
      *R = static_cast<uchar>(std::min(*R + val, 255));
    }
  }

  cvtColor(brighten, brighten, COLOR_BGR2HSV);
  for (int i = 0; i < size.height; i++) {
    for (int j = 0; j < size.width; j++) {
      uchar *col = brighten.ptr<uchar>(i, j);
      auto *hue = col;

      *hue += 100;
    }
  }
  cvtColor(brighten, brighten, COLOR_HSV2BGR);

  auto ads = image.clone();
  rectangle(ads, Point(0, 945), Point(80, 960), Scalar(0, 0, 255), -1);

  auto lined = image.clone();
  line(lined, Point(165, 125), Point(180, 125), Scalar(0, 0), 3);

  auto circled = image.clone();
  circle(circled, Point(172, 110), 228 - 172, Scalar(0, 0, 255), 3);

  // auto polylined = image.clone();
  // polylines(polylined, );

  auto outlined = image.clone();
  rectangle(outlined, Point(0, 0), Point(outlined.size().width * 2, 100),
            Scalar(), 5);

  auto texted = image.clone();
  putText(texted, "Cool", Point(175, 420), FONT_HERSHEY_COMPLEX, 0.5, Scalar(),
          1);

  auto translated = image.clone();
  auto Tx = translated.size().width / 4;
  auto Ty = translated.size().height / 4;
  std::array<std::array<float, 3>, 2> T{
      std::array<float, 3>{1, 0, static_cast<float>(Tx)},
      std::array<float, 3>{0, 1, static_cast<float>(Ty)}};
  warpAffine(translated, translated, T,
             Size(translated.size().width, translated.size().height));

  slideshow(
      {image, brighten, ads, lined, circled, outlined, texted, translated});

  return 0;
}
