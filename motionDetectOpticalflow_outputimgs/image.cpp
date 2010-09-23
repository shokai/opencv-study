// オプティカルフロー(ブロックマッチング)でモーション認識
// 動いた場所を認識する
// 手ぶれを除去する
// 動画ファイルから入力して、連番画像ファイルに書き出す
#include <cv.h>
#include <highgui.h>
#include <cxcore.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <boost/format.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;

struct DPoint{
  int x;
  int y;
  int dx;
  int dy;
};

struct Rect{
  int left;
  int right;
  int top;
  int bottom;
};

void detect_flow(IplImage *img, IplImage *img_p, IplImage *dst);
tuple<string, Rect, DPoint> to_string(vector<DPoint> points, int width, int height);
string print_vector(const string& prefix, vector<DPoint> points, int width, int height, int block_size, IplImage *dst_img, CvScalar color);
bool is_bluring(vector<DPoint> points, Rect most, DPoint median, int width, int height, int block_size);

int main(int argc, char* argv[]) {
  IplImage *img = NULL;
  CvCapture *capture = NULL;
  if(argc < 2){
    capture = cvCreateCameraCapture(0);
    if(capture == NULL){
      cerr << "capture device not found!!" << endl;
      return -1;
    }
  }
  else{
    capture = cvCaptureFromAVI(argv[1]);
    if(capture == NULL){
      cerr << "video file load error!!" << endl;
      return -1;
    }
  }

  CvSize size = cvSize(320, 240);
  IplImage *img_resized = cvCreateImage(size, IPL_DEPTH_8U, 3);
  IplImage *img_gray = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_gray_p = cvCreateImage(size, IPL_DEPTH_8U, 1);
  IplImage *img_dst = cvCreateImage(size, IPL_DEPTH_8U, 3);

  char winNameCapture[] = "Capture";
  cvNamedWindow(winNameCapture, CV_WINDOW_AUTOSIZE);
  
  bool is_play = true;
  char wait_key;
  int i = 1;
  while (1) {
    if(is_play){
      img = cvQueryFrame(capture);
      cvResize(img, img_resized);
      cvCvtColor(img_resized, img_gray, CV_BGR2GRAY);
      cvCopy(img_resized, img_dst);
      detect_flow(img_gray, img_gray_p, img_dst);
      cvShowImage(winNameCapture, img_dst);
      cvCopy(img_gray, img_gray_p);
      ostringstream oss;
      oss << i << ".jpg";
      string out_filename = oss.str();
      cout << out_filename << endl;
      cvSaveImage(out_filename.c_str(), img_dst);
      i++;
    }
    wait_key = cvWaitKey(10);
    if (wait_key == 'q') break;
    if (wait_key == ' ') is_play = !is_play;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(winNameCapture);
  
  return 0;
}

void detect_flow(IplImage *src_img1, IplImage *src_img2, IplImage *dst_img){
  int width = src_img1->width;
  int height = src_img1->height;
  int i, j, dx, dy, rows, cols;
  int block_size = 20;
  int shift_size = 3;
  CvMat *velx, *vely;
  CvSize block = cvSize(block_size, block_size);
  CvSize shift = cvSize(shift_size, shift_size);
  CvSize max_range = cvSize(50, 50);

  //cols = floor((width - block_size) / shift_size); 
  //rows = floor((height - block_size) / shift_size); 

  rows = floor(ceil (double (height) / block_size));
  cols = floor(ceil (double (width) / block_size));

  //rows = int(ceil (double (height) / block_size));
  //cols = int(ceil (double (width) / block_size));
  velx = cvCreateMat(rows, cols, CV_32FC1);
  vely = cvCreateMat(rows, cols, CV_32FC1);
  cvSetZero(velx);
  cvSetZero(vely);

  cvCalcOpticalFlowBM(src_img1, src_img2, block, shift, max_range, 0, velx, vely);

  vector<DPoint> left_points, right_points, up_points, down_points;
  for (i = 0; i < velx->width; i++) {
    for (j = 0; j < vely->height; j++) {
      dx = (int)cvGetReal2D(velx, j, i); // 既にblock_sizeが乗算されてる
      dy = (int)cvGetReal2D(vely, j, i);
      int d = abs(dx) + abs(dy);
      if(0 < d && d < width*height/3000){ // 大きすぎる変化は捨てる
	DPoint p = {i*block_size, j*block_size, dx, dy};
	// 方向成分で分類
	if(p.dx > 0) left_points.push_back(p);
	else right_points.push_back(p);
	if(p.dy > 0) up_points.push_back(p);
	else down_points.push_back(p);
	
	cvLine(dst_img, cvPoint(i * block_size, j * block_size),
	       cvPoint(i * block_size + dx, j * block_size + dy), CV_RGB(255, 0, 0), 1, CV_AA, 0);
      }
    }
  }

  print_vector("left", left_points, width, height, block_size, dst_img, CV_RGB(0,0,255));
  print_vector("right", right_points, width, height, block_size, dst_img, CV_RGB(0,255,0));
  print_vector("up", up_points, width, height, block_size, dst_img, CV_RGB(0,255,255));
  print_vector("down", down_points, width, height, block_size, dst_img, CV_RGB(255,0,255));

}


// 手ぶれ認識
bool is_bluring(vector<DPoint> points, Rect most, DPoint median, int img_width, int img_height, int block_size){
  int h = most.bottom-most.top;
  int w = most.right-most.left;
  return !(points.size() >= h*w/((block_size-2)*(block_size-2)*3) && // ある程度密度が必要
	   h < (double)img_height*0.7 && // 画面全体が動いているかどうか
	   w < (double)img_width*0.7);
}


// 成分分解済みベクトルを解析する
tuple<string, Rect, DPoint> to_string(vector<DPoint> points, int width, int height){
  DPoint total = {0,0,0,0};
  string points_str;
  Rect most = {width,0,height,0};
  for(int i = 0; i < points.size(); i++){
    DPoint p = points[i];
    points_str += str(format("[%d,%d,%d,%d]") % 
		      p.x % 
		      p.y %
		      p.dx %
		      p.dy);
    total.x += p.x;
    total.y += p.y;
    total.dx += p.dx;
    total.dy += p.dy;
    if(most.left>p.x) most.left = p.x; // 最も左の点
    if(most.right<p.x) most.right = p.x;
    if(most.top>p.y) most.top = p.y;
    if(most.bottom<p.y) most.bottom = p.y;
  }
  string result_str = str(format("{points:%s}") % points_str); // 点の列挙

  result_str += str(format("{most:[%d,%d,%d,%d]}") % // 最も遠い上下左右
		    most.left %
		    most.right %
		    most.top %
		    most.bottom);

  int size = points.size();
  DPoint median = {0,0,0,0};
  if(size>0){
    DPoint median = {total.x/size, total.y/size, total.dx/size, total.dy/size};
    result_str += str(format("{median:[%d,%d,%d,%d]}") % // 平均
			    median.x % 
			    median.y %
			    median.dx %
			    median.dy);
  }
  
  return make_tuple(result_str, most, median);
}

// 手ぶれしていないvectorのみ出力
string print_vector(const string& prefix, vector<DPoint> points, int width, int height, int block_size, IplImage *dst_img, CvScalar color){
  string result_str;
  Rect most;
  DPoint median;
  tie(result_str, most, median) = to_string(points, width, height);
  if(!is_bluring(points, most, median, width, height, block_size)){
    cvRectangle(dst_img, cvPoint(most.left, most.top),
		cvPoint(most.right, most.bottom),
		color, 2, CV_AA, 0);
    cout << prefix << " : " << result_str << endl;
    return result_str;
  }
  return "";
}
