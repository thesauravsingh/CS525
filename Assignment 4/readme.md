525 Assignment No.4 -FNU Saurav -Aakash Vasishta -Dresha Reddy Bommana -Azha Manzoor

Running the Script
---------------------------------------------------------------------------------------------
1. Navigate to Project Root (`Assignment`) using Terminal:
    cd path/to/Assignment 3
2. List Files to Confirm Directory:
    ls
    *Verify that you are in the correct directory.

3. Clean Old Compiled Files if any:
    make clean
    *Delete previously compiled `.o` files.

4. Compile Project Files:
    make
    *Compile all project files, including `test_assign4_1.c`.

5. Run `test_assign4_1.c` File:
     ./test_assign4
    *Execute the `test_assign4_1.c` file.

6. Compile Test Expression Files:
    make test_expr
    *Compile test expression-related files, including `test_expr.c`.

7. Run `test_expr.c` File:
    make run_expr
    *Execute the `test_expr.c` file.

1. B TREE FUNCTIONS
   ------------------------------------------------------------
   1. initIndexManager(void *managementData):
   
   Description: Initializes the B-tree index manager by performing any necessary setup routines, such as allocating memory or initializing data structures.
   
   Procedure:
      The provided code simply returns RC_OK, indicating successful initialization.

   2. shutdownIndexManager():
   
   Description: Shuts down the B-tree index manager by releasing allocated resources and performing cleanup tasks.
   
   Procedure:
      The function simply returns RC_OK, indicating successful shutdown.

   3. createBtree(char *indexId, DataType keyType, int n):
   
   Description: Creates a new B-tree index with the given identifier, key data type, and maximum number of elements per node.
   
   Procedure:
      Allocate memory for the root node of the B-tree, including memory for the keys, RIDs, and child pointers.
      Initialize the child pointers to NULL using a do-while loop.
      Set the maximum number of elements per node (maxEle) to the provided value.
      Create a new page file with the given identifier using the createPageFile function.
      Return RC_OK to indicate successful creation of the B-tree.
   
   4. openBtree(BTreeHandle **tree, char *indexId):
      
   Description: Opens an existing B-tree index with the given identifier.
   
   Procedure:
      Open the page file associated with the given identifier using the openPageFile function.
      If the page file is opened successfully, return RC_OK; otherwise, return RC_ERROR.
   
   5. closeBtree(BTreeHandle *tree):
      
   Description: Closes an open B-tree index.
   
   Procedure:
     Close the page file associated with the B-tree using the closePageFile function.
     If the page file is closed successfully, free the memory allocated for the root node and return RC_OK; otherwise, return RC_ERROR.
   
   6. deleteBtree(char *indexId):
      
   Description: Deletes an existing B-tree index with the given identifier.
   
   Procedure:
      Destroy the page file associated with the given identifier using the destroyPageFile function.
      If the page file is destroyed successfully, return RC_OK; otherwise, return RC_ERROR

3. ACCESS FUNCTIONS
   ------------------------------------------------------------
	1. getNumNodes(BTreeHandle *tree, int *result):
	
 	Description: Retrieves the number of nodes in the B-tree.

	Procedure:
	   Allocate memory for a temporary BTree node.
	   Initialize a numNodes counter and iterate through the B-tree.
	   Increments the counter until the maximum number of elements per node plus 2 is reached.
	   Store the numNodes value in the result parameter and free the temporary BTree node.
   
	2. getNumEntries(BTreeHandle *tree, int *result):
	
 	Description: Retrieves the total number of entries stored in the B-tree.

	Procedure:
	   Allocate memory for a temporary BTree node and initialize a totalElement counter.
	   Traverse the B-tree, counts the number of non-zero keys in each node and add them to the totalElement counter.
	   Store the totalElement value in the result parameter and free the temporary BTree node.
	   
	3. getKeyType(BTreeHandle *tree, DataType *result):
	
 	Description: Retrieves the data type of the keys stored in the B-tree.

	Procedure:
	   Retrieves the data type that the B-tree's keys are made of, making it possible to handle and interpret stored data correctly.
	   Returns the data type of the stored key.
   
