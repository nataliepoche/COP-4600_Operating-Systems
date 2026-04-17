#include "Wad.h" // Include our class definition
#include <unistd.h> // Include POSIX API for read, write, lseek, close
#include <fcntl.h> // Include POSIX API for file control options like open, O_RDWR
#include <cstring> // Include C string utilities like strcmp, memcpy
#include <iostream> // Include input/output stream library
#include <regex> // Include standard regex library to detect Map Markers
#include <vector>

Wad* Wad::loadWad(const string &path) { // Implement the static factory method
    return new Wad(path); // Dynamically allocate a new Wad instance and return it
} // End of loadWad

Wad::Wad(const string &path) { // Implement the private constructor
    fd = open(path.c_str(), O_RDWR); // Open the WAD file using POSIX with Read/Write permissions
    if (fd < 0) return; // If the file fails to open, exit early
    
    char headerData[12]; // Create a buffer to hold the 12-byte WAD header
    read(fd, headerData, 12); // Read the first 12 bytes from the file into the buffer
    
    memcpy(magic, headerData, 4); // Copy the first 4 bytes into the magic array
    magic[4] = '\0'; // Append a null terminator to make it a valid C-string
    
    memcpy(&numDescriptors, headerData + 4, 4); // Extract the 4-byte number of descriptors
    memcpy(&descriptorOffset, headerData + 8, 4); // Extract the 4-byte descriptor list offset
    
    root = new Node(); // Instantiate the root directory node
    root->name = "/"; // Set the root node's name to a forward slash
    root->isDir = true; // Mark the root node as a directory
    root->parent = nullptr; // The root has no parent, so set it to nullptr
    root->descIndexStart = 0; // Initialize root's descriptor start index
    root->descIndexEnd = (int)numDescriptors; // Root's end bounds effectively cover all descriptors
    
    parse(); // Call the parse helper to build the filesystem tree
} // End of Wad constructor

Wad::~Wad() { // Implement the destructor
    close(fd); // Close the POSIX file descriptor to prevent leaks
} // End of Wad destructor

void Wad::parse() { // Implement the parsing method to build the tree
    lseek(fd, descriptorOffset, SEEK_SET); // Move the file pointer to the start of the descriptor list
    
    for (uint32_t i = 0; i < numDescriptors; i++) { // Loop through all descriptors indicated by the header
        Descriptor d; // Create a temporary descriptor struct
        char rawDesc[16]; // Create a 16-byte buffer for a single descriptor
        read(fd, rawDesc, 16); // Read 16 bytes from the WAD file into the buffer
        
        memcpy(&d.offset, rawDesc, 4); // Extract the 4-byte element offset
        memcpy(&d.length, rawDesc + 4, 4); // Extract the 4-byte element length
        memcpy(d.name, rawDesc + 8, 8); // Extract the 8-byte ASCII name
        d.name[8] = '\0'; // Append a null terminator to the name
        descriptors.push_back(d); // Store the descriptor in our in-memory vector
    } // End of descriptor read loop
    
    Node* currentDir = root; // Start tree traversal at the root directory
    regex mapRegex("^E[0-9]M[0-9]$"); // Define a regular expression to match Map Markers (e.g., E1M1)
    int mapElementsRemaining = 0; // Counter for the 10 map elements that follow a map marker
    
    for (int i = 0; i < (int)descriptors.size(); i++) { // Cast size to int to avoid signedness warnings
        string name(descriptors[i].name); // Convert the C-string name to a C++ std::string
        
        if (mapElementsRemaining > 0) { // Check if we are currently parsing elements inside a map directory
            Node* fileNode = new Node(); // Create a new node for the map content file
            fileNode->name = name; // Set the file's name
            fileNode->isDir = false; // Mark it as a content file
            fileNode->offset = descriptors[i].offset; // Set the file's data offset
            fileNode->length = descriptors[i].length; // Set the file's data length
            fileNode->parent = currentDir; // Set the file's parent to the map directory
            fileNode->descIndexStart = i; // Save the file's descriptor index
            currentDir->children.push_back(fileNode); // Add the file to the parent's children list
            
            mapElementsRemaining--; // Decrement the remaining map element counter
            if (mapElementsRemaining == 0) { // Check if we've parsed all 10 elements for this map
                currentDir = currentDir->parent; // Step back up to the parent directory
            } // End of map elements check
        } // End of map elements block
        else if (regex_match(name, mapRegex)) { // Check if the current descriptor is a map marker
            Node* mapDir = new Node(); // Create a new node for the map directory
            mapDir->name = name; // Set the directory's name
            mapDir->isDir = true; // Mark it as a directory
            mapDir->parent = currentDir; // Link it to the current active directory
            mapDir->descIndexStart = i; // Mark its starting descriptor index
            currentDir->children.push_back(mapDir); // Add it to the active directory's children
            
            currentDir = mapDir; // Move into the new map directory
            mapElementsRemaining = 10; // Setup the counter to absorb the next 10 descriptors
        } // End of map marker block
        else if (name.length() > 6 && name.substr(name.length() - 6) == "_START") { // Check for namespace start
            Node* nsDir = new Node(); // Create a new node for the namespace directory
            nsDir->name = name.substr(0, name.length() - 6); // Extract the prefix to use as the directory name
            nsDir->isDir = true; // Mark it as a directory
            nsDir->parent = currentDir; // Link it to the current directory
            nsDir->descIndexStart = i; // Mark its starting descriptor index
            currentDir->children.push_back(nsDir); // Add the directory to the current directory's children
            
            currentDir = nsDir; // Move into the new namespace directory
        } // End of namespace start block
        else if (name.length() > 4 && name.substr(name.length() - 4) == "_END") { // Check for namespace end
            currentDir->descIndexEnd = i; // Record the index of the _END marker in the namespace node
            currentDir = currentDir->parent; // Move back up to the parent directory
        } // End of namespace end block
        else { // If it's none of the above markers, it must be a standard content file
            Node* fileNode = new Node(); // Create a new node for the file
            fileNode->name = name; // Set the name
            fileNode->isDir = false; // Mark it as a content file
            fileNode->offset = descriptors[i].offset; // Set the physical WAD offset
            fileNode->length = descriptors[i].length; // Set the length of the data
            fileNode->parent = currentDir; // Link it to the current directory
            fileNode->descIndexStart = i; // Record its descriptor index
            currentDir->children.push_back(fileNode); // Add it to the children list
        } // End of standard file block
    } // End of descriptor traversal loop
} // End of parse method

