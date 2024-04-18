#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dberror.h"
#include "tables.h"

// Structure for accessing B-trees
typedef struct BTreeHandle {
  DataType keyType;
  char *indexId; // Changed from idxId
  void *managementData; // Changed from mgmtData
} BTreeHandle;

typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *managementData; // Changed from mgmtData
} BT_ScanHandle;

// Init and shutdown index manager
extern RC initIndexManager(void *managementData); // Changed from mgmtData
extern RC shutdownIndexManager();

// Create, destroy, open, and close a B-tree index
extern RC createBtree(char *indexId, DataType keyType, int n);
extern RC openBtree(BTreeHandle **tree, char *indexId); // Changed from idxId
extern RC closeBtree(BTreeHandle *tree);
extern RC deleteBtree(char *indexId); // Changed from idxId

// Access information about a B-tree
extern RC getNumNodes(BTreeHandle *tree, int *result);
extern RC getNumEntries(BTreeHandle *tree, int *result);
extern RC getKeyType(BTreeHandle *tree, DataType *result);

// Index access
extern RC findKey(BTreeHandle *tree, Value *key, RID *result);
extern RC insertKey(BTreeHandle *tree, Value *key, RID rid);
extern RC deleteKey(BTreeHandle *tree, Value *key);
extern RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle);
extern RC nextEntry(BT_ScanHandle *handle, RID *result);
extern RC closeTreeScan(BT_ScanHandle *handle);

// Debug and test functions
extern char *printTree(BTreeHandle *tree);

#endif // BTREE_MGR_H

