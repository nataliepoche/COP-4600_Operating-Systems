#include <iostream> // Standard I/O library for printing outputs
#include <unistd.h> // POSIX operating system API, required for fork(), pipe(), read(), write(), and close()
#include <sys/types.h> // Defines data types used in system calls, like pid_t
#include <sys/wait.h> // Conains declarations for waiting on child processes (wait())
#include <algorithm> // Conatins the std::sort function to sort the integer array
#include <cstdlib> // Contains std::atoi to conver string command-line arguments to integers

int main(int argc, char* argv[]) { // Main function, accepting command-line arguments (argument count and argument vector)
    // Ensure exactly 5 integer arguments are provided (+1 for the program name)
    if (argc != 6) { // Checks if exactly 5 arguments were passed (plus 1 for the program name itself)
        std::cerr << "Usage: ./part3.o <int1> <int2> <int3> <int4> <int5>\n"; // Prints usage instructions if argument count is wrong
        return 1; // Exits with an error code
    }

    // Initialize 4 pipes
    int p1[2]; // Parent to Child 1, delcares an array of 2 integers for Pipe 1 (Parent to Child 1)
    int p2[2]; // Child 1 to Parent, Declares an array of 2 integers for Pipe 2 (Child1 to Parent)
    int p3[2]; // Child 1 to Child 2, decares an array of 2 integers for Pipe 3 (Child 1 to Child 2)
    int p4[2]; // Child 2 to Parent, declares an array of 2 integers for Pipe 4 (Child 2 to Parent)

    // Initializes Pipes, turning px[0] into the read and px[1] into the write end
    pipe(p1); pipe(p2); pipe(p3); pipe(p4);

    pid_t pid1 = fork(); // Creates the first child process, storing the process ID

    if (pid1 == 0) { // Checks if the current executing process is Child 1 (fork() return 0 to the child)
        // --- CHILD PROCESS 1 ---
        // Close unused pipe ends
        close(p1[1]); // Parent closes read end of Pipe 1 (it only writes to Child 1)
        close(p2[0]); // Parent closes write end of Pipe 2 (it only reads from Child 1)
        close(p3[0]); // Parent closes read end of Pipe 3 (not used by Parent)
        close(p4[0]); // Parent closes write end of Pipe 3 (not used by Parent)
        close(p4[1]); // Child 1 does not use p4 at all, parent closes write end of Pipe 4 (it only reads from Child 2)

        int nums[5]; // Declares an integer array to store the 5 numbers
        read(p1[0], nums, sizeof(nums)); // Reads the 5 integers sent by Parent from Pipe 1 into the nums array
        close(p1[0]); // Closes the read end of Pipe 1 since reading is finished

        std::sort(nums, nums + 5); // Sorts the array of 5 integers in ascending order

        write(p2[1], nums, sizeof(nums)); // Send sorted list to Parent, writes the sorted array into Pipe 2 to send back to the Parent
        close(p2[1]); // Closes the write end of Pipe 2 since writing is finished

        write(p3[1], nums, sizeof(nums)); // Send sorted list to Child 2, Writes the sorted array into Pipe 3 to send to  Child 2
        close(p3[1]); // Closes the write end of the Pipe 3 since writing is finished

        return 0; // Exit Child 1 process
    }

    pid_t pid2 = fork(); // Creates the second child process from Parent process

    if (pid2 == 0) { // Checks if the current executing process is Child 2
        // --- CHILD PROCESS 2 ---
        // Close unused pipe ends
        close(p1[0]); // Closes the read end of Pipe 1 as Child 2 doesn't use it
        close(p1[1]); // Closes the write end of Pipe 1 as Child 2 doesn't use it
        close(p2[0]); // Closes the read end of Pipe 2 as Child 2 doesn't use it
        close(p2[1]); // Closes the write end of Pipe 2 as Child 2 doesn't use it
        close(p3[1]); // Closes the writ eend of Pipe 3 because Child 2 only reads from Child 1
        close(p4[0]); // Closes the read end of Pipe 4 because Child 2 onlt writes to the Parent

        int nums[5]; // Declares the integer array to store the sorted numbers
        read(p3[0], nums, sizeof(nums)); // Read sorted array send by Child 1 from Pipe 3
        close(p3[0]); // Clones the read end of Pipe 3 since reading is done

        int median = nums[2]; // Identify median (middle element of 5), extracts the 3rd element (index 2), which is the median of a sorted 5-element array

        write(p4[1], &median, sizeof(median)); // Send median to Parent, Writes the median value into Pipe 4 to send to Parent
        close(p4[1]); // Closes the write end of Pipe 4 since writing is done

        return 0; // Exit Child 2 process
    }

    // --- PARENT PROCESS ---
    // Close unused pipe ends
    close(p1[0]); // Parent closes read end of Pipe 1 (it only write to Child 1)
    close(p2[1]); // Parent closes write end of Pipe 2 (it only read from Child 1)
    close(p3[0]); // Parent closes read end of Pipe 3 (not used by Parent)
    close(p3[1]); // Parent closes write end of Pipe 3 (not used by Parent)
    close(p4[1]); // Parent closes write end of Pipe 4 (it only reads from Child 2)

    int nums[5]; // Declares an array to store the parsed command-line integers
    for (int i = 0; i < 5; ++i) { // Loops 5 tiems to process the 5 command-line arguments
        nums[i] = std::atoi(argv[i+1]); // Converts each argument from string to integer (offset by 1 to skip program name)
    } 

    write(p1[1], nums, sizeof(nums)); // Send initial 5 ints to Child 1, writes the 5 integers into Pipe 1 to send to child 1
    close(p1[1]); // closes the write end of Pipe 1 as data is sent

    int sorted_nums[5]; // Declares an array to hold the sorted numbers returned by Child 1
    read(p2[0], sorted_nums, sizeof(sorted_nums)); // Receive sorted list from Child 1, reads the sorted array from Pipe 2 into sorted_nums
    close(p2[0]); // Closes the read end of Pipe 2 as reading is finished

    int median; // declares a variable to hold the median returned by Child 2
    read(p4[0], &median, sizeof(median)); // Receive median from Child 2, reads median value from Pipe 4 into th emedian variable
    close(p4[0]); // Closes the read end of Pipe 5 as reading is finished

    // Format output with 1 space after colon, 2 spaces between numbers
    std::cout << "Sorted list of ints: "; // prints the label for the sorted list output
    for (int i = 0; i < 5; ++i) { // Loops through the sorted array to print each number
        std::cout << sorted_nums[i]; // Prints the current sorted number
        if (i < 4) std::cout << "  "; // Prints exactly 2 spaces after each number except the last one
    }
    std::cout << "\nMedian: " << median << "\n"; // Prints a newline, the "Median: " label, the edian value, and a final newline

    // Wait for both children to finish
    wait(NULL); // Suspends Parent execution until the first child process terminates
    wait(NULL); // Suspends Parent exeution until the second child process terminates

    return 0;
}