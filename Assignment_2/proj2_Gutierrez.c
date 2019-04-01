//-------PROJ2_GUTIERREZ.C-------
//
// Ryan Gutierrez
// CDA3101
// Pipeline Simulator

// Potential problems:	-writeDataReg not calculated correctly
//			-Branch target is not calculated correctly for add, sll, or sub
//			-aluRes for sub is not calculated correctly (seems like Forwarding.exe got it wrong too)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// STRUCTURE DEFINITIONS -----------------------------------------------------------------
struct Instr{char type; int op; int rs; int rt; int rd; int shamt; int funct; 
	     int imm; char inst[100]; int aluRes; int srcReg; int destReg; int destCont;};
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
int aluRes(struct Instr,struct Instr,struct Instr,int*,int*,int,int);	// helper to find the ALU result for any given instruction
void regFileUpdate(struct Instr,int*,int*,int);		// helper to actually execute an instruction and change the registers and what not
void memDataUpdate(struct Instr,int*,int*,int);		// helper to deal with updates to data memory (really only affects teh sw instruction)
void stall(struct Instr*,struct Instr,int,int);		// injects a stall at the given location in the instruction array
int needStall(struct Instr,struct Instr);		// helper that returns 1 if true and 0 if false
int isHazard(struct Instr,struct Instr,struct Instr,int);	// helper to determine if there is a hazard (returns -1 if false, 0-9 otherwise)

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
	struct Instr instructions[500];	// 500 to account for stalls (I know it's less but I'm not doing the math)
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
	i = 0;
	int cycle = 0;
	int numStalls = 0;
	int numBranches = 0;
	int numMisBranches = 0;
	while (halt == 0){
		printCycle(pc,dataMem,regFile,newState,++i);
		if (pc >= 4)	// I originally had cycle = pc/4 but needed to change this to account for pc not incrementing during a stall
			cycle++;
		// updates the register file. (need to do that at the top to make sure all information is provided for forwarding)
		if (cycle-4 >= 0 && (cycle-4) < numOfIn+numStalls)
			regFileUpdate(instructions[cycle-4],regFile,dataMem,numOfIn);

		// IF/ID
		newState.ifid = EmptyIfid;
		if (cycle < numOfIn+numStalls)
			newState.ifid.instruction = instructions[cycle].inst;
		newState.ifid.pcPlus4 = pc + 4;		// tracking pcPlus4
		// ID/EX
		newState.idex = EmptyIdex;
		newState.idex.pcPlus4 = pc;
		if ((cycle-1) >= 0 && (cycle-1) < numOfIn+numStalls){
			// execution stage (accessing the alu)
			instructions[cycle-1].aluRes = aluRes(instructions[cycle-1],instructions[cycle-2],instructions[cycle-3],regFile,dataMem,numOfIn,cycle);
				// need to do this bc of load word (aluRes != contents of destination register for lw)
				instructions[cycle-1].destCont = instructions[cycle-1].aluRes;
				if (strncmp(instructions[cycle-1].inst,"lw",2) == 0)
					instructions[cycle-1].destCont = dataMem[((regFile[instructions[cycle-1].rs] + instructions[cycle-1].imm)-numOfIn*4)/4];
			// DEALING WITH STALLS (testing a specific instruciton in test case 1 currently)
			if ((cycle-2) >= 0 && strncmp(instructions[cycle-2].inst,"lw",2) == 0 && needStall(instructions[cycle-2],instructions[cycle-1]) == 1){
					stall(instructions,EmptyInstr,cycle,numOfIn+(numStalls++));
					pc -= 4;	// to make sure pc doesn't change during a stall
					newState.ifid.pcPlus4 = pc + 4;		// need to recalculate bc PC changed
			}// end stall handling
			newState.idex.instruction = instructions[cycle-1].inst;
			newState.idex.rData1 = regFile[instructions[cycle-1].rs];
			newState.idex.rData2 = regFile[instructions[cycle-1].rt];
			newState.idex.imm = instructions[cycle-1].imm;
			// dealing with what is printed with halt and noop (specifically, this handles telling the differenece between '$0' and '0' in printing the ID/EX section)
			if (strcmp(instructions[cycle-1].inst,"halt") != 0 && strcmp(instructions[cycle-1].inst,"NOOP") != 0){
				strcpy(newState.idex.rs,findRegName(instructions[cycle-1].rs));
				strcpy(newState.idex.rt,findRegName(instructions[cycle-1].rt));
				// to make sure rd and rs read "0" or "$0" when they should
				if (strncmp(instructions[cycle-1].inst,"add",3) == 0 || strncmp(instructions[cycle-1].inst,"sub",3) == 0 || strncmp(instructions[cycle-1].inst,"sll",3) == 0){
					if (strncmp(instructions[cycle-1].inst,"sll",3) == 0)
						strcpy(newState.idex.rs,"0");		// updating rt
					strcpy(newState.idex.rd,findRegName(instructions[cycle-1].rd));		// updating rd
				}
			}
			newState.idex.branchTarg = ((instructions[cycle-1].imm*4) + newState.idex.pcPlus4);
		}
		// EX/MEM
		newState.exmem = EmptyExmem;
		if (((cycle-2) >= 0 && (cycle-2) < numOfIn+numStalls) && strcmp(instructions[cycle-2].inst,"NOOP") != 0){
			newState.exmem.instruction = instructions[cycle-2].inst;
			newState.exmem.aluRes = instructions[cycle-2].aluRes;
			newState.exmem.wrDatReg = instructions[cycle-2].rt;			// CHECK AFTER FINISHING PIPELINING
			strcpy(newState.exmem.wrReg,findWrReg(instructions[cycle-2]));
		}
		// MEM/WB
		newState.memwb = EmptyMemwb;
		if (((cycle-3) >= 0 && (cycle-3) < numOfIn+numStalls) && strcmp(instructions[cycle-3].inst,"NOOP") != 0){
			newState.memwb.instruction = instructions[cycle-3].inst;
			// should only be updated when data memory is accessed (STILL CANNOT TELL WHAT THE KECJ THIS DOES*********************)
			if (strncmp(instructions[cycle-3].inst,"lw",2) == 0)
				newState.memwb.wrDatMem = dataMem[(prevState.exmem.aluRes-numOfIn*4)/4];
			newState.memwb.wrDatALU = prevState.exmem.aluRes;
			strcpy(newState.memwb.wrReg,findWrReg(instructions[cycle-3]));
			memDataUpdate(instructions[cycle-4],regFile,dataMem,numOfIn);	// updates the data memory********************
			
			if (strcmp(instructions[cycle-3].inst,"halt") == 0)
				halt = 1;
		}

		prevState = newState;
		pc += 4;
		if (halt == 1)		// print the last cycle
			printCycle(pc,dataMem,regFile,newState,++i);
	}
	printf("********************\n");
	printf("Total number of cycles executed: %d\n",i);
	printf("Total number of stalls: %d\n", numStalls);
	printf("Total number of branches: %d\n", numBranches);
	printf("Total number of mispredicted branches: %d\n", numMisBranches);
	
	return 0;
}// end main

