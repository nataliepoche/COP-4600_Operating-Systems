#include "./Messages.h"
#include <string>
#include <iostream> // Includes standard C++ input/output stream library for console logging
#include <cstring> // Includes the C-string library needed for functions like strlen() and memset()
#include <sys/socket.h> // Includes the core socket library containing socket(), bind(), and listen() definitions
#include <netinet/in.h> // Includes strictires for internet domain addresses like sockaddr_in
#include <unistd.h> // Includes POSIX operating system API functions, such as close()
#include <deque> // Includes the double-edged queue container from the Standard Temple Library to act as the FIFO wall queue

#define PORT 8080 // Defines a preprocessor macro to set the port number the server wil bind to

void trim_newline(char* str) { // Calculates the current function that take sa C-string pointer to remove trailing newling or carriage return characters sent by netcat
    int length = strlen(str); // Calculates the current length of the provided string to determine where the end is
    while (length > 0 && (str[length - 1] == '\n' || str[length - 1] == '\r')) { //Loops backwards from the end of the string as long as there are newline or return characters
        str[length - 1] = '\0'; // Replaces the newline or carriage return character with a null terminator to trunicate the string
        length--; //Decrements the length variable to check th enext preceeding charater in the next loop iteration
    }
}

void send_message(int fd, const char* message) {
    send(fd, message, strlen(message), 0);
}

ssize_t receive_message(int fd, char* buffer, size_t buffer_size) {
    memset(buffer, 0, buffer_size);
    ssize_t result = recv(fd, buffer, buffer_size - 1, 0);
    return result;
}

int main(int argc, char const* argv[]) { // The main execution function, accepting command-line arguments (argc) and values (arvc)
    int max_messages = 20;
    int port = 5514;
    
    if (argc >= 2) max_messages = std::stoi(argv[1]);
    if (argc >= 3) port = std::stoi(argv[2]);

    std::cout << "Wall server running on port " << port << " with queue size " << max_messages << "." << std::endl;

    int server_fd;
    int client_fd;
    struct sockaddr_in address;
    int option = 1;
    socklen_t addrlength = sizeof(address);
    char buffer[1024] = {0};
    std::deque<std::string> wall_queue;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
        std::cerr << "setsockopt failed" << std::endl;
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return -1;
    }

    bool server_running = true;
    while(server_running) {
        if((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlength)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        bool client_connected = true;
        while (client_connected) {
            send_message(client_fd, WALL_HEADER);
            if(wall_queue.empty()) {
                send_message(client_fd, EMPTY_MESSAGE);
            }
            else {
                for (const std::string& msg : wall_queue) {
                    send(client_fd, msg.c_str(), msg.length(), 0);
                    send(client_fd, "\n", 1, 0);
                }
            }

            send_message(client_fd, COMMAND_PROMPT);

            ssize_t bytes_read = receive_message(client_fd, buffer, sizeof(buffer));

            if (bytes_read <= 0) {
                client_connected = false;
                continue;
            }

            trim_newline(buffer);
            if(strcmp(buffer, "clear") == 0) {
                wall_queue.clear();
                send_message(client_fd, CLEAR_MESSAGE);
            }

            else if (strcmp(buffer, "post") == 0) {
                send_message(client_fd, NAME_PROMPT);
                receive_message(client_fd, buffer, sizeof(buffer));
                trim_newline(buffer);

                std::string name(buffer);
                if (name.length() > (80 - 2)) {
                    send_message(client_fd, "Error: name is too long!\n\n");
                }
                else{
                    int max_post_len = 80 - name.length() - 2;

                    std::string promt_max = std::string(POST_PROMPT1) + std::to_string(max_post_len) + std::string({POST_PROMPT2});
                    send(client_fd, promt_max.c_str(), promt_max.length(), 0);

                    receive_message(client_fd, buffer, sizeof(buffer));
                    trim_newline(buffer);

                    std::string post_text(buffer);

                    if (post_text.length() > (size_t)max_post_len) {
                        send_message(client_fd, ERROR_MESSAGE);
                    }
                    else {
                        std::string full_post = name + ": " + post_text;
                        if(wall_queue.size() >= (size_t)max_messages) {
                            wall_queue.pop_front();
                        }
                        wall_queue.push_back(full_post);
                        send_message(client_fd, SUCCESS_MESSAGE);
                    }
                }
            }
            else if(strcmp(buffer, "kill") == 0) {
                send_message(client_fd, KILL_MESSAGE);
                client_connected = false;
                server_running = false;
            }
            else if(strcmp(buffer, "quit") == 0) {
                send_message(client_fd, QUIT_MESSAGE);
                client_connected = false;
            }
        }
        close(client_fd);
    }
    close(server_fd);
    return 0;
}