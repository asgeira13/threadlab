#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

void init()
{

}

void spawn_pedestrian(thread_info *arg)
{

}

void spawn_vehicle(thread_info *arg)
{

}

void *vehicles(void *arg)
{
    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = vehicle_arrive(info);
    vehicle_drive(info);
    vehicle_leave(info);

    return NULL;
}

void *pedestrians(void *arg)
{
    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = pedestrian_arrive(info);
    pedestrian_walk(info);
    pedestrian_leave(info);

    return NULL;
}


void clean()
{

}
