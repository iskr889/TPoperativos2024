//#include "semaforos.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>


sem_t sem_dispatch;
sem_t sem_interrupcion;

void init_semaforos(){
    sem_init(&sem_dispatch, 0, 1);
    sem_init(&sem_interrupcion,0,0);

}
