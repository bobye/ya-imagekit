include make.rules

# Programs
# CC=
# CXX=clang++

# Flags
CFLAGS= -O2 
INCLUDES=-Iinclude/ -I$(OPENCV_PATH)/include -I/usr/X11/include \
	-Isrc/segmentation/FelzenszwalbP04 \
	-Isrc/segmentation/ComanicuM02MeanShift \
	-I$(MOSEK)/h

LIBRARIES=-L$(OPENCV_PATH)/lib \
	-lopencv_core -lopencv_imgproc -lopencv_highgui\
	-L$(MOSEK)/bin -lmosek64 -pthread\
	-framework accelerate
#	 -framework OpenGL

# Files which require compiling
C_SOURCE_FILES=\
	src/d2_solver_mosek.c\
	src/blas_like.c

CPP_SOURCE_FILES=\
	src/imageProcessor.cc\
	src/imageProcessor_segmentation.cc\
	src/imageProcessor_resample.cc\
	src/imageProcessor_superGradient.cc\
	src/segmentation/FelzenszwalbP04/segment-image-opencv.cc\
	src/segmentation/ComanicuM02MeanShift/msImageProcessor_interface.cc

CPP_SOURCE_3RDPARTY=\
	src/segmentation/ComanicuM02MeanShift/RAList.cpp\
	src/segmentation/ComanicuM02MeanShift/ms.cpp\
	src/segmentation/ComanicuM02MeanShift/msImageProcessor.cpp\
	src/segmentation/ComanicuM02MeanShift/rlist.cpp\
	src/segmentation/ComanicuM02MeanShift/msSysPrompt.cpp

# Source files which contain a int main(..) function
SOURCE_FILES_WITH_MAIN=src/extractColorFactors.cc

ALL_OBJECTS=\
	$(patsubst %.c,%.o,$(C_SOURCE_FILES))\
	$(patsubst %.cc,%.o,$(CPP_SOURCE_FILES)) \
	$(patsubst %.cpp,%.o,$(CPP_SOURCE_3RDPARTY))\
	$(patsubst %.cc,%.o,$(SOURCE_FILES_WITH_MAIN))

DEPENDENCY_FILES=\
	$(patsubst %.o,%.d,$(ALL_OBJECTS)) 

all: bin/extractColorFactors

%.o: %.c Makefile
	@# Make dependecy file
	$(CC) -MM -MT $@ -MF $(patsubst %.c,%.d,$<) $(CFLAGS) $(INCLUDES) $<
	@# Compile
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

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
	install_name_tool -change  @loader_path/libmosek64.7.0.dylib  @loader_path/../../../../mosek/7/tools/platform/osx64x86/bin/libmosek64.7.0.dylib $@

-include $(DEPENDENCY_FILES)

.PHONY: clean
clean:
	$(RM) */*.d
	$(RM) */*.o
	$(RM) bin/extractColorFactors
