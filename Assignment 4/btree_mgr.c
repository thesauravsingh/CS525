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

// Index Manager
// Initialization and Shutdown
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
    // Allocate memory for the root node
    root = malloc(sizeof(BTree));
    root->key = malloc(sizeof(int) * (n - 1));
    root->id = malloc(sizeof(RID) * n);
    root->children = malloc(sizeof(BTree *) * n);

    // Initialize child pointers to NULL
    for (int i = 0; i < n; i++) {
        root->children[i] = NULL;
    }

    // Set the maximum number of elements
    maxEle = n;

    // Create a page file
    createPageFile(idxId);

    return RC_OK;
}

RC openBtree(BTreeHandle **tree, char *idxId)
{
    return (openPageFile(idxId, &btree_fh) == 0)? RC_OK : RC_ERROR;
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



RC deleteKey(BTreeHandle *tree, Value *key)
{
    // Allocate memory for a temporary BTree node
    BTree *temp = (BTree*)malloc(sizeof(BTree));
    // Variable to track if the key is found
    int found = 0;
    // Loop through the tree nodes
    for (temp = root; temp != NULL; temp = temp->children[maxEle]) {
        // Initialize loop variable
        int i = 0;
        // Loop through the keys in the current node
        while (i < maxEle && !found) {
            // If the key is found, mark it as deleted
            if (temp->key[i] == key->v.intV) {
                temp->key[i] = 0;
                temp->id[i].page = 0;
                temp->id[i].slot = 0;
                // Set found flag to true
                found = 1;
                // Exit the loop
                break;
            }
            // Move to the next key
            i++;
        }
        // If the key is found, exit the outer loop
        if (found == 1)
            break;
    }
    // Return success code
    return RC_OK;
}



//Opens a scan on the B-tree for sequential access.
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle)
{
    // Allocate memory for the scan handle
    scan = (BTree *)malloc(sizeof(BTree));
    // Set the scan handle to the root of the tree
    scan = root;
    // Initialize index number
    indexNum = 0;

    // Allocate memory for temporary BTree node
    BTree *temp = (BTree *)malloc(sizeof(BTree));
    int totalEle = 0, i;
    
    // Set the temporary node to the root of the tree
    temp = root;
    // Count total elements in the tree
    while (temp != NULL) {
        i = 0;
        while (i < maxEle) {
            if (temp->key[i] != 0) {
                totalEle++;
            }
            i++;
        }
        // Move to the next child node
        temp = temp->children[maxEle];
    }

    // Declare arrays based on the total number of elements
    int key[totalEle];
    int elements[maxEle][totalEle];
    int count = 0;

    // Set the temporary node to the root of the tree
    temp = root;
    // Populate key and elements arrays with node values
    while (temp != NULL) {
        i = 0;
        while (i < maxEle) {
            key[count] = temp->key[i];
            elements[0][count] = temp->id[i].page;
            elements[1][count] = temp->id[i].slot;
            count++;
            i++;
        }
        // Move to the next child node
        temp = temp->children[maxEle];
    }

    int swap;
    int pg, st, c, d;
    c = 0;
    // Sort the key array and corresponding elements arrays
    while (c < count - 1) {
        d = 0;
        while (d < count - c - 1) {
            if (key[d] > key[d + 1]) {
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
        }
        c++;
    }

    count = 0;

    // Set the temporary node to the root of the tree
    temp = root;
    // Update the tree nodes with sorted values
    while (temp != NULL) {
        i = 0;
        while (i < maxEle) {
            temp->key[i] = key[count];
            temp->id[i].page = elements[0][count];
            temp->id[i].slot = elements[1][count];
            count++;
            i++;
        }
        // Move to the next child node
        temp = temp->children[maxEle];
    }

    return RC_OK;
}

//Retrieves the next entry (key-RID pair) during a scan on the B-tree.
RC nextEntry(BT_ScanHandle *handle, RID *result)
{
    // Check if scan->children[maxEle] is not NULL
    switch (scan->children[maxEle] != NULL) {
        case 1: // If not NULL
            // Check if maxEle is equal to indexNum
            switch (maxEle == indexNum) {
                case 1: // If equal
                    // Reset indexNum to 0
                    indexNum = 0;
                    // Move to the next level in the tree
                    scan = scan->children[maxEle];
                    break;
                case 0: // If not equal
                    break; // Do nothing
            }

            // Set result page and slot to current indexNum values
            (*result).page = scan->id[indexNum].page;
            (*result).slot = scan->id[indexNum].slot;
            // Increment indexNum
            indexNum++;
            break;
        case 0: // If NULL
            // Return no more entries error
            return RC_IM_NO_MORE_ENTRIES;
    }
    
    // Return OK
    return RC_OK;
}

//Closes an active scan on the B-tree
RC closeTreeScan (BT_ScanHandle *handle)
{
    indexNum = 0;
    return RC_OK;
}


// Debugging function to print the contents of the B-tree.
char *printTree (BTreeHandle *tree)
{
    return RC_OK;
}
