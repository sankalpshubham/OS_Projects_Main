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
    // check if the arguments are valid ----
    if (argc != 3) {                
        cout << "ERROR: This program requires 2 arugments. Enter file name and timer interrupt." << endl;
        exit(1);
    }

    // DELETE
    // store the filename and interrupt timer inputs
    // fileName = argv[1];
    // timer = stoi(argv[2]);
    // cout << fileName << endl << timer << endl;                                  // checking if the values are read in. (DELETE LATER !!!)
    string fileName;
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
    pid_t pid = fork();
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

            if (instruction[0] == '.') {                                        // IF THIS rotate DOEST WORK THEN TRY THE FOR LOOP INSTEAD
                rotate(instruction, instruction + 1, instruction + 6);          // removing the '.' from the array to process the command
                instruction[5] = '\0';
                currPtr = atoi(instruction);
                cout << "Error occured here if there was one" << endl;                          // DELETE LATER !!!
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

            rotate(input, input + 1, input + 6);                            // MIGHT NEED TO CHANGE TO THE FOR LOOP TO GET THE INST !!!
            input[5] = '\0';

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

    }   // ----------- end of child ------------


    else {                              // parent (cpu) process
        int PC, SP, IR, AC, X, Y;
        int timer = atoi(argv[2]);

        


        // create multiple switch case statements for CPU processing
        switch (true) {
            case 1:                                 // Load the value into the AC
                // something
            
            case 2:                                 // Load the value at the address into the AC
                // something
            
            case 3:                                 // Load the value from the address found in the given address into the AC
                // something
            
            case 4:                                 // Load the value at (address+X) into the AC
                // something
            
            case 5:                                 // Load the value at (address+Y) into the AC
                // something
            
            case 6:                                 // Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991)
                // something
            
            case 7:                                 // Store the value in the AC into the address
                // something
            
            case 8:                                 // Gets a random int from 1 to 100 into the AC
                // something
            
            case 9:                                 // If port=1, writes AC as an int to the screen. If port=2, writes AC as a char to the screen
                // something
            
            case 10:                                // Add the value in X to the AC
                // something
            
            case 11:                                // Add the value in Y to the AC
                // something
            
            case 12:                                // Subtract the value in X from the AC
                // something
            
            case 13:                                // Subtract the value in Y from the AC
                // something
            
            case 14:                                // Copy the value in the AC to X
                // something
            
            case 15:                                // Copy the value in X to the AC
                // something
            
            case 16:                                // Copy the value in the AC to Y
                // something
            
            case 17:                                // Copy the value in Y to the AC
                // something
            
            case 18:                                // Copy the value in AC to the SP
                // something
            
            case 19:                                // Copy the value in SP to the AC 
                // something
            
            case 20:                                // Jump to the address
                // something
            
            case 21:                                // Jump to the address only if the value in the AC is zero
                // something
            
            case 22:                                // Jump to the address only if the value in the AC is not zero
                // something
            
            case 23:                                // Push return address onto stack, jump to the address
                // something
            
            case 24:                                // Pop return address from the stack, jump to the address
                // something
            
            case 25:                                // Increment the value in X
                // something
            
            case 26:                                // Decrement the value in X
                // something
            
            case 27:                                // Push AC onto stack
                // something
            
            case 28:                                // Pop from stack into AC
                // something
            
            case 29:                                // Perform system call
                // something
            
            case 30:                                // Return from system call
                // something
            
            case 50:                                // End execution
                cout << "Program executed successfully" << endl;
                exit(0);
            
            default:                                // ERROR because some invalid IR
                cout << "ERROR. Invalid IR" << endl;
                exit(1);
        }



        cout << "PARENT: read \"" << buf << "\"\n";
        waitpid(-1, NULL, 0);
    }




}