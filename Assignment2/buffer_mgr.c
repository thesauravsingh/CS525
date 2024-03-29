
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"


#define RC_QUEUE_IS_EMPTY 5;
#define RC_NO_FREE_BUFFER_ERROR 6;


typedef struct Page
{
	SM_PageHandle data; // Stores the actual data of the page.
	PageNumber pageNum; // Represents a unique identification integer assigned to each page.
	int dirtyBit; // Indicates whether the contents of the page have been modified by the client.
	int fixCount; // Tracks the number of clients currently using that page.
	int hitNum;   // Utilized by the Least Recently Used (LRU) algorithm to identify the least recently accessed page.	
	int refNum;   // Employed by the Least Frequently Used (LFU) algorithm to identify the least frequently accessed page.
} PageFrame;



/** 
bufferSize: Represents the maximum number of page frames that can be stored in the buffer pool.

rearIndex: Stores the count of the number of pages read from the disk. It is also utilized by the FIFO function to calculate the frontIndex.

writeCount: Counts the number of I/O writes to the disk, indicating the number of pages written to the disk.

hitCount: Serves as a general counter incremented whenever a page frame is added to the buffer pool. It is employed by the LRU algorithm to determine the least recently added page in the buffer pool.

clockPointer: Used by the CLOCK algorithm to point to the last added page in the buffer pool.

lfuPointer: Utilized by the LFU algorithm to store the position of the least frequently used page frame. It facilitates faster operations from the second replacement onwards.
**/
int bufferSize = 0;
int rearIndex = 0;
int writeCount = 0;
int hitCount = 0;
int clock_pointer = 0;
int lfuPointer = 0;



extern void FIFO(BM_BufferPool *const bm, PageFrame *page) {
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    int bufferSize = bm->numPages; // Access buffer pool size from the buffer pool structure

    int frontIndex = 0;
    int replaced = 0; // Flag to track if replacement occurred

    // Find the first unpinned frame (FIFO)
    for (int i = 0; i < bufferSize; i++) {
        if (pageFrame[frontIndex].fixCount == 0) {
            // If the page in the selected frame is dirty, write it back to disk
            if (pageFrame[frontIndex].dirtyBit == 1) {
                SM_FileHandle fh;
                openPageFile(bm->pageFile, &fh);
                writeBlock(pageFrame[frontIndex].pageNum, &fh, pageFrame[frontIndex].data);
                // Increment write count or perform other operations
            }

            // Replace the content of the selected frame with the incoming page
            pageFrame[frontIndex].data = page->data;
            pageFrame[frontIndex].pageNum = page->pageNum;
            pageFrame[frontIndex].dirtyBit = page->dirtyBit;
            pageFrame[frontIndex].fixCount = page->fixCount;
            
            replaced = 1; // Mark replacement occurred
            break; // Exit the loop after replacement
        } else {
            frontIndex = (frontIndex + 1) % bufferSize; // Move to the next frame
        }
    }

    if (!replaced) {
        // If no replacement occurred, report an error or handle it accordingly
        printf("Error: No unpinned frames available for replacement.\n");
        // You may choose to handle this case differently based on your requirements
    }
}


// Function to perform FIFO page replacement algorithm
// extern void FIFO(BM_BufferPool *const bm, PageFrame *page)
// {

// 	PageFrame *pageFrame = (PageFrame *) bm->mgmtData;
	
// 	int i, frontIndex;
// 	frontIndex = rearIndex % bufferSize;

// 	for(i = 0; i < bufferSize; i++)
// 	{
// 		if(pageFrame[frontIndex].fixCount == 0)
// 		{
// 			if(pageFrame[frontIndex].dirtyBit == 1)
// 			{
// 				SM_FileHandle fh;
// 				openPageFile(bm->pageFile, &fh);
// 				writeBlock(pageFrame[frontIndex].pageNum, &fh, pageFrame[frontIndex].data);
				
// 				writeCount++;
// 			}
			
