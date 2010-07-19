#include <stdio.h>
#include <highgui.h>

int main(int argc, char** argv) {
  CvCapture *capture;
  IplImage *frameImage;
  char windowNameCapture[] = "Capture";
  
  capture = cvCreateCameraCapture(0);
  cvNamedWindow(windowNameCapture, CV_WINDOW_AUTOSIZE);
  
  while (1) {
    frameImage = cvQueryFrame(capture);
    
    cvShowImage(windowNameCapture, frameImage);
    if (cvWaitKey(33) == 'q') break;
  }
  
  cvReleaseCapture(&capture);
  cvDestroyWindow(windowNameCapture);
  
  return 0;
}
