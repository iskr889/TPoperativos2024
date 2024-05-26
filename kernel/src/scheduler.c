#include "scheduler.h"

scheduler_t *scheduler = NULL;

// Función para inicializar el planificador
void init_scheduler() {
    scheduler = malloc(sizeof(scheduler_t));
    if (scheduler == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }
    scheduler->procesos = dictionary_create();
    scheduler->cola_new = queue_create();
    scheduler->cola_ready = queue_create();
    scheduler->proceso_ejecutando = NULL;
    scheduler->colas_blocked = dictionary_create();
    scheduler->cola_exit = queue_create();
    pthread_mutex_init(&scheduler->mutex_procesos, NULL);
    pthread_mutex_init(&scheduler->mutex_new, NULL);
    pthread_mutex_init(&scheduler->mutex_ready, NULL);
    pthread_mutex_init(&scheduler->mutex_exit, NULL);
    pthread_mutex_init(&scheduler->mutex_exec, NULL);
    pthread_mutex_init(&scheduler->mutex_blocked, NULL);
    pthread_cond_init(&scheduler->cond_ready, NULL);
    pthread_cond_init(&scheduler->cond_exec, NULL);
    return;
}

// Función para destruir el planificador
void destroy_scheduler(scheduler_t *scheduler) {
    if (scheduler == NULL)
        return;
    dictionary_destroy_and_destroy_elements(scheduler->procesos, free);
    queue_destroy_and_destroy_elements(scheduler->cola_new, free);
    queue_destroy_and_destroy_elements(scheduler->cola_ready, free);
    scheduler->proceso_ejecutando = NULL;
    dictionary_destroy_and_destroy_elements(scheduler->colas_blocked, free);
    queue_destroy_and_destroy_elements(scheduler->cola_exit, free);
    pthread_mutex_destroy(&scheduler->mutex_procesos);
    pthread_mutex_destroy(&scheduler->mutex_new);
    pthread_mutex_destroy(&scheduler->mutex_ready);
    pthread_mutex_destroy(&scheduler->mutex_exit);
    pthread_mutex_destroy(&scheduler->mutex_exec);
    pthread_mutex_destroy(&scheduler->mutex_blocked);
    pthread_cond_destroy(&scheduler->cond_ready);
    pthread_cond_destroy(&scheduler->cond_exec);
    free(scheduler);
}

void proceso_a_cola_new(pcb_t* proceso) {
    pthread_mutex_lock(&scheduler->mutex_new);
    proceso->estado = NEW;
    queue_push(scheduler->cola_new, proceso);
    pthread_mutex_unlock(&scheduler->mutex_new);
}

void cola_new_a_ready() {
    pthread_mutex_lock(&scheduler->mutex_new);
    if (queue_is_empty(scheduler->cola_new)) {
        pthread_mutex_unlock(&scheduler->mutex_new);
        return;
    }
    pcb_t* proceso = queue_pop(scheduler->cola_new);
    pthread_mutex_unlock(&scheduler->mutex_new);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    queue_push(scheduler->cola_ready, proceso);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);
}

void cola_ready_a_exec() {
    pthread_mutex_lock(&scheduler->mutex_ready);
    while (queue_is_empty(scheduler->cola_ready)) {
        pthread_cond_wait(&scheduler->cond_ready, &scheduler->mutex_ready);
    }
    pcb_t* proceso = queue_pop(scheduler->cola_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    pthread_mutex_lock(&scheduler->mutex_exec);
    scheduler->proceso_ejecutando = proceso;
    proceso->estado = EXEC;
    pthread_cond_signal(&scheduler->cond_exec);
    pthread_mutex_unlock(&scheduler->mutex_exec);
}

void proceso_exec_a_blocked(pcb_t* proceso, char* nombre_cola) {
    pthread_mutex_lock(&scheduler->mutex_exec);
    scheduler->proceso_ejecutando = NULL;
    proceso->estado = BLOCKED;
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_blocked);
    t_queue* cola_bloqueada = dictionary_get(scheduler->colas_blocked, nombre_cola);
    if (cola_bloqueada == NULL) {
        cola_bloqueada = queue_create();
        dictionary_put(scheduler->colas_blocked, nombre_cola, cola_bloqueada);
    }
    queue_push(cola_bloqueada, proceso);
    pthread_mutex_unlock(&scheduler->mutex_blocked);
}

void proceso_exec_a_ready() {
    pthread_mutex_lock(&scheduler->mutex_exec);
    pcb_t* proceso = scheduler->proceso_ejecutando;
    scheduler->proceso_ejecutando = NULL;
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    queue_push(scheduler->cola_ready, proceso);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);
}

void cola_blocked_a_ready(char* nombre_cola) {
    pthread_mutex_lock(&scheduler->mutex_blocked);
    t_queue* cola_bloqueada = dictionary_get(scheduler->colas_blocked, nombre_cola);
    if (cola_bloqueada == NULL || queue_is_empty(cola_bloqueada)) {
        pthread_mutex_unlock(&scheduler->mutex_blocked);
        return;
    }
    pcb_t* proceso = queue_pop(cola_bloqueada);
    pthread_mutex_unlock(&scheduler->mutex_blocked);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso->estado = READY;
    queue_push(scheduler->cola_ready, proceso);
    pthread_cond_signal(&scheduler->cond_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);
}

void proceso_a_exit(pcb_t* proceso, t_queue* cola_actual, pthread_mutex_t* mutex_cola_actual) {
    pthread_mutex_lock(mutex_cola_actual);
    queue_pop(cola_actual);
    pthread_mutex_unlock(mutex_cola_actual);

    pthread_mutex_lock(&scheduler->mutex_exit);
    proceso->estado = EXIT;
    queue_push(scheduler->cola_exit, proceso);
    pthread_mutex_unlock(&scheduler->mutex_exit);
}
