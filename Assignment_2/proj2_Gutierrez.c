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
	     int shamt; int funct; int imm; char* inst;};
// pipeline register structures
struct IfId{char *instruction; int pcPlus4;};
struct IdEx{char *instruction; int pcPlus4; int branchTarg;
	    int rData1; int rData2; int imm; int rs; int rt; int rd;};
struct ExMem{char *instruction; int aluRes; int wrDatReg; int wrReg;};
struct MemWb{char *instruction; int wrDatMem; int wrDatALU; int wrReg;};
// state structure
struct State{struct IfId ifid; struct IdEx idex; struct ExMem exmem; struct MemWb memwb;};

// FUNCTION DEFINITIONS -------------------------------------------------------------------
void translate(struct Instr*,const int*,int);		// translates machine language into bits I can store into the Instr struct
void printCycle(int,int*,int*,struct State,int);	// prints one the state of current cycle
void printMemReg(int*,int);	// helper to print things through printCycle
void printState(struct State);	// helper to print things through printCycle
char* findRegName(int);		// helper to find a given register number ***(this is an edited version of a function I used in my project one)

int main(){
	// used to reset state when necessary
	const struct State EmptyState = {.ifid.instruction="NOOP",.idex.instruction="NOOP",.exmem.instruction="NOOP",.memwb.instruction="NOOP"};
	static const struct Instr EmptyInstr;

	int pc = 0;			// holds current PC
	int dataMem[32];		// holds data memory value
	int regFile[32];		// holds register values
	int machInstr[100];
	int dataWords[32];
	struct Instr instructions[100];
		int i;
		for(i = 0; i < 100; i++)
			instructions[i] = EmptyInstr;
	struct State state = EmptyState;

	char currLine[13];		// 13 is the number of digits in the largest possible signed int (plus a negative symbol and a null character)
	// getting data and populating storage*****************************************************
	memset(dataMem,0,sizeof(dataMem));	// zero-ing out dataMem
	memset(regFile,0,sizeof(regFile));	// zero-ing out regFile
	memset(machInstr,0,sizeof(machInstr));	// zero-ing out machInstr
	// extracting instructions
	int track = 0;
	int line = 0;
	while(fgets(currLine, sizeof(currLine), stdin)){
		sscanf(currLine,"%d",&line);
		if (line == 1)
			break;
		machInstr[track++] = line;	// REMEMBER TO ONLY DO THIS ONCE PER LOOP CYCLE
	}
	const int numOfIn = track;		// holds the number of instructions
	
	// extracting words and populating data memory
	track = 0;
	fgets(currLine,sizeof(currLine),stdin);	// ignoring the empty line after the halt
	while(fgets(currLine,sizeof(currLine),stdin)){
		sscanf(currLine,"%d",&line);
		dataMem[track++] = line;
	}
	const int numOfDM = track;		// holds the number of words stored in data memory
	
	// populating instruction struct array with translated machine instructions
	translate(instructions,machInstr,numOfIn);
		
	// PRINTING DATA FOR ***TESTING*** PURPOSES
	/*for (i = 0; i < numOfIn; i++)
		printf("%d: %d\n",i+1,machInstr[i]);
	for (i = 0; i < numOfDM; i++)
		printf("%d: %d\n",numOfIn+i+1,dataMem[i]);
	for (i = 0; i < numOfIn; i++)
		printf("%c %d %d %d %d %d %d %d %s\n",instructions[i].type,instructions[i].op,instructions[i].rs,instructions[i].rt,instructions[i].rd,instructions[i].shamt,instructions[i].funct,instructions[i].imm,instructions[i].inst);
	*/
	
	// actual pipeline logic*******************************************************************
	for (i = 0; i < numOfIn; i++)
		printCycle(pc,dataMem,regFile,state,i+1);
	
	return 0;
}

void translate(struct Instr* inst,const int* machInstr, int size){
	int currLine = 0;
	int i;
	for (i = 0; i < size; i++){
		currLine = machInstr[i];
		// handling noops
		if (currLine == 0){
			inst[i].type = 'x';	// x will denote a type that is neither j nor i
			inst[i].inst = "NOOP";
		}
		// handling other instructions
		inst[i].op = currLine >> 26 & 63;
		inst[i].rs = (currLine >> 21) & 31;
		inst[i].rt = (currLine >> 16) & 31;
		inst[i].rd = (currLine >> 11) & 31;
		inst[i].shamt = (currLine >> 6) & 31;
		inst[i].funct = currLine & 63;
		inst[i].imm = currLine & 65535;
		// assigning instruction types
		if (inst[i].op == 12 || inst[i].op == 13 || inst[i].op == 5 || inst[i].op == 35 || inst[i].op == 43)
			inst[i].type = 'i';
		else if (inst[i].op == 0 && inst[i].inst != "NOOP")	// for noops
			inst[i].type = 'r';
		else
			inst[i].type = 'r';
		// finding mips representation of an instruction
		switch(inst[i].op)
		{
			case 0:
				if(inst[i].funct == 32)
					inst[i].inst = "add";					
				else if(inst[i].funct == 34)
					inst[i].inst = "sub";					
				else if(inst[i].funct == 0)
					inst[i].inst = "sll";
				break;
			case 35:
				inst[i].inst = "lw";
				break;
			case 43:
				inst[i].inst = "sw";
				break;
			case 12:
				inst[i].inst = "andi";
				break;
			case 13:
				inst[i].inst = "ori";
				break;
			case 5:
				inst[i].inst = "bne";
				break;
		}
	}
	
}

char* findRegName(int reg){
	static char* temp;
	char* numAsStr;
	if (reg == 0)
		temp = "$0";
	else if (reg == 1)
		temp = "$at";
	else if (reg == 2)
		temp = "$v0";
	else if (reg == 3)
		temp = "$v1";
	else if (reg > 3 && reg < 8){
		temp = "$a";
		sprintf(numAsStr,"%d",(reg-4));
		strcat(temp,numAsStr);
	}
	else if (reg > 7 && reg < 16){
		temp = "$t";
		sprintf(numAsStr,"%d",(reg-8));
		strcat(temp,numAsStr);
	}
	else if (reg > 15 && reg < 24){
		temp = "$s";
		sprintf(numAsStr,"%d",(reg-16));
		strcat(temp,numAsStr);
	}
	else if (reg == 24)
		temp = "$t8";
	else if (reg == 25)
		temp = "$t9";
	return temp;
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
// prints both Data Memory and Register File, depending on which one I send it
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
