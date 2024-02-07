#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *page;

//initializing page handler to NUll for the storage manager.
extern void initStorageManager (void){
	printf("The file pointer initialised to null");
	page = NULL;
}



// Function to create a new page file
extern RC createPageFile(char *fileName) {
    FILE *page = fopen(fileName, "w+");
    
    // Check if the file was successfully opened
    if (page == NULL) {
        printf("Failed to open file.\n");
        return RC_FILE_NOT_FOUND;
    }
    
    // Create an empty page in memory
    SM_PageHandle nullBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    if (nullBlock == NULL) {
        fclose(page);
        printf("Memory allocation failed.\n");
        return RC_WRITE_FAILED;
    }
    
    // Write the nullBlock page to file
    if (fwrite(nullBlock, sizeof(char), PAGE_SIZE, page) != PAGE_SIZE) {
        fclose(page);
        free(nullBlock);
        printf("Write failed.\n");
        return RC_WRITE_FAILED;
    }
    
    // Flush all file buffers
    fflush(page);
    
    // Close the file
    fclose(page);
    
    // De-allocate the memory allocated to nullBlock to prevent memory leaks
    free(nullBlock);
    
    printf("Page file created successfully.\n");
    return RC_OK;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	//fopen : opens the file, r denote that we want to open it in readmode
	page = fopen(fileName,"r");
  //To check if the file was successfully opened or not
    if(page == NULL){
      printf("file not found");
      return RC_FILE_NOT_FOUND;
    }
    else{
        fHandle->fileName = fileName;
     fHandle->curPagePos = 0;
     struct stat fileInfo;
     //fstat gives us the total file size
     if(fstat(fileno(page), &fileInfo) < 0)    
     return RC_FILE_NOT_FOUND;
     fHandle->totalNumPages = fileInfo.st_size/ PAGE_SIZE;
     fHandle->mgmtInfo = page;
     printf("Returning RC_OK");
     fclose(page);
     return RC_OK;
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	// Checking if file pointer was initialized or not.
	if(page != NULL)
		page = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// fopen : opens the file, r denote that we want to open it in readmode	
	page = fopen(fileName, "r");
	
	if(page == NULL)
		return RC_FILE_NOT_FOUND; 
	
	// Deleting the given filename so that it is no longer accessible.	
	remove(fileName);
	return RC_OK;
}


extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// To check if the pageNum is less than total number of pages or less than zero
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

	// fopen : opens the file, r denote that we want to open it in readmode	
	page = fopen(fHandle->fileName, "r");

	// Checking if file was successfully opened.
	if(page == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Setting the cursor(pointer) position of the file stream. Position is calculated by Page Number x Page Size
	// And the seek is success if fseek() return 0
	int isSeekSuccess = fseek(page, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == 0) {
		// We're reading the content and storing it in the location pointed out by memPage.
		fread(memPage, sizeof(char), PAGE_SIZE, page);
	} else {
		return RC_READ_NON_EXISTING_PAGE; 
	}
    	
	// Setting the current page position to the cursor(pointer) position of the file stream
	fHandle->curPagePos = ftell(page); 
	
	// Closing file stream so that all the buffers are flushed.     	
	fclose(page);
	
    	return RC_OK;
}

extern int getBlockPos(SM_FileHandle *fHandle){
//Check If FileHandle is not null0: 
if(fHandle != NULL){
//Return Page position
  printf("Block Position Successfully Returned /n");
   return (*fHandle).curPagePos;
}
else
{    printf("File not found!");
 return RC_FILE_NOT_FOUND; }
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Check If FileHandle is not null
	if(fHandle != NULL)	
	//reading the file
	return readBlock(0, fHandle, memPage);
	else
	return RC_FILE_NOT_FOUND;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	RC previousblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandle!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        previousblock =(*fHandle).curPagePos - 1; 

		if (previousblock < 0)
			return RC_READ_NON_EXISTING_PAGE;
//STEP 1: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 2:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 else {
 if((*fHandle).totalNumPages>0 && (fseek(page, (previousblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 3:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, page);
(*fHandle).curPagePos = (*fHandle).curPagePos - 1;
  if(fileReadSize > PAGE_SIZE || fileReadSize < 0 )
  {
	  printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;
  }

     //STEP 4:Set the position of the cursor to the pageNumber to read
  printf("Read Previous Page Succeeded!");
    return RC_OK;
 }else{
	 printf("Cannot read a non-existing page!");
	return RC_READ_NON_EXISTING_PAGE;
 }
}}
 else {
	printf("File not found!");
	return RC_FILE_NOT_FOUND; 
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	 RC currentblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandle != NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
         currentblock =(*fHandle).curPagePos;

	if (currentblock < 0)
	{       printf("Cannot read a non-existing page!");
		return RC_READ_NON_EXISTING_PAGE;}
 else {
	 //STEP 2: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 3:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 if((fseek(page, (currentblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 4:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, page);
 (*fHandle).curPagePos = fileReadSize;
  if(fileReadSize > PAGE_SIZE || fileReadSize < 0)
  {
	  printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;
  }
else{
printf("Read Current Block Succeeded!");
return RC_OK;
}
 }else{
	 	printf("Cannot read a non-existing page!");
		return RC_READ_NON_EXISTING_PAGE;
 }
}
}
 else {
	     printf("File not found!");
		return RC_FILE_NOT_FOUND; 
	}
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	 RC nextblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandle!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        nextblock =(*fHandle).curPagePos + 1; 

		if (nextblock < 0 || nextblock > (*fHandle).totalNumPages)
		{
			printf("Cannot read a non-existing page!");
			return RC_READ_NON_EXISTING_PAGE;
		}
 else {
    //STEP 1:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 if((fseek(page,(nextblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, page);
        //STEP 3:Page Size should not be greater than 0 but not greater then page size
(*fHandle).curPagePos = nextblock;
  if(fileReadSize>0 && fileReadSize >PAGE_SIZE)
	  printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;
     //STEP 4:Set the position of the cursor to the next block position
printf("Read Next Block Succeeded!");
    return RC_OK;
 }else{
	 	printf("Cannot read a non-existing page!");
		return RC_READ_NON_EXISTING_PAGE;
 }
}
}
 else {
	     printf("File not found!");
		return RC_FILE_NOT_FOUND; 
	}
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC lastblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandle!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        lastblock =(*fHandle).totalNumPages - 1; 

		if((fseek(page,(lastblock*PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, page);
        //STEP 3:Page Size should not be greater than 0 but not greater then page size
(*fHandle).curPagePos = lastblock;
  if(fileReadSize>0 && fileReadSize >PAGE_SIZE)
  { printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;
  }
     //STEP 4:Set the position of the cursor to the next block position
printf("Read Last Block Succeeded!");
    return RC_OK;
 }else{
	 	 printf("Cannot read a non-existing page!");
		return RC_READ_NON_EXISTING_PAGE;
 }
}
 else {
	     printf("File not found!");
		return RC_FILE_NOT_FOUND; 
	}
}