// 			// Setting page frame's content 
// 			pageFrame[frontIndex].data = page->data;
// 			pageFrame[frontIndex].pageNum = page->pageNum;
// 			pageFrame[frontIndex].dirtyBit = page->dirtyBit;
// 			pageFrame[frontIndex].fixCount = page->fixCount;
// 			break;
// 		}
// 		else
// 		{
// 			frontIndex++;
// 			frontIndex = (frontIndex % bufferSize == 0) ? 0 : frontIndex;
// 		}
// 	}
// }
// Declaring the Least Frequently Used function
extern void LFU(BM_BufferPool *const bm, PageFrame *page) {
    
    PageFrame *pageFrame = (PageFrame *) bm->mgmtData;

    int i, j, leastFreqIndex, leastFreqRef;
    leastFreqIndex = lfuPointer;

    // Iterate through all the page frames in the buffer pool
    i = 0;
    while (i < bufferSize && pageFrame[leastFreqIndex].fixCount != 0) {
        leastFreqIndex = (leastFreqIndex + i) % bufferSize;
        leastFreqRef = pageFrame[leastFreqIndex].refNum;
        i++;
    }

    i = (leastFreqIndex + 1) % bufferSize;

    // Find the page frame with the minimum refNum (i.e., the least frequently used page frame)
    j = 0;
    while (j < bufferSize) {
        if (pageFrame[i].refNum < leastFreqRef) {
            leastFreqIndex = i;
            leastFreqRef = pageFrame[i].refNum;
        }
        i = (i + 1) % bufferSize;
        j++;
    }

    // If the page in memory has been modified (dirtyBit = 1), write the page to disk	
    if (pageFrame[leastFreqIndex].dirtyBit == 1) {
        SM_FileHandle fh;
        openPageFile(bm->pageFile, &fh);
        writeBlock(pageFrame[leastFreqIndex].pageNum, &fh, pageFrame[leastFreqIndex].data);

        // Increment the writeCount which records the number of writes done by the buffer manager.
        writeCount++;
    }

    // Set the content of the page frame to the content of the new page
    pageFrame[leastFreqIndex].data = page->data;
    pageFrame[leastFreqIndex].pageNum = page->pageNum;
    pageFrame[leastFreqIndex].dirtyBit = page->dirtyBit;
    pageFrame[leastFreqIndex].fixCount = page->fixCount;
    lfuPointer = leastFreqIndex + 1;
}

// Declaring LRU (Least Recently Used) function
extern void LRU(BM_BufferPool *const bufferPool, PageFrame *page) {
    // Cast the management data of buffer pool to page frames
    PageFrame *frames = (PageFrame *)bufferPool->mgmtData;
    // Initialize variables to track the least recently used page frame
    int index, leastRecentlyUsedIndex = -1, leastRecentlyUsedCount = INT_MAX;

    // Find the least recently used page frame with fixCount = 0
    for(index = 0; index < bufferSize; index++) {
        // Check if the current page frame has fixCount = 0 and its hitNum is lower than the least recently used count
        if(frames[index].fixCount == 0 && frames[index].hitNum < leastRecentlyUsedCount) {
            // Update the index and count of the least recently used page frame
            leastRecentlyUsedIndex = index;
            leastRecentlyUsedCount = frames[index].hitNum;
        }
    }    

    // If there's a page frame available for replacement
    if(leastRecentlyUsedIndex != -1) {
        // If the page in memory has been modified (dirtyBit = 1), write page to disk
        if(frames[leastRecentlyUsedIndex].dirtyBit == 1) {
            SM_FileHandle fileHandle;
            // Open the page file associated with the buffer pool
            openPageFile(bufferPool->pageFile, &fileHandle);
            // Write the page to disk
            writeBlock(frames[leastRecentlyUsedIndex].pageNum, &fileHandle, frames[leastRecentlyUsedIndex].data);
            // Increase the writeCount which records the number of writes done by the buffer manager
            writeCount++;
        }
        
        // Update the page frame's content to the new page's content
        frames[leastRecentlyUsedIndex].data = page->data;
        frames[leastRecentlyUsedIndex].pageNum = page->pageNum;
        frames[leastRecentlyUsedIndex].dirtyBit = page->dirtyBit;
        frames[leastRecentlyUsedIndex].fixCount = page->fixCount;
        frames[leastRecentlyUsedIndex].hitNum = page->hitNum;
    } else {
        // No page frame available for replacement, handle accordingly
        printf("Buffer pool is full. Cannot perform LRU replacement.\n");
    }
}

