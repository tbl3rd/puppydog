OPENCV := ../opencv-tutorials/opencv/install
RESOURCES := ../opencv-tutorials/resources
LIBS := \
-lopencv_core \
-lopencv_highgui \
-lopencv_imgproc \
-lopencv_objdetect \
-lopencv_video \
#

CXXFLAGS := -g -O0
CXXFLAGS := -O3
CXXFLAGS += -I./include -I$(OPENCV)/include
CXXFLAGS += -L$(OPENCV)/lib $(LIBS)

EXECUTABLE := ./source/main
ARGS := 0
ARGS := $(RESOURCES)/TownCentreXVID.avi ./output.avi

main: $(EXECUTABLE)

help: main
	DYLD_LIBRARY_PATH=$(OPENCV)/lib:$$DYLD_LIBRARY_PATH $(EXECUTABLE)

test: main
	DYLD_LIBRARY_PATH=$(OPENCV)/lib:$$DYLD_LIBRARY_PATH \
	$(EXECUTABLE) $(ARGS)

clean:
	rm -rf $(EXECUTABLE) *.dSYM

debug: main
	DYLD_LIBRARY_PATH=$(OPENCV)/lib:$$DYLD_LIBRARY_PATH \
	lldb $(EXECUTABLE) -- $(ARGS)

.PHONY: main help test clean debug
