#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "main.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>

typedef struct {
    t_dictionary *procesos;
    t_list *cola_new;
    t_list *cola_ready;
    pcb_t *proceso_ejecutando;
    t_dictionary *colas_blocked;
    t_list *cola_exit;
    t_list *cola_aux_blocked; //agrego cola auxiliar de bloqueados
    pthread_mutex_t mutex_procesos;
    pthread_mutex_t mutex_new;
    pthread_mutex_t mutex_ready;
    pthread_mutex_t mutex_exit;
    pthread_mutex_t mutex_exec;
    pthread_mutex_t mutex_blocked;
    pthread_mutex_t mutex_aux_blocked; // agrego mutex cola auxiliar bloqueados
    pthread_cond_t cond_ready;
    pthread_cond_t cond_exec;
    pthread_cond_t cond_aux_blocked;
} scheduler_t;

void init_scheduler();

void destroy_scheduler(scheduler_t *scheduler);

void proceso_a_cola_new(pcb_t* proceso);

void cola_new_a_ready();

void cola_ready_a_exec();

void cola_aux_blocked_a_exec();

void proceso_exec_a_blocked(pcb_t* proceso, char* nombre_cola);

void proceso_exec_a_ready();

void pcb_a_ready(pcb_t* proceso);

void cola_blocked_a_ready(char* nombre_cola);

void cola_blocked_a_aux_blocked(char* nombre_cola);

void proceso_a_exit(pcb_t* proceso, t_list* cola_actual, pthread_mutex_t* mutex_cola_actual);

void pcb_a_blocked(pcb_t* proceso, char* nombre_cola);

void pcb_a_exit(pcb_t* proceso);

void list_push(t_list* queue, void* element);

void* list_pop(t_list* queue);

#endif
