// 動いた場所を差分で検出
// 輪郭を緑で表示
// 重心を青丸で表示
#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>
#include <iostream>
#include <boost/format.hpp>
#include <Labeling.h>

using namespace std;
using namespace boost;

int draw_biggestLabel(IplImage *img_bin, IplImage *img_dst);
void draw_moment(IplImage *img_bin, IplImage *img_dst);
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
  IplImage *img_label = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_moment = cvCreateImage(size, IPL_DEPTH_8U, 3);

  char winNameCapture[] = "Capture";
  char winNameDiff[] = "Diff";
  char winNameContour[] = "Contour";
  char winNameLabel[] = "Label";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameDiff, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameContour, CV_WINDOW_AUTOSIZE);
  cvNamedWindow(winNameLabel, CV_WINDOW_AUTOSIZE);
  
  while (1) {
    img = cvQueryFrame(capture);
    cvShowImage(winNameCapture, img);

    cvCvtColor(img, img_gray, CV_BGR2GRAY);
    diff(img_gray, img_gray_p, img_diff);
    cvShowImage(winNameDiff, img_diff);

    cvSetZero(img_contour);
    draw_contours(img_diff, img_contour);
    cvShowImage(winNameContour, img_contour);

    cvSetZero(img_label);
    int size = draw_biggestLabel(img_diff, img_label);
    cout << "size : " << size << endl;
    cvCvtColor(img_label, img_moment, CV_GRAY2BGR);
    if(size < img->width * img->height / 16){
      draw_moment(img_label, img_moment);
    }
    cvShowImage(winNameLabel, img_moment);

    cvCopy(img_gray, img_gray_p);
    if (cvWaitKey(30) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  cvDestroyWindow(winNameDiff);
  cvDestroyWindow(winNameContour);
  cvDestroyWindow(winNameLabel);
  return 0;
}

int draw_biggestLabel(IplImage *img_bin, IplImage *img_dst){
  short *dst = new short[img_bin->width*img_bin->height];
  LabelingBS labeling;
  labeling.Exec((uchar *)img_bin->imageData, dst, img_bin->width, img_bin->height, true, 10);
  cout << "number of regions: " << labeling.GetNumOfRegions() << endl;
  int size = 0;
  for(int y = 0; y < img_dst->height; y++){
    for(int x = 0; x < img_dst->width; x++){
      if(dst[x+y*img_dst->width] == 1){
	img_dst->imageDataOrigin[x+y*img_dst->width] = 255;
	size++;
      }
    }
  }
  return size;
}

void draw_moment(IplImage *img_bin, IplImage *img_dst){
  CvMoments moments;
  cvMoments(img_bin, &moments, 0);
  double m00 = cvGetSpatialMoment(&moments, 0, 0);
  double m10 = cvGetSpatialMoment(&moments, 1, 0);
  double m01 = cvGetSpatialMoment(&moments, 0, 1);
  int gX = m10/m00;
  int gY = m01/m00;
  if (gX < 1 && gY < 1) return;
  cout << str(format("moment %d,%d") % gX % gY) <<endl;
  cvCircle(img_dst, cvPoint(gX, gY), 80, CV_RGB(0,0,255), 6, 8, 0);
}

void draw_contours(IplImage *img_bin, IplImage *img_dst){
  // 輪郭抽出、青線で囲む
  CvSeq* find_contour = NULL;
  CvMemStorage* storage = cvCreateMemStorage(0); // 共有する
  int num = cvFindContours(cvCloneImage(img_bin), storage, &find_contour,
				   sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
  if(num < 1 || find_contour == NULL) return;
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
