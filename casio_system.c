#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define BUF_LEN 200
#define CASIO_TASKS_NUM 20

/*
Aquí encontramos máximos, mínimos y tiempos.
Se puede identificar cada tarea.
*/

struct casio_tasks_config {
    int pid;
    double min_exec;
    double max_exec;
    double min_inter_arrival;
    double max_inter_arrival;
    double deadline;
    double min_offset;
    double max_offset;
};

pid_t casio_tasks_pid[CASIO_TASKS_NUM];
int casio_tasks_num = 0;

int get_int_val(char* str) {
    char* s = str;
    int val;
    for (s = str; *s != '\t'; s++);
    *s = '\0';
    val = atoi(str); 
    return val;
}

//--------------------------------------------
// Imprime la configuración de las tareas CASIO
void print_casio_tasks_config(struct casio_tasks_config *tasks, int num) {
    // Cabecera
    printf("\nCASIO TASKS CONFIG\n");
    printf("pid\tmin_c\tmax_c\tmin_t\tmax_t\tdeadl\tmin_o\tmax_o\n");
    
    // Imprime la configuración de cada tarea
    for (int i = 0; i < num; i++) {
        printf("%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
            tasks[i].pid,
            tasks[i].min_exec,
            tasks[i].max_exec,
            tasks[i].min_inter_arrival,
            tasks[i].max_inter_arrival,
            tasks[i].deadline,
            tasks[i].min_offset,
            tasks[i].max_offset
        );
    }
}

// Borra la información de configuración de las tareas CASIO
void clear_casio_tasks_config_info(struct casio_tasks_config *tasks, int num) {
    // Itera por cada tarea
    for (int i = 0; i < num; i++) {
        // Reinicia todos los campos de la estructura
        tasks[i].pid = 0;
        tasks[i].min_exec = 0;
        tasks[i].max_exec = 0;
        tasks[i].min_inter_arrival = 0;
        tasks[i].max_inter_arrival = 0;
        tasks[i].deadline = 0;
        tasks[i].min_offset = 0;
        tasks[i].max_offset = 0;
    }
}
//--------------------------------------------
void get_casio_task_config_info(char *str, struct casio_tasks_config *tasks, int *n)
{
    char *tok = strtok(str, "\t");
    int i = 0;
    
    while (tok != NULL && i < 7) {
        switch (i) {
            case 0:
                tasks[*n].pid = atoi(tok);
                break;
            case 1:
                tasks[*n].min_exec = atof(tok);
                break;
            case 2:
                tasks[*n].max_exec = atof(tok);
                break;
            case 3:
                tasks[*n].min_inter_arrival = atof(tok);
                break;
            case 4:
                tasks[*n].max_inter_arrival = atof(tok);
                break;
            case 5:
                tasks[*n].deadline = atof(tok);
                break;
            case 6:
                tasks[*n].min_offset = atof(tok);
                tok = strtok(NULL, "\t");
                tasks[*n].max_offset = atof(tok);
                break;
        }
        i++;
        tok = strtok(NULL, "\t");
    }
    (*n)++;
}
//----------------------------
void get_casio_tasks_config_info(char *file, int *duration, struct casio_tasks_config *tasks, int *n) {
    char buffer[BUF_LEN];
    int count = 0;
    *n = 0;
    buffer[0] = '\0';
    FILE *fd = fopen(file, "r");
    if (fd == NULL) {
        printf("Error opening file: %s\n", file);
        exit(1);
    }
    while (fgets(buffer, BUF_LEN, fd) != NULL) {
        if (strlen(buffer) > 1) {
            char *token = strtok(buffer, "\t");
            switch (count) {
                case 0:
                    *duration = atoi(token);
                    count++;
                    break;
                default:
                    tasks[*n].pid = atoi(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].min_exec = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].max_exec = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].min_inter_arrival = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].max_inter_arrival = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].deadline = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].min_offset = atof(token);
                    token = strtok(NULL, "\t");
                    tasks[*n].max_offset = atof(token);
                    (*n)++;
                    break;
            }
        }
        buffer[0] = '\0';
    }
    fclose(fd);
}
//------------------------------------------
void start_simulation() {
    printf("Starting simulation...\n");
    for (int i = 0; i < casio_tasks_num; i++) {
        if (kill(casio_tasks_pid[i], SIGUSR1) == -1) {
            perror("Error sending signal");
        }
    }
}

void end_simulation(int signal) {
    printf("Ending simulation...\n");
    for (int i = 0; i < casio_tasks_num; i++) {
        if (kill(casio_tasks_pid[i], SIGUSR2) == -1) {
            perror("Error sending signal");
        }
    }
}

void help(char* program_name) {
    fprintf(stderr, "Usage: %s <file_name>\n", program_name);
    exit(EXIT_FAILURE);
}

//------------------------------------------
int main(int argc, char *argv[]) {
    int duration, n;
    struct casio_tasks_config casio_tasks_config[CASIO_TASKS_NUM];
    struct itimerval sim_time;
    char arg[CASIO_TASKS_NUM][BUF_LEN], *parg[CASIO_TASKS_NUM];
    srand(time(NULL));
    
    // Check for correct number of arguments
    if (argc != 2) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // Load tasks configuration from file
    clear_casio_tasks_config_info(casio_tasks_config, CASIO_TASKS_NUM);
    get_casio_tasks_config_info(argv[1], &duration, casio_tasks_config, &casio_tasks_num);
    
    // Set up timer to end simulation
    sim_time.it_interval.tv_sec = 0;
    sim_time.it_interval.tv_usec = 0;
    sim_time.it_value.tv_sec = duration;
    sim_time.it_value.tv_usec = 0;
    signal(SIGALRM, end_simulation);
    setitimer(ITIMER_REAL, &sim_time, NULL);
    
    // Start tasks
    for (int i = 0; i < casio_tasks_num; i++) {
        // Prepare command-line arguments
        sprintf(arg[0], "casio_task");
        sprintf(arg[1], "%d", casio_tasks_config[i].pid);
        sprintf(arg[2], "%f", casio_tasks_config[i].min_exec);
        sprintf(arg[3], "%f", casio_tasks_config[i].max_exec);
        sprintf(arg[4], "%f", casio_tasks_config[i].min_inter_arrival);
        sprintf(arg[5], "%f", casio_tasks_config[i].max_inter_arrival);
        sprintf(arg[6], "%f", casio_tasks_config[i].deadline);
        sprintf(arg[7], "%f", casio_tasks_config[i].min_offset);
        sprintf(arg[8], "%f", casio_tasks_config[i].max_offset);
        sprintf(arg[9], "%ld", rand());
        n = 10;
        for (int k = 0; k < n; k++) {
            parg[k] = arg[k];
        }
        parg[n] = NULL;
        
        // Start task process
        casio_tasks_pid[i] = fork();
        if (casio_tasks_pid[i] == 0) {
            execv("./casio_task", parg);
            perror("Error: execv\n");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }
    
    start_simulation(); // Time zero of the execution
    // Wait for the end of the simulation
    pause();
    for (int i = 0; i < casio_tasks_num; i++) {
        wait(NULL);
    }
    printf("All tasks have finished properly!!!\n");
    return 0;
}
