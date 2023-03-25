import java.util.concurrent.Semaphore;

public class PostOfficeSimulation {

    /* Semaphores for the maximum number of customers inside the post office at once,
    *  the number of postal workers, scale, customerReady, postal worker availability, task finished, 
    *  which postal worker is currently being used, and two communication sempahores to specify the 
    *  order of execution between customer and postal worker thread prints
    */
    private static Semaphore postOfficeCapacity = new Semaphore(10);
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
        // create and start the postal worker threads
        for (int i = 0; i < 3; i++) {
            PostalWorker postalWorker = new PostalWorker(i);
            postalWorker.start();
        }

        // create the customer threads and randomly assign them a task
        Customer[] customer = new Customer[50];
        for (int i = 0; i < 50; i++) {
            int randomTaskTime = TASK_TIMES[(int) (Math.random() * TASK_TIMES.length)];
            customer[i] = new Customer(i, randomTaskTime);
            customer[i].start();
        }

        // join the threads after completion
        for(int i = 0; i < 50; i++) {
			try {
				customer[i].join();
				System.out.println("Joined customer " + i);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

        System.exit(0);
    }

    private static class Customer extends Thread {
        private final int id;
        private final int taskTime;

        // initialize the customer threads with their id and task time
        public Customer(int id, int taskTime) {
            this.id = id;
            this.taskTime = taskTime;
            System.out.println("Customer " + id + " created");
        }

        public void run() {
            try {
                // Acquire a permit to enter the post office.
                postOfficeCapacity.acquire();

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
                    postalWorkerSem[2].acquire();
                    postalWorkerID = 2;
                }
                // assign customer id and the customer assigned task to the postal worker number
                customer_postalWorker_connection[postalWorkerID][0] = id;
                customer_postalWorker_connection[postalWorkerID][1] = taskTime;
                
                // signal that customer is ready to be attended and wait for communication back from postal worker
                customerReady.release();
                thread_communication.acquire();

                switch (taskTime) {
                    case 60:          
                        // buy a stamp
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to buy a stamp");

                        // waiting on the postal worker to finish
                        thread_communication2.release();
                        finished.acquire();
                        
                        System.out.println("Customer " + id + " finished buying stamps");
                        break;
                    case 90:
                        // mail a letter
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to mail a letter");

                        thread_communication2.release();
                        finished.acquire();
                        
                        System.out.println("Customer " + id + " finished mailing a letter");
                        break;
                    case 120:
                        // mail a package
                        System.out.println("Customer " + id + " asks postal worker " + postalWorkerID + " to mail a package");

                        thread_communication2.release();
                        finished.acquire();
                        
                        System.out.println("Customer " + id + " finished mailing a package");
                        break;
                }
                
                System.out.println("Customer " + id + " leaves the post office");

                // signal the customer capacity since customer left
                postOfficeCapacity.release();

            } catch (InterruptedException e) {
                e.printStackTrace();
            }

        }
    }

    private static class PostalWorker extends Thread {
        private final int id;

        // initialize postal worker thread
        public PostalWorker(int id) {
            this.id = id;
            System.out.println("Postal worker " + id + " created");
        }

        public void run() {
            while (true) {
                try {
                    // waiting for the customer to come in
                    customerReady.acquire();

                    // get the customer id and the customer task that postal worker is serving
                    int customer_id = customer_postalWorker_connection[id][0];
                    int task_time = customer_postalWorker_connection[id][1];

                    // serving the customer
                    System.out.println("Postal worker " + id + " is serving customer " + customer_id);
                    thread_communication.release();     // signal for next steps and
                    thread_communication2.acquire();    // wait for further steps from the cusomter thread
                    
                    switch (task_time) {
                        case 60:
                            // buy a stamp
                            Thread.sleep(1000);
                            break;
                        case 90:
                            // mail a letter
                            Thread.sleep(1500);
                            break;
                        case 120:
                            // wait for the scale
                            scale.acquire();
                            System.out.println("Scale in use by postal worker " + id);

                            Thread.sleep(2000);

                            // signal finished using the scale
                            scale.release();

                            System.out.println("Scale released by postal worker " + id);
                            break;
                    }

                    System.out.println("Postal worker " + id + " finished serving customer " + customer_id);
                    
                    // signal finished fulfilling the customer task
                    finished.release(); 

                    // signal postal worker id and postal worker is ready for next customer
                    postalWorkerSem[id].release();
                    postalWorkerAvailable.release();
                    
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

            }
        }
    }
}