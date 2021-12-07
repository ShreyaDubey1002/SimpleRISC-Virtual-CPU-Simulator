
/*

The project is developed as part of Computer Architecture class
Project Name: Functional/Pipeline Simulator for simpleRISC Processor

Developer's Name:
Developer's Email id:
Date:

*/

/* mySimpleSim.cpp
   Purpose of this file: implementation file for mySimpleSim
*/

//#include "mySimpleSim.h"
#include <iostream>
#include <fstream>

//Register file
static unsigned int R[16];
static int PC = 0x0;
static int offset = 0;
static int branchTarget = 0;
static int immx =0;
static int immediate_bit = 0;
static int modifier_bit = 0;
static int isSt = 0;
static int isCall = 0;
static int branchPC = 0;
static int isBranchTaken = 0;
static int aluResult = 0;
static int ldResult = 0;

 static int opcode = 0;
static unsigned mar = 0,mdr = 0;
//flags
static int gt = 0,eq = 0;
//memory
static unsigned char MEM[4000];

//intermediate data path and control path signals
static unsigned int inst ;
static unsigned int operand1;
static unsigned int operand2;
static unsigned int op2;
static unsigned int isRet=0;
static unsigned int isBeq=0;
static unsigned int isBgt=0;
static unsigned int isLd=0;
static unsigned int isWb = 1;
static unsigned int index_register = 0 ;
static unsigned int index_register2 = 0 ;
static int no_instruction = 0 ;
static int run = 1 ;

void run_simplesim();
void reset_proc();
void load_program_memory(char* file_name);
void write_data_memory();

//reads from the instruction memory and updates the instruction register
void fetch();
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode();
//executes the ALU operation based on ALUop
void execute();
//perform the memory operation
void mem();
//writes the results back to register file
void write_back();
void printRegisters();
void display_control_Signals();

int read_word(char *mem, unsigned int address);
void write_word(char *mem, unsigned int address, unsigned int data);
void exit(int a);

class pipeline{
public:
int PC ;
int instruction;
int branchTarget;
int A ;
int B;
int op2 ;
int isSt;        // control flow
int isRet;       // control flow
int isImmediate; // control flow
int isWb ;       // control flow
int isBeq ;	     // control flow
int isBgt ;      // control flow
int isCall ;       // control flow
int aluResult;
int ldResult;
int isLd ;
//int isBranchTaken ;


   	pipeline(){
    	this->PC = 0 ;
    	this->instruction = 0x68000000 ;
   	this->branchTarget = 0;
    	this->A = 0 ;
    	this->B = 0;
    	this->op2 = 0 ;
    	this->isSt = 0;        // control flow
    	this->isRet = 0;       // control flow
    	this->isImmediate = 0; // control flow
    	this->aluResult = 0;
    	this->ldResult = 0;
        this->isBeq = 0 ;      // control flow
        this->isBgt = 0 ;      // control flow
        this->isCall = 0 ;       // control flow
    	this->isLd = 0 ;
    	//this->isBranchTaken = 0 ;

    	}
   	void pipeline_flush(){
    	this->PC = 0 ;
    	this->instruction = 0x68000000;
    	this->branchTarget = 0;
    	this->A = 0 ;
    	this->B = 0;
    	this->op2 = 0 ;
    	this->isSt = 0;        // control flow
    	this->isRet = 0;       // control flow
    	this->isImmediate = 0; // control flow
        this->isWb = 0 ;
    	this->aluResult = 0;
    	this->ldResult = 0;
    	this->isBeq = 0 ;      // control flow
        this->isBgt = 0 ;      // control flow
        this->isCall = 0 ;       // control flow
        //this->isBranchTaken = 0 ;
         }
	};
pipeline *IF_OF = new pipeline();
pipeline *OF_EX = new pipeline();
pipeline *EX_MA = new pipeline();
pipeline *MA_RW = new pipeline();

int Branch_lock_unit();
int Data_lock_Unit();


void run_simplesim() {
int data_dependency = 0 ;
int branch_dependency = 0 ;
 while(MA_RW->instruction != 0xffffffff) {

    write_back();
    mem();
    execute();
    decode();
    fetch();
    // now check  for data and branch dependency
    data_dependency =   Data_lock_Unit();
    branch_dependency = Branch_lock_unit();
    if (data_dependency == 1){
       IF_OF->instruction =  0x68000000 ;
        }
    if (branch_dependency == 1){
        IF_OF->instruction =  0x68000000 ;
        OF_EX->instruction =  0x68000000 ;
        }
    }
}

// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
    int i ;
    for (i = 0 ; i < 16 ; i++){
        R[i] = 0 ;
    }

   R[14] = 4000;
}

//load_program_memory reads the input memory, and pupulates the instruction
// memory
void load_program_memory(char *file_name) {
  FILE *fp;
  unsigned int address, instruction;
  fp = fopen(file_name, "r");
  if(fp == NULL) {
    printf("Error opening input mem file\n");
    exit(1);
  }
  while(fscanf(fp, "%x %x", &address, &instruction) != EOF) {
    write_word(MEM, address, instruction);
    no_instruction ++ ;
    printf("instruction:  %x\n",instruction);
    }
  fclose(fp);
}

//writes the data memory in "data_out.mem" file
void write_data_memory() {
  FILE *fp;
  unsigned int i;
  fp = fopen("data_out.mem", "w");
  if(fp == NULL) {
    printf("Error opening dataout.mem file for writing\n");
    return;
  }

  for(i=0; i < 4000; i = i+0x4){
     fprintf(fp, "%x %x\n", i, read_word(MEM, i));
  }
  fclose(fp);
}

//reads from the instruction memory and updates the instruction register
void fetch() {
    //int PC ;

    if (isBranchTaken == 1){
    PC = branchPC ; // check once
   }
    isBranchTaken = 0 ;
inst  = read_word(MEM,PC);
if (inst == 0xffffffff){
    return ;
}
printf("FETCH UNIT :\n");
printf("\n");
printf("Instruction Read: 0x%x\n",inst);

if(inst == 0xffffffff){
printf("EXIT\n");
}
printf("PC= %x\n",PC);
printf("\n");

IF_OF->PC = PC ;
IF_OF->instruction = inst ;

}
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode() {
   // int i=PC;
     // extracting opcode
    // there are 3 cases
    if (IF_OF->instruction == 0xffffffff){
    return ;
    }
   printf("DECODE UNIT : \n");
   printf("\n");
	int opcode = (IF_OF->instruction & 0xf8000000) >> 27 ;
    //unsigned int isSt,isWb,isRet,offset,opcode,isBeq,isBgt,isCall,immx,immediate_bit,modifier_bit,operand1,operand2 ;
    //case 1: o Address instruction
    if (opcode == 0x0f){ // given instruction is store
        isSt = 1 ;
        isWb = 0 ;
	//printf("instruction : STORE\n");
    }

    if(opcode== 0x0d){ // opcode = 0x
        // given instruction is NOP so do nothing
        isWb = 0 ;
        printf("instruction : NOP\n");
   }

    else if(opcode==0x14)// opcode = 0x14
    {
        //Given instruction : RET
        isRet=1;
        isWb = 0 ;
        printf("instruction : RET\n");
    }

     //case 2: 1 Address instruction :b,beq ,bgt ,call

    else if((opcode&0x10)>>4 ==1)
     {
       offset=  IF_OF->instruction & 0x07ffffff;
      if ((( IF_OF->instruction & 0x04000000) >> 26) == 1)
       offset = offset | 0xf8000000 ;

        offset = (offset << 2 );
       printf("offset %d\n",offset);
        branchTarget=IF_OF->PC + offset;
       if (opcode == 0x10) // beq
       {
           printf("instruction : BEQ\n");
           isBeq=1;
           isWb = 0 ;
       }
    else if (opcode == 0x11) // bgt
       {
           printf("instruction : BGT\n");
           isBgt=1;
           isWb = 0 ;
       }
     else if (opcode == 0x13){ // call
        printf("instruction : CALL\n");
          isCall = 1 ;

}

else
printf("instruction : B\n");
     }

     else { // 2 and 3 encoded address
           immx= IF_OF->instruction & 0x0000ffff;
        immediate_bit=(IF_OF->instruction & 0x4000000) >> 26;
        if(immediate_bit == 1){

            modifier_bit = ( IF_OF->instruction & 0x30000)>>16;
            if(modifier_bit==0)  //signed extension
               {
                  // immx=immx<<16;
                  // immx=immx>>16;
                // looking 16 th bit & extending
               if (((IF_OF->instruction & 0x00008000) >> 15 ) == 1){
                     immx = immx | 0xffff0000 ;
               }

               }
           else if (modifier_bit == 0x2){ // 'h' check one more time `
            immx=immx<<16;
            }  // 'u' for 'u'
            else {
            immx = immx | 0x00000000;

            }
            }
         }
          if (isRet == 1){
            operand1=R[15];
            }
        else{
        index_register = (IF_OF->instruction & 0x3c0000)>>18 ;
        operand1 =  R[index_register];
 	printf("index_reg = %d\n",index_register);
        }
        if (isSt == 1){
          index_register2 = (IF_OF->instruction & 0x3c00000)>>22;
        operand2 = R[index_register2];
        }
        else {
           index_register2 = (IF_OF->instruction & 0x3c000)>>14;
         operand2 = R[index_register2];
        }
        if (opcode == 0x05 ){// instruction is compare
         isWb = 0 ;
        }
        printf("Immx %x\n",immx);
        printf("Operand 1: %d\n",operand1);
        printf("Operand 2: %d\n",operand2);
        printf("\n");


        OF_EX->PC = IF_OF->PC ;
        OF_EX->branchTarget = branchTarget ;
        OF_EX->op2 = operand2 ;
        OF_EX->instruction = IF_OF->instruction ;
        OF_EX->isBeq = isBeq ;
        OF_EX->A = operand1;

        OF_EX->isCall = isCall;
        OF_EX->isBgt = isBgt;
        //OF_EX->isImmediate = i;
        OF_EX->isImmediate = immediate_bit;
        OF_EX->isBeq = isBeq;
        OF_EX->isRet = isRet;
        OF_EX->isSt = isSt;
        OF_EX->isWb = isWb;

        if (immediate_bit == 1){
           OF_EX->B = immx;
        }
        else {
            OF_EX->B = operand2 ;
        }

        IF_OF->pipeline_flush();




        }
    // now reading the register files
   // printf("%d\n",immx);

