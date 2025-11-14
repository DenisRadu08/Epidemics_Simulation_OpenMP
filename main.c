#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <string.h>

#define INFECTED_DURATION 10
#define IMMUNE_DURATION 2
#define STATUS_INFECTED 0
#define STATUS_SUSCEPTIBLE 1
#define STATUS_IMMUNE 2
//#define DEBUG

typedef struct{

    int PersonID;
    int x,y;
    int initial_status;
    int moving_direction;
    int moving_amplitude;
    int current_status;
    int future_status;
    int still_infected_timer;
    int still_immune_timer;
    int infected_counter;
}Person;

void read_arguments(int argc,char **argv,int *TOTAL_SIMULATION_TIME, FILE **InputFileName,int *ThreadNumber) {

    if (argc!=4) {
        printf("Not enough arguments\n");
        exit(-1);
    }

    *TOTAL_SIMULATION_TIME = atoi(argv[1]);
    *InputFileName=fopen(argv[2],"r");
    if (*InputFileName==NULL) {
        printf("Error: Input file not found\n");
        exit(-1);
    }
    *ThreadNumber = atoi(argv[3]);
}

void read_from_file(FILE **InputFileName, int *MAX_X_COORD, int *MAX_Y_COORD, int *N,Person **person) {

    if (fscanf(*InputFileName,"%d %d %d",MAX_X_COORD, MAX_Y_COORD,N)!=3) {
        printf("Error: Could not read from fileguramica\n");
        return;
    }

    *person=(Person *)malloc((*N)*sizeof(Person));
    if (*person==NULL) {
        printf("Error: Could not allocate memory for the population\n");
        return;
    }
    for (int i=0;i<*N;i++) {
        if (fscanf(*InputFileName,"%d %d %d %d %d %d",&((*person)[i].PersonID),&((*person)[i].x),&((*person)[i].y),&((*person)[i].initial_status),&((*person)[i].moving_direction),&((*person)[i].moving_amplitude))!=6) {
            printf("Error: Could not read from filelalala\n");
            return;
        }
        (*person)[i].current_status=(*person)[i].initial_status;
        (*person)[i].still_immune_timer=0;
        if ((*person)[i].current_status==0) {
            (*person)[i].infected_counter=1;
            (*person)[i].still_infected_timer=INFECTED_DURATION;
        }
        else {
            (*person)[i].infected_counter=0;
            (*person)[i].still_infected_timer=0;
        }
    }

}

void compute_future_status(Person *person, int N, int MAX_X_COORD, int MAX_Y_COORD) {
    for (int i=0;i<N;i++) {
        Person *p=&person[i];
        p->future_status=p->current_status;

        if (p->current_status==STATUS_INFECTED) {

            p->still_infected_timer--;
            if (p->still_infected_timer==0) {
                p->future_status=STATUS_IMMUNE;
                p->still_immune_timer=IMMUNE_DURATION;
            }
        }

        else if (p->current_status==STATUS_IMMUNE) {
            p->still_immune_timer--;
            if (p->still_immune_timer==0) {
                p->future_status=STATUS_SUSCEPTIBLE;
            }
        }
    }

    int **grid=(int **)malloc((MAX_X_COORD+1)*sizeof(int *));
    for (int i=0;i<MAX_X_COORD+1;i++) {
        grid[i]=(int *)calloc((MAX_Y_COORD+1),sizeof(int));
    }

    for (int i=0;i<N;i++) {
        Person *p=&person[i];
        if (p->current_status==STATUS_INFECTED) {
            grid[p->x][p->y]=1;
        }
    }

    for (int i=0;i<N;i++) {
        Person *p=&person[i];
        if (p->future_status==STATUS_SUSCEPTIBLE && grid[p->x][p->y]==1) {
            p->future_status=STATUS_INFECTED;
            p->still_infected_timer=INFECTED_DURATION;
            p->infected_counter++;
        }
    }

    for (int i=0;i<=MAX_X_COORD;i++) {
        free(grid[i]);
    }
    free(grid);
}

