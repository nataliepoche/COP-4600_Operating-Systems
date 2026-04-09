#include <iostream> // Includes the standard C++ input/output stream library
#include <sys/socket.h> // Includes definitions for socket programming functions and data structures
#include <netinet/in.h> // Includes definitions for internet domain address structures like sockaddr_in
#include <unistd.h> // Includes POSIX standard operating system API functions like read, write, and close
#include <cstring> // Includes C-style string manipulation functions like strlen
#include <cstdlib> // Includes standard C library functions like atoi for integer conversion

using namespace std; // Brings standard namespace into scope so we don't need std:: prefix

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        cerr << "Usage: ./server <port>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    int server_fd;
    int new_socket;
    struct sockaddr_in address;
    int option = 1;
    int addrlength = sizeof(address);
    char buffer[1024] = {0};
    const char *hello = "Welcome to the server running on REPTILIAN";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed" << endl;
        return 1;
    }

    cout << "Waiting for a connection on port " << port << "..." << endl;

    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlength);
    if (new_socket < 0) {
        cerr << "Accept failed" << endl;
        return 1;
    }

    read(new_socket, buffer, 1024);
    cout << "Client says: " << buffer << endl;

    send(new_socket, hello, strlen(hello), 0);
    cout << "Welcome message sent" << endl;

    close(new_socket);
    close(server_fd);
    return 0;
}