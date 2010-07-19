SRC = hough_line.cpp
DST = hough_line.bin

CV_INCPATH=/opt/local/include/opencv
CV_LIBPATH=/opt/local/lib 

CV_LIBS= -lcv -lcvaux -lcxcore -lhighgui 

all:
	g++ -O $(SRC) -o $(DST) -I$(CV_INCPATH) -L. -L$(CV_LIBPATH) $(CV_LIBS)
