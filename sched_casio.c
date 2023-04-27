// Estructura para el registro de eventos de Casio
struct casio_event_log casio_event_log;

// Devuelve un puntero a la estructura de registro de eventos de Casio
struct casio_event_log* get_casio_event_log() {
return &casio_event_log;
}

// Registra un evento en la estructura de registro de eventos de Casio
void register_casio_event(unsigned long long t, char* m, int a) {
if (casio_event_log.lines >= CASIO_MAX_EVENT_LINES) {
printk(KERN_ALERT "register_casio_event : full\n");
return;
}

casio_event_log.casio_event[casio_event_log.lines].action = a;
casio_event_log.casio_event[casio_event_log.lines].timestamp = t;
strncpy(casio_event_log.casio_event[casio_event_log.lines].msg, m, CASIO_MSG_SIZE - 1);
casio_event_log.lines++;

}

// Inicializa la estructura de registro de eventos de Casio
void init_casio_event_log() {
char msg[CASIO_MSG_SIZE];
casio_event_log.lines = casio_event_log.cursor = 0;
snprintf(msg, CASIO_MSG_SIZE, "init_casio_event_log: (%lu:%lu)", casio_event_log.lines, casio_event_log.cursor);
register_casio_event(sched_clock(), msg, CASIO_MSG);
}

// Inicializa la estructura de ejecución de tareas de Casio
void init_casio_rq(struct casio_rq* casio_rq) {
casio_rq->casio_rb_root = RB_ROOT;
INIT_LIST_HEAD(&casio_rq->casio_list_head);
atomic_set(&casio_rq->nr_running, 0);
}

// Agrega una tarea a la lista de tareas de Casio
void add_casio_task_2_list(struct casio_rq* rq, struct task_struct* p) {
if (!rq || !p) {
printk(KERN_ALERT "add_casio_task_2_list: null pointers\n");
return;
}

struct list_head* ptr = NULL;
struct casio_task* new = (struct casio_task*)kzalloc(sizeof(struct casio_task), GFP_KERNEL);
if (!new) {
    printk(KERN_ALERT "add_casio_task_2_list: kzalloc\n");
    return;
}

new->task = p;
new->absolute_deadline = 0;
list_for_each(ptr, &rq->casio_list_head) {
    struct casio_task* casio_task = list_entry(ptr, struct casio_task, casio_list_node);
    if (casio_task && new->task->casio_id < casio_task->task->casio_id) {
        list_add(&new->casio_list_node, ptr);
        char msg[CASIO_MSG_SIZE];
        snprintf(msg, CASIO_MSG_SIZE, "add_casio_task_2_list: %d : %d : %llu", new->task->casio_id, new->task->pid, new->absolute_deadline);
        register_casio_event(sched_clock(), msg, CASIO_MSG);
        return;
    }
}

list_add_tail(&new->casio_list_node, &rq->casio_list_head);
char msg[CASIO_MSG_SIZE];
snprintf(msg, CASIO_MSG_SIZE, "add_casio_task_2_list: %d : %d : %llu", new->task->casio_id, new->task
