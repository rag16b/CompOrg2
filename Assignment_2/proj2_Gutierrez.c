//-------PROJ2_GUTIERREZ.C-------
//
// Ryan Gutierrez
// CDA3101
// Pipeline Simulator

// Potential problems:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STRUCTURE DEFINITIONS -----------------------------------------------------------------
struct Instr{char type; int op; int rs; int rt; int rd;
	     int shamt; int funct; int imm; char inst[100]; char targReg[3];};
// pipeline register structures
struct IfId{char *instruction; int pcPlus4;};
struct IdEx{char *instruction; int pcPlus4; int branchTarg;
	    int rData1; int rData2; int imm; char rs[4]; char rt[4]; char rd[4];};
struct ExMem{char *instruction; int aluRes; int wrDatReg; char wrReg[4];};
struct MemWb{char *instruction; int wrDatMem; int wrDatALU; char wrReg[4];};
// state structure
struct State{struct IfId ifid; struct IdEx idex; struct ExMem exmem; struct MemWb memwb;};

// FUNCTION DEFINITIONS -------------------------------------------------------------------
void translate(struct Instr*,const int*,int);		// translates machine language into bits I can store into the Instr struct
void printCycle(int,int*,int*,struct State,int);	// prints one the state of current cycle
void printMemReg(int*,int);	// helper to print things through printCycle
void printState(struct State);	// helper to print things through printCycle
char* findRegName(int);		// helper to find a given register number ***(this is an edited version of a function I used in my project one)
char* findWrReg(struct Instr);	// helper to find the write registers for EX/MEM and MEM/WB
int aluRes(struct Instr,int*);	// helper to find the ALU result for any given instruction
void regFileUpdate(struct Instr,int*,int*,int);		// helper to actually execute an instruction and change the registers and what not
void memDataUpdate(struct Instr,int*,int*,int);		// helper to deal with updates to data memory (really only affects teh sw instruction)
void stall(struct Instr*,struct Instr,int,int);		// injects a stall at the given location in the instruction array
int isHazard(struct Instr*,int);	// helper that returns 1 if true and 0 if false

