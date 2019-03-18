//-------PORJ1.C-------
//
// Ryan Gutierrez
// CDA3101
// Implementation of a MIPS Assembler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Label{char name[256]; int pos;};

int findRegNum(char *r){
	char regType;
	int regNum;
	sscanf(r,"%c%d",&regType,&regNum);
	
	if (regType == '0')
		return 0;
	else if (regType == '1')
		return 1;
	else if (regType == 't')
		return (regNum + 8);
	else if (regType == 's')
		return (regNum + 16);
}

int findLabelPos(struct Label *labels, char *l){
	int i;
	for (i = 0; i < 202; i++){
		if (strcmp(labels[i].name,l) == 0)
			return labels[i].pos * 4;
	}
}

int main(){

	int isDirective = -1;	// tracks whether a label references a directive, 0 for true -1 for false (the default)
	char currLine[530];	// creating an array of 530 to store lines while accounting for length of labels
	char tempLine[530];	// used to copy strings for modification
	char tempLabel[256];	// used as a temporary place holder for labels when splitting up an la instruct
	char tempOp[3];		// used as temporary storage for operands

	// main data storage	// INCLUDE WHY 202 IN README--------------------------------------------------------
	char Lines[202][530];	// 202 is to account for 100 possible lines of code (with all of the instructions being la)
	struct Label labels[202];	// 202 is the largestt possible amnt of labels
	
	// first pass*************************
	int i = 0;
	int k = 0;
	fgets(currLine, sizeof(currLine), stdin);	// ignoring the first line (.text)
	while(fgets(currLine, sizeof(currLine), stdin)){

		// splitting la into lui and ori
		memset(tempLine, 0, sizeof(tempLine));	// resetting tempLine
		if (currLine[0] != '\t')
			sscanf(currLine,"%*[^\t]%s",tempLine);	// grabbing the instruction
		else
			sscanf(currLine,"%*1c%s",tempLine);	// grabbing the instruction			
		
		// grabbing labels
		if (currLine[0] != '\t'){	// if there is no tab that would mean it is a label
			sscanf(currLine,"%[^:]", labels[k].name);
			labels[k++].pos = i;
		}
		
		if (strcmp(tempLine,"la") == 0){
			// grabbing information
			memset(tempLabel, 0, sizeof(tempLabel));	// resetting tempLabel
			sscanf(currLine,"%*[^,]%*1c%s",tempLabel);	// grabbing label at end of la instruc
			memset(tempOp, 0, sizeof(tempOp));
			sscanf(currLine,"%*[^$]%4s",tempOp);
			// storing lui
			strcat(Lines[i],"lui	$1,"); strcat(Lines[i],tempLabel);
				strcat(Lines[i],"[31-16]");
			strcat(Lines[i++],"\n");	// adding a newline to help me better read when printing instrucs and retain formatting
			// storing ori
			strcat(Lines[i],"ori	"); strcat(Lines[i],tempOp); 
			strcat(Lines[i],"$1,"); strcat(Lines[i],tempLabel);
				strcat(Lines[i],"[15-0]");
			strcat(Lines[i++],"\n");
		}
		
		// storing everything else (all of the instrucions)
		else if (strcmp(tempLine,"la") != 0 && currLine[0] != '\t'){	// if the instr is not la but it has a label
			sscanf(currLine,"%*[^\t]%*1c%[^\n]",Lines[i]);
			strcat(Lines[i++],"\n");
		}
		else if (strcmp(tempLine,"la") != 0){	// if the instruction is not la
			sscanf(currLine,"%*1c%[^\n]",Lines[i]);
			strcat(Lines[i++],"\n");
		}
	}
	
	// second pass***********************
	for (k = 0; k < i; k++){
		char instruct[4];	// set to 4 bc addi is the longest instruction we have been asked to handle
		char reg1[2];		// possibility of three registers in R type instructs
		char reg2[2];
		char reg3[256];		// 256 so that it may be used as a label as well
		int shamtImm;		// using for both shift amount and immediate
		int instcode = 0;
		sscanf(Lines[k],"%s",instruct);
		
		printf("0x%08X: ",k*4);
		 // handling J type instructions (did this first bc it's one implementation for 10 pnts)
		if (strcmp(instruct,"j") == 0){
			sscanf(Lines[k],"%*2c%[^\n]",reg3);
			shamtImm = findLabelPos(labels,reg3)/4;
			instcode = (2 << 26) | shamtImm;
			printf("0x%08X\n",instcode);
		}// handling R type instructions
		else if (strcmp(instruct,"add") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*2c%[^\n]",reg1,reg2,reg3);
			instcode = ((((((((instcode << 5) | findRegNum(reg2)) << 5) | findRegNum(reg3)) << 5) | findRegNum(reg1)) << 11) | 32);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"nor") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*2c%[^\n]",reg1,reg2,reg3);
			instcode = ((((((((instcode << 5) | findRegNum(reg2)) << 5) | findRegNum(reg3)) << 5) | findRegNum(reg1)) << 11) | 39);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"sll") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*c%d",reg1,reg2,&shamtImm);
			instcode = (((((((instcode << 10) | findRegNum(reg2)) << 5) | findRegNum(reg1)) << 5) | shamtImm) << 6);
			printf("0x%08X\n",instcode);
		}// handling I type instructions
		else if (strcmp(instruct,"addi") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*c%d",reg1,reg2,&shamtImm);
			instcode = ((((((8 << 5) | findRegNum(reg2)) << 5) | findRegNum(reg1)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"ori") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*c%d",reg1,reg2,&shamtImm);
			if (strchr(Lines[k],'[') != NULL){	// handling the la verion
				sscanf(Lines[k],"%*[^,]%*4c%[^[]",tempLabel);	// was giving me errors when putting %*[^,] where %*4c is
				shamtImm = findLabelPos(labels,tempLabel);
				shamtImm = shamtImm & 65535;
			}
			instcode = ((((((13 << 5) | findRegNum(reg2)) << 5) | findRegNum(reg1)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"lui") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*c%d",reg1,&shamtImm);
			if (strchr(Lines[k],'[') != NULL){	// handling the la verion
				sscanf("%*[^,]%*4c%[^[]",tempLabel);
				shamtImm = findLabelPos(labels,tempLabel);
				shamtImm = shamtImm >> 16;
			}
			instcode = ((((15 << 10) | findRegNum(reg1)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"sw") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*c%d%*2c%[^)]",reg1,&shamtImm,reg2);
			instcode = ((((((43 << 5) | findRegNum(reg2)) << 5) | findRegNum(reg1)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"lw") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*c%d%*2c%[^)]",reg1,&shamtImm,reg2);
			instcode = ((((((35 << 5) | findRegNum(reg2)) << 5)  | findRegNum(reg1)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
		else if (strcmp(instruct,"bne") == 0){
			sscanf(Lines[k],"%*[^$]%*c%[^,]%*2c%[^,]%*c%[^\n]",reg1,reg2,reg3);
			shamtImm = ((findLabelPos(labels,reg3)/4) - k)*4;
			instcode = ((((((5 << 5) | findRegNum(reg1)) << 5) | findRegNum(reg2)) << 16) | shamtImm);
			printf("0x%08X\n",instcode);
		}
	}
	
	// printing instructions		// for testing purposes
	//printf("\n");
	//for (k = 0; k < i; k++)		// i holds the number of instructions
	//	printf("%s",Lines[k]);
	
	return 0;
}
