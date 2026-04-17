# Define the compiler
CXX = g++

# Define the compilation flas, explicitly requesting 64-bit offsets and FUSE 26
CXXFLAGS = -Wall -Werror -std=c++11 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26

# Default target
all: wadfs

# Compilee the daemon, linking against FUSE and the custom libWad archive
wadfs: wadfs.cpp
	$(CXX) $(CXXFLAGS) wadfs.cpp -o wadfs -lfuse -L./libWad -lWad

# Clean target to remove the executable
clean:
	rm -f wadfs