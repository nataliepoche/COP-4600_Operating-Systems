#include <iostream> // Includes the standard C++ input/output stream library
#include <sys/socket.h> // Includes definitions for socket programming functions and data structures
#include <arpa/inet.h> // Includes definitions for internet operations, specifically IP address conversion functions
#include <unistd.h> // Includes POSIX standard operating system API functions like read, write, and close
#include <cstring> // Includes C-style string manipulation functions like strlen
#include <cstdlib> // Includes standard C library functions like atoi for integer conversion

using namespace std; // Brings the standard namespace into scope so we don't need the std:: prefix

int main(int argc, char const *argv[]) { // Defines the main entry point of the program, accepting command-line arguments
    if (argc != 2) { // Checks if the number of arguments provided is exactly 2 (the program name and the port number)
        cerr << "Usage: ./client <port>" << endl; // Prints the correct usage syntax to standard error if arguments are incorrect
        return 1; // Exits the program with a non-zero status code indicating an error
    } // Closes the argument checking if-statement block

    int port = atoi(argv[1]); // Converts the port number provided in the command line from a character string to an integer
    int sock = 0; // Declares and initializes an integer variable to store the client socket file descriptor
    struct sockaddr_in serv_addr; // Declares a structure variable to hold the IP address and port number of the target server
    const char *message = "John Doe: 12345678"; // Defines a constant character pointer holding your name and UFID to send to the server
    char buffer[1024] = {0}; // Declares a character array of 1024 bytes and initializes all elements to zero to act as an input buffer

    sock = socket(AF_INET, SOCK_STREAM, 0); // Creates an IPv4 TCP socket and assigns its file descriptor to the sock variable
    if (sock < 0) { // Checks if the socket creation function failed to return a valid file descriptor
        cerr << "Socket creation error" << endl; // Prints an error message to standard error indicating socket creation failure
        return 1; // Exits the program with a non-zero status code indicating an error
    } // Closes the socket creation checking if-statement block

    serv_addr.sin_family = AF_INET; // Sets the address family of the target server address structure to IPv4
    serv_addr.sin_port = htons(port); // Converts the target port number from host byte order to network byte order and stores it

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) { // Converts the localhost IP string ("127.0.0.1") to a binary network structure and checks for failure
        cerr << "Invalid address or Address not supported" << endl; // Prints an error message if the IP address translation fails
        return 1; // Exits the program with a non-zero status code indicating an error
    } // Closes the address translation checking if-statement block

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { // Attempts to establish a TCP connection to the server at the configured IP and port
        cerr << "Connection Failed" << endl; // Prints an error message to standard error if the connection attempt is refused or times out
        return 1; // Exits the program with a non-zero status code indicating an error
    } // Closes the connection attempt checking if-statement block

    send(sock, message, strlen(message), 0); // Transmits your defined name and UFID message over the network to the connected server
    cout << "Message sent" << endl; // Prints a confirmation message to standard output indicating the message was successfully transmitted

    read(sock, buffer, 1024); // Reads the response data sent by the server into the local buffer array, up to 1024 bytes
    cout << "Server says: " << buffer << endl; // Prints the message received from the server out to the standard output console

    close(sock); // Closes the client communication socket to clean up resources and terminate the connection
    return 0; // Exits the program successfully with a zero status code
} // Closes the main function block