#include "MemoryManager.h"  // Include our header file for the class definition
#include <fcntl.h>          // Include for POSIX file contol options like O_RDWR, O_CREAT
#include <unistd.h>         // Inclde for POSIX standard library calls like write() and close()
#include <sys/mman.h>       // Include for POSIX mmap() and munmap() for extra credit OS-level alliocation
#include <cmath>            // Include for math functions like ceil() used in bitmap byte calculations
#include <string>           // Include for string manipulation used in formatting the dumpMemoryMap output

// Constructor initializes the default state of the manager
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator) {
    this->wordSize = wordSize;      // Set the word size for byte/word conversion
    this->allocator = allocator;    // Set the default allocation algorithm
    this->physicalMem = nullptr;    // Initialize memory pointer to null to indicate uninitialized state
    this->head = nullptr;           // Initialize tracking list head to null
    this->totalWords = 0;           // Initialize capacity tracking to 0
}

// Destructor ensures memeory is not leaked when the object is destroyed
MemoryManager::~MemoryManager() {
    shutdown();                     // Call shutdown to release all mapped memory and delete tracking nodes
}

// Extra Credit Helper: Creates the initial tracking node without using 'new' inside initialize()
void MemoryManager::generateInitialNode(size_t sizeInWords) {
    // Dynmaically allocate the first node representing the entirety of the memory block as one large hole.
    head = new MemoryNode{0, (uint16_t)sizeInWords, true, nullptr};
}

