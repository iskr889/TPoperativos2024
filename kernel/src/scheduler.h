#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "main.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>

typedef struct {
    t_dictionary *procesos;
    t_queue *cola_new;
    t_queue *cola_ready;
    pcb_t *proceso_ejecutando;
    t_dictionary *colas_blocked;
    t_queue *cola_exit;
    pthread_mutex_t mutex_procesos;
    pthread_mutex_t mutex_new;
    pthread_mutex_t mutex_ready;
    pthread_mutex_t mutex_exit;
    pthread_mutex_t mutex_exec;
    pthread_mutex_t mutex_blocked;
    pthread_cond_t cond_ready;
    pthread_cond_t cond_exec;
} scheduler_t;

void init_scheduler();

void destroy_scheduler(scheduler_t *scheduler);

void proceso_a_cola_new(pcb_t* proceso);

void cola_new_a_ready();

void cola_ready_a_exec();

void proceso_exec_a_blocked(pcb_t* proceso, char* nombre_cola);

void proceso_exec_a_ready();

void cola_blocked_a_ready(char* nombre_cola);

void proceso_a_exit(pcb_t* proceso, t_queue* cola_actual, pthread_mutex_t* mutex_cola_actual);

#endif
