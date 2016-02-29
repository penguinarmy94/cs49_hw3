/*
 * Note: Output is in two places:
 * 1. table.txt for the seating chart
 * 2. time_stamp.txt for the time stamp
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define TRUE        1
#define FALSE       0

/* a seat for the seating chart */
typedef struct seat
{
	char *id;
	int taken;
} seat;


typedef struct Queue
{
        int capacity;
        int size;
        int front;
        int rear;
        int *elements;
}Queue;

/* the seats are critical, so a thread needs the lock to read/write */
seat seats[100];
pthread_mutex_t seat_lock;
clock_t start;
Queue *all_vendors [10];
int sold_out = FALSE;
FILE *time_stamps;
FILE *table;
int rejected[10] = {0};
int sold [10] = {0};
int vendors_are_done = 0;

/* Functions for use with the Queue data structure*/
Queue * createQueue(int maxElements);
void Dequeue(Queue *Q);
int RemoveFront(Queue *Q);
void Enqueue(Queue *Q,int element);
int isEmpty (Queue *Q);
void init_queue (int N);

/* Functions for use with the seating chart*/
void init_seats ();
void show_seats ();
int is_empty (seat s);
int find_empty_seat(char letter);
void take_spot(int i, char *letter, int cusnum);
int random_num(int low, int high);
void add_to_sold_list (char *letter);

/* Low price vendor functions for L0 L1 L2 L3 L4 L5*/
void *low_price_seller0(void *arg);
void *low_price_seller1(void *arg);
void *low_price_seller2(void *arg);
void *low_price_seller3(void *arg);
void *low_price_seller4(void *arg);
void *low_price_seller5(void *arg);

/* Medium price vendor functions for M0 M1 M3*/
void *medium_price_seller0(void *arg);
void *medium_price_seller1(void *arg);
void *medium_price_seller2(void *arg);

/* High price vendor functions*/
void *high_price_seller(void *arg);

/* Customer thread (one thread for adding customers to queue)*/
void *queue_thread(void *arg);

/* main thread */
int main(int argc, char **argv)
{
	int N;
	if (argc < 2) {
		fprintf(stderr, "need to supply a command-line argument\n");
		exit(1);
	}
	/* program initialization */
	init_seats();
	sscanf(argv[1], "%d", &N);
	pthread_mutex_init(&seat_lock, NULL);
	table = fopen("table.txt", "w");
	if (table == NULL) {
		printf("Nothing found");
	}
	time_stamps = fopen("time_stamp.txt", "w");
	if (time_stamps == NULL) {
		printf("Nothing found");
	}

	/* start creating threads here */
	pthread_t L[6], M[3], H, Q;
	start = clock();
	init_queue (N);
	pthread_create(&Q, NULL, queue_thread, NULL);

	pthread_create(&L[0], NULL, low_price_seller0, NULL);
	pthread_create(&L[1], NULL, low_price_seller1, NULL);
	pthread_create(&L[2], NULL, low_price_seller2, NULL);
	pthread_create(&L[3], NULL, low_price_seller3, NULL);
	pthread_create(&L[4], NULL, low_price_seller4, NULL);
	pthread_create(&L[5], NULL, low_price_seller5, NULL);

	pthread_create(&M[0], NULL, medium_price_seller0, NULL);
	pthread_create(&M[1], NULL, medium_price_seller1, NULL);
	pthread_create(&M[2], NULL, medium_price_seller2, NULL);

	pthread_create(&H, NULL, high_price_seller, NULL);

	pthread_exit(NULL);

	return (0);
}

/* creates a queue with N elements (5, 10, 15)*/
Queue * createQueue(int maxElements)
{
        /* Create a Queue */
        Queue *Q;
        Q = (Queue *)malloc(sizeof(Queue));
        /* Initialise its properties */
        Q->elements = (int *)malloc(sizeof(int)*maxElements);
        Q->size = 0;
        Q->capacity = maxElements;
        Q->front = 0;
        Q->rear = -1;
        /* Return the pointer */
        return Q;
}

/* removes an element from the queue*/
void Dequeue(Queue *Q)
{
        if(Q->size == 0)
        {
           return;
        }
        else
        {
                Q->size--;
                Q->front++;
                /* As we fill elements in circular fashion */
                if(Q->front == Q->capacity)
                {
                        Q->front = 0;
                }
        }
        return;
}

