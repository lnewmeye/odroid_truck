# This is a makefile for testing OpenCV

TARGET = odroid_truck

CXX=g++
CFLAGS = -std=c++11
LFLAGS = /usr/local/lib
LIBS = -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_imgproc -lopencv_calib3d
CLASSES = truck.cpp

all: $(TARGET).cpp
	$(CXX) $(CLASSES) $(TARGET).cpp $(CFLAGS) -o $(TARGET) -L$(LFLAGS) $(LIBS)

clean:
	rm -rf $(TARGET)
