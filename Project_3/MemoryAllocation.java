/* Sankalp Shubham
 * sxs190290
 */

import java.util.ArrayList;
import java.util.*;
import java.io.*;

public class MemoryAllocation {
    public static ArrayList<int []> memory = new ArrayList<int[]>();        // initialize variables to store the memory and instructions
    public static ArrayList<String> commands = new ArrayList<String>();

    public static void main (String[] args) throws FileNotFoundException{

        if (args.length != 1) {
            System.out.println("Please provide a text file to read");
            System.exit(0);
        }

        Scanner in = new Scanner(new File(args[0]));
        while (in.hasNext()){
            commands.add(in.nextLine());
        }

        memory.add(new int[]{32, 1024});        // initilize memory and print it
        printMemory("");
        int alphabet = 65;                      // define the ascii for A for starting memory requests

        for (String input : commands){
            String instructions[] = input.split(" ");
            String command = instructions[0], size = instructions[1];

            if (command.equals("Request")) {                        // process Request and Release
                memoryRequest(Integer.parseInt(size.substring(0,size.length()-1)), alphabet++);
            } else {
                releaseMemory(size.charAt(0));
            }

            printMemory(input);
        }

    }

    public static void printDash(int length) {                          // printing the dash for the memory blocks
        for (int i = 0; i < length + 2; i ++){
            System.out.print("-");
        }
        System.out.println("");
    }

    public static void printMemory(String inst){                        // print the memory blocks
        String output = "";
        for (int item[] : memory){
            output = output.concat((char)(item[0]) + " " + item[1] + "K |");
        }

        System.out.println(inst);
        printDash(output.length());
        System.out.print("| ");
        System.out.println(output);
        printDash(output.length());
    }  


    public static void memoryRequest(int size, int letter) {
        int i;
        for (i = 0; i < memory.size(); i++) {                           // find the first empty block with satisfiable size request
            if (memory.get(i)[0] == 32 && memory.get(i)[1] >= size) {
                break;
            }
        }
        
        if (i >= memory.size()) {                                       // if requesting memory spaces that can't be fulfilled 
            System.out.println("Invalid Memory Request: Not enough memory space to fulfill request!");
            System.exit(1);
        }

        while (true) {                                                  // half the memory blocks until find the free block with the perfect size
            if (memory.get(i)[1] >= size * 2) {
                int halfMem = memory.get(i)[1] / 2;
                memory.get(i)[1] = halfMem;
                memory.add(i, new int[] {32, halfMem});
            } else {
                memory.get(i)[0] = letter;
                break;
            }
        }

    }

    public static void releaseMemory(int letter) {
        int i;
        for (i = 0; i < memory.size(); i++){            // release the memory with the correct memory block name/request
            if (memory.get(i)[0] == letter){
                memory.get(i)[0] = 32;
                break;
            }
        }
        mergeMemory(i);                                 // merge memory after relasing the memory blocks
    }


    public static void mergeMemory(int i) {             // merge memory on the both sides of the memory block to return to original size
        for (; i < memory.size(); i++) {
            // merge memory on the right
            if(i != memory.size() - 1  && memory.get(i)[1] == memory.get(i+1)[1]){  
                while(i < memory.size() - 1 ) {
                    if(memory.get(i)[1] == memory.get(i+1)[1] && memory.get(i)[0] == 32 && memory.get(i+1)[0]==32){    // merge the blocks
                        int originalSize = memory.get(i)[1] *2 ;
                        memory.get(i)[1] = originalSize;
                        memory.remove(i+1);
                    } else {
                        break;
                    }
                }
            }
            // merge memory on the left
            if(i != 0 && memory.get(i)[1] == memory.get(i-1)[1]){
                while(i > 0){
                    if(memory.get(i)[1] == memory.get(i-1)[1] && memory.get(i)[0] == 32 && memory.get(i-1)[0]==32){    // merge the blocks
                        int originalSize = memory.get(i)[1] *2 ;
                        memory.get(i-1)[1] = originalSize;
                        memory.remove(i);
                        i--;
                    } else {
                        break;
                    }
                }
            }
        }
    }

}