#include "pcb.h"

// Función para inicializar el planificador
scheduler_t* init_scheduler() {
    scheduler_t *scheduler = malloc(sizeof(scheduler_t));
    if (scheduler == NULL) {
        perror("Error en malloc()");
        return NULL;
    }
    scheduler->cola_new = queue_create();
    scheduler->cola_ready = queue_create();
    scheduler->execute = NULL;
    scheduler->colas_blocked = dictionary_create();
    scheduler->cola_exit = queue_create();
    return scheduler;
}

// Función para destruir el planificador
void destroy_scheduler(scheduler_t *scheduler) {
    if (scheduler != NULL) {
        queue_destroy_and_destroy_elements(scheduler->cola_new, free);
        queue_destroy_and_destroy_elements(scheduler->cola_ready, free);
        free(scheduler->execute);
        dictionary_destroy_and_destroy_elements(scheduler->colas_blocked, free);
        queue_destroy_and_destroy_elements(scheduler->cola_exit, free);
        free(scheduler);
    }
}
