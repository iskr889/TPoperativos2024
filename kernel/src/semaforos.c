#include "semaforos.h"

extern t_kernel_config* kernel_config;
bool estado_planificacion_activa = true;
int cantidad_procesos_ejecuntandose;
sem_t sem_dispatch;
sem_t sem_interrupcion;
sem_t sem_multiprogramacion_ready;
sem_t sem_iniciar_planificacion;
sem_t sem_iniciar_dispatcher;
sem_t sem_hay_encolado_VRR;
sem_t sem_proceso_a_new;
sem_t sem_planificador_corto_comando;
sem_t sem_planificador_largo_comando;
sem_t sem_manejo_interrupciones_comando;
pthread_mutex_t cantidad_procesos_ejecuntandose_mutex;

void init_semaforos(){
    cantidad_procesos_ejecuntandose = 0;
    pthread_mutex_init(&cantidad_procesos_ejecuntandose_mutex, NULL);
    sem_init(&sem_dispatch, 0, 1);
    sem_init(&sem_interrupcion, 0, 0);
    sem_init(&sem_multiprogramacion_ready, 0, kernel_config->grado_multiprogramacion - 1);
    sem_init(&sem_iniciar_planificacion, 0, 0);
    sem_init(&sem_iniciar_dispatcher, 0, 0);
    sem_init(&sem_hay_encolado_VRR, 0, 0);
    sem_init(&sem_proceso_a_new, 0, 0);
    sem_init(&sem_planificador_corto_comando, 0, 0);
    sem_init(&sem_planificador_largo_comando, 0, 0);
    sem_init(&sem_manejo_interrupciones_comando, 0, 0);
}
