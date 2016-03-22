#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

int v_arr [20];
int p_arr [20];

int n = 5;
int j = 0;

int v_head = 0;
int v_tail = 0;

int p_head = 0;
int p_tail = 0;

int basecondition = 0; //helps with the first car or person that wants to pass

int sensor = 0; //counts number of people or cars that have passed during light

void *vehicles(void *arg);
void *pedestrians(void *arg);

sem_t * mutexv_arr; // arrays that store semophores for individual vehicles and pedestrians
sem_t * mutexp_arr; // these help with deciding when to let them through
sem_t light_mutex;
sem_t thread_mutex;// binary sempaphore for locking
sem_t red_vehicle;
sem_t red_pedestrian;

pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;



void init()
{
    sem_init(&light_mutex, 0, 1);
    sem_init(&thread_mutex, 0, 1);
    //sem_init(&red_vehicle, 0, 0);
    //sem_init(&red_pedestrian, 0, 5);
    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
    mutexv_arr = malloc(sizeof(sem_t) * num_vehicles);
    mutexp_arr = malloc(sizeof(sem_t) * num_vehicles);

    for(int v = 0; v < num_vehicles; ++v)
    {
	sem_init(&mutexv_arr[v], 0, 0);
    }
    for(int p = 0; p < num_pedestrians; ++p)
    {
	sem_init(&mutexp_arr[p], 0, 0);
    }
}

void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
    p_tail++;
}

void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
    v_tail++;
}

void *vehicles(void *arg)
{
    
    thread_info *info = arg;
    
    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = vehicle_arrive(info);
    P(&light_mutex);
    if(!basecondition){ // base condition to initiate the first thread
	V(&mutexv_arr[v_head]);
	basecondition++;
//	fprintf(stdout, "\nvehicle base condition\n");

    }
    V(&light_mutex);
    
    P(&mutexv_arr[place-1]);
    
    
    vehicle_drive(info);
    vehicle_leave(info);
    v_head++;
    sensor++;
    if(sensor >= K || v_head == v_tail)
    {
	fprintf(stdout,"\n\nentered vehicle! sensor = %d, v_head = %d, v_tail = %d \n\n",sensor, p_head, p_tail);
	sensor = 0;
	V(&mutexp_arr[p_head]);
    }
    else{
	V(&mutexv_arr[v_head]);
//	fprintf(stdout,"\n\n allow vehicle nr: %d to cross, sensor says %d\n\n", v_head, sensor);
    }
    
    
    
    
    return NULL;
}

void *pedestrians(void *arg)
{

    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = pedestrian_arrive(info);
    P(&light_mutex);
    if(!basecondition){
	V(&mutexp_arr[p_head]);
	basecondition++;
//	fprintf(stdout, "\npedestrian base condition\n");
    }
    V(&light_mutex);
    
    P(&mutexp_arr[place-1]);
    
    pedestrian_walk(info);
    pedestrian_leave(info);
    p_head++;
    sensor++;
    if(sensor ==  K || p_head == p_tail)
    {
//	fprintf(stdout,"\n\nentered pedestrian! sensor = %d, p_head = %d, p_tail = %d \n\n",sensor, p_head, p_tail);
	sensor = 0;
	V(&mutexv_arr[v_head]);
    }
    else{
	//	fprintf(stdout,"\n\n allow pedestrian nr: %d to cross,tail says %d sensor says %d\n\n", p_head, p_tail, sensor);
	V(&mutexp_arr[p_head]);
    }    

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


