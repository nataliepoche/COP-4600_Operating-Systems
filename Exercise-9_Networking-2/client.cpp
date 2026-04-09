#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

using namespace std;

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        cerr << "Usage ./client <port>" << endl;
        return 1;
    }

    int port = atoi(argv[1]);
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *message = "Natalie Poche: 15339442";
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation error" << endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address or Address not supported" << endl;
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed" << endl;
        return 1;
    }

    send(sock, message, strlen(message), 0);
    cout << "message sent" << endl;

    read(sock, buffer, 1024);
    cout << "Server says: " << buffer << endl;

    close (sock);
    return 0;
}