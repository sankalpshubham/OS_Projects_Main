The project has only one file that needs to be run called "main.cpp"

The project contains the following sample files that test the output of the program:
1. sample1.txt
2. sample2.txt
3. sample3.txt
4. sample4.txt
5. sample5.txt 

The sample5.txt is the sample file that I created. The program reads in the text file and prints a house on the terminal by priting symbols, spaces, and new lines.

Before running the program, you have to make sure to use the right version of g++. In this case it is: ```g++ -std=c++11 -o main main.cpp``` when running on the UTD cs1 server.

To run the program make sure you are in the right directory and type:

```
./main <sample_file_name>.txt <timer_number>
```

The <sample_file_name> will be either sample1 or sample2 (out of any 5 sample files).
The <timer_number> will be a number value that you input. The timer can never be 0. While running any program that doesn't require the timer (ex: sample1.txt and sample2.txt), the timer value should be 1.
