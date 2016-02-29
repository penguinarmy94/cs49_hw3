#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define true 1
#define false 0

/* a seat's id should say who sold it and customer number */
typedef struct seat
{
	char * id;
    int taken;
} seat;

/* the seats are critical, so a thread needs the lock to read/write */
seat seats[100];
int lowcus, midcus, highcus, seatstaken;
pthread_mutex_t seat_lock;

/* create an array of empty seats */
void init_seats()
{
	int i;
	for (i = 0; i < 100; ++i) {
		seat s = {"", 0};
		seats[i] = s;
	}
    lowcus = 0;
    midcus = 0;
    highcus = 0;
    seatstaken = 0;
}

/* print seating chart */
void show_seats()
{
	int row, column, i;
	for (i = row = 0; row < 10; ++row) {
        for (column = 0; column < 10; ++column) {
			if(seats[row*10 + column].taken == 0) {
                printf(" ----");
            }
            else {
                printf(" %s%03d", seats[row*10 + column].id, seats[row*10 + column].taken);
            }
        }
        printf("\n");
	}
}

/* check if a seat is empty */
int is_empty(seat s) {
    if(s.taken == 0) {
        return true;
    }
    return false;
}

/* find the first empty seat starting from the front*/
int find_empty_front_seat()
{
	int i;
	for (i = 0; i < 100; ++i) {
        seat s = seats[i];
        if (is_empty(s) == 1) {
            ++highcus;
            return i;
        }
	}
	return -1;
}

/* find the first empty seat starting from the middle*/
int find_empty_middle_seat()
{
	int i, j = 0, k = 1;
    for (i = 40; i < 100 && i > -1; ++i) {
        seat s = seats[i];
        if (is_empty(s) == 1) {
            ++midcus;
            return i;
        }
        if (i%10 == 9) {
            j++;
            k *= -1;
            i = i - 9 - k*j*10;
        }
    }
	return -1;
}

/* find the first empty seat starting from the back*/
int find_empty_back_seat()
{
    int i;
    for (i = 90; i > -1; ++i) {
        seat s = seats[i];
        if (is_empty(s) == 1) {
            ++lowcus;
            return i;
        }
	if (i % 10 == 9) {
           i = i - 20;
	}
    }
    return -1;
}

/* mark a specific seat as taken, using its index */
void take_spot(int i, char c)
{
	if (i >= 0 && i < 100) {
		seat *s = &seats[i];
        switch (c) {
            case ('L'):
            {
                s->id = "L\0";
                s->taken = lowcus;
                break;
            }
            case ('M'):
            {
                s->id = "M\0";
                s->taken = midcus;
                break;
            }
            case ('H'):
            {
                s->id = "H\0";
                s->taken = highcus;
                break;
            }
        }
        ++seatstaken;
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
	while (1) {
        pthread_mutex_lock(&seat_lock);
		int spot = find_empty_back_seat();
		take_spot(spot, 'L');
		pthread_mutex_unlock(&seat_lock);
		if (spot < 0) pthread_exit(NULL);
	}
}

/* middle price selling thread */
void *middle_price_seller(void *arg)
{
	while (1) {
        pthread_mutex_lock(&seat_lock);
		int spot = find_empty_middle_seat();
		take_spot(spot, 'M');
		pthread_mutex_unlock(&seat_lock);
		if (spot < 0) pthread_exit(NULL);
	}
}

/* high price selling thread */
void *high_price_seller(void *arg)
{
	while (1) {
        pthread_mutex_lock(&seat_lock);
		int spot = find_empty_front_seat();
		take_spot(spot, 'H');
		pthread_mutex_unlock(&seat_lock);
		if (spot < 0) pthread_exit(NULL);
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
	pthread_create(&threads[1], NULL, low_price_seller, NULL);
	pthread_create(&threads[2], NULL, low_price_seller, NULL);
	pthread_create(&threads[3], NULL, low_price_seller, NULL);
	pthread_create(&threads[4], NULL, low_price_seller, NULL);
	pthread_create(&threads[5], NULL, low_price_seller, NULL);
	pthread_create(&threads[6], NULL, middle_price_seller, NULL);
	pthread_create(&threads[7], NULL, middle_price_seller, NULL);
	pthread_create(&threads[8], NULL, middle_price_seller, NULL);
	pthread_create(&threads[9], NULL, high_price_seller, NULL);
    while(seatstaken < 100) {
        /* put random number generation and queue adding here */
    }
    show_seats();
    pthread_exit(NULL);
}