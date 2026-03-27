# Define the compiler to use, standard GNU C++ compiler
CXX = g++

# Define compiler flags: use C++17 standard as requested in the program linkage, enable all warnings, and add debugging info for Valgrind
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# The Fefault target that runs when you just type 'make' in the console
all: libMemoryManager.a

# Target to create the static library archive using the object file
libMemoryManager.a: MemoryManager.o
	# 'ar' is the archive utility; 'rcs' means replace/create/sort index to package the .o fiel int a .a library
	ar rcs libMemoryManager.a MemoryManager.o

# Target to compile the C++ source code into an object file (.o) without linking
MemoryManager.o: MemoryManager.cpp MemoryManager.h
	# Execute the compiler with our flags, '-c' tells it to only compile and not link, producing the .o file
	$(CXX) $(CXXFLAGS) -c MemoryManager.cpp

# Target to clean up generated files to ensure a fresh build
clean:
	# Forcibly remove (-f) all object files (.o) and archive files (.a) in the directory
	rm -f *.o *.a