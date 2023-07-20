#include "list.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

// Node structure for list
typedef struct Node_t Node;
struct Node_t {
    void* data;
    Node* next;
    Node* previous;
};

// List structure for linked list
typedef struct List_t List;
struct List_t {
    Node* first;
    Node* last;
    Node* current;
    int numOfItems;
    List* nextFree;
    int lastOutOfBoundsError;
};

// Static array for list heads and nodes
static List listHeadsArray[LIST_MAX_NUM_HEADS];
static Node listNodesArray[LIST_MAX_NUM_NODES];
static List *firstFreeListHead;
static Node *firstFreeListNode;
static bool listInitialized = false;

// Error codes for out of bounds conditions
#define LIST_START_OUT_OF_BOUNDS -1
#define LIST_END_OUT_OF_BOUNDS -2

// Mutex for thread-safety
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;

/* Function Declarations */
static void initializeList();
static bool checkOutOfBoundsStart(List* list);
static bool checkOutOfBoundsEnd(List* list);

// Mutex related functions
static void initializeMutex() 
{   
    pthread_mutexattr_t MutexAttr;
    pthread_mutexattr_init(&MutexAttr);
    pthread_mutexattr_settype(&MutexAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&listMutex, &MutexAttr);
    pthread_mutexattr_destroy(&MutexAttr);
}
static void lockListMutex()
{
    pthread_mutex_lock(&listMutex);
}
static void unlockListMutex()
{
    pthread_mutex_unlock(&listMutex);
}

// Function to create a new list
List* createNewList()
{
    if (!listInitialized) {
        initializeList();
    }

    lockListMutex();

    List* newList = firstFreeListHead;
    if (newList) {
        firstFreeListHead = firstFreeListHead->nextFree;
        newList->nextFree = NULL;
    }

    unlockListMutex();
    return newList;
}

// Function to get the size of the list
int getListSize(List* list)
{
    lockListMutex();
    int size = list->numOfItems;
    unlockListMutex();
    return size;
}

// Function to get the first element in the list
void* getFirstElement(List* list)
{
    lockListMutex();
    list->current = list->first;
    void *data = getCurrentData(list);
    unlockListMutex();
    return data;
}

// Function to get the last element in the list
void* getLastElement(List* list)
{
    lockListMutex();
    list->current = list->last;
    void *data = getCurrentData(list);
    unlockListMutex();
    return data;
}

// Function to get the next element in the list
void* getNextElement(List* list)
{
    lockListMutex();
    if (checkOutOfBoundsStart(list)) {
        list->current = list->first;
    } else if(!checkOutOfBoundsEnd(list)) {
        list->current = list->current->next;
    }

    if (list->current == NULL) {
        list->lastOutOfBoundsError = LIST_END_OUT_OF_BOUNDS;
    }

    void* data = getCurrentData(list);
    unlockListMutex();
    return data;
}

// Function to get the previous element in the list
void* getPrevElement(List* list)
{
    lockListMutex();
    if (checkOutOfBoundsEnd(list)) {
        list->current = list->last;
    } else if(!checkOutOfBoundsStart(list)){
        list->current = list->current->previous;
    }

    if (list->current == NULL) {
        list->lastOutOfBoundsError = LIST_START_OUT_OF_BOUNDS;
    }
    
    void* data = getCurrentData(list);
    unlockListMutex();
    return data;
}

// Current Item in the List
void* ListItem(List* givenList)
{
    lockMutex();
    void* item = NULL;
    if (givenList->currentNode != NULL) {
        item = givenList->currentNode->item;
    }
    unlockMutex();
    return item;
}

// Function to check if no nodes are available
static bool isNodesEmpty()
{
    return freeNode == NULL;
}

// Function to create a new node and assign an item to it
static Node* createNode(void* item) 
{
    assert(!isNodesEmpty());
    Node* newNode = freeNode;
    newNode->item = item;
    freeNode = freeNode->next;
    newNode->next = NULL;
    return newNode;
}

// Function to link a new node at the start of the list
static void addNodeAtStart(List* givenList, Node* newNode)
{
    newNode->prev = NULL;
    newNode->next = givenList->firstNode;
    if (givenList->count >= 1) {
        givenList->firstNode->prev = newNode;
    } else {
        givenList->lastNode = newNode;
    }
    givenList->firstNode = newNode;
    givenList->currentNode = newNode;
    givenList->count++;
}

