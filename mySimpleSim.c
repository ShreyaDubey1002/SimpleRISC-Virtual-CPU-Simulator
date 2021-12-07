
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

#include "mySimpleSim.h"
#include <stdlib.h>
#include <stdio.h>

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
static int inst = 0;
 static int opcode = 0;
static unsigned mar = 0,mdr = 0;
//flags
static int gt = 0,eq = 0;
//memory
static unsigned char MEM[4000];

//intermediate data path and control path signals
static unsigned int instruction_word;
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

void run_simplesim() {
 while(inst != 0xffffffff) {
  //while(0){
  int i ;
// for (i = 0 ; i < 17 ; i++)  {
  fetch();
   if(opcode==0x1f)
   break;

    decode();
    execute();
    mem();
    write_back();
 }
}
// it is used to set the reset values
//reset all registers and memory content to 0
void reset_proc() {
    int i ;
    for (i = 0 ; i < 16 ; i++){
        R[i] = 0 ;
    }
    for (i = 0 ; i<4000 ;i++){
        MEM[i] = '0' ; // check one more time
    }
   R[14] = 2000;
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
    if (isBranchTaken == 1){
    PC = branchPC ; // check once
   }
    isBranchTaken = 0 ;
inst  = read_word(MEM,PC);
printf("FETCH UNIT :\n");
printf("\n");
printf("Instruction Read: 0x%x\n",inst);
opcode = (inst & 0xf8000000) >> 27 ;
if(inst == 0xffffffff){
printf("EXIT\n");
}
printf("PC= %x\n",PC);
printf("\n");
}
//reads the instruction register, reads operand1, operand2 from register file, decides the operation to be performed in execute stage
void decode() {
    int i=PC;
     // extracting opcode
    // there are 3 cases
   printf("DECODE UNIT : \n");
   printf("\n");
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
       offset=inst&0x07ffffff;
      if (((inst & 0x04000000) >> 26) == 1)
       offset = offset | 0xf8000000 ;
       
        offset = (offset << 2 );
       printf("offset %d\n",offset);
        branchTarget=PC+offset;
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
           immx=inst& 0x0000ffff;
        immediate_bit=(inst& 0x4000000) >> 26;
        if(immediate_bit == 1){
             
            modifier_bit = (inst&0x30000)>>16;
            if(modifier_bit==0)  //signed extension
               {
                  // immx=immx<<16;
                  // immx=immx>>16;
                // looking 16 th bit & extending
               if (((inst & 0x00008000) >> 15 ) == 1){
                     immx = immx | 0xffff0000 ;
               }

               }
           else if (modifier_bit == 0x2){ // 'h' check one more time `
            immx=immx<<16;
            }  // 'u' for 'u'
            }
         }
          if (isRet == 1){
            operand1=R[15];
            }
        else{
        index_register = (inst & 0x3c0000)>>18 ;
        operand1 =  R[index_register];
 	printf("index_reg = %d\n",index_register);
        }
        if (isSt == 1){
          index_register2 = (inst & 0x3c00000)>>22;
        operand2 = R[index_register2];
        }
        else {
           index_register2 = (inst & 0x3c000)>>14;
         operand2 = R[index_register2];
        }
        if (opcode == 0x05 ){// instruction is compare
         isWb = 0 ;
        }
        printf("Immx %x\n",immx);
        printf("Operand 1: %d\n",operand1);
        printf("Operand 2: %d\n",operand2);
        printf("\n");
        }
    // now reading the register files
   // printf("%d\n",immx);

