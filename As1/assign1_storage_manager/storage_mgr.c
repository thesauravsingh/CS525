#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<math.h>
#include "storage_mgr.h"

FILE *pagefile;

//page handler is initialised to NUll for the storage manager.
extern void initStorageManager (void){
	pagefile = NULL;
		printf("\n <---------------CS525  - Assigsment 1 ----------------->\n ");
		printf("BY\n");
        printf("Azha Manzoor A20557552\n");
      	printf("FNU Saurav A20536122\n");
		printf("Dresha Reddy  \n");
        printf("Aakash Vasishta \n ");

	
    printf("The file pointer initialised to null");
}

extern RC createPageFile (char *fileName){
  pagefile = fopen(fileName,"w+");
	//  Checking if the file was successfully opened or not
	if(pagefile == NULL){
		return RC_FILE_NOT_FOUND;
        printf("Page is NULL!");
	}
	else {
        //Creating an empty page in memory
		SM_PageHandle nullBlock = (SM_PageHandle)calloc(PAGE_SIZE,sizeof(char));
        // Writing the nullBlock page to file
		if(fwrite(nullBlock,sizeof(char),PAGE_SIZE, pagefile) >= PAGE_SIZE) {
			//flushing all file buffers and freeing the memory to prevent memory leaks.
			fclose(pagefile);
            //De-allocating the memory allocated to nullBlock to free up memory
	    free(nullBlock);
        printf("Write Succeeded /n");
      
		}else{
            printf("Write failed /n");
        }
		return RC_OK;
	}
}
extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    pagefile = fopen(fileName,"r");
    if(pagefile != NULL){
     fHandle->fileName = fileName;
     fHandle->curPagePos = 0;
     //fseek moves the pointer to the end of the file, thus we get the total file size
     fseek(pagefile, 0L, SEEK_END);
     int totalPages = ftell(pagefile);
     //dividing the total file size by page size to get the total number of pages
     totalPages = totalPages/PAGE_SIZE;
     fHandle->totalNumPages = totalPages;
     fHandle->mgmtInfo = pagefile;
     printf("Returning RC_OK");
     fclose(pagefile);
     return RC_OK;
    }
    else{
        printf("file not found");
        return RC_FILE_NOT_FOUND;
    }

}
extern RC closePageFile (SM_FileHandle *fHandle){
    // Checking if file pointer or the storage manager is intialised. If initialised, then close.
	if(pagefile != NULL)
		pagefile = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName){
    //remove deleted the file and returns 0 if it's successfully deleted
    int temp  = remove(fileName);
	if(temp != 0){
        printf("File wasn't deleted");
        return RC_FILE_NOT_FOUND;
        
	}else{
        printf("File Deleted");
		return RC_OK; 
    }
}

/* reading blocks from disc */


extern RC readBlock (int pageNum ,SM_FileHandle *fHandle ,SM_PageHandle memPage )
{
//STEP 0: Check If FileHandle is not null
if(fHandle != NULL){
    //STEP 1: Page number should be non negative and less than total number of pages 
if (pageNum > 0 || pageNum > (*fHandle).totalNumPages )	{
	printf("Cannot read a non-existing page!");
	return RC_READ_NON_EXISTING_PAGE;
}
else{
    //STEP 2: Creating an empty file in read mode for reading only, and validate if it was created and opened successfully
    if(fopen((*fHandle).fileName, "r") == NULL)
    {
	printf("File not found!");
        return RC_FILE_NOT_FOUND;
    }
    //STEP 3:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
    //return 0 on successfull return
    if(fseek(pagefile, pageNum*PAGE_SIZE, SEEK_SET)==0)
    {
    //STEP 4:Reading items from the file
    RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
    //STEP 5:Count of Item's read should should be within page limit
    if(fileReadSize < PAGE_SIZE || fileReadSize > PAGE_SIZE)
	  printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 6:Set the position of the cursor to the pageNumber to read
     (*fHandle).curPagePos = pageNum;
	printf("Read Succeeded /n");
          return RC_OK;
    }
    else {
	printf("Cannot read a non-existing page!");
	return RC_READ_NON_EXISTING_PAGE; 
	}
}
}
else{
    printf("File not found!");
    return RC_FILE_NOT_FOUND;
}

}
extern int getBlockPos(SM_FileHandle *fHandle){
//STEP 0: Check If FileHandle is not null
if(fHandle != NULL){
//STEP 1: Return Page position
  printf("Block Position Successfully Returned /n");
   return (*fHandle).curPagePos;
}
else
{    printf("File not found!");
 return RC_FILE_NOT_FOUND; }
}

