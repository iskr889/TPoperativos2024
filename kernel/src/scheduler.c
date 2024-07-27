#include "scheduler.h"
#include "manejo_interrupciones_cpu.h"

scheduler_t *scheduler = NULL;

extern t_log* extra_logger, *logger;

static char *generar_lista_pids(t_list *cola) {
    char* cadena = string_new();
    t_list_iterator *cola_iterator = list_iterator_create(cola);

    pcb_t *elemento = list_iterator_next(cola_iterator);
    char *pid_str = string_itoa(elemento->pid);
    string_append(&cadena, pid_str);
    free(pid_str);

    while (list_iterator_has_next(cola_iterator)) {
        elemento = list_iterator_next(cola_iterator);
        pid_str = string_itoa(elemento->pid);
        string_append_with_format(&cadena, ", %s", pid_str);
        free(pid_str);
    }

    list_iterator_destroy(cola_iterator);
    return cadena;
}

void init_scheduler() {
    scheduler = malloc(sizeof(scheduler_t));
    if (scheduler == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }
    scheduler->procesos = dictionary_create();
    scheduler->cola_new = list_create();
    scheduler->cola_ready = list_create();
    scheduler->proceso_ejecutando = NULL;
    scheduler->colas_blocked = dictionary_create();
    scheduler->cola_exit = list_create();
    scheduler->cola_aux_blocked = list_create();
    pthread_mutex_init(&scheduler->mutex_procesos, NULL);
    pthread_mutex_init(&scheduler->mutex_new, NULL);
    pthread_mutex_init(&scheduler->mutex_ready, NULL);
    pthread_mutex_init(&scheduler->mutex_exit, NULL);
    pthread_mutex_init(&scheduler->mutex_exec, NULL);
    pthread_mutex_init(&scheduler->mutex_blocked, NULL);
    pthread_mutex_init(&scheduler->mutex_aux_blocked, NULL);
    pthread_cond_init(&scheduler->cond_new, NULL);
    pthread_cond_init(&scheduler->cond_ready, NULL);
    pthread_cond_init(&scheduler->cond_exec, NULL);
    pthread_cond_init(&scheduler->cond_aux_blocked, NULL);
    return;
}

static void free_cola_blocked(void *cola_blocked) {
    list_destroy((t_list *)cola_blocked);
}

void destroy_scheduler(scheduler_t *scheduler) {
    if (scheduler == NULL)
        return;
    dictionary_destroy_and_destroy_elements(scheduler->procesos, free); // Libero todos los procesos del kernel
    list_destroy(scheduler->cola_new);
    list_destroy(scheduler->cola_ready);
    scheduler->proceso_ejecutando = NULL;
    dictionary_destroy_and_destroy_elements(scheduler->colas_blocked, free_cola_blocked);
    list_destroy(scheduler->cola_exit);
    list_destroy(scheduler->cola_aux_blocked);
    pthread_mutex_destroy(&scheduler->mutex_procesos);
    pthread_mutex_destroy(&scheduler->mutex_new);
    pthread_mutex_destroy(&scheduler->mutex_ready);
    pthread_mutex_destroy(&scheduler->mutex_exit);
    pthread_mutex_destroy(&scheduler->mutex_exec);
    pthread_mutex_destroy(&scheduler->mutex_blocked);
    free(scheduler);
}

void proceso_a_cola_new(pcb_t* proceso) {
    pthread_mutex_lock(&scheduler->mutex_new);
    proceso->estado = NEW;
    list_push(scheduler->cola_new, proceso);
    pthread_cond_signal(&scheduler->cond_new);
    pthread_mutex_unlock(&scheduler->mutex_new);
}

void cola_new_a_ready() {
    pthread_mutex_lock(&scheduler->mutex_new);
    if (list_is_empty(scheduler->cola_new)) {
        pthread_cond_wait(&scheduler->cond_new, &scheduler->mutex_new);
    }
    pcb_t* proceso = list_pop(scheduler->cola_new);
    pthread_mutex_unlock(&scheduler->mutex_new);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    list_push(scheduler->cola_ready, proceso);
    char *lista_pids = generar_lista_pids(scheduler->cola_ready);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", proceso->pid);
    log_info(logger, "Cola Ready: [%s]", lista_pids);

    free(lista_pids);
}

void cola_ready_a_exec() {
    pthread_mutex_lock(&scheduler->mutex_ready);
    while (list_is_empty(scheduler->cola_ready)) {
        pthread_cond_wait(&scheduler->cond_ready, &scheduler->mutex_ready);
    }
    pcb_t* proceso = list_pop(scheduler->cola_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    pthread_mutex_lock(&scheduler->mutex_exec);
    scheduler->proceso_ejecutando = proceso;
    proceso->estado = EXEC;
    pthread_cond_signal(&scheduler->cond_exec);
    pthread_mutex_unlock(&scheduler->mutex_exec);

    log_info(logger, "PID: %d - Estado Anterior: READY - Estado Actual: EXEC", proceso->pid);
}

void cola_aux_blocked_a_exec() {
    pthread_mutex_lock(&scheduler->mutex_aux_blocked);
    while (list_is_empty(scheduler->cola_aux_blocked)) {
        pthread_cond_wait(&scheduler->cond_aux_blocked, &scheduler->mutex_aux_blocked);
    }
    pcb_t* proceso = list_pop(scheduler->cola_aux_blocked);
    pthread_mutex_unlock(&scheduler->mutex_aux_blocked);

    pthread_mutex_lock(&scheduler->mutex_exec);
    scheduler->proceso_ejecutando = proceso;
    proceso->estado = EXEC;
    pthread_cond_signal(&scheduler->cond_exec);
    pthread_mutex_unlock(&scheduler->mutex_exec);

    log_info(logger, "PID: %d - Estado Anterior: AUX_BLOCKED - Estado Actual: EXEC", proceso->pid);
}

void proceso_exec_a_blocked(char* nombre_cola) {

    pthread_mutex_lock(&scheduler->mutex_exec);
    pcb_t* proceso = scheduler->proceso_ejecutando;
    scheduler->proceso_ejecutando = NULL;
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_blocked);
    t_list* cola_bloqueada = dictionary_get(scheduler->colas_blocked, nombre_cola);
    if (cola_bloqueada == NULL) {
        cola_bloqueada = list_create();
        dictionary_put(scheduler->colas_blocked, nombre_cola, cola_bloqueada);
    }
    proceso->estado = BLOCKED;
    list_push(cola_bloqueada, proceso);
    pthread_mutex_unlock(&scheduler->mutex_blocked);

    log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCKED", proceso->pid);
}

