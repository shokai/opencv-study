# Mac用Makefile
SRC = transimg.cpp
DST = transimg.bin

prefix=/opt/local
INCPATH=$(prefix)/include
LIBPATH=$(prefix)/lib

CV_LIBS= -lcv -lcvaux -lcxcore -lhighgui
BOOST_LIBS= $(LIBPATH)/libboost_program_options-mt.dylib

all:
	g++ -O $(SRC) -o $(DST) -I$(INCPATH)/opencv -L. -L$(LIBPATH) $(CV_LIBS) -I$(INCPATH)/boost $(BOOST_LIBS)

