#include "planificador_corto_plazo.h"
#include "scheduler.h"

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern sem_t sem_dispatch, sem_interrupcion, sem_iniciar_dispatcher, sem_hay_encolado_VRR, sem_planificador_corto_comando;
extern bool estado_planificacion_activa;
pthread_t thread_quantum;

bool VRR_modo = false;
bool FIFO_modo = false;
t_temporal *tiempoVRR;
pthread_mutex_t mutex_tiempoVRR;

void dispatch_handler() {
    pthread_t thread_dispatch;

    if (pthread_create(&thread_dispatch, NULL, dispatcher, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_dispatch);

    return;
}


void* dispatcher(){

    if (strcmp(kernel_config->algoritmo_planificacion, "FIFO") == 0) {
        FIFO_modo = true;

        while(1) { 
            sem_wait(&sem_dispatch);
            
            if (!estado_planificacion_activa) {
                sem_wait(&sem_planificador_corto_comando);
            }

            cola_ready_a_exec();
            
            send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);
            
            sem_post(&sem_interrupcion);
        }

    } else if (strcmp(kernel_config->algoritmo_planificacion, "RR") == 0) {
    
        while(1) { 
            sem_wait(&sem_dispatch);

            if (!estado_planificacion_activa) {
                sem_wait(&sem_planificador_corto_comando);
            }

            
            cola_ready_a_exec();
            
            send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);

            if (pthread_create(&thread_quantum, NULL, thread_hilo_quantum, &kernel_config->quantum) != 0) {
                perror("No se pudo crear el hilo para manejar quantum");
                exit(EXIT_FAILURE);
            }

            sem_post(&sem_interrupcion);
        }

    } else if (strcmp(kernel_config->algoritmo_planificacion, "VRR") == 0) {
        pthread_mutex_init(&mutex_tiempoVRR, NULL);
        VRR_modo = true;

        while(1) { 
            sem_wait(&sem_dispatch);
            log_info(extra_logger, "Paso sem_dispatch");//Prueba
            sem_wait(&sem_hay_encolado_VRR);
            log_info(extra_logger, "Paso sem_encolado_VRR");//Prueba

            if (!estado_planificacion_activa) {
                sem_wait(&sem_planificador_corto_comando);
            }

            //verifico si la lista aux_bloqueados esta vacia, por las prioridades
            pthread_mutex_lock(&scheduler->mutex_aux_blocked);

            if (!list_is_empty(scheduler->cola_aux_blocked)) {
                pthread_mutex_unlock(&scheduler->mutex_aux_blocked);
                cola_aux_blocked_a_exec();
                log_debug(extra_logger, "Paso proceso cola de Aux Bloqueados");
            } else {
                pthread_mutex_unlock(&scheduler->mutex_aux_blocked);
                cola_ready_a_exec();
                log_debug(extra_logger, "Paso proceso cola de Ready");
            }

            send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);
            
            if (pthread_create(&thread_quantum, NULL, thread_hilo_quantum, &scheduler->proceso_ejecutando->quantum) != 0) {
                perror("No se pudo crear el hilo para manejar quantum");
                exit(EXIT_FAILURE);
            }

            //empiezo a contar el tiempo
            pthread_mutex_lock(&mutex_tiempoVRR);
            tiempoVRR = temporal_create();
            pthread_mutex_unlock(&mutex_tiempoVRR);

            sem_post(&sem_interrupcion);
        }
    }

    return NULL;
}

void* thread_hilo_quantum(void *tiempo) {
    log_debug(extra_logger, "Inicio Quantum: %d", *(uint16_t*)tiempo);
    ESPERAR_X_MILISEGUNDOS(*(uint16_t*)tiempo);
    enviar_operacion(DESALOJO_QUANTUM, conexion_interrupt); // Envio orden de desalojo
    log_debug(extra_logger, "Finalizo Quantum");
    pthread_exit(NULL);
}