// Declaring the CLOCK function
extern void CLOCK(BM_BufferPool *const bm, PageFrame *page)
{
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    int initialClockPointer = clock_pointer;

    while (1)
    {
        clock_pointer = (clock_pointer % bufferSize == 0) ? 0 : clock_pointer;

        if (pageFrame[clock_pointer].hitNum != 1)
        {
            // If page in memory has been modified (dirtyBit = 1), then write page to disk
            if (pageFrame[clock_pointer].dirtyBit == 1)
            {
                SM_FileHandle fh;
                openPageFile(bm->pageFile, &fh);
                writeBlock(pageFrame[clock_pointer].pageNum, &fh, pageFrame[clock_pointer].data);

                // Increase the writeCount which records the number of writes done by the buffer manager.
                writeCount++;
            }

            // Setting page frame's content to new page's content
            pageFrame[clock_pointer].data = page->data;
            pageFrame[clock_pointer].pageNum = page->pageNum;
            pageFrame[clock_pointer].dirtyBit = page->dirtyBit;
            pageFrame[clock_pointer].fixCount = page->fixCount;
            pageFrame[clock_pointer].hitNum = page->hitNum;
            clock_pointer++;
            break;
        }
        else
        {
            // Reset hitNum to 0 to avoid an infinite loop
            pageFrame[clock_pointer].hitNum = 0;
            clock_pointer++;

            // If we have checked all page frames and couldn't find a suitable one, reset clockPointer to the initial position
            if (clock_pointer == initialClockPointer)
            {
                break;
            }
        }
    }
}

// Buffer Manager Interface Pool Handling
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy,void *stratData)
{
        //valid inputs check
    if (bm == NULL || pageFileName == NULL || numPages <= 0) {
        return RC_ERROR;
    }
	bufferSize = numPages;	
    bm->pageFile = malloc(strlen(pageFileName) + 1);
    if (bm->pageFile == NULL) {
        return RC_FILE_NOT_FOUND;
    }
    strcpy(bm->pageFile, pageFileName);

    // Set other buffer pool properties
    bm->numPages = numPages;
    bm->strategy = strategy;

    PageFrame *pageFrames = malloc(sizeof(PageFrame) * numPages);
    if (pageFrames == NULL) {
        free(bm->pageFile);
        return RC_FILE_NOT_FOUND;
    }

    // Initialize page frames
    for (int i = 0; i < numPages; i++) {
        pageFrames[i].data = NULL;
        pageFrames[i].pageNum = -1;
        pageFrames[i].dirtyBit = 0;
        pageFrames[i].fixCount = 0;
        pageFrames[i].hitNum = 0;
        pageFrames[i].refNum = 0;
    }

    // Set buffer pool properties
    bm->mgmtData = pageFrames;

    // Additional initialization based on strategy or other parameters can be added here

    return RC_OK;
}
extern RC shutdownBufferPool(BM_BufferPool *const bm)
{
    // Write all dirty pages (modified pages) back to disk
    forceFlushPool(bm);

    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;

    // Check for pinned pages
    for (int i = 0; i < bm->numPages; i++)
    {
        // If fixCount != 0, it means that the contents of the page were modified by some client and have not been written back to disk.
        if (pageFrame[i].fixCount != 0)
        {
            return RC_PINNED_PAGES_IN_BUFFER;
        }
    }

    // Releasing space occupied by the page frames
    free(pageFrame);
    bm->mgmtData = NULL;

    return RC_OK;
}

