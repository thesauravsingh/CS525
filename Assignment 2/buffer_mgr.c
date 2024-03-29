
#include <stdlib.h>
#include "buffer_mgr.h"
#include "buffer_mgr_stat.h"
#include <limits.h>
#include "dberror.h"
#include "dt.h"
#include "storage_mgr.h"

// Global variables to keep track of logical timestamps for operations
int logicClockForOperation = 0;
int logicClockForLoadToMemoryOperation = 0;
// Constant to represent an uninitialized page number
const int INIT_PAGE_NUMBER = -1;


// Function to initialize a buffer pool with the given parameters
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData) {
 
    // Initialize buffer pool statistics
    bm->numReads = 0;
    bm->numWrites = 0;
    bm->pageFile = (char *)pageFileName;
    bm->numPages = numPages;
    bm->strategy = strategy;

   
    // Allocate memory for page frames
    BM_PageFrame *head = malloc(sizeof(BM_PageFrame) * numPages);
    int index = 0;
    // Initialize each page frame
    while (index < bm->numPages) {
        head[index].data = NULL;
        head[index].pageNumber = INIT_PAGE_NUMBER;
        head[index].isDirty = false;
        head[index].fixCount = 0;
        head[index].lastOperationLogicTimestamp = 0;
        head[index].loadToMemoryLogicTimestamp = 0;
        index++;
    }
    // Set the page frames as the buffer pool's management data
    bm->mgmtData = head;
    return RC_OK;
}

// Function to shutdown the buffer pool and free allocated memory
RC shutdownBufferPool(BM_BufferPool *const bm) {
    // Write all dirty pages to disk before shutdown
    forceFlushPool(bm);

    // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *) bm->mgmtData;

    // Free memory allocated for page data
    int i = 0;
    while (i < bm->numPages) {
        if (head[i].pageNumber != INIT_PAGE_NUMBER) {
            free(head[i].data);
        }
        i++;
    }

    // Free the buffer pool's management data
    free(bm->mgmtData);
    return RC_OK;
}

// Function to write a page from memory to disk
void writePageToDisk(BM_BufferPool *bm, BM_PageFrame *head, int pageIndexInMemory) {
    // Increment the write count
    bm->numWrites ++;
    // Open the page file
    SM_FileHandle fileHandle;
    openPageFile(bm->pageFile, &fileHandle);
    // Write the page to disk
    writeBlock(head[pageIndexInMemory].pageNumber, &fileHandle, head[pageIndexInMemory].data);
    // Close the page file
    closePageFile(&fileHandle);
    // Mark the page as not dirty
    head[pageIndexInMemory].isDirty = false;
}

// Function to force flushing of all dirty pages to disk
RC forceFlushPool(BM_BufferPool *const bm) {
    // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *) bm->mgmtData;
    int i = 0;
    // Iterate through each page frame
    while (i < bm->numPages) {
        // If the page is not fixed and is dirty, write it to disk
        if (head[i].fixCount == 0 && head[i].isDirty == true) {
            writePageToDisk(bm, head, i);
            head[i].isDirty = false;
        }
        i++;
    }
    return RC_OK;
}

// Function to mark a page as dirty in the buffer pool
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page) {
   // Get the start address of the page frames array
   BM_PageFrame *head = (BM_PageFrame *) bm->mgmtData;
    bool pageFound = false;
    int pageIndexInMemory = -1;
    int i = 0;
    // Find the page in the buffer pool
    while (i < bm->numPages) {
        if (head[i].pageNumber == page->pageNum) {
            pageFound = true;
            pageIndexInMemory = i;
            break;
        }
        i++;
    }

    // If the page is found, mark it as dirty
    if (pageFound) {
        printf("markDirty %d", page->pageNum);
        head[pageIndexInMemory].isDirty = true;
    } else {
        printf("cannot find page %d in markDirty, ignore", page->pageNum);
    }

    return RC_OK;
}