extern RC readFirstBlock( SM_FileHandle *fHandle , SM_PageHandle memPage ){
    //STEP 0: Check If FileHandle is not null
    if(fHandle!=NULL){
//STEP 1: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 2:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 if (((*fHandle).totalNumPages>0) && (fseek(pagefile, 0, SEEK_SET)==0))
 {
        //STEP 3:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
  if(fileReadSize < PAGE_SIZE || fileReadSize > PAGE_SIZE)
	  printf("Cannot read a non-existing page!");
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 4:Set the position of the cursor to the pageNumber to read
     (*fHandle).curPagePos = 0;
	printf("Read First Block Succeeded!");
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


extern RC readPreviousBlock ( SM_FileHandle *fHandle , SM_PageHandle memPage )
{
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
 if((*fHandle).totalNumPages>0 && (fseek(pagefile, (previousblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 3:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
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

extern RC readCurrentBlock ( SM_FileHandle *fHandle , SM_PageHandle memPage )
{
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
 if((fseek(pagefile, (currentblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 4:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
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

extern RC readNextBlock ( SM_FileHandle *fHandle , SM_PageHandle memPage )
{
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
 if((fseek(pagefile,(nextblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
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

extern RC readLastBlock( SM_FileHandle *fHandle , SM_PageHandle memPage )
{
    RC lastblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandle!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        lastblock =(*fHandle).totalNumPages - 1; 

		if((fseek(pagefile,(lastblock*PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
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

/* writing blocks to a page file */

RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)
{
   // Check if the file handle and management info are initialized
  if (fHandle == NULL || fHandle->mgmtInfo == NULL)
  {
    // Print error message if file is not available
    printf("Write Block Action Failure\n");
    printf("File doesn't exist.\n");
    // Return with error File not found
    return RC_FILE_HANDLE_NOT_INIT;
  }

  // Inspects the validity of page
  if (fHandle->totalNumPages <= pageNum)
  {
    printf("Write Block Action Failure\n");
    printf("Not a valid page number.\n");
    // Return with the write failed error
    return RC_WRITE_FAILED;
  }

  // Started seeking here
  if (fseek((FILE *)fHandle->mgmtInfo, PAGE_SIZE * pageNum, SEEK_SET) != 0)
  {
    // Print error message if seeking fails
    printf("Write Block Action Failed\n");
    printf("Couldn't reach the file's beginning.\n");
    // return with write failed error
    return RC_WRITE_FAILED;
  }

  if (fwrite(memPage, 1, PAGE_SIZE, (FILE *)fHandle->mgmtInfo) != PAGE_SIZE)
  {
    // failure to write
    printf("Write Block Action Failure\n");
    printf("Could not write data.\n");
    return RC_WRITE_FAILED;
  }

  // Updating the page
  fHandle->curPagePos = pageNum;

  return RC_OK;
}


RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage)
{
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



// append an empty new block to the end of the file, since the previous blocks are full
RC appendEmptyBlock(SM_FileHandle *fHandle)
{
   // Obtain the file pointer
  pagefile = (FILE *)fHandle->mgmtInfo;
  if (fHandle == NULL)
  {
    printf("Initializing is not done in File handle. Ending execution.\n");
    return RC_FILE_HANDLE_NOT_INIT;
  }

 // Create and allocate memory for a new block within the page size of the file
  SM_PageHandle newBlock = (char *)calloc(PAGE_SIZE, sizeof(char));

  fseek(pagefile, 0, SEEK_END);// Move to the end of the file to add a new block

  for (int i = 0; i < PAGE_SIZE; i++) // Write empty spaces to create an empty block
  {
     // Write null to create an empty page
    fwrite("\0", 1, 1, pagefile);
    // Move to the end of the file for additional empty spaces if needed
    fseek(pagefile, 0, SEEK_END);
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


extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)
{
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
