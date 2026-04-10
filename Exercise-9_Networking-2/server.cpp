#include <iostream> // Includes the standard C++ input/output stream library
#include <sys/socket.h> // Includes definitions for socket programming functions and data structures
#include <netinet/in.h> // Includes definitions for internet domain address structures like sockaddr_in
#include <unistd.h> // Includes POSIX standard operating system API functions like read, write, and close
#include <cstring> // Includes C-style string manipulation functions like strlen
#include <cstdlib> // Includes standard C library functions like atoi for integer conversion

using namespace std; // Brings standard namespace into scope so we don't need std:: prefix

int main(int argc, char const *argv[]) { // Defines the main entry point of the program, accepting command-line arguments
    if (argc != 2) { // Checks if the number of arguments provided is exactly 2 (program name and port number)
        cerr << "Usage: ./server <port>" << endl; // Prints the correct use example if the argument is inforrect
        return 1; // Exits the program with a non-zero status to indicate an error
    }

    int port = atoi(argv[1]); // Converts the por number provided in the command line from a character string to an integer
    int server_fd; // Declares an integer variable to store the file descriptor for the server's listening socket
    int new_socket; // Declares an integer veriable to store the file descriptor for the newly accepted client connection
    struct sockaddr_in address; // Declares a structure variable to hold the IP address and port number for the server
    int option = 1; // Declares and initializes an integer option variable used for configuring socket options
    int addrlength = sizeof(address); // Deeclares and initializes an integer variable with the size of the address structure in bytes
    char buffer[1024] = {0}; // Declares a character array of 1024 bytes and initializes all elements to zero to act as an input buffer
    const char *hello = "Welcome to the server running on REPTILIAN"; // Defines a constant character pointer holding the specific welcome message

    server_fd = socket(AF_INET, SOCK_STREAM, 0); // Creates an IPv4 TCP socket and assigns its file descriptor to server_fd
    if (server_fd == 0) { // Checks if the socket creation function failed to return a valid file descriptor
        cerr << "Socket creation failed" << endl; // Prints an error message to standed error indicating socket creation failure
        return 1; // Exits the program with a non-zero status code indicating an error
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)); // Configures the socket to forecefully attach to the port and reuse the address, preventing "Address already in use" errors

    address.sin_family = AF_INET; // Sets the address family of the sockaddr_in structure to IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Cofigures the socket to bind all available local network interfaces automatically
    address.sin_port = htons(port); // Converts the port number from host byte order to network byte order and stores it in the structure

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { // Attempts to bind the socket file descriptor to the specified local port and address
        cerr << "Bind failed" << endl; // Prints an error message to standard error if the bind process fails
        return 1; // Exits the program with a non-zero status code indicating an error
    }

    if (listen(server_fd, 3) < 0) { // Places the socket in a passive mode, listeneing for incoming client connections with a backlog queue size of 3
        cerr << "Listen failed" << endl; // Prints an error message to standard error if the socket fails to enter listening mode
        return 1; // Exits the program with a non-zero status code indicating an error
    }

    //cout << "Waiting for a connection on port " << port << "..." << endl; // Prints a status message indicating the server is successfully running and waiting

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlength); // Extracts the first connection request on the queue, creating a new connected socket and returning its file descriptor
    if (new_socket < 0) { // Checks if the connection acceptance process failed
        cerr << "Accept failed" << endl; // Prints an error message to standard error if accepting the client fails
        return 1; // Exits the program with a non-zero status code indicating an error
    }

    read(new_socket, buffer, 1024); // Reads data sent by the connected client into the local buffer array, up to 1024 bytes
    //cout << "Client says: " << buffer << endl; // Prints the message recieved from the client out to the standard output console
    cout << buffer << endl; // Prints the message recieved from the client out to the standard output console

    send(new_socket, hello, strlen(hello), 0); // Transmits the defined welcome message over the network to the connected client socket
    //cout << "Welcome message sent" << endl; // prints a confirmation message to standard output indicating the message was successfully transmitted

    close(new_socket); // Closes the communication socet dedicated to this specific client connection
    close(server_fd); // Closes the main server listening socket to clean up resources
    return 0; // Exits the program successfully with a zero status code
}