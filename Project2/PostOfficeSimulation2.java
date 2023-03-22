import java.util.concurrent.Semaphore;

public class PostOfficeSimulation2 {

    // Constants for the number of customers, the maximum number of customers inside the post office at once,
    // and the number of postal workers.
    // Semaphores for controlling access to the post office, scale, customerReady, postal worker availability, and task finished
    private static Semaphore customerCapacity = new Semaphore(5);   // NEEDS TO BE 10 *****************
    private static Semaphore postalWorkerAvailable = new Semaphore(3);
    private static Semaphore customerReady = new Semaphore(0);
    private static Semaphore finished = new Semaphore(0);
    private static Semaphore scale = new Semaphore(1);
    private static Semaphore postalWorkerSem[] = {new Semaphore(1, true), new Semaphore(1, true), new Semaphore(1, true)};
    private static Semaphore thread_communication = new Semaphore(0);
    private static Semaphore thread_communication2 = new Semaphore(0);

    // Task table with times for each task in seconds. (FOR EACH 60 SECs, the thread sleeps for 1 SEC)
    private static final int[] TASK_TIMES = {60, 90, 120};
    public static int[][] customer_postalWorker_connection = new int[3][3];

    public static void main(String[] args) {
        // Create and start the postal worker threads.
        for (int i = 0; i < 3; i++) {
            PostalWorker postalWorker = new PostalWorker(i);
            postalWorker.start();
        }

        // Create the customer threads and randomly assign them a task.
        Customer[] customer = new Customer[10]; // NEEDS TO BE 50 ******************
        for (int i = 0; i < 10; i++) {      // NEEDS TO BE 50 *******************
            int randomTaskTime = TASK_TIMES[(int) (Math.random() * TASK_TIMES.length)]; // need to send this to postal worker
            customer[i] = new Customer(i, randomTaskTime);
            customer[i].start();
        }

        for(int i = 0; i < 10; i++) {       // NEEDS TO BE 50 *******************
			try {
				customer[i].join();
				System.out.println("Joined customer " + i);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
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

                System.out.println("Customer " + id + " enters post office");

                // wait if customer can enter queue to be attended by a postal worker
                postalWorkerAvailable.acquire();

                // identify which postal worker is serving which one of the customers
                int postalWorkerID;
                if (postalWorkerSem[0].tryAcquire()) {
                    postalWorkerID = 0;
                } else if (postalWorkerSem[1].tryAcquire()) {
                    postalWorkerID = 1;
                } else {
                    postalWorkerSem[2].tryAcquire();
                    postalWorkerID = 2;
                }
                // assign customer id and the customer assigned task to the postal worker number
                customer_postalWorker_connection[postalWorkerID][0] = id;
                customer_postalWorker_connection[postalWorkerID][1] = taskTime;
                
                // signal that the customer is ready to be attended
                customerReady.release();
                thread_communication.acquire();
                //System.out.println(postalWorkerID + " -------------------------------------------------- CUSTOMER: THIS IS AFTER THREAD COMMUNICATION ---------- ");

                switch (taskTime) {
                    case 60:          
                        // buy a stamp
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to buy a stamp");

                        // waiting on the postal worker to finish
                        thread_communication2.release();
                        finished.acquire();
                        

                        System.out.println("Customer " + id + " finished buying stamps");
                    case 90:
                        // mail a letter
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to mail a letter");

                        thread_communication2.release();
                        finished.acquire();
                        

                        System.out.println("Customer " + id + " finished mailing a letter");
                    case 120:
                        // mail a package
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to mail a package");

                        thread_communication2.release();
                        finished.acquire();
                        

                        System.out.println("Customer " + id + " finished mailing a package");
                    default:
                        // error
                }
                
                System.out.println("Customer " + id + " leaves the post office");

                // signal the customer leaving
                customerCapacity.release();

            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }
    }

    private static class PostalWorker extends Thread {
        private final int id;

        public PostalWorker(int id) {
            this.id = id;
            System.out.println("Postal worker " + id + " created");
        }

        public void run() {
            while (true) {
                try {
                    // waiting for the customer
                    customerReady.acquire();

                    int customer_id = customer_postalWorker_connection[id][0];
                    int task_time = customer_postalWorker_connection[id][1];

                    // Serving the customer.
                    System.out.println("Postal worker " + id + " is serving customer " + customer_id);
                    thread_communication.release();     // signal for next steps

                    // ACQUIRE HERE PLS
                    thread_communication2.acquire();    // wait for further steps

                    //System.out.println(id + " -------------------------------------------------- POSTAL: THIS IS AFTER THREAD COMMUNICATION ---------- ");
                    
                    //System.out.println(" -------------------------------------------------- Called by FINSHED SEMAPHORE");
                    // NEED THE CUSTOMER id AND task number TO sleep AND DO THE TASK                    
                    switch (task_time) {
                        case 60:
                            // buy a stamp
                            Thread.sleep(100);  // NEEDS TO BE 1000 ****
                            
                        case 90:
                            // mail a letter
                            Thread.sleep(150); // NEEDS TO BE 1500 ****

                        case 120:
                            // wait for the scale
                            scale.acquire();
                            System.out.println("Scale in use by postal worker " + id);

                            Thread.sleep(200); // NEEDS TO BE 2000 ****

                            // signal finished using the scale
                            scale.release();

                            System.out.println("Scale released by postal worker " + id);
                        default:
                            // error
                    }

                    System.out.println("Postal worker " + id + " finished serving customer " + customer_id);
                    
                    // finished fulfilling the customer task
                    finished.release(); 

                    // signal postal worker is ready for next customer
                    postalWorkerSem[id].release();
                    postalWorkerAvailable.release();
                    
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

            }
        }
    }
}