extern RC forceFlushPool(BM_BufferPool *const bm)
{
    if (bm == NULL || bm->mgmtData == NULL) {
        return RC_ERROR;
    }

    FILE *fp = fopen(bm->pageFile, "r+");  // Open the page file for both reading and writing

    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    PageFrame *pageFrames = (PageFrame *)(bm->mgmtData);

    for (int i = 0; i < bm->numPages; i++) {
        if (pageFrames[i].dirtyBit == 1 && pageFrames[i].fixCount == 0) {
            // Write the dirty page to disk only if not in use (fixCount == 0)
            fseek(fp, PAGE_SIZE * pageFrames[i].pageNum, SEEK_SET);
            fwrite(pageFrames[i].data, 1, PAGE_SIZE, fp);
            pageFrames[i].dirtyBit = 0;  // Reset dirty bit after writing to disk
        }
    }

    fclose(fp);

    return RC_OK;
}

// Buffer Manager Interface Access Pages

// This function marks page as dirty indicating that the data of the page has been modified the client
extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    int i;
    // Iterating through all the pages in the buffer
    for(i = 0; i < bufferSize; i++)
    {
        // If the current page the page to be marked dirty, then set dirtyBit = 1 (page has been modified) for that page
        if(pageFrame[i].pageNum == page->pageNum)
        {
            pageFrame[i].dirtyBit = 1;
            return RC_OK;        
        }            
    }        
    return RC_ERROR;
}

// This function unpins a page from the memory i.e. removes a page from the memory
extern RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page)
{   
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    int i;
    // Iterating through all the pages in the buffer pool
    for(i = 0; i < bufferSize; i++)
    {
        // If the current page is the page to be unpinned, then decrease fixCount (which means client has completed work on that page) and exit loop
        if(pageFrame[i].pageNum == page->pageNum)
        {
            pageFrame[i].fixCount--;
            break;      
        }       
    }
    return RC_OK;
}

// This function writes the contents of the modified pages back to the page file on disk
extern RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page)
{
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    int i;
    // Iterating through all the pages in the buffer pool
    for(i = 0; i < bufferSize; i++)
    {
        // If the current page = page to be written to disk, then right the page to the disk using the storage manager functions
        if(pageFrame[i].pageNum == page->pageNum)
        {       
            SM_FileHandle fh;
            openPageFile(bm->pageFile, &fh);
            writeBlock(pageFrame[i].pageNum, &fh, pageFrame[i].data);
        
            // Mark page as undirty because the modified page has been written to disk
            pageFrame[i].dirtyBit = 0;
            
            // Increase the writeCount which records the number of writes done by the buffer manager.
            writeCount++;
        }
    }   
    return RC_OK;
}


extern RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum)
{
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    int i;
    // Checking if the page is already in the buffer pool
    for(i = 0; i < bufferSize; i++)
    {
        if(pageFrame[i].pageNum == pageNum)
        {
            pageFrame[i].fixCount++;
            pageFrame[i].refNum++;
            hitCount++;
            return RC_OK;
        }
    }
    
    // If the page is not in the buffer pool, then replace a page using the clock replacement strategy
    int clockIndex = rearIndex;
    do
    {
        clockIndex = (clockIndex + 1) % bufferSize;
        if(pageFrame[clockIndex].fixCount == 0)
        {
            // If the page is dirty, then write it to disk before replacing it
            if(pageFrame[clockIndex].dirtyBit == 1)
            {
                SM_FileHandle fh;
                openPageFile(bm->pageFile, &fh);
                writeBlock(pageFrame[clockIndex].pageNum, &fh, pageFrame[clockIndex].data);
                writeCount++;
            }
            
            // Reading the new page from disk and initializing page frame's content
            pageFrame[clockIndex].data = (SM_PageHandle) malloc(PAGE_SIZE);
            SM_FileHandle fh;
            openPageFile(bm->pageFile, &fh);
            ensureCapacity(pageNum,&fh);
            readBlock(pageNum, &fh, pageFrame[clockIndex].data);
            pageFrame[clockIndex].pageNum = pageNum;
            pageFrame[clockIndex].fixCount++;
            rearIndex = clockIndex;
            pageFrame[clockIndex].hitNum = hitCount;
            pageFrame[clockIndex].refNum = 0;
            page->pageNum = pageNum;
            page->data = pageFrame[clockIndex].data;
            return RC_OK;
        }
    } while(clockIndex != rearIndex);
    
    // If all pages are fixed, then return an error
    return RC_ERROR;
}