int main(){
	// used to reset state when necessary
	const struct State EmptyState = {.ifid.instruction="NOOP",.idex.instruction="NOOP",.exmem.instruction="NOOP",.memwb.instruction="NOOP"};
	static const struct Instr EmptyInstr = {.inst="NOOP"};
	const struct IfId EmptyIfid={.instruction="NOOP"};
	const struct IdEx EmptyIdex={.instruction="NOOP",.rs="0",.rt="0",.rd="0"};
	const struct ExMem EmptyExmem={.instruction="NOOP",.wrReg="0"};
	const struct MemWb EmptyMemwb={.instruction="NOOP",.wrReg="0"};
	
	int pc = 0;			// holds current PC
	int halt = 0;			// 0 if false 1 if true
	int dataMem[32];		// holds data memory value
	int regFile[32];		// holds register values
	int machInstr[100];
	int dataWords[32];
	struct Instr instructions[500];	// 500 becuase stalls (I know it's less but I'm not doing the math)
		int i;
		for(i = 0; i < 100; i++)
			instructions[i] = EmptyInstr;
	struct State prevState = EmptyState;
	struct State newState = EmptyState;

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
		if (line == 1){
			machInstr[track++] = line;
			break;
		}
		machInstr[track++] = line;
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
	/*for (i = 0; i < numOfDM; i++)
		printf("%d: %d\n",numOfIn+i+1,dataMem[i]);
	for (i = 0; i < numOfIn; i++)
		printf("%c %d %d %d %d %d %d %d %s\n",instructions[i].type,instructions[i].op,instructions[i].rs,instructions[i].rt,instructions[i].rd,instructions[i].shamt,instructions[i].funct,instructions[i].imm,instructions[i].inst);
	*/
	
	// actual pipeline logic*******************************************************************
	//regFile[8] = 5;	// MANIPULATING REGISTERS FOR TESTING
	//regFile[9] = 6;
	i = 0;
	int cycle;
	int numStalls;
	while (halt == 0){
		printCycle(pc,dataMem,regFile,newState,++i);
		cycle = pc/4;

		// IF/ID
		newState.ifid = EmptyIfid;
		if (cycle < numOfIn+numStalls)
			newState.ifid.instruction = instructions[cycle].inst;
		newState.ifid.pcPlus4 = pc + 4;		// trackin pcPlus4, will have to change when dealing with hazards
		// ID/EX
		newState.idex = EmptyIdex;
		if ((cycle-1) >= 0 && (cycle-1) < numOfIn+numStalls){
			// DEALING WITH STALLS
			if (strncmp(instructions[cycle-1].inst,"lw",2) == 0 && isHazard(instructions,cycle) == 1){
				printf("Got Here\n");
				stall(instructions,EmptyInstr,cycle-1,numOfIn+(numStalls++));
			}// end stall handling
			newState.idex.instruction = instructions[cycle-1].inst;
			newState.idex.rData1 = regFile[instructions[cycle-1].rs];
			newState.idex.rData2 = regFile[instructions[cycle-1].rt];
			newState.idex.imm = instructions[cycle-1].imm;
			if (strcmp(instructions[cycle-1].inst,"halt") != 0 && strcmp(instructions[cycle-1].inst,"NOOP") != 0){
				strcpy(newState.idex.rs,findRegName(instructions[cycle-1].rs));
				strcpy(newState.idex.rt,findRegName(instructions[cycle-1].rt));
				// to deal make sure rd reads "0" or "$0" when it should
				if (strncmp(instructions[i-1].inst,"add",3) == 0 || strncmp(instructions[cycle-1].inst,"sub",3) == 0)
					strcpy(newState.idex.rd,findRegName(instructions[cycle-1].rd));
			}
		}
		newState.idex.pcPlus4 = pc;
		newState.idex.branchTarg = ((instructions[cycle-1].imm*4) + newState.idex.pcPlus4);
		// EX/MEM
		newState.exmem = EmptyExmem;
		if (((cycle-2) >= 0 && (cycle-2) < numOfIn+numStalls) && strcmp(instructions[cycle-2].inst,"NOOP") != 0){
			newState.exmem.instruction = instructions[cycle-2].inst;
			newState.exmem.aluRes = aluRes(instructions[cycle-2],regFile);
			//newState.exmem.wrDatReg = ;
			strcpy(newState.exmem.wrReg,findWrReg(instructions[cycle-2]));
		}
		// MEM/WB
		newState.memwb = EmptyMemwb;
		if (((cycle-3) >= 0 && (cycle-3) < numOfIn+numStalls) && strcmp(instructions[cycle-3].inst,"NOOP") != 0){
			newState.memwb.instruction = instructions[cycle-3].inst;
			//newState.memwb.wrDatMem = ;
			newState.memwb.wrDatALU = aluRes(instructions[cycle-3],regFile);
			strcpy(newState.memwb.wrReg,findWrReg(instructions[cycle-3]));
			memDataUpdate(instructions[cycle-4],regFile,dataMem,numOfIn);	// updates the data memory
			
			if (strcmp(instructions[cycle-3].inst,"halt") == 0)
				halt = 1;
		}
		// updates the register file
		if (cycle-4 >= 0 && (cycle-4) < numOfIn+numStalls)
			regFileUpdate(instructions[cycle-4],regFile,dataMem,numOfIn);

		if (halt == 1)		// print the last cycle
			printCycle(pc,dataMem,regFile,newState,++i);
		prevState = newState;
		pc += 4;

		if (cycle > 17)
			break;
	}
	
	return 0;
}// end main

int isHazard(struct Instr* inst, int cycle){	// returns 1 if true and 0 if false
	
	return 0;
}

void stall(struct Instr* inst, struct Instr empty, int pos, int numOfIn){
	int i;
	for (i = numOfIn-1; i >= pos-1; i--)
		inst[i+1] = inst[i];
	inst[pos-1] = empty;
}

