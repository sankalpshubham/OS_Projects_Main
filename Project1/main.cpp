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

    if (argc == 1) {   
        cout << "ERROR: No arguments inputed." << endl;
        exit(1);
    } else {
        fileName = argv[1];
        timer = stoi(argv[2]);
        cout << fileName << endl<< timer << endl;
    }

    int fds[2];
    char buf[30];
    pid_t pid;

    pid = pipe(fds);                    // do a pipe and check if failed
    if (pid == -1)
        exit(1);

    switch (pid = fork()) {                         // create processes
        case -1:{
            printf("The fork failed!");
            exit(-1);
        }

        case 0:{                                     // child process
            int num;
            string x;
            ifstream myFile(fileName);
            if (!myFile)
                cout << "File not found!" << endl;

            int mem[2000];
            int i = 0;
            while (getline(myFile, x)){
                if (x == "")
                    continue;
                num = stoi(x);
                mem[i] = num;
                i++;
                cout << mem[i-1] << "\n";
            }

            write(fds[1], "done", 5);
            cout << "hello! i am child!" << endl;
            _exit(0);
        }

        default: {                                       // parent process
            int PC, SP, IR, AC, X, Y;
            cout << "I am parent!!\n";
            read(fds[0], buf, 5);
            cout << "PARENT: read \"" << buf << "\"\n";
            waitpid(-1, NULL, 0);
        }
    }
}