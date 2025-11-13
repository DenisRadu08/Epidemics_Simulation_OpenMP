#include <stdio.h>
#include <omp.h>
#include <stdlib.h>

#define INFECTED_DURATION 5
#define IMMUNE_DURATION 5
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

    /*for (int i=0;i<*N;i++) {
        printf("%d %d %d %d %d %d",(*person)[i].PersonID,(*person)[i].x,(*person)[i].y,(*person)[i].initial_status,(*person)[i].moving_direction,(*person)[i].moving_amplitude);
        printf("\n");
    }*/

}

void move_persons(Person *person,int N, int MAX_X_COORD, int MAX_Y_COORD) {

    for (int i=0;i<N;i++) {
        if ((person[i].moving_direction==0 && person[i].y==MAX_Y_COORD)) {
            person[i].moving_direction=1;
        }

        if (person[i].moving_direction==1 && person[i].y==0) {
            person[i].moving_direction=0;
        }

        if (person[i].moving_direction==2 && person[i].x==MAX_X_COORD) {
            person[i].moving_direction=3;
        }

        if (person[i].moving_direction==3 && person[i].x==0) {
            person[i].moving_direction=2;
        }

        int curr_amplitude=person[i].moving_amplitude;
        while (curr_amplitude>0) {
            if (person[i].moving_direction==0) {
                    person[i].y++;
                    curr_amplitude--;
                    if (person[i].y==MAX_Y_COORD) {
                        person[i].moving_direction=1;
                    }
            }
            else if (person[i].moving_direction==1) {
                person[i].y--;
                curr_amplitude--;
                if (person[i].y==0) {
                    person[i].moving_direction=0;
                }
            }
            else if (person[i].moving_direction==2) {
                person[i].x++;
                curr_amplitude--;
                if (person[i].x==MAX_X_COORD) {
                    person[i].moving_direction=3;
                }
            }
            else if (person[i].moving_direction==3) {
                person[i].x--;
                curr_amplitude--;
                if (person[i].x==0) {
                    person[i].moving_direction=2;
                }
            }
        }
    }
}

int main(int argc, char**argv)
{
    int TOTAL_SIMULATION_TIME,ThreadNumber;
    FILE *InputFileName;
    int MAX_X_COORD,MAX_Y_COORD,N;
    Person *person;

    read_arguments(argc,argv,&TOTAL_SIMULATION_TIME, &InputFileName, &ThreadNumber);
    read_from_file(&InputFileName,&MAX_X_COORD,&MAX_Y_COORD,&N,&person);





#ifdef DEBUG
    blabla
#endif

    if (fclose(InputFileName)!=0) {
        printf("Error: Could not close file\n");
    }
    free(person);
    return 0;
}