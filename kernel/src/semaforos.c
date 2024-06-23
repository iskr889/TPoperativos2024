#include "semaforos.h"

extern t_kernel_config* kernel_config;
sem_t sem_dispatch;
sem_t sem_interrupcion;
sem_t sem_multiprogramacion_ready;
sem_t sem_iniciar_planificacion;
sem_t sem_iniciar_dispatcher;
sem_t sem_hay_encolado_VRR;
sem_t sem_proceso_a_new;

void init_semaforos(){
    sem_init(&sem_dispatch, 0, 1);
    sem_init(&sem_interrupcion, 0, 0);
    sem_init(&sem_multiprogramacion_ready, 0, kernel_config->grado_multiprogramacion);
    sem_init(&sem_iniciar_planificacion, 0, 0);
    sem_init(&sem_iniciar_dispatcher, 0, 0);
    sem_init(&sem_hay_encolado_VRR, 0, 0);
    sem_init(&sem_proceso_a_new, 0, 0);
}
