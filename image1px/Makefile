SRC = image1px.cpp
DST = image1px.bin

prefix=/opt/local
CV_INCPATH=$(prefix)/include/opencv
CV_LIBPATH=$(prefix)/lib 

CV_LIBS= -lcv -lcvaux -lcxcore -lhighgui 

all:
	g++ -O $(SRC) -o $(DST) -I$(CV_INCPATH) -L. -L$(CV_LIBPATH) $(CV_LIBS)
