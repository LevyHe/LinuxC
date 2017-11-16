
CC       = gcc
CXX      = g++ -fPIC --shared
CFLAGS   = -g
CXXFLAGS = $(CFLAGS)

LIBPATH = ./
LIBS = -Wl,-rpath=./:./HCNetSDKCom -lhcnetsdk 
SRC = ./main.cpp
TARGET = ./libhcnetapp.so

all: 
	$(CXX) $(OPTI) $(CXXFLAGS) $(SRC) -o $(TARGET) -L$(LIBPATH) $(LIBS) 

.PHONY: clean
clean:
	rm -f $(TARGET)