#include <iostream> // Standard I/O library to read and print data
#include <string> // String library to use std::string data type

int main() {
    std::string line; // Declares a string variable named line to hold each input line
    int operationCount = 1; // Initialize an integer counter to track current operation number, starts at 1

    // Read lines from standard input (piped from part1.o)
    while (std::getline(std::cin, line)) { // Loops continuosly, reading one line from standard input into 'line' until end of file (EOF)
        // Check if the current line contains the word "failed"
        if (line.find("failed") != std::string::npos) { // Checks if the "failed" exists within the current line
            std::cout << "Program failed on operation " << operationCount << "\n"; // Prints the required failure message with the current operation number
            break; //Exits the while loop since the failure point has been found
        }
        operationCount++; // Increases the operation counter by 1 for the next successful operation
    }

    return 0;
}