void move_one_person(Person *person, int MAX_X_COORD, int MAX_Y_COORD) {

    if ((person->moving_direction == 0 && person->y == MAX_Y_COORD)) {
        person->moving_direction = 1;
    }
    if (person->moving_direction == 1 && person->y == 0) {
        person->moving_direction = 0;
    }
    if (person->moving_direction == 2 && person->x == MAX_X_COORD) {
        person->moving_direction = 3;
    }
    if (person->moving_direction == 3 && person->x == 0) {
        person->moving_direction = 2;
    }

    int curr_amplitude = person->moving_amplitude;
    while (curr_amplitude > 0) {
        if (person->moving_direction == 0) {
            person->y++;
            curr_amplitude--;
            if (person->y == MAX_Y_COORD) {
                person->moving_direction = 1;
            }
        } else if (person->moving_direction == 1) {
            person->y--;
            curr_amplitude--;
            if (person->y == 0) {
                person->moving_direction = 0;
            }
        } else if (person->moving_direction == 2) {
            person->x++;
            curr_amplitude--;
            if (person->x == MAX_X_COORD) {
                person->moving_direction = 3;
            }
        } else if (person->moving_direction == 3) {
            person->x--;
            curr_amplitude--;
            if (person->x == 0) {
                person->moving_direction = 2;
            }
        }
    }
}

void serial_simulation(int TOTAL_SIMULATION_TIME,Person *person,int N,int MAX_X_COORD, int MAX_Y_COORD) {

    int t=0;
    while (TOTAL_SIMULATION_TIME!=0) {

        for (int i=0;i<N;i++) {
            move_one_person(&person[i],MAX_X_COORD,MAX_Y_COORD);
        }
        compute_future_status(person,N,MAX_X_COORD,MAX_Y_COORD);
        for (int i=0;i<N;i++) {
            Person *p=&person[i];
            p->current_status=p->future_status;
        }

#ifdef DEBUG
        {
            int infected=0,immune=0,susceptible=0;
            for (int i=0;i<N;i++) {
                if (person[i].current_status==STATUS_INFECTED) {
                    infected++;
                }
                else if (person[i].current_status==STATUS_IMMUNE) {
                    immune++;
                }
                else susceptible++;
            }

            printf("Step %d: Infected=%d, Immune=%d, Susceptible=%d\n",t++,infected,immune,susceptible);
        }
#endif

        TOTAL_SIMULATION_TIME--;
    }



}

void parallel_simulationV1(int TOTAL_SIMULATION_TIME,Person *person,int N,int MAX_X_COORD,int MAX_Y_COORD,int ThreadNumber) {
    int **grid=(int **)malloc((MAX_X_COORD+1)*sizeof(int *));
    if (grid==NULL) {
        printf("Error when allocating memory\n");
    }
    for (int i=0;i<MAX_X_COORD+1;i++) {
        grid[i]=(int *)calloc((MAX_Y_COORD+1),sizeof(int));
    }

    for (int t=0;t<TOTAL_SIMULATION_TIME;t++) {
#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<N;i++) {
            move_one_person(&person[i],MAX_X_COORD,MAX_Y_COORD);
        }

#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<N;i++) {
            Person *p=&person[i];
            p->future_status=p->current_status;
            if (p->current_status==STATUS_INFECTED) {

                p->still_infected_timer--;
                if (p->still_infected_timer==0) {
                    p->future_status=STATUS_IMMUNE;
                    p->still_immune_timer=IMMUNE_DURATION;
                }
            }

            else if (p->current_status==STATUS_IMMUNE) {
                p->still_immune_timer--;
                if (p->still_immune_timer==0) {
                    p->future_status=STATUS_SUSCEPTIBLE;
                }
            }
        }

#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<=MAX_X_COORD;i++) {
            memset(grid[i],0,(MAX_Y_COORD+1) * sizeof(int));
        }

#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<N;i++) {
            Person *p=&person[i];
            if (p->current_status==STATUS_INFECTED) {
                grid[p->x][p->y]=1;
            }
        }

#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<N;i++) {
            Person *p=&person[i];
            if (p->future_status==STATUS_SUSCEPTIBLE && grid[p->x][p->y]==1) {
                p->future_status=STATUS_INFECTED;
                p->still_infected_timer=INFECTED_DURATION;
                p->infected_counter++;
            }
        }

#pragma omp parallel for num_threads(ThreadNumber)
        for (int i=0;i<N;i++) {
            person[i].current_status=person[i].future_status;
        }
    }

    for (int i=0;i<=MAX_X_COORD;i++)
        free(grid[i]);
    free(grid);
}



