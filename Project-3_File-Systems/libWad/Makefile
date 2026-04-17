# Define the compiler
CXX = g++

# Define the compilation flags
CXXFLAGS = -Wall -Werror -std=c++11 -D_FILE_OFFSET_BITS=64

# Default target
all: libWad.a

# Compile the Wad.cpp file into an object file
Wad.o: Wad.cpp Wad.h
	$(CXX) $(CXXFLAGS) -c Wad.cpp -o Wad.o

# Archive the object file into a static library
libWad.a: Wad.o
	ar rcs libWad.a Wad.o

# Clean target to remove compiled binaries
clean:
	rm -f *.o *.a