Node* Wad::findNode(const string &path) { // Implement the path search helper
    if (path == "/") return root; // If the path is just the root, return the root node

    // // Remove trailing slash to standardize the lookup path
    // if (path.length() > 1 && path.back() == '/') {
    //     path.pop_back();
    // }
    
    Node* curr = root; // Start searching from the root
    size_t start = 1; // Start index for substring extraction (size_t avoids warnings)
    size_t end = path.find('/', start); // Find the first slash after the root
    
    while (end != string::npos || start < path.length()) { // Loop through all path segments
        string part = path.substr(start, end - start); // Extract the current path segment
        if (part.empty()) break; // If the segment is empty (e.g., trailing slash), break the loop
        
        bool found = false; // Flag to track if we found the segment in the current directory
        for (Node* child : curr->children) { // Iterate over all children of the current node
            if (child->name == part) { // If the child's name matches the path segment
                curr = child; // Move down into this child
                found = true; // Set the flag indicating success
                break; // Stop searching the children
            } // End of name match check
        } // End of children loop
        
        if (!found) return nullptr; // If we couldn't find the segment, the path is invalid
        
        if (end == string::npos) break; // If there are no more slashes, we're at the end of the path
        start = end + 1; // Move the start index past the slash
        end = path.find('/', start); // Find the next slash
    } // End of path segment loop
    
    return curr; // Return the located node
} // End of findNode method

string Wad::getMagic() { // Implement the getMagic method
    return string(magic); // Convert the magic char array to a std::string and return
} // End of getMagic

bool Wad::isContent(const string &path) { // Implement isContent
    if (path.empty() || path.back() == '/') return false; // Fix for createFileTest3/5/6
    Node* n = findNode(path); // Search for the node at the specified path
    return (n != nullptr && !n->isDir); // Return true if node exists and is NOT a directory
} // End of isContent

bool Wad::isDirectory(const string &path) { // Implement isDirectory
    if (path.empty()) return false; // Fix for LibReadTests.isDirectory
    Node* n = findNode(path); // Search for the node at the specified path
    return n != nullptr && n->isDir; // Return true if node exists and IS a directory
} // End of isDirectory

int Wad::getSize(const string &path) { // Implement getSize
    Node* n = findNode(path); // Search for the node at the path
    if (n != nullptr && !n->isDir) { // If it exists and is a file
        return (int)n->length; // Return its length (cast to int to avoid warnings)
    } // End of valid file check
    return -1; // If it's a directory or invalid, return -1
} // End of getSize