// Function to check if the page number is valid
int isValidPageNumber(int pageNum, SM_FileHandle *fHandle) {
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages)
        return 0;
    return 1;
}

// Function to open the file in read-write mode
FILE *openFileReadWrite(SM_FileHandle *fHandle) {
    FILE *page = fopen(fHandle->fileName, "r+");
    if (page == NULL)
        return NULL;
    return page;
}

// Function to seek to the appropriate position in the file
int seekToFilePosition(FILE *page, int pageNum) {
    int value = fseek(page, pageNum * PAGE_SIZE, SEEK_SET);
    return value;
}

// Function to write data to the file
RC writeDataToFile(FILE *page, SM_PageHandle memPage, size_t size) {
    size_t bytesWritten = fwrite(memPage, sizeof(char), size, page);
    if (bytesWritten != size)
        return RC_WRITE_FAILED;
    return RC_OK;
}

// Main function to write a block
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Check if the page number is valid
    if (!isValidPageNumber(pageNum, fHandle))
        return RC_WRITE_FAILED;

    // Open the file in read-write mode
    FILE *page = openFileReadWrite(fHandle);
    if (page == NULL)
        return RC_FILE_NOT_FOUND;

    // Seek to the appropriate position in the file
    int seekValue = seekToFilePosition(page, pageNum);
    if (seekValue != 0) {
        fclose(page);
        return RC_FILE_NOT_FOUND;
    }

    // Write data to the file
    RC writeResult = writeDataToFile(page, memPage, strlen(memPage));
    if (writeResult != RC_OK) {
        fclose(page);
        return writeResult;
    }

    // Update the current page position
    fHandle->curPagePos = pageNum;

    // Close the file
    fclose(page);

    return RC_OK;
}

