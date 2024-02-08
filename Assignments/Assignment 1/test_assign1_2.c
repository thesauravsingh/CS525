#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

char *testName;

#define TESTPF "test_pagefile.bin"

static void ExtraTestCast(void);

int
main (void)
{
  testName = "Extra Test Cases";
  
  initStorageManager();

  ExtraTestCast();

  return 0;
}
void ExtraTestCast(void){
     SM_FileHandle fh;
	  SM_PageHandle ph;
	  int i;

	  testName = "ExtraTestCast1";

	  ph = (SM_PageHandle) malloc(PAGE_SIZE);

	  // creating a new page
	  TEST_CHECK(createPageFile (TESTPF));
      printf("created file\n");
	  TEST_CHECK(openPageFile (TESTPF, &fh));
	  printf("opened file\n");

//Reading First Block should be empty, since it's a newly created block
TEST_CHECK(readFirstBlock (&fh, ph));
for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
    printf("first block was empty\n");

//Now writing on that first block
for (i=0; i < PAGE_SIZE; i++)
		ph[i] = (i % 10) + '0';
printf("Starting to write on first block i.e.0\n");
TEST_CHECK(writeBlock(0, &fh, ph));
printf("Writing on first block successful\n");


 //Now writing on that second block
printf("Starting to write on second block i.e=1\n");
TEST_CHECK(writeBlock (1, &fh, ph));
printf("Writing on second block successful\n");

printf("Testing if writing on second block was successful i.e=1\n");
TEST_CHECK(readBlock(1,&fh, ph));
for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");

// ASSERT_TRUE((fh.totalNumPages == 2), "Before Appending a new block total number of pages should be 2");

printf("Appending new block\n");
TEST_CHECK(appendEmptyBlock (&fh));
printf("Now checking if the new block is added\n");
ASSERT_TRUE((fh.totalNumPages == 2), "Total number of pages should be 3");
printf("Appending new block successful\n");


printf("Reading the new appended block i.e. 2\n");
TEST_CHECK(readBlock(1,&fh, ph));
for (i=0; i < PAGE_SIZE; i++)
    ASSERT_TRUE((ph[i] == 0), "expected zero byte in appened page");
printf("Testing reading in appended block was successful i.e=2\n");

printf("Starting to write on the appended block i.e=2\n");
TEST_CHECK(writeBlock (2, &fh, ph));
printf("Writing on appended block successful\n");



TEST_CHECK(ensureCapacity (3, &fh));
printf("%d\n",fh.totalNumPages);
ASSERT_TRUE((fh.totalNumPages == 3), "3 pages after ensure capacity");

TEST_CHECK(destroyPageFile (TESTPF));  
 
TEST_DONE();
}