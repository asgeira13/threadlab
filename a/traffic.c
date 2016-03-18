#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

void *vehicles(void *arg);
void *pedestrians(void *arg);

sem_t thread_mutex;

pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;

void init()
{
    sem_init(&thread_mutex, 0, 1);
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
    
}

void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
}

void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
}

void *vehicles(void *arg)
{
    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = vehicle_arrive(info);
    P(&thread_mutex);
    vehicle_drive(info);
    vehicle_leave(info);
    V(&thread_mutex);
    return NULL;
}

void *pedestrians(void *arg)
{
    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = pedestrian_arrive(info);
    P(&thread_mutex);
    pedestrian_walk(info);
    pedestrian_leave(info);
    V(&thread_mutex);
    return NULL;
}


void clean()
{
    for (int i = 0; i < num_pedestrians; i++) {
	Pthread_join(pedestrian_thread[i], NULL);
    }
    for(int i = 0; i < num_vehicles; i++) {
	Pthread_join(vehicle_thread[i], NULL);
    }
}
