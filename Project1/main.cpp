#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <stack>
#include <time.h>
#include <sstream>
#include <string>
#include <algorithm>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {                                                                            // check if the arguments are valid ----
        cout << "ERROR: This program requires 2 arugments. Enter file name and timer interrupt." << endl;
        exit(1);
    }

    // DELETE
    // store the filename and interrupt timer inputs
    // fileName = argv[1];
    // timer = stoi(argv[2]);
    // cout << fileName << endl << timer << endl;                                  // checking if the values are read in. (DELETE LATER !!!)
    //Seed for random number

	srand(time(0));                                 // CAN THIS BE PLACED SOMEWHERE ELSE ????
    string fileName = argv[1];;
    ifstream myFile(fileName);
    if (!myFile) {
        cout << "ERROR: File not found!" << endl;
        exit(1);
    }


    // setting up pipes for communication between child and parent process
    int cpuToMemory[2];                                                         // parent communication
    int memoryToCpu[2];                                                         // child commnunication

    if (pipe(cpuToMemory) < 0 || pipe(memoryToCpu) < 0) {                       // do a pipe and check if it failed
        write(STDERR_FILENO, "Pipe failed\n", 12);
        exit(1);
    }


    // start the fork -------------------------
    int pid = fork();
    if (pid == -1) {
        printf("The fork failed!");
        exit(-1);
    }

    else if (pid == 0) {                    // child (memory) process
        int memory[2000];                                                       // set up memory
        int userPrgm = 0;                                                       // pointer to user and system program
        int sysPrgm = 1000;
        int currPtr = userPrgm;

        string line;                                                            // read the lines of the file in the memory
        while (getline(myFile, line)) {
            if (line == "")
                continue;
            
            int currChar = 0;
            char instruction[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};

            if (line[0] == '.') {                                               // processing a jump to address command
                instruction[0] = '.';
                currChar++;
            }

            while (isdigit(line[currChar])) {                                   // get the instruction number
                instruction[currChar] = line[currChar];
                currChar++;
            }
            
            if (instruction[0] == '.') {                                        // IF THIS rotate DOEST WORK THEN TRY THE FOR LOOP INSTEAD !!!!!
                rotate(instruction, instruction + 1, instruction + 6);          // removing the '.' from the array to process the command
                instruction[5] = '\0';
                
            } else if (isdigit(instruction[0])) {                               // process user command
                memory[currPtr] = atoi(instruction);
                currPtr++;
            }
            
        }

        // processing CPU read/write requests
        while (true) {
            char input[6] = {'\0', '\0', '\0', '\0', '\0', '\0'};

            read(cpuToMemory[0], &input, 5);                                    // get instruction from CPU
            char instr = input[0];

            // rotate(input, input + 1, input + 6);                            // MIGHT NEED TO CHANGE TO THE FOR LOOP TO GET THE INST !!!
            // input[5] = '\0';
            for (int i = 0; i < 5; i++)
				input[i] = input[i + 1];

            if (instr == 'r') {                                                 // process the read command
                char convertToStr[5];
                snprintf(convertToStr, 5, "%d", memory[atoi(input)]);
                write(memoryToCpu[1], &convertToStr, 4);
            }

            if (instr == 'w') {
                char writeValue[5];
                read(cpuToMemory[0], &writeValue, 4);
                memory[atoi(input)] = atoi(writeValue);
            }

            if (instr == 'e')
                exit(0);
            
        }

    }   // ----------------------------- end of child ------------------------------------


    else { // -------------------------- parent (cpu) process -------------------------------------
        int PC, SP, IR, AC, X, Y;
        int timer = atoi(argv[2]);

        // stack pointers for user and kernel 
        int userStackPtr = 999;
        int sysStackPtr = 1999;

        // setting up interrupts and stack pointers
        int instCount = 0;
        bool user = true;
        bool interrupt = false;
        int numInterrupts = 0;
        PC = 0;
        SP = userStackPtr;

        while (true) {
            char instruction[5] = {'\0', '\0', '\0', '\0', '\0'};                                       // storing instructions
            char buffer[10] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};             // sending read/write commands
            bool jump = false;                                                                          // check for jump instruction

            if (interrupt && instCount > 0 && (instCount % timer) == 0) {                               // checking for multiple interrupts
                numInterrupts++;
            }

            if ((!interrupt && instCount > 0 && (instCount % timer) == 0) || (!interrupt && numInterrupts)) {       // timer interrupt check
                user = false;
                interrupt = true;
                numInterrupts--;

                int userSP = SP;                                                                        // saving user SP and PC to proceed with the interrupt and changing values
                int userPC = PC;
                SP = sysStackPtr;
                PC = 1000;
                jump = true;

				snprintf(buffer, 10, "w%d", SP);                                                        // writing the user SP into memory
				write(cpuToMemory[1], &buffer, 5);
				snprintf(buffer, 10, "%d", userSP);
				write(cpuToMemory[1], &buffer, 4);
				SP--;

				snprintf(buffer, 10, "w%d", SP);                                                        // writing the user PC into memory
				write(cpuToMemory[1], &buffer, 5);
				snprintf(buffer, 10, "%d", userPC);
				write(cpuToMemory[1], &buffer, 4);
				SP--;

				continue;
            }
            
            
            snprintf(buffer, 10, "r%d", PC);                                                        // get next instruction from memory
			write(cpuToMemory[1], &buffer, 5);
			read(memoryToCpu[0], &instruction, 4);
			instCount++; 

            if (instruction[0] == '\0') {                                                           // if didn't get the instruction
                continue;
            }
            
            IR = atoi(instruction);
                        // THIS IS INSIDE EVERY SWITCH BUT IM TRYING OUTSIDE TO SEE IF IT MAKES A DIFFERENCE !!!!!!*****************************8


            // create multiple switch case statements for CPU processing
            switch (IR) {
                case 1: {                                                                            // Load the value into the AC
                    char readFromMem[5];
					PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);
					AC = atoi(readFromMem);
                }
                case 2: {                                                                            // Load the value at the address into the AC
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);                                    // get next line                   
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					if (atoi(readFromMem) > 999 && user)                                // checking if memory out of bounds
						printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
					else {
						snprintf(buffer, 10, "r%d", atoi(readFromMem));
						write(cpuToMemory[1], &buffer, 5);
						read(memoryToCpu[0], &readFromMem, 4);
						AC = atoi(readFromMem);
					}
                }
                case 3: {                                                                             // Load the value from the address found in the given address into the AC
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);                                    // reading next line
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);
                    
					if (atoi(readFromMem) > 999 && user)                                // checking if memory out of bounds
						printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
					else {
						snprintf(buffer, 10, "r%d", atoi(readFromMem));
						write(cpuToMemory[1], &buffer, 5);
						read(memoryToCpu[0], &readFromMem, 4);
						AC = atoi(readFromMem);

						if (atoi(readFromMem) > 999 && user)
							printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
						else {
							snprintf(buffer, 10, "r%d", AC);                            // loading AC address
							write(cpuToMemory[1], &buffer, 5);
							read(memoryToCpu[0], &readFromMem, 4);
							AC = atoi(readFromMem);
						}
					}
                }
                case 4: {                                                                             // Load the value at (address+X) into the AC
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);                                    // get next line
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					if (X + atoi(readFromMem) > 999 && user)
						printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
					else {                                                              // load the new value in AC
						snprintf(buffer, 10, "r%d", X + atoi(readFromMem));
						write(cpuToMemory[1], &buffer, 5);
						read(memoryToCpu[0], &readFromMem, 4);
						AC = atoi(readFromMem);
					}
                }
                case 5: {                                                                            // Load the value at (address+Y) into the AC
                    char readFromMem[5];
                    PC++;
                    snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					if (Y + atoi(readFromMem) > 999 && user)
						printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
					else {                                                              // load new value in AC
						snprintf(buffer, 10, "r%d", Y + atoi(readFromMem));
						write(cpuToMemory[1], &buffer, 5);
						read(memoryToCpu[0], &readFromMem, 4);
						AC = atoi(readFromMem);
					}
                }
                case 6: {                                                                            // Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991)
                    char readFromMem[5];    
                    if (X + SP > 999 && !interrupt)
						printf("Memory violation: accessing system address %d in user mode.\n", atoi(readFromMem));
					else {                                                              // load new value in AC
						snprintf(buffer, 10, "r%d", SP + 1 + X);
						write(cpuToMemory[1], &buffer, 5);
						read(memoryToCpu[0], &readFromMem, 4);
						AC = atoi(readFromMem);
					}
                }
                case 7: {                                                                            // Store the value in the AC into the address
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					snprintf(buffer, 10, "w%d", atoi(readFromMem));                                 // write ac into memory
					write(cpuToMemory[1], &buffer, 5);
					snprintf(buffer, 10, "%d", AC);
					write(cpuToMemory[1], &buffer, 4);
                }
                case 8:                                                                            // Gets a random int from 1 to 100 into the AC
                    AC = (rand() % 100) + 1;
                    cout << AC << endl;
                
                case 9: {                                                                            // If port=1, writes AC as an int to the screen. If port=2, writes AC as a char to the screen
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					if (atoi(readFromMem) == 1)                                         // check port value
                        cout << AC << endl;
						//printf("%d", AC);
					if (atoi(readFromMem) == 2)
                        cout << char(AC) << endl;
						//printf("%c", AC);
                }
                case 10:                                                                            // Add the value in X to the AC
                    AC += X;
                
                case 11:                                                                            // Add the value in Y to the AC
                    AC += Y;
                
                case 12:                                // Subtract the value in X from the AC
                    AC -= X;
                
                case 13:                                // Subtract the value in Y from the AC
                    AC -= Y;
                
                case 14:                                // Copy the value in the AC to X
                    X = AC;
                
                case 15:                                // Copy the value in X to the AC
                    AC = X;
                
                case 16:                                // Copy the value in the AC to Y
                    Y = AC;
                
                case 17:                                // Copy the value in Y to the AC
                    AC = Y;
                
                case 18:                                // Copy the value in AC to the SP
                    SP = AC;
                
                case 19:                                // Copy the value in SP to the AC 
                    AC = SP;
                
                case 20: {                               // Jump to the address
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					PC = atoi(readFromMem);
					jump = true;
                }
                case 21: {                                                                            // Jump to the address only if the value in the AC is zero
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);
                    
					if (AC == 0) {                       // changing PC value as per current value
						PC = atoi(readFromMem);
						jump = true;
					}
                }
                case 22: {                                                                            // Jump to the address only if the value in the AC is not zero
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					if (AC != 0) {                      // changing PC value as per current value
						PC = atoi(readFromMem);
						jump = true;
					}
                }
                case 23: {                                                                            // Push return address onto stack, jump to the address
                    char readFromMem[5];
                    PC++;
					snprintf(buffer, 10, "r%d", PC);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					snprintf(buffer, 10, "w%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					snprintf(buffer, 10, "%d", PC + 1);
					write(cpuToMemory[1], &buffer, 4);
					
                    SP--;
					PC = atoi(readFromMem);
					jump = true;
                }
                case 24: {                                                                            // Pop return address from the stack, jump to the address
                    char readFromMem[5];
                    SP++;
					snprintf(buffer, 10, "r%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					PC = atoi(readFromMem);
					jump = true;
                }
                case 25:                                                                            // Increment the value in X
                    X++;
                
                case 26:                                                                            // Decrement the value in X
                    X--;
                
                case 27: {                                                                            // Push AC onto stack
					snprintf(buffer, 10, "w%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					snprintf(buffer, 10, "%d", AC);
					write(cpuToMemory[1], &buffer, 4);
					SP--;
                }
                case 28: {                                                                            // Pop from stack into AC
                    char readFromMem[5];
                    SP++;
					snprintf(buffer, 10, "r%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);
					AC = atoi(readFromMem);
                }
                case 29: {                                                                            // Perform system call
                    if (interrupt) {
                        break;
                    }
					user = false;                                       // setting kernel/interrupt mode
					interrupt = true;   

					int userSP = SP;                                    // preserve values and write to stack
					int userPC = PC + 1;
					SP = sysStackPtr;
					PC = 1500;
					jump = true;
					
					snprintf(buffer, 10, "w%d", SP);                    // write user sp in memory at sp
					write(cpuToMemory[1], &buffer, 5);
					snprintf(buffer, 10, "%d", userSP);
					write(cpuToMemory[1], &buffer, 4);
					SP--;

					snprintf(buffer, 10, "w%d", SP);                    // write user pc in memory at sp
					write(cpuToMemory[1], &buffer, 5);
					snprintf(buffer, 10, "%d", userPC);
					write(cpuToMemory[1], &buffer, 4);
					SP--;
                }
                case 30: {                                                                            // Return from system call
                    char readFromMem[5];
                    user = true;
					interrupt = false;
					SP++;

					snprintf(buffer, 10, "r%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);

					PC = atoi(readFromMem);                             // read the jump into PC
					jump = true;
					SP++;

					snprintf(buffer, 10, "r%d", SP);
					write(cpuToMemory[1], &buffer, 5);
					read(memoryToCpu[0], &readFromMem, 4);
					SP = atoi(readFromMem);                             // read the jump into SP
                }
                case 50: {                                                                          // End execution
                    
                    write(cpuToMemory[1], "e", 5);                      // close all pipes (communications) before exiting
					close(cpuToMemory[0]);
					close(cpuToMemory[1]);
					close(memoryToCpu[0]);
					close(memoryToCpu[1]);

                    cout << "Program executed successfully" << endl;


                    exit(0);
                }

                default: {                                                                           // ERROR because some invalid IR
                    cout << "ERROR. Invalid IR" << endl;
                    exit(1);
                }
            }   // end of switch cases

            if (!jump)
				PC++;


        }       // end of while loop for executing instructions


    }   // end of parent




}