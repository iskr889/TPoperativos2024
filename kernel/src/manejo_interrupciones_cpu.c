#include "scheduler.h"
#include "manejo_interrupciones_cpu.h"
#include "kernel_interface_handler.h"
#include "recursos.h"
#include "planificador_corto_plazo.h"
#include "consola.h"

#include <inttypes.h>

extern t_log* extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern pthread_t thread_quantum;
extern sem_t sem_dispatch, sem_interrupcion, sem_multiprogramacion_ready, sem_hay_encolado_VRR;
extern t_dictionary *interfaces;
extern t_temporal *tiempoVRR;
int64_t tiempo_VRR_restante;
//pthread_t thread_mock_IO;
extern t_dictionary *recursos;
extern bool VRR_modo, FIFO_modo;
extern pthread_mutex_t mutex_tiempoVRR;

void interrupt_handler() {
    pthread_t thread_manejo_interrupciones;

    if (pthread_create(&thread_manejo_interrupciones, NULL, manejo_interrupciones_cpu, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_manejo_interrupciones);

    return;
}


void* manejo_interrupciones_cpu(){

    while(1){

        paquete_t *paquete = recibir_interrupcion(conexion_dispatch);

        //VRR
        pthread_mutex_lock(&mutex_tiempoVRR);
        if(VRR_modo){
            tiempo_VRR_restante = temporal_gettime(tiempoVRR);
            temporal_destroy(tiempoVRR);
        }
        pthread_mutex_unlock(&mutex_tiempoVRR);

        int codigo_operacion = paquete->operacion;
        interrupcion_t *interrupcion = interrupcion_deserializar(paquete->payload);

        char **tokens = split_string(interrupcion->instruccion);
        
        switch(codigo_operacion){
            case FINALIZADO:
            
                sem_wait(&sem_interrupcion);
                printf("Proceso finalizado \n");
                imprimir_pcb(interrupcion->pcb);
                if (!FIFO_modo) pthread_cancel(thread_quantum);
                pcb_a_exit(interrupcion->pcb);
                //finalizar_proceso_en_memoria(interrupcion->pcb->pid);
                log_info(extra_logger, "Finalizo proceso %d - Motivo: SUCCESS", interrupcion->pcb->pid);
                sem_post(&sem_dispatch);
                sem_post(&sem_multiprogramacion_ready);//aumento grado multi
                
            break;

            case DESALOJO_QUANTUM:

                sem_wait(&sem_interrupcion);
                if (VRR_modo) actualizar_quantum(interrupcion->pcb, kernel_config->quantum);
                pcb_a_ready(interrupcion->pcb);
                if (VRR_modo) sem_post(&sem_hay_encolado_VRR);
                log_info(extra_logger, "PID: %d - Desalojado por Fin de Quantum", interrupcion->pcb->pid);
                sem_post(&sem_dispatch);

            break;

            case IO:
            
                sem_wait(&sem_interrupcion);
                if (!FIFO_modo) pthread_cancel(thread_quantum);

                if (VRR_modo) actualizar_quantum(interrupcion->pcb, tiempo_VRR_restante);
                
                if (!dictionary_has_key(interfaces,tokens[1])) {//verifico que existe la interfaz

                    pcb_a_exit(interrupcion->pcb);//si No existe
                    finalizar_proceso_en_memoria(interrupcion->pcb->pid);
                    log_info(extra_logger, "Finalizo proceso %d - Motivo: INVALID_WRITE", interrupcion->pcb->pid);
                    sem_post(&sem_multiprogramacion_ready);//aumento grado multi

                } else if (!verificar_instruccion(interfaces, tokens)) {//Si NO existe

                    pcb_a_exit(interrupcion->pcb);//si No existe
                    finalizar_proceso_en_memoria(interrupcion->pcb->pid);
                    sem_post(&sem_multiprogramacion_ready);//aumento grado multi
                    
                } else {

                    interfaz_t *interfaz = dictionary_get(interfaces, tokens[1]);
                    pcb_a_blocked(interrupcion->pcb, tokens[1]); //SI existe
                    log_info(extra_logger, "PID: %d Bloqueado por: %s",interrupcion->pcb->pid, tokens[1]);
                    agregar_instruccion(interfaz, tokens);
                    sem_post(&interfaz->sem_IO_ejecucion);
                }
            
                sem_post(&sem_dispatch);

            break;

            case WAIT:

                sem_wait(&sem_interrupcion);
                if (!FIFO_modo) pthread_cancel(thread_quantum);
                
                if (VRR_modo) actualizar_quantum(interrupcion->pcb, kernel_config->quantum);

                if (!dictionary_has_key(recursos, tokens[1])) {

                    pcb_a_exit(interrupcion->pcb);
                    finalizar_proceso_en_memoria(interrupcion->pcb->pid);
                    log_info(extra_logger, "Finalizo proceso %d - Motivo: INVALID_RESOURCE", interrupcion->pcb->pid);
                    sem_post(&sem_multiprogramacion_ready);//aumento grado multi

                } else if (obtenerInstancia(recursos, tokens[1]) > 0) {

                    restar_recurso(recursos,tokens[1]);

                } else {

                    pcb_a_blocked(interrupcion->pcb,tokens[1]);
                    log_info(extra_logger, "PID: %d Bloqueado por: %s",interrupcion->pcb->pid, tokens[1]);

                }

                sem_post(&sem_dispatch);

            break;

            case SIGNAL:

                sem_wait(&sem_interrupcion);
                if (!FIFO_modo) pthread_cancel(thread_quantum);

                if (VRR_modo) actualizar_quantum(interrupcion->pcb, kernel_config->quantum);

                if (!dictionary_has_key(recursos, tokens[1])) {
                    pcb_a_exit(interrupcion->pcb);
                    finalizar_proceso_en_memoria(interrupcion->pcb->pid);
                    log_info(extra_logger, "Finalizo proceso %d - Motivo: INVALID_RESOURCE", interrupcion->pcb->pid);
                    sem_post(&sem_multiprogramacion_ready);//aumento grado multi

                } else if (obtenerInstancia(recursos, tokens[1]) > 0) {
                   
                        cola_blocked_a_ready(tokens[1]);
                        if (VRR_modo) sem_post(&sem_hay_encolado_VRR);
                    
                }

                sumar_recurso(recursos, tokens[1]);

                sem_post(&sem_dispatch);

            break;

        }

        liberar_paquete(paquete);
    }
    
return NULL;
}


void actualizar_quantum(pcb_t *pcb, int quantum) {
    pcb->quantum = quantum;
}

bool verificar_instruccion(t_dictionary *diccionario, char **tokens) {

    interfaz_t *interfaz = dictionary_get(diccionario, tokens[1]);
    int instruccion = obtener_tipo_instruccion(tokens[0]);

    int casos_inst[] = {I_IO_FS_CREATE, I_IO_FS_DELETE, I_IO_FS_READ, I_IO_FS_TRUNCATE,I_IO_FS_WRITE};

    switch(interfaz->tipo){

        case GENERIC:
            return instruccion == I_IO_GEN_SLEEP;

        case STDIN:
            return instruccion == I_IO_FS_READ;

        case STDOUT:
            return instruccion == I_IO_FS_WRITE;

        case DIALFS:
        
            for(int i=0; i < sizeof(casos_inst); i++){
                if(casos_inst[i]==instruccion){
                    return true;
                }
            }
            return false;
        
        default:

            return false;

    }

}


int ejecutar_IO(int socket_interfaz, char **instruccion_tokens) {

    //int socket_interfaz = *(int*)dictionary_get(interfaces,instruccion_tokens[1]);
    
    switch(obtener_tipo_instruccion(instruccion_tokens[0])){

        case I_IO_GEN_SLEEP:
            send_io_gen_sleep(socket_interfaz, atoi(instruccion_tokens[2]));
        break;

        case I_IO_STDIN_READ:
            send_io_stdin_read(socket_interfaz, atoi(instruccion_tokens[2]), atoi(instruccion_tokens[3]));
        break;

        case I_IO_STDOUT_WRITE:
            send_io_stdout_write(socket_interfaz, atoi(instruccion_tokens[2]), atoi(instruccion_tokens[3]));
        break;

        default:
        printf("Fallo lectura instruccion");
        break;

    }

    return recibir_operacion(socket_interfaz); //se queda esperando hasta que recibe el ok de la interfaz

}

char** split_string(char* str) {
    int spaces = 0;
    char* temp = str;
    //char* save_ptr;

    while (*temp) {
        if (*temp == ' ') spaces++;
        temp++;
    }

    char** result = malloc((spaces + 2) * sizeof(char*));
    if (!result) {
        fprintf(stderr, "Error en la asignación de memoria.\n");
        exit(EXIT_FAILURE);
    }

    char* str_copy = strdup(str);
    if (!str_copy) {
        fprintf(stderr, "Error en la asignación de memoria.\n");
        exit(EXIT_FAILURE);
    }

    int idx = 0;
    char* token = strtok(str_copy, " ");
    while (token) {
        result[idx++] = token;
        token = strtok(NULL, " ");
    }
    result[idx] = NULL;

    return result;
}

int obtener_tipo_instruccion(const char* tipo_str) {
    if (strcmp(tipo_str, "SET") == 0) return I_SET;
    if (strcmp(tipo_str, "SUM") == 0) return I_SUM;
    if (strcmp(tipo_str, "SUB") == 0) return I_SUB;
    if (strcmp(tipo_str, "JNZ") == 0) return I_JNZ;
    if (strcmp(tipo_str, "MOV_IN") == 0) return I_MOV_IN;
    if (strcmp(tipo_str, "MOV_OUT") == 0) return I_MOV_OUT;
    if (strcmp(tipo_str, "RESIZE") == 0) return I_RESIZE;
    if (strcmp(tipo_str, "COPY_STRING") == 0) return I_COPY_STRING;
    if (strcmp(tipo_str, "WAIT") == 0) return I_WAIT;
    if (strcmp(tipo_str, "SIGNAL") == 0) return I_SIGNAL;
    if (strcmp(tipo_str, "IO_GEN_SLEEP") == 0) return I_IO_GEN_SLEEP;
    if (strcmp(tipo_str, "IO_STDIN_READ") == 0) return I_IO_STDIN_READ;
    if (strcmp(tipo_str, "IO_STDOUT_WRITE") == 0) return I_IO_STDOUT_WRITE;
    if (strcmp(tipo_str, "IO_FS_CREATE") == 0) return I_IO_FS_CREATE;
    if (strcmp(tipo_str, "IO_FS_DELETE") == 0) return I_IO_FS_DELETE;
    if (strcmp(tipo_str, "IO_FS_TRUNCATE") == 0) return I_IO_FS_TRUNCATE;
    if (strcmp(tipo_str, "IO_FS_WRITE") == 0) return I_IO_FS_WRITE;
    if (strcmp(tipo_str, "IO_FS_READ") == 0) return I_IO_FS_READ;
    if (strcmp(tipo_str, "EXIT") == 0) return I_EXIT;
    return -1;
}


void agregar_instruccion(interfaz_t *interfaz, char** tokens) {
    pthread_mutex_lock(&interfaz->mutex_IO_instruccion); // Mutex para proteger la lista
    list_push(interfaz->instruccion_IO, tokens);
    pthread_mutex_unlock(&interfaz->mutex_IO_instruccion);
}