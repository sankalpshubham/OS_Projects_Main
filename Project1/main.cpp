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

using namespace std;

const int MAX_MEMORY_SIZE = 2000;
const int END_PROGRAM = 50;
const int SYS_STACK_FLAG = -4;
const int USER_STACK_FLAG = -3;
const int WRITE_MEM_FLAG = -2;

void push_or_pop_stack(int fd1, int fd2, int *mem, int operation) {                 // push and pop the stack for read and write
    int value, stackPtr;
    read(fd2, &stackPtr, sizeof(stackPtr));

    if (mem[stackPtr] == -1) {                                                      // Pushing on stack
        read(fd2, &value, sizeof(value));
        mem[stackPtr] = value;
    } else {                                                                        // Poping the stack
        write(fd1, &mem[stackPtr], sizeof(mem[stackPtr]));
        mem[stackPtr] = -1;
    }
}


void write_to_memory(int fd, int *mem) {                                            // writing to memory
    int stack, value;
    read(fd, &stack, sizeof(stack));
    read(fd, &value, sizeof(value));
    mem[stack] = value;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {                                                                // check if we have three arguments passed in 
        cout << "ERROR: This program requires 2 arugments. Enter file name and timer interrupt." << endl;
        exit(1);
    }

    int timer = stoi(argv[2]);
    string fileName = argv[1];
    ifstream myFile(fileName);
    if (!myFile) {
        cout << "ERROR: File not found!" << endl;
        exit(1);
    }

    // setting up pipes for communication between child and parent process
    int memoryToCpu [2];
    int cpuToMemrory[2];

    if (pipe(memoryToCpu ) < 0 || pipe(cpuToMemrory) < 0) {                       // do a pipe and check if it failed
        write(STDERR_FILENO, "Pipe failed\n", 12);
        exit(1);
    }


    // start the fork 
    int pid = fork();
    if (pid == -1) {
        printf("The fork failed!");
        exit(-1);
    }

    else if (pid == 0) { // child (memory) process
        close(memoryToCpu [0]);                                                     // Close file descriptors
        close(cpuToMemrory[1]);

        int mem[MAX_MEMORY_SIZE];                                                   // memory setup
        fill_n(mem, MAX_MEMORY_SIZE, -1);
        int i = 0;
        string line;
        while (getline(myFile, line)) {                                             // read in the lines from files
            if (isdigit(line[0])) {
                mem[i++] = stoi(line);
            } else if (line[0] == '.') {
                i = stoi(line.substr(1));
            }
        }

        int PC;
        int stack;
        while (true) {                                                              // handle stack actions
            read(cpuToMemrory[0], &PC, sizeof(PC));
            if (PC == SYS_STACK_FLAG || PC == USER_STACK_FLAG) {                    // Access stack
                push_or_pop_stack(memoryToCpu [1], cpuToMemrory[0], mem, PC);
                continue;
            }
            if (PC == WRITE_MEM_FLAG) {                                             // Write to memory location
                write_to_memory(cpuToMemrory[0], mem);
                continue;
            }
            write(memoryToCpu [1], &mem[PC], sizeof(mem[PC]));
            if (mem[PC] == END_PROGRAM)
                break;
        }

        _exit(0);

    }   // end of child 



    else { // parent (cpu) process 
        close(memoryToCpu [1]);
        close(cpuToMemrory[0]);

        int PC = 0, AC = 0, X = 0, Y = 0, IR = 0, SP = 1000, SS = 2000;
        int op = 0, randNum = 0, x = 0;
        int flag1 = -4, flag2 = -3, flag3 = -2;
        bool user = true, intr = false;
        int intrCount = 0, count = 0, userSP = 0;

        srand((unsigned int)time(NULL));
        
        while (IR != 50) {                                                          // read instructions while we don't reach end of file
            if (intr && count > 0 && (count % timer) == 0) {
                intrCount++;
            }

            if ((!intr && count > 0) && (count & timer == 0) || (!intr && intrCount)) {     // check for interrupts and timer
                user = false;
                intr = true;
                intrCount--;

                userSP = SP;
                SP = SS;
                SP--;
                write(cpuToMemrory[1], &flag1, sizeof(flag1));
                write(cpuToMemrory[1], &SP, sizeof(SS));
                write(cpuToMemrory[1], &userSP, sizeof(userSP));
                SP--;
                write(cpuToMemrory[1], &flag1, sizeof(flag1));
                write(cpuToMemrory[1], &SP, sizeof(SP));
                PC--;
                write(cpuToMemrory[1], &PC, sizeof(PC));
                PC = 1000;
            }
            
            write(cpuToMemrory[1], &PC, sizeof(PC));
            //cout << "PC " << PC << endl;
            read(memoryToCpu [0], &IR, sizeof(IR));
            count++;
            
            switch (IR) {
                case 1:                                                                     // Load the value into the AC
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    AC = op;
                    break;

                case 2:                                                                     // Load the value at the address into the AC
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));

                    if (op > 999 && user) {
                        cout << "Memory violation: accessing system address 1000 in user mode" << endl;
                    } else {
                        write(cpuToMemrory[1], &op, sizeof(PC));
                        read(memoryToCpu [0], &op, sizeof(op));
                        AC = op;
                    }
                    break;

                case 3: // Load the value from the address found in the given address into the AC
                        // (for example, if LoadInd 500, and 500 contains 100, then load from 100). *********************
                        // WRITE CASE # HERE
                        PC++;
                        write(cpuToMemrory[1], &PC, sizeof(PC));
                        read(memoryToCpu [0], &op, sizeof(op));

                        if (op > 999 && user) {
                            cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                        } else {
                            write(cpuToMemrory[1], &op, sizeof(op));
                            read(memoryToCpu [0], &op, sizeof(op));

                            if (op > 999 && user)
                                cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                            else {
                                // write(cpuToMemrory[1], &op, sizeof(PC));
                                // read(memoryToCpu [0], &op, sizeof(op));
                                AC = op;
                            }
                        }
                        break;

                case 4:                                                                     // Load the value at (address+X) into the AC
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    x = op + X;

                    if (x > 999 && user) {
                        cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                    } else {
                        write(cpuToMemrory[1], &x, sizeof(PC));
                        read(memoryToCpu [0], &op, sizeof(op));
                        AC = op;
                    }
                    break;

                case 5:                                                                     // Load the value at (address+Y) into the AC
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    x = op + Y;
                    if (x > 999 && user) {
                        cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                    }
                    else {
                        write(cpuToMemrory[1], &x, sizeof(PC));
                        read(memoryToCpu [0], &op, sizeof(op));
                        AC = op;
                    }
                    break;

                case 6:                                                                     // Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991).
                    AC = SP + X;
                    write(cpuToMemrory[1], &AC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    AC = op;
                    break;

                case 7:                                                                     // Store the value in the AC into the address
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    write(cpuToMemrory[1], &flag3, sizeof(flag3));
                    write(cpuToMemrory[1], &op, sizeof(op));
                    write(cpuToMemrory[1], &AC, sizeof(AC));
                    break;

                case 8:                                                                     // Gets a random int from 1 to 100 into the AC
                    randNum = (rand() % 100) + 1;
                    AC = randNum;
                    cout << AC << endl;
                    break;

                case 9:                                                                     // If port=1, writes AC as an int to the screen. If port=2, writes AC as a char to the screen
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    
                    if (op == 1)
                        cout << AC;
                    else if (op == 2)
                        cout << char(AC);
                    break;

                case 10:                                                                    // Add the value in X to the AC
                    AC += X;
                    break;

                case 11:                                                                    // Add the value in Y to the AC
                    AC += Y;
                    break;

                case 12:                                                                    // Subtract the value in X from the AC
                    AC -= X;
                    break;

                case 13:                                                                    // Subtract the value in Y from the AC
                    AC -= Y;
                    break;

                case 14:                                                                    // Copy the value in the AC to X
                    X = AC;
                    break;
                    
                case 15:                                                                    // Copy the value in X to the AC
                    AC = X;
                    break;

                case 16:                                                                    // Copy the value in the AC to Y
                    Y = AC;
                    break;

                case 17:                                                                    // Copy the value in Y to the AC
                    AC = Y;
                    break;

                case 18:                                                                    // Copy the value in AC to the SP
                    SP = AC;
                    break;

                case 19:                                                                    // Copy the value in SP to the AC
                    AC = SP;
                    break;

                case 20:                                                                    // Jump to the address
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    PC = op - 1;
                    break;

                case 21:                                                                    // Jump to the address only if the value in the AC is zero
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));

                    if (AC == 0)
                        PC = op - 1;
                    break;

                case 22:                                                                    // Jump to the address only if the value in the AC is not zero
                    PC++;
                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    if (AC != 0)
                        PC = op - 1;
                    break;

                case 23:                                                                    // Push return address onto stack, jump to the address
                    PC++;
                    SP--;

                    write(cpuToMemrory[1], &flag2, sizeof(PC));
                    write(cpuToMemrory[1], &SP, sizeof(SP)); 
                    write(cpuToMemrory[1], &PC, sizeof(PC));

                    write(cpuToMemrory[1], &PC, sizeof(PC));
                    read(memoryToCpu [0], &op, sizeof(op));
                    
                    PC = op - 1;
                    break;

                case 24:                                                                    // Pop return address from the stack, jump to the address
                    write(cpuToMemrory[1], &flag2, sizeof(flag2));
                    write(cpuToMemrory[1], &SP, sizeof(SP));
                    read(memoryToCpu [0], &op, sizeof(op));

                    PC = op;
                    SP++;
                    break;

                case 25:                                                                    // Increment the value in X
                    X++;
                    break;

                case 26:                                                                    // Decrement the value in X
                    X--;
                    break;

                case 27:                                                                    // Push AC onto stack
                    SP--;
                    if (intr)
                        write(cpuToMemrory[1], &flag1, sizeof(flag1));
                    else
                        write(cpuToMemrory[1], &flag2, sizeof(flag2));
                    write(cpuToMemrory[1], &SP, sizeof(SP));
                    write(cpuToMemrory[1], &AC, sizeof(PC));
                    break;

                case 28:                                                                    // Pop from stack into AC
                    if (intr)
                        write(cpuToMemrory[1], &flag1, sizeof(flag1));
                    else
                        write(cpuToMemrory[1], &flag2, sizeof(flag2));
                    write(cpuToMemrory[1], &SP, sizeof(SP));
                    read(memoryToCpu [0], &op, sizeof(op));
                    SP++;
                    AC = op;
                    break;

                case 29:                                                                    // Perform system call
                    if (!intr) {
                        user = false;
                        intr = true;
                        userSP = SP;
                        SP = SS;

                        SP--;
                        write(cpuToMemrory[1], &flag1, sizeof(PC));
                        write(cpuToMemrory[1], &SP, sizeof(SP));
                        write(cpuToMemrory[1], &userSP, sizeof(userSP));
                        SP--;
                        write(cpuToMemrory[1], &flag1, sizeof(PC));
                        write(cpuToMemrory[1], &SP, sizeof(SP));
                        write(cpuToMemrory[1], &PC, sizeof(SP));
                        PC = 1499;
                    }
                    break;

                case 30:                                                                    // Return from system call
                    user = true;
                    intr = false;
                    write(cpuToMemrory[1], &flag1, sizeof(PC));
                    write(cpuToMemrory[1], &SP, sizeof(SP));
                    read(memoryToCpu [0], &op, sizeof(op));
                    PC = op;

                    SP++;
                    write(cpuToMemrory[1], &flag1, sizeof(PC));
                    write(cpuToMemrory[1], &SP, sizeof(SP));
                    read(memoryToCpu [0], &op, sizeof(op));
                    SP = op;
                    break;

                default:                                                                    // End execution
                    cout << endl;
                    break;

            } // end of switch 
            PC++;
        } // end of while
        
        waitpid(-1, NULL, 0);
    }   // end of parent

    exit(0);
}