PageNumber *allocateFrameContents(int bufferSize) {
    return (PageNumber *)malloc(sizeof(PageNumber) * bufferSize);
}

// Function to get frame contents
extern PageNumber *getFrameContents(BM_BufferPool *const bm) {
    PageNumber *frameContents = allocateFrameContents(bufferSize);
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    // Iterate through all the pages in the buffer pool and set frameContents' value to pageNum of the page
    for (int i = 0; i < bufferSize; i++) {
        frameContents[i] = (pageFrame[i].pageNum != -1) ? pageFrame[i].pageNum : NO_PAGE;
    }
    
    return frameContents;
}


bool *allocateDirtyFlags(int bufferSize) {
    return (bool *)malloc(sizeof(bool) * bufferSize);
}

// Function to get dirty flags
extern bool *getDirtyFlags(BM_BufferPool *const bm) {
    bool *dirtyFlags = allocateDirtyFlags(bufferSize);
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    
    // Iterate through all the pages in the buffer pool and set dirtyFlags based on the dirtyBit of the page
    for (int i = 0; i < bufferSize; i++) {
        dirtyFlags[i] = (pageFrame[i].dirtyBit == 1) ? true : false;
    }
    
    return dirtyFlags;
}


/**
This function takes three parameters: pageFrame, fixCounts, and bufferSize.
It iterates over bufferSize elements of the pageFrame array.
For each element, it checks if the fixCount is not equal to -1. If true, it assigns the fixCount to the corresponding index of fixCounts. Otherwise, it assigns 0.
This function effectively calculates the fix counts for each page frame and stores them in the fixCounts array.
**/
void calculateFixCounts(PageFrame *pageFrame, int *fixCounts, int bufferSize) {
    int i = 0;
    while(i < bufferSize) {
        fixCounts[i] = (pageFrame[i].fixCount != -1) ? pageFrame[i].fixCount : 0;
        i++;
    }
}

/**
This function takes a single parameter bm, which is a pointer to a BM_BufferPool structure representing a buffer pool handler.
It allocates memory for an integer array fixCounts of size bufferSize, where bufferSize is a global variable or defined elsewhere.
It retrieves the pageFrame array from the bm buffer pool handler.
It then calls the calculateFixCounts function to compute the fix counts and populate the fixCounts array.
Finally, it returns the fixCounts array containing the fix counts for each page frame.
**/
extern int *getFixCounts(BM_BufferPool *const bm) {
    int *fixCounts = malloc(sizeof(int) * bufferSize);
    PageFrame *pageFrame = (PageFrame *)bm->mgmtData;
    calculateFixCounts(pageFrame, fixCounts, bufferSize);
    return fixCounts;
}


/**
This function, getNumReadIO, takes a single parameter bm, which is a pointer to a BM_BufferPool structure representing a buffer pool handler.
It returns the number of pages that have been read from disk since the buffer pool has been initialized.
The implementation returns the value of rearIndex incremented by one. This is because rearIndex tracks the number of pages read from disk, and adding one accounts for starting the index from zero.
**/
extern int getNumReadIO (BM_BufferPool *const bm)
{
    int pageread = rearIndex + 1;
	return pageread;
}

/**
The getNumWriteIO function takes a single parameter bm, which is a pointer to a BM_BufferPool structure representing a buffer pool handler.
It returns the number of write operations that have been performed since the buffer pool has been initialized.
The implementation directly returns the value of writeCount, which tracks the number of write operations. 
**/
extern int getNumWriteIO (BM_BufferPool *const bm)
{
	return writeCount;
}
