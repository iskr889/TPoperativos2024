#include "planificador_largo_plazo.h"
#include "scheduler.h"

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern bool VRR_modo;
extern bool estado_planificacion_activa;
extern int cantidad_procesos_ejecuntandose;
extern sem_t sem_dispatch, sem_interrupcion, sem_multiprogramacion_ready, sem_hay_encolado_VRR, sem_planificador_largo_comando;
extern pthread_mutex_t cantidad_procesos_ejecuntandose_mutex;

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
        
        if (!estado_planificacion_activa) {
            sem_wait(&sem_planificador_largo_comando);
        }

        esperar_confirmacion_memoria();
        cola_new_a_ready();

        pthread_mutex_lock(&cantidad_procesos_ejecuntandose_mutex);
        cantidad_procesos_ejecuntandose++;
        pthread_mutex_unlock(&cantidad_procesos_ejecuntandose_mutex);
        //log_info(extra_logger, "Paso sem iniciar dispather largo plazo");
        if (VRR_modo) sem_post(&sem_hay_encolado_VRR);
        //log_info(extra_logger, "Paso sem hay encolado largo plazo");
    }
    
    return NULL;
}

void cambiar_grado_multiprogramacion(int nuevo_grado_multi) {
    int diferencia =  nuevo_grado_multi - kernel_config->grado_multiprogramacion;

    while(diferencia-- > 0)
        sem_post(&sem_multiprogramacion_ready);

    kernel_config->grado_multiprogramacion = nuevo_grado_multi;
}

void esperar_confirmacion_memoria() {
    if(recibir_operacion(conexion_memoria) == MEMORY_INVALID_PATH)
        exit(EXIT_FAILURE);
}
