HEADERS = storage_mgr.h dberror.h test_helper.h
CC = gcc
CFLAGS = -I

default: test_1

test_1: test_case_1.o storage_mgr.o dberror.o
	$(CC) -o test_1 test_case_1.o storage_mgr.o dberror.o

test_case_1.o: test_assign1_1.c $(HEADERS)
	$(CC) -o test_case_1.o -c test_assign1_1.c -lm

storage_mgr.o: storage_mgr.c $(HEADERS)
		$(CC) -c storage_mgr.c -o storage_mgr.o

dberror.o: dberror.c dberror.h
		$(CC) -c dberror.c

clean:
	$(RM) test_1 *.o *~
run_test_1:
	./test_1

