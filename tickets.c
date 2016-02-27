#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define TRUE        1
#define FALSE       0

/* a seat's id should say who sold it */
typedef struct seat
{
	char * id;
	int taken;
} seat;

/* the seats are critical, so a thread needs the lock to read/write */
seat seats[100];
pthread_mutex_t seat_lock;

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
			printf("%s ", seats[i++].id);
		}
		printf("%s\n", seats[i++].id);
	}
}

/* check if a seat is empty */
int is_empty(seat s) {
	return s.taken ? FALSE : TRUE;
}

/* find the first empty seat */
int find_empty_seat()
{
	int i;
	for (i = 0; i < 100; ++i) {
		seat s = seats[i];
		if (is_empty(s)) return i;
	}
	return -1;
}

/* mark a specific seat as taken, using its index */
void take_spot(int i)
{
	if (i >= 0 && i < 100) {
		seat *s = &seats[i];
		s->taken = TRUE;
		s->id = "GONE\0";
	}
}

/* generate a random number between low and high (inclusive) */
int random_num(int low, int high)
{
	return rand() % (high + 1 - low) + low;
}

/* low price selling thread */
void *low_price_seller(void *arg)
{
	while (TRUE) {
		pthread_mutex_lock(&seat_lock);
		int spot = find_empty_seat();
		take_spot(spot);
		pthread_mutex_unlock(&seat_lock);
		if (spot < 0) pthread_exit(NULL);
		show_seats();
	}
}

/* main thread */
int main(int argc, char **argv)
{
	/* program initialization */
	init_seats();
	srand(time(0));
	pthread_mutex_init(&seat_lock, NULL);
	
	/* start creating threads here */
	pthread_t threads[10];
	pthread_create(&threads[0], NULL, low_price_seller, NULL);
	
	pthread_exit(NULL);
}