# Define the compiler to use, standard GNU C++ compiler
CXX = g++

# Define compiler flags: use C++17 standard as requested in the program linkage, enable all warnings, and add debugging info
CXXFLAGS = -std=c++17 -Wall -Wextra -g -pthread -I..

TARGETS = server client

# The default target that runs when you just type 'make' in the console
all: $(TARGETS)

server: server.cpp
	$(CXX) server.cpp -o server

client: client.cpp
	$(CXX) client.cpp -o client

# client: client.cpp
# 	$(CXX) $(CXXFLAGS) client.cpp -o client

clean:
	rm -f $(TARGETS)