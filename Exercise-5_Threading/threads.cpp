#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>

// Thread function that takes an ID and a target number
void target_search(int id, int target){
    // Continuously generate random numbers 0-9999
    while(true){
        int random = rand() % 10000;

        // Checks if random number matches target
        if (random == target){
            std::cout << "Thread " << id << " completed.\n";
            break; // Exit loop once target is found
        }
    }
}

int main(int argc, char* argv[]){
    
    // 1. Get target from command-line argument
    // Assume target is valid, no check needed
    int target = std::stoi(argv[1]);

    // Initialize vector to store and manage threads
    std::vector<std::thread> threads;

    // 2. Spawn 10 threads inside a loop
    for(int i = 0; i < 10; i++){
        //Creaate a thread, passing function, the unique id (0-9), and target
        threads.push_back(std::thread(target_search, i, target));
    }

    // Wait for all threads to finish execuing before continuing
    for (int i = 0; i < 10; ++i){
        threads[i].join();
    }

    // 3. Print the final message once all threads have finished
    std::cout << "All threads have finished finding number.\n";

    return 0;
}