int Wad::getContents(const string &path, char *buffer, int length, int offset) { // Implement getContents
    Node* n = findNode(path); // Locate the file node
    if (n == nullptr || n->isDir) return -1; // Return -1 if invalid path or points to a directory
    
    if (offset >= (int)n->length) return 0; // If offset is beyond EOF, no bytes can be read, return 0
    
    int bytesToRead = length; // Initialize the bytes to read to the requested length
    if (offset + length > (int)n->length) { // Cast n->length to int for safe comparison
        bytesToRead = (int)n->length - offset; // Cap the bytes to read to the remaining file data
    } // End of bounds check
    
    lseek(fd, n->offset + offset, SEEK_SET); // Seek the POSIX file descriptor to the exact content offset
    int bytesRead = read(fd, buffer, bytesToRead); // Read the data directly into the provided buffer
    return bytesRead; // Return the actual number of bytes read
} // End of getContents

int Wad::getDirectory(const string &path, vector<string> *directory) { // Implement getDirectory
    if(path.empty()) return -1;
    
    Node* n = findNode(path); // Locate the directory node
    if (n == nullptr || !n->isDir) {
        return -1; // Return -1 if invalid or a file
    }
    
    if (directory != nullptr) {
        directory->clear(); // To ensure we don't append old data
        for (Node* child : n->children) { // Loop through all children of this directory
            directory->push_back(child->name); // Append each child's name to the provided vector
        } // End of children loop
    }
    
    return (int)n->children.size(); // Cast size to int to avoid return warnings
} // End of getDirectory

void Wad::extractFileName(const string &path, string &parentPath, string &fileName) { // Helper to split paths
    string cleanPath = path;
    // Strip trailing slash for proper parsing 
    if (cleanPath.length() > 1 && cleanPath.back() == '/') {
        cleanPath.pop_back();
    }
    
    size_t lastSlash = cleanPath.find_last_of('/'); // Find the position of the last slash
    if (lastSlash == 0) { // If the slash is at the very beginning
        parentPath = "/"; // The parent is the root
        fileName = cleanPath.substr(1); // The file name is everything after the slash
    } else { // Otherwise, it's nested
        parentPath = cleanPath.substr(0, lastSlash); // Extract everything before the last slash
        fileName = cleanPath.substr(lastSlash + 1); // Extract everything after the last slash
    } // End of slash conditional
} // End of extractFileName

void Wad::writeHeader() { // Implement header writing helper
    lseek(fd, 4, SEEK_SET); // Seek past the 4-byte magic to the numDescriptors field
    write(fd, &numDescriptors, 4); // Write the updated number of descriptors to the file
    write(fd, &descriptorOffset, 4); // Write the updated descriptor list offset to the file
} // End of writeHeader

void Wad::writeDescriptors() { // Implement descriptor flushing helper
    lseek(fd, descriptorOffset, SEEK_SET); // Seek to the start of the descriptor list
    for (int i = 0; i < (int)descriptors.size(); i++) { // Cast size to int to prevent warnings
        const Descriptor& d = descriptors[i]; // Fetch current descriptor
        char rawDesc[16]; // Buffer for a single descriptor
        memcpy(rawDesc, &d.offset, 4); // Copy offset
        memcpy(rawDesc + 4, &d.length, 4); // Copy length
        memset(rawDesc + 8, 0, 8); // Zero out the name field completely
        
        // Use a safe loop instead of strnlen to avoid compiler/POSIX extension warnings
        for(int j = 0; j < 8 && d.name[j] != '\0'; j++) { 
            rawDesc[8 + j] = d.name[j]; 
        } // End copy loop
        
        write(fd, rawDesc, 16); // Write the 16 bytes to the file
    } // End of write loop
} // End of writeDescriptors

