GROUP #08

RUNNING THE SCRIPT
=======================================

A) Type "make" to execute all executable files.

B) Type "test_assign4" to run the file "test_assign4_1.c".

C) Type "test_expr" to compile files related to test expressions, including "test_expr.c".

----------B Tree Functions------------

A) initIndexManager
- Initializes B-tree structures by setting up necessary parameters and memory allocations for subsequent operations.
- Performs setup routines for the B-tree index manager, ensuring proper initialization for efficient operations.

B) shutdownIndexManager
- Releases allocated resources, performs cleanup tasks, and gracefully shuts down the B-tree index manager.
- Handles graceful termination of the B-tree index manager, ensuring proper closure and resource deallocation.

C) createBtree
- Allocates memory for B-tree elements and creates a page file to store B-tree data under the given identifier (idxId).
- Establishes the B-tree structure by allocating memory and initializing storage for its data within a specified page file.

D) openBtree
- Accesses and opens the specified page file associated with idxID to retrieve stored B-tree data.
- Performs file operations to open and access the stored B-tree data within the specified page file.

E) closeBtree
- Frees allocated memory and resources associated with the B-tree, performing necessary cleanup operations.
- Handles the deallocation of memory and resources used by the B-tree structure before closing its operations.

F) deleteBtree
- Removes and clears all records and entries within the B-tree, resetting it to an empty state.
- Clears and erases all data and structures associated with the specified B-tree, preparing it for deletion.

---------Access Functions------------

G) getNumNodes
- Retrieves the count of nodes currently present within the B-tree structure for reference or analysis.
- Provides the number of nodes existing in the B-tree structure, aiding in understanding its current state.

H) getNumEntries
- Calculates and returns the total count of entries stored within the B-tree, facilitating data analysis.
- Retrieves the total count of entries within the B-tree, providing insights into the stored data volume.

I) getKeyType
- Identifies and returns the data type (e.g., Integer, String) of keys stored within the B-tree structure.
- Fetches the data type used for keys in the B-tree, enabling proper handling and interpretation of stored data.

--------Index Access Functions----------

J) findKey
- Searches for and retrieves a specific key within the B-tree, facilitating data retrieval operations.
- Enables the search functionality to locate and fetch a specified key within the B-tree structure.

K) insertKey
- Adds a new key into the B-tree structure, maintaining the B-tree properties and ensuring proper insertion.
- Facilitates the addition of a new key into the B-tree while adhering to its structural integrity and properties.

L) deleteKey
- Removes the specified key and its associated entry from the B-tree, ensuring accurate deletion without affecting structure.
- Handles the removal of a specific key and its corresponding entry from the B-tree without disrupting its organization.

M) openTreeScan
- Initiates a scanning process to access all entries within the B-tree, enabling comprehensive data traversal.
- Sets up the necessary operations to commence a scan of the B-tree's entries for subsequent traversal.

N) nextEntry
- Continues the traversal process, reading and providing the next entry in the B-tree during ongoing scanning.
- Facilitates the step-by-step traversal by reading and fetching the subsequent entry in the B-tree structure.

O) closeTreeScan
- Finalizes the ongoing scanning/traversal process within the B-tree, ensuring proper completion and resource cleanup.
- Handles the conclusion of the scanning/traversal process, ensuring proper closure and cleanup of associated resources.