/* peek at the front element to grab its value*/
int RemoveFront(Queue *Q)
{
        if(Q->size == 0)
        {
           return -1;
        }
        /* Return the element which is at the front*/
        return Q->elements[Q->front];
}

/* add an element to the back of the queue in a circular fashion*/
void Enqueue(Queue *Q,int element)
{
        /* If the Queue is full, we cannot push an element into it as there is no space for it.*/
        if(Q->size == Q->capacity)
        {
                printf("Queue is Full\n");
        }
        else
        {
                Q->size++;
                Q->rear = Q->rear + 1;
                /* As we fill the queue in circular fashion */
                if(Q->rear == Q->capacity)
                {
                    Q->rear = 0;
                }
                /* Insert the element in its rear side */
                Q->elements[Q->rear] = element;
        }
        return;
}

/* checks if a queue has no filled elements*/
int isEmpty (Queue *Q) {
	if (Q->size == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

/* initializes all the queues needed for the 10 vendors*/
void init_queue(int N) {
	int i;
	for (i = 0; i < 10; i++) {
		all_vendors [i] = createQueue (N);
	}
}

/* create an array of empty seats */
void init_seats()
{
	int i;
	for (i = 0; i < 100; ++i) {
		seat s = {"----\0", FALSE};
		seats[i] = s;
	}
}

/* print seating chart */
void show_seats()
{
	int row, column, i;
	for (i = row = 0; row < 10; ++row) {
		for (column = 0; column < 9; ++column) {
			fprintf(table, "%s ", seats[i++].id);
		}
		fprintf(table, "%s\n", seats[i++].id);
	}
	fprintf(table, "\n\n");
	fflush (stdout);
}

/* check if a seat is empty */
int is_empty(seat s) {
	return s.taken ? FALSE : TRUE;
}

/* find the first empty seat */
int find_empty_seat(char letter)
{
	int seatnum, rownum, m_index = 0;
	int m_seat_finder [10] = {40,50,30,60,70,20,80,90,10,0};
	if (letter == 'H') {
		for (seatnum = 0; seatnum < 100; ++seatnum) {
			seat s = seats[seatnum];
			if (is_empty(s)) return seatnum;
		}
	}
	else if (letter == 'M') {
		rownum = m_seat_finder [m_index++];
		for (seatnum = rownum; seatnum < rownum + 10 ; ++seatnum) {
			seat s = seats[seatnum];
			if (is_empty(s)) return seatnum;
			if (seatnum == rownum + 9 && m_index < 10) {
				rownum = m_seat_finder [m_index++];
				seatnum = rownum - 1;
			}
		}
	}
	else {
		for (seatnum = 99; seatnum >= 0; --seatnum) {
			seat s = seats[seatnum];
			if (is_empty(s)) return seatnum;
		}
	}

	return -1;
}

/* mark a specific seat as taken, using its index */
void take_spot(int i, char *letter, int cusnum)
{
	if (i >= 0 && i < 100) {
		seat *s = &seats[i];
		s->taken = TRUE;
		char *string = (char *)malloc(6);
		if (cusnum < 10) {
			sprintf(string, "%s0%i", letter, cusnum);
		}
		else {
			sprintf(string, "%s%i", letter, cusnum);
		}
		s->id = string;
		add_to_sold_list (letter);
	}
	show_seats();
	pthread_mutex_unlock(&seat_lock);
}

/* adds a point to each vendor for each ticket sold to a customer*/
void add_to_sold_list (char *letter) {
	if (strcmp(letter,"H0") == 0) {sold[0]++;}
	else if (strcmp(letter,"M0") == 0) {sold[1]++;}
	if (strcmp(letter,"M1") == 0) {sold[2]++;}
	else if (strcmp(letter,"M2") == 0) {sold[3]++;}
	else if (strcmp(letter,"L0") == 0) {sold[4]++;}
	else if (strcmp(letter,"L1") == 0) {sold[5]++;}
	else if (strcmp(letter,"L2") == 0) {sold[6]++;}
	else if (strcmp(letter,"L3") == 0) {sold[7]++;}
	else if (strcmp(letter,"L4") == 0) {sold[8]++;}
	else if (strcmp(letter,"L5") == 0) {sold[9]++;}
}
/* generate a random number between low and high (inclusive) */
int random_num(int low, int high)
{
	//srand(clock()/CLOCKS_PER_SEC);
	rand();
	int random_number = rand();
	int to_mod = (high + 1 - low);
	int the_mod = random_number%to_mod;
	return  the_mod  + low;
}

/* low price vendor L0 */
void *low_price_seller0(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[4]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors[4]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L0 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L0 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[4]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L0 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L0\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L0\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L0\n",
							time, customer);
		}
	}
	return 0;
}

