#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sys/stat.h"
#include "storage_mgr.h"
#include "dberror.h"

FILE *fp;

/* manipulating page files */


extern void initStorageManager(void)
{
  // printf("This is the initialization of storage manager\n");
  fp = NULL;
  // printf("Initialization complete\n");
}

/*

The createPageFile (method) is employed for creating a new page to the disk at the end of the file
Data is written to the end of the file to check whether the new page is readable and writable
The page after creation is closed to avoid data overwriting
*/
extern RC createPageFile(char *fileName)
{
  fp = fopen(fileName, "w+");
  // Then, check if is available or not
  if (fp == NULL)
  {
    // If it gets into "If" condition then it means that file is not available to write
    // printf("Write Block Action Failed\n");
    // printf("(Reason) File not found.\n");
    // Return with error File is not found
    return RC_FILE_NOT_FOUND;
  }

  SM_PageHandle new_pagefile = (char *)calloc(PAGE_SIZE, sizeof(char));
  // Begun writing here
  if (fwrite(new_pagefile, 1, PAGE_SIZE, fp) != PAGE_SIZE)
  {
    // fails to write
    // printf("Write Block Action Failed\n");
    // printf("(Reason) Could not write data.\n");
    // return with write failed error
    return RC_WRITE_FAILED;
  }
  // releasing the newly created page from writing
  free(new_pagefile);

  // close file after writing and releasing
  if (fclose(fp) != 0)
  {
    // fails to close
    // printf("Close File Action Failed\n");
    // printf("(Reason) Could not close open file.\n");
    // return with close failed error
    return RC_CLOSE_FAILED;
  }

  return RC_OK;
}

/**
 * @param fHandle
 * @param fileName
 * @return
 * Open the page file based on fileName.
 */
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle)
{
  // Open file in read mode
  // printf("Opening file to read...\n");
  fp = fopen(fileName, "r+");

  if (fp == NULL)
  {
    // printf("openPageFile: Failed to read file.\n");
    return RC_FILE_NOT_FOUND;
  }

  // Initialize file attributes
  // printf("Initinializing file attributes...\n");
  fHandle->mgmtInfo = fp;
  fHandle->curPagePos = 0;
  fHandle->fileName = fileName;

  // Get file information using the stat function
  struct stat buffer;
  int fileStat = fstat(fileno(fp), &buffer);
  // printf("openPageFile: file stats: %d\n", fileStat);

  if (fileStat != 0)
  {
    // printf("openPageFile: Failed to fetch file information.\n");
    return RC_FETCH_FAILED;
  }

  // Get the file size from the buffer
  off_t pageSize = buffer.st_size;
  // print off_t https://stackoverflow.com/questions/5251669/how-to-print-off-t-in-c
  // printf("openPageFile: file size %d\n", (int)pageSize);

  // Calculate the total number of pages based on file size and page size
  fHandle->totalNumPages = (int)pageSize / PAGE_SIZE;

  // Return RC_OK for success
  return RC_OK;
}

/**
 * close the page file
 * @param fHandle
 * @return
 */
extern RC closePageFile(SM_FileHandle *fHandle)
{
  if (fHandle == NULL)
  {
    // printf("closePageFile fail: RC_FILE_HANDLE_NOT_INIT \n");
    return RC_FILE_HANDLE_NOT_INIT;
  }
  else
  {
    int closeFileResult = fclose(fHandle->mgmtInfo);
    if (closeFileResult != 0)
    {
      // printf("closePageFile fail: RC_FILE_CLOSE_FAILED, no specific RC return \n");
      // printf("closePageFile fail silently \n");
    }
    else
    {
      fHandle->mgmtInfo = NULL;
    }
    // printf("closePageFile complete: RC_OK\n");
    return RC_OK;
  }
}

/**
 * destroy the page file
 * @param fileName
 * @return
 */
extern RC destroyPageFile(char *fileName)
{
  remove(fileName);
  // printf("destroyPageFile complete: RC_OK \n");
  return RC_OK;
}

/* reading blocks from disc */

