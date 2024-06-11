#include "scheduler.h"
#include "manejo_interrupciones_cpu.h"
#include "kernel_interface_handler.h"
#include "semaforos.h"
#include "recursos.h"

#include <inttypes.h>

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern pthread_t thread_quantum;
extern sem_t sem_dispatch, sem_interrupcion;
extern t_dictionary *interfaces;
extern t_temporal *tiempoVRR;
int64_t tiempo_VRR_restante;
pthread_t thread_mock_IO;
extern t_dictionary *recursos;


void* manejo_interrupciones_cpu(){
    while(1){

        paquete_t *paquete = recibir_interrupcion(conexion_dispatch);

        //VRR
        if(tiempoVRR != NULL){
            tiempo_VRR_restante = temporal_gettime(tiempoVRR);
            temporal_destroy(tiempoVRR);
        }

        int codigo_operacion = paquete->operacion;
        interrupcion_t *interrupcion = interrupcion_deserializar(paquete->payload);
        
        switch(codigo_operacion){
            case FINALIZADO:
            
                sem_wait(&sem_interrupcion);
                printf("Proceso finalizado \n");
                imprimir_pcb(interrupcion->pcb);
                pthread_cancel(thread_quantum);
                pcb_a_exit(interrupcion->pcb);
            
                //grado_multiprogramacion ++;

                sem_post(&sem_dispatch);
            break;

            case DESALOJO_QUANTUM:
                sem_wait(&sem_interrupcion);
                printf("Proceso desalojado por quantum \n" );

                //Reinicializar quantum de nuevo para VRR
                imprimir_pcb(interrupcion->pcb);
                if(tiempoVRR != NULL){
                    actualizar_quantum(interrupcion->pcb, kernel_config->quantum);
                }
                
                pcb_a_ready(interrupcion->pcb);

                sem_post(&sem_dispatch);

            break;

            case IO:
            
                sem_wait(&sem_interrupcion);
                pthread_cancel(thread_quantum);

                //interfaz_t *interfaz = dictionary_get(interfaces,interrupcion->instruccion->interfaz);
                //enviar_instruccion(interfaz->socket, interrupcion->instruccion);


                if(tiempoVRR !=NULL){
                    actualizar_quantum(interrupcion->pcb, tiempo_VRR_restante);
                }
                imprimir_pcb(interrupcion->pcb);
                printf("El valor es: %" PRIu64 "\n", tiempo_VRR_restante);
                
                pcb_a_blocked(interrupcion->pcb, interrupcion->instruccion->interfaz);

                //mock_ejecutar_interfaz, ver tema interfaces

                if (pthread_create(&thread_mock_IO, NULL, thread_hilo_mock_IO, interrupcion->instruccion->interfaz) != 0) {
                perror("No se pudo crear el hilo para manejar quantum");
                exit(EXIT_FAILURE);
                }
                
                sem_post(&sem_dispatch);
            break;

            case WAIT:
                sem_wait(&sem_interrupcion);
                printf("WAIT \n");
                imprimir_pcb(interrupcion->pcb);
                pthread_cancel(thread_quantum);
                
                if(tiempoVRR != NULL){//ver tema flag para VRR
                        actualizar_quantum(interrupcion->pcb, kernel_config->quantum);
                    }

                if(!dictionary_has_key(recursos, interrupcion->instruccion->recurso)){
                    pcb_a_exit(interrupcion->pcb);
                }else if(obtenerInstancia(recursos, interrupcion->instruccion->recurso) > 0){
                    restar_recurso(recursos,interrupcion->instruccion->recurso );
                }else{
                    pcb_a_blocked(interrupcion->pcb,interrupcion->instruccion->recurso );
                }
                imprimir_instancia_prueba(recursos, "RA");
                sem_post(&sem_dispatch);

            break;

            case SIGNAL:
                sem_wait(&sem_interrupcion);
                printf("SIGNAL \n");
                imprimir_pcb(interrupcion->pcb);
                pthread_cancel(thread_quantum);

                if(tiempoVRR != NULL){
                        actualizar_quantum(interrupcion->pcb, kernel_config->quantum);
                    }
                if(!dictionary_has_key(recursos, interrupcion->instruccion->recurso)){
                    pcb_a_exit(interrupcion->pcb);

                }else if(obtenerInstancia(recursos, interrupcion->instruccion->recurso) > 0){
                    if(tiempoVRR ==NULL){//deberia crear un flag para VRR
                        cola_blocked_a_aux_blocked(interrupcion->instruccion->recurso);
                    }else{
                        cola_blocked_a_ready(interrupcion->instruccion->recurso);
                    }
                }
                sumar_recurso(recursos, interrupcion->instruccion->recurso);

                sem_post(&sem_dispatch);

            break;

        }
    }
    
return NULL;
}


void actualizar_quantum(pcb_t *pcb, int quantum){
    pcb->quantum = quantum;

}

void *thread_hilo_mock_IO(void *arg){
    printf("MOCKEO ENVIAR INTERFAZ \n");
    usleep(300);
    cola_blocked_a_aux_blocked(arg);
    pthread_exit(NULL);
}
