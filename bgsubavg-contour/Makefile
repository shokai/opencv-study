SRC = image.cpp
DST = image.bin

prefix=/opt/local
INCPATH=$(prefix)/include
LIBPATH=$(prefix)/lib 

OPT= -lcv -lcvaux -lcxcore -lhighgui 

CC=g++ -O

CFLAGS= -I$(INCPATH)/opencv
LDFLAGS=-L. -L$(LIBPATH) 


all:
	$(CC) $(SRC)  -o $(DST) $(CFLAGS)  $(LDFLAGS) $(OPT)
