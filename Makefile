# Simple Makefile for Roulette Simulator.
# Usage:
#   make        -> builds the "roulette" executable
#   make run    -> builds and runs it
#   make clean  -> removes build artifacts

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = roulette
SOURCE = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) roulette_statistics.txt

.PHONY: all run clean
