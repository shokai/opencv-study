#include <stdio.h>
#include <iostream>
#include <cv.h>
#include <highgui.h>

using namespace std;

int main(int argc, char** argv) {
  if(argc < 3){
    cerr << "args error" << endl;
    cerr << "./compositeImages /path/to/base_image img2 img3 [...] imgN" << endl;
    return 1;
  }
  CvCapture *capture;
  IplImage *img, *img_b;
  img = cvLoadImage(argv[1]);
  printf(argv[0]);
  
  char *winName = "CompositeImages";
  cvNamedWindow(winName, CV_WINDOW_AUTOSIZE);
  
  img_b = cvLoadImage(argv[2]);
  cvCopy(img_b, img);
  
  cvShowImage(winName, img);
  

  while (1) {
    if (cvWaitKey(33) == 'q') break;
  }
  
  cvDestroyWindow(winName);
  return 0;
}
