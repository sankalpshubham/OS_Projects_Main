import java.util.ArrayList;
import java.util.*;
import java.io.*;

// ADD INVALID REQUEST (not enough memory to allocate) OR RELEASE (asked to release something not inside the list)

public class Memory {
    public static void main (String[] args) throws FileNotFoundException{
        ArrayList<int []> memory = new ArrayList<>();
        ArrayList<String> instructions = new ArrayList<>();
        Scanner in = new Scanner(new File(args[0]));

        while (in.hasNext()){
            instructions.add(in.nextLine());
        }
        memory.add(new int[]{32, 1024});
        printBlock(memory,"");
        int letter = 65;

        System.out.println("WHATS UP!");
        
        for (String inst : instructions){
            String instruction[] = inst.split(" ");
            String command = instruction[0];
            String mem = instruction[1];
            switch (command){
                case "Request":
                    memoryOp(0,memory, Integer.parseInt(mem.substring(0,mem.length()-1)), letter++);
                    break;
                case "Release":
                    memoryOp(1,memory, -100, mem.charAt(0));
                    break;
                default:
                    break;
            }
            printBlock(memory,inst);
        }
    }

    public static void printBlock(ArrayList<int[]> Mem, String inst){
        System.out.println(inst);
        System.out.print("| ");
        for (int item[] : Mem){
            System.out.print((char)(item[0]) + " " + item[1] + "K |");
        }
        System.out.println();
    }

    public static void memoryOp(int op, ArrayList<int []> Mem, int size, int letter){
        switch (op){
            case 0:                     // [ [letter, size] ]
                int indexOffset;
                for (indexOffset = 0; indexOffset < Mem.size(); indexOffset++){                 // find the first empty block with satisfiable size for the request
                    if (Mem.get(indexOffset)[0] == 32 && Mem.get(indexOffset)[1] >= size){
                        break;
                    }
                }
                while (true){                                           // half the memory blocks until find the free block with the perfect size
                    if (Mem.get(indexOffset)[1] >= size*2){
                        int halved = Mem.get(indexOffset)[1]/2;
                        Mem.get(indexOffset)[1] = halved;
                        Mem.add(indexOffset,new int[]{32,halved});
                    }else{
                        Mem.get(indexOffset)[0] = letter;
                        break;
                    }
                }
                break;
            case 1:
                int i;
                for (i = 0; i < Mem.size(); i++){
                    if (Mem.get(i)[0] == letter){
                        Mem.get(i)[0] = 32;
                        break;
                    }
                }
                mergeMemory(i, Mem);
                break;
            default:
                break;
        }
    }




    public static void mergeMemory(int i, ArrayList<int []> Mem) {
        for (i = 0; i < Mem.size(); i++) {
            // check right
            if(i != Mem.size() - 1  && Mem.get(i)[1] == Mem.get(i+1)[1]){
                while(i < Mem.size() - 1 ) {
                    if( Mem.get(i)[1] == Mem.get(i+1)[1] && Mem.get(i)[0] == 32 && Mem.get(i+1)[0]==32){
                        // merge
                        int doubledNum = Mem.get(i)[1] *2 ;
                        Mem.get(i)[1] = doubledNum;
                        Mem.remove(i+1);
                    } else {
                        break;
                    }
                } // end while
            } // end check right
            // check left
            if(i != 0 && Mem.get(i)[1] == Mem.get(i-1)[1]){
                while(i > 0){
                    if( Mem.get(i)[1] == Mem.get(i-1)[1] && Mem.get(i)[0] == 32 && Mem.get(i-1)[0]==32){
                        // merge
                        int doubledNum = Mem.get(i)[1] *2 ;
                        Mem.get(i-1)[1] = doubledNum;
                        Mem.remove(i);
                        i--;
                    }else{break;}
                }
            } // end check left
        } // end for loop
    }
}