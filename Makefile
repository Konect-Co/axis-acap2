AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/rules/common.mak

PROGS = spaspect

CC=/usr/local/mipsisa32r2el/r23/bin/mipsisa32r2el-axis-linux-gnu-g++
#CCFLAGS = -std=c++
OPENCVTAGS = -I/axis/axis-acap2/cppTest/opencv/build/install/include -I/axis/emb-app-sdk_2_0_3/target/mipsisa32r2el-axis-linux-gnu/usr/include
OPENCVSTATICLINKING = -LcppTest/opencv/build/install/lib
LIBS = -lopencv_core -lopencv_imgcodecs -lopencv_videoio -lopencv_imgproc -lopencv_highgui


CFLAGS += -Wall -g -O2
ifeq ($(AXIS_BUILDTYPE),host)
	LDFLAGS += -lcapturehost -ljpeg
else
	LDFLAGS += -lcapture
endif  #AXIS_BUILDTYPE == host


#OBJS = spaspect.o

all: $(PROGS)

#debug: CCFLAGS += -DDEBUG -g
#	debug: executable

$(PROGS):$(OBJS)
		$(CC) $(OPENCVTAGS) $(LDFLAGS) $^ $(OPENCVSTATICLINKING) $(LIBS) -o $@

test: executable

executable:#$(OBJS)
	$(CC) $(OPENCVTAGS) $(LDFLAGS) spaspect.cpp $(OPENCVSTATICLINKING) $(LIBS)

clean:
		rm -f $(PROGS) *.o core
		rm -f *.tar
		#rm ./a.out
