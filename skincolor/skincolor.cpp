/***
 肌色部分を検出
 参考: opencvプログラミングブック p.59
***/

#include <stdio.h>
#include <highgui.h>
#include <cv.h>

const int COLOR_TOP = 17;
const int COLOR_BOTTOM = 15;

int main(int argc, char** argv) {
  bool isStop = false;
  CvCapture *capture = NULL;
  capture = cvCreateCameraCapture(0);
  //capture = cvCaptureFromAVI("dnp-shokai1.avi");
  if(capture == NULL){
    printf("capture device not found!!");
    return -1;
  }

  IplImage *img = NULL;
  img = cvQueryFrame(capture);
  const int w = img->width;
  const int h = img->height;

  IplImage *imgBg = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgGray = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgDiff = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgHsv = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
  IplImage *imgHue = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgSat = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgVal = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgThreshold1 = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgThreshold2 = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgThreshold3 = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgSkin = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);

  char winNameCapture[] = "Capture";
  char winNameDiff[] = "Difference";
  char winNameBg[] = "Background";
  char winNameSkin[] = "Skin";

  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameDiff, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameBg, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameSkin, CV_WINDOW_AUTOSIZE);

  img = cvQueryFrame(capture);
  cvCvtColor(img, imgBg, CV_BGR2GRAY);  

  int waitKey;
  while (1) {
    if(!isStop){
      if((img = cvQueryFrame(capture)) == NULL) break;
      cvCvtColor(img, imgGray, CV_BGR2GRAY);
      cvAbsDiff(imgGray, imgBg, imgDiff);

      cvCvtColor(img, imgHsv, CV_BGR2HSV);
      cvSplit(imgHsv, imgHue, imgSat, imgVal, NULL);
      cvThreshold(imgHue, imgThreshold1, COLOR_BOTTOM, 255, CV_THRESH_BINARY);
      cvThreshold(imgHue, imgThreshold2, COLOR_TOP, 255, CV_THRESH_BINARY_INV);
      cvAnd(imgThreshold1, imgThreshold2, imgThreshold3, NULL);

      cvAnd(imgDiff, imgThreshold3, imgSkin, NULL);

      cvShowImage(winNameCapture, img);
      cvShowImage(winNameDiff, imgDiff);
      cvShowImage(winNameBg, imgBg);
      cvShowImage(winNameSkin, imgSkin);
    }

    waitKey = cvWaitKey(33);
    if(waitKey == 'q') break;
    if(waitKey == 'b'){ // 背景再取得
      img = cvQueryFrame(capture);
      cvCvtColor(img, imgBg, CV_BGR2GRAY);
    }
    if(waitKey == ' '){
      isStop = !isStop;
      if(isStop) printf("stop\n");
      else printf("start\n");
    }
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameDiff);
  cvDestroyWindow(winNameBg);
  cvDestroyWindow(winNameSkin);
  return 0;
}

