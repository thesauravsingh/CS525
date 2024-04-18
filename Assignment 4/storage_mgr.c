#include<stdio.h>
#include<stdlib.h>
#include "storage_mgr.h"
#include<string.h>


FILE *page;

//initializing page handler to NUll for the storage manager.
extern void initStorageManager (void){
    page = NULL;
}

//initializing page handler to NUll for the storage manager.
extern RC createPageFile(char *fileName) {
    FILE *page = fopen(fileName, "w+");

    if (page == NULL) {
        return RC_FILE_NOT_FOUND;
    } else {
        SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));

        if (emptyBlock != NULL && fwrite(emptyBlock, sizeof(char), PAGE_SIZE, page) >= PAGE_SIZE) {
            // Flushing all file buffers and freeing the memory to prevent memory leaks.
            fclose(page);
            free(emptyBlock);
            return RC_OK;
        }

        // Handling failure to allocate memory for emptyBlock or write to the file.
        if (emptyBlock != NULL) {
            free(emptyBlock);
        }

        fclose(page);
        return RC_WRITE_FAILED;
    }
}


extern RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *page = fopen(fileName, "r");

    if (page == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    fHandle->fileName = fileName;

    // Finding total number of pages by getting total file size and dividing by PAGE_SIZE
    fseek(page, 0L, SEEK_END);
    int fileSize = ftell(page);
    rewind(page);
    fHandle->totalNumPages = fileSize / PAGE_SIZE;

    fHandle->mgmtInfo = page;
    fHandle->curPagePos = 0;

    return RC_OK;
}


//closing all file handlers
extern RC closePageFile(SM_FileHandle *fileHandle) {
    int closeResult = fclose(fileHandle->mgmtInfo);
    
    if (closeResult == EOF) {
        return RC_ERROR;
    }

    return RC_OK;
}

extern RC destroyPageFile(char *fileName) {
    int removeResult = remove(fileName);

    if (removeResult == 0) {
        return RC_OK;
    } else {
        return RC_FILE_NOT_FOUND; 
    }
}


//This method reads blocks from the file specified by pageNum parameter and and stores the contents in the memory pointed to by memPage.
extern RC readBlock(int pageNum, SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (fileHandle->mgmtInfo == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    if (pageNum < 0 && pageNum >fileHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Move the cursor to the starting of the block by multiplying pageNum with PAGE_SIZE
    int seekResult = fseek(fileHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET);

    if (seekResult != 0) {
        return RC_ERROR;
    }

    // Read from the beginning of the page number pointed by the cursor
    fread(memPage, sizeof(char), PAGE_SIZE, fileHandle->mgmtInfo);
    fileHandle->curPagePos = pageNum;

    return RC_OK;
}


//returns the current cursor position.

extern int getBlockPos(SM_FileHandle *fileHandle) {
    return fileHandle->curPagePos;
}

extern RC readFirstBlock(SM_FileHandle *fHandle , SM_PageHandle memPage){
    readBlock(0,fHandle,memPage);
}

extern RC readLastBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    if (fileHandle->mgmtInfo == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    int totalPages = fileHandle->totalNumPages;
    return readBlock(totalPages - 1, fileHandle, memPage);
}

extern RC readPreviousBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    int currentPage = getBlockPos(fileHandle);

    if (currentPage <= 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    int previousPage = currentPage - 1;
    return readBlock(previousPage, fileHandle, memPage);
}

extern RC readCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    int currentPage = getBlockPos(fileHandle);
    return readBlock(currentPage, fileHandle, memPage);
}

extern RC readNextBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    int currentPage = getBlockPos(fileHandle);
    int nextPage = currentPage + 1;
    return readBlock(nextPage, fileHandle, memPage);
}

//This method writes a page to the disk. It takes contents from the memPage PageHandle and writes it to the Disk.
extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){

	if(pageNum > fHandle->totalNumPages || pageNum <0){
		return RC_WRITE_FAILED;
	}
	  page = fopen(fHandle->fileName,"r+");
	  if(page == NULL){
	    return RC_FILE_NOT_FOUND;
	  }
  int val = fseek(page, pageNum*PAGE_SIZE,SEEK_SET);
	if(val != 0){
		return RC_ERROR;
	}else{
    fwrite(memPage,sizeof(char),strlen(memPage),page);
		fHandle->curPagePos = pageNum;
		fclose(page);

		return RC_OK;
  }
}


extern RC writeCurrentBlock(SM_FileHandle *fileHandle, SM_PageHandle memPage) {
    int currentPosition = getBlockPos(fileHandle);
    fileHandle->totalNumPages += 1;
    return writeBlock(currentPosition, fileHandle, memPage);
}

extern RC appendEmptyBlock(SM_FileHandle *fileHandle) {
    FILE *page = fopen(fileHandle->fileName, "r+");

    if (page == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    int totalPages = fileHandle->totalNumPages;
    fileHandle->totalNumPages += 1;

    fseek(page, totalPages * PAGE_SIZE, SEEK_SET);
    char c = 0;

    for (int i = 0; i < PAGE_SIZE; i++) {
        fwrite(&c, sizeof(c), 1, page);
    }

    fclose(page);
    return RC_OK;
}

extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fileHandle) {
    int pagesToAdd = numberOfPages - fileHandle->totalNumPages;

    if (pagesToAdd > 0) {
        for (int i = 0; i < pagesToAdd; i++) {
            RC result = appendEmptyBlock(fileHandle);
            if (result != RC_OK) {
                return result;
            }
        }
    }

    return (fileHandle->totalNumPages == numberOfPages) ? RC_OK : RC_ERROR;
}
