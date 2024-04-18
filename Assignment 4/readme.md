GROUP #39

Anjali Asnani (A20521347)
Janki Thakar (A20516458)
Krishna Panchal (A20547471)
Shreya Padaganur (A20551549)

RUNNING THE SCRIPT
=======================================


1) Type "make " to run all excuatble files 

3) Type "test_assign4" "test_assign4_1.c" file.

2) Type "test_expr" to compile test expression related files including "test_expr.c".


----------B Tree Functions------------

1) initIndexManager
- nitializes B-tree structures, setting up necessary parameters and memory allocations for subsequent operations.
- Performs setup routines for the B-tree index manager, ensuring proper initialization for efficient operations.

2) shutdownIndexManager
- Releases allocated resources, performs cleanup tasks, and gracefully shuts down the B-tree index manager.
-   Handles graceful termination of the B-tree index manager, ensuring proper closure and resource deallocation.
3) createBtree
  -   Allocates memory for B-tree elements, creating a page file to store B-tree data under the given identifier (idxId).
-   Establishes the B-tree structure by allocating memory and initializing storage for its data within a specified page file.

4) openBtree
-   Accesses and opens the specified page file associated with idxID to retrieve stored B-tree data.
-   Performs file operations to open and access the stored B-tree data within the specified page file.

5) closeBtree
 -   Frees allocated memory and resources associated with the B-tree, performing necessary cleanup operations.
-   Handles the deallocation of memory and resources used by the B-tree structure before closing its operations.

6) deleteBtree
  - Removes and clears all records and entries within the B-tree, resetting it to an empty state.
-   Clears and erases all data and structures associated with the specified B-tree, preparing it for deletion.

---------Access Functions------------

7) getNumNodes
  -  Retrieves the count of nodes currently present within the B-tree structure for reference or analysis.
-   Provides the number of nodes existing in the B-tree structure, aiding in understanding its current state.

8) getNumEntries
  -   Calculates and returns the total count of entries stored within the B-tree, facilitating data analysis.
-   Retrieves the total count of entries within the B-tree, providing insights into the stored data volume.

9) getKeyType
 -   Identifies and returns the data type (e.g., Integer, String) of keys stored within the B-tree structure.
-   Fetches the data type used for keys in the B-tree, enabling proper handling and interpretation of stored data.

--------Index Access Functions----------

10) findKey
-   Searches for and retrieves a specific key within the B-tree, facilitating data retrieval operations.
-   Enables the search functionality to locate and fetch a specified key within the B-tree structure.
11) insertKey
-   Adds a new key into the B-tree structure, maintaining the B-tree properties and ensuring proper insertion.
-   Facilitates the addition of a new key into the B-tree while adhering to its structural integrity and properties.
12) deleteKey
  -   Removes the specified key and its associated entry from the B-tree, ensuring accurate deletion without affecting structure.
-   Handles the removal of a specific key and its corresponding entry from the B-tree without disrupting its organization.

13) openTreeScan
   -   Initiates a scanning process to access all entries within the B-tree, enabling comprehensive data traversal.
-   Sets up the necessary operations to commence a scan of the B-tree's entries for subsequent traversal.

14) nextEntry
   -   Continues the traversal process, reading and providing the next entry in the B-tree during ongoing scanning.
-   Facilitates the step-by-step traversal by reading and fetching the subsequent entry in the B-tree structure.
15) closeTreeScan
  -   Finalizes the ongoing scanning/traversal process within the B-tree, ensuring proper completion and resource cleanup.
-   Handles the conclusion of the scanning/traversal process, ensuring proper closure and cleanup of associated resources.