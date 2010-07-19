// オプティカルフロー(ブロックマッチング)
// http://opencv.jp/sample/optical_flow.html

#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace boost;

void detect_flow(IplImage *img, IplImage *img_p, IplImage *dst);

int main(int argc, char* argv[]) {
  IplImage *img = NULL;
  CvCapture *capture = NULL;
  capture = cvCreateCameraCapture(0);
  //capture = cvCaptureFromAVI("test.mov");
  if(capture == NULL){
    cerr << "capture device not found!!" << endl;
    return -1;
  }

  CvSize size = cvSize(320, 240);
  IplImage *img_resized = cvCreateImage(size, IPL_DEPTH_8U, 3);
  IplImage *img_gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_gray_p = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_dst = cvCreateImage(size, IPL_DEPTH_8U, 3);

  char winNameCapture[] = "Capture";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  
  while (1) {
    img = cvQueryFrame(capture);
    cvResize(img, img_resized);
    cvCvtColor(img_resized, img_gray, CV_BGR2GRAY);
    cvCopy(img_resized, img_dst);
    detect_flow(img_gray, img_gray_p, img_dst);
    cvShowImage(winNameCapture, img_dst);
    cvCopy(img_gray, img_gray_p);
    if (cvWaitKey(10) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  
  return 0;
}

void detect_flow(IplImage *src_img1, IplImage *src_img2, IplImage *dst_img){
  int i, j, dx, dy, rows, cols;
  int block_size = 24;
  int shift_size = 10;
  CvMat *velx, *vely;
  CvSize block = cvSize(block_size, block_size);
  CvSize shift = cvSize(shift_size, shift_size);
  CvSize max_range = cvSize(50, 50);

  rows = int(ceil (double (src_img1->height) / block_size));
  cols = int(ceil (double (src_img1->width) / block_size));
  velx = cvCreateMat(rows, cols, CV_32FC1);
  vely = cvCreateMat(rows, cols, CV_32FC1);
  cvSetZero(velx);
  cvSetZero(vely);

  cvCalcOpticalFlowBM(src_img1, src_img2, block, shift, max_range, 0, velx, vely);

  for (i = 0; i < velx->width; i++) {
    for (j = 0; j < vely->height; j++) {
      dx = (int)cvGetReal2D(velx, j, i);
      dy = (int)cvGetReal2D(vely, j, i);
      cvLine(dst_img, cvPoint(i * block_size, j * block_size),
              cvPoint(i * block_size + dx, j * block_size + dy), CV_RGB(255, 0, 0), 1, CV_AA, 0);
    }
  }

}
