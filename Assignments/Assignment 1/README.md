# GROUP #2
525 Assignment No.1
-FNU Saurav
-Aakash Vasishta
-Dresha Reddy Bommana
-Azha Manzoor

## RUNNING THE MAKEFILE 

* Go to Folder root  using  Linux Terminal.
* Type "make"
* Type "make run_test_1" to run "test_assign1_1.c" file.

## Initialization and File Management Functions

initStorageManager()
- This function initializes the Storage Manager, setting the FileStream object to null.

createPageFile(char *fileName)
- Creates a new page file with a size of one page (PAGE SIZE) and initializes it with '0' bytes. 
- It utilizes the fopen() method to create the file in write mode.

openPageFile(char *fileName, SM_FileHandle *fHandle)
- Verifies the existence of the supplied file. If the file exists, it opens it; otherwise, it returns an error code (RC_FILE_NOT_FOUND).  
- It calculates the total number of pages by seeking to the end of the file and dividing its size by the page size. The file handle is then updated with the file name, total number of pages, and current page position.

closePageFile(SM_FileHandle *fHandle)
- Closes the currently open page file. Returns RC_OK upon successful closure, otherwise returns RC_FILE_NOT_FOUND.

destroyPageFile(char *fileName)
- Removes an existing page file using the remove function. Returns RC_OK if the file is successfully deleted, otherwise returns RC_FILE_NOT_FOUND.

## Functions for Reading Blocks

readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
- Checks if the requested page number (pageNum) is within the range of total pages. If it exceeds, an error is thrown. Otherwise, it moves the cursor to the specified page and reads the block into memory.

getBlockPos(SM_FileHandle *fHandle)
- Returns the current page position.

readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Reads the first page of the file by calling readBlock() with pageNumber set to 0.

readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Reads the last page of the file by calling readBlock() with (pageNumber - 1) as the argument.

readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Reads the previous block by passing (currentPage - 1) as the parameter to readBlock().

readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Reads the current block by calling readBlock() with currentPage as the parameter.

readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Reads the next block by passing (currentPage + 1) as the parameter to readBlock().

## Writing Functions

writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
- Verifies that the page number is valid and that fHandle contains necessary information. If not, it throws an error. Then, it moves the cursor to the specified page, writes the block, updates the current page to the newly entered page, and updates the overall page count.

writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
- Writes to the current block by calling writeBlock() with currentPosition as the parameter.

appendEmptyBlock(SM_FileHandle *fHandle)
- Checks if fHandle contains necessary information. If so, it creates a PAGE SIZE buffer, writes an empty block, and updates the file pointer position to the end. 
- It updates the total number of pages and current page position.

ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
- Checks if the total number of pages is less than the supplied number of pages (numberOfPages). 
- It calculates the number of additional pages needed and uses a loop to append empty blocks using the appendEmptyBlock() method.