//executes the ALU operation based on ALUop
void execute() {
    //calculating branch unit
    printf("EXECUTE UNIT \n");
    printf("\n");
    if (isRet == 1){
    branchPC= operand1;
    }
    else
    branchPC=branchTarget;

    if((isBeq==1 && eq==1) || (isBgt==1 && gt==1) || opcode == 0x12 || opcode == 0x13 || opcode == 0x14)
    {
    isBranchTaken = 1 ;
    }
    if(immediate_bit==1)
    {
    op2=immx;
    }
    else
    op2=operand2;
//ALU:---
   gt = 0 ;
   eq = 0 ;
    if(opcode==0x00){//Addition
printf("instruction : ADDITION\n");
    aluResult = operand1 + op2;
}
    else if(opcode==0x01){//Subtraction
printf("instruction : SUBTRACTION\n");
    aluResult = operand1 - op2;
       // printf("aluResult_subs %d\n",aluResult);
       //     printf("operand1 %d\n",operand1);
       }

    else if(opcode==0x05)//Compare
    {
printf("instruction : COMPARE\n");
    aluResult = operand1 - op2;

    if(aluResult>0)
    gt=1;

    else if(aluResult==0)
    eq=1;

}
else if(opcode==0x02){//Multiplication
printf("instruction : MULTIPLICATION\n");
 aluResult = operand1 * op2;
}
else if(opcode==0x03){//Division
printf("instruction : DVISION\n");
 aluResult = operand1 / op2;
}
else if(opcode==0x04){//Modulus
printf("instruction : MODULUS\n");
 aluResult = operand1 % op2;
}
  else if(opcode==0x0a){//LSL
printf("instruction : LSL\n");
 aluResult = operand1 << op2;
}
  else if(opcode==0x0b){//LSR
printf("instruction : LSR\n");
 aluResult = (int)((unsigned int)operand1 >> op2);
}
  else if(opcode==0x0c){//ASR
printf("instruction : ASR\n");
 aluResult = operand1 >> op2;
}
  else if(opcode==0x07){//OR
printf("instruction : OR\n");
 aluResult = operand1 | op2;
}
  else if(opcode==0x08){//NOT
printf("instruction : NOT\n");
 aluResult = ~ operand1;
}
  else if(opcode==0x06){//AND
 printf("instruction : AND\n");
 aluResult = operand1 & op2;
}
  else if(opcode==0x09){//MOV{
printf("instruction : MOV\n");
  aluResult=op2;//check once
  printf("MOV Result %d\n",op2);
}
 else if(opcode==0x0e){//LD
   // printf("Enter1\n");
printf("instruction : LD\n");
    isLd = 1 ;
   aluResult=(operand1+immx);
  //  printf("&R[index_register] = %d\n",&R[index_register]);
   //printf("alu Result = %d\n",aluResult);
  //  printf("immx = %d\n",immx);

}
   else if(opcode==0x0f){//ST
printf("instruction : ST\n");
       isSt = 1;
       printf("immx %x\n",immx);
  aluResult=(operand1+immx);

   }
  // printf("index = %d\n",index_register);
 printf("AluResult : %d\n" , aluResult);
 printf("isBranchTaken : %d\n" , isBranchTaken);
 printf("\n");
 }
//perform the memory operation
void mem() {
   //  printf(" opcode = %d\n",opcode);
     printf("MEM UNIT\n");
     printf("\n");
    if((opcode==0x0e)||(opcode==0x0f))//if isLd or isSt
        {
        printf("operand 1 %d\n",operand1);
        mdr=operand1;
        mar=aluResult;
        if (opcode==0x0e){//Load
            // printf("Enter2\n");
            isLd = 1 ;

           // int *p ;
           // p = (int *)aluResult ;
            // printf("alu_load %x\n",aluResult);
            // mdr = *p ;
             // printf("*p %x\n",*p);
          // ldResult = mdr ;
             ldResult  = read_word(MEM, aluResult);    
          printf("ld Result : %x\n",ldResult);
           }
        else{
            write_word(MEM,aluResult,operand2) ;
             mdr = operand2 ;
           // int *p ;
          //  p = (int *)aluResult ;
          //  *p=mdr;
         // printf("mdr is %d\n",mdr);
         // printf("*p is %d\n",*p);
          //printf("data is %d\n",);
        }

    }
    else{
        printf("No Change\n");
    }

  printf("\n");
}
//writes the results back to register file
void write_back() {
//printf("isLd=%d\n",isLd);
//printf("isSt=%d\n",isSt);
    printf("WRITE BACK UNIT :\n");
    printf("\n");
   int index_destination;
  //  printf("write %d\n ",isWb);
    if (isWb == 1){
  if (isCall == 1){
    R[15] = PC + 0x4 ;
     printf("R[15] =  %d\n ",R[15]);
    }
  else {
       index_destination = (inst & 0x03c00000) >> 22 ;
        //printf("index %d\n ",index_destination);
        //printf("aluResult %d\n",isLd);
        if (isLd == 1){
            R[index_destination] = ldResult ;
          printf("ldResult = %d\n",ldResult);
           isLd = 0 ;
          }
         else {
            R[index_destination] = aluResult ;
         }
 printf("R[%d] =  %d\n ",index_destination,R[index_destination]);
  }
 
  }
      display_control_Signals();
    if (isBranchTaken == 0){
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