// Function to link a new node at the end of the list
static void addNodeAtEnd(List* givenList, Node* newNode)
{
    newNode->prev = givenList->lastNode;
    newNode->next = NULL;
    if (givenList->count >= 1) {
        givenList->lastNode->next = newNode;
    } else {
        givenList->firstNode = newNode;
    }
    givenList->lastNode = newNode;
    givenList->currentNode = newNode;
    givenList->count++;
}

// Function to add a node after the current node in the list
static void addNodeAfterCurrent(List* givenList, Node* newNode)
{
    bool isInsertAtEnd = givenList->currentNode == givenList->lastNode
        || isOutOfBoundsEnd(givenList);
    if (isInsertAtEnd) {
        addNodeAtEnd(givenList, newNode);
    } else if (isOutOfBoundsStart(givenList)) {
        addNodeAtStart(givenList, newNode);
    } else {
        assert(givenList->currentNode != NULL);
        newNode->prev = givenList->currentNode;
        newNode->next = givenList->currentNode->next;
        givenList->currentNode->next = newNode;
        newNode->next->prev = newNode;
        givenList->currentNode = newNode;
        givenList->count++;
    }
}

// Function to add an item after the current item
int ListAdd(List* givenList, void* item)
{
    lockMutex();
    if (isNodesEmpty()) {
        unlockMutex();
        return LIST_FAIL;
    }
    Node* newNode = createNode(item);
    addNodeAfterCurrent(givenList, newNode);
    unlockMutex();
    return LIST_SUCCESS;
}

// Function to add an item before the current item
int ListInsert(List* givenList, void* item)
{
    lockMutex();
    if (isNodesEmpty()) {
        unlockMutex();
        return LIST_FAIL;
    }
    Node* newNode = createNode(item);
    ListPrev(givenList);
    addNodeAfterCurrent(givenList, newNode);
    unlockMutex();
    return LIST_SUCCESS;
}

// Function to add an item at the end of the list
int ListEndAdd(List* givenList, void* item)
{
    lockMutex();
    if (isNodesEmpty()) {
        unlockMutex();
        return LIST_FAIL;
    }
    Node* newNode = createNode(item);
    givenList->currentNode = givenList->lastNode;
    addNodeAtEnd(givenList, newNode);
    unlockMutex();
    return LIST_SUCCESS;
}

// Function to add an item at the start of the list
int ListStartAdd(List* givenList, void* item)
{
    lockMutex();
    if (isNodesEmpty()) {
        unlockMutex();
        return LIST_FAIL;
    }
    Node* newNode = createNode(item);
    addNodeAtStart(givenList, newNode);
    unlockMutex();
    return LIST_SUCCESS;
}

// Function to remove the current item from the list
void* ListRemove(List* givenList)
{
    lockMutex();
    if (givenList->count == 0
        || isOutOfBoundsStart(givenList)
        || isOutOfBoundsEnd(givenList)
    ) {
        unlockMutex();
        return NULL;
    }
    Node* removeNode = givenList->currentNode;
    void* item = removeNode->item;
    Node* prevNode = removeNode->prev;
    Node* nextNode = removeNode->next;

    if (prevNode != NULL) {
        prevNode->next = nextNode;
    } else {
        givenList->firstNode = nextNode;
    }
    if (nextNode != NULL) {
        nextNode->prev = prevNode;
    } else {
        givenList->lastNode = prevNode;
    }
    givenList->count --;
    removeNode->next = freeNode;
    freeNode = removeNode;
    givenList->currentNode = nextNode;
    if (givenList->currentNode == NULL) {
        givenList->lastOutOfBoundsReason = LIST_OOB_END;
    }
    unlockMutex();
    return item;
}

// Function for getting the last element from the list and removing it
void* extractLast(List* listHandle)
{
    // Lock the list to avoid conflicts
    accessMutex();

    // Set the pointer to the last node and remove it
    moveToLast(listHandle);
    void* lastItem = removeCurrent(listHandle);
    moveToLast(listHandle);
    
    // Unlock the list after modification
    releaseMutex();
    return lastItem;
}

