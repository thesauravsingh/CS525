#include "btree_mgr.h"
#include "tables.h"
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdlib.h>
#include <string.h>

SM_FileHandle btree_fh;
int maxEle;

typedef struct BTREE
{
    int *key;
    struct BTREE **children; // Rename 'next' to 'children' for clarity
    RID *id;
} BTree;

BTree *root;
BTree *scan;
int indexNum = 0;

// init and shutdown index manager
RC initIndexManager(void *mgmtData)
{
    return RC_OK;
}

RC shutdownIndexManager()
{
    return RC_OK;
}

// create, destroy, open, and close a B-tree index
RC createBtree(char *idxId, DataType keyType, int n)
{
    int i = 0;

    // Allocate memory for the root node
    root = (BTree *)malloc(sizeof(BTree));
    root->key = malloc(sizeof(int) * (n - 1)); // Change to (n-1) for a B-tree
    root->id = malloc(sizeof(RID) * n);
    root->children = malloc(sizeof(BTree *) * n); // Change to (n) for a B-tree

    // Initialize child pointers to NULL using a do-while loop
    do {
        root->children[i] = NULL;
        ++i;
    } while (i < n);

    // Set the maximum number of elements
    maxEle = n;

    // Create a page file
    createPageFile(idxId);

    return RC_OK;
}


RC openBtree(BTreeHandle **tree, char *idxId)
{
    return (openPageFile(idxId, &btree_fh) == 0) ? RC_OK : RC_ERROR;
}
RC closeBtree(BTreeHandle *tree)
{
    return (closePageFile(&btree_fh) != 0) ? RC_ERROR : (free(root), RC_OK);
}

RC deleteBtree(char *idxId)
{
    return (destroyPageFile(idxId) == 0) ? RC_OK : RC_ERROR;
}



// access information about a b-tree
RC getNumNodes(BTreeHandle *tree, int *result)
{
    BTree *temp = (BTree*)malloc(sizeof(BTree));

    int numNodes = 0;
    int i = 0;

    do {
        numNodes++;
        i++;
    } while (i < maxEle + 2);

    *result = numNodes;

    free(temp);

    return RC_OK;
}


RC getNumEntries(BTreeHandle *tree, int *result)
{
    int totalEle = 0, i;
    BTree *temp = (BTree *)malloc(sizeof(BTree));

    temp = root;
    do {
        i = 0;
        while (i < maxEle) {
            if (temp->key[i] != 0) {
                totalEle++;
            }
            i++;
        }
        temp = temp->children[maxEle];
    } while (temp != NULL);

    *result = totalEle;
    free(temp);  // Assuming you want to free the allocated memory for temp
    return RC_OK;
}

//
RC getKeyType (BTreeHandle *tree, DataType *result)
{
    return RC_OK;
}


// index access
RC findKey(BTreeHandle *tree, Value *key, RID *result)
{
    BTree *temp = (BTree *)malloc(sizeof(BTree));
    int found = 0, i;

    temp = root;
    // Traverse the B-tree until the key is found or no more nodes to explore
    do {
        // Search for the key in the current node
        for (i = 0; i < maxEle; i++) {
            if (temp->key[i] == key->v.intV) {
                // If key is found, set the result and exit the loop
                result->page = temp->id[i].page;
                result->slot = temp->id[i].slot;
                found = 1;
                break;
            }
        }
        // Move to the next node
        temp = temp->children[maxEle];
        // If key is found, exit the loop
        if (found == 1)
            break;
    } while (temp != NULL);

    // Free dynamically allocated memory (if necessary)
    // free(temp); // Assuming you want to free the allocated memory for temp

    // Return appropriate status based on whether the key was found or not
    if (found == 1)
        return RC_OK;
    else
        return RC_IM_KEY_NOT_FOUND;
}