//executes the ALU operation based on ALUop
void execute() {
    //calculating branch unit
    if (OF_EX->instruction == 0xffffffff){
    return ;
    }
    opcode = (OF_EX->instruction & 0xf8000000) >> 27 ;
    printf("EXECUTE UNIT \n");
    printf("\n");
    if (OF_EX->isRet == 1){
    branchPC= OF_EX->A;
    }
    else
    branchPC=OF_EX->branchTarget;

    if((OF_EX->isBeq==1 && eq==1) || (OF_EX->isBgt==1 && gt==1) || opcode == 0x12 || opcode == 0x13 || opcode == 0x14)
    {
    isBranchTaken = 1 ;
    }
 //   if(immediate_bit==1)
   // {
    //op2=OF_EX->B;
    //}
    //else
    //op2=operand2;
//ALU:---
   gt = 0 ;
   eq = 0 ;
    if(opcode==0x00){//Addition
printf("instruction : ADDITION\n");
    aluResult = OF_EX->A + OF_EX->B;
}
    else if(opcode==0x01){//Subtraction
printf("instruction : SUBTRACTION\n");
    aluResult = OF_EX->A - OF_EX->B;
       // printf("aluResult_subs %d\n",aluResult);
       //     printf("operand1 %d\n",operand1);
       }

    else if(opcode==0x05)//Compare
    {
printf("instruction : COMPARE\n");
    aluResult = OF_EX->A - OF_EX->B;

    if(aluResult>0)
    gt=1;

    else if(aluResult==0)
    eq=1;

}
else if(opcode==0x02){//Multiplication
printf("instruction : MULTIPLICATION\n");
 aluResult = OF_EX->A * OF_EX->B;
}
else if(opcode==0x03){//Division
printf("instruction : DIVISION\n");
 aluResult = OF_EX->A / OF_EX->B;
}
else if(opcode==0x04){//Modulus
printf("instruction : MODULUS\n");
 aluResult = OF_EX->A % OF_EX->B;
}
  else if(opcode==0x0a){//LSL
printf("instruction : LSL\n");
 aluResult = OF_EX->A << OF_EX->B;
}
  else if(opcode==0x0b){//LSR
printf("instruction : LSR\n");
 aluResult = (int)((unsigned int)OF_EX->A >> OF_EX->B);
}
  else if(opcode==0x0c){//ASR
printf("instruction : ASR\n");
 aluResult = OF_EX->A >> OF_EX->B ;
}
  else if(opcode==0x07){//OR
printf("instruction : OR\n");
 aluResult = OF_EX->A | OF_EX->B;
}
  else if(opcode==0x08){//NOT
printf("instruction : NOT\n");
 aluResult = ~ OF_EX->A;
}
  else if(opcode==0x06){//AND
 printf("instruction : AND\n");
 aluResult =  OF_EX->A & OF_EX->B;
}
  else if(opcode==0x09){//MOV{
printf("instruction : MOV\n");
  aluResult=OF_EX->A;//check once
  printf("MOV Result %d\n",OF_EX->B);
}
 else if(opcode==0x0e){//LD
   // printf("Enter1\n");
printf("instruction : LD\n");
    isLd = 1 ;
   aluResult=(OF_EX->A+OF_EX->B);
  //  printf("&R[index_register] = %d\n",&R[index_register]);
   //printf("alu Result = %d\n",aluResult);
  //  printf("immx = %d\n",immx);

}
   else if(opcode==0x0f){//ST
printf("instruction : ST\n");
       OF_EX->isSt = 1;
       printf("immx %x\n",OF_EX->B);
  aluResult=(OF_EX->A+OF_EX->B);

   }
  // printf("index = %d\n",index_register);
 printf("AluResult : %d\n" , aluResult);
 printf("isBranchTaken : %d\n" , isBranchTaken);
 printf("\n");

        EX_MA->PC = OF_EX->PC ;
        EX_MA->op2 = OF_EX->op2 ;
        EX_MA->instruction = OF_EX->instruction ;
        EX_MA->isBeq = isBeq ;
       // EX_MA->isBranchTaken = isBranchTaken ;
        EX_MA->isCall = OF_EX->isCall;
        EX_MA->isBgt = OF_EX->isBgt;
        EX_MA->isImmediate = OF_EX->isImmediate;
        EX_MA->isBeq = OF_EX->isBeq;
        EX_MA->isRet = OF_EX->isRet;
        EX_MA->isSt = OF_EX->isSt;
        EX_MA->isWb = OF_EX->isWb;
        EX_MA->isLd = isLd;
        EX_MA->aluResult = aluResult;

        OF_EX->pipeline_flush();

 }
