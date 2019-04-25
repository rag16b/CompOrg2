//-------PROJ3_GUTIERREZ.C-------
//
// Ryan Gutierrez
// CDA3101
// Cache Simulator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// reference struct that will be used to hold references throughoutt an array
typedef struct {char type; int addrs;} Reference;
typedef struct {int tag; int dirty; int trackLRU;} Word;

int main(){
	const Word EmptyWord = {.tag = -1};
	
	int blockSize;	// block size
	int numSets;	// number of sets
	int setAssoc;	// set associativity
	int numOffBits, numIndBits, numTagBits;	// seperations of the addresses
	int numRef;	// number of references
	Reference refs[100];	// array of 100 possible references
	char currLine[13];	// temporary string for parsing
	int wtHit=0, wtMiss=0, wtMemRef=0;	// storage for write through, no write allocate
	int wbHit=0, wbMiss=0, wbMemRef=0;	// storage for wtite back, write allocate
	int index, tag;	// index and tag for current word
	
	// grabbing the first three lines as needed
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&blockSize);
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&numSets);
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&setAssoc);
	// various calculations
	numOffBits = log(blockSize)/log(2);
	numIndBits = log(numSets)/log(2);
	numTagBits = 32 - numIndBits - numOffBits;
	
	printf("\nBlock size: %d\nNumber of sets: %d\nAssociativity: %d\n",blockSize,numSets,setAssoc);
	printf("Number of offset bits: %d\nNumber of index bits: %d\nNumber of tag bits: %d\n",numOffBits,numIndBits,numTagBits);

	// parsing the rest of the data
	numRef = 0;
	while (fgets(currLine, sizeof(currLine), stdin))
		sscanf(currLine,"%c%d",&refs[numRef++].type,&refs[numRef].addrs);
	
	// dynamically allocating a 2d array of Word structs
	Word **cache = (Word **)malloc(numSets * sizeof(Word*));
	int row;
	for (row = 0; row < numSets; row++)
		cache[row] = (Word *)malloc(setAssoc * sizeof(Word));
	
	// initializing the cache with all -1 in the tag so I can find completely empty sets
	int col;
	for (row = 0; row < numSets; row++)
		for (col = 0; col < setAssoc; col++)
			cache[row][col] = EmptyWord;

	// CACHING SECTION
	// WRITE-THROUGH, NO WRITE ALLOCATE
	int i;
	for (i = 0; i < numRef; i++) {
		// prints references
		//printf("%c %d\n",refs[i].type,refs[i].addrs);
		
		// get tag and index for current reference
		index = (refs[i].addrs >> numOffBits) & (0xFFFFFFFF >> (numTagBits + numOffBits));
		tag = refs[i].addrs >> (numIndBits + numOffBits);
		
		// prints index and tag
		//printf("Index: %d	Tag: %d\n",index,tag);
		
		// check read or write
		if (refs[i].type == 'W') {
			int currRef;
			int didHit = 0;
			// goto index and check every slot in set to see if its already inside
			for (currRef = 0; currRef < setAssoc; currRef++) {
				// tag and index are in cache
				if (cache[index][currRef].tag == tag) {
					wtHit++;
					wtMemRef++;
					cache[index][currRef].trackLRU = i;	// update cache
					didHit = 1;
					break;
				}
			}
			// tag and index are not in cache
			if (didHit == 0) {
				wtMiss++;
				wtMemRef++;
			}
		}
		else if (refs[i].type == 'R') {
			int currRef;
			int didHit = 0;
			// goto index and check every slot in set to see if its already inside
			for (currRef = 0; currRef < setAssoc; currRef++) {
				// tag and index are in cache
				if (cache[index][currRef].tag == tag) {
					wtHit++;
					didHit = 1;
					break;
				}
			}
			// tag and index are not in cache
			if (didHit == 0) {
				int isRoom = 0;
				wtMiss++;
				// is there room in cache?
				for (currRef = 0; currRef < setAssoc; currRef++) {
					// if there is room in the cache
					if (cache[index][currRef].tag == -1) {
						wtMemRef++;
						// write in
						cache[index][currRef].tag = tag;
						cache[index][currRef].trackLRU = i;
						isRoom = 1;
						break;
					}
				}
				// if there is NOT room in the cache
				if (isRoom == 0) {
					wtMemRef++;
					// finding the LRU
					int LRUref = 0;
					for (currRef = 0; currRef < setAssoc; currRef++)
						if (cache[index][LRUref].trackLRU > cache[index][currRef].trackLRU)
							LRUref = currRef;
					// replacing the LRU
					cache[index][LRUref].tag = tag;
					cache[index][LRUref].trackLRU = i;
				}
			}	
		}
		else
			printf("Error: One of the references was neither read nor written.");
	}
	
	printf("****************************************\nWrite-through with No Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n",numRef,wtHit,wtMiss,wtMemRef);
	printf("****************************************\nWrite-back with Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n\n",numRef,wbHit,wbMiss,wbMemRef);
	
	
	// USED TO PRINT CONTENTS OF 2D ARRAY ////////////////////////////////////////////////////////////////////////////////////////////////////
	/*printf("\n");																//
	for (row = 0; row < numSets; row++)													//
		for (col = 0; col < setAssoc; col++)												//
			printf("cache[%d][%d] = %d | %d | %d\n",row,col,cache[row][col].tag,cache[row][col].dirty,cache[row][col].trackLRU);	//
	printf("\n");*/																//
	// end cache print ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
		
	// freeing all allocated data
	for (row = 0; row < numSets; row++)
		free(cache[row]);
	free(cache);
	
	return 0;
}
