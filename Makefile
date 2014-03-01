include make.rules

# Programs
CC=
CXX=g++

# Flags
CFLAGS= -O2
INCLUDES=-Iinclude/ -I$(OPENCV_PATH)/include \
	-Isrc/segmentation/FelzenszwalbP04 \
	-Isrc/segmentation/ComanicuM02MeanShift

LIBRARIES=-L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

# Files which require compiling
SOURCE_FILES=src/colorfactor.cc\
	src/segmentation/FelzenszwalbP04/segment-image-opencv.cc\
	src/segmentation/ComanicuM02MeanShift/msImageProcessor_interface.cc

SOURCE_3RDPARTY=\
	src/segmentation/ComanicuM02MeanShift/RAList.cpp\
	src/segmentation/ComanicuM02MeanShift/ms.cpp\
	src/segmentation/ComanicuM02MeanShift/msImageProcessor.cpp\
	src/segmentation/ComanicuM02MeanShift/rlist.cpp\
	src/segmentation/ComanicuM02MeanShift/msSysPrompt.cpp

# Source files which contain a int main(..) function
SOURCE_FILES_WITH_MAIN=src/extractColorFactors.cc

ALL_OBJECTS=\
	$(patsubst %.cc,%.o,$(SOURCE_FILES)) \
	$(patsubst %.cpp,%.o,$(SOURCE_3RDPARTY))\
	$(patsubst %.cc,%.o,$(SOURCE_FILES_WITH_MAIN))

DEPENDENCY_FILES=\
	$(patsubst %.o,%.d,$(ALL_OBJECTS)) 

all: bin/extractColorFactors

%.o: %.cc Makefile
	@# Make dependecy file
	$(CXX) -MM -MT $@ -MF $(patsubst %.cc,%.d,$<) $(CFLAGS) $(INCLUDES) $<
	@# Compile
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

%.o: %.cpp Makefile
	@# Make dependecy file
	$(CXX) -MM -MT $@ -MF $(patsubst %.cpp,%.d,$<) $(CFLAGS) $(INCLUDES) $<
	@# Compile
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

bin/extractColorFactors: $(ALL_OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDES) $(ALL_OBJECTS) -o $@ $(LIBRARIES) 

-include $(DEPENDENCY_FILES)

.PHONY: clean
clean:
	$(RM) */*.d
	$(RM) */*.o
	$(RM) bin/extractColorFactors