//perform the memory operation
void mem() {
   //  printf(" opcode = %d\n",opcode);
   if (EX_MA->instruction == 0xffffffff){
    return ;
    }
    int opcode = (EX_MA->instruction & 0xf8000000) >> 27 ;
     printf("MEM UNIT\n");
     printf("\n");
    if((opcode==0x0e)||(opcode==0x0f))//if isLd or isSt
        {
        printf("operand 1 %d\n",operand1);
        mdr=EX_MA->op2;
        mar=EX_MA->aluResult;

        if (opcode==0x0e){//Load
            isLd = 1 ;
            ldResult  = read_word(MEM, EX_MA->aluResult);
            printf("ld Result : %x\n",ldResult);
           }
        else{
            write_word(MEM,EX_MA->aluResult,EX_MA->op2) ;
             mdr = EX_MA->op2 ;
            }

    }
    else{
        printf("No Change\n");
    }

  printf("\n");



        MA_RW->PC = EX_MA->PC ;
        MA_RW->instruction = EX_MA->instruction ;
        MA_RW->isBeq = EX_MA->isBeq ;
       // MA_RW->isBranchTaken = EX_MA->isBranchTaken;
        MA_RW->isCall = EX_MA->isCall;
        MA_RW->isBgt = EX_MA->isBgt;
        MA_RW->isImmediate = EX_MA->isImmediate;
        MA_RW->isBeq = EX_MA->isBeq;
        MA_RW->isRet = EX_MA->isRet;
        MA_RW->isSt = EX_MA->isSt;
        MA_RW->isWb = EX_MA->isWb;
        MA_RW->isLd = EX_MA->isLd;
        MA_RW->aluResult = EX_MA->aluResult;
        MA_RW->ldResult = ldResult;

        EX_MA->pipeline_flush();

}
//writes the results back to register file
void write_back() {
    if (MA_RW->instruction == 0xffffffff){
    return ;
    }
  int  opcode = (MA_RW->instruction & 0xf8000000) >> 27 ;
    printf("WRITE BACK UNIT :\n");
    printf("\n");
   int index_destination;
  //  printf("write %d\n ",isWb);
    if (MA_RW->isWb == 1){
  if (MA_RW->isCall == 1){
    R[15] = MA_RW->PC + 0x4 ;
     printf("R[15] =  %d\n ",R[15]);
    }
  else {
       index_destination = (MA_RW->instruction & 0x03c00000) >> 22 ;
        //printf("index %d\n ",index_destination);
        //printf("aluResult %d\n",isLd);
        if (MA_RW->isLd == 1){
            R[index_destination] = MA_RW->ldResult ;
          printf("ldResult = %d\n",MA_RW->ldResult);
           isLd = 0 ;
          }
         else {
            R[index_destination] = MA_RW->aluResult ;
         }
 printf("R[%d] =  %d\n ",index_destination,R[index_destination]);
  }

  }
      display_control_Signals();
    if (Data_lock_Unit() == 0 && isBranchTaken == 0){
     PC = PC + 0x4 ;
       branchPC = 0 ;}


   isRet = 0 ;
   isBeq = 0 ;
   isBgt = 0 ;
   isSt = 0 ;
   isWb =1 ;
   isCall = 0;
   isLd = 0;
   isSt = 0;
   operand1 = 0 ;
   operand2 = 0 ;
   immx = 0 ;
   op2 = 0 ;
   mdr = 0 ;
   mar = 0 ;
   aluResult = 0 ;
   ldResult = 0 ;
  // printf("R[15] %d\n",R[15]);
printf("Branch PC = %d\n",branchPC);
printRegisters();
//printf("isRet = %d\n",isRet);

  MA_RW->pipeline_flush();
printf("*-----------------------------------------------------------------*\n");

}
void display_control_Signals(){
printf("isSt  %d\n",isSt);
printf("isLd  %d\n",isLd);
printf("isBeq %d\n",isBeq);
printf("isBgt %d\n",isBgt);
printf("isRet %d\n",isRet);
printf("isCall %d\n",isCall);
printf("isWb %d\n",isWb);
printf("\n");
printf("Flags: \n");
printf("isGt : %d\n",  gt);
printf("isEq : %d\n",eq);
printf("\n");
};

