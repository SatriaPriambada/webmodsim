#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "simlib.h"

#define STREAM_INTERARRIVAL   1	/* Random-number stream for interarrivals. */
#define SAMPST_DELAYS        1  /* sampst variable for delays in queue(s). */

#define SIMULATION_TIME 90 * 60
#define GROUP_MAX_SIZE 4
#define GROUP_ARRIVAL_TIME_MEAN 30

#define EVENT_GROUP_COME 1
#define EVENT_ARRIVE_COUNTER 2
#define EVENT_DEPART_COUNTER 3
#define EVENT_END_SIMULATION 4

#define NUM_COUNTER 4
#define COUNTER_HOT_FOOD 1
#define COUNTER_SANDWICH 2
#define COUNTER_DRINK 3
#define COUNTER_CASHIER 4

#define QUEUE_HOT_FOOD 1
#define QUEUE_SANDWICH 2
#define QUEUE_CASHIER 3

#define HOT_FOOD_MIN_ST 50
#define HOT_FOOD_MAX_ST 120
#define SANDWICH_MIN_ST 60
#define SANDWICH_MAX_ST 180
#define DRINK_MIN_ST 5
#define DRINK_MAX_ST 20

#define HOT_FOOD_MIN_ACT 20
#define HOT_FOOD_MAX_ACT 40
#define SANDWICH_MIN_ACT 5
#define SANDWICH_MAX_ACT 15
#define DRINK_MIN_ACT 5
#define DRINK_MAX_ACT 10

#define STREAM_GROUP_COUNT 3
#define STREAM_GROUP_ARRIVAL 2
#define STREAM_ROUTE_CHOICE 5
#define STREAM_SERVICE_TIME 6
#define STREAM_ACT_TIME 7

#define GROUP_1_PROBABILITY 0.5
#define GROUP_2_PROBABILITY 0.3
#define GROUP_3_PROBABILITY 0.1
#define GROUP_4_PROBABILITY 0.1

#define ROUTE_1_PROBABILITY 0.8
#define ROUTE_2_PROBABILITY 0.15
#define ROUTE_3_PROBABILITY 0.05

using namespace std;

int hot_food_people, sandwich_people, cashier_people, teller, shortest_length, shortest_queue;
bool counter_busy[NUM_COUNTER + 1];

int min_serve_time[NUM_COUNTER + 1];
int max_serve_time[NUM_COUNTER + 1];

int min_act_time[NUM_COUNTER + 1];
int max_act_time[NUM_COUNTER + 1];

int sum_counter_delay[NUM_COUNTER + 1];
int num_counter_delay[NUM_COUNTER + 1];
int max_counter_delay[NUM_COUNTER + 1];

//service time
int sum_st[NUM_COUNTER + 1];
int num_st[NUM_COUNTER + 1];
int max_st[NUM_COUNTER + 1];

int num_worker[NUM_COUNTER + 1];

FILE *infile, *outfile;

int getRandomNumber();

void group_come();

void arrive_counter();

void depart_counter();

void serve_counter();

void report();

