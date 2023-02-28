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

const int MAX_MEMORY_SIZE = 2000;
const int END_PROGRAM = 50;
const int SYS_STACK_FLAG = -11;
const int USER_STACK_FLAG = -10;
const int WRITE_MEM_FLAG = -9;

void push_or_pop_stack(int fd1, int fd2, int *mem, int operation) {
    int value;
    int stack;
    read(fd2, &value, sizeof(value));
    stack = value;

    // if (operation == SYS_STACK_FLAG) {
    //     read(fd, &value, sizeof(value));
    //     mem[stack] = value;
    // } else {
    //     value = stack;
    // }
    //read(fd, &value, sizeof(value));
    //mem[stack] = value;
    //read(memoryToCpu[0], &PC, sizeof(PC));        FIRST READ INSIDE THE WHILE
    //push_or_pop_stack(cpuToMemory[1], memoryToCpu[0], mem, PC); THIS IS CALLING THE FUNCTIONS

    if (mem[stack] == -1) {  // Push
        read(fd2, &value, sizeof(value));
        mem[stack] = value;
    } else {  // Pop
        write(fd1, &mem[stack], sizeof(mem[stack]));         // NEEED TO WRITE TO cpuToMemory
        mem[stack] = -1;
    }
}


void write_to_memory(int fd, int *mem) {
    int stack, value;
    read(fd, &stack, sizeof(stack));
    read(fd, &value, sizeof(value));
    mem[stack] = value;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {                                                                            // check if the arguments are valid ----
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





    // **************** Change Start here ************************

    else if (pid == 0) {                    // child (memory) process
        close(cpuToMemory[0]);  // Close file descriptors
        close(memoryToCpu[1]);

        int mem[MAX_MEMORY_SIZE];  // User program stored in memory
        fill_n(mem, MAX_MEMORY_SIZE, -1);
        int i = 0;
        string line;
        while (getline(myFile, line)) {
            //cout << "printing from READING FILE in CHILD" << endl;
            if (isdigit(line[0])) {
                mem[i++] = stoi(line);
            } else if (line[0] == '.') {
                i = stoi(line.substr(1));
            }
        }


        // int PC, stack;
        // while (true)
        // {
        //     read(memoryToCpu[0], &PC, sizeof(PC));
        //     //cout << "printing from STACK ADJUSTMENT in CHILD" << endl;
        //     switch (PC) {
        //     case -11: // system stack
        //         read(memoryToCpu[0], &stack, sizeof(stack));

        //         if (mem[stack] == -1) { // push to system stack
        //             read(memoryToCpu[0], &PC, sizeof(PC));
        //             mem[stack] = PC;
        //         }
        //         else { // pop from system stack
        //             write(cpuToMemory[1], &mem[stack], sizeof(mem[stack]));
        //             mem[stack] = -1;
        //         }

        //         continue;

        //     case -10: // user stack
        //         read(memoryToCpu[0], &stack, sizeof(stack));

        //         if (mem[stack] == -1) { // push to user stack
        //             read(memoryToCpu[0], &PC, sizeof(PC));
        //             mem[stack] = PC;
        //         }
        //         else { // pop from user stack
        //             write(cpuToMemory[1], &mem[stack], sizeof(mem[stack]));
        //             mem[stack] = -1;
        //         }

        //         continue;

        //     case -9: // write to memory location
        //         read(memoryToCpu[0], &stack, sizeof(stack));
        //         read(memoryToCpu[0], &PC, sizeof(PC));
        //         mem[stack] = PC;
        //         continue;

        //     default: // execute instruction
        //         write(cpuToMemory[1], &mem[PC], sizeof(mem[PC]));

        //         if (mem[PC] == 50)
        //         {
        //             break;
        //         }
        //     }
        // }
        // ------------------------- WRONG SECTION ------------------
        int PC;
        int stack;
        while (true) {
            //cout << "printing from STACK ADJUSTMENT in CHILD" << endl;
            read(memoryToCpu[0], &PC, sizeof(PC));
            //cout << "PC: " << PC << endl;
            if (PC == SYS_STACK_FLAG || PC == USER_STACK_FLAG) {  // Access stack
                push_or_pop_stack(cpuToMemory[1], memoryToCpu[0], mem, PC);
                continue;
            }
            if (PC == WRITE_MEM_FLAG) {  // Write to memory location
                write_to_memory(memoryToCpu[0], mem);
                continue;
            }
            write(cpuToMemory[1], &mem[PC], sizeof(mem[PC]));
            if (mem[PC] == END_PROGRAM) {
                break;
            }
        }
        // ------------------------- WRONG SECTION ------------------

        _exit(0);

    }   // ----------------------------- end of child ------------------------------------



    else { // -------------------------- parent (cpu) process -------------------------------------
        close(cpuToMemory[1]);
        close(memoryToCpu[0]);

        int PC = 0, AC = 0, X = 0, Y = 0, IR = 0, SP = 1000, SS = 2000;
        int op = 0, randNum = 0, x = 0, in = -11, uin = -10, sev = -9;
        bool user = true, intr = false;
        int intrCount = 0, count = 0, userSP = 0;

        srand((unsigned int)time(NULL));

        while (IR != 50) {
            //cout << "printing from CPU" << endl;
            //cout << "PC: " << PC << endl;
            if (intr && count > 0 && (count % timer) == 0) {
                intrCount++;
            }

            if ((!intr && count > 0) && (count & timer == 0) || (!intr && intrCount)) {
                user = false;
                intr = true;
                intrCount--;

                userSP = SP;
                SP = SS;
                SP--;
                write(memoryToCpu[1], &in, sizeof(in));
                write(memoryToCpu[1], &SP, sizeof(SS));
                write(memoryToCpu[1], &userSP, sizeof(userSP));
                SP--;
                write(memoryToCpu[1], &in, sizeof(in));
                write(memoryToCpu[1], &SP, sizeof(SP));
                PC--;
                write(memoryToCpu[1], &PC, sizeof(PC));
                PC = 1000;
            }
            //cout << "PC BEFORE WRITE: " << PC << endl;
            write(memoryToCpu[1], &PC, sizeof(PC));
            //cout << "PC after WRITE: " << PC << endl;

            //cout << "----Timer----: " << timer << endl;

            read(cpuToMemory[0], &IR, sizeof(IR));
            //cout << "IR: " << IR << endl;
            count++;

            switch (IR) {
                case 1: // Load the value into the AC
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    AC = op;
                    break;

                case 2: // Load the value at the address into the AC
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));

                    if (op > 999 && user) {
                        cout << "Memory violation: accessing system address 1000 in user mode" << endl;
                    } else {
                        write(memoryToCpu[1], &op, sizeof(PC));
                        read(cpuToMemory[0], &op, sizeof(op));
                        AC = op;
                    }
                    break;

                case 3: // Load the value from the address found in the given address into the AC
                        // (for example, if LoadInd 500, and 500 contains 100, then load from 100).
                    break;

                case 4: // Load the value at (address+X) into the AC
                        // (for example, if LoadIdxX 500, and X contains 10, then load from 510).
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    x = op + X;

                    if (x > 999 && user) {
                        cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                    } else {
                        write(memoryToCpu[1], &x, sizeof(PC));
                        read(cpuToMemory[0], &op, sizeof(op));
                        AC = op;
                    }
                    break;

                case 5: // Load the value at (address+Y) into the AC
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    x = op + Y;
                    if (x > 999 && user)
                    {
                        cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                    }
                    else
                    {
                        write(memoryToCpu[1], &x, sizeof(PC));
                        read(cpuToMemory[0], &op, sizeof(op));
                        AC = op;
                    }
                    break;
                case 6: // Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991).
                    AC = SP + X;
                    write(memoryToCpu[1], &AC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    AC = op;
                    break;
                case 7: // Store the value in the AC into the address
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    write(memoryToCpu[1], &sev, sizeof(sev));
                    write(memoryToCpu[1], &op, sizeof(op));
                    write(memoryToCpu[1], &AC, sizeof(AC));
                    break;
                case 8: // Gets a random int from 1 to 100 into the AC
                    randNum = (rand() % 100) + 1;
                    AC = randNum;
                    cout << AC << endl;
                    break;
                case 9: // If port=1, writes AC as an int to the screen
                        // If port=2, writes AC as a char to the screen
                    // cout << "9: " << endl;
                    PC++;
                    // cout << "PC: " << PC << endl;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    // cout << op;
                    if (op == 1)
                        cout << AC;
                    else if (op == 2)
                        cout << char(AC);

                    break;
                case 10: // Add the value in X to the AC
                    AC += X;
                    break;
                case 11: // Add the value in Y to the AC
                    AC += Y;
                    break;
                case 12: // Subtract the value in X from the AC
                    AC -= X;
                    break;
                case 13: // Subtract the value in Y from the AC
                    AC -= Y;
                    break;
                case 14: // Copy the value in the AC to X
                    // cout << "14: " << endl;
                    X = AC;
                    break;
                case 15: // Copy the value in X to the AC
                    AC = X;
                    break;
                case 16: // Copy the value in the AC to Y
                    Y = AC;
                    break;
                case 17: // Copy the value in Y to the AC
                    AC = Y;
                    break;
                case 18: // Copy the value in AC to the SP
                    SP = AC;
                    break;
                case 19: // Copy the value in SP to the AC
                    AC = SP;
                    break;
                case 20: // Jump to the address
                    // cout << "20: " << endl;
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    PC = op - 1;
                    break;
                case 21: // Jump to the address only if the value in the AC is zero
                    // cout << "21: " << endl;
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));

                    if (AC == 0)
                        PC = op - 1;
                    break;
                case 22: // Jump to the address only if the value in the AC is not zero
                    PC++;
                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    if (AC != 0)
                        PC = op - 1;
                    break;
                case 23: // Push return address onto stack, jump to the address
                    PC++;
                    SP--;

                    write(memoryToCpu[1], &uin, sizeof(PC)); // -10
                    write(memoryToCpu[1], &SP, sizeof(SP)); 
                    write(memoryToCpu[1], &PC, sizeof(PC));

                    write(memoryToCpu[1], &PC, sizeof(PC));
                    read(cpuToMemory[0], &op, sizeof(op));
                    
                    PC = op - 1;
                    //cout << "PC in 23: " << PC << endl;
                    break;
                case 24: // Pop return address from the stack, jump to the address
                    write(memoryToCpu[1], &uin, sizeof(uin));
                    write(memoryToCpu[1], &SP, sizeof(SP));
                    //cout << "Did the writing ---------------------------" << endl;
                    read(cpuToMemory[0], &op, sizeof(op));
                    //cout << "DID THE READ" << endl;
                    PC = op;
                    SP++;
                    break;
                case 25: // Increment the value in X
                    X++;
                    break;
                case 26: // Decrement the value in X
                    X--;
                    break;
                case 27: // Push AC onto stack
                    SP--;
                    if (intr)
                        write(memoryToCpu[1], &in, sizeof(in));
                    else
                        write(memoryToCpu[1], &uin, sizeof(uin));
                    write(memoryToCpu[1], &SP, sizeof(SP));
                    write(memoryToCpu[1], &AC, sizeof(PC));
                    break;
                case 28: // Pop from stack into AC
                    if (intr)
                        write(memoryToCpu[1], &in, sizeof(in));
                    else
                        write(memoryToCpu[1], &uin, sizeof(uin));
                    write(memoryToCpu[1], &SP, sizeof(SP));
                    read(cpuToMemory[0], &op, sizeof(op));
                    SP++;
                    AC = op;
                    break;
                case 29: // Perform system call
                    if (!intr)
                    {

                        user = false;
                        intr = true;
                        userSP = SP;
                        SP = SS;
                        SP--;
                        write(memoryToCpu[1], &in, sizeof(PC));
                        write(memoryToCpu[1], &SP, sizeof(SP));
                        write(memoryToCpu[1], &userSP, sizeof(userSP));
                        SP--;
                        write(memoryToCpu[1], &in, sizeof(PC));
                        write(memoryToCpu[1], &SP, sizeof(SP));
                        write(memoryToCpu[1], &PC, sizeof(SP));
                        PC = 1499;
                    }
                    break;
                case 30: // Return from system call

                    user = true;
                    intr = false;
                    write(memoryToCpu[1], &in, sizeof(PC));
                    write(memoryToCpu[1], &SP, sizeof(SP));
                    read(cpuToMemory[0], &op, sizeof(op));
                    PC = op;
                    // cout << "PROGRAM COUNTER AFTER RETURN: " << PC << endl;

                    SP++;
                    write(memoryToCpu[1], &in, sizeof(PC));
                    write(memoryToCpu[1], &SP, sizeof(SP));
                    read(cpuToMemory[0], &op, sizeof(op));
                    SP = op;
                    // cout << "STACK POINTER AFTER RETURN: " << SP << endl;
                    break;
                default: // End execution
                    cout << endl; //<< "Invalid instruction" << endl;
                    break;
            } // end of switch 
            
            PC++;

        } // end of while
        
        waitpid(-1, NULL, 0);


    }   // end of parent

    exit(0);


}