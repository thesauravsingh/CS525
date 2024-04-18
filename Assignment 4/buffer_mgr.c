#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct Frame {
        SM_PageHandle content;
        int dirty;
        int pinStatus;
        int freeStat;
        int fixCount;
        PageNumber PageNumber;
        void *qPointer ;
} Frame;

typedef struct Queue {
        int count;
        int position;
        Frame *framePointer;
        int pNo;
} Queue;

int bufferSize= 0;
int maxBufferSize ;
int queueSize;
int isBufferFull= 0;
int currentQueueSize;
int writeCount = 0;
int readCount = 0;

Queue *q;
//This function initiates the buffer manager with the necessary parameters.
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData){

        queueSize = maxBufferSize = numPages;
        currentQueueSize = 0;
        writeCount= 0;
        readCount =0;

        bm->pageFile = (char *)pageFileName;
        bm->numPages = numPages;
        bm->strategy = strategy;
        int i;
        Frame *pageFrame = malloc(sizeof(Frame) * numPages);
        Queue *queueFrame = malloc(sizeof(Queue) * numPages);

        //Initializing all the values in the PageFrame and Queue.
        for(i=0; i<numPages; i++) {
                pageFrame[i].content = (SM_PageHandle)malloc(PAGE_SIZE);
                pageFrame[i].dirty = 0;
                pageFrame[i].pinStatus = 0;
                pageFrame[i].freeStat = 0;
                pageFrame[i].PageNumber = -1;
                pageFrame[i].fixCount = 0;

                queueFrame[i].count = 0;
                queueFrame[i].position = 0;
                queueFrame[i].framePointer = NULL;
                queueFrame[i].pNo = -1;
        }

        pageFrame[0].qPointer = queueFrame;
        q = queueFrame;

        bm->mgmtData = pageFrame;
        return RC_OK;
}

Frame *returnPagePointer(BM_BufferPool *const bm, BM_PageHandle *const page) {
    Frame *pageFrames = (Frame *)bm->mgmtData;
    
    for (int i = 0; i < maxBufferSize; i++) {
        if (page->pageNum == pageFrames[i].PageNumber) {
            return &pageFrames[i];
        }
    }
    return NULL;  
}

int maxQueue(Queue *q){
        int max = -1;
        for(int i = 0; i < currentQueueSize; i++) {
                if(q[i].position > max) {
                        max = q[i].position;
                }
        }
        return max;
}

