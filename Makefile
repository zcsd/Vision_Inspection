# Makefile for pylon5 and opencv
CXX ?= g++

PYLON_ROOT ?= /opt/pylon5
# pylon
CXXFLAGS   := $(shell $(PYLON_ROOT)/bin/pylon-config --cflags)
LDFLAGS    := $(shell $(PYLON_ROOT)/bin/pylon-config --libs-rpath)
LDLIBS     := $(shell $(PYLON_ROOT)/bin/pylon-config --libs)
# opencv
CXXFLAGS   += $(shell pkg-config --cflags opencv)
LDLIBS     += $(shell pkg-config --libs --static opencv)

TARGET = pylon_cv
OBJECTS = main.o frame_grabber.o

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

main.o: main.cc cvui.h
	$(CXX) -std=c++11 $(CXXFLAGS) -c -o $@ $<

frame_grabber.o: frame_grabber.cc frame_grabber.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(TARGET)

.PHONY: tidy
tidy:
	$(RM) $(OBJECTS)
