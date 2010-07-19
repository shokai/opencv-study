#include "cv.h"
#include "highgui.h"
#include <boost/program_options.hpp>
#include <iostream>
using namespace boost;
using namespace std;

void detect_draw(IplImage *img, IplImage *dst, CvHaarClassifierCascade *cascade, CvScalar color, bool isFlip);

int main(int argc, char* argv[]) {
  program_options::options_description opts("options");
  opts.add_options()
    ("help,h", "ヘルプを表示")
    ("cascade,c", program_options::value<string>(), "haarcascade設定ファイル")
    ("input,i", program_options::value<string>(), "入力画像ファイル名")
    ("output,o", program_options::value<string>(), "出力ファイル名")
    ("preview,p", "プレビュー表示")
    ("flip,f","左右反転した画像も判定(青でマーク)")
    ("nogray", "グレースケール、ヒストグラム均一化せずに判定");
  program_options::variables_map argmap;
  program_options::store(parse_command_line(argc, argv, opts), argmap);
  program_options::notify(argmap);
  if (argmap.count("help") || !argmap.count("cascade") || !argmap.count("input")) {
    cerr << "cascadeとinputが必要です" << endl;
    cerr << opts << endl;
    return 1;
  }
  
  CvHaarClassifierCascade *cascade;
  cascade = (CvHaarClassifierCascade*)cvLoad(argmap["cascade"].as<string>().c_str(), 0, 0, 0);
  if(!cascade){
    cerr << "error! Cascade not Found" << endl;
    return -1;
  }
  
  IplImage *image = cvLoadImage(argmap["input"].as<string>().c_str());
  IplImage *image_orig = cvCreateImage(cvSize(image->width, image->height), 8, 3);
  cvCopy(image, image_orig);
  if(!image){
    cerr << "error! Image File not Found" << endl;
    return -11;
  }

  if(!argmap.count("nogray")){
    IplImage* gray = cvCreateImage(cvSize(image->width, image->height), 8, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);
    cvEqualizeHist(gray, gray);
    image = gray;
  }
  
  detect_draw(image, image_orig, cascade, CV_RGB(255, 0, 0), false);
  if(argmap.count("flip")){
    cvFlip(image, image);
    detect_draw(image, image_orig, cascade, CV_RGB(0, 0, 255), true);
  }
  
  if(argmap.count("output")){
    string out_filename = argmap["output"].as<string>();
    cout << "save! " << out_filename << endl;
    cvSaveImage(out_filename.c_str(), image_orig);
  }
  
  if(argmap.count("preview")){
    char winName[] = "haarcascade test";
    cvNamedWindow(winName, CV_WINDOW_AUTOSIZE);
    cvShowImage(winName, image_orig);
    while (1) {
      if (cvWaitKey(1) == 'q') break;
    }
    cvDestroyWindow(winName);
  }
  
  cvReleaseImage(&image);
  cvReleaseImage(&image_orig);
  return 0;
}

void detect_draw(IplImage *img, IplImage *dst, CvHaarClassifierCascade *cascade, CvScalar color, bool isFlip = false){
  CvMemStorage *storage = 0;
  storage = cvCreateMemStorage(0);
  CvSeq* faces = cvHaarDetectObjects(img, cascade, storage,
                                     1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
                                      cvSize(30, 30));

  for(int i = 0; i < faces->total; i++){
    CvRect *rect = (CvRect*)cvGetSeqElem(faces, i);
    cout << "x:" << rect->x << ", y:" << rect->y
         << ", width:" << rect->width << ", height:" << rect->height << endl;
    CvPoint center;
    center.x = rect->x + rect->width/2.0;
    center.y = rect->y + rect->height/2.0;
    if(isFlip){
      center.x = dst->width - center.x;
      center.y = dst->height - center.y;
    }
    int r = (rect->width + rect->height)/4.0;
    cvCircle(dst, center, r, color, 2, CV_AA, 0);
  }
}
