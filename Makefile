CC=/usr/local/mipsisa32r2el/r23/bin/mipsisa32r2el-axis-linux-gnu-g++
CCFLAGS = -std=c++
OPENCVTAGS = -IcppTest/opencv/build/install/include
OPENCVSTATICLINKING = -LcppTest/opencv/build/install/lib
LIBS = -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_imgproc -lopencv_highgui

all: executable

debug: CCFLAGS += -DDEBUG -g
	debug: executable

executable:
		$(CC) $(CCFLAGS) $(OPENCVTAGS) core.cpp $(OPENCVSTATICLINKING) $(LIBS)

clean:
		rm a.out
