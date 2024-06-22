//#include "../../utils/src/pcb.h"
#include "planificador_largo_plazo.h"
#include "scheduler.h"

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern sem_t sem_dispatch, sem_interrupcion, sem_multiprogramacion_ready;

void planificador_largo_plazo() {
    pthread_t thread_largo_new_a_ready;

    if (pthread_create(&thread_largo_new_a_ready, NULL, planificador_largo_new_a_ready, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_largo_new_a_ready);

    return;
}

void *planificador_largo_new_a_ready(){

    while(1) {
        sem_wait(&sem_multiprogramacion_ready);
        cola_new_a_ready();
    }
    
    return NULL;
}

void cambiar_grado_multiprogramacion(int nuevo_grado_multi) {

    int diferencia_grado = nuevo_grado_multi - kernel_config -> grado_multiprogramacion ;

    if (diferencia_grado > 0) {
        for (int i = 0; i < diferencia_grado; i++) {
            sem_post(&sem_multiprogramacion_ready);
        }
    } else if (diferencia_grado < 0) {
        for (int i = 0; i < abs(diferencia_grado); i++) {
            sem_wait(&sem_multiprogramacion_ready);
        }
    }
}
