import java.util.ArrayList;
import java.util.*;
import java.io.*;

public class MemoryAllocation {
    public static ArrayList<int []> memory = new ArrayList<>();
    public static ArrayList<String> commands = new ArrayList<>();

    // ADD INVALID REQUEST (not enough memory to allocate) OR RELEASE (asked to release something not inside the list)

    public static void main (String[] args) throws FileNotFoundException{

        // PUT IN AN ERROR FOR NO FILE

        Scanner in = new Scanner(new File(args[0]));
        while (in.hasNext()){
            commands.add(in.nextLine());
        }

        memory.add(new int[]{32, 1024});        // initilize memory and print it
        printMemory("");
        int alphabet = 65;                      // define the ascii for A for starting point

        for (String input : commands){
            String instructions[] = input.split(" ");
            String command = instructions[0], size = instructions[1];

            if (command.equals("Request")) {
                memoryRequest(Integer.parseInt(size.substring(0,size.length()-1)), alphabet++);
            } else {
                releaseMemory(size.charAt(0));
            }

            printMemory(input);
        }

    }

    public static void printDash(int length) {
        for (int i = 0; i < length + 2; i ++){
            System.out.print("-");
        }
        System.out.println("");
    }

    public static void printMemory(String inst){
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
        for (i = 0; i < memory.size(); i++) { // find the first empty block with satisfiable
            if (memory.get(i)[0] == 32 && memory.get(i)[1] >= size) {
                break;
            }
        }
        while (true) { // half the memory blocks until find the free block with the perfect size
            if (memory.get(i)[1] >= size * 2) {
                int halved = memory.get(i)[1] / 2;
                memory.get(i)[1] = halved;
                memory.add(i, new int[] { 32, halved });
            } else {
                memory.get(i)[0] = letter;
                break;
            }
        }

    }

    public static void releaseMemory(int letter) {
        int i;
        for (i = 0; i < memory.size(); i++){
            if (memory.get(i)[0] == letter){
                memory.get(i)[0] = 32;
                    break;
                }
            }
        mergeMemory(i);
    }


    public static void mergeMemory(int i) {
        for (; i < memory.size(); i++) {
            // check right
            if(i != memory.size() - 1  && memory.get(i)[1] == memory.get(i+1)[1]){
                while(i < memory.size() - 1 ) {
                    if( memory.get(i)[1] == memory.get(i+1)[1] && memory.get(i)[0] == 32 && memory.get(i+1)[0]==32){
                        // merge
                        int doubledNum = memory.get(i)[1] *2 ;
                        memory.get(i)[1] = doubledNum;
                        memory.remove(i+1);
                    } else {
                        break;
                    }
                } // end while
            } // end check right
            // check left
            if(i != 0 && memory.get(i)[1] == memory.get(i-1)[1]){
                while(i > 0){
                    if( memory.get(i)[1] == memory.get(i-1)[1] && memory.get(i)[0] == 32 && memory.get(i-1)[0]==32){
                        // merge
                        int doubledNum = memory.get(i)[1] *2 ;
                        memory.get(i-1)[1] = doubledNum;
                        memory.remove(i);
                        i--;
                    }else{break;}
                }
            } // end check left
        } // end for loop
    }



}