// Function to insert a key into a B-tree
RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{
    int i = 0;
    BTree *temp = (BTree *)malloc(sizeof(BTree));
    BTree *node = (BTree *)malloc(sizeof(BTree));
    node->key = malloc(sizeof(int) * maxEle);
    node->id = malloc(sizeof(RID) * maxEle);
    node->children = malloc(sizeof(BTree) * (maxEle + 1));

    // Initialize node's key array to 0
    for (i = 0; i < maxEle; i++) {
        node->key[i] = 0;
    }

    int nodeFull = 0;

    temp = root;
    // Traverse the B-tree until a suitable position is found
    while (temp != NULL) {
        nodeFull = 0;
        // Search for an empty slot in the current node
        for (i = 0; i < maxEle; i++) {
            if (temp->key[i] == 0) {
                // Insert the key and RID into the current node
                temp->id[i].page = rid.page;
                temp->id[i].slot = rid.slot;
                temp->key[i] = key->v.intV;
                temp->children[i] = NULL;
                nodeFull++;
                break;
            }
        }

        // If no empty slot found and current node is not a leaf node, move to the next child
        if ((nodeFull == 0) && (temp->children[maxEle] == NULL)) {
            node->children[maxEle] = NULL;
            temp->children[maxEle] = node;
        }

        temp = temp->children[maxEle];
    }

    int totalEle = 0;
    temp = root;
    // Count the total number of elements in the B-tree
    while (temp != NULL) {
        for (i = 0; i < maxEle; i++) {
            if (temp->key[i] != 0) {
                totalEle++;
            }
        }
        temp = temp->children[maxEle];
    }

    // If the total number of elements reaches the maximum capacity, perform a split
    if (totalEle == 6) {
        // Perform split by redistributing keys and children
        node->key[0] = root->children[maxEle]->key[0];
        node->key[1] = root->children[maxEle]->children[maxEle]->key[0];
        node->children[0] = root;
        node->children[1] = root->children[maxEle];
        node->children[2] = root->children[maxEle]->children[maxEle];
    }

    return RC_OK;
}



// Function to delete a key from a B-tree
RC deleteKey(BTreeHandle *tree, Value *key)
{
    BTree *temp = (BTree*)malloc(sizeof(BTree));
    int found = 0, i;
    
    // Traverse the B-tree until the key is found or no more nodes to explore
    while (temp != NULL) {
        // Iterate through keys in the current node
        for (i = 0; i < maxEle; i++) {
            // If key is found, mark it as deleted
            if (temp->key[i] == key->v.intV) {
                temp->key[i] = 0; // Mark key as deleted
                temp->id[i].page = 0; // Reset page ID
                temp->id[i].slot = 0; // Reset slot ID
                found = 1; // Set found flag
                break; // Exit the loop
            }
        }
        // If key is found, exit the outer loop
        if (found == 1)
            break;
        // Move to the next node
        temp = temp->children[maxEle];
    }
    
    // Free dynamically allocated memory (if necessary)
    // free(temp); // Uncomment if memory allocation was needed
    
    return RC_OK; // Return success
}



//
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{
    scan = (BTree *)malloc(sizeof(BTree));
    scan = root;
    indexNum = 0;

    BTree *temp = (BTree *)malloc(sizeof(BTree));
    int totalEle = 0, i;
    
    temp = root;
    do {
        i = 0;
        do {
            if (temp->key[i] != 0) {
                totalEle++;
            }
            i++;
        } while (i < maxEle);
        temp = temp->children[maxEle];
    } while (temp != NULL);

    int key[totalEle];
    int elements[maxEle][totalEle];
    int count = 0;

    temp = root;
    do {
        i = 0;
        do {
            key[count] = temp->key[i];
            elements[0][count] = temp->id[i].page;
            elements[1][count] = temp->id[i].slot;
            count++;
            i++;
        } while (i < maxEle);
        temp = temp->children[maxEle];
    } while (temp != NULL);

    int swap;
    int pg, st, c, d;
    c = 0;
    do {
        d = 0;
        do {
            if (key[d] > key[d + 1])
            {
                swap = key[d];
                pg = elements[0][d];
                st = elements[1][d];

                key[d] = key[d + 1];
                elements[0][d] = elements[0][d + 1];
                elements[1][d] = elements[1][d + 1];

                key[d + 1] = swap;
                elements[0][d + 1] = pg;
                elements[1][d + 1] = st;
            }
            d++;
        } while (d < count - c - 1);
        c++;
    } while (c < count - 1);

    count = 0;

    temp = root;
    do {
        i = 0;
        do {
            temp->key[i] = key[count];
            temp->id[i].page = elements[0][count];
            temp->id[i].slot = elements[1][count];
            count++;
            i++;
        } while (i < maxEle);
        temp = temp->children[maxEle];
    } while (temp != NULL);

    return RC_OK;
}

RC nextEntry (BT_ScanHandle *handle, RID *result)
{
    if(scan->children[maxEle] != NULL) {
        if(maxEle == indexNum) {
            indexNum = 0;
            scan = scan->children[maxEle];
        }

        (*result).page = scan->id[indexNum].page;
        (*result).slot = scan->id[indexNum].slot;
        indexNum ++;
    }
    else
        return RC_IM_NO_MORE_ENTRIES;
    
    return RC_OK;
}

RC closeTreeScan (BT_ScanHandle *handle)
{
    indexNum = 0;
    return RC_OK;
}


// debug and test functions
char *printTree (BTreeHandle *tree)
{
    return RC_OK;
}
