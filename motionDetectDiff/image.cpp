// 動いた場所を差分で検出
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace boost;

void draw_contours(IplImage *img_bin, IplImage *img_dst);
void diff(IplImage *src_img1, IplImage *src_img2, IplImage *diff);

int main(int argc, char* argv[]) {
  IplImage *img = NULL;
  CvCapture *capture = NULL;
  capture = cvCreateCameraCapture(0);
  //capture = cvCaptureFromAVI("test.mov");
  if(capture == NULL){
    cerr << "capture device not found!!" << endl;
    return -1;
  }
  
  img = cvQueryFrame(capture);
  CvSize size = cvSize(img->width, img->height);
  IplImage *img_gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_gray_p = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_diff = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_contour = cvCreateImage(size, IPL_DEPTH_8U, 3);

  char winNameCapture[] = "Capture";
  char winNameDiff[] = "Diff";
  char winNameContour[] = "Contour";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameDiff, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameContour, CV_WINDOW_AUTOSIZE);
  
  while (1) {
    img = cvQueryFrame(capture);
    cvShowImage(winNameCapture, img);

    cvCvtColor(img, img_gray, CV_BGR2GRAY);
    diff(img_gray, img_gray_p, img_diff);
    cvShowImage(winNameDiff, img_diff);

    cvSetZero(img_contour);
    draw_contours(img_diff, img_contour);
    cvShowImage(winNameContour, img_contour);

    cvCopy(img_gray, img_gray_p);
    if (cvWaitKey(30) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameDiff);
  cvDestroyWindow(winNameContour);
  return 0;
}

void draw_contours(IplImage *img_bin, IplImage *img_dst){
  // 輪郭抽出、青線で囲む
  CvSeq* find_contour = NULL;
  CvMemStorage* storage = cvCreateMemStorage(0);
  int num = cvFindContours(cvCloneImage(img_bin), storage, &find_contour,
				   sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
  if(num < 1) return;
  cout << str(format("contour %d") % num) << endl;
  CvScalar green = CV_RGB(0,255,0);
  cvDrawContours(img_dst, find_contour, green, green, 1, 2, 8, cvPoint(0,0));
}

void diff(IplImage *img_gray1, IplImage *img_gray2, IplImage *imgDiff){
  cvAbsDiff(img_gray1, img_gray2, imgDiff);
  cvThreshold(imgDiff, imgDiff, 10, 255, CV_THRESH_BINARY);
  int dilateErode = img_gray1->width / 80;
  cvDilate(imgDiff, imgDiff, NULL, dilateErode); // 膨張
  cvErode(imgDiff, imgDiff, NULL, dilateErode*2); // 収縮
  cvDilate(imgDiff, imgDiff, NULL, dilateErode); // 膨張
}
