//#include "../../utils/src/pcb.h"
#include "planificador_corto_plazo.h"
#include "scheduler.h"
#include "semaforos.h"
#include "recursos.h"

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern sem_t sem_dispatch, sem_interrupcion;


pthread_t thread_quantum;
t_temporal *tiempoVRR;

void manejador_de_dispatcher() {
    pthread_t thread_dispatch;

    if (pthread_create(&thread_dispatch, NULL, dispatcher,NULL) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_dispatch);

    return;
}


void* dispatcher(){

    if(strcmp(kernel_config->algoritmo_planificacion, "FIFO")==0){
        
        while(1){ 
            sem_wait(&sem_dispatch);

            cola_ready_a_exec();
            
            send_pcb(conexion_dispatch,scheduler->proceso_ejecutando);
            
            sem_post(&sem_interrupcion);

            }
    }

    if(strcmp(kernel_config->algoritmo_planificacion, "RR")==0){
    
        while(1){ 
            sem_wait(&sem_dispatch);
            
            cola_ready_a_exec();
            
            send_pcb(conexion_dispatch,scheduler->proceso_ejecutando);
            printf("%d", kernel_config->quantum);

            if (pthread_create(&thread_quantum, NULL, thread_hilo_quantum, &kernel_config->quantum) != 0) {
            perror("No se pudo crear el hilo para manejar quantum");
            exit(EXIT_FAILURE);
            }

            sem_post(&sem_interrupcion);
            }
    }


    if(strcmp(kernel_config->algoritmo_planificacion, "VRR")==0){

        while(1){ 
            sem_wait(&sem_dispatch);

            //verifico si la lista aux_bloqueados esta vacia, por las prioridades
            if(!list_is_empty(scheduler->cola_aux_blocked)){
                cola_aux_blocked_a_exec();
                printf("COLA AUX BLOQUEADOS \n");
                
            }else{
                cola_ready_a_exec();
                printf("COLA DE READYS \n");
            }

            send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);
            

            if (pthread_create(&thread_quantum, NULL, thread_hilo_quantum, &scheduler->proceso_ejecutando->quantum) != 0) {
            perror("No se pudo crear el hilo para manejar quantum");
            exit(EXIT_FAILURE);
            }

            //empiezo a contar el tiempo
            tiempoVRR = temporal_create();

            sem_post(&sem_interrupcion);
            }
    }
    return NULL;
}

void* thread_hilo_quantum(void *arg){
    printf("inicio quantum \n");
    uint16_t tiempo = *(uint16_t*)arg;
    usleep(1000*(tiempo));
    //envio orden de desalojo
    enviar_operacion(DESALOJO_QUANTUM, conexion_interrupt);
    printf("Finalizo quantum \n");
    pthread_exit(NULL);
}