void printRegisters(){
int i = 0 ;
for(i = 0 ;i<16 ; i++){
    printf("R[%d]  : %d\n",i,R[i]);
}
}



int read_word(char *mem, unsigned int address) {
  int *data;
  data =  (int*) (mem + address);
  return *data;
}

void write_word(char *mem, unsigned int address, unsigned int data) {
  int *data_p;
  data_p = (int*) (mem + address);
  *data_p = data;
//printf("writing %x\n",data);
}
int dependency(unsigned int A , unsigned int B){
   int  opcodeA = (A & 0xf8000000) >> 27 ;
   int  opcodeB = (B & 0xf8000000) >> 27 ;
   if (opcodeA == 0x0d ||opcodeA == 0x10 || opcodeA == 0x11 || opcodeA == 0x12 ||opcodeA == 0x13){// instA = nop,b,beq,bgt,call
    return 0 ;
   }
   if (opcodeB == 0x0d ||opcodeB == 0x10 || opcodeB == 0x11 || opcodeB == 0x12 ||opcodeB == 0x14 ||opcodeB == 0x05||opcodeB == 0x0f  ){// instA = nop,b,beq,bgt,ret,cmp,st
    return 0 ;
   }
   unsigned int src1 =  (A & 0x3c0000)>>18 ;// [A].rs1
   unsigned int src2 =  (A & 0x3c000)>>14;     // [A].rs2

        if (opcodeA == 0x0f){ // if opcode of A is store
        src2 = (A & 0x3c00000)>>22;}

        if (opcodeA == 0x14){ // if opcode of A is ret
        src1 = R[15];}

        int dest = (B & 0x03c00000) >> 22 ; // [B.red
        if (opcodeB == 0x13){ // if opcode B is call{
        dest = R[15];                }
        int hasSrc2 = 1; // check the second operand if it is a register
         unsigned int immediate_bit_A =(A & 0x4000000) >> 26;
        if (opcodeA != 0x0f){  // A.opcode does not belongs to store
            if (immediate_bit_A == 1){ // immediate bit of A is 1 then hasSrc2 will be 0
                hasSrc2 = 0 ;
            }
        }
       // detecting conflicts
       if (src1 == dest){
        return 1 ;
       }
       if ((hasSrc2 == 1) && (src2 == dest)){
        return 1 ;
       }
       return 0 ;
}

int Data_lock_Unit(){
int checkOF_EX = dependency(IF_OF->instruction,OF_EX->instruction);
int checkOF_MA = dependency(IF_OF->instruction,EX_MA->instruction);
int checkOF_RW = dependency(IF_OF->instruction,MA_RW->instruction);

if (checkOF_EX == 1 || checkOF_MA == 1 || checkOF_RW == 1){
    return 1;
}
return 0 ;
}

int Branch_lock_unit(){
return isBranchTaken ;
}

int main() {
  //printf("yes\n");
  char* prog_mem_file;

//  if(argc < 2) {
//    printf("Incorrect number of arguments. Please invoke the simulator \n\t./mySimpleSim <input mem file> \n");
//    exit(1);
//  }
// printf("yes\n");
  //reset the processor
  reset_proc();
  //load the program memory
 //printf("yes\n");
  load_program_memory(simple_add.mem);
  //run the simulator
 //printf("yes\n");
  run_simplesim();
write_data_memory();
  return 1;
}