int main() {

	min_serve_time[COUNTER_HOT_FOOD] = HOT_FOOD_MIN_ST;
	min_serve_time[COUNTER_SANDWICH] = SANDWICH_MIN_ST;
	min_serve_time[COUNTER_DRINK] = DRINK_MIN_ST;
	max_serve_time[COUNTER_HOT_FOOD] = HOT_FOOD_MAX_ST;
	max_serve_time[COUNTER_SANDWICH] = SANDWICH_MAX_ST;
	max_serve_time[COUNTER_DRINK] = DRINK_MAX_ST;

	min_act_time[COUNTER_HOT_FOOD] = HOT_FOOD_MIN_ACT;
	min_act_time[COUNTER_SANDWICH] = SANDWICH_MIN_ACT;
	min_act_time[COUNTER_DRINK] = DRINK_MIN_ACT;
	max_act_time[COUNTER_HOT_FOOD] = HOT_FOOD_MAX_ACT;
	max_act_time[COUNTER_SANDWICH] = SANDWICH_MAX_ACT;
	max_act_time[COUNTER_DRINK] = DRINK_MAX_ACT;

	for (int i = 1; i < NUM_COUNTER + 1; i++) {
		num_worker[i] = 1;
	}

	srand(time(NULL));

	infile = fopen ("cafeteria.in", "r");
	outfile = fopen ("cafeteria.out", "w");

	fscanf(infile, "%d %d %d", &num_worker[COUNTER_HOT_FOOD], &num_worker[COUNTER_SANDWICH], &num_worker[COUNTER_CASHIER]);

	init_simlib();

	sampst(0.0, SAMPST_DELAYS);

	// Schedule group come
	int cur_time = expon(GROUP_ARRIVAL_TIME_MEAN, STREAM_GROUP_ARRIVAL);
	while (cur_time < SIMULATION_TIME) {
		event_schedule(cur_time, EVENT_GROUP_COME);
		cur_time += expon(GROUP_ARRIVAL_TIME_MEAN, STREAM_GROUP_ARRIVAL);
	}

	// Schedule simulation end
	event_schedule(SIMULATION_TIME, EVENT_END_SIMULATION);

	do {
		timing();
		switch (next_event_type) {
			case EVENT_GROUP_COME:
				group_come();
				break;
			case EVENT_ARRIVE_COUNTER:
				arrive_counter();
				break;
			case EVENT_DEPART_COUNTER:
				depart_counter();
				break;
		}
	} while (next_event_type != EVENT_END_SIMULATION);

	cout << "=========================" << endl;
	for (int i = 1; i <= NUM_COUNTER + num_worker[COUNTER_CASHIER]; i++) {
		if (num_counter_delay[i] != 0) {
			cout << i << " max delay: " << max_counter_delay[i] << " avg delay: " <<
				sum_counter_delay[i] / num_counter_delay[i] << endl;
			cout << i << " max serve_time: " << max_st[i] << " avg serve_time: " <<
				sum_st[i] / num_st[i] << endl;
		}
	}
	report();
	fclose(infile);
	fclose(outfile);
}

/**
 * Note to transfer variable:
 * transfer[3] = counter_number
 * transfer[4] = start queue-ing time
 * transfer[5] = accumulated time
 */

void group_come() {
	int num_people;

	double rand_num_group = uniform(0, 1, STREAM_GROUP_COUNT);
	if (rand_num_group < GROUP_1_PROBABILITY) {
		num_people = 1;
	} else if (rand_num_group < GROUP_1_PROBABILITY + GROUP_2_PROBABILITY) {
		num_people = 2;
	} else if (rand_num_group < GROUP_1_PROBABILITY + GROUP_2_PROBABILITY +  GROUP_3_PROBABILITY) {
		num_people = 3;
	} else {
		num_people = 4;
	}

	for (int i = 0; i < num_people; i++) {
		int route_choice;

		double rand_route_choice = uniform(0, 1, STREAM_ROUTE_CHOICE);
		if (rand_route_choice < ROUTE_1_PROBABILITY) {
			route_choice = COUNTER_HOT_FOOD;
		} else if (rand_route_choice < ROUTE_1_PROBABILITY + ROUTE_2_PROBABILITY) {
			route_choice = COUNTER_SANDWICH;
		} else {
			route_choice = COUNTER_DRINK;
		}
		transfer[3] = route_choice;
		transfer[4] = sim_time;
		transfer[5] = 0;

		event_schedule(sim_time, EVENT_ARRIVE_COUNTER);
	}
}

void arrive_counter() {
	int counter_number = transfer[3];
	int accumulated_time = transfer[5];
	transfer[4] = sim_time;

	if (counter_busy[counter_number] && (counter_number != COUNTER_DRINK)) {
		if (counter_number == COUNTER_CASHIER){
			//find the shortest queue
		    shortest_length = list_size[NUM_COUNTER];
		    shortest_queue  = NUM_COUNTER;
		    for (teller = NUM_COUNTER; teller > NUM_COUNTER - num_worker[COUNTER_CASHIER]; --teller){
		        if (list_size[teller] < shortest_length) {
		            shortest_length = list_size[teller];
		            shortest_queue  = teller;
		        }
		    }
		    transfer[3] = counter_number;
			transfer[5] = accumulated_time;
			//cout << "using cashier " << shortest_queue << endl;
			list_file(LAST, shortest_queue);

		} else {
			transfer[3] = counter_number;
			transfer[5] = accumulated_time;
			list_file(LAST, counter_number);
		}
	} else {
		
		serve_counter();
	}
}