void Wad::createDirectory(const string &path) { // Implement createDirectory
    string parentPath, dirName; // Strings to hold the split path components
    extractFileName(path, parentPath, dirName); // Split the path into parent and target name
    
    // Check constraint: Namespace markers max 2 chars length limit
    if (dirName.length() > 2) return;

    Node* parentNode = findNode(parentPath); // Locate the parent directory node
    if (!parentNode || !parentNode->isDir) return; // Abort if parent is invalid

    // Check constraint: Directories cannot be created inside Map directories
    regex mapRegex("^E[0-9]M[0-9]$");
    if (regex_match(parentNode->name, mapRegex)) return;

    int insertIndex; // Variable to hold where in the descriptor list we will insert
    if (parentNode == root) { // If the parent is the root directory
        insertIndex = (int)descriptors.size(); // Insert at the very end (casted to avoid warnings)
    } else { // If the parent is a nested namespace directory
        insertIndex = parentNode->descIndexEnd; // Insert immediately before the parent's _END marker
    } // End of root conditional check
    
    Descriptor startDesc, endDesc; // Create the two required namespace markers
    startDesc.offset = 0; startDesc.length = 0; // Directory markers have 0 offset and length
    endDesc.offset = 0; endDesc.length = 0; // Directory markers have 0 offset and length
    
    string startName = dirName + "_START"; // Form the _START marker name
    string endName = dirName + "_END"; // Form the _END marker name
    
    // Use memset and safe loops instead of strncpy to avoid -Wstringop-truncation warnings
    memset(startDesc.name, 0, 9); 
    memset(endDesc.name, 0, 9);
    for(size_t i = 0; i < 8 && i < startName.length(); i++) startDesc.name[i] = startName[i];
    for(size_t i = 0; i < 8 && i < endName.length(); i++) endDesc.name[i] = endName[i];
    
    descriptors.insert(descriptors.begin() + insertIndex, startDesc); // Insert the _START marker
    descriptors.insert(descriptors.begin() + insertIndex + 1, endDesc); // Insert the _END marker
    
    numDescriptors += 2; // Increase the total descriptor count in the header by 2
    writeHeader(); // Write the new header to disk
    writeDescriptors(); // Overwrite the physical descriptor list to include our new markers
    
    Node* newDir = new Node(); // Create the in-memory tree node for the new directory
    newDir->name = dirName; // Set its name
    newDir->isDir = true; // Mark as a directory
    newDir->parent = parentNode; // Set the parent
    newDir->descIndexStart = insertIndex; // Record start index
    newDir->descIndexEnd = insertIndex + 1; // Record end index
    parentNode->children.push_back(newDir); // Add to parent's child list
    
    // Shift indices dynamically for all affected nodes across the tree
    vector<Node*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        Node* curr = stack.back();
        stack.pop_back();
        if (curr != newDir) {
            if (curr->descIndexStart >= insertIndex) curr->descIndexStart += 2;
            if (curr->isDir && curr->descIndexEnd >= insertIndex) curr->descIndexEnd += 2;
        }
        for (Node* child : curr->children) stack.push_back(child);
    }

    // for (Node* child : parentNode->children) { // Iterate parent's children to fix any shifted end markers
    //     if (child != newDir && child->descIndexEnd >= insertIndex) { // Check if sibling's markers were shifted
    //         child->descIndexStart += 2; // Adjust sibling start
    //         child->descIndexEnd += 2; // Adjust sibling end
    //     } // End of sibling check
    // } // End of tree index adjustment
} // End of createDirectory

// void Wad::createDirectory(const string &path) {
//     // Validate that the parent exists and the new directory name is valid
//     if (isDirectory(path) || isContent(path)) return; 

//     // Extract the new directory name from the path
//     string dirName = extractName(path); 

//     // 1. Add START descriptor to our internal list
//     Descriptor startDesc;
//     strncpy(startDesc.name, (dirName + "_START").c_str(), 8);
//     startDesc.offset = 0; // Directories have no data length
//     startDesc.length = 0;
//     descriptors.push_back(startDesc);

//     // 2. Add END descriptor to our internal list
//     Descriptor endDesc;
//     strncpy(endDesc.name, (dirName + "_END").c_str(), 8);
//     endDesc.offset = 0;
//     endDesc.length = 0;
//     descriptors.push_back(endDesc);

//     // 3. Increment the total descriptor count by 2
//     numDescriptors += 2;

//     // 4. Update the physical WAD file header on disk
//     writeHeader();

//     // 5. Rewrite the entire descriptor list at the end of the file
//     writeDescriptors();

//     // 6. Refresh the internal tree structure to include the new directory
//     buildTree(); 
// }

