#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define TRUE        1
#define FALSE       0

typedef struct seat
{
	char * id;
	int taken;
} seat;

seat seats[100];

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

/* generate a random number between low and high (inclusive) */
int random_num(int low, int high)
{
	return rand() % (high + 1 - low) + low;
}

/* main thread */
int main(int argc, char **argv)
{
	/* program initialization */
	init_seats();
	srand(time(0));
	
	/* start creating threads here */
	
	pthread_exit(NULL);
}