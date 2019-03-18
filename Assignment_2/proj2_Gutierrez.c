//-------PROJ2_GUTIERREZ.C-------
//
// Ryan Gutierrez
// CDA3101
// Pipeline Simulator

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STRUCTURE DEFINITIONS -----------------------------------------------------------------
struct Instr{char type; int op; int rs; int rt; int rd;
	     int shamt; int funct; int imm; int addr;};
// pipeline register structures
struct IfId{char *instruction; int pcPlus4;};
struct IdEx{char *instruction; int pcPlus4; int branchTarg;
	    int rData1; int rData2; int imm; int rs; int rt; int rd;};
struct ExMem{char *instruction; int aluRes; int wrDatReg; int wrReg;};
struct MemWb{char *instruction; int wrDatMem; int wrDatALU; int wrReg;};
// state structure
struct State{struct IfId ifid; struct IdEx idex; struct ExMem exmem; struct MemWb memwb;};

// FUNCTION DEFINITIONS -------------------------------------------------------------------
struct Instr extract(int*);
void printCycle(int,int*,int*,struct State,int);
void printMemReg(int*,int);
void printState(struct State);

int main(){
	// used to reset state when necessary
	const struct State EmptyState = {.ifid.instruction="NOOP",.idex.instruction="NOOP",.exmem.instruction="NOOP",.memwb.instruction="NOOP"};

	int pc = 0;			// holds current PC
	int dataMem[32];		// holds data memory value
	int regFile[32];		// holds register values
	int machInstr[100];
	int dataWords[32];
	struct Instr instructions[100];
	struct State state = EmptyState;

	// pipeline state declaration
	

	char currLine[11];		// eleven is the number of digits in the largest possible signed int (plus a negative symbol)
	// getting data and populating storage*****************************************************
	memset(dataMem,0,sizeof(dataMem));	// zero-ing out dataMem
	memset(regFile,0,sizeof(regFile));	// zero-ing out regFile
	int i = 1;
	while(fgets(currLine, sizeof(currLine), stdin)){
		printCycle(pc,dataMem,regFile,state,i++);
	}
	
	/*int i = 0;
	for (i = 0; i < instructions.size(); i++)
		
	}*/
	
	return 0;
}

struct Instr extract(int *machInstr){
	
}

void printCycle(int pc, int *dataMem, int *regFile, struct State state, int cycle){
	printf("********************\n");
	printf("State at the beginning of cycle %d\n", cycle);
	printf("\tPC = %d\n", pc);
	printf("\tData Memory:\n");
	printMemReg(dataMem,0);
	printf("\tRegisters:\n");
	printMemReg(regFile,1);
	printState(state);
}

void printMemReg(int *memReg, int which){
	char *name;
	if (which == 0)
		name = "dataMem";
	else if (which == 1)
		name = "regFile";
	
	char col1[25] = "";
	char *begin = "[";
	char *end = "] = ";
	char temp[11];
	int i;
	for(i = 0; i < 16; i++){
		sprintf(temp,"%d",i);
		strcat(col1,name); strcat(col1,begin); strcat(col1,temp); strcat(col1,end);
		sprintf(temp,"%d",memReg[i]);
		strcat(col1,temp);
		printf("\t\t%-25s%s[%d] = %d\n",col1,name,i+16,memReg[i+16]);
		memset(col1,0,sizeof(col1));
	}
}

void printState(struct State state){
	// IF/ID
	printf("\tIF/ID:\n\t\tInstruction: %s\n\t\tPCPlus4: %d\n",state.ifid.instruction,state.ifid.pcPlus4);
	// ID/EX
	printf("\tID/EX:\n\t\tInstruction: %s\n\t\tPCPlus4: %d\n",state.idex.instruction,state.idex.pcPlus4);
	printf("\t\tbranchTarget: %d\n\t\treadData1: %d\n\t\treadData2: %d\n",state.idex.branchTarg,state.idex.rData1,state.idex.rData2);
	printf("\t\timmed: %d\n\t\trs: %d\n\t\trt: %d\n\t\trd: %d\n",state.idex.imm,state.idex.rs,state.idex.rt,state.idex.rd);
	// EX/MEM
	printf("\tEX/MEM:\n\t\tInstruction: %s\n\t\taluResult: %d\n",state.exmem.instruction,state.exmem.aluRes);
	printf("\t\twriteDataReg: %d\n\t\twriteReg: %d\n",state.exmem.wrDatReg,state.exmem.wrReg);
	// MEM/WB
	printf("\tMEM/WB:\n\t\tInstruction: %s\n\t\twriteDataMem: %d\n",state.memwb.instruction,state.memwb.wrDatMem);
	printf("\t\twriteDataALU: %d\n\t\twriteReg: %d\n",state.memwb.wrDatALU,state.memwb.wrReg);
}