void Wad::createFile(const string &path) { // Implement createFile
    string parentPath, fileName; // Strings to hold the split path components
    extractFileName(path, parentPath, fileName); // Split the path
    
    // Check constraint: Length cap & reserved patterns bypass
    if (fileName.length() > 8) return;
    
    regex mapRegex("^E[0-9]M[0-9]$");
    if (regex_match(fileName, mapRegex)) return;
    if (fileName.length() > 6 && fileName.substr(fileName.length() - 6) == "_START") return;
    if (fileName.length() > 4 && fileName.substr(fileName.length() - 4) == "_END") return;

    Node* parentNode = findNode(parentPath); // Locate parent
    if (!parentNode || !parentNode->isDir) return; // Exit if invalid parent
    
    // Check constraint: Cannot create inside map directory
    if (regex_match(parentNode->name, mapRegex)) return;

    int insertIndex; // Descriptor array index for insertion
    if (parentNode == root) { // If parent is root
        insertIndex = (int)descriptors.size(); // Add to end (casted to avoid warnings)
    } else { // Otherwise
        insertIndex = parentNode->descIndexEnd; // Insert before parent's _END marker
    } // End of parent logic
    
    Descriptor fileDesc; // Create the new file descriptor
    fileDesc.offset = 0; // Offset is 0 initially
    fileDesc.length = 0; // Length is 0 initially
    
    // Use memset and safe loops instead of strncpy to avoid -Wstringop-truncation warnings
    memset(fileDesc.name, 0, 9); 
    for(size_t i = 0; i < 8 && i < fileName.length(); i++) fileDesc.name[i] = fileName[i];
    
    descriptors.insert(descriptors.begin() + insertIndex, fileDesc); // Insert into descriptor vector
    
    numDescriptors += 1; // Increment descriptor count
    writeHeader(); // Flush header
    writeDescriptors(); // Flush descriptor list
    
    Node* newFile = new Node(); // Create tree node
    newFile->name = fileName; // Set name
    newFile->isDir = false; // Mark as file
    newFile->offset = 0; // Init offset
    newFile->length = 0; // Init length
    newFile->parent = parentNode; // Link to parent
    newFile->descIndexStart = insertIndex; // Set index
    parentNode->children.push_back(newFile); // Attach to tree

    // Shift indices dynamically for all affected nodes across the tree
    vector<Node*> stack;
    stack.push_back(root);
    while (!stack.empty()) {
        Node* curr = stack.back();
        stack.pop_back();
        if (curr != newFile) {
            if (curr->descIndexStart >= insertIndex) curr->descIndexStart += 1;
            if (curr->isDir && curr->descIndexEnd >= insertIndex) curr->descIndexEnd += 1;
        }
        for (Node* child : curr->children) stack.push_back(child);
    }
    // for (Node* child : parentNode->children) { // Fix sibling indexes due to insertion
    //     if (child != newFile && child->descIndexStart >= insertIndex) { // If shifted
    //         child->descIndexStart += 1; // Increment start
    //         if (child->isDir) child->descIndexEnd += 1; // Increment end if dir
    //     } // End shift check
    // } // End of sibling adjustment
} // End of createFile

int Wad::writeToFile(const string &path, const char *buffer, int length, int offset) { // Implement writeToFile
    Node* n = findNode(path); // Find the file node

    // Check constraint: If the file already has pre-existing content, you cannot overwrite.
    if (n->length > 0) return 0;

    // if (!n || n->isDir) return -1; // Abort if invalid or directory
    if (n == nullptr || n->isDir) return -1; // Abort if invalid or directory
    
    if (n->length == 0) { // If the file is newly created and completely empty
        n->offset = descriptorOffset; // Assign it an offset exactly at the start of the current descriptor list
        descriptors[n->descIndexStart].offset = n->offset; // Update the physical descriptor offset
    } // End empty check
    
    lseek(fd, n->offset + offset, SEEK_SET); // Seek to the target write location
    
    // Explicitly track bytes written to avoid potential errors
    int bytesWritten = write(fd, buffer, length); 
    // // Even if we wrote bytes, the test expects '0' for a successful operation
    // if (bytesWritten != -1) {
    //     return 0; // Fix for LibWriteTests.writeToFileTest1/2
    // }
    if (bytesWritten <= 0) return bytesWritten; // Return immediately if the write failed
    
    descriptorOffset += bytesWritten; // Shift the descriptor list physical offset forward by the written length
    descriptors[n->descIndexStart].length += bytesWritten; // Augment the file's descriptor length by the chunk size
    n->length += bytesWritten; // Augment the node's length tracker
    
    writeHeader(); // Write the new descriptor offset to the physical header
    writeDescriptors(); // Rewrite the entire descriptor list at its new, shifted physical offset
    
    return bytesWritten; // Return the number of bytes successfully written
} // End of writeToFile