// read a block on the file
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // printf("storage mgr, total page %d, pageNum: %d. \n", fHandle->totalNumPages, pageNum);

  // point to the block of file to be read
  fp = (FILE *)fHandle->mgmtInfo;

  // check if the file handle is initialized. if not, return RC_FILE_HANDLE_NOT_INIT error code
  if (fHandle == NULL)
  {
    // printf("The file handle is not initialized. Ending the execution.\n");
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // check if the page read really exists. if not, return RC_READ_NON_EXISTING_PAGE error code
  if (fseek(fp, PAGE_SIZE * pageNum, SEEK_SET) != 0)
  {
    // printf("The requested page was not found. Check the page number. \n");
    return RC_READ_NON_EXISTING_PAGE;
  }
  // check if the page number of the requested file to be read is out of size limit. if it is, return  error as RC_READ_NON_EXISTING_PAGE since the page doesnt exist
  if (fHandle->totalNumPages <= pageNum || pageNum < 0)
  {
    // printf("The file requested for read is out of range. Check the page number. \n");
    return RC_READ_NON_EXISTING_PAGE;
  }

  // check if the requested page number is readable. if not, return RC_READ_FAILED error code, since the page might have faulty data
  if (fread(memPage, sizeof(char), PAGE_SIZE, fp) != PAGE_SIZE)
  {
    // printf("The requested page number %d has some errors. Check the page data. \n", pageNum);
    return RC_READ_FAILED;
  }

  // if the above conditions are passed, change the current page position to the requested page for reading and confirm that the requested page data is correct
  fseek(fp, pageNum * PAGE_SIZE, SEEK_SET);
  fHandle->curPagePos = pageNum;
  return RC_OK;
}

/**
 */
extern int getBlockPos(SM_FileHandle *fHandle)
{
  int blockPos;                   // Declaring a variable to store the position
  blockPos = fHandle->curPagePos; // Assigninig the current page position to the declared variable
  return blockPos;                // return the block position
}

/**
 */
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  fseek(fp, 0, SEEK_SET);                      // Set the file pointer 'file' to the beginning of the file.
  fread(memPage, sizeof(char), PAGE_SIZE, fp); // Read the first block
  fHandle->curPagePos = 0;                     // Set current page position to 0
  return RC_OK;                                // return statement for successful operation
}

/**
 */
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{

  int currentPage = getBlockPos(fHandle); // Calculating the current position
  int previousPage = currentPage - 1;     // Calculating the previous  position

  RC returnCode = readBlock(previousPage, fHandle, memPage); // Calling the readBlock function
  return returnCode;                                         // Return statement
}

/**
 * @param fHandle
 * @param memPage
 * @return
 * Read the current block based on the given page position reference.
 */
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // printf("Reading current block...\n");
  // Invoke the readBlock function to read the next block based on the page position
  RC readBlockRes = readBlock(fHandle->curPagePos, fHandle, memPage);
  // Check if the read operation was successful
  if (readBlockRes != RC_OK)
  {
    // printf("Failed to read the next block.\n");
    return readBlockRes; // Return the error code from readBlock
  }

  // Return RC_OK for a successful read operation
  return RC_OK;
}

/**
 * @param fHandle
 * @param memPage
 * @return
 * Read the next block based on the given page position in the parameter.
 */
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // printf("Reading the next block available...\n");
  // Invoke the readBlock function to read the next block based on the page position
  RC readBlockRes = readBlock(fHandle->curPagePos + 1, fHandle, memPage);
  // Check if the read operation was successful
  if (readBlockRes != RC_OK)
  {
    // printf("Failed to read the next block.\n");
    return readBlockRes; // Return the error code from readBlock
  }

  fHandle->curPagePos += 1;
  // Return RC_OK for a successful read operation
  return RC_OK;
}

/**
 * @param fHandle
 * @param memPage
 * @return
 * Read the last block available based on the given page reference.
 */
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // printf("Reading the last block available...\n");
  // Invoke the readBlock function to read the last block based on the page reference
  RC readBlockRes = readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
  // Check if the read operation was successful
  if (readBlockRes != RC_OK)
  {
    // printf("Failed to read the last block.\n");
    return readBlockRes; // Return the error code from readBlock
  }

  fHandle->curPagePos -= fHandle->totalNumPages - 1;
  // Return RC_OK for a successful read operation
  return RC_OK;
}

/* writing blocks to a page file */