// Initializes the physical memory block and resets the tracking list
void MemoryManager::initialize(size_t sizeInWords) {
    if (physicalMem != nullptr) { // Checls of memory is already initialized
        shutdown(); // Call shutdown to clean up the existing block before creating a new one
    }

    totalWords = sizeInWords; // Store the total word capacity for bounds checking and getters

    // Extra credit: Use POSIX mmap instead of 'new' or 'malloc' to acquire the initial block from the OS
    // MAP_PRIVARE and MAP_ANONYMOUS give us raw memory pages not backed by a file descriptor. PROT_READ/WRITE allows access
    physicalMem = (uint8_t*)mmap(NULL, sizeInWords * wordSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    generateInitialNode(sizeInWords); // Call out helper to create the head node
}

// Shuts down the manager, freeing mapped memory and deleting all list nodes to prevent leaks
void MemoryManager::shutdown() {
    if(physicalMem != nullptr) { // Only attempt to unmap if memory was actually initialized
        // Extra Credit: Use munmap to return the pages to the OS since they were created with mmap
        munmap(physicalMem, totalWords * wordSize);
        physicalMem = nullptr; // Reset pointer to safe state
    }

    MemoryNode* current = head; // Start at the head of the tracking list
    while(current != nullptr) { // Loop until the end of the list is reached
        MemoryNode* temp = current; // Temporarily hold the current node
        current = current->next; // Advances the pointer to the next node
        delete temp; // Safely delete the node we are leaving behind to prevent memory leaks
    }
    head = nullptr; // Reset the head pointer to safe state
    totalWords = 0; // Reset the capacity counter
}

// Allocates a chunk of memory and returns a physical pointer to the start of the space
void* MemoryManager::allocate(size_t sizeInBytes) {
    if(!physicalMem || sizeInBytes == 0) return nullptr; // Return null if unintialized or sking for 0 bytes

    // Convert requested bytes to words. (size + wordSize - 1) / wordSize is an integer math trick to round up
    int wordsNeeded = (sizeInBytes + wordSize - 1) / wordSize;

    void* holeList = getList(); // Get the current list of holes to pass to the allicator function
    int targerOffset = allocator(wordsNeeded, holeList); // Call active allocator to find the best hole offset

    // getList() dynamically allocates an array, so we must cast and delete to prevent memory leak
    delete[] static_cast<uint16_t*>(holeList);

    if(targerOffset == -1) return nullptr; // If allocator returns -1, no hole fits the request, return null

    MemoryNode* current = head; // Start iterating through the tracking list
    while(current != nullptr) { // Loop until the node ois found or hit the end
        // Look for the exact free hole node whose offset matches what the allocator returned
        if(current->isHole &&current->offset == targerOffset) {
            if (current->length > wordsNeeded) { // If the hole is larger than needed, split it
                // Create a new node for the leftover free space that immediately follows the new allocation
                MemoryNode* leftoverHole = new MemoryNode;
                leftoverHole->offset = current->offset + wordsNeeded; // New offset starts right after the allocated words
                leftoverHole->length = current->length - wordsNeeded; // The length is whatever is left over
                leftoverHole->isHole = true; // Mark it as a free hold
                leftoverHole->next = current->next; // Link it to the node that was previously following the current one

                current->next = leftoverHole; // Link our current node to this new leftover hole
                current->length = wordsNeeded; // Shrink our current ndoe to exactly the requested size
            }

            current->isHole = false; // mark our node as allocated (no longer a hole)

            // Calculate and return the exact physical byte address by multiplying word offset by word size
            return physicalMem + (current->offset * wordSize);
        }
        current = current->next; // Move to the next node if the current one does not match
    }
    return nullptr; // Failsafe return if the requested offset was somehow not found
}

// Frees previously allocated memory and merges adjacent free chunks
void MemoryManager::free(void* address) {
    if(!address || !physicalMem) return; // Failsafe check to ensure there are valid pointers

    // Convert the passed physical byte pointer back into a word offset
    uint16_t targetOffset = (static_cast<uint8_t*>(address) - physicalMem) / wordSize;

    MemoryNode* current = head; // Start the head of the list
    while (current != nullptr) { // Iterates to find the specific allocated node
        if(current->offset == targetOffset && !current->isHole){ // If offsets match and it's currently allocated
            current->isHole = true; // Mark the node as a free hole
            break; // Stop iterating as we have freed the target
        }

        current = current->next; // Move to next node
    }

    // Pass 2: Merging logic to combine adjacent holes and prevent fragmentation
    current = head; // Restart at the head of the list
    while(current != nullptr && current->next != nullptr) { // Loop while we have a current and a next node
        if (current->isHole && current->next->isHole) { // If both the current and next node are free holes
            MemoryNode* redundantNode = current->next; // Isolate the second node
            current->length += redundantNode->length; // Absorb the second node's length into the first node
            current->next = redundantNode->next; // Bypass the second node by linking current to the node after the second
            delete redundantNode; // Safely delete the bypassed node to prevent memory leaks
            // Notice: we do not advance 'current' here because the newly linked next node might also be a hole
        }
        else {
            current = current->next; // If no merge happened, it is safe to aadvance to the next node
        }
    }
}

// Updates the active allocation algorithm
void MemoryManager::setAllocator(std::function<int(int, void*)> allocator) {
    this->allocator = allocator; // Overwites the member variable with the passed function
}

// Returns a dynamically allocated 2-byte integer array formatted as [Count, Offset, Length...]
void* MemoryManager::getList() {
    if(!head) return nullptr; // If uninitialized, standard requirement dictates returning the nullptr

    std::vector<uint16_t> holeInfo; // Vector to temporarily gather offset/length pairs without knowing count beforehand
    uint16_t holeCount = 0; // Counter for the total number of holes found

    MemoryNode* current = head; // start at the head of the tracking list
    while(current != nullptr) { // Iterate through the entire memory map
        if(current->isHole) { // If the node is a hole
            holeCount++; // Increment the hole count
            holeInfo.push_back(current->offset); // Store the hole's starting offset
            holeInfo.push_back(current->length); // Store the hole's length
        }
        current = current->next; // Move to next node
    }

    // Allocate the specific array format: 1 index for the count, plus 2 indexes per hole (offset + length)
    uint16_t* listArray = new uint16_t[1 + holeInfo.size()];
    listArray[0] = holeCount; // The very first element must be the count of the holes

    // Copy the collected vector data directly into the allocated array
    for(size_t i = 0; i < holeInfo.size(); i++){
        listArray[i+1] = holeInfo[i];
    }
    
    return static_cast<void*>(listArray); // Return as a void pointer per the signature requirement
}

// Returns a byte array acting as a bit-stream where 1 = allocated word, 0 = free word
void* MemoryManager::getBitmap() {
    if (!head) return nullptr; // Return null if uninitialized

    // Calculate how many bytes we need to hold the bits (8 bits per byte). ceil() accounts for spillovers
    uint16_t byteCount = std::ceil(totalWords / 8.0);

    // Allocate array: 2 bytes for the size header + the actual byte count needed for bits
    // The () at the end zero-initializes the entire array automatically, ensuring all bits start as 0 (holes)
    uint8_t* bitmap = new uint8_t[byteCount + 2]();

    // Store the byte count size in Little-Endian format actoss the first two bytes
    bitmap[0] = byteCount & 0xFF; // Bitwise and isolates the lower 8 bits of the size
    bitmap[1] = (byteCount >> 8) & 0xFF; // Bitwise shift and isolates the upper 8 bits

    MemoryNode* current = head; // Start at the list head
    while(current != nullptr) { // Iterate through all memory chunks
        if(!current->isHole) { // Only process allocated blocks, since hoels are already 0
            for(int i = 0; i < current->length; i++) { // Iterate through every single word in this block
                int exactWordIndex = current->offset+i; // Find the exact integer index of the specific word
                int bytePosition = 2 + (exactWordIndex / 8); // Calculate which byte in the array this bit belongs to (offset by 2)
                int bitPosition = exactWordIndex % 8; // Calculates which bit inside that byte to flip

                // Use bitwise or to flip the specific bit at 'bitPosition' to 1
                bitmap[bytePosition] |= (1 << bitPosition);
            }
        }
        current = current->next; // Move to next node
    }
    return static_cast<void*>(bitmap); // Return as void pointer
}

// Writes the current list of holes to a text file using strictly low-level POSIX calls
int MemoryManager:: dummpMemoryMap(char* filename) {
    // POSIX open() call. Flags: Read/Write, Create if missing, overwrite if exists, 077 permissions
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(fd == -1) return -1; // Return -1 to indicate an error occurred opening the file

    uint16_t* holeList = static_cast<uint16_t*>(getList()); // Retrieve the current hole list array
    uint16_t totalHoles = holeList[0]; // The first element is the hole count

    std::string output = ""; // String to build the requested output format

    // Loop through each hole to build the "[offset, length]" string format
    for(int i = 0; i < totalHoles; i++){
        uint16_t holeOffset = holeList[1 + (i * 2)]; // Retrieve offset baseed on index
        uint16_t holeLength = holeList[2 + (i * 2)]; // Retrieve length based on index

        // Append formatted string
        output += "[]" + std::to_string(holeOffset) + ", " + std::to_string(holeLength) + "]";

        // If this is not the last hole, append the separator dash
        if (i < totalHoles - 1) output += " - ";
    }
    // POSIX write call. Pass he fole descriptor, a C-style string pointer, and the exact byte length
    write(fd, output.c_str(), output.length());

    // POSIX close() call to flush changes and release the fole descriptor
    close(fd);

    delete[] holeList; // Clean up the dynamically allocted list array from getList() to prevent leaks

    return 0; // Return 0 to indicate success
}

// Getter: Returns the assigned word size
unsigned MemoryManager::getWordSize() {
    return wordSize;
}

// Fetter: Returns the phsycial memory array pointers
void* MemoryManager::getMemoryStart() {
    return physicalMem;
}

// Getter: Returns the maximum capacity of the block in bytes (words * wordSize)
unsigned MemoryManager::getMemoryLimit() {
    return totalWords * wordSize;
}

// Global Allocation Algorithms

// Iterates through holes to find the smallest hole that fits the requested size
int bestFit(int sizeInWords, void* list) {
    uint16_t* holeArray = static_cast<uint16_t*>(list); // Cast list back to readable uint16_t array
    uint16_t holeCount = holeArray[0]; // Read the total number of holes

    int optimalOffset = -1; // Defaultt to -1 if no fit is found
    int smallestFitLength = -1; // Tracks the smallest hole size that successfully fits the  request

    for(int i = 0; i < holeCount; i++) { // Iterates through all provided holes
        int currentOffset = holeArray[1 + (i * 2)]; // Extract offset
        int currentLength = holeArray[2 + (i * 2)]; // Extract length

        // If hole is big enough and (this is first fit or it's smaller than the previous best fit)
        if(currentLength >= sizeInWords && (smallestFitLength == -1 || currentLength < smallestFitLength)) {
            optimalOffset = currentOffset; // update the best offset
            smallestFitLength = currentLength; // Update the new target to beat
        }
    }
    return optimalOffset; // Returns the offset, or -1 if unchanged
}

// Iterates through holes to find the largest hole that fits the requested size
int worstFit(int sizeInWords, void* list) {
    uint16_t* holeArray = static_cast<uint16_t*>(list); // Cast list back to readable uint16_t array
    uint16_t holeCount = holeArray[0]; // Read the total number of holes

    int worstOffset = -1; // Default to -1 if no fits is found
    int largestFitLength = -1; // Tracks the largest hole size found so far

    for(int i = 1; i < holeCount; i++) {
        int currentOffset = holeArray[1 + (i * 2)]; // Extract offset
        int currentLength = holeArray[2 + (i * 2)]; // Extract length

        // If hole is big enough and it is larger than our previous largest hole
        if (currentLength >= sizeInWords && currentLength > largestFitLength) {
            worstOffset = currentOffset; // Update worst offset
            largestFitLength = currentLength; // Update new target to beat
        }
    }
    return worstOffset; // Returns the offset, or -1 if unchanged.
}