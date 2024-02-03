#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<math.h>
#include "storage_mgr.h"


FILE *pagefile;

bool pageExist(int pageNum){
    if(pageNum>0)
            return true;
    else{
        return false
    }

}

extern RC readBlock ( int pageNum , SM_FileHandle *fHandle , SM_PageHandle memPage )
{
//STEP 0: Check If FileHandle is not null
if(fHandler!=NULL){
    //STEP 1: Page number should be non negative and less than total number of pages 
if (pageExist(pageNum) || pageNum > (*fHandle).totalNumPages )	
		return RC_READ_NON_EXISTING_PAGE;
else{
    //STEP 2: Creating an empty file in read mode for reading only, and validate if it was created and opened successfully
    if(fopen((*fHandle).fileName, "r")==NULL)
    {
        return RC_FILE_NOT_FOUND;
    }
    //STEP 3:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
    //return 0 on successfull return
    if(fseek(pagefile, pageNum*PAGE_SIZE, SEEK_SET)==0)
    {
    //STEP 4:Reading iteams from the file
    RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
    //STEP 5:Count of Item's read should should be within page limit
    if(fileReadSize<PAGE_SIZE || fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 6:Set the position of the cursor to the pageNumber to read
     (*fHandle).curPagePos = pageNum;
          return RC_OK;
    }
    else {
		return RC_READ_NON_EXISTING_PAGE; 
	}
}
}
else{
    return RC_FILE_NOT_FOUND;
}

}


extern int getBlockPos (SM_FileHandle*fHandle){
//STEP 0: Check If FileHandle is not null
if(fHandler!=NULL)
//STEP 1: Return Page position
   return (*fHandle).curPagePos;
else
{
	return RC_FILE_NOT_FOUND; 
}
}

extern RC readFirstBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage ){
    //STEP 0: Check If FileHandle is not null
    if(fHandler!=NULL){
//STEP 1: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 2:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 if (pageExist((*fHandle).totalNumPages) && (fseek(pagefile, 0, SEEK_SET)==0))
 {
        //STEP 3:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
  if(fileReadSize<PAGE_SIZE || fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 4:Set the position of the cursor to the pageNumber to read
     (*fHandle).curPagePos = pageNum;
          return RC_OK;
 }else{
		return RC_READ_NON_EXISTING_PAGE;
 }
}
 else {
		return RC_FILE_NOT_FOUND; 
	}
}


extern RC readPreviousBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage )
{
    RC previousblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandler!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        previousblock =(*fHandle).curPagePos - 1; 

		if (previousblock < 0)
			return RC_READ_NON_EXISTING_PAGE;
//STEP 1: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 2:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 else {
 if(!pageExist((*fHandle).totalNumPages) && (fseek(pagefile, (previousblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 3:Reading iteams from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
  if(fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 4:Set the position of the cursor to the pageNumber to read
    return RC_OK;
 }else{
		return RC_READ_NON_EXISTING_PAGE;
 }
}
}
 else {
		return RC_FILE_NOT_FOUND; 
	}
}

extern RC readCurrentBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage )
{
    RC currentblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandler!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
         currentblock =(*fHandle).curPagePos;

		if (currentblock < 0)
			return RC_READ_NON_EXISTING_PAGE;
//STEP 1: Page number should be not be NULL and total pages in the file should be greater than 0
//STEP 2:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 else {
 if(!pageExist((*fHandle).totalNumPages) && (fseek(pagefile, (currentblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 3:Reading items from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
  if(fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;

     //STEP 4:Set the position of the cursor to the pageNumber to read
    return RC_OK;
 }else{
		return RC_READ_NON_EXISTING_PAGE;
 }
}
}
 else {
		return RC_FILE_NOT_FOUND; 
	}
}

extern RC readNextBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage )
{
     RC nextblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandler!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        nextblock =(*fHandle).curPagePos + 1; 

		if (nextblock < 0 || nextblock > (*fHandle).totalNumPages)
			return RC_READ_NON_EXISTING_PAGE;

 else {
    //STEP 1:Moving pointer of the file to the calculated position i.e.(pageNum*PAGE_SIZE)
//return 0 on successfull return
 if((fseek(pagefile,(nextblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading iteams from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
        //STEP 3:Page Size should not be greater than 0 but not greater then page size

  if(!pageExist(fileReadSize) && fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;
     //STEP 4:Set the position of the cursor to the next block position
  (*fHandle).curPagePos = fileReadSize;
    return RC_OK;
 }else{
		return RC_READ_NON_EXISTING_PAGE;
 }
}
}
 else {
		return RC_FILE_NOT_FOUND; 
	}
}

extern RC readLastBlock ( SM_FileHandle * fHandle , SM_PageHandle memPage )
{
    RC lastblock;
    //STEP 0: Check If FileHandle is not null
    if(fHandler!=NULL){
        //STEP 1: Set the Position of the Block to previous i.e. (current -1)
        lastblock =(*fHandle).totalNumPages - 1; 

		if((fseek(pagefile,(lastblock * PAGE_SIZE), SEEK_SET)==0))
 {
        //STEP 2:Reading iteams from the file
RC fileReadSize =fread(memPage, sizeof(char), PAGE_SIZE, pagefile);
        //STEP 3:Page Size should not be greater than 0 but not greater then page size
(*fHandle).curPagePos = lastblock;
  if(!pageExist(fileReadSize) && fileReadSize >PAGE_SIZE)
          return RC_READ_NON_EXISTING_PAGE;
     //STEP 4:Set the position of the cursor to the next block position
    return RC_OK;
 }else{
		return RC_READ_NON_EXISTING_PAGE;
 }
}
 else {
		return RC_FILE_NOT_FOUND; 
	}
}