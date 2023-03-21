// class HelloWorld {
//     public static void main(String[] args) {
//         System.out.println("");
//         System.out.println("Hello, World!"); 
//     }
// }

import java.util.concurrent.Semaphore;

public class PostOfficeSimulation2 {

    // Constants for the number of customers, the maximum number of customers inside the post office at once,
    // and the number of postal workers.
    // Semaphores for controlling access to the post office and the scales.
    private static Semaphore customerCapacity = new Semaphore(10);
    private static Semaphore scale = new Semaphore(1);

    // *******************************
    // Enter Post office is based on customerCapacity semaphore
    // 
    //
    //
    // *******************************

    // Task table with times for each task in seconds. (FOR EACH 60 SECs, the thread sleeps for 1 SEC)
    private static final int[] TASK_TIMES = {60, 90, 120};

    public static void main(String[] args) {
        // Create and start the postal worker threads.
        for (int i = 1; i <= 3; i++) {
            PostalWorker postalWorker = new PostalWorker(i);
            postalWorker.start();
        }

        // Create the customer threads and randomly assign them a task.
        for (int i = 1; i <= 50; i++) {
            int randomTaskTime = TASK_TIMES[(int) (Math.random() * TASK_TIMES.length)];
            Customer customer = new Customer(i, randomTaskTime);
            customer.start();
        }
    }

    private static class Customer extends Thread {
        private final int id;
        private final int taskTime;

        public Customer(int id, int taskTime) {
            this.id = id;
            this.taskTime = taskTime;
            System.out.println("Customer " + id + " created");
        }

        public void run() {
            try {
                // Acquire a permit to enter the post office.
                customerCapacity.acquire();
                System.out.println("Customer " + id + " enters post office.");
                
                // insert if statement to identify what task it is 
                // ^ CUSTOMER + id +     ASKS POSTAL WORKER + id +   (insert task [buy a stamp, mail a letter, mail a package])

                // Sleep for the task time.
                Thread.sleep((taskTime * 1000 / 60));

                System.out.println("Customer " + id + " completed task and left post office.");
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                // Release the permit for the post office.
                customerCapacity.release();
            }
        }
    }

    private static class PostalWorker extends Thread {
        private final int id;
        // Get customer id
        // Get task number

        public PostalWorker(int id) {
            this.id = id;
            System.out.println("Postal worker " + id + " created.");
        }

        public void run() {
            while (true) {
                try {
                    // Acquire a permit to enter the post office.
                    customerCapacity.acquire();

                    // Wait for a customer to arrive.
                    System.out.println("Postal worker " + id + " waiting for customer.");
                    while (customerCapacity.availablePermits() == 10) {
                        Thread.sleep(1000);
                    }

                    // Serve the next customer in line.
                    System.out.println("Postal worker " + id + " serving customer.");           // NEED TO PASS IN A CUSTOMER ID **********

                    // This is for mailing a package
                    scale.acquire();
                    System.out.println("Postal worker " + id + " using scales.");
                    Thread.sleep(TASK_TIMES[(int) (Math.random() * TASK_TIMES.length)] * 1000);
                    System.out.println("Postal worker " + id + " finished serving customer.");
                    scale.release();
                    // --------------
                } catch (InterruptedException e) {
                    e.printStackTrace();
                } finally {
                    // Release the permit for the post office.
                    customerCapacity.release();
                }
            }
        }
    }
}
