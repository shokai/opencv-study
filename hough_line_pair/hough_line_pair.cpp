// カメラからキャプチャしてハフ変換
// 参考 http://opencv.jp/sample/special_transforms.html#hough_line

#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
  CvMemStorage *storage = cvCreateMemStorage(0);
  CvSeq *lines = 0;
  CvCapture *capture;
  capture = cvCreateCameraCapture(0);
  IplImage *img, *img_gray, *img_result;
  img = cvQueryFrame(capture);
  const int width = img->width;
  const int height = img->height;
  img_gray = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
  img_result = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  
  char *winNameCapture = "Capture";
  char *winNameHough = "Hough Lines";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameHough, CV_WINDOW_AUTOSIZE);

  while (1) {
    cout << "loop" << endl;
    img = cvQueryFrame(capture);
    cvCopy(img, img_result);
    for(int i = 0; i < 10; i++){
      img = cvQueryFrame(capture);
      cvShowImage(winNameCapture, img);
      
      cvCvtColor(img, img_gray, CV_BGR2GRAY);
      cvCanny(img_gray, img_gray, 50, 200, 3);
      
      cvClearMemStorage(storage);
      lines = cvHoughLines2(img_gray, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI / 180, 50, 50, 10);
      cout << lines->total << "lines" << endl;
      for (int j = 0; j < lines->total; j++) {
	CvPoint *p = (CvPoint *)cvGetSeqElem(lines, j);
	cvLine(img_result, p[0], p[1], CV_RGB (0, 0, 255), 3, 8, 0);
      }
      cvClearSeq(lines);

    }

    cvShowImage(winNameHough, img_result);
    if (cvWaitKey(1) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameHough);
  cvReleaseMemStorage(&storage);
  return 0;
}