void LRU(BM_BufferPool *const bm, BM_PageHandle *const page, int pageNum) {
    Frame *pageFrames = (Frame *)bm->mgmtData;
    Queue *queue = (Queue *)pageFrames[0].qPointer;

    // Check if the pageFrame is already in the buffer; if yes, update its position.
    for (int i = 0; i < currentQueueSize; i++) {
        if (pageFrames[i].PageNumber == pageNum) {
            queue[i].position = 1;

            pageFrames[i].fixCount++;
            for (int j = 0; j < currentQueueSize; j++) {
                if (j != i) {
                    queue[j].position++;
                }
            }

            // Update the page handler.
            page->data = pageFrames[i].content;
            page->pageNum = pageFrames[i].PageNumber;
            return;
        }
    }

    // If the buffer is not full, insert pages normally.
    if (currentQueueSize < bm->numPages) {
        // Insert the first page in the queue.
        if (currentQueueSize == 0) {
            queue[0].position = 1;
            queue[0].framePointer = &pageFrames[0];
            currentQueueSize++;
            return;
        } else {
            // Find the place in the queue which is free and insert the page.
            for (int i = 0; i < maxBufferSize; i++) {
                // Finding which pageFrame is null.
                if (queue[i].framePointer == NULL) {
                    queue[i].position = 1;
                    for (int j = 0; j < maxBufferSize; j++) {
                        if (j != i) {
                            queue[j].position += 1;
                        }
                    }
                    queue[i].framePointer = returnPagePointer(bm, page);
                    currentQueueSize++;
                    return;
                }
            }
        }
    }

    // If the queue is full, perform LRU replacement.
    else if (currentQueueSize == queueSize) {
        for (int i = 0; i < currentQueueSize; i++) {
            // The queue frame with the maximum position is the least recently used; replace it.
            if (queue[i].position == maxQueue(queue)) {
                // Check if no other client is accessing the page.
                if (pageFrames[i].fixCount == 0) {
                    queue[i].position = 1;
                    for (int j = 0; j < currentQueueSize; j++) {
                        if (j != i) {
                            queue[j].position++;
                        }
                    }

                    SM_FileHandle fh;

                    // If the page is dirty, write it back to the disk.
                    if (pageFrames[i].dirty == 1) {
                        openPageFile(bm->pageFile, &fh);
                        ensureCapacity(pageFrames[i].PageNumber, &fh);
                        writeBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);
                        closePageFile(&fh);
                        writeCount++;
                    }

                    // Pin the page and store the pagenumber.
                    pageFrames[i].pinStatus = 1;
                    pageFrames[i].PageNumber = pageNum;
                    pageFrames[i].freeStat = 1;
                    pageFrames[i].dirty = 0;
                    pageFrames[i].fixCount = 0;

                    // Read the data from the disk and store it in the page handler.
                    openPageFile(bm->pageFile, &fh);
                    ensureCapacity(pageFrames[i].PageNumber, &fh);
                    readBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);
                    closePageFile(&fh);

                    // Increment the read count.
                    readCount++;

                    // Update the page handler.
                    page->data = pageFrames[i].content;
                    page->pageNum = pageFrames[i].PageNumber;

                    return;
                }
            }
        }

        // If the page is in use by other clients, find the next largest page and check if we can replace it.
        for (int i = 0; i < currentQueueSize; i++) {
            int temp = maxQueue(queue) - 1;
            if (queue[i].position == temp && pageFrames[i].fixCount == 0) {
                queue[i].position = 1;
                for (int j = 0; j < currentQueueSize; j++) {
                    if (j != i) {
                        queue[j].position++;
                    }
                }

                SM_FileHandle fh;

                if (pageFrames[i].dirty == 1) {
                    openPageFile(bm->pageFile, &fh);
                    ensureCapacity(pageFrames[i].PageNumber, &fh);
                    writeBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);
                    closePageFile(&fh);
                    writeCount++;
                }

                // Pin the page and store the pagenumber.
                pageFrames[i].pinStatus = 1;
                pageFrames[i].PageNumber = pageNum;
                pageFrames[i].freeStat = 1;
                pageFrames[i].dirty = 0;
                pageFrames[i].fixCount = 0;

                // Read the data from the disk and store it in the page handler.
                openPageFile(bm->pageFile, &fh);
                ensureCapacity(pageFrames[i].PageNumber, &fh);
                readBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);
                closePageFile(&fh);

                // Increment the read count.
                readCount++;

                // Update the page handler.
                page->data = pageFrames[i].content;
                page->pageNum = pageFrames[i].PageNumber;

                return;
            }
        }
    }
}