void parallel_simulationV2(int TOTAL_SIMULATION_TIME,Person *person,int N,int MAX_X_COORD,int MAX_Y_COORD,int ThreadNumber) {

    int **grid=(int **)malloc((MAX_X_COORD+1)*sizeof(int *));
    for (int i=0;i<MAX_X_COORD+1;i++) {
        grid[i]=(int *)calloc((MAX_Y_COORD+1),sizeof(int));
    }
#pragma omp parallel num_threads(ThreadNumber)
    {
        #pragma omp single
            {
                for (int t=0;t<TOTAL_SIMULATION_TIME;t++) {

                    for (int th=0;th<ThreadNumber;th++)
                    {
                        #pragma omp task
                        {
                            int start=(th*N)/ThreadNumber;
                            int end=((th+1)*N)/ThreadNumber;
                            for (int i=start;i<end;i++) {
                                move_one_person(&person[i],MAX_X_COORD,MAX_Y_COORD);
                            }
                        }
                    }
#pragma omp taskwait

                    for (int th=0;th<ThreadNumber;th++) {
#pragma omp task
                        {
                            int start = (th * N) / ThreadNumber;
                            int end = ((th + 1) * N) / ThreadNumber;
                            for (int i = start; i < end; i++)
                            {
                                Person *p=&person[i];
                                p->future_status=p->current_status;
                                if (p->current_status==STATUS_INFECTED) {

                                    p->still_infected_timer--;
                                    if (p->still_infected_timer==0) {
                                        p->future_status=STATUS_IMMUNE;
                                        p->still_immune_timer=IMMUNE_DURATION;
                                    }
                                }

                                else if (p->current_status==STATUS_IMMUNE) {
                                    p->still_immune_timer--;
                                    if (p->still_immune_timer==0) {
                                        p->future_status=STATUS_SUSCEPTIBLE;
                                    }
                                }
                            }
                        }
                    }
#pragma omp taskwait

                    for (int i=0;i<=MAX_X_COORD;i++)
                        memset(grid[i],0,(MAX_Y_COORD+1) * sizeof(int));

                    for (int th=0;th<ThreadNumber;th++) {
#pragma omp task
                        {
                            int start = (th * N) / ThreadNumber;
                            int end = ((th + 1) * N) / ThreadNumber;
                            for (int i = start; i < end; i++)
                            {
                                Person *p=&person[i];
                                if (p->current_status==STATUS_INFECTED) {
                                    grid[p->x][p->y]=1;
                                }
                            }
                        }
                    }
                    #pragma omp taskwait

                    for (int th=0;th<ThreadNumber;th++) {
#pragma omp task
                        {
                            int start = (th * N) / ThreadNumber;
                            int end = ((th + 1) * N) / ThreadNumber;
                            for (int i = start; i < end; i++)
                            {
                                Person *p=&person[i];
                                if (p->future_status==STATUS_SUSCEPTIBLE && grid[p->x][p->y]==1) {
                                    p->future_status=STATUS_INFECTED;
                                    p->still_infected_timer=INFECTED_DURATION;
                                    p->infected_counter++;
                                }
                            }
                        }
                    }
                    #pragma omp taskwait

                    for (int th=0;th<ThreadNumber;th++) {
                        #pragma omp task
                        {
                            int start = (th * N) / ThreadNumber;
                            int end = ((th + 1) * N) / ThreadNumber;
                            for (int i = start; i < end; i++)
                                person[i].current_status=person[i].future_status;
                        }
                    }
#pragma omp taskwait
                }
            }
    }

    for (int i=0;i<=MAX_X_COORD;i++)
        free(grid[i]);
    free(grid);
}


void output(char *input_filename,char *suffix,Person *person, int N) {

    char base_file[256];
    strcpy(base_file,input_filename);
    char *dot=strrchr(base_file, '.');
    if (dot!=NULL) {
        *dot='\0';
    }
    char output_filename[256];
    sprintf(output_filename,"%s_%s.txt",base_file,suffix);

    FILE *OutputFile=fopen(output_filename,"w");
    if (OutputFile==NULL) {
        printf("Can't open output file!\n");
        return;
    }

    for (int i=0;i<N;i++) {
        char *status;
        if (person[i].current_status==STATUS_INFECTED) {
            status="INFECTED";
        }
        else if (person[i].current_status==STATUS_SUSCEPTIBLE) {
            status="SUSCEPTIBLE";
        }
        else status="IMMUNE";
        fprintf(OutputFile,"Final coordinates: %d, %d\nFinal status: %s\nInfection counter: %d\n",person[i].x,person[i].y,status,person[i].infected_counter);
    }

    if (fclose(OutputFile)!=0) {
        printf("Can't close output file!\n");
    }
}