// when the load word instruction is immediately followed by another instruction which reads load words destination register, return true (1 in this case)
int needStall(struct Instr subject, struct Instr test1){
	if (test1.op == 35){
		if (subject.destReg == test1.srcReg)
			return 1;
	}
	else{
		if (subject.destReg == test1.rs || subject.destReg == test1.rt)
			return 1;
	}
	return 0;
}

// shifts over all contents in the intruction array and sticks a noop into the freed space in the middle
void stall(struct Instr* inst, struct Instr empty, int pos, int numOfIn){
	int i;
	for (i = numOfIn-1; i >= pos-1; i--)
		inst[i+1] = inst[i];
	inst[pos-1] = empty;
}

// updates the memData integer array
void memDataUpdate(struct Instr inst, int* regFile, int* dataMem, int numOfIn){
	if (strncmp(inst.inst,"sw",2) == 0)
		dataMem[(inst.aluRes-numOfIn*4)/4] = regFile[inst.rt];
}

// updates the register file integer array
void regFileUpdate(struct Instr inst, int* regFile, int* dataMem, int numOfIn){
	if (strncmp(inst.inst,"add",3) == 0 || strncmp(inst.inst,"sub",3) == 0 || strncmp(inst.inst,"sll",3) == 0)
		regFile[inst.rd] = inst.aluRes;
	if (strncmp(inst.inst,"andi",4) == 0 || strncmp(inst.inst,"ori",3) == 0)
		regFile[inst.rt] = inst.aluRes;
	if (strncmp(inst.inst,"lw",2) == 0)
		regFile[inst.rt] = dataMem[(inst.aluRes-numOfIn*4)/4];
}

// determines if there is a hazard based on given instructions (returns -1 if false, 0-9 if true).
int isHazard(struct Instr subject, struct Instr test1, struct Instr test2, int cycle){
	if (cycle > 1){
		// if the instruction is lw, sw, sll, andi, or ori (will have one source register)
		if (subject.op == 35 || subject.op == 43 || (subject.op == 0 && subject.funct == 0) || subject.op == 12 || subject.op == 13){
			if (subject.srcReg == test2.destReg)	// if source is equal to target of current MEM/WB instruction
				return 0;
			else if (subject.srcReg == test1.destReg)	// if source is equal to target of current EX/MEM instruction
				return 1;
		}
		else{	// if the instruction is add,sub,or bne (will have two source registers)
		// deal with the fact that there is two source registers (rs and rt for all instructions)
			if (subject.rs == test2.destReg && subject.rt == test1.destReg)
				return 2;
			else if (subject.rt == test2.destReg && subject.rs == test1.destReg)
				return 3;
			else if (subject.rs == test2.destReg && subject.rt != test1.destReg && (cycle-4) >= 0)
				return 4;
			else if (subject.rs != test2.destReg && subject.rt == test1.destReg)
				return 5;
			else if (subject.rt == test2.destReg && subject.rs != test1.destReg && (cycle-4) >= 0)
				return 6;
			else if (subject.rt != test2.destReg && subject.rs == test1.destReg)
				return 7;
			else if (subject.rs == test2.destReg && subject.rt == test2.destReg && (cycle-4) >= 0)
				return 8;
			else if (subject.rs == test1.destReg && subject.rt == test1.destReg)
				return 9;
		}
	}
	return -1;
}

