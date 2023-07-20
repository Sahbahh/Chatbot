#ifndef _CHAIN_H_
#define _CHAIN_H_
#include <stdbool.h>

#define CHAIN_SUCCESS 0
#define CHAIN_FAIL -1

// Full types defined in .c file
typedef struct Segment_s Segment;
struct Segment_s;

typedef struct Chain_s Chain;
struct Chain_s;


// Maximum number of unique chains the system can support
#define CHAIN_MAX_NUM_HEADS 10

// Maximum total number of segments (statically allocated) to be shared across all chains
#define CHAIN_MAX_NUM_SEGMENTS 100

// Makes a new, empty chain, and returns its reference on success. 
// Returns a NULL pointer on failure.
Chain* Chain_create();

// Returns the number of items in pChain.
int Chain_count(Chain* pChain);

// Functions for traversing the chain
void* Chain_first(Chain* pChain);
void* Chain_last(Chain* pChain); 
void* Chain_next(Chain* pChain);
void* Chain_prev(Chain* pChain);
void* Chain_curr(Chain* pChain);

// Functions for adding items to the chain
int Chain_add(Chain* pChain, void* pItem);
int Chain_insert(Chain* pChain, void* pItem);
int Chain_append(Chain* pChain, void* pItem);
int Chain_prepend(Chain* pChain, void* pItem);

// Function for removing item from the chain
void* Chain_remove(Chain* pChain);

// Function for joining two chains
void Chain_join(Chain* pChain1, Chain* pChain2);

// Function for freeing a chain
typedef void (*DESTRUCTOR_FN)(void* pItem);
void Chain_free(Chain* pChain, DESTRUCTOR_FN pDestructorFn);

// Function for trimming a chain
void* Chain_trim(Chain* pChain);

// Function for searching through a chain
typedef bool (*MATCHER_FN)(void* pItem, void* pComparisonArg);
void* Chain_search(Chain* pChain, MATCHER_FN pMatcher, void* pComparisonArg);

#endif