void memDataUpdate(struct Instr inst, int* regFile, int* dataMem, int numOfIn){
	if (strncmp(inst.inst,"sw",2) == 0)
		dataMem[(aluRes(inst,regFile)-numOfIn*4)/4] = regFile[inst.rt];
}

void regFileUpdate(struct Instr inst, int* regFile, int* dataMem, int numOfIn){
	if (strncmp(inst.inst,"add",3) == 0 || strncmp(inst.inst,"sub",3) == 0 || strncmp(inst.inst,"sll",3) == 0)
		regFile[inst.rd] = aluRes(inst,regFile);
	if (strncmp(inst.inst,"andi",4) == 0 || strncmp(inst.inst,"ori",3) == 0)
		regFile[inst.rt] = aluRes(inst,regFile);
	if (strncmp(inst.inst,"lw",2) == 0)
		regFile[inst.rt] = dataMem[(aluRes(inst,regFile)-numOfIn*4)/4];
}

int aluRes(struct Instr inst, int* regFile){
	int result;		// **********NEED TO GET regFile ELEMENT AT EVERY GIVEN REGISTER
	if (strncmp(inst.inst,"add",3) == 0)
		result = regFile[inst.rs] + regFile[inst.rt];
	else if(strncmp(inst.inst,"sub",3) == 0)
		result = regFile[inst.rs] - regFile[inst.rt];
	else if(strncmp(inst.inst,"lw",2) == 0 || strncmp(inst.inst,"sw",2) == 0)
		result = regFile[inst.rs] + inst.imm;		// MAY NEED TO SIGN EXTEND
	else if(strncmp(inst.inst,"sll",3) == 0)
		result = regFile[inst.rt] << inst.shamt;
	else if(strncmp(inst.inst,"andi",4) == 0)
		result = regFile[inst.rs] & inst.imm;		// MAY NEED TO ZERO EXTEND LEFT
	else if(strncmp(inst.inst,"ori",3) == 0)
		result = regFile[inst.rs] | inst.imm;		// MAY NEED TO ZERO EXTEND LEFT
	else if(strcmp(inst.inst,"halt") == 0)
		result = regFile[0];
	//else if(strncmp(inst.inst,"bne",3) == 0)	// I THINK THIS IS JUST BRANCH TARG (IF YES DO IN MAIN)
	return result;
}

