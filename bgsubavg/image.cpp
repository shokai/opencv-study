#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  bool isStop = false;
  const int INIT_TIME = 50;
  const double BG_RATIO = 0.02; // 背景領域更新レート
  const double OBJ_RATIO = 0.005; // 物体領域更新レート
  const double Zeta = 10.0;
  IplImage *img = NULL;

  CvCapture *capture = NULL;
  capture = cvCreateCameraCapture(0);
  //capture = cvCaptureFromAVI("test.avi");
  if(capture == NULL){
    printf("capture device not found!!");
    return -1;
  }

  img = cvQueryFrame(capture);
  int w = img->width;
  int h = img->height;

  IplImage *imgAverage = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 3);
  IplImage *imgSgm = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 3);
  IplImage *imgTmp = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 3);
  IplImage *img_lower = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 3);
  IplImage *img_upper = cvCreateImage(cvSize(w, h), IPL_DEPTH_32F, 3);
  IplImage *imgSilhouette = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgSilhouetteInv = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);
  IplImage *imgResult = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);

  printf("背景初期化中...\n");
  cvSetZero(imgAverage);
  for(int i = 0; i < INIT_TIME; i++){
    img = cvQueryFrame(capture);
    cvAcc(img, imgAverage);
    printf("輝度平均 %d/%d\n", i, INIT_TIME);
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
    printf("輝度振幅 %d/%d\n", i, INIT_TIME);
  }
  cvConvertScale(imgSgm, imgSgm, 1.0 / INIT_TIME);
  printf("背景初期化完了\n");

  char winNameCapture[] = "Capture";
  char winNameSilhouette[] = "Silhouette";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameSilhouette, CV_WINDOW_AUTOSIZE);

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

      cvErode(imgSilhouette, imgSilhouette, NULL, 1); // 収縮
      cvDilate(imgSilhouette, imgSilhouette, NULL, 2); // 膨張
      cvErode(imgSilhouette, imgSilhouette, NULL, 1); // 収縮

      cvMerge(imgSilhouette, imgSilhouette, imgSilhouette, NULL, imgResult);
      cvShowImage(winNameCapture, img);
      cvShowImage(winNameSilhouette, imgResult);
    }
    int waitKey = cvWaitKey(33);
    if(waitKey == 'q') break;
    if(waitKey == ' '){
      isStop = !isStop;
      if(isStop) printf("stop\n");
      else printf("start\n");
    }
  }

  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameSilhouette);

  return 0;
}