void proceso_exec_a_ready() {
    pthread_mutex_lock(&scheduler->mutex_exec);
    pcb_t* proceso = scheduler->proceso_ejecutando;
    scheduler->proceso_ejecutando = NULL;
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    list_push(scheduler->cola_ready, proceso);
    char *lista_pids = generar_lista_pids(scheduler->cola_ready);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: READY", proceso->pid);
    log_info(logger, "Cola Ready: [%s]", lista_pids);
}

void cola_blocked_a_ready(char* nombre_cola) {
    pthread_mutex_lock(&scheduler->mutex_blocked);
    t_list* cola_bloqueada = dictionary_get(scheduler->colas_blocked, nombre_cola);
    if (cola_bloqueada == NULL || list_is_empty(cola_bloqueada)) {
        pthread_mutex_unlock(&scheduler->mutex_blocked);
        return;
    }
    pcb_t* proceso = list_pop(cola_bloqueada);
    pthread_mutex_unlock(&scheduler->mutex_blocked);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    list_push(scheduler->cola_ready, proceso);
    char *lista_pids = generar_lista_pids(scheduler->cola_ready);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    log_info(logger, "Cola Ready: [%s]", lista_pids);
    log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: READY", proceso->pid);
}

void cola_blocked_a_aux_blocked(char* nombre_cola) {
    pthread_mutex_lock(&scheduler->mutex_blocked);
    t_list* cola_bloqueada = dictionary_get(scheduler->colas_blocked, nombre_cola);
    if (cola_bloqueada == NULL || list_is_empty(cola_bloqueada)) {
        pthread_mutex_unlock(&scheduler->mutex_blocked);
        return;
    }
    pcb_t* proceso = list_pop(cola_bloqueada);
    pthread_mutex_unlock(&scheduler->mutex_blocked);

    pthread_mutex_lock(&scheduler->mutex_aux_blocked);
    proceso->estado = READY;
    list_push(scheduler->cola_aux_blocked, proceso);
    char *lista_pids = generar_lista_pids(scheduler->cola_aux_blocked);
    pthread_cond_signal(&scheduler->cond_aux_blocked);
    pthread_mutex_unlock(&scheduler->mutex_aux_blocked);

    log_info(logger, "PID: %d - Estado Anterior: BLOCKED - Estado Actual: AUX_BLOCKED", proceso->pid);
    log_info(logger, "Cola Ready Prioridad: [%s]", lista_pids);
}

void proceso_a_exit(pcb_t* proceso, t_list* cola_actual) {
    if (proceso == NULL)
        return;

    list_pop(cola_actual);

    if(proceso->estado != NEW)
        aumentar_grado_multiprogramacion();

    pthread_mutex_lock(&scheduler->mutex_exit);
    proceso->estado = EXIT;
    list_push(scheduler->cola_exit, proceso);
    pthread_mutex_unlock(&scheduler->mutex_exit);
}

void proceso_exec_a_exit() {
    pthread_mutex_lock(&scheduler->mutex_exec);
    pcb_t* proceso = scheduler->proceso_ejecutando;
    scheduler->proceso_ejecutando = NULL;
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_exit);
    proceso->estado = EXIT;
    list_push(scheduler->cola_exit, proceso);
    pthread_mutex_unlock(&scheduler->mutex_exit);

    log_info(logger, "PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", proceso->pid);
}

void list_push(t_list* queue, void* element) {
    list_add(queue, element);
}

void* list_pop(t_list* queue) {
    if (list_is_empty(queue))
        return NULL;
    return list_remove(queue, 0);
}

void actualizar_contexto_de_ejecucion(pcb_t *old_pcb, pcb_t *new_pcb) {
    if (old_pcb->pid != new_pcb->pid) {
        fprintf(stderr, "Se recibio un PID que no coincide con el proceso actual en ejecución!");
        exit(EXIT_FAILURE);
    }
    old_pcb->estado = new_pcb->estado;
    old_pcb->registros.ax = new_pcb->registros.ax;
    old_pcb->registros.bx = new_pcb->registros.bx;
    old_pcb->registros.cx = new_pcb->registros.cx;
    old_pcb->registros.dx = new_pcb->registros.dx;
    old_pcb->registros.eax = new_pcb->registros.eax;
    old_pcb->registros.ebx = new_pcb->registros.ebx;
    old_pcb->registros.ecx = new_pcb->registros.ecx;
    old_pcb->registros.edx = new_pcb->registros.edx;
    old_pcb->registros.pc = new_pcb->registros.pc;
    old_pcb->registros.si = new_pcb->registros.si;
    old_pcb->registros.di = new_pcb->registros.di;
}