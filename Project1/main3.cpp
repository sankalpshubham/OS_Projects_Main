#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <stack>

using namespace std;

int main(int argc, char **argv)
{

    string fileName;
    int timer;
    if (argc == 1){
        cout << "ERROR: No arguments inputed." << endl;
    }

    else{
        fileName = argv[1];
        timer = stoi(argv[2]);
        // cout << fileName << endl<< timer << endl;

        int fd[2];
        int fd2[2];
        char buf[30];
        int result;

        result = pipe(fd);
        if (result == -1)
            exit(1);
        result = pipe(fd2);
        if (result == -1)
            exit(1);

        result = fork();
        if (result == -1)
            exit(1);

        if (result == 0){ // child process(Memory)

            close(fd[0]); // close file descriptors
            close(fd2[1]);

            int num;
            string x;
            ifstream myFile(fileName);
            if (!myFile)
                cout << "File not found!" << endl;

            int mem[2000];                              //user program stored in memory
            fill_n(mem, 2000, -1);
            int i = 0;
            while (getline(myFile, x)){
                if(isdigit(x[0])){
                    num = stoi(x);
                    mem[i] = num;
                    i++;
                }
                else if(x[0] == '.'){
                    x = x.substr(1);
                    i = stoi(x);
                    continue;
                }
                else
                    continue;
                
            }
            i = 0;
            int PC;
            int stack;
            while (true){
                read(fd2[0], &PC, sizeof(PC));
                if(PC == -11){                                       //access system stack
                    read(fd2[0], &PC, sizeof(PC));
                    stack = PC;
                    if(mem[stack] == -1){                           //push to system stack
                        read(fd2[0], &PC, sizeof(PC));
                        mem[stack] = PC;
                        //cout << "STACK SYS PUSH " << stack << ": " << mem[stack] << endl;
                        continue;

                    }else{                                          //pop from system stack
                        write(fd[1], &mem[stack], sizeof(mem[stack]));
                        //cout << "STACK SYS POP " << stack << ": " << mem[stack] << endl;
                        mem[stack] = -1;
                        continue;

                    }

                    
                }
                else if(PC == -10){       //access user stack
                    //cout << "..";
                    read(fd2[0], &PC, sizeof(PC));
                    stack = PC;
                    if(mem[stack] == -1){                           //push to user stack
                        read(fd2[0], &PC, sizeof(PC));
                        mem[stack] = PC;
                        //cout << "STACK PUSH " << stack << ": " << mem[stack] << endl;
                        continue;                              
                        
                    }
                    else{                                           //pop from user stack
                        write(fd[1], &mem[stack], sizeof(mem[stack]));
                        //cout << "STACK POP " << stack << ": " << mem[stack] << endl;
                        mem[stack] = -1;
                        continue;
                    }
                    
                }
                else if(PC == -9){                              //write to memory location
                    read(fd2[0], &PC, sizeof(PC));
                    stack = PC;
                    read(fd2[0], &PC, sizeof(PC));
                    mem[stack] = PC;
                    continue;

                }
                write(fd[1], &mem[PC], sizeof(mem[PC]));
                if(mem[PC] == 50)
                    break;
            }
            _exit(0);
        }
        else{ // parent process(CPU)
            

            close(fd[1]);                   //close file descriptors
            close(fd2[0]);
            //registers
            int PC, AC, X, Y, IR, SS;
            PC = AC = X = Y = IR = 0;
            int op = 0;
            int SP = 1000;
            SS = 2000;
            //
            int randNum;
            int x;
            int in = -11;
            int uin = -10;
            int sev = -9;
            //interrupts
            bool user = true;
            bool intr = false;
            int intrCount = 0;
            int count = 0;
            int userSP = 0;


            srand((unsigned int)time(NULL));
            while (IR != 50){

                if(intr && count > 0 && (count % timer) == 0){                      //check if there are interrupts doing interrupt
                    intrCount++;
                    //cout << "INTCOUNT: " << intrCount << endl;
                }                                           
                    



                if((!intr && count > 0) && (count & timer == 0) || (!intr && intrCount)){                                     //timer interrupt
                    //cout << "PC: " << PC << " IR: " << IR << endl;
                    user = false;
                    intr = true;
                    intrCount--;
                    
                    userSP = SP;
                    SP = SS;
                    SP--;
                    write(fd2[1], &in, sizeof(in));
                    write(fd2[1], &SP, sizeof(SS));
                    write(fd2[1], &userSP, sizeof(userSP));
                    SP--;
                    write(fd2[1], &in, sizeof(in));
                    write(fd2[1], &SP, sizeof(SP));
                    PC--;
                    write(fd2[1], &PC, sizeof(PC));
                    PC = 1000;

                }

                write(fd2[1], &PC, sizeof(PC));
                read(fd[0], &IR, sizeof(IR));
                //cout << "PC: " << PC << endl;
                //cout << "PC: " << PC << " Count: " << count+1 << endl;
                count++;
                
                //switch cases for CPU Instructions

                switch(IR){
                    case 1:                                             //Load the value into the AC
                        //cout << "1: " << endl;
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        AC = op;
                        break;
                    case 2:                                             //Load the value at the address into the AC
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        if(op > 999 && user){
                            cout << "Memory violation: accessing system address 1000 in user mode" << endl;
                        }else{
                            write(fd2[1], &op, sizeof(PC));
                            read(fd[0], &op, sizeof(op));
                            AC = op;
                        }
                        break;
                        

                    case 3:                                             //Load the value from the address found in the given address into the AC
                                                                        //(for example, if LoadInd 500, and 500 contains 100, then load from 100).
                        
                        break;

                    case 4:                                             //Load the value at (address+X) into the AC
                                                                        //(for example, if LoadIdxX 500, and X contains 10, then load from 510).
                        //cout << "4: " << endl;
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        x = op + X;
                        if(x > 999 && user){
                            cout << "Memory violation: accessing system address 1000 in user mode " << endl;
                        }else{
                            write(fd2[1], &x, sizeof(PC));
                            read(fd[0], &op, sizeof(op));
                            AC = op;
                        }
                        break;
                    case 5:                                             //Load the value at (address+Y) into the AC
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        x = op + Y;
                        if(x > 999 && user){
                            cout << "Memory violation: accessing system address 1000 in user mode " <<  endl;
                        }else{
                            write(fd2[1], &x, sizeof(PC));
                            read(fd[0], &op, sizeof(op));
                            AC = op;
                        }
                        break;
                    case 6:                                             //Load from (Sp+X) into the AC (if SP is 990, and X is 1, load from 991).
                        AC = SP + X;
                        write(fd2[1], &AC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        AC = op;
                        break;
                    case 7:                                             //Store the value in the AC into the address
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        write(fd2[1], &sev, sizeof(sev));
                        write(fd2[1], &op, sizeof(op));
                        write(fd2[1], &AC, sizeof(AC));
                        break;
                    case 8:                                             //Gets a random int from 1 to 100 into the AC
                        randNum = (rand() % 100) + 1;
                        AC = randNum;
                        cout << AC << endl;
                        break;
                    case 9:                                             //If port=1, writes AC as an int to the screen
                                                                        //If port=2, writes AC as a char to the screen
                        //cout << "9: " << endl;
                        PC++;
                        //cout << "PC: " << PC << endl;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        //cout << op;
                        if(op == 1)
                            cout << AC;
                        else if(op == 2)
                            cout << char(AC);

                        break;
                    case 10:                                             //Add the value in X to the AC
                        AC += X;
                        break;
                    case 11:                                             //Add the value in Y to the AC
                        AC += Y;
                        break;
                    case 12:                                             //Subtract the value in X from the AC
                        AC -= X;
                        break;
                    case 13:                                             //Subtract the value in Y from the AC
                        AC -= Y;
                        break;
                    case 14:                                             //Copy the value in the AC to X
                        //cout << "14: " << endl;
                        X = AC;
                        break;
                    case 15:                                             //Copy the value in X to the AC
                        AC = X;
                        break;
                    case 16:                                             //Copy the value in the AC to Y
                        Y = AC;
                        break;
                    case 17:                                             //Copy the value in Y to the AC
                        AC = Y;
                        break;
                    case 18:                                             //Copy the value in AC to the SP
                        SP = AC;
                        break;
                    case 19:                                             //Copy the value in SP to the AC
                        AC = SP; 
                        break;
                    case 20:                                             //Jump to the address
                        //cout << "20: " << endl;
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        PC = op - 1;
                        break;
                    case 21:                                             //Jump to the address only if the value in the AC is zero
                        //cout << "21: " << endl;
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        
                        if(AC == 0)
                            PC = op - 1;
                        break;
                    case 22:                                             //Jump to the address only if the value in the AC is not zero
                        PC++;
                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));
                        if(AC != 0)
                            PC = op - 1;
                        break;
                    case 23:                                             //Push return address onto stack, jump to the address
                        PC++;
                        SP--;
         
                        write(fd2[1], &uin, sizeof(PC));
                        write(fd2[1], &SP, sizeof(SP));
                        write(fd2[1], &PC, sizeof(PC));

                        write(fd2[1], &PC, sizeof(PC));
                        read(fd[0], &op, sizeof(op));        
                        PC = op-1;
                        break;
                    case 24:                                             //Pop return address from the stack, jump to the address
                        write(fd2[1], &uin, sizeof(uin));
                        write(fd2[1], &SP, sizeof(SP));
                        read(fd[0], &op, sizeof(op));
                        PC = op;
                        SP++;
                        break;
                    case 25:                                             //Increment the value in X
                        X++;
                        break;
                    case 26:                                             //Decrement the value in X
                        X--;
                        break;
                    case 27:                                             //Push AC onto stack
                        SP--;
                        if(intr)
                            write(fd2[1], &in, sizeof(in));
                        else
                            write(fd2[1], &uin, sizeof(uin));
                        write(fd2[1], &SP, sizeof(SP));
                        write(fd2[1], &AC, sizeof(PC));
                        break;
                    case 28:                                             //Pop from stack into AC
                        if(intr)
                            write(fd2[1], &in, sizeof(in));
                        else
                            write(fd2[1], &uin, sizeof(uin));
                        write(fd2[1], &SP, sizeof(SP));
                        read(fd[0], &op, sizeof(op));
                        SP++;
                        AC = op;
                        break;
                    case 29:                                             //Perform system call
                        if(!intr){

                            user = false;
                            intr = true;
                            userSP = SP;
                            SP = SS;
                            SP--;
                            write(fd2[1], &in, sizeof(PC));
                            write(fd2[1], &SP, sizeof(SP));
                            write(fd2[1], &userSP, sizeof(userSP));
                            SP--;
                            write(fd2[1], &in, sizeof(PC));
                            write(fd2[1], &SP, sizeof(SP));
                            write(fd2[1], &PC, sizeof(SP));
                            PC = 1499;
                        }
                        break;
                    case 30:                                             //Return from system call

                        user = true;
                        intr = false;
                        write(fd2[1], &in, sizeof(PC));
                        write(fd2[1], &SP, sizeof(SP));
                        read(fd[0], &op, sizeof(op));
                        PC = op;
                        //cout << "PROGRAM COUNTER AFTER RETURN: " << PC << endl;

                        SP++;
                        write(fd2[1], &in, sizeof(PC));
                        write(fd2[1], &SP, sizeof(SP));
                        read(fd[0], &op, sizeof(op));
                        SP = op;
                        //cout << "STACK POINTER AFTER RETURN: " << SP << endl;
                        break;
                    default:                                             //End execution
                        break;
                }
                PC++;
                
            }
            waitpid(-1, NULL, 0);
        }
    }
}