int compare_outputs(char *file1,char *file2) {

    FILE *fp1,*fp2;
    fp1=fopen(file1,"r");
    if (fp1==NULL) {
        printf("Can't open output file!\n");
        return -1;
    }
    fp2=fopen(file2,"r");
    if (fp2==NULL) {
        printf("Can't open output file!\n");
        return -1;
    }

    char line1[512],line2[512];
    int line_num=0;

    while (fgets(line1,sizeof(line1),fp1) != NULL && fgets(line2, sizeof(line2), fp2) != NULL) {
        line_num++;

        if (strcmp(line1,line2)!=0) {
            printf("THE RESULTS ARE NOT THE SAME\n");
            printf("Differences appear at line: %d\n",line_num);
            printf("Serial: %s\n",line1);
            printf("Parallel: %s\n",line2);
            if (fclose(fp1)!=0) {
                printf("Can't close output file!\n");
            }
            if (fclose(fp2)!=0) {
                printf("Can't close output file!\n");
            }
            return 1;
        }
    }

    fseek(fp1,0,SEEK_END);
    fseek(fp2,0,SEEK_END);

    if (ftell(fp1) != ftell(fp2)) {
        printf("THE RESULTS ARE NOT THE SAME\nThe files have different lengths\n");
        if (fclose(fp1)!=0) {
            printf("Can't close output file!\n");
        }
        if (fclose(fp2)!=0) {
            printf("Can't close output file!\n");
        }
        return 1;
    }

    if (fclose(fp1)!=0) {
        printf("Can't close output file!\n");
    }
    if (fclose(fp2)!=0) {
        printf("Can't close output file!\n");
    }

    printf("THE RESULTS ARE THE SAME!\n");
    return 0;
}

int main(int argc, char**argv) {
    int TOTAL_SIMULATION_TIME,ThreadNumber;
    FILE *InputFileName;
    int MAX_X_COORD,MAX_Y_COORD,N;
    double start,run_time;
    Person *person_serial;

    read_arguments(argc,argv,&TOTAL_SIMULATION_TIME, &InputFileName, &ThreadNumber);
    read_from_file(&InputFileName,&MAX_X_COORD,&MAX_Y_COORD,&N,&person_serial);

    Person *person_parallelV1=(Person *)malloc(N*sizeof(Person));
    if (person_parallelV1==NULL) {
        printf("Error when allocating memory\n");
        exit(-1);
    }
    Person *person_parallelV2=(Person *)malloc(N*sizeof(Person));
    if (person_parallelV2==NULL) {
        printf("Error when allocating memory\n");
        exit(-1);
    }

    memcpy(person_parallelV1,person_serial,N*sizeof(Person));
    memcpy(person_parallelV2,person_serial,N*sizeof(Person));

    printf("Serial simulation...\n");
    start=omp_get_wtime();
    serial_simulation(TOTAL_SIMULATION_TIME,person_serial,N,MAX_X_COORD,MAX_Y_COORD);
    run_time=omp_get_wtime()-start;
    printf("Serial simulation time: %lf\n",run_time);
    output(argv[2],"serial_out",person_serial,N);

    printf("\nParallelV1 simulation...\n");
    start=omp_get_wtime();
    parallel_simulationV1(TOTAL_SIMULATION_TIME,person_parallelV1,N,MAX_X_COORD,MAX_Y_COORD,ThreadNumber);
    run_time=omp_get_wtime()-start;
    printf("ParallelV1 simulation time: %lf\n",run_time);
    output(argv[2],"parallelV1_out",person_parallelV1,N);

    printf("\nParallelV2 simulation...\n");
    start=omp_get_wtime();
    parallel_simulationV2(TOTAL_SIMULATION_TIME,person_parallelV2,N,MAX_X_COORD,MAX_Y_COORD,ThreadNumber);
    run_time=omp_get_wtime()-start;
    printf("ParallelV2 simulation time: %lf\n",run_time);
    output(argv[2],"parallelV2_out",person_parallelV2,N);

    char base_file[256];
    strcpy(base_file,argv[2]);
    char *dot=strrchr(base_file,'.');
    if (dot!=NULL)
        *dot='\0';

    char serial_file[256];
    char parallelV1_file[256];
    char parallelV2_file[256];

    sprintf(serial_file,"%s_serial_out.txt",base_file);
    sprintf(parallelV1_file,"%s_parallelV1_out.txt",base_file);
    sprintf(parallelV2_file,"%s_parallelV2_out.txt",base_file);

    printf("\nComparing Serial vs ParallelV1...\n");
    compare_outputs(serial_file,parallelV1_file);

    printf("\nComparing Serial vs ParallelV2...\n");
    compare_outputs(serial_file,parallelV2_file);

    printf("\nComparing ParallelV1 vs ParallelV2...\n");
    compare_outputs(parallelV1_file,parallelV2_file);


    if (fclose(InputFileName)!=0) {
        printf("Error: Could not close file\n");
    }
    free(person_serial);
    free(person_parallelV1);
    free(person_parallelV2);
    return 0;
}