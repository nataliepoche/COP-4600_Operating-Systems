#define FUSE_USE_VERSION 26 // Define the FUSE API version requested
#include <fuse.h> // Include the main FUSE library header
#include <iostream> // Include IO stream
#include <string> // Include string library
#include <vector> // Include vector library
#include <errno.h> // Include standard error codes
#include <cstring> // FIX: Added to provide memset
#include "../libWad/Wad.h" // Include our custom Wad library header

using namespace std; // Use the standard namespace

Wad* wadFileSystem = nullptr; // Global pointer to hold our loaded Wad instance

static int wad_getattr(const char *path, struct stat *stbuf) { // Implement getattr FUSE callback
    memset(stbuf, 0, sizeof(struct stat)); // Zero out the stat buffer to prevent garbage data
    
    string p(path); // Convert the C-string path to std::string
    if (wadFileSystem->isDirectory(p)) { // Check if the path is a directory in our WAD
        stbuf->st_mode = S_IFDIR | 0777; // Set the mode to directory with full rwx permissions
        stbuf->st_nlink = 2; // Directories conventionally have 2 links
        return 0; // Return success code
    }
    else if (wadFileSystem->isContent(p)) { // Check if the path is a content file in our WAD
        stbuf->st_mode = S_IFREG | 0777; // Set the mode to regular file with full rwx permissions
        stbuf->st_nlink = 1; // Regular files have 1 link
        stbuf->st_size = wadFileSystem->getSize(p); // Set the file size attribute via our library
        return 0; // Return success code
    }
    
    return -ENOENT; // If neither, return the POSIX "No such file or directory" error
}

static int wad_mknod(const char *path, mode_t mode, dev_t rdev) { // Implement mknod FUSE callback
    string p(path); // Convert path to string
    wadFileSystem->createFile(p); // Ask the Wad library to create a new empty file
    return 0; // Return success code
}

static int wad_mkdir(const char *path, mode_t mode) { // Implement mkdir FUSE callback
    string p(path); // Convert path to string
    wadFileSystem->createDirectory(p); // Ask the Wad library to create a new directory
    return 0; // Return success code
}

static int wad_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { // Implement read callback
    string p(path); // Convert path to string
    return wadFileSystem->getContents(p, buf, size, offset); // Pass the buffer and offset parameters to our library's getContents
}

static int wad_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) { // Implement write callback
    string p(path); // Convert path to string
    return wadFileSystem->writeToFile(p, buf, size, offset); // Pass the buffer and chunk information to our library's writeToFile
}

static int wad_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) { // Implement readdir callback
    string p(path); // Convert path to string
    vector<string> entries; // Vector to hold directory child names
    
    if (wadFileSystem->getDirectory(p, &entries) == -1) { // Populate vector and check if directory is valid
        return -ENOENT; // Return error if path is invalid
    }
    
    filler(buf, ".", NULL, 0); // FUSE requires the current directory link "."
    filler(buf, "..", NULL, 0); // FUSE requires the parent directory link ".."
    
    for (const string &entry : entries) { // Loop over the names returned by getDirectory
        filler(buf, entry.c_str(), NULL, 0); // Pass each child's name to the FUSE filler function to populate the fs view
    }
    
    return 0; // Return success code
}

static struct fuse_operations wad_oper = { // Declare and map the fuse_operations struct
    .getattr = wad_getattr, // Map getattr
    .mknod   = wad_mknod, // Map mknod
    .mkdir   = wad_mkdir, // Map mkdir
    .read    = wad_read, // Map read
    .write   = wad_write, // Map write
    .readdir = wad_readdir, // Map readdir
};

int main(int argc, char *argv[]) { // The main function entry point
    if (argc < 3) { // Ensure the user passed enough arguments to function
        cout << "Usage: ./wadfs -s <wadfile> <mountdir>" << endl; // Output usage hint
        return 1; // Exit with error code
    }
    
    string wadPath = argv[argc - 2]; // Extract the second to last argument as the target WAD file path
    wadFileSystem = Wad::loadWad(wadPath); // Initialize our global Wad filesystem pointer
    
    argv[argc - 2] = argv[argc - 1]; // Shift the mountdir argument left to overwrite the WAD file path
    argc--; // Decrement the argument count so FUSE doesn't attempt to parse the WAD file path as a flag
    
    return fuse_main(argc, argv, &wad_oper, NULL); // Hand control over to the FUSE daemon loop
}