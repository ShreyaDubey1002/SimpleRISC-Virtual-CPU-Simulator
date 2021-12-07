/*

The project is developed as part of Computer Architecture class
Project Name: Functional/Pipeline Simulator for SimpleRISC Processor

Developer's Name: Shreya Dubey and Piyush Jain
Developer's Email id:2015csb1074@iitrpr.ac.in and 2015csb1023@iitrpr.ac.in
Date:

*/

/* main.cpp
   Purpose of this file: The file handles the input and output, and
   invokes the simulator
*/

#include "mySimpleSim.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  //printf("yes\n");
  char* prog_mem_file;
  
  if(argc < 2) {
    printf("Incorrect number of arguments. Please invoke the simulator \n\t./mySimpleSim <input mem file> \n");
    exit(1);
  }
// printf("yes\n");
  //reset the processor
  reset_proc();
  //load the program memory
 //printf("yes\n");
  load_program_memory(argv[1]);
  //run the simulator
 //printf("yes\n");
  run_simplesim();
write_data_memory();
  return 1;
}
