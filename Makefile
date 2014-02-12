OPENCV_PATH=/usr/local

# Programs
CC=
CXX=g++

# Flags
CFLAGS=-Wextra -Wall -pedantic-errors -O3
INCLUDES=-Iinclude/ -I$(OPENCV_PATH)/include 

LIBRARIES=-L$(OPENCV_PATH)/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

# Files which require compiling
SOURCE_FILES=

# Source files which contain a int main(..) function
SOURCE_FILES_WITH_MAIN=src/demo.cpp

ALL_OBJECTS=\
	$(SOURCE_OBJECTS) \
	$(patsubst %.cpp,%.o,$(SOURCE_FILES_WITH_MAIN))

DEPENDENCY_FILES=\
	$(patsubst %.o,%.d,$(ALL_OBJECTS)) 

all: bin/demo

%.o: %.cc Makefile
	@# Make dependecy file
	$(CXX) -MM -MT $@ -MF $(patsubst %.cc,%.d,$<) $(CFLAGS) $(INCLUDES) $<
	@# Compile
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

bin/demo: $(ALL_OBJECTS)
	$(CXX) $(CFLAGS) $(INCLUDES) $< -o $@ $(LIBRARIES)

-include $(DEPENDENCY_FILES)

.PHONY: clean
clean:
	$(RM) src/*.o
	$(RM) bin/demo