// Function to unpin a page in the buffer pool
RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page) {
    // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *) bm->mgmtData;
    bool pageFound = false;
    int pageIndexInMemory = -1;
    int i = 0;
    // Find the page in the buffer pool
    while (i < bm->numPages) {
        if (head[i].pageNumber == page->pageNum) {
            pageFound = true;
            pageIndexInMemory = i;
            break;
        }
        i++;
    }

    // If the page is found and its fix count is greater than 0, decrement the fix count
    if (pageFound && head[pageIndexInMemory].fixCount > 0) {
        printf("unpinPage %d", page->pageNum);
        head[pageIndexInMemory].fixCount--;
    } else {
        printf("cannot find page %d with fixCount > 0 in memory in unpinPage, ignore", page->pageNum);
    }

    return RC_OK;
}

// Function to force writing a specific page back to disk
RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page) {
     // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *)bm->mgmtData;
    bool pageFound = false;
    int pageIndexInMemory = -1;
    int i = 0;
    // Find the page in the buffer pool
    while (i < bm->numPages) {
        if (head[i].pageNumber == page->pageNum) {
            pageFound = true;
            pageIndexInMemory = i;
            break;
        }
        i++;
    }

    // If the page is found and is dirty, write it back to disk
    if (pageFound && head[pageIndexInMemory].isDirty == true) {
        printf("Write page %d back to disk in forcePage", page->pageNum);
        writePageToDisk(bm, head, pageIndexInMemory);
    } else {
        printf("Cannot find dirty page %d in memory in forcePage, ignore", page->pageNum);
    }

    return RC_OK;
}

// Function to evict a page from the buffer pool based on the replacement strategy
int evict(BM_BufferPool *const bm) {
   // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *) bm->mgmtData;

    int toEvictPageIndex = -1;

    // Get all pages in memory that are not currently fixed by clients
    int *noClientPages = NULL;
    int noClientPagesSize = 0;
    int i = 0;
    while (i < bm->numPages) {
        if (head[i].fixCount == 0) {
            noClientPagesSize++;
            noClientPages = (int *)realloc(noClientPages, noClientPagesSize * sizeof(int));
            noClientPages[noClientPagesSize - 1] = i;
        }
        i++;
    }

    // Determine the page to evict based on the replacement strategy
    if (bm->strategy == RS_FIFO) {
        int pageEarliestLoadLogicTimestamp = INT_MAX;
        i = 0;
        while (i < noClientPagesSize) {
            if (head[noClientPages[i]].loadToMemoryLogicTimestamp < pageEarliestLoadLogicTimestamp) {
                pageEarliestLoadLogicTimestamp = head[noClientPages[i]].loadToMemoryLogicTimestamp;
                toEvictPageIndex = noClientPages[i];
            }
            i++;
        }
    } else if (bm->strategy == RS_LRU) {
        int pageOldestUsed = INT_MAX;
        i = 0;
        while (i < noClientPagesSize) {
            if (head[noClientPages[i]].lastOperationLogicTimestamp < pageOldestUsed) {
                pageOldestUsed = head[noClientPages[i]].lastOperationLogicTimestamp;
                toEvictPageIndex = noClientPages[i];
            }
            i++;
        }
    } else {
        printf("Not implemented yet %d", bm->strategy);
    }

    // If a page to evict is found, write it to disk if it's dirty and free its memory
    if (toEvictPageIndex != -1) {
        if (head[toEvictPageIndex].isDirty == true) {
            writePageToDisk(bm, head, toEvictPageIndex);
        }
        free(head[toEvictPageIndex].data);
    }

    return toEvictPageIndex;
}

