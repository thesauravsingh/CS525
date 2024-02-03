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
        printf("Azha Manzoor \n");
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
      return RC_OK;
		}else{
            printf("Write failed /n");
        }
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
extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern int getBlockPos (SM_FileHandle *fHandle){

}
extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}

/* writing blocks to a page file */
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){

}
extern RC appendEmptyBlock (SM_FileHandle *fHandle){

}
extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){

}
