#include <stdbool.h>
#include <assert.h>
#include "crossing.h"

int K;
int num_vehicles;
int num_pedestrians;

int vh_head = 0;
int vh_tail = -1;
int vv_head = 0;
int vv_tail = -1;

int ph_head = 0;
int ph_tail = -1;
int pv_head = 0;
int pv_tail = -1;

int basecondition = 0; //helps with the first car or person that wants to pass

int sensorvv = 0; //counts number of people or cars that have passed during light
int sensorvh = 0;
int sensorpv = 0;
int sensorph = 0;

int stopped = 0; //helps with making sure one of the crossing has stopped

void *vehicles(void *arg);
void *pedestrians(void *arg);

sem_t * mutexvh_arr; // arrays that store semophores for individual vehicles and pedestrians
sem_t * mutexph_arr; // these help with deciding when to let them through
sem_t * mutexvv_arr;
sem_t * mutexpv_arr;


sem_t light_mutex;
sem_t thread_mutex;// binary sempaphore for locking


pthread_t *pedestrian_thread;
pthread_t *vehicle_thread;



void init()
{
    sem_init(&light_mutex, 0, 1);
    sem_init(&thread_mutex, 0, 1);

    pedestrian_thread = malloc(sizeof(pthread_t) * num_pedestrians);
    vehicle_thread = malloc(sizeof(pthread_t) * num_vehicles);
    mutexvh_arr = malloc(sizeof(sem_t) * num_vehicles);
    mutexph_arr = malloc(sizeof(sem_t) * num_pedestrians);
    mutexvv_arr = malloc(sizeof(sem_t) * num_vehicles);
    mutexpv_arr = malloc(sizeof(sem_t) * num_pedestrians);

    for(int v = 0; v < num_vehicles; ++v)
    {
	sem_init(&mutexvh_arr[v], 0, 0);
	sem_init(&mutexvv_arr[v], 0, 0);
    }
    for(int p = 0; p < num_pedestrians; ++p)
    {
	sem_init(&mutexph_arr[p], 0, 0);
	sem_init(&mutexpv_arr[p], 0, 0);
    }
}

void spawn_pedestrian(thread_info *arg)
{
    Pthread_create(&pedestrian_thread[arg->thread_nr], 0, pedestrians, arg);
    //fprintf(stdout,"\n p crossing: %d ", arg->crossing);
    if(arg->crossing == 0)
	pv_tail++;
//    fprintf(stdout, "\n pv_tail %d \n", pv_tail);
    if(arg->crossing == 2)
	ph_tail++;
//    fprintf(stdout, "\n ph_tail %d \n", ph_tail);
}

void spawn_vehicle(thread_info *arg)
{
    Pthread_create(&vehicle_thread[arg->thread_nr], 0, vehicles, arg);
    if(arg->crossing == 3)
	vh_tail++;
//    fprintf(stdout, "\n vh_tail %d \n", vh_tail);
    if(arg->crossing == 1)
	vv_tail++;
//    fprintf(stdout, "\n vv_tail %d \n", vv_tail);
}

void *vehicles(void *arg)
{
    thread_info *info = arg;
    int cross = info->crossing;
    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = vehicle_arrive(info);
    
    P(&light_mutex);
    if(!(basecondition) ){ // base condition to initiate the first crossing
	if(cross == 1){
	    V(&mutexvv_arr[0]);
	    //V(&mutexpv_arr[0]);
	}
	if(cross == 3){
	    V(&mutexvh_arr[0]);
//	    V(&mutexph_arr[0]);
	}
	stopped = 1;

//	fprintf(stdout, "\nvehicle base condition\n");
	basecondition++;
    }
   
    V(&light_mutex);
    if(cross == 1){
	P(&mutexvv_arr[vv_tail]);
	P(&thread_mutex);
	vehicle_drive(info);
	vehicle_leave(info);
	vv_head++;
	sensorvv++;
//	fprintf(stdout,"entered vertical vehicle\n");
	if(sensorvv >= K || vv_head > vv_tail)
	{
//	    fprintf(stdout,"entered vehicle vertical! sensor = %d, v_head = %d, v_tail = %d \n",sensorvv, vv_head, vv_tail);
	    sensorvv = 0;
	    //might need lock
	    if(stopped == 1){
		stopped = 0;
		if(ph_head <= ph_tail)
		    V(&mutexph_arr[ph_head]);
		else 
		    stopped ++;
		if(vh_head <= vh_tail)
		    V(&mutexvh_arr[vh_head]);
		else
		    stopped ++;
		if(stopped > 1){
		    if(pv_head <= pv_tail)
			V(&mutexpv_arr[pv_head]);
		    if(vv_head <= vv_tail)
			V(&mutexvv_arr[vv_head]);
		}
	    }
	    else
		stopped = 1;
	}
	else if(vv_head <= vv_tail){
	    V(&mutexvv_arr[vv_head]);
//  	    fprintf(stdout,"allow vertical vehicle nr: %d to cross, tail says %d,  sensor says %d\n", vv_head, vv_tail, sensorvv);
	}
    }
    if(cross == 3){
	P(&mutexvh_arr[vh_tail]);
	P(&thread_mutex);
	vehicle_drive(info);
	vehicle_leave(info);
	vh_head++;
	sensorvh++;

	if(sensorvh >= K || vh_head > vh_tail)
	{
//  	    fprintf(stdout,"entered vehicle horizontal! sensor = %d, head = %d, tail = %d  \n",sensorvh, vh_head, vh_tail);
	    sensorvh = 0;
	    
	    if(stopped == 1){
		stopped = 0;
		if(pv_head <= pv_tail)
		    V(&mutexpv_arr[pv_head]);
		else
		    stopped++;
		if(vv_head <= vv_tail)
		    V(&mutexvv_arr[vv_head]);
		else 
		    stopped++;

		if(stopped > 1){
		    if(ph_head <= ph_tail)
			V(&mutexph_arr[ph_head]);
		    if(vh_head <= vh_tail)
			V(&mutexvh_arr[vh_head]);
		}

	    }
	    else
		stopped = 1;
	}
	else if(vh_head <= vh_tail){
	    V(&mutexvh_arr[vh_head]);
//  	    fprintf(stdout,"allow horizontal vehicle nr: %d to cross,tail says %d,  sensor says %d\n", vh_head,vh_tail, sensorvh);
	}
    }    
      V(&thread_mutex);    
    
//    fprintf(stdout, "vehicle exited\n");
    return NULL;
}

