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
typedef struct {int tag; int dirty;} Word;	// ADD INT LRU TO THIS STRUCT**************

int main(){
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
	int r = 0;
	for (r; r < numSets; r++)
		cache[r] = (Word *)malloc(setAssoc * sizeof(Word));
	
	// CACHING SECTION
	int i = 0;
	for (i; i < numRef; i++) {
		// prints references
		//printf("%c %d\n",refs[i].type,refs[i].addrs);
		
		// WRITE-THROUGH, NO WRITE ALLOCATE
		// get tag and index for current reference
		index = (refs[i].addrs >> numOffBits) & (0xFFFFFFFF >> (numTagBits + numOffBits));
		tag = refs[i].addrs >> (numIndBits + numOffBits);
		//printf("Index: %d	Tag: %d\n",index,tag);
		
		// check read or write
		if (refs[i].type == 'W') {
			// goto index and check every slot in set to see if its already inside
			// yes -> hit++, memref++, update cache (LRU = i)
			// no -> mis++, memref++
		}
		else if (refs[i].type == 'R') {
			
		}
		else
			printf("Error: One of the references was neither read nor write.");
	}
	
	printf("****************************************\nWrite-through with No Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n",numRef,wtHit,wtMiss,wtMemRef);
	printf("****************************************\nWrite-back with Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n\n",numRef,wbHit,wbMiss,wbMemRef);
	
	
	// USED TO PRINT CONTENTS OF 2D ARRAY ////////////////////////////////////////////////////////////
	printf("\n");											//
	int c = 0;											//
	for (r = 0; r < numSets; r++)									//
		for (c = 0; c < setAssoc; c++)								//
			printf("cache[%d][%d] = %d | %d\n",r,c,cache[r][c].tag,cache[r][c].dirty);	//
	printf("\n");											//
	// end cache print ///////////////////////////////////////////////////////////////////////////////
	
		
	// freeing all allocated data
	r = 0;
	for (r; r < numSets; r++)
		free(cache[r]);
	free(cache);
	
	return 0;
}
