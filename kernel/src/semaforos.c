#include "semaforos.h"

extern t_kernel_config* kernel_config;
sem_t sem_dispatch;
sem_t sem_interrupcion;
sem_t sem_multiprogramacion_ready;

void init_semaforos(){
    sem_init(&sem_dispatch, 0, 1);
    sem_init(&sem_interrupcion, 0, 0);
    sem_init(&sem_multiprogramacion_ready, 0, kernel_config->grado_multiprogramacion);
}
