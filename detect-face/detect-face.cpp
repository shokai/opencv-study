#include "cv.h"
#include "highgui.h"
#include <boost/program_options.hpp>
#include <iostream>

using namespace boost;
using namespace std;

int main(int argc, char* argv[]) {
  program_options::options_description opts("options");
  opts.add_options()
    ("help,h", "ヘルプを表示")
    ("cascade,c", program_options::value<string>(), "haarcascade設定ファイル")
    ("input,i", program_options::value<string>(), "画像ファイル名")
    ("output,o", program_options::value<string>(), "出力ファイル名")
    ("preview,p", "プレビュー表示");
  
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
  if(!image){
    cerr << "error! Image File not Found" << endl;
    return -11;
  }
  
  CvMemStorage *storage = 0;
  storage = cvCreateMemStorage(0);
  
  CvSeq* faces = cvHaarDetectObjects(image, cascade, storage,
                                     1.1, 2, CV_HAAR_DO_CANNY_PRUNING,
                                      cvSize(30, 30) );
  for(int i = 0; i < faces->total; i++){
    CvRect *rect = (CvRect*)cvGetSeqElem(faces, i);
    CvPoint center;
    center.x = rect->x + rect->width/2.0;
    center.y = rect->y + rect->height/2.0;
    int r = (rect->width + rect->height)/4.0;
    cvCircle(image, center, r, CV_RGB(255, 0, 0), 2, CV_AA, 0);
  }

  if(argmap.count("output")){
    string out_filename = argmap["output"].as<string>();
    cout << "save! " << out_filename << endl;
    cvSaveImage(out_filename.c_str(), image);
  }
  
  if(argmap.count("preview")){
    char winName[] = "detect face";
    cvNamedWindow(winName, CV_WINDOW_AUTOSIZE);
    cvShowImage(winName, image);
    while (1) {
      if (cvWaitKey(1) == 'q') break;
    }
    cvDestroyWindow(winName);
  }
  
  cvReleaseImage(&image);
  return 0;
}
