#include <iostream> // Standard I/O linrary for printing to the console
#include <fstream> // Includes file steam library to read from the named pip (FIFO)
#include <string> // String library for text manipulation

int main() {
    // Open the named pipe as a standard file
    std::ifstream pipeFile("ex7_Pipe"); // Opens the named pipe "ex7_Pipe" as an input file stream
    
    if (!pipeFile.is_open()) { // Checks if the file stream failed to open the named pipe (aka, if the named pipe exists)
        std::cerr << "Error: Could not open named pipe 'ex7_Pipe'." << std::endl; // Prints error message to the standard error stream
        return 1; // Exits the program with error code 1
    }

    std::string line; // Declares a string to store each line read from the pipe
    int operationCount = 1; // Initializes a counter for the operations, starting with 1

    while (std::getline(pipeFile, line)) { // Reads line-by-line from the named pipe until it's empty or closed
        if (line.find("failed") != std::string::npos) { // Checks if the current line contains the word "failed"
            std::cout << "Program failed on operation " << operationCount << "\n"; // Outputs the failure operation number
            break; // Stops reading further lines once the failure is found
        }
        operationCount++; // Increases the counter for every successful operation read
    }

    pipeFile.close(); // Closesthe file stream connected to th enamed pipe to free resources
    return 0; // Returns 0 to indicate the program finished successfully
}