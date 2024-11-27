# Simple Makefile for WiFi Simulation

# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I./include -lpthread

# Executable name
TARGET = wifi_simulation

# Source files
SOURCES = src/wificom.cpp main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compiling
%.o: %.cpp include/wificom.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Phony targets
.PHONY: all clean