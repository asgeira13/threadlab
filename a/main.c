#include "traffic.c"

/* The main function takes in the arguments and runs the process based on their values.  If there are no arguements
 * default values are used */
int main(int argc, char **argv)
{
    /* parse arguments / seed random */
    srand(time(NULL));
    int max_time = 7;
    num_pedestrians = 20;
    num_vehicles = 20;
    K = 5;
    switch (argc) {
        default:
        case 5:
            max_time = atoi(argv[4]);
        case 4:
            num_pedestrians = atoi(argv[3]);
        case 3:
            num_vehicles = atoi(argv[2]);
        case 2:
            K = atoi(argv[1]);
        case 1:
            break;
    }
    fprintf(stdout, "%d %d %d\n", K, num_vehicles, num_pedestrians);

    /* initialise variables */
    cross_init(num_vehicles, num_pedestrians);
    init();

    /* spawn the threads */
    int i = num_vehicles;
    int j = num_pedestrians;
    for (;;) {
        if (j > 0) {
            int pedestrian_type = 0;
            thread_info *thread_data = malloc(sizeof(thread_info));
            thread_data->crossing = pedestrian_type;
            thread_data->thread_nr = num_pedestrians - j;
            thread_data->type = PEDESTRIAN;
            spawn_pedestrian(thread_data);
            j--;
        }
        if (i > 0) {
            int vehicle_type = 3;
            thread_info *thread_data = malloc(sizeof(thread_info));
            thread_data->crossing = vehicle_type;
            thread_data->type = VEHICLE;
            thread_data->thread_nr = num_vehicles - i;
            spawn_vehicle(thread_data);
            i--;
        }
        if (i <= 0 && j <= 0) {
            break;
        }
        rand_sleep(max_time);
    }

    clean();
    return 0;
}