// Function to pin a page in the buffer pool
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum) {
    page->pageNum = pageNum;

    // Update the logical clock for the current operation
    logicClockForOperation++;

    // Get the start address of the page frames array
    BM_PageFrame *head = (BM_PageFrame *)bm->mgmtData;

    bool pageInMemory = false;
    int pageIndexInMemory = -1;
    int i = 0;
    // Check if the page is already in memory
    while (i < bm->numPages) {
        if (head[i].pageNumber == pageNum) {
            pageInMemory = true;
            pageIndexInMemory = i;
            break;
        }
        i++;
    }

    if (pageInMemory) {
        // If the page is already in memory
        // 1.1 Return the page data
        page->data = head[pageIndexInMemory].data;
        // 1.2 Update the page metadata
        head[pageIndexInMemory].fixCount++;
        head[pageIndexInMemory].lastOperationLogicTimestamp = logicClockForOperation;
    } else {
        // If the page is not in memory
        // Increment the read count
        bm->numReads++;

        // Update the logical clock for loading a page into memory
        logicClockForLoadToMemoryOperation++;

        // Check if there is free space in the buffer pool
        bool isBufferPoolFull = true;
        int firstFreePageIndexInMemory = -1;
        i = 0;
        while (i < bm->numPages) {
            if (head[i].pageNumber == INIT_PAGE_NUMBER) {
                isBufferPoolFull = false;
                firstFreePageIndexInMemory = i;
                break;
            }
            i++;
        }

        if (isBufferPoolFull) {
            // If the buffer pool is full, evict a page
            int evictedPageIndexInMemory = evict(bm);
            firstFreePageIndexInMemory = evictedPageIndexInMemory;
        }

        // Read the page from disk and store it in memory
        SM_FileHandle fileHandle;
        openPageFile(bm->pageFile, &fileHandle);
        ensureCapacity(pageNum + 1, &fileHandle);
        head[firstFreePageIndexInMemory].data = (SM_PageHandle)malloc(PAGE_SIZE);
        readBlock(pageNum, &fileHandle, head[firstFreePageIndexInMemory].data);
        closePageFile(&fileHandle);
        head[firstFreePageIndexInMemory].pageNumber = pageNum;
        head[firstFreePageIndexInMemory].isDirty = false;
        head[firstFreePageIndexInMemory].fixCount = 1;
        head[firstFreePageIndexInMemory].lastOperationLogicTimestamp = logicClockForOperation;
        head[firstFreePageIndexInMemory].loadToMemoryLogicTimestamp = logicClockForLoadToMemoryOperation;

        page->data = head[firstFreePageIndexInMemory].data;
    }

    return RC_OK;
}


// Function to retrieve an array of page numbers currently stored in the buffer pool
PageNumber *getFrameContents (BM_BufferPool *const bm) {
    // Get the start address of the page frames array
    BM_PageFrame *page = (BM_PageFrame *)bm->mgmtData;
    // Allocate memory for the result array
    PageNumber *result = malloc(sizeof(PageNumber) * bm->numPages);
    // Retrieve the page numbers from the buffer pool
    int i = 0;
    while (i < bm->numPages) {
        if (page[i].pageNumber == INIT_PAGE_NUMBER) {
            result[i] = NO_PAGE;
        } else {
            result[i] = page[i].pageNumber;
        }
        i++;
    }
    return result;
}

// Function to retrieve an array indicating whether each page in the buffer pool is dirty
bool *getDirtyFlags (BM_BufferPool *const bm) {
    // Get the start address of the page frames array
    BM_PageFrame *page = (BM_PageFrame *)bm->mgmtData;
    // Allocate memory for the result array
    bool *result = malloc(sizeof(bool) * bm->numPages);
    // Retrieve the dirty flags from the buffer pool
    int i = 0;
    while (i < bm->numPages) {
        result[i] = page[i].isDirty;
        i++;
    }
    return result;
}

// Function to retrieve an array containing the fix count of each page in the buffer pool
int *getFixCounts (BM_BufferPool *const bm) {
   // Get the start address of the page frames array
    BM_PageFrame *page = (BM_PageFrame *) bm->mgmtData;
    // Allocate memory for the result array
    int *result = malloc(sizeof(int) * bm->numPages);
    // Retrieve the fix counts from the buffer pool
    int i = 0;
    while (i < bm->numPages) {
        result[i] = page[i].fixCount;
        i++;
    }
    return result;
}

// Function to get the total number of read operations performed on the buffer pool
int getNumReadIO (BM_BufferPool *const bm) {
   // Return the accumulated number of read operations
    return bm->numReads;
}

// Function to get the total number of write operations performed on the buffer pool
int getNumWriteIO (BM_BufferPool *const bm) {
   // Return the accumulated number of write operations
    return bm->numWrites;
}