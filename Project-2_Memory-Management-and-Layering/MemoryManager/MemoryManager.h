#ifndef MEMORY_MANAGER_H    // Preprocessor directive to prevent double inclusion of this header file
#define MEMORY_MANAGER_H    // Define the macro to mark this file as included

#include <functional>       // Required to use std::function for passing the allicator algorithms
#include <vector>           // Required for using standard vectors to dynamically collect hole data
#include <cstdint>          // Required to use specifically sized integer types like uint16_t, uint8_t
#include <cstddef>          // Required to use the size_t type for standard byte sizing

// Transfer all files <put -r *>
// Test the code

// Struct defining a node in our single linked list to track memory segments
struct MemoryNode {
    uint16_t offset;        // Tracks the starting location of this memory chunk in words (0 to 65535)
    uint16_t length;        // Tracks the size of this memory chunk in words
    bool isHole;            // Boolean flag: True means is available (hole), false means in use (block)
    MemoryNode* next;       // Pointer to the next node in the physical memory sequence to form a linked list
};

// Main class representing the memory management system
class MemoryManager {
    private:
        unsigned wordSize;                              // Stores the size of one word in bytes, used for alignment and calculations
        std::function<int(int, void*)> allocator;       // Stores the active alliocation algorithm function pointer
        uint8_t* physicalMem;                           // Pointer to the start of the contiguous byte array representing our physical memory block
        size_t totalWords;                              // Stores the total cpacity of our initalized memory bblock in words
        MemoryNode* head;                               // Pointer to the first node in our linked list tracking memory state

        void generateInitialNode(size_t sizeInWords);   // Helper to bypass the "no new in initialize" rule for extra credit

    public:
        // Constructor to set native word size and default allocation algorithm
        MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
        // Destructor to release all memory and prevent memory leaks when object is destroyed
        ~MemoryManager();

        // Instantiates the main memory block of the requiested size in words
        void initialize(size_t sizeInWords);
        // Releases the memory block and cleans up all tracking nodes
        void shutdown();
        // Allocates memory based on requested bytes and returns a pointer to it
        void* allocate(size_t sizeInBytes);
        // Frees previously allocated memroy block using its pointer address
        void free(void* address);
        // Updates the allocation algorithm used by the manager
        void setAllocator(std::function<int(int,void*)> allocator);
        // Writes the list of current holes to a file using POSIX calls
        int dumpMemoryMap(char* filename);
        // Returns a little-endian array containing hole offsets and lengths
        void* getList();
        // Returns a little-endian bit-stream array where 1 us used memory and 0 is free
        void* getBitmap();
        // Returns the native word size defined at construction
        unsigned getWordSize();
        // Returns the starting memroy address of the physical memory block
        void* getMemoryStart();
        // Returns the total cpacity of the memory block in bytes
        unsigned getMemoryLimit();
};

//Global function signature for the Best Fit allocation algorithm
int bestFit(int sizeInWords, void* list);
//Global function signature for the Worst Fit allocation algorithm
int worstFit(int sizeInWords, void* list);

#endif // End of the preprocessor directive block