// Function for merging two lists
void mergeLists(List* primaryList, List* secondaryList)
{
    // Lock the list to avoid conflicts
    accessMutex();

    // Reconnecting the nodes from the secondary list to the primary list
    Node* lastPrimaryNode = primaryList->lastNode;
    Node* firstSecondaryNode = secondaryList->firstNode;
    if (firstSecondaryNode == NULL) {
        // If secondary list is empty, then nothing to do
    } else if (lastPrimaryNode == NULL) {
        primaryList->firstNode = firstSecondaryNode;
        primaryList->lastNode = secondaryList->lastNode;
    } else {
        lastPrimaryNode->next = firstSecondaryNode;
        firstSecondaryNode->prev = lastPrimaryNode;
        primaryList->lastNode = secondaryList->lastNode;        
    }
    primaryList->nodeCount += secondaryList->nodeCount;

    secondaryList->nodeCount = 0;
    secondaryList->currentNode = NULL;
    secondaryList->firstNode = NULL;
    secondaryList->lastNode = NULL;

    // Deallocate the secondary list, which is now empty.
    deallocList(secondaryList, NULL);

    // Unlock the list after modification
    releaseMutex();
}

// Function for deallocating the list and its nodes
void deallocList(List* listHandle, DEALLOC_FN deallocator)
{
    // Lock the list to avoid conflicts
    accessMutex();
    // Deallocate all nodes
    while (nodeCount(listHandle) > 0) {
        Node* node = extractLast(listHandle);

        // Invoke deallocator function to clean up the memory
        if (deallocator != NULL) {
            (*deallocator)(node);
        }
    }

    // Deallocate list
    listHandle->nextFreeList = firstFreeList;
    firstFreeList = listHandle;
    releaseMutex();
}

// Function to search through the list from the current item until the end, 
// returns a pointer to the item if a match is found, NULL otherwise.
void* searchList(List* listHandle, COMPARE_FN comparator, void* comparisonArg)
{
    // Lock the list to avoid conflicts
    accessMutex();
    if (atListStart(listHandle)) {
        moveToFirst(listHandle);
    }

    while(listHandle->currentNode != NULL) {
        // Checking for a match
        void* item = listHandle->currentNode->item;
        if ( (*comparator)(item, comparisonArg) == 1) {
            return item;
        }

        moveToNext(listHandle);
    }
    // Unlock the list after searching
    releaseMutex();
    return NULL;
}



//-----PRIVATE FUNCTIONS-------

// Function for initializing data structures
static void setupDataStructures() {
    initMutex();
    
    accessMutex();

    assert(MAX_LIST_NODES > 0);
    assert(MAX_LIST_HEADS > 0);


    // Setting up nodes
    firstFreeNode = &nodeArray[0];
    for (int i = 0; i < MAX_LIST_NODES; i++) {
        nodeArray[i].item = NULL;
        nodeArray[i].prev = NULL;
        nodeArray[i].next = NULL;
        if (i + 1 < MAX_LIST_NODES) {
            nodeArray[i].next = &nodeArray[i+1];
        }
    }

    // Setting up list heads
    firstFreeList = &listArray[0];
    for (int i = 0; i < MAX_LIST_HEADS; i++) {
        listArray[i].nodeCount = 0;
        listArray[i].currentNode = NULL;
        listArray[i].outOfBoundsReason = LIST_START_OOB;
        listArray[i].firstNode = NULL;
        listArray[i].lastNode = NULL;
        listArray[i].nextFreeList = NULL;
        if (i + 1 < MAX_LIST_HEADS) {
            listArray[i].nextFreeList = &listArray[i + 1];
        }
    }
    initialized = true;
    releaseMutex();
}

// Function for checking if the current position is at the start of the list
static bool atListStart(List* listHandle) {
    assert(listHandle->outOfBoundsReason == LIST_START_OOB 
        || listHandle->outOfBoundsReason == LIST_END_OOB);
    return listHandle->currentNode == NULL 
        && listHandle->outOfBoundsReason == LIST_START_OOB;
}
// Function for checking if the current position is at the end of the list
static bool atListEnd(List* listHandle) {
    assert(listHandle->outOfBoundsReason == LIST_START_OOB 
        || listHandle->outOfBoundsReason == LIST_END_OOB);

    return listHandle->currentNode == NULL 
        && listHandle->outOfBoundsReason == LIST_END_OOB;
}