// ALU result function that calculates what the alu would produce for a given instruction (also handles pipelining).
int aluRes(struct Instr inst, struct Instr test1, struct Instr test2, int* regFile, int* dataMem, int numOfIn, int cycle){
	int result;
	int rs = regFile[inst.rs];
	int rt = regFile[inst.rt];

	// printing for testing purposes
	printf("%d %d %s\n",isHazard(inst,test1,test2,cycle),regFile[8],inst.inst);
	printf("%d\n",test2.aluRes);
	if (isHazard(inst,test1,test2,cycle) > -1){		// if there is a hazard... forward appropiately based on cases 0-9
		switch(isHazard(inst,test1,test2,cycle))
		{
			case 0:
				if (inst.op == 0 && inst.funct == 0)
					rt = test2.destCont;
				else
					rs = test2.destCont;
				break;
			case 1:
				if (inst.op == 0 && inst.funct == 0)
					rt = test1.destCont;
				else
					rs = test1.destCont;
				break;
			case 2:
				rs = test2.destCont;
				rt = test1.destCont;
				break;
			case 3:
				rs = test1.destCont;
				rt = test2.destCont;
				break;
			case 4:
				rs = test2.destCont;
				break;
			case 5:
				rt = test1.destCont;
				break;
			case 6:
				rt = test2.destCont;
				break;
			case 7:
				rs = test1.destCont;
				break;
			case 8:
				rs = test2.destCont;
				rt = test2.destCont;
				break;
			case 9:
				rs = test1.destCont;
				rt = test1.destCont;
				break;
		}
	}
	// actual ALU calculation depending on the instruction
	if (strncmp(inst.inst,"add",3) == 0)
		result = rs + rt;
	else if(strncmp(inst.inst,"sub",3) == 0)
		result = rs - rt;
	else if(strncmp(inst.inst,"lw",2) == 0)
		result = rs + inst.imm;
	else if( strncmp(inst.inst,"sw",2) == 0)
		result = rs + inst.imm;
	else if(strncmp(inst.inst,"sll",3) == 0)
		result = rt << inst.shamt;
	else if(strncmp(inst.inst,"andi",4) == 0)
		result = rs & inst.imm;
	else if(strncmp(inst.inst,"ori",3) == 0)
		result = rs | inst.imm;
	else if(strncmp(inst.inst,"bne",3) == 0)
		result = rs != rt;
	else if(strcmp(inst.inst,"halt") == 0)
		result = 0;
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
				else if(inst[i].funct == 0){
					sprintf(inst[i].inst,"sll %s,%s,%d",rd,rt,inst[i].shamt);
					inst[i].srcReg = inst[i].rt;
				}
				inst[i].destReg = inst[i].rd;
				break;
			case 35:
				sprintf(inst[i].inst,"lw %s,%d(%s)",rt,inst[i].imm,rs);
				inst[i].srcReg = inst[i].rs;
				inst[i].destReg = inst[i].rt;
				break;
			case 43:
				sprintf(inst[i].inst,"sw %s,%d(%s)",rt,inst[i].imm,rs);
				inst[i].srcReg = inst[i].rs;
				inst[i].destReg = -1;	// there is no destination register
				break;
			case 12:
				sprintf(inst[i].inst,"andi %s,%s,%d",rt,rs,inst[i].imm);
				inst[i].srcReg = inst[i].rs;
				inst[i].destReg = inst[i].rt;
				break;
			case 13:
				sprintf(inst[i].inst,"ori %s,%s,%d",rt,rs,inst[i].imm);
				inst[i].srcReg = inst[i].rs;
				inst[i].destReg = inst[i].rt;
				break;
			case 5:
				sprintf(inst[i].inst,"bne %s,%s,%d",rs,rt,inst[i].imm);
				break;
		}
		// handling noops
		if (currLine == 0)
			sprintf(inst[i].inst,"NOOP");
		else if (currLine == 1)
			sprintf(inst[i].inst,"halt");
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

// prints all the information for one full cycle
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

// helper function to brute force print all the information for each stage of the pipeline for one cycle
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

// helper to find write register (the thing that prints in the EX/MEM stage)
char* findWrReg(struct Instr inst){
	char* temp;
	if (strncmp(inst.inst,"bne",3) == 0 || strcmp(inst.inst,"halt") == 0)
		temp = "0";
	else if (strncmp(inst.inst,"add",3) == 0 || strncmp(inst.inst,"sub",3) == 0 || strncmp(inst.inst,"sll",3) == 0)
		temp = findRegName(inst.rd);
	else
		temp = findRegName(inst.rt);
	return temp;
}
