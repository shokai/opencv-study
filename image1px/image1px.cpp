#include <stdio.h>
#include <cv.h>
#include <highgui.h>

int main(int argc, char** argv) {
  CvCapture *capture;
  IplImage *img, *img1px, *imgR, *imgG, *imgB, *img2px;
  img1px = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 3);
  imgR = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);
  imgG = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);
  imgB = cvCreateImage(cvSize(1,1), IPL_DEPTH_8U, 1);
  img2px = cvCreateImage(cvSize(2,2), IPL_DEPTH_8U, 3);
  
  char *winNameCapture = "Capture";
  char *winName1px = "Image1px";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winName1px, CV_WINDOW_AUTOSIZE);
  capture = cvCreateCameraCapture(0);

  while (1) {
    img = cvQueryFrame(capture);
    cvShowImage(winNameCapture, img);
    cvResize(img, img1px, CV_INTER_CUBIC);
    cvSplit(img1px, imgB, imgG, imgR, NULL);
    printf("r:%d, g:%d, b:%d\n", 
	   (unsigned char)imgR->imageDataOrigin[0],
	   (unsigned char)imgG->imageDataOrigin[0],
	   (unsigned char)imgB->imageDataOrigin[0]);
    cvResize(img1px, img2px, CV_INTER_CUBIC);
    cvShowImage(winName1px, img2px);
    if (cvWaitKey(33) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winName1px);
  return 0;
}