//The FIFO Function is a void function which the maintains the queue and performs FIFO when the buffer size is full.
void FIFO(BM_BufferPool *const bm, BM_PageHandle *const page,int pageNum){
        //Get the PageFrame pointer and the Queue pointer.
        Frame *pageFrames = (Frame *)bm->mgmtData;
        Queue *queue = q;

        //if the buffer is not full insert pages normally.
        if(currentQueueSize < bm->numPages) {
                //insert the first page in the queue.
                if(currentQueueSize == 0) {

                        queue[0].position = 1;
                        queue[0].framePointer = &pageFrames[0];
                        queue[0].pNo = pageNum;
                        currentQueueSize++;

                        return;
                }
                else{
                        //find the place in the queue which is free and insert the page.
                        for(int i =0; i<maxBufferSize; i++) {
                                //finding which pageFrame is null.
                                if(queue[i].framePointer == NULL) {
                                        queue[i].position = 1;
                                        queue[i].pNo = pageNum;
                                        for(int j=0; j<maxBufferSize; j++) {
                                                if(j != i)
                                                        queue[j].position +=1;
                                        }
                                        queue[i].framePointer = returnPagePointer(bm,page);
                                        currentQueueSize++;
                                        return;
                                }
                        }
                }
        }
        //If the queue is full we need to do FIFO.
        else if(currentQueueSize == queueSize ) {


                for(int i =0; i<currentQueueSize; i++) {
                        //if the page already exists in buffer then just increment the fix count.
                        if(pageFrames[i].PageNumber == pageNum) {

                                pageFrames[i].fixCount += 1;

                                return;
                        }
                        //Remove the element with the maximum position value which will be first one which came in to the buffer
                        if(queue[i].position == currentQueueSize) {
                          printf("s");

                                if(pageFrames[i].fixCount == 0) {
                                        //printf("%d",pageFrames[i].PageNumber);
                                        //change its position to the first and insert the new frame.
                                        queue[i].position = 1;
                                        for(int j=0; j<currentQueueSize; j++) {
                                                if(j != i)
                                                        queue[j].position++;

                                        }

                                        SM_FileHandle fh;
                                        if(pageFrames[i].dirty ==1) {
                                                openPageFile(bm->pageFile, &fh);
                                                ensureCapacity(pageFrames[i].PageNumber,&fh);
                                                writeBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);

                                                closePageFile(&fh);
                                                writeCount++;
                                        }

                                        // pin the page and store the pagenumber.
                                        pageFrames[i].pinStatus = 1;
                                        pageFrames[i].PageNumber = pageNum;
                                      //  printf("%d\n",pageFrames[i].PageNumber);
                                        pageFrames[i].freeStat = 1;
                                        pageFrames[i].dirty = 0;
                                        pageFrames[i].fixCount = 0;

                                        //read the data from the disk and store it in the page handler.

                                        openPageFile(bm->pageFile,&fh);

                                        ensureCapacity(pageFrames[i].PageNumber, &fh);
                                        readBlock(pageFrames[i].PageNumber,&fh,pageFrames[i].content);
                                        closePageFile(&fh);
                                        //increment the read count
                                        readCount++;

                                        //update the page handler.
                                        page->data  = pageFrames[i].content;
                                        page->pageNum = pageFrames[i].PageNumber;


                                        return;
                                }
                        }
                }

                // if the page is in use by other client find the next largest page and check if we can replace that
                for(int i =0; i<currentQueueSize; i++) {
                        int temp = currentQueueSize-1;
                        if(queue[i].position == temp && pageFrames[i].fixCount == 0) {
                                queue[i].position = 1;
                                for(int j=0; j<currentQueueSize; j++) {
                                        if(j != i)
                                                queue[j].position++;

                                }

                                SM_FileHandle fh;
                                if(pageFrames[i].dirty ==1) {
                                        openPageFile(bm->pageFile, &fh);
                                        ensureCapacity(pageFrames[i].PageNumber,&fh);
                                        writeBlock(pageFrames[i].PageNumber, &fh, pageFrames[i].content);
                                        closePageFile(&fh);
                                        writeCount++;
                                }

                                // pin the page and store the pagenumber.
                                pageFrames[i].pinStatus = 1;
                                pageFrames[i].PageNumber = pageNum;
                                pageFrames[i].freeStat = 1;
                                pageFrames[i].dirty = 0;
                                pageFrames[i].fixCount = 0;

                                //read the data from the disk and store it in the page handler.
                                openPageFile(bm->pageFile,&fh);
                                ensureCapacity(pageFrames[i].PageNumber, &fh);
                                readBlock(pageFrames[i].PageNumber,&fh,pageFrames[i].content);

                                //closePageFile(&fh);
                                //increment the read count
                                readCount++;

                                //update the page handler.
                                page->data  = pageFrames[i].content;
                                page->pageNum = pageFrames[i].PageNumber;


                                return;
                        }
                }
        }
}

RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum) {
  int size = bm->numPages;
  Frame *pageFrames = (Frame *) bm->mgmtData;

  if (bufferSize == maxBufferSize && isBufferFull) {
    if (bm->strategy == RS_FIFO) {
      FIFO(bm, page, pageNum);
    } else {
      LRU(bm, page, pageNum);
    }
    return RC_OK;
  }

  if (isBufferFull == 0) {
    for (int i = 0; i < bufferSize; i++) {
      if (pageFrames[i].PageNumber == pageNum) {
        page->data = pageFrames[i].content;
        page->pageNum = pageNum;
        return RC_OK;
      }
    }

    if (bufferSize == 0) {
      if (bm->strategy == RS_FIFO) {
        FIFO(bm, page, pageNum);
      } else {
        LRU(bm, page, pageNum);
      }

      SM_FileHandle fh;

      pageFrames[0].pinStatus = 1;
      pageFrames[0].PageNumber = pageNum;
      pageFrames[0].freeStat = 1;
      pageFrames[0].fixCount++;

      openPageFile(bm->pageFile, &fh);
      ensureCapacity(pageNum, &fh);
      readBlock(pageNum, &fh, pageFrames[0].content);
      closePageFile(&fh);
      readCount++;

      page->data = pageFrames[0].content;
      page->pageNum = pageFrames[0].PageNumber;

      bufferSize++;
      return RC_OK;
    } else {
      for (int i = 1; i < size; i++) {
        if (pageFrames[i].freeStat == 0) {
          SM_FileHandle fh;

          if (bm->strategy == RS_FIFO) {
            FIFO(bm, page, pageNum);
          } else {
            LRU(bm, page, pageNum);
          }

          pageFrames[i].freeStat = 1;
          pageFrames[i].pinStatus = 1;
          pageFrames[i].PageNumber = pageNum;
          pageFrames[i].fixCount++;

          openPageFile(bm->pageFile, &fh);
          ensureCapacity(pageNum, &fh);
          readBlock(pageNum, &fh, pageFrames[i].content);
          closePageFile(&fh);
          readCount++;

          page->data = pageFrames[i].content;
          page->pageNum = pageFrames[i].PageNumber;

          bufferSize++;
          return RC_OK;
        }
      }
      return RC_OK;
    }
  }

  return RC_OK;
}


//unpinning the page.
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    Frame *pageFrame = (Frame *) bm->mgmtData;
    int i;

    for (i = 0; i < bufferSize; i++) {
        if (pageFrame[i].PageNumber == page->pageNum) {
            // Decrement the fix count since the client is unpinning the page.
            pageFrame[i].fixCount--;

            if (pageFrame[i].fixCount == 0) {
                // Set the pin status to 0 since the page is no longer pinned.
                pageFrame[i].pinStatus = 0;
            }

            return RC_OK;
        }
    }

    return RC_ERROR;
}


//marking the page as dirty.
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
  Frame *pageFrame = (Frame *) bm->mgmtData;
  int i;

  for (i = 0; i < bufferSize; i++) {
    if (pageFrame[i].PageNumber == page->pageNum) {
      // Mark the page as dirty.
      pageFrame[i].dirty = 1;

      return RC_OK;
    }
  }

  return RC_ERROR;
}

