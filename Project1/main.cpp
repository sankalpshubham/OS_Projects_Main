#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <stack>

using namespace std;

int main(int argc, char **argv) {

    string fileName;
    int timer;

    // check if the arguments are valid ----
    if (argc == 1) {                
        cout << "ERROR: No arguments inputed." << endl;
        exit(1);
    } else {
        fileName = argv[1];
        timer = stoi(argv[2]);
        cout << fileName << endl<< timer << endl;
    }
    
    char buf[30];
    pid_t pid;

    int fd1[2];                                     // two way communication using pipes
    int fd2[2];

    if (pipe(fd1) < 0 || pipe(fd2) < 0) {           // do a pipe and check if failed
        write(STDERR_FILENO, "Pipe failed\n", 12);
        exit(1);
    }

    // start the fork -----
    pid = fork();
    if (pid == -1) {
        printf("The fork failed!");
        exit(-1);
    }
    else if (pid == 0) {                            // child process
        int num;
        string x;
        ifstream myFile(fileName);
        if (!myFile)
            cout << "File not found!" << endl;

        int memory[2000];
        int i = 0;                                  // user programs needs to be stored from 0 - 999
        while (getline(myFile, x)) {
            if (x == "")
                continue;
            num = stoi(x);
            memory[i] = num;
            //cout << i << endl;
            cout << memory[i] << "\n";
            //cout << "-------" << endl;
            i++;
        }

        write(fd1[1], "done", 5);
        cout << "hello! i am child!" << endl;
        _exit(0);


        


    } 
    else {                                        // parent process
        int PC, SP, IR, AC, X, Y;

        cout << "I am parent!!\n";
        read(fd1[0], buf, 5);
        cout << "PARENT: read \"" << buf << "\"\n";
        waitpid(-1, NULL, 0);


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



    }




}