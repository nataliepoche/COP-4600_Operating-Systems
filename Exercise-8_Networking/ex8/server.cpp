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

void send_message(int fd, const char* message) { // Executes the send message, wanted to make writing it shorter
    send(fd, message, strlen(message), 0); // Actually send the send message
}

ssize_t receive_message(int fd, char* buffer, size_t buffer_size) { // Execute the receive function, wanted to make it shorter
    memset(buffer, 0, buffer_size); // Wipes the byffer strictly clean again to safely prepare to receive the user's name input
    ssize_t result = recv(fd, buffer, buffer_size - 1, 0); // Reads the data transmitted from the client via the socket into the buffer, leaving 1 byte safe for the null terminator
    return result; // Returns the result
}

int main(int argc, char const* argv[]) { // The main execution function, accepting command-line arguments (argc) and values (arvc)
    int max_messages = 20; // declares and initializes an integer to hold the maximum allowed messages on the wall, defaulting to 20
    int port = 5514; // Declares and initializes an integer to hold the server's listening port number, defaulting to 5514
    
    if (argc >= 2) max_messages = std::stoi(argv[1]); // Checks if at least 1 extra argument was passed, if so converts the first argument to an integer to set the queue sie
    if (argc >= 3) port = std::stoi(argv[2]); // Checks if at least 2 extra arguments were passed; if so, converts the second argument to an integer to set the custom port number

    std::cout << "Wall server running on port " << port << " with queue size " << max_messages << "." << std::endl; // Prints an initial startup status message to the server console to aid in debugging

    int server_fd; // Declares an integer variable to act as the master file descriptor for the server's passive listening socket
    int client_fd; // Declares an integer variable to act as the file descriptor for activelt connected individual clients
    struct sockaddr_in address; // Declares a specialized networking structure to hold the server's internal IP addres and port congiguration
    int option = 1; // declares an integer option flag set to 1, used in setsockopt to forcefully configure the socket to reuse the address
    socklen_t addrlength = sizeof(address); // Declares and initializes a variable path with the byte size of the address structure, which is required by accept()
    char buffer[1024] = {0}; // Declares a character array of 1024 bytes initializes entirely to zero, used as a temporary buffer for incoming client data
    std::deque<std::string> wall_queue; // declares a C++ deque (double-ended queue) of strings to hold the active messages on the wall in a strict FIFO manner

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Creates an IPv4 TCP socket and checks if the resulting file descriptor is less than 0 (0 indicates an error)
        std::cerr << "Socket creation failed" << std::endl; // Pritns an error messafe to the standard error console if the underlying system could not allocate a socket
        return -1; // Exits the entire program with an error code to signal failure to the operating system
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) { // Configures the socket to forcefully reuse the port to prevent "addres already in use" errors during rapid restart testing
        std::cerr << "setsockopt failed" << std::endl; // Prints an error message to the console if the low-level socket options could not be applied
        return -1; // Exits the entire program with an error code to signal failure
    }

    address.sin_family = AF_INET; // Sets the address structure's family protocol to IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Configures the server to accept connections automatically on and all available network itnerferences (0.0.0.0)
    address.sin_port = htons(port); // Converts the port variable dom standard host byte order to network byte order (big-endian) and assigns it to the address structure

    if(bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) { // Binds the initializes socket file descriptor to the configrured IP address and port, checking for a failure code
        std::cerr << "Bind failed" << std::endl; // Prints an error message to the console if binding the socket is rejected by the operating system
        return -1; // Exits the enrite program with an error code to signal failure
    }

    if (listen(server_fd, 3) < 0) { // Marks the socket as passive, telling it to activelt listening for incoming connections with a meaximum pending backlog queue of 3
        std::cerr << "Listen failed" << std::endl; // Prints an error message if the socket cannot successfully enter the listening state
        return -1; // Exits the entire program with an error code to signal failure
    }

    bool server_running = true; // Declares a boolean flag to control the outermost main server loop, leeping the server active until the 'kill' command explicitly switches it off
    while(server_running) { // Begins the main infinite loop that allows the server to continuouslt wait for and accept new clients one after another
        if((client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlength)) < 0) { // Blocks execution until a client connects, accets that connection into client_fd, and checks for system errors
            std::cerr << "Accept failed" << std::endl; // Prints an error message to the server console if accepting the incoming client connecgtion request fails
            continue; // Skips the rest of the specified loop iteration and goes immediately back to the top to try accepting a new connection again
        }

        bool client_connected = true; // Declares a boolean flag to manage the inner communication loop for the single user currently connected to the server
        while (client_connected) { // Begins the inner loop that repreatedly prompts and processes commands from the current user until they intentionally disconnect
            send_message(client_fd, WALL_HEADER); // Sends the static "Wall Contents" header string imported from Messages.h to the connect client
            if(wall_queue.empty()) { // Checks if the double-ended queue holding the wall messages currently contains exactly zero items
                send_message(client_fd, EMPTY_MESSAGE); // If the queue is empty, send the "[NO MESSAGES..]" fallback string from Messages.h to the client
            }
            else { // Begins the alternative else block for when the wall actually contains one or more messages
                for (const std::string& msg : wall_queue) { // Uses a range-based for loop to iterate through every stored string message currently inside the wall queue
                    send(client_fd, msg.c_str(), msg.length(), 0); // Converts the C++ std::string into a raw C-string and send the message text to the client over the socket
                    send(client_fd, "\n", 1, 0); // Sends a single newline character immeditaely after the message to ensure proper vertical line formatting on the client's screen
                }
            }

            send_message(client_fd, COMMAND_PROMPT); // Sned the command prompt form Messages.h to ask the user what action they want to take next
            ssize_t bytes_read = receive_message(client_fd, buffer, sizeof(buffer)); // Reads the data transmitted from client via socket into the buffer, leaving 1 byte safe for null terminator

            if (bytes_read <= 0) { // Checks if the client disconnected abruptly, crashed, or if a sever network error occured (causes recv to return 0 or less)
                client_connected = false; // Toggles the inner connection loop flag to false so the current client session terminates immediately
                continue; // Skips the remainder of the command processing logic and forces the inner loop to evaluate the false condition and exit
            }

            trim_newline(buffer); // Calls the custom helper function to strip any trailing newlines (\n) or carriage returns (\r) injected by netcat or the client script
            if(strcmp(buffer, "clear") == 0) { // Compares the cleaned incoming buffer text to see if it exactly matches the specific command word "clear"
                wall_queue.clear(); // Clears all items completely out of the C++ double-ended queue, effectively resetting the wall to the empty state
                send_message(client_fd, CLEAR_MESSAGE); // Sends the wall cleared success message in Messages.h back to the client
            }

            else if (strcmp(buffer, "post") == 0) { // Compares the cleaned incoming buffer text to see if it exactly matches the specific command word "post"
                send_message(client_fd, NAME_PROMPT); // Sends the enter name prompt to the clent to begin the posting phase
                receive_message(client_fd, buffer, sizeof(buffer)); // Resets the buffer and gets input from the socket into the cleared buffer
                trim_newline(buffer); // Strips the inherent newline or carriage return from the user's inputted name to prevent formatting bugs

                std::string name(buffer); // Constructs a standard C++ std::string object using the raw C-string buffer containing the user's name
                if (name.length() > (80 - 2)) { // Checks to ensure the inputted name is under 78 characters to exclude ": " to keep the 80 character maximum
                    send_message(client_fd, ERROR_MESSAGE); // Sends error message if true
                }
                else{
                    int max_post_len = 80 - name.length() - 2; // Dynamically calculates the remaining allowed characters by subtracting the nmelength and the ": " separrator from the absolute 80 maximum limit

                    std::string promt_max = std::string(POST_PROMPT1) + std::to_string(max_post_len) + std::string({POST_PROMPT2}); // Combines the prompt segments from Messages.h with the integer to customize the post prompt
                    send(client_fd, promt_max.c_str(), promt_max.length(), 0); // Sends the customized warning to the connected client

                    receive_message(client_fd, buffer, sizeof(buffer)); // Wipes the data and gets the message
                    trim_newline(buffer); // Strips the final newline or carriage return from the user's inputted post text

                    std::string post_text(buffer); // Constructs a standard C++ std::string object using the raw C-string buffer contianing the post text

                    if (post_text.length() > (size_t)max_post_len) { // Cgecks if the length of the typed message exceed the dynamically calculated max characters aowe
                        send_message(client_fd, ERROR_MESSAGE); // If the message is too long, it aborts the post and send the error message from Message.h
                    }
                    else { // Fallback block for when the message successfully passes the length validation
                        std::string full_post = name + ": " + post_text; // OCmbines the username, colon space separator, and the message into a single string
                        if(wall_queue.size() >= (size_t)max_messages) { // Checks if the wall queque has been reached or exceeded the max allowed queue capacity 
                            wall_queue.pop_front(); // Removes the single oldest message from the front of the FIFO queue to make room for the new entry
                        }
                        wall_queue.push_back(full_post); // Adds the new pull post string to the back of the double-ended FIFO queue
                        send_message(client_fd, SUCCESS_MESSAGE); // Send the success message from Message.h
                    }
                }
            }
            else if(strcmp(buffer, "kill") == 0) { // Compares the cleaned incoming buffer text to see if it matches the command word "kill"
                send_message(client_fd, KILL_MESSAGE); // Sends the shutdoen message from Messages.h
                client_connected = false; // Toggles the inner client communication loop flag to false to disconnect the current user
                server_running = false; // Toggles the outer main sever loop flag to falsem sensuring the entire program breaks out and shuts down
            }
            else if(strcmp(buffer, "quit") == 0) { // Compares the cleaned incomming buffer to see if it matches the "quit" word command
                send_message(client_fd, QUIT_MESSAGE); // Sends the goodbye message from message.h
                client_connected = false; // Toggles the inner client loop flag to false, to close the current socket but leaves the server running for the next person
            }
        }
        close(client_fd); // Properly closes and releases the specific file descriptor for the client that just disconnected, freeing operating systme resources
    }
    close(server_fd); // Properly closes the main listening socket descriptor permanatly as the server is shutting down
    return 0;
}