void translate(struct Instr* inst,const int* machInstr, int size){
	int currLine = 0;
	int i;
	char rs[4];
	char rt[4];
	char rd[4];
	for (i = 0; i < size; i++){
		currLine = machInstr[i];
		// handling noops
		if (currLine == 0)
			sprintf(inst[i].inst,"NOOP");
		else if (currLine == 1)
			sprintf(inst[i].inst,"halt");
		// handling other instructions
		inst[i].op = currLine >> 26 & 63;
		inst[i].rs = (currLine >> 21) & 31;
		inst[i].rt = (currLine >> 16) & 31;
		inst[i].rd = (currLine >> 11) & 31;
		inst[i].shamt = (currLine >> 6) & 31;
		inst[i].funct = currLine & 63;
		inst[i].imm = currLine & 65535;
		// turning each register into a string, clearing the strings each time
		memset(rs,0,sizeof(rs));
		memset(rt,0,sizeof(rt));
		memset(rd,0,sizeof(rd));
		strcat(rs,findRegName(inst[i].rs));
		strcat(rt,findRegName(inst[i].rt));
		strcat(rd,findRegName(inst[i].rd));
		// finding mips representation of an instruction
		switch(inst[i].op)
		{
			case 0:
				if(inst[i].funct == 32)
					sprintf(inst[i].inst,"add %s,%s,%s",rd,rs,rt);
				else if(inst[i].funct == 34)
					sprintf(inst[i].inst,"sub %s,%s,%s",rd,rs,rt);
				else if(inst[i].funct == 0)
					sprintf(inst[i].inst,"sll %s,%s,%d",rd,rt,inst[i].shamt);
				sprintf(inst[i].targReg,"%s","rd");
				break;
			case 35:
				sprintf(inst[i].inst,"lw %s,%d(%s)",rt,inst[i].imm,rs);
				sprintf(inst[i].targReg,"%s","rt");
				break;
			case 43:
				sprintf(inst[i].inst,"sw %s,%d(%s)",rt,inst[i].imm,rs);
				sprintf(inst[i].targReg,"%s","0");	// bc it has no target registers
				break;
			case 12:
				sprintf(inst[i].inst,"andi %s,%s,%d",rt,rs,inst[i].imm);
				sprintf(inst[i].targReg,"%s","rt");
				break;
			case 13:
				sprintf(inst[i].inst,"ori %s,%s,%d",rt,rs,inst[i].imm);
				sprintf(inst[i].targReg,"%s","rt");
				break;
			case 5:
				sprintf(inst[i].inst,"bne %s,%s,%s",rs,rt,inst[i].imm);
				sprintf(inst[i].targReg,"%s","0");	// bc it has no target registers
				break;
		}
		// assigning instruction types
		if (inst[i].op == 12 || inst[i].op == 13 || inst[i].op == 5 || inst[i].op == 35 || inst[i].op == 43)
			inst[i].type = 'i';		// string compares below are to handle noops and halts
		else if (strcmp(inst[i].inst,"NOOP") == 0 || strcmp(inst[i].inst,"halt") == 0)
			inst[i].type = 'x';		// x will denote a type that is neither j nor i
		else
			inst[i].type = 'r';
	}	
}// end translate

char* findRegName(int reg){
	char* temp = NULL;
	char numAsStr[2];
	static char stor[5];
	if (reg == 0)
		temp = "$0";
	else if (reg == 1)
		temp = "$at";
	else if (reg == 2)
		temp = "$v0";
	else if (reg == 3)
		temp = "$v1";
	else if (reg > 3 && reg < 8){
		strcpy(stor,"$a");
		sprintf(numAsStr,"%d",(reg-4));
		strcat(stor,numAsStr);
		temp = stor;
	}
	else if (reg > 7 && reg < 16){
		strcpy(stor,"$t");
		sprintf(numAsStr,"%d",(reg-8));
		strcat(stor,numAsStr);
		temp = stor;
	}
	else if (reg > 15 && reg < 24){
		strcpy(stor,"$s");
		sprintf(numAsStr,"%d",(reg-16));
		strcat(stor,numAsStr);
		temp = stor;
	}
	else if (reg == 24)
		temp = "$t8";
	else if (reg == 25)
		temp = "$t9";

	return temp;
}// end findRegName

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
	printf("\t\timmed: %d\n\t\trs: %s\n\t\trt: %s\n\t\trd: %s\n",state.idex.imm,state.idex.rs,state.idex.rt,state.idex.rd);
	// EX/MEM
	printf("\tEX/MEM:\n\t\tInstruction: %s\n\t\taluResult: %d\n",state.exmem.instruction,state.exmem.aluRes);
	printf("\t\twriteDataReg: %d\n\t\twriteReg: %s\n",state.exmem.wrDatReg,state.exmem.wrReg);
	// MEM/WB
	printf("\tMEM/WB:\n\t\tInstruction: %s\n\t\twriteDataMem: %d\n",state.memwb.instruction,state.memwb.wrDatMem);
	printf("\t\twriteDataALU: %d\n\t\twriteReg: %s\n",state.memwb.wrDatALU,state.memwb.wrReg);
}

char* findWrReg(struct Instr inst){
	char* temp;
	if (strncmp(inst.inst,"bne",3) == 0 || strcmp(inst.inst,"halt") == 0)
		temp = "0";
	else if (strncmp(inst.inst,"add",3) == 0 || strncmp(inst.inst,"sub",3) == 0)
		temp = findRegName(inst.rd);
	else
		temp = findRegName(inst.rt);
	return temp;
}
