#pragma once // Prevent multiple inclusions of this header
#include <string> // Include standard string library
#include <vector> // Include standard vector library
#include <map> // Include standard map library
#include <sys/types.h> // Include POSIX data types

using namespace std; // Use the standard namespace for convenience

struct Descriptor { // Define a structure to represent a WAD file descriptor
    uint32_t offset; // The physical offset of the element's data in the WAD file
    uint32_t length; // The length of the element's data in bytes
    char name[9]; // The 8-character ASCII name of the element, plus a null terminator
};

struct Node { // Define a structure to represent a file or directory in our filesystem tree
    string name; // The name of the file or directory
    bool isDir; // Boolean flag: true if directory, false if content file
    uint32_t offset; // The physical offset of the content in the WAD file (0 for directories)
    uint32_t length; // The size of the content in bytes (0 for directories)
    vector<Node*> children; // Pointers to the children nodes (if this is a directory)
    Node* parent; // Pointer to the parent directory node
    int descIndexStart; // The index of this element's descriptor (or _START marker) in the WAD's descriptor list
    int descIndexEnd; // The index of this element's _END marker (if it's a namespace directory)
};

class Wad { // Declare the Wad class
public: // Public member access modifier
    static Wad* loadWad(const string &path); // Factory method to create a Wad object and load data
    ~Wad(); // Destructor to clean up resources

    string getMagic(); // Method to retrieve the WAD file's magic string
    bool isContent(const string &path); // Method to check if a path points to a file
    bool isDirectory(const string &path); // Method to check if a path points to a directory
    int getSize(const string &path); // Method to get the size of a file
    int getContents(const string &path, char *buffer, int length, int offset = 0); // Method to read file data
    int getDirectory(const string &path, vector<string> *directory); // Method to list directory contents
    void createDirectory(const string &path); // Method to create a new directory
    void createFile(const string &path); // Method to create a new empty file
    int writeToFile(const string &path, const char *buffer, int length, int offset = 0); // Method to write to a file (Extra Credit)

private: // Private member access modifier
    Wad(const string &path); // Private constructor called by loadWad
    int fd; // File descriptor for the open POSIX file
    char magic[5]; // Array to hold the 4-character magic string plus null terminator
    uint32_t numDescriptors; // The total number of descriptors in the WAD file
    uint32_t descriptorOffset; // The physical offset of the descriptor list in the WAD file
    Node* root; // Pointer to the root directory node of our virtual filesystem
    vector<Descriptor> descriptors; // In-memory copy of the WAD's descriptor list

    void parse(); // Internal method to parse the WAD file and build the tree
    Node* findNode(const string &path); // Internal helper to find a node by its string path
    void extractFileName(const string &path, string &parentPath, string &fileName); // Helper to split paths
    void writeHeader(); // Helper to flush header updates to the physical file
    void writeDescriptors(); // Helper to flush the descriptor list to the physical file
};