void depart_counter() {
	int counter_number = transfer[3];

	if ((counter_number == COUNTER_HOT_FOOD) || (counter_number == COUNTER_SANDWICH)) {
		transfer[3] = COUNTER_DRINK;
	} else if (counter_number == COUNTER_DRINK) {
		transfer[3] = COUNTER_CASHIER;
	}

	sum_counter_delay[counter_number] += 0;
	num_counter_delay[counter_number]++;
	if (counter_number != COUNTER_CASHIER) {
		event_schedule(sim_time, EVENT_ARRIVE_COUNTER);
	}

	//cout << counter_number << " " << list_size[counter_number] << endl;
	if (list_size[counter_number] == 0) {
		counter_busy[counter_number] = false;
	} else {
		list_remove(FIRST, counter_number);
		sampst(sim_time - transfer[1], SAMPST_DELAYS);
		serve_counter();
	}
}

void serve_counter() {
	int counter_number = transfer[3];
	int accumulated_time = transfer[5];

	if (counter_number != COUNTER_CASHIER) {
		double serve_time = uniform(min_serve_time[counter_number], max_serve_time[counter_number], STREAM_SERVICE_TIME) / num_worker[counter_number];
		double counter_act_time = uniform(min_act_time[counter_number], max_act_time[counter_number], STREAM_ACT_TIME);

		transfer[3] = counter_number;
		transfer[5] = accumulated_time + counter_act_time;

		counter_busy[counter_number] = true;

		sum_counter_delay[counter_number] += sim_time - transfer[4];
		num_counter_delay[counter_number]++;
		max_counter_delay[counter_number] = max((int) (sim_time - transfer[4]), max_counter_delay[counter_number]);

		sum_st[counter_number] += serve_time;
		num_st[counter_number]++;
		max_st[counter_number] = max((int) (serve_time), max_st[counter_number]);

		event_schedule(sim_time + serve_time, EVENT_DEPART_COUNTER);
	} else {
		double serve_time = transfer[5];
		counter_busy[counter_number] = true;

		sum_counter_delay[counter_number] += sim_time - transfer[4];
		num_counter_delay[counter_number]++;
		max_counter_delay[counter_number] = max((int) (sim_time - transfer[4]), max_counter_delay[counter_number]);

		//get service time from act in transfer 5
		sum_st[counter_number] += transfer[5];
		num_st[counter_number]++;
		max_st[counter_number] = max((int) (transfer[5]), max_st[counter_number]);

		event_schedule(sim_time + serve_time, EVENT_DEPART_COUNTER);
	}
}

void report(void)  /* Report generator function. */
{
    int   worker;
    float avg_num_in_queue_hotfood, avg_num_in_queue_sandwich, avg_num_in_queue_drink, avg_num_in_queue_cashier;

    /* Compute and write out estimates of desired measures of performance. */

    avg_num_in_queue_hotfood += filest(COUNTER_HOT_FOOD);
    avg_num_in_queue_sandwich += filest(COUNTER_SANDWICH);
    avg_num_in_queue_drink += filest(COUNTER_DRINK);
    avg_num_in_queue_cashier += filest(COUNTER_CASHIER);

    fprintf(outfile, "\n\nWith hotfood%2d workers, average number in queue = %10.3f",
            num_worker[COUNTER_HOT_FOOD], avg_num_in_queue_hotfood);
    fprintf(outfile, "\n\nWith sandwich%2d workers, average number in queue = %10.3f",
            num_worker[COUNTER_SANDWICH], avg_num_in_queue_sandwich);
    /*fprintf(outfile, "\n\nWith drink%2d workers, average number in queue = %10.3f",
            num_worker[COUNTER_DRINK], avg_num_in_queue_drink);*/
    fprintf(outfile, "\n\nWith cashier%2d workers, average number in queue = %10.3f",
            num_worker[COUNTER_CASHIER], avg_num_in_queue_cashier);
    fprintf(outfile, "\n\nDelays in queue, in seconds:\n");
    out_sampst(outfile, SAMPST_DELAYS, SAMPST_DELAYS);
}

