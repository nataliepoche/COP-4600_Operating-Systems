#include <thread>
#include <iostream>

void foo(int id) {
    std::cout << "My id is " << id << std::endl;
}

int main(){
    std::cout << "Spwaning a thread." << std::endl;

    // Create a thread. The first argument is the function that the thread should execute.
    // The second argument will be passed to the function itself.
    std::thread th1(foo, 4);

    // Wait for the threads to finish executing before continuing.
    th1.join();

    std::cout << "Thread has finished executing." << std::endl;

    return 0;
}