5. INDEX ACCESS FUNCTIONS
   ---------------------------------------------------------------
	1. findKey(BTreeHandle *tree, Value *key, RID *result):
	
 	Description: Searches for and retrieves a specific key within the B-tree, facilitating data retrieval operations.

	Procedure:
	   Allocate memory for a temporary BTree node and initialize a found flag.
	   Set the temporary node to the root of the B-tree and Iterate through the keys in each node, to check if the provided key matches any of the keys in the node.
	   If found, store the corresponding RID in the parameter(result) and set the found flag to 1.
	   If the temporary node has no more children,it breaks out of the loop, else, move to the next child node.
	   If the key was found, return RC_OK; Free the temporary BTree node.
	   
	2. insertKey(BTreeHandle *tree, Value *key, RID rid):

    	Description: Adds a new key into the B-tree structure, maintaining the B-tree properties and ensuring proper insertion.
    
	   Procedure:
	   Allocate memory for temporary BTree nodes, including one for a new node.
	   Initialize the new node's keys to 0 and its child pointers to NULL.
	   Set the temporary node to the root of the B-tree.
	   Iterate through the keys in each node, finding the first empty slot to insert the new key and RID.
	   If a node is full and has no more children, create a new child node and link it to the current node.
	   If the total number of elements in the B-tree reaches a certain threshold i.e, 6 perform a split operation to maintain the B-tree properties.
	   Return RC_OK is insertion is succesful. Free the temporary BTree nodes.
	  
	4. deleteKey(BTreeHandle *tree, Value *key):

    	Description: Removes the specified key and its associated entry from the B-tree, ensuring accurate deletion without affecting structure.
    
	   Procedure:
	   Allocate memory for a temporary BTree node.
	   Set the temporary node to the root of the B-tree.
	   Iterate through the keys in each node, finding the specified key.
	   If the key is found, set its value to 0 and clear the associated RID (page and slot).
	   Set the found flag to 1 and break out of the loop.
	   If the key was found, return RC_OK; otherwise, return RC_OK.
	   Free the temporary BTree node.

  
	6. openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle):
	
 	   Description: Initiates a scanning process to access all entries within the B-tree, enabling comprehensive data traversal.
    
	   Procedure:
	    Allocate memory for a temporary BTree node and set it to the root of the B-tree.
	    Initialize a totalEle counter and traverse the B-tree, counting the number of non-zero keys in each node and adding them to the totalEle counter.
	    Allocate memory for arrays to store the sorted keys and their corresponding RIDs.
	    Traverse the B-tree again, populating the key and RID arrays in sorted order.
	    Update the keys and RIDs in the B-tree nodes to match the sorted order.
	    Set the scan pointer to the root of the B-tree and the indexNum to 0 to prepare for the scanning process.
	    Return RC_OK to indicate successful initialization.
    

	8. nextEntry(BT_ScanHandle *handle, RID *result):
		
        Description: Continues the traversal process, reading and providing the next entry in the B-tree during ongoing scanning.
    
	   Procedure:
	    Check if the scan pointer has a non-NULL child node.
	    If the indexNum is equal to the maximum number of elements per node, move the scan pointer to the next child node and reset the indexNum to 0.
	    Retrieve the page and slot values from the current entry in the scan node and store them in the result parameter.
	    Increment the indexNum to move to the next entry.
	    If there are no more entries to retrieve, return RC_IM_NO_MORE_ENTRIES else, return RC_OK.

	10. closeTreeScan(BT_ScanHandle *handle):

        Description: Finalizes the ongoing scanning/traversal process within the B-tree, ensuring proper completion and resource cleanup.

           Procedure:
	    Reset the indexNum to 0 to prepare for the next scan.
	    Return RC_OK to indicate successful completion of the tree scan.
	   

