To following program simulates the Buddy System for memory allocation. The program is written in Java.

To run the program, first compile the program by typing in:

```
javac MemoryAllocation.java
```

Then after compiling the program, run the program by typing in:

```
java MemoryAllocation <filename>.txt
```

where the filename is the name of the file that contains the commands for memory requests and releases.
The format of the .txt file should be like this for example:

```
Request 100K
Request 240K
Request 64K
Release C
Release A
Release B
```

DISCLAIMER: On some platforms, when you first run the Java program after compiling it, the output sometimes isn't as expected because Java, when it first compiles, assigns some variables garbage/random values. If this occurs on your platform and the output isn't as expected, then run the program again without re-compiling it and the output will be exactly correct because by the second run, the variables are assigned the right values by the compiler. Again, the output maybe be wrong but ONLY for the first run because of the behavior of the Java compiler, but all the subsquent runs return the correct output. 