extern RC shutdownBufferPool(BM_BufferPool *const bm) {
  Frame *pageFrame = (Frame *) bm->mgmtData;
  Queue *queue = q;

  // Write all dirty pages to disk before shutting down.
  forceFlushPool(bm);

  // Check if any pages are still pinned.
  for (int i = 0; i < bufferSize; i++) {
    if (pageFrame[i].fixCount != 0) {
      printf("Page %d is still pinned, cannot shutdown buffer pool\n",
             pageFrame[i].PageNumber);
      return RC_ERROR;
    }
  }

  // Free the memory allocated to the content variable in the frame pointer.
  for (int i = 0; i < maxBufferSize; i++) {
    free(pageFrame[i].content);
  }

  // Free the memory allocated to the pageFrame and queue arrays.
  free(pageFrame);
  free(queue);

  // Reset all variables to their initial state.
  bufferSize = 0;
  isBufferFull = 0;
  maxBufferSize = 0;
  currentQueueSize = 0;
  bm->mgmtData = NULL;

  return RC_OK;
}



extern RC forceFlushPool(BM_BufferPool *const bm) {
  Frame *pageFrame = (Frame *) bm->mgmtData;

  // Write all dirty pages to disk.
  for (int i = 0; i < bufferSize; i++) {
    if (pageFrame[i].dirty == 1) {
      SM_FileHandle fh;
      openPageFile(bm->pageFile, &fh);
      ensureCapacity(pageFrame[i].PageNumber, &fh);
      writeBlock(pageFrame[i].PageNumber, &fh, pageFrame[i].content);
      closePageFile(&fh);

      pageFrame[i].dirty = 0;

      writeCount++;
    }
  }

  return RC_OK;
}

extern RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page) {
  Frame *pageFrame = (Frame *) bm->mgmtData;

  // Write the specified page to disk.
  for (int i = 0; i < bufferSize; i++) {
    if (pageFrame[i].PageNumber == page->pageNum) {
      SM_FileHandle fh;
      openPageFile(bm->pageFile, &fh);
      writeBlock(pageFrame[i].PageNumber, &fh, pageFrame[i].content);
      closePageFile(&fh);

      pageFrame[i].dirty = 0;

      writeCount++;

      return RC_OK;
    }
  }

  return RC_ERROR;
}


//returns the contents of the frame.
PageNumber *getFrameContents(BM_BufferPool *const bm) {
  PageNumber *pageNumbers;

  // Allocate memory for page numbers array
  pageNumbers = malloc(bufferSize * sizeof(PageNumber));
  if (pageNumbers == NULL) {
    // Handle memory allocation error
    return NULL;
  }

  Frame *pageFrame = (Frame *) bm->mgmtData;

  int i = 0;
  while (i < bufferSize) {
    pageNumbers[i] = (pageFrame[i].PageNumber != -1) ? pageFrame[i].PageNumber : NO_PAGE;
    i++;
  }

  return pageNumbers; // Return allocated memory
}

bool *getDirtyFlags(BM_BufferPool *const bm) {
  bool *dirtyBits;

  // Allocate memory for dirty bits array
  dirtyBits = malloc(bufferSize * sizeof(bool));
  if (dirtyBits == NULL) {
    // Handle memory allocation error
    return NULL;
  }

  Frame *pageFrame = (Frame *) bm->mgmtData;

  int i = 0;
  for (i = 0; i < bufferSize; i++) {
    dirtyBits[i] = (pageFrame[i].dirty == 1) ? true : false;
  }

  return dirtyBits; // Return allocated memory
}

int *getFixCounts(BM_BufferPool *const bm) {
  int *fixCounts;

  // Allocate memory for fix counts array
  fixCounts = malloc(bufferSize * sizeof(int));
  if (fixCounts == NULL) {
    // Handle memory allocation error
    return NULL;
  }

  Frame *pageFrame = (Frame *) bm->mgmtData;

  int i = 0;
  while (i < bufferSize) {
    fixCounts[i] = (pageFrame[i].fixCount != -1) ? pageFrame[i].fixCount : 0;
    i++;
  }

  return fixCounts; // Return allocated memory
}


//returns the number of reads done by the buffer manager.
extern int getNumReadIO (BM_BufferPool *const bm)
{
        return readCount;
}

//returns the number of writes done by the buffer manager,
extern int getNumWriteIO (BM_BufferPool *const bm)
{
        return writeCount;
}
