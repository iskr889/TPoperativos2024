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

// Crear un nuevo PCB
pcb_t* crear_proceso(uint16_t pid, uint16_t quantum) {
    pcb_t *proceso = malloc(sizeof(pcb_t));
    if (proceso == NULL) {
        perror("Error en malloc()");
        return NULL; // Fallo en la asignación de memoria
    }
    proceso->pid = pid;
    proceso->quantum = quantum;
    proceso->registros.pc  = 0;
    proceso->registros.ax  = 0; proceso->registros.bx  = 0; proceso->registros.cx  = 0; proceso->registros.dx  = 0;
    proceso->registros.eax = 0; proceso->registros.ebx = 0; proceso->registros.ecx = 0; proceso->registros.edx = 0;
    proceso->registros.si  = 0; proceso->registros.di = 0;
    proceso->estado = NEW; // El estado inicial del proceso es NEW
    return proceso;
}
