#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <boost/format.hpp>

using namespace std;
using namespace boost;

int main(int argc, char **argv)
{
  const int INIT_TIME = 50;
  const double BG_RATIO = 0.02; // 背景領域更新レート
  const double OBJ_RATIO = 0.005; // 物体領域更新レート
  const double Zeta = 10.0;
  IplImage *img = NULL;

  CvMemStorage* storage_contour = cvCreateMemStorage(0);
  CvSeq* find_contour = NULL;

  CvCapture *capture = NULL;
  capture = cvCreateCameraCapture(0);
  //capture = cvCaptureFromAVI("test.avi");
  if(capture == NULL){
    cerr << "capture device not found!!" << endl;
    return -1;
  }

  img = cvQueryFrame(capture);
  CvSize size = cvSize(img->width, img->height);

  IplImage *imgAverage = cvCreateImage(size, IPL_DEPTH_32F, 3);
  IplImage *imgSgm = cvCreateImage(size, IPL_DEPTH_32F, 3);
  IplImage *imgTmp = cvCreateImage(size, IPL_DEPTH_32F, 3);
  IplImage *img_lower = cvCreateImage(size, IPL_DEPTH_32F, 3);
  IplImage *img_upper = cvCreateImage(size, IPL_DEPTH_32F, 3);
  IplImage *imgSilhouette = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *imgSilhouetteInv = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *imgResult = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *imgContour = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *imgSilhouette_p = cvCreateImage(size, IPL_DEPTH_8U, 1);

  cout << "背景初期化中..." << endl;
  cvSetZero(imgAverage);
  for(int i = 0; i < INIT_TIME; i++){
    img = cvQueryFrame(capture);
    cvAcc(img, imgAverage);
    cout << str(format("輝度平均 %d/%d") % i % INIT_TIME) << endl;
  }
  cvConvertScale(imgAverage, imgAverage, 1.0 / INIT_TIME);
  cvSetZero(imgSgm);
  for(int i = 0; i < INIT_TIME; i++){
    img = cvQueryFrame(capture);
    cvConvert(img, imgTmp);
    cvSub(imgTmp, imgAverage, imgTmp);
    cvPow(imgTmp, imgTmp, 2.0);
    cvConvertScale(imgTmp, imgTmp, 2.0);
    cvPow(imgTmp, imgTmp, 0.5);
    cvAcc(imgTmp, imgSgm);
    cout << str(format("輝度振幅 %d/%d") % i % INIT_TIME) << endl;
  }
  cvConvertScale(imgSgm, imgSgm, 1.0 / INIT_TIME);
  cout << "背景初期化完了" << endl;

  char winNameCapture[] = "Capture";
  char winNameSilhouette[] = "Silhouette";
  char winNameContour[] = "Contour";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameSilhouette, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameContour, CV_WINDOW_AUTOSIZE);
  
  bool isStop = false;
  while(1){
    if(!isStop){
      img = cvQueryFrame(capture);
      if(img == NULL) break;
      cvConvert(img, imgTmp);

      // 輝度範囲
      cvSub(imgAverage, imgSgm, img_lower);
      cvSubS(img_lower, cvScalarAll(Zeta), img_lower);
      cvAdd(imgAverage, imgSgm, img_upper);
      cvAddS(img_upper, cvScalarAll(Zeta), img_upper);
      cvInRange(imgTmp, img_lower, img_upper, imgSilhouette);

      // 輝度振幅
      cvSub(imgTmp, imgAverage, imgTmp);
      cvPow(imgTmp, imgTmp, 2.0);
      cvConvertScale(imgTmp, imgTmp, 2.0);
      cvPow(imgTmp, imgTmp, 0.5);

      // 背景領域を更新
      cvRunningAvg(img, imgAverage, BG_RATIO, imgSilhouette);
      cvRunningAvg(imgTmp, imgSgm, BG_RATIO, imgSilhouette);

      // 物体領域を更新
      cvNot(imgSilhouette, imgSilhouetteInv);
      cvRunningAvg(imgTmp, imgSgm, OBJ_RATIO, imgSilhouetteInv);

      cvErode(imgSilhouette, imgSilhouette, NULL, 2); // 収縮
      cvDilate(imgSilhouette, imgSilhouette, NULL, 4); // 膨張
      cvErode(imgSilhouette, imgSilhouette, NULL, 2); // 収縮
      cvAnd(imgSilhouette, imgSilhouette_p, imgResult);
      
      // 輪郭抽出、青線で囲む
      int contour_num = cvFindContours(cvCloneImage(imgResult), storage_contour, &find_contour,
				       sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, 
				       cvPoint(0,0));
      CvScalar white = CV_RGB(255,255,255);
      cvSetZero(imgContour);
      cvDrawContours(imgContour, find_contour, white, white, 2, 2, 8, cvPoint(0,0));
      cvNot(imgContour, imgContour);

      cvShowImage(winNameCapture, img);
      cvShowImage(winNameSilhouette, imgResult);
      cvShowImage(winNameContour, imgContour);
      cvCopy(imgSilhouette, imgSilhouette_p);
    }
    int waitKey = cvWaitKey(33);
    if(waitKey == 'q') break;
    if(waitKey == ' '){
      isStop = !isStop;
      if(isStop) cout << "stop" << endl;
      else cout << "start" << endl;
    }
  }  
    
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameSilhouette);
  cvDestroyWindow(winNameContour);
  return 0;
}
  
