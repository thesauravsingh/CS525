Assignment 2 - Buffer Manager

GROUP #08
525 Assignment No.1 -FNU Saurav -Aakash Vasishta -Dresha Reddy Bommana -Azha Manzoor

The code for Assignment 2, focusing on a buffer manager, is ready for testing.

Follow these steps to run the test:
Navigate to the assignment directory.
Clean previous builds using "make clean".
Build the code with "make".
Execute the test with "./test_assign2_1".

Overview
This C file provides an implementation of a Buffer Manager for managing buffer pools and page handling in a database system. The Buffer Manager is responsible for efficiently managing the loading, eviction, and pinning/unpinning of pages in the buffer pool, as well as handling read and write operations to and from disk.

File Structure
buffer_mgr.c: Contains the implementation of the Buffer Manager functions.
buffer_mgr.h: Header file containing function declarations and necessary data structures for the Buffer Manager.
storage_mgr.h: Header file from the storage manager module, providing necessary declarations for storage management functions.
dberror.h: Header file defining error codes and messages.
dt.h: Header file defining data types and macros.
Functions
initBufferPool: Initializes a buffer pool with the specified parameters.
shutdownBufferPool: Shuts down the buffer pool and frees allocated memory.
writePageToDisk: Writes a page from memory to disk.
forceFlushPool: Forces flushing of all dirty pages to disk.
markDirty: Marks a page in the buffer pool as dirty.
unpinPage: Unpins a page in the buffer pool.
forcePage: Forces writing a specific page back to disk.
evict: Evicts a page from the buffer pool based on the replacement strategy.
pinPage: Pins a page in the buffer pool.
getFrameContents: Retrieves an array of page numbers currently stored in the buffer pool.
getDirtyFlags: Retrieves an array indicating whether each page in the buffer pool is dirty.
getFixCounts: Retrieves an array containing the fix count of each page in the buffer pool.
getNumReadIO: Gets the total number of read operations performed on the buffer pool.
getNumWriteIO: Gets the total number of write operations performed on the buffer pool.
Global Variables
logicClockForOperation: Global variable to keep track of logical timestamps for operations.
logicClockForLoadToMemoryOperation: Global variable to keep track of logical timestamps for loading pages into memory.
Constants
INIT_PAGE_NUMBER: Constant representing an uninitialized page number.
Dependencies
This implementation depends on functions and data structures defined in storage_mgr.h and dberror.h for storage management and error handling.

Usage
To use the Buffer Manager, include buffer_mgr.h in your C code and call the provided functions as needed to manage buffer pools and perform page handling operations.