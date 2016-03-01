Running the program:
1. Compile the tickets.c file (gcc -g -Wall -pthread tickets.c -lpthread -o ticket_sale)
2. Run the program with one parameter which will be the N for amount of customers per queue (N = 5, N = 10, N = 15)
3. A timer will run until Time 61 where it will say: "END TIME: 61"
4. When "END TIME: 61" appears, open the two files in the same directory as tickets.c:
      a. table.txt
      b. time_stamp.txt
5. table.txt shows the seat chart for every ticket purchase where each seat is occupied by the name of the vendor and the customer number
      a. Example: say vendor H0 sold to customer 2 seat 5, then seat number 5 has tag "H005" on it in the seating chart
6. time_stamp.txt shows the events at every second of the 60+ seconds that the simulation runs
      a. Events include: customer enters queue, customer is being attended by vendor, vendor finishes transaction, and vendor turns a customer away because tickets are sold out
      b. After the events, a list of how many customers were turned away (rejected) by each vendor and a list of how many transactions succeeded by each vendor
7. table (N = ?).txt where ? = 5, 10, 15 shows the results of table.txt when N = 5, 10, or 15 customers per queue
8. time_stamp (N = ?).txt where ? = 5, 10, 15 shows the results of time_stamp.txt when N = 5, 10, or 15 customers per queue


Reading the code:
1. Each vendor thread has a function for it to run on while it is active
      a. Vendor Functions: H0 has function high_price_seller(), M0 has function medium_price_seller0(), M1 has function medium_price_seller1(), etc.
      b. Vendor Functions all work the same way except:
            1. Each vendor function has its own queue to work with
            2. Each vendor function has a string as the name of the vendor as part of the code
            3. high_price_seller(), medium_price_seller(), and low_price_seller() all have different transaction times
2. For each vendor, the code first checks whether the Queue correspoding to them (all_vendors[x]) has any customers waiting
      a. For all_vendors [x],
            1. all_vendors[0] is queue for H0
            2. all_vendors[1] is queue for M0
            3. all_vendors[2] is queue for M1
            4. all_vendors[3] is queue for M2
            5. all_vendors[4] is queue for L0
                    .
                    .
                    .
            9. all_vendors[9] is queue for L5
3. If a customer is in a queue, the vendor uses pthread_mutex_lock(&seat_lock) to try to get the lock. All threads try to do this or they go to sleep until the lock is available.
4. Once a vendor gets the lock, the vendor checks if there are seats available (int spot = find_empty_seat('L');)
        a. if the returned value from find_empty_seat() is -1, all seats have been sold.
5. If there is a seat available, the vendor reserves the seat in the function:   take_spot(spot, "L0\0", customer);
        a. Inside the take_spot() function, if the vendor succesfully reserves a seat, the lock is released and a new thread can execute the above
        b. Also inside the take_spot() function, the updated seat_chart is printed out using the function show_seats()
6. Once the vendor exits the take_spot() function, the vendor waits for a specified number of seconds in a while loop that keeps track of the seconds that have passed since it first reserved a seat
7. The process continues for each thread until either all seats have been sold or the 60 seconds real-time have passed. 
8. The queue_thread method is used to control when the customers enter a specific vendor queue
        a. For a total of 60 seconds real-time, there are 10 conditional statements which decide whether a customer can come in the vendor queue by using a random number. If the random number equals 1, a customer is allowed to go inside the queue
9. For each round, step 8 is realized for all of the queues and then the while loop rests for anywhere from 1 to 3 seconds real-time before starting the next wave of customers
10. When the 60 seconds are up, the while loop is closed and a record of the transcations and rejections for each vendor is printed out to the file time_stamp.txt
11. Once the program is done executing, new table.txt and time_stamp.txt files are created in the same directory as the tickets.c file