/* low price vendor L1*/
void *low_price_seller1(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[5]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [5]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L1 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L1 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[5]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L1 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L1\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L1\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L1\n",
							time, customer);
		}
	}
	return 0;
}

/* low price vendor L2*/
void *low_price_seller2(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[6]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [6]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L2 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L2 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[6]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L2 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L2\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L2\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L2\n",
							time, customer);
		}
	}
	return 0;
}

/* low price vendor L3*/
void *low_price_seller3(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[7]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [7]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L3 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L3 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[7]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L3 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L3\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L3\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L3\n",
							time, customer);
		}
	}
	return 0;
}

/* low price vendor L4*/
void *low_price_seller4(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[8]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [8]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L4 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L4 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[8]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L4 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L4\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L4\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L4\n",
							time, customer);
		}
	}
	return 0;
}

/* low price vendor L5*/
void *low_price_seller5(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[9]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [9]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] L5 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] L5 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('L');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[9]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] L5 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "L5\0", customer);
		int customer_wait_time = random_num(4, 7);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at L5\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at L5\n",
							time, customer);
		}
	}
	return 0;
}

/* mid price vendor M0*/
void *medium_price_seller0(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[1]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [1]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] M0 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] M0 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('M');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[1]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] M0 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "M0\0", customer);
		int customer_wait_time = random_num(2, 4);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at M0\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at M0\n",
							time, customer);
		}
	}
	return 0;
}

/* mid price vendor M1*/
void *medium_price_seller1(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[2]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [2]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] M1 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] M1 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('M');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[2]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] M1 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "M1\0", customer);
		int customer_wait_time = random_num(2, 4);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at M1\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at M1\n",
							time, customer);
		}
	}
	return 0;
}

/* mid price vendor M2*/
void *medium_price_seller2(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[3]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [3]);
		pthread_mutex_lock(&seat_lock);
		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] m2 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] M2 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('M');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[3]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] M2 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "M2\0", customer);
		int customer_wait_time = random_num(2, 4);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at M2\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at M2\n",
							time, customer);
		}
	}
	return 0;
}

/* high price vendor H0 */
void *high_price_seller(void *arg)
{
	int customer = 0;
	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		customer = RemoveFront(all_vendors[0]);
		if (customer == -1) {
			continue;
		}
		Dequeue (all_vendors [0]);
		pthread_mutex_lock(&seat_lock);

		int time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] H0 is selling to customer %i\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] H0 is selling to customer %i\n",
							time, customer);
		}
		clock_t s = clock();
		int spot = find_empty_seat('H');
		if (spot < 0) {
			pthread_mutex_unlock(&seat_lock);
			rejected[0]++;
			vendors_are_done++;
			fprintf(time_stamps, "[0:%i] H0 is sold out, customer %i leaves\n",
										time, customer);
			if (!sold_out) {
				printf("SOLD OUT\n");
				fflush(stdout);
				sold_out = TRUE;
			}
			pthread_exit(NULL);
			break;
		}
		take_spot(spot, "H0\0", customer);
		int customer_wait_time = random_num(1, 2);
		while ((clock() - s)/CLOCKS_PER_SEC < customer_wait_time) {
			//ticket wait
		}
		time = (clock() - start)/CLOCKS_PER_SEC;
		if ((clock() - start)/CLOCKS_PER_SEC < 10) {
			fprintf(time_stamps, "[0:0%i] Customer %i completed their purchase at H0\n",
							time, customer);
		}
		else {
			fprintf(time_stamps, "[0:%i] Customer %i completed their purchase at H0\n",
							time, customer);
		}
	}
	return 0;
}

