# Define the compiler to use, standard GNU C++ compiler
CXX = g++

# Define compiler flags: use C++17 standard as requested in the program linkage, enable all warnings, and add debugging info
CXXFLAGS = -std=c++17 -Wall -Wextra -g -pthread -I..

TARGETS = wallserver

# The default target that runs when you just type 'make' in the console
all: $(TARGETS)

wallserver: server.cpp Messages.h
	$(CXX) $(CXXFLAGS) server.cpp -o wallserver

# client: client.cpp
# 	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f $(TARGETS)