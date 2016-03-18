#include "crossing.h"

/**
 * start_wait takes in the type of thread that is calling it and returns a number.
 *
 * This number will be the internal order the checker expects to see for crossing threads of the same type
 */
int start_wait(thread_info *info)
{
    P(&queue_mtx);
    tail[info->crossing]++;
    int local = tail[info->crossing];
    fprintf(stdout, "[TL]: wait %d %d queue_nr %d\n", info->crossing, info->thread_nr, local);
    V(&queue_mtx);
    return local;
}

/**
 * pedestrian_arrive annouces that the pedestrian has arrived at the crossing.
 */
int pedestrian_arrive(thread_info *info)
{
    return start_wait(info);
}

/**
 * pedestrian_walk makes this thread start his walk across the road.
 *
 * The thread simulates the pedestrian crossing the road by sleeping for a random period.
 */
void pedestrian_walk(thread_info *info)
{
    fprintf(stdout, "[TL]: cross pedestrian %d start nr %d\n", info->crossing, info->thread_nr);
    rand_sleep(walk_time);
}

/**
 * pedestrian_leave announces that the pedestrian has finished his crossing
 */
void pedestrian_leave(thread_info *info)
{
    fprintf(stdout, "[TL]: cross pedestrian %d end nr %d\n", info->crossing, info->thread_nr);
}

/**
 * vehicle_arrive annouces that the pedestrian has arrived at the crossing.
 */
int vehicle_arrive(thread_info *info)
{
    return start_wait(info);
}

/**
 * vehicle_drive makes this thread drive across the intersection.
 *
 * The thread simulates the vehicle driving by sleeping for a random period.
 */
void vehicle_drive(thread_info *info)
{
    fprintf(stdout, "[TL]: cross vehicle %d start nr %d\n", info->crossing, info->thread_nr);
    rand_sleep(drive_time);
}

/**
 * vehicle_arrive announces that the vehicle has finished his crossing.
 */
void vehicle_leave(thread_info *info)
{
    fprintf(stdout, "[TL]: cross vehicle %d end nr %d\n", info->crossing, info->thread_nr);
}


/* Random sleep */
void rand_sleep(int ms)
{
    long sleep_factor = rand() % 10;
    usleep((ms * 1000) + (sleep_factor * 1000));
}

void cross_init(int num_vehicles, int num_pedestrians)
{
    Sem_init(&queue_mtx, 0, 1);
    drive_time = 20;
    walk_time = 40;
    int i;
    for (i = 0; i < 4; ++i) {
        tail[i] = 0;
    }
    fprintf(stdout, "[TL]: Starting up the Traffic Controllatron with %d vehicles and %d pedestrians.\n",
            num_vehicles,
            num_pedestrians);
}