void *pedestrians(void *arg)
{

    thread_info *info = arg;

    // Each thread needs to call these functions in this order
    // Note that the calls can also be made in helper functions
    int place = pedestrian_arrive(info);
    int cross = info->crossing;
    //fprintf(stdout,"\nthis pedestrian is at place %d \n", place);
    P(&light_mutex);
    if(!(basecondition)){ // base condition to initiate the first crossing
	if(cross == 0){
	    V(&mutexvv_arr[0]);
	    //V(&mutexpv_arr[0]);
	}
	if(cross == 2){
	    V(&mutexph_arr[0]);
	    //   V(&mutexvh_arr[0]);
	}
	stopped = 1;
	basecondition++;
    }

    V(&light_mutex);

    if(cross == 0){
	P(&mutexpv_arr[pv_tail]);
//	fprintf(stdout,"entered vertical pedestrian\n");
	P(&thread_mutex);
	pedestrian_walk(info);
	pedestrian_leave(info);
	pv_head++;
	sensorpv++;
	
	if(sensorpv >=  K || pv_head > pv_tail)
	{
//  	    fprintf(stdout,"entered pedestrian vertical! sensor = %d, p_head = %d, p_tail = %d \n",sensorpv, pv_head, pv_tail);
	    sensorpv = 0;
	    if(stopped == 1){
		stopped = 0;
		if(vh_head <= vh_tail)
		    V(&mutexvh_arr[vh_head]);
		else
		    stopped ++;
		if(ph_head <= ph_tail)
		    V(&mutexph_arr[ph_head]);
		else
		    stopped ++;
		if(stopped > 1){
		    if(pv_head <= pv_tail)
			V(&mutexpv_arr[pv_head]);
		    if(vv_head <= vv_tail)
			V(&mutexvv_arr[vv_head]);
		}
		
	    }
	    else
		stopped = 1;
	}
	else if(pv_head <= pv_tail){
//  	    fprintf(stdout,"allow vertical  pedestrian nr: %d to cross,tail says %d sensor says %d\n", pv_head, pv_tail, sensorpv);
	    V(&mutexpv_arr[pv_head]);
	}    
    }

    if(cross == 2){
	P(&mutexph_arr[ph_tail]);
	P(&thread_mutex);
	pedestrian_walk(info);
	pedestrian_leave(info);
	ph_head++;
	sensorph++;
	
	if(sensorph >=  K || ph_head > ph_tail)
	{
//	    fprintf(stdout,"entered pedestrian horizontal! sensor = %d, p_head = %d, p_tail = %d \n",sensorph, ph_head, ph_tail);
	    sensorph = 0;
	    if(stopped == 1){
		stopped = 0;
		if(vv_head <= vv_tail)
		    V(&mutexvv_arr[vv_head]);
		else
		    stopped ++;
		if(pv_head <= pv_tail)
		    V(&mutexpv_arr[pv_head]);
		else 
		    stopped ++;
		if(stopped > 1){
		    if(ph_head <= ph_tail)
			V(&mutexph_arr[ph_head]);
		    if(vh_head <= vh_tail)
			V(&mutexvh_arr[vh_head]);
		}
	    }
	    else 
		stopped = 1;
	}
	else if(ph_head <= ph_tail){
//  	    fprintf(stdout,"allow horizontal pedestrian nr: %d to cross,tail says %d sensor says %d\n", ph_head, ph_tail, sensorph);
	    V(&mutexph_arr[ph_head]);
	}    
    }
//    fprintf(stdout, "pedestrian exited\n");
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
    free(mutexph_arr);
    free(mutexvh_arr);
    free(mutexpv_arr);
    free(mutexvv_arr);
    free(pedestrian_thread);
    free(vehicle_thread);
}