/*

The writeBlock (method) is employed for saving a page to the disk at the specified location
This location is specified by 'pageNum'. It accomplishes this by using 'fseek()' to navigate to the desired
page number and 'fwrite()' to transfer the data from memPage to file handle
*/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // First, lets check if the file exists or not
  if (fHandle == NULL || fHandle->mgmtInfo == NULL)
  {
    // If it gets into "If" condition then it means that file is not available to write
    // printf("Write Block Action Failed\n");
    // printf("(Reason) File does not exist.\n");
    // Return with error File not found in it
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // Inspects the validity of page
  if (fHandle->totalNumPages <= pageNum)
  {
    // printf("Write Block Action Failed\n");
    // printf("(Reason) Page number is not valid. pageNum: %d, total: %d\n", pageNum, fHandle->totalNumPages);
    // Return with the write failed error
    return RC_WRITE_FAILED;
  }

  // Begun seeking here
  if (fseek((FILE *)fHandle->mgmtInfo, PAGE_SIZE * pageNum, SEEK_SET) != 0)
  {
    // this means, it coundn't seek and cant reach the beginning of file.
    // printf("Write Block Action Failed\n");
    // printf("(Reason) Couldn't reach the beginning of the file.\n");
    // return with write failed error
    return RC_WRITE_FAILED;
  }

  // Begun writing here
  if (fwrite(memPage, 1, PAGE_SIZE, (FILE *)fHandle->mgmtInfo) != PAGE_SIZE)
  {
    // fails to write
    // printf("Write Block Action Failed\n");
    // printf("(Reason) Could not write data.\n");
    // return with write failed error
    return RC_WRITE_FAILED;
  }

  // Update the page
  fHandle->curPagePos = pageNum;

  return RC_OK;
}

/**
 * @param fHandle
 * @param memPage
 * @return
 * Write the contents from memPage (memory) on to the disk.
 */
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
  // printf("Writing content from memory to disk...\n");
  // Invoke the writeBlock function to write the current block to disk based on the page position
  // This function writes the contents of memPage to the current page position in the file
  RC writeBlockRes = writeBlock(fHandle->curPagePos, fHandle, memPage);
  // Check if the write operation was successful
  if (writeBlockRes != RC_OK)
  {
    // printf("Failed to write the current block to disk.\n");
    return writeBlockRes; // Return the error code from writeBlock
  }

  // Return RC_OK for a successful write operation
  return RC_OK;
}

// append an empty new block to the end of the file, since the previous blocks are full
RC appendEmptyBlock(SM_FileHandle *fHandle)
{
  // point to the file where empty block needs to be appended
  fp = (FILE *)fHandle->mgmtInfo;
  // check if the file handle is initialized. if not, return RC_FILE_HANDLE_NOT_INIT error code
  if (fHandle == NULL)
  {
    // printf("The file handle is not initialized. Ending the execution.\n");
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // creating and assigning memory to new block within the page size of the file
  SM_PageHandle new_block = (char *)calloc(PAGE_SIZE, sizeof(char));

  // jump to the end of the file to add new block
  fseek(fp, 0, SEEK_END);

  // write empty spaces in the page to create empty block
  for (int i = 0; i < PAGE_SIZE; i++)
  {
    // write null to create empty page
    fwrite("\0", 1, 1, fp);
    // jump to the end of the file to add more empty spaces as needed
    fseek(fp, 0, SEEK_END);
  }

  // increasing the total number of page count by 1, since we appended a new block
  int final_page_no = fHandle->totalNumPages;
  final_page_no += 1;
  fHandle->totalNumPages = final_page_no;

  // change the current page position to the new page for writing and reading data
  fHandle->curPagePos += 1;

  // release the newly created block for writing
  free(new_block);
  return RC_OK;
}

/**
 * increase size to numberOfPages if the pages in the file is less than that
 * @param numberOfPages
 * @param fHandle
 * @return
 */
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
  if (fHandle == NULL)
  {
    // printf("ensureCapacity fail: RC_FILE_HANDLE_NOT_INIT \n");
    return RC_FILE_HANDLE_NOT_INIT;
  }
  else
  {
    if (fHandle->totalNumPages >= numberOfPages)
    {
      // printf("ensureCapacity success: already enough \n");
      return RC_OK;
    }
    else
    {
      int count = 0;
      int total = numberOfPages - fHandle->totalNumPages;
      // printf("ensureCapacity: trying to add pages %d \n", total);
      while (count < total)
      {
        count++;
        int appendResult = appendEmptyBlock(fHandle);
        if (appendResult != RC_OK)
        {
          // printf("ensureCapacity fail: due to appendEmptyBlock fail \n");
          return appendResult;
        }
        else
        {
          continue;
        }
      }
      fHandle->totalNumPages = numberOfPages;
      // printf("ensureCapacity successfully \n");
      return RC_OK;
    }
  }
}