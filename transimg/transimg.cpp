#include "cv.h"
#include "highgui.h"
#include <boost/program_options.hpp>
#include <boost/random.hpp>
#include <ctime>
#include <iostream>
using namespace boost;
using namespace std;

int main(int argc, char* argv[]) {
  program_options::options_description opts("options");
  opts.add_options()
    ("help", "ヘルプを表示")
    ("hue,h", program_options::value<int>(), "Hue")
    ("satulation,s", program_options::value<int>(), "Saturation")
    ("value,v", program_options::value<int>(), "Value")
    ("input,i", program_options::value<string>(), "入力画像ファイル名")
    ("output,o", program_options::value<string>(), "出力ファイル名")
    ("random,r", "HSVをランダム変更")
    ("width", program_options::value<double>(), "横幅の拡大倍率")
    ("height", program_options::value<double>(), "高さの拡大倍率")
    ("preview,p", "プレビュー表示");
  program_options::variables_map argmap;
  program_options::store(parse_command_line(argc, argv, opts), argmap);
  program_options::notify(argmap);
  if (argmap.count("help") || !argmap.count("input")) {
    cerr << "入力画像を指定してください" << endl;
    cerr << opts << endl;
    return 1;
  }
  
  IplImage *img = cvLoadImage(argmap["input"].as<string>().c_str());
  if(!img){
    cerr << "error! Image File not Found" << endl;
    return 1;
  }

  if(!argmap.count("preview")&&!argmap.count("output")){
    cerr << "" << endl;
    exit(1);
  }

  int h = 0;
  int s = 0;
  int v = 0;
  double width = 1.0;
  double height = 1.0;
  if(argmap.count("hue")) h = argmap["hue"].as<int>();
  if(argmap.count("satulation")) s = argmap["satulation"].as<int>();
  if(argmap.count("value")) v = argmap["value"].as<int>();
  if(argmap.count("width")) width = argmap["width"].as<double>();
  if(argmap.count("height")) height = argmap["height"].as<double>();

  if(argmap.count("random")){
    mt19937 gen(static_cast<unsigned long>(time(0)));
    uniform_real<> dst(-1, 1);
    variate_generator<mt19937, uniform_real<> >rand(gen, dst);
    if(!argmap.count("hue")) h = 180;
    if(!argmap.count("satulation")) s = 180;
    if(!argmap.count("value")) v = 180;
    if(!argmap.count("width")) width = 0.3;
    if(!argmap.count("height")) height = 0.3;
    h *= rand();
    s *= rand();
    v *= rand();
    width = 1.0+width*rand();
    height = 1.0+height*rand();
  }
  cout << "h: " << h << endl;
  cout << "s: " << s << endl;
  cout << "v: " << v << endl;
  cout << "width: " << width << endl;
  cout << "height: " << height << endl;

  if(width != 1.0 || height != 1.0){
    IplImage *imgResized = cvCreateImage(cvSize((int)img->width*width, (int)img->height*height), IPL_DEPTH_8U, 3);
    cvResize(img, imgResized, CV_INTER_CUBIC);
    img = imgResized;
  }

  IplImage *imgHsv = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
  IplImage *imgResult = cvCreateImage(cvSize(img->width, img->height), IPL_DEPTH_8U, 3);
  IplImage *imgHue = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  IplImage *imgSat = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  IplImage *imgVal = cvCreateImage(cvSize(img->width,img->height), IPL_DEPTH_8U, 1);
  cvCvtColor(img, imgHsv, CV_BGR2HSV);
  cvSplit(imgHsv, imgHue, imgSat, imgVal, NULL);
  cvAddS(imgHue, cvScalarAll(h), imgHue, NULL);
  cvAddS(imgSat, cvScalarAll(s), imgSat, NULL);
  cvAddS(imgVal, cvScalarAll(v), imgVal, NULL);
  cvMerge(imgHue, imgSat, imgVal, NULL, imgHsv);
  cvCvtColor(imgHsv, imgResult, CV_HSV2BGR);

  if(argmap.count("output")){
    string out_filename = argmap["output"].as<string>();
    cout << "save! " << out_filename << endl;
    cvSaveImage(out_filename.c_str(), imgResult);
  }
  
  if(argmap.count("preview")){
    char winName[] = "transimg";
    cvNamedWindow(winName, CV_WINDOW_AUTOSIZE);
    cvShowImage(winName, imgResult);
    while (1) {
      if (cvWaitKey(1) == 'q') break;
    }
    cvDestroyWindow(winName);
  }
  return 0;
}
