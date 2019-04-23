//-------PROJ3_GUTIERREZ.C-------
//
// Ryan Gutierrez
// CDA3101
// Cache Simulator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// reference struct that will be used to hold references throught an array
struct Reference {char rw; int addrs;};

int main(){
	// don't know if I'll need this, adding just in case
	//static const struct Reference EmptyRef = {.rw="x"};	// where "x" means neither read nor write
	
	int blockSize;	// block size
	int numSets;	// number of sets
	int setAssoc;	// set associativity
	int numOffBits;
	int numIndBits;
	int numTagBits;
	int numRef;	// number of references
	struct Reference refs[100];	// array of 100 possible references
	char currLine[13];
	
	// FOR TESTING PURPOSES 
	int zero = 0;
	
	// grabbing the first three lines as needed
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&blockSize);
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&numSets);
	fgets(currLine, sizeof(currLine), stdin); sscanf(currLine,"%d",&setAssoc);
	
	numOffBits = log(blockSize)/log(2);
	numIndBits = log(numSets)/log(2);
	numTagBits = 32 - numIndBits - numOffBits;
	
	printf("\nBlock size: %d\nNumber of sets: %d\nAssociativity: %d\n",blockSize,numSets,setAssoc);
	printf("Number of offset bits: %d\nNumber of index bits: %d\nNumber of tag bits: %d\n",numOffBits,numIndBits,numTagBits);
	
	numRef = 0;
	while (fgets(currLine, sizeof(currLine), stdin))
		sscanf(currLine,"%c%d",&refs[numRef++].rw,&refs[numRef].addrs);
	
	// TESTING
	/*int i = 0;
	for (i; i < numRef; i++)
		printf("%c %d\n",refs[i].rw,refs[i].addrs);*/
	
	
	
	printf("****************************************\nWrite-through with No Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n",zero,zero,zero,zero);
	printf("****************************************\nWrite-back with Write Allocate\n****************************************\n");
	printf("Total number of references: %d\nHits: %d\nMisses: %d\nMemory References: %d\n\n",zero,zero,zero,zero);
	
	return 0;
}
