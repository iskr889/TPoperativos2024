//#include "../../utils/src/pcb.h"
#include "planificador_largo_plazo.h"
#include "scheduler.h"
#include "semaforos.h"
#include "recursos.h"

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern sem_t sem_dispatch, sem_interrupcion, sem_multiprogramacion_ready;


void manejador_de_largo_new_a_ready() {
    pthread_t thread_largo_new_a_ready;

    if (pthread_create(&thread_largo_new_a_ready, NULL, planificador_largo_new_a_ready, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_largo_new_a_ready);

    return;
}


void *planificador_largo_new_a_ready(){

    //Codigo de prueba
    pcb_t *pcb1 = crear_proceso(1,2000);
    proceso_a_cola_new(pcb1);

    pcb_t *pcb2 = crear_proceso(2,2000);
    proceso_a_cola_new(pcb2);

    pcb_t *pcb3 = crear_proceso(3,2000);
    proceso_a_cola_new(pcb3);

    pcb_t *pcb4 = crear_proceso(4,2000);
    proceso_a_cola_new(pcb4);

    pcb_t *pcb5 = crear_proceso(5,2000);
    proceso_a_cola_new(pcb5);

    //Termina codigo de prueba

    while(1){

        sem_wait(&sem_multiprogramacion_ready);

        cola_new_a_ready();

       // printf("Contador sem_wait");

    }
    
    return NULL;
}

void cambiar_grado_multiprogramacion(int nuevo_grado_multi){

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
