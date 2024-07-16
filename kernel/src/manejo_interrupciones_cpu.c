#include "scheduler.h"
#include "manejo_interrupciones_cpu.h"
#include "kernel_interface_handler.h"
#include "recursos.h"
#include "consola.h"

extern t_log *logger, *extra_logger;
extern t_kernel_config* kernel_config;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern scheduler_t* scheduler;
extern pthread_t thread_quantum;
extern sem_t sem_dispatch, sem_interrupcion, sem_multiprogramacion_ready, sem_hay_encolado_VRR, sem_manejo_interrupciones_comando;
extern t_dictionary *interfaces;
extern t_dictionary *instrucciones;
extern pthread_mutex_t diccionario_instrucciones_mutex;
extern t_temporal *tiempoVRR;
int64_t tiempo_VRR_restante;
extern t_dictionary *recursos;
extern bool VRR_modo, FIFO_modo;
extern bool estado_planificacion_activa;
extern pthread_mutex_t mutex_tiempoVRR;
extern pthread_mutex_t cantidad_procesos_ejecuntandose_mutex;
extern int cantidad_procesos_ejecuntandose;

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

    while(1) {


        sem_wait(&sem_interrupcion);
        paquete_t *paquete = recibir_contexto(conexion_dispatch);

        //VRR
        if(VRR_modo) {
            pthread_mutex_lock(&mutex_tiempoVRR);
            tiempo_VRR_restante = temporal_gettime(tiempoVRR);
            temporal_destroy(tiempoVRR);
            pthread_mutex_unlock(&mutex_tiempoVRR);
        }

        contexto_t *contexto = contexto_deserializar(paquete->payload);

        actualizar_contexto_de_ejecucion(scheduler->proceso_ejecutando, contexto->pcb);

        char **tokens = split_string(contexto->instruccion);

        liberar_contexto(contexto);

        if (!estado_planificacion_activa) {
            sem_wait(&sem_manejo_interrupciones_comando);
        }

        switch (paquete->operacion) {
            case FINALIZADO:

                // imprimir_pcb(scheduler->proceso_ejecutando);
                if (!FIFO_modo) pthread_cancel(thread_quantum);
                // liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                log_info(logger, "Finalizo proceso %d - Motivo: SUCCESS", scheduler->proceso_ejecutando->pid);
                proceso_exec_a_exit();
                sem_post(&sem_dispatch);
                aumentar_grado_multiprogramacion();

            break;

            case OUT_OF_MEMORY:
                // imprimir_pcb(scheduler->proceso_ejecutando);
                if (!FIFO_modo) pthread_cancel(thread_quantum);
                // liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                log_info(logger, "Finalizo proceso %d - Motivo: OUT_OF_MEMORY", scheduler->proceso_ejecutando->pid);
                proceso_exec_a_exit();
                sem_post(&sem_dispatch);
                aumentar_grado_multiprogramacion();
            break;

            case INTERRUPTED_BY_USER:
                // imprimir_pcb(scheduler->proceso_ejecutando);
                if (!FIFO_modo) pthread_cancel(thread_quantum);
                // liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                liberar_recursos_de_proceso(scheduler->proceso_ejecutando->pid);
                log_info(logger, "Finalizo proceso %d - Motivo: INTERRUPTED_BY_USER", scheduler->proceso_ejecutando->pid);
                proceso_exec_a_exit();
                sem_post(&sem_dispatch);
                aumentar_grado_multiprogramacion();
            break;

            case DESALOJO_QUANTUM:

                if (VRR_modo) actualizar_quantum(scheduler->proceso_ejecutando, kernel_config->quantum);
                //pcb_a_ready(scheduler->proceso_ejecutando);
                if (VRR_modo) sem_post(&sem_hay_encolado_VRR);
                log_info(logger, "PID: %d - Desalojado por Fin de Quantum", scheduler->proceso_ejecutando->pid);
                proceso_exec_a_ready();
                sem_post(&sem_dispatch);

            break;

            case IO:

                if (!FIFO_modo) pthread_cancel(thread_quantum);

                if (VRR_modo) actualizar_quantum(scheduler->proceso_ejecutando, scheduler->proceso_ejecutando->quantum - tiempo_VRR_restante);
                log_debug(extra_logger, "PID: %d - Tiempo restante Quantum: %ld", scheduler->proceso_ejecutando->pid, tiempo_VRR_restante);

                // Verifico que existe la interfaz y la instrucción coincida con la IO que corresponde
                if (!dictionary_has_key(interfaces, tokens[1]) || !verificar_instruccion(interfaces, tokens)) {

                    finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                    log_info(logger, "Finalizo proceso %d - Motivo: INVALID_INTERFACE", scheduler->proceso_ejecutando->pid);
                    proceso_exec_a_exit();//si No existe
                    aumentar_grado_multiprogramacion();

                } else {

                    interfaz_t *interfaz = dictionary_get(interfaces, tokens[1]);
                    log_info(logger, "PID: %d - Bloqueado por: %s", scheduler->proceso_ejecutando->pid, tokens[1]);
                    agregar_instruccion(scheduler->proceso_ejecutando->pid, tokens);
                    proceso_exec_a_blocked(tokens[1]);
                    //pcb_a_blocked(scheduler->proceso_ejecutando, tokens[1]); //SI existe
                    sem_post(&interfaz->sem_IO_ejecucion);
                }

                sem_post(&sem_dispatch);

            break;

            case WAIT:

                if (!FIFO_modo) pthread_cancel(thread_quantum);

                if (VRR_modo) actualizar_quantum(scheduler->proceso_ejecutando, kernel_config->quantum);

                if (!dictionary_has_key(recursos, tokens[1])) {
                    
                    finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                    log_info(logger, "Finalizo proceso %d - Motivo: INVALID_RESOURCE", scheduler->proceso_ejecutando->pid);
                    proceso_exec_a_exit();
                    aumentar_grado_multiprogramacion();

                } else {
                    asignar_recurso_a_proceso(scheduler->proceso_ejecutando->pid, tokens[1]);
                    restar_recurso(recursos,tokens[1]);
                }

                if (obtenerInstancia(recursos, tokens[1]) < 0) {

                    log_info(logger, "PID: %d - Bloqueado por: %s", scheduler->proceso_ejecutando->pid, tokens[1]);
                    proceso_exec_a_blocked(tokens[1]);
                    sem_post(&sem_dispatch);

                } else {

                    send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);
                    sem_post(&sem_interrupcion);
                }

            break;

            case SIGNAL:

                if (!FIFO_modo) pthread_cancel(thread_quantum);

                if (VRR_modo) actualizar_quantum(scheduler->proceso_ejecutando, kernel_config->quantum);

                if (!dictionary_has_key(recursos, tokens[1])) {
                
                    finalizar_proceso_en_memoria(scheduler->proceso_ejecutando->pid);
                    log_info(logger, "Finalizo proceso %d - Motivo: INVALID_RESOURCE", scheduler->proceso_ejecutando->pid);
                    proceso_exec_a_exit();
                    aumentar_grado_multiprogramacion();

                } else {

                    liberar_recurso_de_proceso(scheduler->proceso_ejecutando->pid, tokens[1]);
                    sumar_recurso(recursos, tokens[1]);
                }

                if (obtenerInstancia(recursos, tokens[1]) >= 0){
                    cola_blocked_a_ready(tokens[1]);
                    if (VRR_modo) sem_post(&sem_hay_encolado_VRR);
                }

                send_pcb(conexion_dispatch, scheduler->proceso_ejecutando);
                sem_post(&sem_interrupcion);

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

    int casos_inst[] = {I_IO_FS_CREATE, I_IO_FS_DELETE, I_IO_FS_READ, I_IO_FS_TRUNCATE, I_IO_FS_WRITE};

    switch(interfaz->tipo){

        case GENERIC:
            return instruccion == I_IO_GEN_SLEEP;

        case STDIN:
            return instruccion == I_IO_STDIN_READ;

        case STDOUT:
            return instruccion == I_IO_STDOUT_WRITE;

        case DIALFS:

            for (int i=0; i < sizeof(casos_inst); i++) {
                if (casos_inst[i]==instruccion) {
                    return true;
                }
            }
            return false;

        default:
            return false;
    }
}

