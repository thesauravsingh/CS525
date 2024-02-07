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



extern RC createPageFile(char *fileName){
	page = fopen(fileName,"w+");
	//  Checking if the file was successfully opened or not
	if(page == NULL){
		return RC_FILE_NOT_FOUND;
        printf("Page is NULL!");
	}
	else {
        //Creating an empty page in memory
		SM_PageHandle nullBlock = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));
        // Writing the nullBlock page to file
		if(fwrite(nullBlock,sizeof(char),PAGE_SIZE, page) >= PAGE_SIZE) { 
        printf("Write Succeeded \n");
		}else{
            printf("Write failed \n");
        }
		//flushing all file buffers and freeing the memory to prevent memory leaks.
		fclose(page);
		//De-allocating the memory allocated to nullBlock to free up memory
		free(nullBlock);
		return RC_OK;
	}
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
	// Checking if file pointer or the storage manager is intialised. If initialised, then close.
	if(page != NULL)
		page = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	page = fopen(fileName, "r");
	
	if(page == NULL)
		return RC_FILE_NOT_FOUND; 
	
	// Deleting the given filename so that it is no longer accessible.	
	remove(fileName);
	return RC_OK;
}

// reading blocks from disc 
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
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

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//checks if the pageNum parameter is greater than the total number of pages in the file or if it is less than 0
	 if(pageNum > fHandle->totalNumPages || pageNum <0){
		
		return RC_WRITE_FAILED;
	}
	//opens the file in read mode
  page = fopen(fHandle->fileName,"r+");
  // checks if the file pointer page is NULL, indicating that the file could not be opened
  if(page == NULL){
    return RC_FILE_NOT_FOUND;
  }
  int value = fseek(page, pageNum*PAGE_SIZE,SEEK_SET);

  //checks if the value returned by fseek is not equal to 0, indicating that the seek operation failed.
	if(value != 0){
		return RC_FILE_NOT_FOUND;
	}else{
    fwrite(memPage,sizeof(char),strlen(memPage),page);
		fHandle->curPagePos = pageNum;
		fclose(page);

		return RC_OK;
  }
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	printf("Writing to disk from the memory");
   // Invoke the writeBlock function to write the current block to disk
  RC write_block_res = writeBlock(fHandle->curPagePos, fHandle, memPage);
  // Checking if write operation was successful
  if (write_block_res != RC_OK)
  {
    printf("Writing current block to disk failed.\n");
    return write_block_res; // Return the error code from writeBlock
  }

  return RC_OK;
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Obtain the file pointer
  page = (FILE *)fHandle->mgmtInfo;
  if (fHandle == NULL)
  {
    printf("Initializing is not done in File handle. Ending execution.\n");
    return RC_FILE_HANDLE_NOT_INIT;
  }

 // Create and allocate memory for a new block within the page size of the file
  SM_PageHandle newBlock = (char *)calloc(PAGE_SIZE, sizeof(char));

  fseek(page, 0, SEEK_END);// Move to the end of the file to add a new block

  for (int i = 0; i < PAGE_SIZE; i++) // Write empty spaces to create an empty block
  {
     // Write null to create an empty page
    fwrite("\0", 1, 1, page);
    // Move to the end of the file for additional empty spaces if needed
    fseek(page, 0, SEEK_END);
  }
  
   // Increment the total number of pages since a new block was appended
  int finalPageNum = fHandle->totalNumPages;
  finalPageNum += 1;
  fHandle->totalNumPages = finalPageNum;

  // Update the current page position for reading and writing data
  fHandle->curPagePos += 1;

  // releasing the new block created, for writing
  free(newBlock);
  return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	if (fHandle == NULL)
  {
    printf("ensureCapacity failure: RC_FILE_HANDLE_NOT_INIT \n");
    return RC_FILE_HANDLE_NOT_INIT;
  }
  else
  {
    if (fHandle->totalNumPages >= numberOfPages)
    {
      printf("success in ensureCapacity: Fine already \n");
      return RC_OK;
    }
    else
    {
      int count = 0;
      int final = numberOfPages - fHandle->totalNumPages;
      printf("ensureCapacity: trying to add pages %d \n", final);
      while (count < final)
      {
        count++;
        int appendResult = appendEmptyBlock(fHandle);
        if (appendResult != RC_OK)
        {
          printf("ensureCapacity failure: due to failure of appendEmptyBlock \n");
          return appendResult;
        }
        else
        {
          continue;
        }
      }
      // Update the total number of pages
      fHandle->totalNumPages = numberOfPages;
      printf("Successfull in ensureCapacity \n");
      return RC_OK;
    }
  }
}