/* thread for letting in customers into the 10 vendor queues*/
void *queue_thread(void *arg) {
	int h = 0, m1 = 0, m2 = 0, m3 = 0,
			l1 = 0, l2 = 0, l3 = 0, l4 = 0, l5 = 0, l6 = 0;
	int max_size = all_vendors[0]->capacity;

	while ((clock() - start)/CLOCKS_PER_SEC < 60) {
		int time = (clock() - start)/CLOCKS_PER_SEC;
		printf("TIME: %i\n", time);
		fflush(stdout);
		int to_pop = random_num(0,1);
		if (to_pop == TRUE && h < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered H0's line\n",
								time, h);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered H0's line\n",
								time, h);
			}
			Enqueue(all_vendors[0], h);
			h++;
		}
		to_pop = random_num(0,1);
		if (to_pop == TRUE && h < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered H0's line\n",
								time, h);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered H0's line\n",
								time, h);
			}
			Enqueue(all_vendors[0], h);
			h++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && m1 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered M0's line\n",
								time, m1);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered M0's line\n",
								time, m1);
			}
			Enqueue(all_vendors[1], m1);
			m1++;
		}
		to_pop = random_num(0,1);
		if (to_pop == TRUE && m2 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered M1's line\n",
								time, m2);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered M1's line\n",
								time, m2);
			}
			Enqueue(all_vendors[2], m2);
			m2++;
		}
		to_pop = random_num(0,1);
		if (to_pop == TRUE && m3 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered M2's line\n",
								time, m3);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered M2's line\n",
								time, m3);
			}
			Enqueue(all_vendors[3], m3);
			m3++;
		}


		to_pop = random_num(0,1);
		if (l1 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L0's line\n",
								time, l1);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L0's line\n",
								time, l1);
			}
			Enqueue(all_vendors[4], l1);
			l1++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && l2 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L1's line\n",
								time, l2);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L1's line\n",
								time, l2);
			}
			Enqueue(all_vendors[5], l2);
			l2++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && l3 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L2's line\n",
								time, l3);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L2's line\n",
								time, l3);
			}
			Enqueue(all_vendors[6], l3);
			l3++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && l4 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L3's line\n",
								time, l4);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L3's line\n",
								time, l4);
			}
			Enqueue(all_vendors[7], l4);
			l4++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && l5 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L4's line\n",
								time, l5);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L4's line\n",
								time, l5);
			}
			Enqueue(all_vendors[8], l5);
			l5++;
		}

		to_pop = random_num(0,1);
		if (to_pop == TRUE && l6 < max_size) {
			int time = (clock() - start)/CLOCKS_PER_SEC;
			if ((clock() - start)/CLOCKS_PER_SEC < 10) {
				fprintf(time_stamps, "[0:0%i] Customer %i entered L5's line\n",
								time, l6);
			}
			else {
				fprintf(time_stamps, "[0:%i] Customer %i entered L5's line\n",
								time, l6);
			}
			Enqueue(all_vendors[9], l6);
			l6++;
		}


		int buffer = random_num(1, 3);
		clock_t buff = clock();
		while ((clock() - buff)/CLOCKS_PER_SEC < buffer) {}

		if (sold_out) {
			break;
		}
	}
	int time = (clock() - start)/CLOCKS_PER_SEC;
	while (vendors_are_done != 10 && time < 60) {
		time = (clock() - start)/CLOCKS_PER_SEC;
		printf("TIME: %i\n", time);
		fflush(stdout);

		clock_t buff = clock();
		while ((clock() - buff)/CLOCKS_PER_SEC < 1) {}
	}
	 time = (clock() - start)/CLOCKS_PER_SEC;
		printf("END TIME: %i\n", time);
		fflush(stdout);
		int left, k;
		for (k = 0; k < 10; k++) {
			left = all_vendors[k]->size;
			rejected[k] += left;
		}
		fprintf(time_stamps, "\nRejected customers\n"
				"H0: %i\n"
				"M0: %i\n"
				"M1: %i\n"
				"M2: %i\n"
				"L0: %i\n"
				"L1: %i\n"
				"L2: %i\n"
				"L3: %i\n"
				"L4: %i\n"
				"L5: %i\n"
				"\nCustomers successfully served\n"
				"H0: %i\n"
				"M0: %i\n"
				"M1: %i\n"
				"M2: %i\n"
				"L0: %i\n"
				"L1: %i\n"
				"L2: %i\n"
				"L3: %i\n"
				"L4: %i\n"
				"L5: %i\n",
				rejected[0], rejected[1], rejected[2],
				rejected[3], rejected[4], rejected[5],
				rejected[6], rejected[7], rejected[8],
				rejected[9], sold[0], sold[1], sold[2],
				sold[3], sold[4], sold[5], sold[6],
				sold[7], sold[8], sold[9]);
		pthread_exit(NULL);

	return 0;
}