int ejecutar_IO(int socket_interfaz, uint16_t pid, char **instruccion_tokens) {

    switch (obtener_tipo_instruccion(instruccion_tokens[0])) {

        case I_IO_GEN_SLEEP:
            send_io_gen_sleep(socket_interfaz, pid, atoi(instruccion_tokens[2]));
        break;

        case I_IO_STDIN_READ:
            send_io_stdin_read(socket_interfaz, pid, atoi(instruccion_tokens[2]), atoi(instruccion_tokens[3]));
        break;

        case I_IO_STDOUT_WRITE:
            send_io_stdout_write(socket_interfaz, pid, atoi(instruccion_tokens[2]), atoi(instruccion_tokens[3]));
        break;

        default:
            printf("Fallo lectura instruccion");
        break;
    }

    return recibir_operacion(socket_interfaz); // Se queda esperando hasta que recibe el ok de la interfaz
}

char** split_string(char* str) {
    int spaces = 0;
    char* temp = str;

    if (str == NULL) {
        fprintf(stderr, "split_string: String nulo!\n");
        exit(EXIT_FAILURE);
    }

    while (*temp) {
        if (*temp == ' ')
            spaces++;
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

void agregar_instruccion(uint16_t pid, char** tokens) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid);
    pthread_mutex_lock(&diccionario_instrucciones_mutex);
    dictionary_put(instrucciones, str_pid, tokens);
    pthread_mutex_unlock(&diccionario_instrucciones_mutex);
}

void aumentar_grado_multiprogramacion() {
    if (cantidad_procesos_ejecuntandose <= kernel_config->grado_multiprogramacion) {
        sem_post(&sem_multiprogramacion_ready);
    }

    pthread_mutex_lock(&cantidad_procesos_ejecuntandose_mutex);
    cantidad_procesos_ejecuntandose--;
    pthread_mutex_unlock(&cantidad_procesos_ejecuntandose_mutex);
}