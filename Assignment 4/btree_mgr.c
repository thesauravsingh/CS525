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
    // Allocate memory for temporary B-tree node
    BTree *temp = (BTree *)malloc(sizeof(BTree));
    int found = 0, i;

    // Initialize temp as the root node
    temp = root;
    // Loop through the B-tree nodes
    while (temp != NULL) {
        // Loop through the keys in the current node
        for (i = 0; i < maxEle; i++) {
            // If the key is found, update the result and set found to 1
            if (temp->key[i] == key->v.intV) {
                result->page = temp->id[i].page;
                result->slot = temp->id[i].slot;
                found = 1;
                break;
            }
        }
        // Move to the next child node
        temp = temp->children[maxEle];
        // If the key is found, break out of the loop
        if (found == 1)
            break;
    }

    // free(temp); // Assuming you want to free the allocated memory for temp

    // Return appropriate status based on whether the key is found or not
    if (found == 1)
        return RC_OK;
    else
        return RC_IM_KEY_NOT_FOUND;
}


// Function to insert a key into a B-tree
RC insertKey(BTreeHandle *tree, Value *key, RID rid)
{
    // Allocate memory for temporary B-tree nodes
    BTree *temp = (BTree *)malloc(sizeof(BTree));
    BTree *node = (BTree *)malloc(sizeof(BTree));
    // Allocate memory for key, ID, and children arrays in the new node
    node->key = malloc(sizeof(int) * maxEle);
    node->id = malloc(sizeof(RID) * maxEle);
    node->children = malloc(sizeof(BTree) * (maxEle + 1));

    // Initialize key array in the new node with zeros
    for (int i = 0; i < maxEle; i++) {
        node->key[i] = 0;
    }

    int nodeFull = 0;

    // Initialize temp as the root node
    temp = root;
    // Loop through the B-tree nodes
    while (temp != NULL) {
        nodeFull = 0;
        // Loop through the keys in the current node
        for (int i = 0; i < maxEle; i++) {
            // If the current key slot is empty, insert the key and ID
            if (temp->key[i] == 0) {
                temp->id[i].page = rid.page;
                temp->id[i].slot = rid.slot;
                temp->key[i] = key->v.intV;
                temp->children[i] = NULL;
                nodeFull++;
                break;
            }
        }

        // If the current node is full and doesn't have a child, link the new node
        if ((nodeFull == 0) && (temp->children[maxEle] == NULL)) {
            node->children[maxEle] = NULL;
            temp->children[maxEle] = node;
        }

        temp = temp->children[maxEle];
    }

    int totalEle = 0;
    // Count the total number of elements in the B-tree
    temp = root;
    while (temp != NULL) {
        for (int i = 0; i < maxEle; i++) {
            if (temp->key[i] != 0) {
                totalEle++;
            }
        }
        temp = temp->children[maxEle];
    }

    // If the B-tree reaches a certain number of elements, perform a split operation
    if (totalEle == 6) {
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
    // Allocate memory for temporary B-tree node
    BTree *temp = (BTree*)malloc(sizeof(BTree));
    // Initialize variables
    int found = 0, i = 0; // Initialize i here
    temp = root; // Initialize temp outside the loop
    
    // Loop through the B-tree nodes
    while (temp != NULL && !found) { // Change for loop to while loop
        // Loop through the keys in the current node
        while (i < maxEle) { // Change for loop to while loop
            // If the key is found, remove it
            if (temp->key[i] == key->v.intV) {
                temp->key[i] = 0;
                temp->id[i].page = 0;
                temp->id[i].slot = 0;
                found = 1;
                break;
            }
            i++; // Increment i here
        }
        temp = temp->children[maxEle];
        i = 0; // Reset i here
    }
    
    // Free the allocated memory
    free(temp);
    // Return success status
    return RC_OK;
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
