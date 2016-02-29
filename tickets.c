#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#define TRUE         1
#define FALSE        0
#define SEAT_ROWS    10
#define SEAT_COLUMNS 10

struct seat
{
	char *seller;
	int customer_number;
	int free;
};

enum customer_status
{
	NOT_HERE, WAITING, SELLING
};

struct customer
{
	enum customer_status status;
};

struct customer_args
{
	struct customer *line;
	pthread_mutex_t *line_lock;
	char *seller_name;
	int start_time;
};

struct seat seats[SEAT_ROWS][SEAT_COLUMNS];
int n; // number of people in each queue

pthread_mutex_t seats_lock;
pthread_mutex_t output_lock;

/* create an array of empty seats */
void init_seats()
{
	int row, column;
	for (row = 0; row < SEAT_ROWS; ++row) {
		for (column = 0; column < SEAT_COLUMNS; ++column) {
			seats[row][column] = (struct seat) {"--", 0, TRUE};
		}
	}
}

void print_seat(struct seat s)
{
	if (s.free)
		printf("----");
	else
		printf("%s%02d", s.seller, s.customer_number);
}

/* print seating chart */
void show_seats()
{
	int row, column;
	for (row = 0; row < SEAT_ROWS; ++row) {
		for (column = 0; column < SEAT_COLUMNS; ++column) {
			print_seat(seats[row][column]);
			printf(" ");
		}
		printf("\n");
	}
}

struct seat *find_seat(int i) {
	int row = i / SEAT_ROWS;;
	int column = i % SEAT_COLUMNS;
	return &seats[row][column];
}

int find_seat_from_back() {
	int row, column, count;
	count = SEAT_ROWS * SEAT_COLUMNS - 1;
	for (row = SEAT_ROWS - 1; row >= 0; --row) {
		for (column = SEAT_COLUMNS  - 1; column >= 0; --column) {
			if (seats[row][column].free) {
				struct seat *s = &seats[row][column];
				s->free = FALSE;
				return count;
			}
			--count;
		}
	}
	return -1;
}

/* generate a random number between low and high (inclusive) */
int random_num(int low, int high)
{
	return rand() % (high + 1 - low) + low;
}

/* customer thread */
void *customer_thread(void *arg)
{
	struct customer_args *args = (struct customer_args *) arg;
	sleep(args->start_time);
	
	pthread_mutex_lock(args->line_lock);
	int i;
	for (i = 0; args->line[i].status != NOT_HERE; ++i);
	struct customer *person = &args->line[i];
	person->status = WAITING;
	pthread_mutex_unlock(args->line_lock);
	pthread_mutex_lock(&output_lock);
	printf("[0:%02d] customer entered %s's line\n",
		args->start_time, args->seller_name);
	pthread_mutex_unlock(&output_lock);
	pthread_exit(NULL);
}

/* low price seller thread */
void *low_seller_thread(void *arg)
{
	char *seller_name = (char *) arg;
	/*
	use memory address as seed. if we use time, the seed will be the same
	for every thread. my system is 64-bit, so i cast it to a long long
	*/
	srand((long long) seller_name); 
	pthread_t *customers = calloc(n, sizeof(pthread_t));
	struct customer *line = calloc(n, sizeof(struct customer));
	pthread_mutex_t *line_lock = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(line_lock, NULL);
	
	int i;
	pthread_mutex_lock(line_lock);
	for (i = 0; i < n; ++i) {
		struct customer_args *args = malloc(sizeof(struct customer_args));
		struct customer *person = &line[i];
		person->status = NOT_HERE;
		args->line = line;
		args->line_lock = line_lock;
		args->seller_name = seller_name;
		args->start_time = random_num(0, 59);
		pthread_create(&customers[i], NULL, customer_thread, args);
	}
	pthread_mutex_unlock(line_lock);
	
	int time = 0;
	int count = 0;
	while (TRUE) {
		pthread_mutex_lock(line_lock);
		int i;
		if (time > 59) break;
		for (i = 0; line[i].status != WAITING && i < n; ++i);
		pthread_mutex_unlock(line_lock);
		if (i >= n) { // no customers in line
			sleep(1);
			++time;
			continue;
		}
		else {
			++count;
			struct customer *person = &line[i];
			int purchase_time = random_num(4, 7);
			pthread_mutex_lock(&seats_lock);
			int spot = find_seat_from_back();
			pthread_mutex_unlock(&seats_lock);
			if (spot < 0) { // no available seats
				person->status = NOT_HERE;
				pthread_mutex_lock(&output_lock);
				printf("[0:%02d] no seats. %s is turning customer %d away\n",
					time, seller_name, count);
				pthread_mutex_unlock(&output_lock);
			}
			else {
				person->status = SELLING;
				struct seat *s = find_seat(spot);
				s->seller = seller_name;
				s->customer_number = count;
				pthread_mutex_lock(&output_lock);
				printf("[0:%02d] %s is selling to customer %d\n",
					time, seller_name, count);
				pthread_mutex_unlock(&output_lock);
				time += purchase_time;
				sleep(purchase_time);
				
				pthread_mutex_lock(&output_lock);
				printf("[0:%02d] %s finished selling to customer %d\n",
					time, seller_name, count);
				show_seats();
				pthread_mutex_unlock(&output_lock);
			}
		}
	}
	pthread_exit(NULL);
}

/* main thread */
int main(int argc, char **argv)
{
	/* program initialization */
	init_seats();
	sscanf(argv[1], "%d", &n);
	pthread_mutex_init(&seats_lock, NULL);
	pthread_mutex_init(&output_lock, NULL);
	
	/* start creating threads here */
	pthread_t *sellers = calloc(10, sizeof(pthread_t));
	pthread_create(&sellers[4], NULL, low_seller_thread, "L1");
	pthread_create(&sellers[5], NULL, low_seller_thread, "L2");
	pthread_create(&sellers[6], NULL, low_seller_thread, "L3");
	pthread_create(&sellers[7], NULL, low_seller_thread, "L4");
	pthread_create(&sellers[8], NULL, low_seller_thread, "L5");
	pthread_create(&sellers[9], NULL, low_seller_thread, "L6");
	
	pthread_exit(NULL);
}