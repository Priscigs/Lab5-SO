#include <sys/time.h>
#include <stdlib.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define LOOP_ITERATIONS_PER_MILLISEC 178250

#define MILLISEC    1000
#define MICROSEC    1000000
#define NANOSEC     1000000000

double min_offset, max_offset; //seconds
double min_exec_time, max_exec_time; //seconds
double min_inter_arrival_time, max_inter_arrival_time; //seconds
unsigned int task_id, job_id = 1; //identificadores de tarea y trabajo
struct itimerval inter_arrival_time; //tiempo de llegada de la tarea

void burn_1millisecs() { //función para utilizar la CPU durante 1 ms
    unsigned long long i;
    for(i=0; i<LOOP_ITERATIONS_PER_MILLISEC; i++);
}

void burn_cpu(long milliseconds){ //función para utilizar la CPU durante un tiempo específico
    long i;
    for(i=0; i<milliseconds; i++)
        burn_1millisecs();
}

void clear_task_param(struct sched_param *param) //función para borrar los parámetros de la tarea
{
    param->task_id=-1;
    param->deadline=0;
}

void print_task_param(struct sched_param *param) //función para imprimir los parámetros de la tarea
{
    printf("\nPid[%d]\n",param->task_id);
    printf("Deadline[%llu]\n",param->deadline);
}

void clear_signal_timer(struct itimerval *t) //función para borrar el temporizador de la señal
{
    t->it_interval.tv_sec = 0;
    t->it_interval.tv_usec = 0;
    t->it_value.tv_sec = 0;
    t->it_value.tv_usec = 0;
}

void set_signal_timer(struct itimerval *t,double secs) //función para establecer el temporizador de la señal
{
    t->it_interval.tv_sec = 0;
    t->it_interval.tv_usec =0 ;
    t->it_value.tv_sec = (int)secs;
    t->it_value.tv_usec = (secs-t->it_value.tv_sec)*MICROSEC;
}

void print_signal_timer(struct itimerval *t) //función para imprimir el temporizador de la señal
{
    printf("Intervalo: secs [%ld] usecs [%ld] VALUE: segunods [%ld] usecs [%ld]\n",
        t->it_interval.tv_sec,
        t->it_interval.tv_usec,
        t->it_value.tv_sec,
        t->it_value.tv_usec);
}

double get_time_value(double min, double max) //función para obtener un valor aleatorio en el rango especificado
{
    if(min==max)
        return min;
    return (min + (((double)rand()/RAND_MAX)*(max-min)));
}

void start_task(int s) //función para iniciar la tarea
{
    printf("\nTask(%d) ahora se inicializó\n",task_id);
    set_signal_timer(&inter_arrival_time,get_time_value(min_offset,max_offset)); //establece el tiempo de llegada de la tarea
    setitimer(ITIMER_REAL, &inter_arrival_time, NULL); //inicia el temporizador de la señal
}

void do_work(int s) //función para realizar el trabajo de la tarea
{
    signal(SIGALRM, do_work);
    set_signal_timer(&

    set_signal_timer(&inter_arrival_time,get_time_value(min_inter_arrival_time,max_inter_arrival_time));

    setitimer(ITIMER_REAL, &inter_arrival_time, NULL);

    clock_t start, end;
    double elapsed=0;
    start = clock();
    printf("Job(%d,%d) empieza en\n",task_id,job_id);
    burn_cpu(get_time_value(min_exec_time,max_exec_time)*MILLISEC);
    end = clock();
    elapsed = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Job(%d,%d) termina en (%f)\n",task_id,job_id,elapsed);
    job_id++;

}

 void final_task(int s)
 {
     printf("\nTask(%d) ha terminado por completo\n", task_id);
     exit(0);
 }

 int main(int argc, char** argv) {
     
     struct sched_param task_param; // Estructura de parámetros de planificación de tareas
     
     unsigned long long seed; // Semilla para la función rand()
     int i;
     
     clear_signal_timer(&inter_arrival_time); // Limpia y configura el temporizador de señales
     
     clear_sched_param(&task_param); // Limpia y configura los parámetros de planificación
     
     /* Asignación de valores de parámetros */
     
     task_param.casio_id = atoi(argv[1]); // Identificador de la tarea
     min_exec_time = atof(argv[2]); // Tiempo mínimo de ejecución de un trabajo
     max_exec_time = atof(argv[3]); // Tiempo máximo de ejecución de un trabajo
     min_inter_arrival_time = atof(argv[4]); // Tiempo mínimo entre llegadas de trabajos
     max_inter_arrival_time = atof(argv[5]); // Tiempo máximo entre llegadas de trabajos
     task_param.deadline = atof(argv[6]) * NANOSEC; // Plazo para completar un trabajo en nanosegundos
     min_offset = atof(argv[7]); // Desplazamiento mínimo en el tiempo de inicio del trabajo
     max_offset = atof(argv[8]); // Desplazamiento máximo en el tiempo de inicio del trabajo
     seed = atol(argv[9]); // Semilla para la función rand()
     srand(seed); // Inicializa la función rand() con la semilla dada
     signal(SIGUSR1, start_task); // Asigna la señal SIGUSR1 a la función start_task()
     signal(SIGALRM, do_work); // Asigna la señal SIGALRM a la función do_work()
     signal(SIGUSR2, end_task); // Asigna la señal SIGUSR2 a la función end_task()
     
     print_task_param(&task_param); // Imprime en pantalla los parámetros de planificación de la tarea
     printf("Antes sched_setscheduler: priorizar %d\n",sched_getscheduler(0)); // Imprime en pantalla la prioridad actual del proceso
     
     if (sched_setscheduler(0, SCHED_CASIO, &task_param) == -1) { // Establece la política de planificación SCHED_CASIO y sus parámetros para el proceso actual
         perror("ERROR"); // Imprime un mensaje de error si la operación falla
     }
     
     printf("Después sched_setscheduler: priorizar %d\n",sched_getscheduler(0)); // Imprime en pantalla la nueva prioridad del proceso
     
     while (1) {
         pause(); // Espera a que llegue una señal
     }
     
     return 0;
 }
