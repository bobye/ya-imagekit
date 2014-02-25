include make.rules

# Programs
CC=
CXX=g++

# Flags
CFLAGS= -O2
INCLUDES=-Iinclude/ -I$(OPENCV_PATH)/include 

LIBRARIES=-L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

# Files which require compiling
SOURCE_FILES=src/colorfactor.cc

# Source files which contain a int main(..) function
SOURCE_FILES_WITH_MAIN=src/extractColorFactors.cc

ALL_OBJECTS=\
	$(patsubst %.cc,%.o,$(SOURCE_FILES)) \
	$(patsubst %.cc,%.o,$(SOURCE_FILES_WITH_MAIN))

DEPENDENCY_FILES=\
	$(patsubst %.o,%.d,$(ALL_OBJECTS)) 

all: bin/extractColorFactors

%.o: %.cc Makefile
	@# Make dependecy file
	$(CXX) -MM -MT $@ -MF $(patsubst %.cc,%.d,$<) $(CFLAGS) $(INCLUDES) $<
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