// Function to calculate the sum of page numbers
int calculatePageNumberSum(int totalNumPages) {
    int sum = 0;
    for (int i = 0; i < totalNumPages; i++) {
        sum += i;
    }
    return sum;
}

// Function to write the current block to disk
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    printf("Writing to disk from memory.\n");
    
    // Calculate the sum of page numbers
    int sumOfPageNumbers = calculatePageNumberSum(fHandle->totalNumPages);
    printf("Sum of page numbers: %d\n", sumOfPageNumbers);
    
    // Invoke the writeBlock function to write the current block to disk
    RC write_block_res = writeBlock(fHandle->curPagePos, fHandle, memPage);
    
    // Checking if the write operation was successful
    if (write_block_res != RC_OK) {
        printf("Failed to write current block to disk.\n");
        return write_block_res; // Return the error code from writeBlock
    }

    return RC_OK;
}

// Function to check if the file handle is initialized
int fileHandleInitialize(SM_FileHandle *fHandle) {
    if (fHandle == NULL) {
        printf("File handle initialization is not done. Ending execution.\n");
        return 0;
    }
    return 1;
}

// Function to move file pointer to the end of the file
void moveFilePointerToEnd(FILE *page) {
    fseek(page, 0, SEEK_END);
}

// Function to write null bytes to create an empty block
void writeNullBytes(FILE *page, int numBytes) {
    for (int i = 0; i < numBytes; i++) {
        fwrite("\0", 1, 1, page);
    }
}

// Function to update file handle with appended block information
void updateFileHandle(SM_FileHandle *fHandle) {
    fHandle->totalNumPages += 1; // Increment the total number of pages
    fHandle->curPagePos += 1; // Update the current page position
}

// Function to release allocated memory for new block
void releaseMemory(SM_PageHandle newBlock) {
    free(newBlock);
}

// Main function to append an empty block
extern RC appendEmptyBlock(SM_FileHandle *fHandle) {
    FILE *page = (FILE *)fHandle->mgmtInfo;

    // Check if the file handle is initialized
    if (!fileHandleInitialize(fHandle))
        return RC_FILE_HANDLE_NOT_INIT;

    // Create and allocate memory for a new block within the page size of the file
    SM_PageHandle newBlock = (char *)calloc(PAGE_SIZE, sizeof(char));

    // Move to the end of the file to add a new block
    moveFilePointerToEnd(page);

    // Write null bytes to create an empty block
    writeNullBytes(page, PAGE_SIZE);

    // Update file handle with appended block information
    updateFileHandle(fHandle);

    // Release allocated memory for new block
    releaseMemory(newBlock);

    return RC_OK;
}

// Function to ensure capacity by adding empty blocks
RC addEmptyBlocks(int numBlocksToAdd, SM_FileHandle *fHandle) {
    int count = 0;
    while (count < numBlocksToAdd) {
        count++;
        int appendResult = appendEmptyBlock(fHandle);
        if (appendResult != RC_OK) {
            printf("ensureCapacity failure: due to failure of appendEmptyBlock \n");
            return appendResult;
        }
    }
    return RC_OK;
}

// Main function to ensure capacity
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Check if the file handle is initialized
    if (!isFileHandleInitialized(fHandle))
        return RC_FILE_HANDLE_NOT_INIT;

    // If total number of pages is already greater than or equal to required pages, return success
    if (fHandle->totalNumPages >= numberOfPages) {
        printf("Success in ensureCapacity: Sufficient pages already \n");
        return RC_OK;
    }

    // Calculate number of empty blocks needed to reach required capacity
    int numBlocksToAdd = numberOfPages - fHandle->totalNumPages;
    printf("ensureCapacity: Trying to add %d pages \n", numBlocksToAdd);

    // Add empty blocks to reach required capacity
    RC addBlocksResult = addEmptyBlocks(numBlocksToAdd, fHandle);
    if (addBlocksResult != RC_OK)
        return addBlocksResult;

    // Update the total number of pages
    fHandle->totalNumPages = numberOfPages;

    printf("Successful in ensureCapacity \n");
    return RC_OK;
}
